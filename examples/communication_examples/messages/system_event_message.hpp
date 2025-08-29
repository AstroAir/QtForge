/**
 * @file system_event_message.hpp
 * @brief System event message implementation
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <chrono>
#include <string>

#include "qtplugin/communication/message_types.hpp"

namespace qtplugin::examples {

/**
 * @brief Custom message type for system events
 */
class SystemEventMessage : public IMessage {
public:
    enum class EventType {
        SystemStartup,
        SystemShutdown,
        PluginLoaded,
        PluginUnloaded,
        ConfigurationChanged,
        ErrorOccurred
    };

    enum class Priority {
        Low = 1,
        Normal = 2,
        High = 3,
        Critical = 4
    };

    SystemEventMessage(EventType event_type, Priority priority = Priority::Normal);
    ~SystemEventMessage() override = default;

    // IMessage interface
    std::string type() const override { return "system_event"; }
    std::string sender() const override { return m_sender; }
    std::string topic() const override { return m_topic; }
    QJsonObject to_json() const override;
    void from_json(const QJsonObject& json) override;
    std::chrono::system_clock::time_point timestamp() const override { return m_timestamp; }

    // Custom properties
    EventType event_type() const { return m_event_type; }
    Priority priority() const { return m_priority; }
    void set_sender(const std::string& sender) { m_sender = sender; }
    void set_topic(const std::string& topic) { m_topic = topic; }
    void set_data(const QJsonObject& data) { m_data = data; }
    QJsonObject data() const { return m_data; }

private:
    EventType m_event_type;
    Priority m_priority;
    std::string m_sender;
    std::string m_topic;
    QJsonObject m_data;
    std::chrono::system_clock::time_point m_timestamp;
};

} // namespace qtplugin::examples
