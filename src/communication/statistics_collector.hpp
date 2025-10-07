/**
 * @file statistics_collector.hpp
 * @brief Statistics collector implementation for communication system
 * @version 3.0.0
 */

#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <qtplugin/communication/interfaces.hpp>

namespace qtplugin::communication {

/**
 * @brief Default implementation of IStatistics
 */
class StatisticsCollector : public IStatistics {
public:
    StatisticsCollector() = default;
    ~StatisticsCollector() override = default;

    MessageStats get_message_stats() const override;
    SubscriptionStats get_subscription_stats() const override;
    void reset_stats() override;

    // Helper methods to update statistics
    void increment_published();
    void increment_delivered();
    void increment_failed();
    void update_delivery_time(std::chrono::milliseconds delivery_time);
    void update_activity_time();
    void add_subscription(const std::string& subscriber_id, std::type_index message_type);
    void remove_subscription(const std::string& subscription_id);

private:
    mutable std::mutex stats_mutex_;
    MessageStats message_stats_;
    SubscriptionStats subscription_stats_;
    std::map<std::string, std::vector<std::type_index>> subscriptions_by_subscriber_;
};

}  // namespace qtplugin::communication