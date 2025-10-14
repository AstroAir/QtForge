/**
 * @file factory.hpp
 * @brief Factory interfaces for communication components (Dependency Inversion
 * Principle)
 * @version 3.0.0
 */

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include "interfaces.hpp"

namespace qtplugin::communication {

/**
 * @brief Configuration for communication components
 */
struct CommunicationConfig {
    struct MessageBusConfig {
        size_t max_queue_size = 10000;
        std::chrono::milliseconds delivery_timeout{5000};
        bool enable_statistics = true;
        bool enable_logging = false;
    } message_bus;

    struct EventSystemConfig {
        size_t max_event_history = 1000;
        bool enable_event_replay = false;
        std::chrono::milliseconds event_timeout{1000};
    } event_system;

    struct RequestResponseConfig {
        std::chrono::milliseconds default_timeout{5000};
        size_t max_concurrent_requests = 100;
        bool enable_request_logging = false;
    } request_response;

    struct NetworkConfig {
        bool enable_network_discovery = false;
        uint16_t discovery_port = 8080;
        std::chrono::milliseconds heartbeat_interval{30000};
    } network;
};

/**
 * @brief Factory interface for creating communication components
 * Implements Dependency Inversion Principle by allowing injection of
 * dependencies
 */
class ICommunicationFactory {
public:
    virtual ~ICommunicationFactory() = default;

    /**
     * @brief Create a message publisher
     */
    virtual std::unique_ptr<IMessagePublisher> create_publisher(
        const CommunicationConfig::MessageBusConfig& config = {}) = 0;

    /**
     * @brief Create a subscription manager
     */
    virtual std::unique_ptr<ISubscriptionManager> create_subscription_manager(
        const CommunicationConfig::MessageBusConfig& config = {}) = 0;

    /**
     * @brief Create a message router
     */
    virtual std::unique_ptr<IMessageRouter> create_router(
        std::shared_ptr<ISubscriptionManager> subscription_manager) = 0;

    /**
     * @brief Create a statistics collector
     */
    virtual std::unique_ptr<IStatistics> create_statistics() = 0;

    /**
     * @brief Create an event system
     */
    virtual std::unique_ptr<IEventSystem> create_event_system(
        const CommunicationConfig::EventSystemConfig& config = {}) = 0;

    /**
     * @brief Create a request-response service
     */
    virtual std::unique_ptr<IRequestResponseService>
    create_request_response_service(
        const CommunicationConfig::RequestResponseConfig& config = {}) = 0;
};

/**
 * @brief Builder for creating configured communication systems
 * Implements Builder pattern with fluent interface
 */
class CommunicationSystemBuilder {
public:
    explicit CommunicationSystemBuilder(
        std::shared_ptr<ICommunicationFactory> factory);

    CommunicationSystemBuilder& with_config(const CommunicationConfig& config);
    CommunicationSystemBuilder& with_publisher(
        std::unique_ptr<IMessagePublisher> publisher);
    CommunicationSystemBuilder& with_subscription_manager(
        std::unique_ptr<ISubscriptionManager> manager);
    CommunicationSystemBuilder& with_router(
        std::unique_ptr<IMessageRouter> router);
    CommunicationSystemBuilder& with_statistics(
        std::unique_ptr<IStatistics> statistics);
    CommunicationSystemBuilder& with_event_system(
        std::unique_ptr<IEventSystem> event_system);
    CommunicationSystemBuilder& with_request_response(
        std::unique_ptr<IRequestResponseService> service);

