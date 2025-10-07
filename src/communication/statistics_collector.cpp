/**
 * @file statistics_collector.cpp
 * @brief Statistics collector implementation for communication system
 * @version 3.0.0
 */

#include "statistics_collector.hpp"

namespace qtplugin::communication {

IStatistics::MessageStats StatisticsCollector::get_message_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return message_stats_;
}

IStatistics::SubscriptionStats StatisticsCollector::get_subscription_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return subscription_stats_;
}

void StatisticsCollector::reset_stats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    message_stats_ = MessageStats{};
    subscription_stats_ = SubscriptionStats{};
    subscriptions_by_subscriber_.clear();
}

void StatisticsCollector::increment_published() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    message_stats_.total_published++;
    message_stats_.last_activity = std::chrono::system_clock::now();
}

void StatisticsCollector::increment_delivered() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    message_stats_.total_delivered++;
    message_stats_.last_activity = std::chrono::system_clock::now();
}

void StatisticsCollector::increment_failed() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    message_stats_.total_failed++;
    message_stats_.last_activity = std::chrono::system_clock::now();
}

void StatisticsCollector::update_delivery_time(std::chrono::milliseconds delivery_time) {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    // Simple moving average calculation
    auto total_time = message_stats_.avg_delivery_time * (message_stats_.total_delivered - 1);
    message_stats_.avg_delivery_time =
        std::chrono::milliseconds((total_time + delivery_time).count() / message_stats_.total_delivered);
}

void StatisticsCollector::update_activity_time() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    message_stats_.last_activity = std::chrono::system_clock::now();
}

void StatisticsCollector::add_subscription(const std::string& subscriber_id, std::type_index message_type) {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    subscription_stats_.active_subscriptions++;
    subscription_stats_.total_subscriptions++;

    subscriptions_by_subscriber_[subscriber_id].push_back(message_type);

    // Update subscribers_by_type count
    std::string type_name = message_type.name();
    subscription_stats_.subscribers_by_type[type_name]++;
}

void StatisticsCollector::remove_subscription(const std::string& subscription_id) {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    if (subscription_stats_.active_subscriptions > 0) {
        subscription_stats_.active_subscriptions--;
    }

    // Note: Simplified removal - in practice we'd need more sophisticated tracking
    // to properly remove from subscribers_by_type and subscriptions_by_subscriber_
}

}  // namespace qtplugin::communication