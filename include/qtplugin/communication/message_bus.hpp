/**
 * @file message_bus.hpp
 * @brief Type-safe message bus for plugin communication
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QThreadPool>

#include <any>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../utils/error_handling.hpp"

namespace qtplugin {

/**
 * @brief Message priority levels
 */
enum class MessagePriority { Low = 0, Normal = 1, High = 2, Critical = 3 };

/**
 * @brief Message delivery modes
 */
enum class DeliveryMode {
    Immediate,  ///< Deliver immediately (synchronous)
    Queued,     ///< Queue for later delivery (asynchronous)
    Broadcast,  ///< Broadcast to all subscribers
    Unicast,    ///< Send to specific recipient
    Multicast,  ///< Send to multiple specific recipients
    Targeted    ///< Send to targeted recipients
};

/**
 * @brief Base message interface
 */
class IMessage {
public:
    virtual ~IMessage() = default;

    /**
     * @brief Get message type identifier
     */
    virtual std::string_view type() const noexcept = 0;

    /**
     * @brief Get message sender
     */
    virtual std::string_view sender() const noexcept = 0;

    /**
     * @brief Get message timestamp
     */
    virtual std::chrono::system_clock::time_point timestamp()
        const noexcept = 0;

    /**
     * @brief Get message priority
     */
    virtual MessagePriority priority() const noexcept = 0;

    /**
     * @brief Serialize message to JSON
     */
    virtual QJsonObject to_json() const = 0;

    /**
     * @brief Get message ID
     */
    virtual std::string id() const noexcept = 0;
};

/**
 * @brief Template base class for typed messages
 */
template <typename Derived>
class Message : public IMessage {
public:
    /**
     * @brief Constructs a typed message with sender and optional priority
     * @param sender Identifier of the message sender
     * @param priority Priority level of the message (default: Normal)
     */
    explicit Message(std::string_view sender,
            MessagePriority priority = MessagePriority::Normal)
        : m_sender(sender),
          m_timestamp(std::chrono::system_clock::now()),
          m_priority(priority),
          m_id(generate_id()) {}

    std::string_view sender() const noexcept override { return m_sender; }
    std::chrono::system_clock::time_point timestamp() const noexcept override {
        return m_timestamp;
    }
    MessagePriority priority() const noexcept override { return m_priority; }
    std::string id() const noexcept override { return m_id; }

    std::string_view type() const noexcept override {
        return typeid(Derived).name();
    }

private:
    std::string m_sender;
    std::chrono::system_clock::time_point m_timestamp;
    MessagePriority m_priority;
    std::string m_id;

    /**
     * @brief Generates a unique identifier for the message
     * @return A unique string ID
     */
    static std::string generate_id() {
        static std::atomic<uint64_t> counter{0};
        return std::to_string(counter.fetch_add(1));
    }
};

/**
 * @brief Message handler interface
 */
template <typename MessageType>
class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;

    /**
     * @brief Handle a message
     * @param message Message to handle
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> handle_message(
        const MessageType& message) = 0;

    /**
     * @brief Check if handler can process the message
     * @param message Message to check
     * @return true if handler can process the message
     */
    virtual bool can_handle(const MessageType& message) const {
        Q_UNUSED(message)
        return true;
    }
};

/**
 * @brief Subscription information
 */
struct Subscription {
    std::string subscriber_id;
    std::type_index message_type;
    std::any handler;
    std::function<bool(const IMessage&)> filter;
    bool is_active = true;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_message_time;
    uint64_t message_count = 0;
    uint64_t messages_received = 0;

    /**
     * @brief Constructs a new subscription entry
     * @param id Identifier of the subscriber
     * @param type Type index of the message
     * @param h Handler any for the subscription
     */
    Subscription(std::string_view id, std::type_index type, std::any h)
        : subscriber_id(id),
          message_type(type),
          handler(std::move(h)),
          created_at(std::chrono::system_clock::now()),
          last_message_time(std::chrono::system_clock::now()) {}
};

/**
 * @brief Message bus interface
 */
class IMessageBus {
public:
    virtual ~IMessageBus() = default;

    /**
     * @brief Publish a message
     * @tparam MessageType Type of message to publish
     * @param message Message to publish
     * @param mode Delivery mode
     * @param recipients Specific recipients (for unicast/multicast)
     * @return Success or error information
     */
    template <typename MessageType>
    qtplugin::expected<void, PluginError> publish(
        const MessageType& message, DeliveryMode mode = DeliveryMode::Broadcast,
        const std::vector<std::string>& recipients = {}) {
        return publish_impl(std::make_shared<MessageType>(message), mode,
                            recipients);
    }

