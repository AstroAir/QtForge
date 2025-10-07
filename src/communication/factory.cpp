/**
 * @file factory.cpp
 * @brief Implementation of communication factory and system
 * @version 3.0.0
 */

#include <qtplugin/communication/factory.hpp>
#include "message_publisher.hpp"
#include "subscription_manager.hpp"
#include "message_router.hpp"
#include "statistics_collector.hpp"
#include "event_system_impl.hpp"
#include "request_response_service_impl.hpp"

namespace qtplugin::communication {

// CommunicationSystemBuilder Implementation

CommunicationSystemBuilder::CommunicationSystemBuilder(std::shared_ptr<ICommunicationFactory> factory)
    : factory_(std::move(factory)) {}

CommunicationSystemBuilder& CommunicationSystemBuilder::with_config(const CommunicationConfig& config) {
    config_ = config;
    return *this;
}

CommunicationSystemBuilder& CommunicationSystemBuilder::with_publisher(std::unique_ptr<IMessagePublisher> publisher) {
    publisher_ = std::move(publisher);
    return *this;
}

CommunicationSystemBuilder& CommunicationSystemBuilder::with_subscription_manager(std::unique_ptr<ISubscriptionManager> manager) {
    subscription_manager_ = std::move(manager);
    return *this;
}

CommunicationSystemBuilder& CommunicationSystemBuilder::with_router(std::unique_ptr<IMessageRouter> router) {
    router_ = std::move(router);
    return *this;
}

CommunicationSystemBuilder& CommunicationSystemBuilder::with_statistics(std::unique_ptr<IStatistics> statistics) {
    statistics_ = std::move(statistics);
    return *this;
}

CommunicationSystemBuilder& CommunicationSystemBuilder::with_event_system(std::unique_ptr<IEventSystem> event_system) {
    event_system_ = std::move(event_system);
    return *this;
}

CommunicationSystemBuilder& CommunicationSystemBuilder::with_request_response(std::unique_ptr<IRequestResponseService> service) {
    request_response_ = std::move(service);
    return *this;
}

CommunicationSystem CommunicationSystemBuilder::build() {
    // Create components if not provided
    if (!publisher_) {
        publisher_ = factory_->create_publisher(config_.message_bus);
    }
    
    if (!subscription_manager_) {
        subscription_manager_ = factory_->create_subscription_manager(config_.message_bus);
    }
    
    if (!router_) {
        // Convert unique_ptr to shared_ptr for router dependency
        auto shared_manager = std::shared_ptr<ISubscriptionManager>(subscription_manager_.release());
        router_ = factory_->create_router(shared_manager);
        // Note: We need to handle the shared ownership properly in the actual system
    }
    
    if (!statistics_) {
        statistics_ = factory_->create_statistics();
    }
    
    if (!event_system_) {
        event_system_ = factory_->create_event_system(config_.event_system);
    }
    
    if (!request_response_) {
        request_response_ = factory_->create_request_response_service(config_.request_response);
    }
    
    return CommunicationSystem(
        std::move(publisher_),
        std::move(subscription_manager_),
        std::move(router_),
        std::move(statistics_),
        std::move(event_system_),
        std::move(request_response_)
    );
}

// CommunicationSystem Implementation

CommunicationSystem::CommunicationSystem(
    std::unique_ptr<IMessagePublisher> publisher,
    std::unique_ptr<ISubscriptionManager> subscription_manager,
    std::unique_ptr<IMessageRouter> router,
    std::unique_ptr<IStatistics> statistics,
    std::unique_ptr<IEventSystem> event_system,
    std::unique_ptr<IRequestResponseService> request_response
) : publisher_(std::move(publisher)),
    subscription_manager_(std::move(subscription_manager)),
    router_(std::move(router)),
    statistics_(std::move(statistics)),
    event_system_(std::move(event_system)),
    request_response_(std::move(request_response)) {}

Result<void> CommunicationSystem::publish_message(
    std::shared_ptr<IMessage> message,
    DeliveryMode mode,
    const std::vector<std::string>& recipients
) {
    return publisher_->publish(std::move(message), mode, recipients);
}

Result<std::shared_ptr<ISubscription>> CommunicationSystem::subscribe_to_messages(
    std::string_view subscriber_id,
    std::type_index message_type,
    ISubscriptionManager::MessageHandler handler,
    ISubscriptionManager::MessageFilter filter
) {
    return subscription_manager_->subscribe(subscriber_id, message_type, std::move(handler), std::move(filter));
}

Result<void> CommunicationSystem::register_service(
    std::string_view service_name,
    IRequestResponseService::RequestHandler handler
) {
    return request_response_->register_service(service_name, std::move(handler));
}

Result<QJsonObject> CommunicationSystem::call_service(
    std::string_view service_name,
    const QJsonObject& request,
    std::chrono::milliseconds timeout
) {
    return request_response_->call_service(service_name, request, timeout);
}

IStatistics::MessageStats CommunicationSystem::get_message_stats() const {
    return statistics_->get_message_stats();
}

IStatistics::SubscriptionStats CommunicationSystem::get_subscription_stats() const {
    return statistics_->get_subscription_stats();
}

void CommunicationSystem::shutdown() {
    // Graceful shutdown of all components
    // Order matters: stop accepting new work first, then clean up
    
    // 1. Stop accepting new requests/messages
    // 2. Wait for pending operations to complete
    // 3. Clean up resources
    
    // Note: Actual implementation would need proper shutdown coordination
}

// DefaultCommunicationFactory Implementation

std::unique_ptr<IMessagePublisher> DefaultCommunicationFactory::create_publisher(
    const CommunicationConfig::MessageBusConfig& config
) {
    return std::make_unique<MessagePublisher>(config);
}

std::unique_ptr<ISubscriptionManager> DefaultCommunicationFactory::create_subscription_manager(
    const CommunicationConfig::MessageBusConfig& config
) {
    return std::make_unique<SubscriptionManager>(config);
}

std::unique_ptr<IMessageRouter> DefaultCommunicationFactory::create_router(
    std::shared_ptr<ISubscriptionManager> subscription_manager
) {
    return std::make_unique<MessageRouter>(std::move(subscription_manager));
}

std::unique_ptr<IStatistics> DefaultCommunicationFactory::create_statistics() {
    return std::make_unique<StatisticsCollector>();
}

std::unique_ptr<IEventSystem> DefaultCommunicationFactory::create_event_system(
    const CommunicationConfig::EventSystemConfig& config
) {
    return std::make_unique<EventSystemImpl>(config);
}

std::unique_ptr<IRequestResponseService> DefaultCommunicationFactory::create_request_response_service(
    const CommunicationConfig::RequestResponseConfig& config
) {
    return std::make_unique<RequestResponseServiceImpl>(config);
}

// Convenience function

CommunicationSystem create_default_communication_system(const CommunicationConfig& config) {
    auto factory = std::make_shared<DefaultCommunicationFactory>();
    return CommunicationSystemBuilder(factory)
        .with_config(config)
        .build();
}

} // namespace qtplugin::communication
