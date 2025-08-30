/**
 * @file performance_metrics_message.hpp
 * @brief Performance monitoring message implementation
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <chrono>
#include <string>
#include <string_view>
#include <unordered_map>

#include "qtplugin/communication/message_bus.hpp"

namespace qtplugin::examples {

/**
 * @brief Performance monitoring message
 */
class PerformanceMetricsMessage : public qtplugin::IMessage {
public:
    PerformanceMetricsMessage();
    ~PerformanceMetricsMessage() = default;

    // IMessage interface
    std::string_view type() const noexcept override { return "performance_metrics"; }
    std::string_view sender() const noexcept override { return m_sender; }
    std::chrono::system_clock::time_point timestamp() const noexcept override { return m_timestamp; }
    qtplugin::MessagePriority priority() const noexcept override { return qtplugin::MessagePriority::Normal; }
    QJsonObject to_json() const override;
    std::string id() const noexcept override { return m_id; }

    // Metrics data
    void set_cpu_usage(double cpu_percent) { m_cpu_usage = cpu_percent; }
    void set_memory_usage(size_t memory_bytes) { m_memory_usage = memory_bytes; }
    void set_message_throughput(size_t messages_per_second) { m_message_throughput = messages_per_second; }
    void set_sender(const std::string& sender) { m_sender = sender; }

    double cpu_usage() const { return m_cpu_usage; }
    size_t memory_usage() const { return m_memory_usage; }
    size_t message_throughput() const { return m_message_throughput; }

private:
    std::string m_sender;
    std::string m_id;
    double m_cpu_usage = 0.0;
    size_t m_memory_usage = 0;
    size_t m_message_throughput = 0;
    std::chrono::system_clock::time_point m_timestamp;
};

} // namespace qtplugin::examples