    /**
     * @brief Publish a message asynchronously
     * @tparam MessageType Type of message to publish
     * @param message Message to publish
     * @param mode Delivery mode
     * @param recipients Specific recipients (for unicast/multicast)
     * @return Future with success or error information
     */
    template <typename MessageType>
    std::future<qtplugin::expected<void, PluginError>> publish_async(
        const MessageType& message, DeliveryMode mode = DeliveryMode::Broadcast,
        const std::vector<std::string>& recipients = {}) {
        return publish_async_impl(std::make_shared<MessageType>(message), mode,
                                  recipients);
    }

    /**
     * @brief Subscribe to messages of a specific type
     * @tparam MessageType Type of message to subscribe to
     * @param subscriber_id Subscriber identifier
     * @param handler Message handler function
     * @param filter Optional message filter
     * @return Success or error information
     */
    template <typename MessageType>
    qtplugin::expected<void, PluginError> subscribe(
        std::string_view subscriber_id,
        std::function<qtplugin::expected<void, PluginError>(const MessageType&)>
            handler,
        std::function<bool(const MessageType&)> filter = nullptr) {
        std::function<bool(const IMessage&)> generic_filter;
        if (filter) {
            generic_filter = [filter](const IMessage& msg) {
                if (auto typed_msg = dynamic_cast<const MessageType*>(&msg)) {
                    return filter(*typed_msg);
                }
                return false;
            };
        }

        // Create a generic handler wrapper
        auto generic_handler = std::function<qtplugin::expected<void, PluginError>(std::shared_ptr<IMessage>)>(
            [handler](std::shared_ptr<IMessage> msg) -> qtplugin::expected<void, PluginError> {
                if (auto typed_msg = std::dynamic_pointer_cast<MessageType>(msg)) {
                    return handler(*typed_msg);
                }
                return make_error<void>(PluginErrorCode::InvalidParameters, "Message type mismatch");
            });

        return subscribe_impl(
            subscriber_id, std::type_index(typeid(MessageType)),
            std::make_any<std::function<qtplugin::expected<void, PluginError>(std::shared_ptr<IMessage>)>>(generic_handler),
            generic_filter);
    }

    /**
     * @brief Unsubscribe from messages
     * @param subscriber_id Subscriber identifier
     * @param message_type Message type to unsubscribe from (optional, all types if empty)
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> unsubscribe(
        std::string_view subscriber_id,
        std::optional<std::type_index> message_type) = 0;

    /**
     * @brief Get list of subscribers for a message type
     * @param message_type Message type
     * @return Vector of subscriber IDs
     */
    virtual std::vector<std::string> subscribers(
        std::type_index message_type) const = 0;

    /**
     * @brief Get subscription information
     * @param subscriber_id Subscriber identifier
     * @return Vector of subscription information
     */
    virtual std::vector<Subscription> subscriptions(
        std::string_view subscriber_id) const = 0;

    /**
     * @brief Check if subscriber exists
     * @param subscriber_id Subscriber identifier
     * @return true if subscriber exists
     */
    virtual bool has_subscriber(std::string_view subscriber_id) const = 0;

    /**
     * @brief Get message bus statistics
     * @return Statistics as JSON object
     */
    virtual QJsonObject statistics() const = 0;

    /**
     * @brief Clear all subscriptions
     */
    virtual void clear() = 0;

    /**
     * @brief Enable or disable message logging
     * @param enabled true to enable logging
     */
    virtual void set_logging_enabled(bool enabled) = 0;

    /**
     * @brief Check if message logging is enabled
     * @return true if logging is enabled
     */
    virtual bool is_logging_enabled() const = 0;

    /**
     * @brief Get message log
     * @param limit Maximum number of messages to return (0 for all)
     * @return Vector of logged messages
     */
    virtual std::vector<QJsonObject> message_log(size_t limit) const = 0;

protected:
    virtual qtplugin::expected<void, PluginError> publish_impl(
        std::shared_ptr<IMessage> message, DeliveryMode mode,
        const std::vector<std::string>& recipients) = 0;

    virtual std::future<qtplugin::expected<void, PluginError>>
    publish_async_impl(std::shared_ptr<IMessage> message, DeliveryMode mode,
                       const std::vector<std::string>& recipients) = 0;

    virtual qtplugin::expected<void, PluginError> subscribe_impl(
        std::string_view subscriber_id, std::type_index message_type,
        std::any handler, std::function<bool(const IMessage&)> filter) = 0;
};

/**
 * @brief Default message bus implementation
 */
