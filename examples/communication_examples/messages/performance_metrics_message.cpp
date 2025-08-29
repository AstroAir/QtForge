/**
 * @file performance_metrics_message.cpp
 * @brief Performance monitoring message implementation
 * @version 3.0.0
 */

#include "performance_metrics_message.hpp"
#include <QJsonDocument>

namespace qtplugin::examples {

PerformanceMetricsMessage::PerformanceMetricsMessage()
    : m_timestamp(std::chrono::system_clock::now())
{
}

QJsonObject PerformanceMetricsMessage::to_json() const {
    QJsonObject json;
    json["type"] = QString::fromStdString(type());
    json["sender"] = QString::fromStdString(m_sender);
    json["topic"] = QString::fromStdString(topic());
    json["cpu_usage"] = m_cpu_usage;
    json["memory_usage"] = static_cast<qint64>(m_memory_usage);
    json["message_throughput"] = static_cast<qint64>(m_message_throughput);
    json["timestamp"] = QString::number(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            m_timestamp.time_since_epoch()).count());
    return json;
}

void PerformanceMetricsMessage::from_json(const QJsonObject& json) {
    m_sender = json["sender"].toString().toStdString();
    m_cpu_usage = json["cpu_usage"].toDouble();
    m_memory_usage = static_cast<size_t>(json["memory_usage"].toInteger());
    m_message_throughput = static_cast<size_t>(json["message_throughput"].toInteger());

    auto timestamp_ms = json["timestamp"].toString().toLongLong();
    m_timestamp = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(timestamp_ms));
}

} // namespace qtplugin::examples