    /**
     * @brief Build the complete communication system
     */
    class CommunicationSystem build();

private:
    std::shared_ptr<ICommunicationFactory> factory_;
    CommunicationConfig config_;
    std::unique_ptr<IMessagePublisher> publisher_;
    std::unique_ptr<ISubscriptionManager> subscription_manager_;
    std::unique_ptr<IMessageRouter> router_;
    std::unique_ptr<IStatistics> statistics_;
    std::unique_ptr<IEventSystem> event_system_;
    std::unique_ptr<IRequestResponseService> request_response_;
};

/**
 * @brief Complete communication system facade
 * Implements Facade pattern to provide unified interface
 */
class CommunicationSystem {
public:
    CommunicationSystem(
        std::unique_ptr<IMessagePublisher> publisher,
        std::unique_ptr<ISubscriptionManager> subscription_manager,
        std::unique_ptr<IMessageRouter> router,
        std::unique_ptr<IStatistics> statistics,
        std::unique_ptr<IEventSystem> event_system,
        std::unique_ptr<IRequestResponseService> request_response);

    // Message Bus Interface
    Result<void> publish_message(
        std::shared_ptr<IMessage> message,
        DeliveryMode mode = DeliveryMode::Broadcast,
        const std::vector<std::string>& recipients = {});

    Result<std::shared_ptr<ISubscription>> subscribe_to_messages(
        std::string_view subscriber_id, std::type_index message_type,
        ISubscriptionManager::MessageHandler handler,
        ISubscriptionManager::MessageFilter filter = nullptr);

    // Event System Interface
    template <typename EventType>
    Result<void> publish_event(const EventType& event) {
        return event_system_->publish_event(event);
    }

    template <typename EventType>
    Result<std::shared_ptr<ISubscription>> subscribe_to_event(
        std::string_view subscriber_id,
        std::function<void(const EventType&)> handler,
        std::function<bool(const EventType&)> filter = nullptr) {
        return event_system_->subscribe_to_event(subscriber_id, handler,
                                                 filter);
    }

    // Request-Response Interface
    Result<void> register_service(
        std::string_view service_name,
        IRequestResponseService::RequestHandler handler);

    Result<QJsonObject> call_service(
        std::string_view service_name, const QJsonObject& request,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

    // Statistics Interface
    IStatistics::MessageStats get_message_stats() const;
    IStatistics::SubscriptionStats get_subscription_stats() const;

    // Lifecycle
    /**
     * @brief Gracefully shutdown the communication system
     *
     * Performs orderly shutdown of all components:
     * 1. Stops accepting new messages/requests
     * 2. Waits for pending operations to complete
     * 3. Cleans up resources in reverse dependency order
     *
     * Thread-safe: Can be called from any thread.
     * Idempotent: Multiple calls are safe.
     */
    void shutdown();

    /**
     * @brief Check if the system has been shut down
     * @return true if shutdown() has been called
     */
    bool is_shutdown() const noexcept;

private:
    std::unique_ptr<IMessagePublisher> publisher_;
    std::unique_ptr<ISubscriptionManager> subscription_manager_;
    std::unique_ptr<IMessageRouter> router_;
    std::unique_ptr<IStatistics> statistics_;
    std::unique_ptr<IEventSystem> event_system_;
    std::unique_ptr<IRequestResponseService> request_response_;

    mutable std::atomic<bool> shutdown_flag_{false};
};

/**
 * @brief Default factory implementation
 */
class DefaultCommunicationFactory : public ICommunicationFactory {
public:
    std::unique_ptr<IMessagePublisher> create_publisher(
        const CommunicationConfig::MessageBusConfig& config = {}) override;

    std::unique_ptr<ISubscriptionManager> create_subscription_manager(
        const CommunicationConfig::MessageBusConfig& config = {}) override;

    std::unique_ptr<IMessageRouter> create_router(
        std::shared_ptr<ISubscriptionManager> subscription_manager) override;

    std::unique_ptr<IStatistics> create_statistics() override;

    std::unique_ptr<IEventSystem> create_event_system(
        const CommunicationConfig::EventSystemConfig& config = {}) override;

    std::unique_ptr<IRequestResponseService> create_request_response_service(
        const CommunicationConfig::RequestResponseConfig& config = {}) override;
};

/**
 * @brief Convenience function to create a default communication system
 */
CommunicationSystem create_default_communication_system(
    const CommunicationConfig& config = {});

}  // namespace qtplugin::communication
