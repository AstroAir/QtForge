/**
 * @file typed_event_system.hpp
 * @brief Typed event system using Qt's meta-object system for plugin communication
 * @version 3.0.0
 */

#pragma once

#include "../core/plugin_interface.hpp"
#include "../utils/error_handling.hpp"
#include <QObject>
#include <QMetaObject>
#include <QMetaType>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QMutex>
#include <QUuid>
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <chrono>
#include <future>

namespace qtplugin {

/**
 * @brief Event priority levels
 */
enum class EventPriority {
    Lowest = 0,
    Low = 25,
    Normal = 50,
    High = 75,
    Highest = 100,
    Critical = 125
};

/**
 * @brief Event delivery modes
 */
enum class EventDeliveryMode {
    Immediate,              ///< Deliver immediately
    Queued,                 ///< Queue for later delivery
    Deferred,               ///< Defer until next event loop iteration
    Batched                 ///< Batch multiple events together
};

/**
 * @brief Event routing modes
 */
enum class EventRoutingMode {
    Broadcast,              ///< Send to all subscribers
    Unicast,                ///< Send to specific recipient
    Multicast,              ///< Send to multiple specific recipients
    RoundRobin,             ///< Distribute among subscribers
    LoadBalanced            ///< Load-balanced distribution
};

/**
 * @brief Base event interface
 */
class IEvent {
public:
    virtual ~IEvent() = default;
    
    /**
     * @brief Get event type identifier
     * @return Event type ID
     */
    virtual QString event_type() const = 0;
    
    /**
     * @brief Get event source
     * @return Source identifier
     */
    virtual QString source() const = 0;
    
    /**
     * @brief Get event timestamp
     * @return Event timestamp
     */
    virtual std::chrono::system_clock::time_point timestamp() const = 0;
    
    /**
     * @brief Get event priority
     * @return Event priority
     */
    virtual EventPriority priority() const { return EventPriority::Normal; }
    
    /**
     * @brief Get event metadata
     * @return Event metadata
     */
    virtual QJsonObject metadata() const { return QJsonObject{}; }
    
    /**
     * @brief Convert event to JSON
     * @return JSON representation
     */
    virtual QJsonObject to_json() const = 0;
    
    /**
     * @brief Clone the event
     * @return Cloned event
     */
    virtual std::unique_ptr<IEvent> clone() const = 0;
};

/**
 * @brief Typed event base class
 */
template<typename T>
class TypedEvent : public IEvent {
public:
    explicit TypedEvent(const QString& source, const T& data)
        : m_source(source)
        , m_data(data)
        , m_timestamp(std::chrono::system_clock::now())
        , m_event_id(QUuid::createUuid().toString()) {}
    
    QString event_type() const override {
        return QString::fromStdString(typeid(T).name());
    }
    
    QString source() const override { return m_source; }
    
    std::chrono::system_clock::time_point timestamp() const override { return m_timestamp; }
    
    QString event_id() const { return m_event_id; }
    
    const T& data() const { return m_data; }
    T& data() { return m_data; }
    
    QJsonObject to_json() const override {
        QJsonObject json;
        json["event_type"] = event_type();
        json["source"] = m_source;
        json["event_id"] = m_event_id;
        json["timestamp"] = QDateTime::fromSecsSinceEpoch(
            std::chrono::duration_cast<std::chrono::seconds>(m_timestamp.time_since_epoch()).count())
            .toString(Qt::ISODate);
        json["priority"] = static_cast<int>(priority());
        json["metadata"] = metadata();
        
        // Serialize data if it's a QVariant or has to_json method
        if constexpr (std::is_same_v<T, QVariant>) {
            json["data"] = QJsonValue::fromVariant(m_data);
        } else if constexpr (requires { m_data.to_json(); }) {
            json["data"] = m_data.to_json();
        } else {
            json["data"] = QJsonValue::fromVariant(QVariant::fromValue(m_data));
        }
        
        return json;
    }
    
    std::unique_ptr<IEvent> clone() const override {
        return std::make_unique<TypedEvent<T>>(m_source, m_data);
    }

private:
    QString m_source;
    T m_data;
    std::chrono::system_clock::time_point m_timestamp;
    QString m_event_id;
};

/**
 * @brief Event subscription information
 */
struct EventSubscription {
    QString subscription_id;                ///< Subscription identifier
    QString subscriber_id;                  ///< Subscriber identifier
    QString event_type;                     ///< Event type filter
    std::function<bool(const IEvent&)> filter; ///< Event filter function
    std::function<void(const IEvent&)> handler; ///< Event handler function
    EventPriority min_priority = EventPriority::Lowest; ///< Minimum priority filter
    bool is_active = true;                  ///< Whether subscription is active
    std::chrono::system_clock::time_point created_time; ///< Subscription creation time
    QJsonObject metadata;                   ///< Subscription metadata
    
