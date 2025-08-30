/**
 * @file system_event_message.hpp
 * @brief System event message implementation
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <chrono>
#include <string>
#include <string_view>

#include "qtplugin/communication/message_bus.hpp"

namespace qtplugin::examples {

/**
 * @brief Custom message type for system events
 */
class SystemEventMessage : public qtplugin::IMessage {
public:
    enum class EventType {
        SystemStartup,
        SystemShutdown,
        PluginLoaded,
        PluginUnloaded,
        ConfigurationChanged,
        ErrorOccurred
    };

    SystemEventMessage(EventType event_type, qtplugin::MessagePriority priority = qtplugin::MessagePriority::Normal);
    ~SystemEventMessage() = default;

    // IMessage interface
    std::string_view type() const noexcept override { return "system_event"; }
    std::string_view sender() const noexcept override { return m_sender; }
    std::chrono::system_clock::time_point timestamp() const noexcept override { return m_timestamp; }
    qtplugin::MessagePriority priority() const noexcept override { return m_priority; }
    QJsonObject to_json() const override;
    std::string id() const noexcept override { return m_id; }

    // Custom properties
    EventType event_type() const { return m_event_type; }
    Priority priority() const { return m_priority; }
    void set_sender(const std::string& sender) { m_sender = sender; }
    void set_topic(const std::string& topic) { m_topic = topic; }
    void set_data(const QJsonObject& data) { m_data = data; }
    QJsonObject data() const { return m_data; }

private:
    EventType m_event_type;
    qtplugin::MessagePriority m_priority;
    std::string m_sender;
    std::string m_id;
    std::string m_topic;
    QJsonObject m_data;
    std::chrono::system_clock::time_point m_timestamp;
};

} // namespace qtplugin::examples
