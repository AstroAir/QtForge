/**
 * @file message_bus_example.hpp
 * @brief Advanced MessageBus example demonstrating sophisticated patterns
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "../messages/performance_metrics_message.hpp"
#include "../messages/system_event_message.hpp"
#include "../statistics/message_statistics.hpp"
#include "../utils/message_utils.hpp"
#include "qtplugin/communication/message_bus.hpp"

namespace qtplugin::examples {

/**
 * @brief Advanced MessageBus example demonstrating sophisticated patterns
 */
class AdvancedMessageBusExample : public QObject {
    Q_OBJECT

public:
    explicit AdvancedMessageBusExample(QObject* parent = nullptr);
    ~AdvancedMessageBusExample() override;

    /**
     * @brief Run the complete example
     * @return Exit code (0 = success, 1 = failure)
     */
    int run_example();

    /**
     * @brief Get current statistics
     */
    QJsonObject get_statistics() const;

    /**
     * @brief Reset all counters and statistics
     */
    void reset_statistics();

private slots:
    void on_system_event_received(std::shared_ptr<SystemEventMessage> message);
    void on_performance_metrics_received(
        std::shared_ptr<PerformanceMetricsMessage> message);
    void on_monitoring_timer();

private:
    // Demonstration methods
    void demonstrate_basic_messaging();
    void demonstrate_priority_messaging();
    void demonstrate_filtered_subscriptions();
    void demonstrate_message_statistics();
    void demonstrate_error_handling();
    void demonstrate_performance_monitoring();
    void demonstrate_subscription_management();
    void demonstrate_message_batching();

    // Setup and cleanup
    void setup_subscriptions();
    void cleanup_subscriptions();
    void start_performance_monitoring();
    void stop_performance_monitoring();

    // Helper methods
    void publish_test_messages();
    void log_message_activity(const QString& activity);

    // Message bus and components
    std::unique_ptr<IMessageBus> m_message_bus;

    // Statistics and monitoring
    std::unique_ptr<MessageStatisticsCollector> m_statistics_collector;
    std::unique_ptr<utils::MessageLatencyMeasurer> m_latency_measurer;
    std::unique_ptr<utils::MessageBatchProcessor> m_batch_processor;

    // Monitoring timer
    QTimer* m_monitoring_timer;

    // Counters
    std::atomic<size_t> m_messages_sent{0};
    std::atomic<size_t> m_messages_received{0};
    std::atomic<size_t> m_messages_failed{0};

    // Subscription tracking
    std::vector<std::string> m_subscription_ids;
    mutable std::mutex m_stats_mutex;

    // Performance metrics
    std::chrono::steady_clock::time_point m_start_time;
    std::unordered_map<std::string, size_t> m_message_type_counts;
    std::vector<std::chrono::milliseconds> m_message_latencies;

    // Configuration
    bool m_verbose_logging = true;
    size_t m_batch_size = 10;
    std::chrono::milliseconds m_monitoring_interval{1000};
};

}  // namespace qtplugin::examples
