/**
 * @file performance_metrics_message.hpp
 * @brief Performance monitoring message implementation
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <chrono>
#include <string>

#include "qtplugin/communication/message_types.hpp"

namespace qtplugin::examples {

/**
 * @brief Performance monitoring message
 */
class PerformanceMetricsMessage : public IMessage {
public:
    PerformanceMetricsMessage();
    ~PerformanceMetricsMessage() override = default;

    // IMessage interface
    std::string type() const override { return "performance_metrics"; }
    std::string sender() const override { return m_sender; }
    std::string topic() const override { return "system.metrics"; }
    QJsonObject to_json() const override;
    void from_json(const QJsonObject& json) override;
    std::chrono::system_clock::time_point timestamp() const override { return m_timestamp; }

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
    double m_cpu_usage = 0.0;
    size_t m_memory_usage = 0;
    size_t m_message_throughput = 0;
    std::chrono::system_clock::time_point m_timestamp;
};

} // namespace qtplugin::examples
