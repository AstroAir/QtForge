/**
 * @file message_utils.hpp
 * @brief Utility functions for message bus examples
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <chrono>
#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>

#include "../messages/system_event_message.hpp"
#include "../messages/performance_metrics_message.hpp"

namespace qtplugin::examples::utils {

/**
 * @brief Create a test system event message
 */
std::shared_ptr<SystemEventMessage> create_test_system_event(
    SystemEventMessage::EventType event_type,
    const std::string& sender = "test_sender",
    const QJsonObject& data = {});

/**
 * @brief Create a test performance metrics message
 */
std::shared_ptr<PerformanceMetricsMessage> create_test_performance_metrics(
    const std::string& sender = "test_sender",
    double cpu_usage = 50.0,
    size_t memory_usage = 1024 * 1024 * 100, // 100MB
    size_t message_throughput = 1000);

/**
 * @brief Format message statistics for display
 */
std::string format_statistics(const QJsonObject& stats);

/**
 * @brief Generate random test data for messages
 */
QJsonObject generate_random_test_data();

/**
 * @brief Validate message content
 */
bool validate_message_content(const QJsonObject& message_json);

/**
 * @brief Measure message round-trip time
 */
class MessageLatencyMeasurer {
public:
    void start_measurement(const std::string& message_id);
    std::chrono::milliseconds end_measurement(const std::string& message_id);
    void clear_measurements();
    
    size_t active_measurements() const;
    std::vector<std::string> get_active_message_ids() const;

private:
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> m_start_times;
    mutable std::mutex m_mutex;
};

/**
 * @brief Message batch processor for efficient bulk operations
 */
class MessageBatchProcessor {
public:
    MessageBatchProcessor(size_t batch_size = 100);
    
    void add_message(std::shared_ptr<SystemEventMessage> message);
    void add_message(std::shared_ptr<PerformanceMetricsMessage> message);
    
    std::vector<std::shared_ptr<SystemEventMessage>> get_system_event_batch();
    std::vector<std::shared_ptr<PerformanceMetricsMessage>> get_metrics_batch();
    
    void clear_batches();
    size_t system_event_count() const;
    size_t metrics_count() const;

private:
    size_t m_batch_size;
    std::vector<std::shared_ptr<SystemEventMessage>> m_system_events;
    std::vector<std::shared_ptr<PerformanceMetricsMessage>> m_metrics;
    mutable std::mutex m_mutex;
};

/**
 * @brief Message content analyzer
 */
class MessageContentAnalyzer {
public:
    struct AnalysisResult {
        size_t total_messages = 0;
        size_t system_events = 0;
        size_t performance_metrics = 0;
        size_t high_priority_messages = 0;
        size_t error_messages = 0;
        double average_message_size_bytes = 0.0;
        std::chrono::milliseconds analysis_duration{0};
    };
    
    AnalysisResult analyze_message_batch(const std::vector<QJsonObject>& messages);
    void reset_analysis();
    
private:
    size_t calculate_message_size(const QJsonObject& message) const;
};

} // namespace qtplugin::examples::utils