    /**
     * @brief Convert to JSON object (excluding function pointers)
     */
    QJsonObject to_json() const;
};

/**
 * @brief Event delivery result
 */
struct EventDeliveryResult {
    QString event_id;                       ///< Event identifier
    bool success = false;                   ///< Delivery success
    int delivered_count = 0;                ///< Number of successful deliveries
    int failed_count = 0;                   ///< Number of failed deliveries
    std::vector<QString> delivered_to;      ///< List of successful recipients
    std::vector<QString> failed_to;         ///< List of failed recipients
    std::chrono::milliseconds delivery_time{0}; ///< Total delivery time
    QString error_message;                  ///< Error message if failed
    QJsonObject metadata;                   ///< Delivery metadata
    
    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Event statistics
 */
struct EventStatistics {
    uint64_t total_events_published = 0;    ///< Total events published
    uint64_t total_events_delivered = 0;    ///< Total events delivered
    uint64_t total_events_failed = 0;       ///< Total events failed
    uint64_t total_subscriptions = 0;       ///< Total active subscriptions
    std::chrono::milliseconds average_delivery_time{0}; ///< Average delivery time
    std::unordered_map<QString, uint64_t> events_by_type; ///< Events count by type
    std::unordered_map<QString, uint64_t> events_by_source; ///< Events count by source
    
    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Typed event system
 * 
 * This class provides a typed event system using Qt's meta-object system
 * for type-safe plugin communication with filtering, routing, and delivery guarantees.
 */
class TypedEventSystem : public QObject {
    Q_OBJECT
    
public:
    explicit TypedEventSystem(QObject* parent = nullptr);
    ~TypedEventSystem() override;
    
    // === Event Publishing ===
    
    /**
     * @brief Publish typed event
     * @param event Event to publish
     * @param delivery_mode Delivery mode
     * @param routing_mode Routing mode
     * @param recipients Specific recipients (for unicast/multicast)
     * @return Event delivery result or error
     */
    qtplugin::expected<EventDeliveryResult, PluginError>
    publish_event(std::unique_ptr<IEvent> event,
                 EventDeliveryMode delivery_mode = EventDeliveryMode::Immediate,
                 EventRoutingMode routing_mode = EventRoutingMode::Broadcast,
                 const std::vector<QString>& recipients = {});
    
    /**
     * @brief Publish typed event asynchronously
     * @param event Event to publish
     * @param delivery_mode Delivery mode
     * @param routing_mode Routing mode
     * @param recipients Specific recipients
     * @return Future with delivery result
     */
    std::future<qtplugin::expected<EventDeliveryResult, PluginError>>
    publish_event_async(std::unique_ptr<IEvent> event,
                       EventDeliveryMode delivery_mode = EventDeliveryMode::Immediate,
                       EventRoutingMode routing_mode = EventRoutingMode::Broadcast,
                       const std::vector<QString>& recipients = {});
    
    /**
     * @brief Create and publish typed event
     * @param source Event source
     * @param data Event data
     * @param delivery_mode Delivery mode
     * @param routing_mode Routing mode
     * @param recipients Specific recipients
     * @return Event delivery result or error
     */
    template<typename T>
    qtplugin::expected<EventDeliveryResult, PluginError>
    publish(const QString& source, const T& data,
           EventDeliveryMode delivery_mode = EventDeliveryMode::Immediate,
           EventRoutingMode routing_mode = EventRoutingMode::Broadcast,
           const std::vector<QString>& recipients = {}) {
        auto event = std::make_unique<TypedEvent<T>>(source, data);
        return publish_event(std::move(event), delivery_mode, routing_mode, recipients);
    }
    
    /**
     * @brief Publish batch of events
     * @param events Vector of events to publish
     * @param delivery_mode Delivery mode
     * @return Vector of delivery results
     */
    std::vector<qtplugin::expected<EventDeliveryResult, PluginError>>
    publish_batch(std::vector<std::unique_ptr<IEvent>> events,
                 EventDeliveryMode delivery_mode = EventDeliveryMode::Queued);
    
    // === Event Subscription ===
    
    /**
     * @brief Subscribe to typed events
     * @param subscriber_id Subscriber identifier
     * @param event_type Event type to subscribe to
     * @param handler Event handler function
     * @param filter Optional event filter function
     * @param min_priority Minimum event priority
     * @return Subscription ID or error
     */
    qtplugin::expected<QString, PluginError>
    subscribe(const QString& subscriber_id,
             const QString& event_type,
             std::function<void(const IEvent&)> handler,
             std::function<bool(const IEvent&)> filter = nullptr,
             EventPriority min_priority = EventPriority::Lowest);
    
