/**
 * @file message_statistics.cpp
 * @brief Message statistics collection and reporting implementation
 * @version 3.0.0
 */

#include "message_statistics.hpp"
#include <QJsonDocument>
#include <algorithm>
#include <numeric>

namespace qtplugin::examples {

// MessageStatisticsCollector Implementation
MessageStatisticsCollector::MessageStatisticsCollector()
    : m_start_time(std::chrono::steady_clock::now())
{
}

void MessageStatisticsCollector::record_message_sent(const std::string& message_type) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sent_counts[message_type]++;
}

void MessageStatisticsCollector::record_message_received(const std::string& message_type) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_received_counts[message_type]++;
}

void MessageStatisticsCollector::record_message_failed(const std::string& message_type, const std::string& error) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_failed_counts[message_type]++;
}

void MessageStatisticsCollector::record_latency(std::chrono::milliseconds latency) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_latencies.push_back(latency);
}

QJsonObject MessageStatisticsCollector::get_statistics() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    QJsonObject stats;

    // Message counts
    QJsonObject sent_counts;
    for (const auto& [type, count] : m_sent_counts) {
        sent_counts[QString::fromStdString(type)] = static_cast<qint64>(count);
    }
    stats["sent_counts"] = sent_counts;

    QJsonObject received_counts;
    for (const auto& [type, count] : m_received_counts) {
        received_counts[QString::fromStdString(type)] = static_cast<qint64>(count);
    }
    stats["received_counts"] = received_counts;

    QJsonObject failed_counts;
    for (const auto& [type, count] : m_failed_counts) {
        failed_counts[QString::fromStdString(type)] = static_cast<qint64>(count);
    }
    stats["failed_counts"] = failed_counts;

    // Latency statistics
    if (!m_latencies.empty()) {
        auto total_latency = std::accumulate(m_latencies.begin(), m_latencies.end(),
                                           std::chrono::milliseconds(0));
        auto avg_latency = total_latency / m_latencies.size();

        auto min_latency = *std::min_element(m_latencies.begin(), m_latencies.end());
        auto max_latency = *std::max_element(m_latencies.begin(), m_latencies.end());

        QJsonObject latency_stats;
        latency_stats["average_ms"] = avg_latency.count();
        latency_stats["min_ms"] = min_latency.count();
        latency_stats["max_ms"] = max_latency.count();
        latency_stats["sample_count"] = static_cast<qint64>(m_latencies.size());

        stats["latency"] = latency_stats;
    }

    // Runtime
    auto runtime = std::chrono::steady_clock::now() - m_start_time;
    stats["runtime_seconds"] = std::chrono::duration_cast<std::chrono::seconds>(runtime).count();

    return stats;
}

void MessageStatisticsCollector::reset_statistics() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sent_counts.clear();
    m_received_counts.clear();
    m_failed_counts.clear();
    m_latencies.clear();
    m_start_time = std::chrono::steady_clock::now();
}

size_t MessageStatisticsCollector::total_sent() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t total = 0;
    for (const auto& [type, count] : m_sent_counts) {
        total += count;
    }
    return total;
}

size_t MessageStatisticsCollector::total_received() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t total = 0;
    for (const auto& [type, count] : m_received_counts) {
        total += count;
    }
    return total;
}

size_t MessageStatisticsCollector::total_failed() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t total = 0;
    for (const auto& [type, count] : m_failed_counts) {
        total += count;
    }
    return total;
}

double MessageStatisticsCollector::average_latency_ms() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_latencies.empty()) return 0.0;
    
    auto total = std::accumulate(m_latencies.begin(), m_latencies.end(),
                               std::chrono::milliseconds(0));
    return static_cast<double>(total.count()) / m_latencies.size();
}

std::chrono::milliseconds MessageStatisticsCollector::min_latency() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_latencies.empty()) return std::chrono::milliseconds(0);
    return *std::min_element(m_latencies.begin(), m_latencies.end());
}

std::chrono::milliseconds MessageStatisticsCollector::max_latency() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_latencies.empty()) return std::chrono::milliseconds(0);
    return *std::max_element(m_latencies.begin(), m_latencies.end());
}

// PerformanceMonitor Implementation
PerformanceMonitor::PerformanceMonitor() = default;

void PerformanceMonitor::start_monitoring() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_monitoring = true;
    m_start_time = std::chrono::steady_clock::now();
}

void PerformanceMonitor::stop_monitoring() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_monitoring = false;
}

void PerformanceMonitor::record_operation(const std::string& operation_name, std::chrono::milliseconds duration) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_monitoring) {
        m_operation_times[operation_name].push_back(duration);
    }
}

QJsonObject PerformanceMonitor::get_performance_report() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    QJsonObject report;
    report["monitoring"] = m_monitoring;
    
    if (m_monitoring) {
        auto runtime = std::chrono::steady_clock::now() - m_start_time;
        report["runtime_seconds"] = std::chrono::duration_cast<std::chrono::seconds>(runtime).count();
    }
    
    QJsonObject operations;
    for (const auto& [op_name, times] : m_operation_times) {
        if (!times.empty()) {
            auto total = std::accumulate(times.begin(), times.end(), std::chrono::milliseconds(0));
            auto avg = total / times.size();
            auto min_time = *std::min_element(times.begin(), times.end());
            auto max_time = *std::max_element(times.begin(), times.end());
            
            QJsonObject op_stats;
            op_stats["count"] = static_cast<qint64>(times.size());
            op_stats["average_ms"] = avg.count();
            op_stats["min_ms"] = min_time.count();
            op_stats["max_ms"] = max_time.count();
            op_stats["total_ms"] = total.count();
            
            operations[QString::fromStdString(op_name)] = op_stats;
        }
    }
    report["operations"] = operations;
    
    return report;
}

} // namespace qtplugin::examples
