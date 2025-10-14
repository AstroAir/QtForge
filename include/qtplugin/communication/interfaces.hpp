/**
 * @file interfaces.hpp
 * @brief Core communication interfaces following SOLID principles
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <vector>
#include "../utils/error_handling.hpp"

namespace qtplugin::communication {

// Forward declarations
class IMessage;
class ISubscription;
class IStatistics;

/**
 * @brief Message priority levels
 */
enum class MessagePriority { Low = 0, Normal = 1, High = 2, Critical = 3 };

/**
 * @brief Message delivery modes
 */
enum class DeliveryMode {
    Broadcast,  ///< Send to all subscribers
    Unicast,    ///< Send to specific recipient
    Multicast   ///< Send to specific group of recipients
};

/**
 * @brief Error information for communication operations
 */
struct CommunicationError {
    enum class Type {
        InvalidMessage,
        NoSubscribers,
        DeliveryFailed,
        TimeoutExpired,
        InvalidHandler,
        SystemError
    };

    Type type;
    std::string message;
    std::string details;
};

/**
 * @brief Result type for communication operations
 * Uses expected<T, E> pattern which properly handles void results
 */
template <typename T>
using Result = qtplugin::expected<T, CommunicationError>;

/**
 * @brief Base message interface (Interface Segregation Principle)
 */
class IMessage {
public:
    virtual ~IMessage() = default;

    virtual std::string_view type() const noexcept = 0;
    virtual std::string_view sender() const noexcept = 0;
    virtual std::chrono::system_clock::time_point timestamp()
        const noexcept = 0;
    virtual MessagePriority priority() const noexcept = 0;
    virtual QJsonObject to_json() const = 0;
    virtual std::string id() const noexcept = 0;
};

/**
 * @brief Message publisher interface (Single Responsibility Principle)
 */
class IMessagePublisher {
public:
    virtual ~IMessagePublisher() = default;

    virtual Result<void> publish(
        std::shared_ptr<IMessage> message,
        DeliveryMode mode = DeliveryMode::Broadcast,
        const std::vector<std::string>& recipients = {}) = 0;

    virtual std::future<Result<void>> publish_async(
        std::shared_ptr<IMessage> message,
        DeliveryMode mode = DeliveryMode::Broadcast,
        const std::vector<std::string>& recipients = {}) = 0;
};

/**
 * @brief Message subscription interface (Interface Segregation Principle)
 */
class ISubscription {
public:
    virtual ~ISubscription() = default;

    virtual std::string id() const noexcept = 0;
    virtual std::string_view subscriber_id() const noexcept = 0;
    virtual std::type_index message_type() const noexcept = 0;
    virtual bool is_active() const noexcept = 0;
    virtual void cancel() = 0;

    /**
     * @brief Deliver a message to this subscription's handler
     * @param message The message to deliver
     * @return Result indicating success or failure
     *
     * This method enables the MessageRouter to actually invoke the
     * subscription's handler without exposing the handler itself, maintaining
     * encapsulation. Thread-safe: Can be called from multiple threads
     * concurrently.
     */
    virtual Result<void> deliver(const IMessage& message) = 0;
};

/**
 * @brief Message subscription manager interface (Single Responsibility
 * Principle)
 */
class ISubscriptionManager {
public:
    virtual ~ISubscriptionManager() = default;

    using MessageHandler = std::function<void(const IMessage&)>;
    using MessageFilter = std::function<bool(const IMessage&)>;

    virtual Result<std::shared_ptr<ISubscription>> subscribe(
        std::string_view subscriber_id, std::type_index message_type,
        MessageHandler handler, MessageFilter filter = nullptr) = 0;

    virtual Result<void> unsubscribe(const std::string& subscription_id) = 0;
    virtual Result<void> unsubscribe_all(std::string_view subscriber_id) = 0;

    virtual std::vector<std::shared_ptr<ISubscription>> get_subscriptions(
        std::string_view subscriber_id = {}) const = 0;
};

/**
 * @brief Statistics collection interface (Interface Segregation Principle)
 */
class IStatistics {
public:
    virtual ~IStatistics() = default;

    struct MessageStats {
        size_t total_published = 0;
        size_t total_delivered = 0;
        size_t total_failed = 0;
        std::chrono::milliseconds avg_delivery_time{0};
        std::chrono::system_clock::time_point last_activity;
    };

    struct SubscriptionStats {
        size_t active_subscriptions = 0;
        size_t total_subscriptions = 0;
        std::map<std::string, size_t> subscribers_by_type;
    };

    virtual MessageStats get_message_stats() const = 0;
    virtual SubscriptionStats get_subscription_stats() const = 0;
    virtual void reset_stats() = 0;
};

/**
 * @brief Message routing interface (Single Responsibility Principle)
 */
class IMessageRouter {
public:
    virtual ~IMessageRouter() = default;

    virtual Result<std::vector<std::shared_ptr<ISubscription>>>
    find_subscribers(const IMessage& message, DeliveryMode mode,
                     const std::vector<std::string>& recipients = {}) const = 0;

    virtual Result<void> deliver_message(
        const IMessage& message,
        const std::vector<std::shared_ptr<ISubscription>>& subscriptions) = 0;
};

/**
 * @brief Event system interface (Dependency Inversion Principle)
 */
class IEventSystem {
public:
    virtual ~IEventSystem() = default;

    template <typename EventType>
    Result<void> publish_event(const EventType& event) {
        return publish_event_impl(std::make_shared<EventType>(event));
    }

    template <typename EventType>
    Result<std::shared_ptr<ISubscription>> subscribe_to_event(
        std::string_view subscriber_id,
        std::function<void(const EventType&)> handler,
        std::function<bool(const EventType&)> filter = nullptr) {
        auto type_erased_handler = [handler](const IMessage& msg) {
            if (const auto* typed_event =
                    dynamic_cast<const EventType*>(&msg)) {
                handler(*typed_event);
            }
        };

        std::function<bool(const IMessage&)> type_erased_filter;
        if (filter) {
            type_erased_filter = [filter](const IMessage& msg) -> bool {
                if (const auto* typed_event =
                        dynamic_cast<const EventType*>(&msg)) {
                    return filter(*typed_event);
                }
                return false;
            };
        }

        return subscribe_event_impl(subscriber_id,
                                    std::type_index(typeid(EventType)),
                                    type_erased_handler, type_erased_filter);
    }

protected:
    virtual Result<void> publish_event_impl(
        std::shared_ptr<IMessage> event) = 0;
    virtual Result<std::shared_ptr<ISubscription>> subscribe_event_impl(
        std::string_view subscriber_id, std::type_index event_type,
        std::function<void(const IMessage&)> handler,
        std::function<bool(const IMessage&)> filter) = 0;
};

/**
 * @brief Request-response service interface (Interface Segregation Principle)
 */
class IRequestResponseService {
public:
    virtual ~IRequestResponseService() = default;

    using RequestHandler = std::function<QJsonObject(const QJsonObject&)>;

    virtual Result<void> register_service(std::string_view service_name,
                                          RequestHandler handler) = 0;

    virtual Result<void> unregister_service(std::string_view service_name) = 0;

    virtual Result<QJsonObject> call_service(
        std::string_view service_name, const QJsonObject& request,
        std::chrono::milliseconds timeout =
            std::chrono::milliseconds(5000)) = 0;

    virtual std::future<Result<QJsonObject>> call_service_async(
        std::string_view service_name, const QJsonObject& request,
        std::chrono::milliseconds timeout =
            std::chrono::milliseconds(5000)) = 0;

    virtual std::vector<std::string> list_services() const = 0;
};

}  // namespace qtplugin::communication