    /**
     * @brief Subscribe to typed events with template
     * @param subscriber_id Subscriber identifier
     * @param handler Typed event handler function
     * @param filter Optional typed event filter function
     * @param min_priority Minimum event priority
     * @return Subscription ID or error
     */
    template<typename T>
    qtplugin::expected<QString, PluginError>
    subscribe(const QString& subscriber_id,
             std::function<void(const TypedEvent<T>&)> handler,
             std::function<bool(const TypedEvent<T>&)> filter = nullptr,
             EventPriority min_priority = EventPriority::Lowest) {
        
        QString event_type = QString::fromStdString(typeid(T).name());
        
        auto generic_handler = [handler](const IEvent& event) {
            if (const auto* typed_event = dynamic_cast<const TypedEvent<T>*>(&event)) {
                handler(*typed_event);
            }
        };
        
        std::function<bool(const IEvent&)> generic_filter = nullptr;
        if (filter) {
            generic_filter = [filter](const IEvent& event) -> bool {
                if (const auto* typed_event = dynamic_cast<const TypedEvent<T>*>(&event)) {
                    return filter(*typed_event);
                }
                return false;
            };
        }
        
        return subscribe(subscriber_id, event_type, generic_handler, generic_filter, min_priority);
    }
    
    /**
     * @brief Unsubscribe from events
     * @param subscription_id Subscription identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError>
    unsubscribe(const QString& subscription_id);
    
    /**
     * @brief Unsubscribe all subscriptions for subscriber
     * @param subscriber_id Subscriber identifier
     * @return Number of removed subscriptions
     */
    int unsubscribe_all(const QString& subscriber_id);
    
    /**
     * @brief Get subscriptions for subscriber
     * @param subscriber_id Subscriber identifier
     * @return Vector of subscription information
     */
    std::vector<EventSubscription> get_subscriptions(const QString& subscriber_id) const;
    
    /**
     * @brief Enable/disable subscription
     * @param subscription_id Subscription identifier
     * @param enabled Whether subscription is enabled
     * @return Success or error
     */
    qtplugin::expected<void, PluginError>
    set_subscription_enabled(const QString& subscription_id, bool enabled);
    
    // === Event Management ===
    
    /**
     * @brief Get pending events count
     * @return Number of pending events
     */
    size_t get_pending_events_count() const;
    
    /**
     * @brief Process pending events
     * @param max_events Maximum number of events to process
     * @return Number of processed events
     */
    size_t process_pending_events(size_t max_events = 100);
    
    /**
     * @brief Clear pending events
     * @param event_type Optional event type filter
     * @return Number of cleared events
     */
    size_t clear_pending_events(const QString& event_type = QString());
    
    /**
     * @brief Get event statistics
     * @return Event system statistics
     */
    EventStatistics get_statistics() const;
    
    /**
     * @brief Reset event statistics
     */
    void reset_statistics();
    
    // === Event History ===
    
    /**
     * @brief Enable event history
     * @param enabled Whether to enable event history
     * @param max_history_size Maximum history size
     */
    void set_event_history_enabled(bool enabled, size_t max_history_size = 1000);
    
    /**
     * @brief Get event history
     * @param event_type Optional event type filter
     * @param max_events Maximum number of events to return
     * @return Vector of historical events
     */
    std::vector<QJsonObject> get_event_history(const QString& event_type = QString(),
                                              size_t max_events = 100) const;

signals:
    /**
     * @brief Emitted when event is published
     * @param event_type Event type
     * @param source Event source
     * @param event_id Event identifier
     */
    void event_published(const QString& event_type, const QString& source, const QString& event_id);
    
    /**
     * @brief Emitted when event is delivered
     * @param event_id Event identifier
     * @param recipient Recipient identifier
     * @param success Whether delivery was successful
     */
    void event_delivered(const QString& event_id, const QString& recipient, bool success);
    
    /**
     * @brief Emitted when subscription is created
     * @param subscription_id Subscription identifier
     * @param subscriber_id Subscriber identifier
     * @param event_type Event type
     */
    void subscription_created(const QString& subscription_id, const QString& subscriber_id, const QString& event_type);
    
    /**
     * @brief Emitted when subscription is removed
     * @param subscription_id Subscription identifier
     */
    void subscription_removed(const QString& subscription_id);

private slots:
    void process_queued_events();
    void process_deferred_events();
    void process_batched_events();

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::EventPriority)
Q_DECLARE_METATYPE(qtplugin::EventDeliveryMode)
Q_DECLARE_METATYPE(qtplugin::EventRoutingMode)
Q_DECLARE_METATYPE(qtplugin::EventSubscription)
Q_DECLARE_METATYPE(qtplugin::EventDeliveryResult)
Q_DECLARE_METATYPE(qtplugin::EventStatistics)
