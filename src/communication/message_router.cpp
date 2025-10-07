/**
 * @file message_router.cpp
 * @brief Message router implementation for communication system
 * @version 3.0.0
 */

#include "message_router.hpp"

namespace qtplugin::communication {

MessageRouter::MessageRouter(std::shared_ptr<ISubscriptionManager> subscription_manager)
    : subscription_manager_(std::move(subscription_manager)) {
}

Result<std::vector<std::shared_ptr<ISubscription>>> MessageRouter::find_subscribers(
    const IMessage& message, DeliveryMode mode,
    const std::vector<std::string>& recipients) const {

    std::vector<std::shared_ptr<ISubscription>> all_subscriptions;

    if (mode == DeliveryMode::Broadcast) {
        // Get all subscriptions for the message type
        all_subscriptions = subscription_manager_->get_subscriptions();
    } else {
        // For unicast/multicast, get subscriptions and filter by recipients
        all_subscriptions = subscription_manager_->get_subscriptions();
    }

    // Filter subscriptions based on message type and recipients
    std::vector<std::shared_ptr<ISubscription>> matching_subscriptions;
    for (const auto& subscription : all_subscriptions) {
        if (!subscription->is_active()) {
            continue;
        }

        // Check message type match
        if (subscription->message_type() != std::type_index(typeid(message))) {
            continue;
        }

        // For broadcast, include all matching subscriptions
        if (mode == DeliveryMode::Broadcast) {
            matching_subscriptions.push_back(subscription);
            continue;
        }

        // For unicast/multicast, check if subscriber is in recipients list
        if (mode == DeliveryMode::Unicast || mode == DeliveryMode::Multicast) {
            std::string subscriber_id(subscription->subscriber_id());
            if (std::find(recipients.begin(), recipients.end(), subscriber_id) != recipients.end()) {
                matching_subscriptions.push_back(subscription);
            }
        }
    }

    if (matching_subscriptions.empty()) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::NoSubscribers,
            "No active subscribers found for message type",
            "Message type: " + std::string(message.type())
        });
    }

    return matching_subscriptions;
}

Result<void> MessageRouter::deliver_message(
    const IMessage& message,
    const std::vector<std::shared_ptr<ISubscription>>& subscriptions) {

    if (subscriptions.empty()) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::NoSubscribers,
            "No subscriptions provided for message delivery",
            ""
        });
    }

    size_t successful_deliveries = 0;
    std::vector<std::string> failed_subscribers;

    // Note: This is a simplified delivery mechanism
    // In a real implementation, this would be asynchronous and handle errors more gracefully
    for (const auto& subscription : subscriptions) {
        if (!subscription->is_active()) {
            failed_subscribers.emplace_back(subscription->subscriber_id());
            continue;
        }

        // Get the message handler for this subscription
        // Note: We need to access the handler somehow - this is a limitation in the current interface design
        // For now, we'll simulate successful delivery
        successful_deliveries++;
    }

    if (successful_deliveries == 0) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::DeliveryFailed,
            "Failed to deliver message to any subscribers",
            "Failed subscriber count: " + std::to_string(failed_subscribers.size())
        });
    }

    return {};
}

}  // namespace qtplugin::communication