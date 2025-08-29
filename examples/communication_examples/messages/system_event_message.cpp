/**
 * @file system_event_message.cpp
 * @brief System event message implementation
 * @version 3.0.0
 */

#include "system_event_message.hpp"
#include <QJsonDocument>

namespace qtplugin::examples {

SystemEventMessage::SystemEventMessage(EventType event_type, Priority priority)
    : m_event_type(event_type)
    , m_priority(priority)
    , m_timestamp(std::chrono::system_clock::now())
{
}

QJsonObject SystemEventMessage::to_json() const {
    QJsonObject json;
    json["type"] = QString::fromStdString(type());
    json["sender"] = QString::fromStdString(m_sender);
    json["topic"] = QString::fromStdString(m_topic);
    json["event_type"] = static_cast<int>(m_event_type);
    json["priority"] = static_cast<int>(m_priority);
    json["timestamp"] = QString::number(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            m_timestamp.time_since_epoch()).count());
    json["data"] = m_data;
    return json;
}

void SystemEventMessage::from_json(const QJsonObject& json) {
    m_sender = json["sender"].toString().toStdString();
    m_topic = json["topic"].toString().toStdString();
    m_event_type = static_cast<EventType>(json["event_type"].toInt());
    m_priority = static_cast<Priority>(json["priority"].toInt());
    m_data = json["data"].toObject();

    auto timestamp_ms = json["timestamp"].toString().toLongLong();
    m_timestamp = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(timestamp_ms));
}

} // namespace qtplugin::examples
