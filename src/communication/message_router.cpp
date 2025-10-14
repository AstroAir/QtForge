/**
 * @file message_router.cpp
 * @brief Message router implementation for communication system
 * @version 3.0.0
 */

#include "message_router.hpp"

namespace qtplugin::communication {

MessageRouter::MessageRouter(
    std::shared_ptr<ISubscriptionManager> subscription_manager)
    : subscription_manager_(std::move(subscription_manager)) {}

Result<std::vector<std::shared_ptr<ISubscription>>>
MessageRouter::find_subscribers(
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
            if (std::find(recipients.begin(), recipients.end(),
                          subscriber_id) != recipients.end()) {
                matching_subscriptions.push_back(subscription);
            }
        }
    }

    if (matching_subscriptions.empty()) {
        return qtplugin::unexpected(
            CommunicationError{CommunicationError::Type::NoSubscribers,
                               "No active subscribers found for message type",
                               "Message type: " + std::string(message.type())});
    }

    return matching_subscriptions;
}

Result<void> MessageRouter::deliver_message(
    const IMessage& message,
    const std::vector<std::shared_ptr<ISubscription>>& subscriptions) {
    if (subscriptions.empty()) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::NoSubscribers,
            "No subscriptions provided for message delivery", ""});
    }

    size_t successful_deliveries = 0;
    size_t filtered_deliveries = 0;
    std::vector<std::string> failed_subscribers;
    std::string last_error_message;

    // Deliver message to each subscription
    // Note: This is synchronous delivery. For async delivery with >5
    // subscribers, the MessageBus uses a thread pool (see message_bus.cpp)
    for (const auto& subscription : subscriptions) {
        if (!subscription) {
            continue;  // Skip null subscriptions
        }

        if (!subscription->is_active()) {
            failed_subscribers.emplace_back(subscription->subscriber_id());
            continue;
        }

        // Use the new deliver() method from ISubscription interface
        auto result = subscription->deliver(message);
        if (result.has_value()) {
            successful_deliveries++;
        } else {
            // Check if it was filtered (not an error) or actual failure
            const auto& error = result.error();
            if (error.type == CommunicationError::Type::DeliveryFailed) {
                failed_subscribers.emplace_back(subscription->subscriber_id());
                last_error_message = error.message;
            } else {
                // Message was filtered or other non-critical issue
                filtered_deliveries++;
            }
        }
    }

    // Consider filtered messages as successful (they were processed, just
    // skipped)
    const size_t total_processed = successful_deliveries + filtered_deliveries;

    if (total_processed == 0 && !failed_subscribers.empty()) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::DeliveryFailed,
            "Failed to deliver message to any subscribers",
            "Failed subscriber count: " +
                std::to_string(failed_subscribers.size()) +
                (last_error_message.empty()
                     ? ""
                     : ", Last error: " + last_error_message)});
    }

    // Partial success is still success - at least some subscribers got the
    // message
    return {};
}

}  // namespace qtplugin::communication
