/**
 * @file message_utils.cpp
 * @brief Utility functions for message bus examples implementation
 * @version 3.0.0
 */

#include "message_utils.hpp"
#include <QJsonDocument>
#include <QJsonArray>
#include <random>
#include <sstream>
#include <iomanip>

namespace qtplugin::examples::utils {

std::shared_ptr<SystemEventMessage> create_test_system_event(
    SystemEventMessage::EventType event_type,
    const std::string& sender,
    const QJsonObject& data) {

    auto message = std::make_shared<SystemEventMessage>(event_type);
    message->set_sender(sender);
    message->set_topic("system.events");
    message->set_data(data);
    return message;
}

std::shared_ptr<PerformanceMetricsMessage> create_test_performance_metrics(
    const std::string& sender,
    double cpu_usage,
    size_t memory_usage,
    size_t message_throughput) {

    auto message = std::make_shared<PerformanceMetricsMessage>();
    message->set_sender(sender);
    message->set_cpu_usage(cpu_usage);
    message->set_memory_usage(memory_usage);
    message->set_message_throughput(message_throughput);
    return message;
}

std::string format_statistics(const QJsonObject& stats) {
    QJsonDocument doc(stats);
    return doc.toJson(QJsonDocument::Indented).toStdString();
}

QJsonObject generate_random_test_data() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> int_dist(1, 1000);
    static std::uniform_real_distribution<> real_dist(0.0, 100.0);

    QJsonObject data;
    data["random_id"] = int_dist(gen);
    data["random_value"] = real_dist(gen);
    data["timestamp"] = QString::number(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    
    QJsonArray test_array;
    for (int i = 0; i < 3; ++i) {
        test_array.append(int_dist(gen));
    }
    data["test_array"] = test_array;
    
    return data;
}

bool validate_message_content(const QJsonObject& message_json) {
    // Basic validation checks
    if (!message_json.contains("type") || !message_json.contains("sender")) {
        return false;
    }
    
    QString type = message_json["type"].toString();
    if (type.isEmpty()) {
        return false;
    }
    
    QString sender = message_json["sender"].toString();
    if (sender.isEmpty()) {
        return false;
    }
    
    // Type-specific validation
    if (type == "system_event") {
        return message_json.contains("event_type") && message_json.contains("priority");
    } else if (type == "performance_metrics") {
        return message_json.contains("cpu_usage") && 
               message_json.contains("memory_usage") &&
               message_json.contains("message_throughput");
    }
    
    return true;
}

// MessageLatencyMeasurer Implementation
void MessageLatencyMeasurer::start_measurement(const std::string& message_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_start_times[message_id] = std::chrono::steady_clock::now();
}

std::chrono::milliseconds MessageLatencyMeasurer::end_measurement(const std::string& message_id) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_start_times.find(message_id);
    if (it != m_start_times.end()) {
        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - it->second);
        m_start_times.erase(it);
        return latency;
    }

    return std::chrono::milliseconds(0);
}

void MessageLatencyMeasurer::clear_measurements() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_start_times.clear();
}

size_t MessageLatencyMeasurer::active_measurements() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_start_times.size();
}

std::vector<std::string> MessageLatencyMeasurer::get_active_message_ids() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> ids;
    ids.reserve(m_start_times.size());
    
    for (const auto& [id, time] : m_start_times) {
        ids.push_back(id);
    }
    
    return ids;
}

// MessageBatchProcessor Implementation
MessageBatchProcessor::MessageBatchProcessor(size_t batch_size)
    : m_batch_size(batch_size)
{
}

void MessageBatchProcessor::add_message(std::shared_ptr<SystemEventMessage> message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_system_events.push_back(message);
}

void MessageBatchProcessor::add_message(std::shared_ptr<PerformanceMetricsMessage> message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_metrics.push_back(message);
}

std::vector<std::shared_ptr<SystemEventMessage>> MessageBatchProcessor::get_system_event_batch() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_system_events.size() >= m_batch_size) {
        std::vector<std::shared_ptr<SystemEventMessage>> batch;
        batch.reserve(m_batch_size);
        
        auto end_it = m_system_events.begin() + m_batch_size;
        batch.assign(m_system_events.begin(), end_it);
        m_system_events.erase(m_system_events.begin(), end_it);
        
        return batch;
    }
    
    return {};
}

std::vector<std::shared_ptr<PerformanceMetricsMessage>> MessageBatchProcessor::get_metrics_batch() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_metrics.size() >= m_batch_size) {
        std::vector<std::shared_ptr<PerformanceMetricsMessage>> batch;
        batch.reserve(m_batch_size);
        
        auto end_it = m_metrics.begin() + m_batch_size;
        batch.assign(m_metrics.begin(), end_it);
        m_metrics.erase(m_metrics.begin(), end_it);
        
        return batch;
    }
    
    return {};
}

void MessageBatchProcessor::clear_batches() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_system_events.clear();
    m_metrics.clear();
}

size_t MessageBatchProcessor::system_event_count() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_system_events.size();
}

size_t MessageBatchProcessor::metrics_count() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_metrics.size();
}

// MessageContentAnalyzer Implementation
MessageContentAnalyzer::AnalysisResult MessageContentAnalyzer::analyze_message_batch(
    const std::vector<QJsonObject>& messages) {
    
    auto start_time = std::chrono::steady_clock::now();
    
    AnalysisResult result;
    result.total_messages = messages.size();
    
    size_t total_size = 0;
    
    for (const auto& message : messages) {
        QString type = message["type"].toString();
        
        if (type == "system_event") {
            result.system_events++;
            
            int priority = message["priority"].toInt();
            if (priority >= static_cast<int>(SystemEventMessage::Priority::High)) {
                result.high_priority_messages++;
            }
            
            int event_type = message["event_type"].toInt();
            if (event_type == static_cast<int>(SystemEventMessage::EventType::ErrorOccurred)) {
                result.error_messages++;
            }
        } else if (type == "performance_metrics") {
            result.performance_metrics++;
        }
        
        total_size += calculate_message_size(message);
    }
    
    if (result.total_messages > 0) {
        result.average_message_size_bytes = static_cast<double>(total_size) / result.total_messages;
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.analysis_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    
    return result;
}

void MessageContentAnalyzer::reset_analysis() {
    // Nothing to reset in this simple implementation
}

size_t MessageContentAnalyzer::calculate_message_size(const QJsonObject& message) const {
    QJsonDocument doc(message);
    return static_cast<size_t>(doc.toJson(QJsonDocument::Compact).size());
}

} // namespace qtplugin::examples::utils
