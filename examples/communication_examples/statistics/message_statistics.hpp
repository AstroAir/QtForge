/**
 * @file message_statistics.hpp
 * @brief Message statistics collection and reporting
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace qtplugin::examples {

/**
 * @brief Message statistics collector
 */
class MessageStatisticsCollector {
public:
    MessageStatisticsCollector();

    void record_message_sent(const std::string& message_type);
    void record_message_received(const std::string& message_type);
    void record_message_failed(const std::string& message_type, const std::string& error);
    void record_latency(std::chrono::milliseconds latency);

    QJsonObject get_statistics() const;
    void reset_statistics();

    // Additional statistics methods
    size_t total_sent() const;
    size_t total_received() const;
    size_t total_failed() const;
    double average_latency_ms() const;
    std::chrono::milliseconds min_latency() const;
    std::chrono::milliseconds max_latency() const;

private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, size_t> m_sent_counts;
    std::unordered_map<std::string, size_t> m_received_counts;
    std::unordered_map<std::string, size_t> m_failed_counts;
    std::vector<std::chrono::milliseconds> m_latencies;
    std::chrono::steady_clock::time_point m_start_time;
};

/**
 * @brief Real-time performance monitor
 */
class PerformanceMonitor {
public:
    PerformanceMonitor();

    void start_monitoring();
    void stop_monitoring();
    void record_operation(const std::string& operation_name, std::chrono::milliseconds duration);

    QJsonObject get_performance_report() const;
    bool is_monitoring() const { return m_monitoring; }

private:
    mutable std::mutex m_mutex;
    bool m_monitoring = false;
    std::chrono::steady_clock::time_point m_start_time;
    std::unordered_map<std::string, std::vector<std::chrono::milliseconds>> m_operation_times;
};

} // namespace qtplugin::examples