class MessageBus : public QObject, public IMessageBus {
    Q_OBJECT

public:
    /**
     * @brief Constructs a new message bus instance
     * @param parent Optional parent QObject for Qt ownership
     */
    explicit MessageBus(QObject* parent = nullptr);
    
    /**
     * @brief Destroys the message bus and cleans up resources
     */
    ~MessageBus() override;

    // IMessageBus implementation
    qtplugin::expected<void, PluginError> unsubscribe(
        std::string_view subscriber_id,
        std::optional<std::type_index> message_type = std::nullopt) override;

    std::vector<std::string> subscribers(
        std::type_index message_type) const override;
    std::vector<Subscription> subscriptions(
        std::string_view subscriber_id) const override;
    bool has_subscriber(std::string_view subscriber_id) const override;
    QJsonObject statistics() const override;
    void clear() override;
    void set_logging_enabled(bool enabled) override;
    bool is_logging_enabled() const override;
    std::vector<QJsonObject> message_log(size_t limit) const override;

signals:
    /**
     * @brief Emitted when a message is published
     * @param message_type Message type name
     * @param sender Sender identifier
     * @param recipient_count Number of recipients
     */
    void message_published(const QString& message_type, const QString& sender,
                           int recipient_count);

    /**
     * @brief Emitted when a subscription is added
     * @param subscriber_id Subscriber identifier
     * @param message_type Message type name
     */
    void subscription_added(const QString& subscriber_id,
                            const QString& message_type);

    /**
     * @brief Emitted when a subscription is removed
     * @param subscriber_id Subscriber identifier
     * @param message_type Message type name
     */
    void subscription_removed(const QString& subscriber_id,
                              const QString& message_type);

protected:
    qtplugin::expected<void, PluginError> publish_impl(
        std::shared_ptr<IMessage> message, DeliveryMode mode,
        const std::vector<std::string>& recipients) override;

    std::future<qtplugin::expected<void, PluginError>> publish_async_impl(
        std::shared_ptr<IMessage> message, DeliveryMode mode,
        const std::vector<std::string>& recipients) override;

    qtplugin::expected<void, PluginError> subscribe_impl(
        std::string_view subscriber_id, std::type_index message_type,
        std::any handler, std::function<bool(const IMessage&)> filter) override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
    static constexpr size_t MAX_LOG_SIZE = 10000;

    /**
     * @brief Logs the message details if logging is enabled
     * @param message The message being published
     * @param recipients List of recipient IDs
     */
    void log_message(const IMessage& message,
                     const std::vector<std::string>& recipients);
    
    /**
     * @brief Delivers the message to the specified recipients
     * @param message Shared pointer to the message
     * @param recipients List of target recipient IDs
     * @return Success result or PluginError
     */
    qtplugin::expected<void, PluginError> deliver_message(
        const IMessage& message, const std::vector<std::string>& recipients);
    
    /**
     * @brief Finds appropriate recipients for the message
     * @param message_type Type index of the message
     * @param specific_recipients Optional specific recipients to filter
     * @return Vector of matching subscriber IDs
     */
    std::vector<std::string> find_recipients(
        std::type_index message_type,
        const std::vector<std::string>& specific_recipients) const;

    /**
     * @brief Delivers message synchronously to recipients
     * @param message Shared pointer to the message
     * @param recipients List of recipient IDs
     * @return Success result or PluginError
     */
    qtplugin::expected<void, PluginError> deliver_sync(
        std::shared_ptr<IMessage> message,
        const std::vector<std::string>& recipients);
    
    /**
     * @brief Delivers message asynchronously using thread pool
     * @param message Shared pointer to the message
     * @param recipients List of recipient IDs
     * @return Success result or PluginError
     */
    qtplugin::expected<void, PluginError> deliver_async(
        std::shared_ptr<IMessage> message,
        const std::vector<std::string>& recipients);
    
    /**
     * @brief Retrieves all subscribers for a given message type
     * @param message_type Type index of the message
     * @return Vector of all subscriber IDs
     */
    std::vector<std::string> get_all_subscribers(std::type_index message_type) const;
    
    /**
     * @brief Cleans up expired messages and inactive subscriptions periodically
     */
    void cleanup_expired_messages();
    
    /**
     * @brief Generates detailed statistics about the message bus
     * @return JSON object containing statistics
     */
    QJsonObject get_detailed_statistics() const;
    
    /**
     * @brief Calculates the total number of subscriptions
     * @return Total subscription count
     */
    size_t get_total_subscription_count() const;
    
    /**
     * @brief Calculates the number of active subscriptions
     * @return Active subscription count
     */
    size_t get_active_subscription_count() const;
};

}  // namespace qtplugin
