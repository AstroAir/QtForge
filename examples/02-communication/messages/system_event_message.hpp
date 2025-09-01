/**
 * @file system_event_message.hpp
 * @brief System event message implementation for communication examples
 * @version 3.0.0
 */

#pragma once

#include <chrono>
#include <string>

namespace qtplugin::examples {

/**
 * @brief Simple system event message for demonstration purposes
 */
class SystemEventMessage {
public:
    enum class Priority { Low = 0, Normal = 1, High = 2, Critical = 3 };

    enum class EventType { Info, Warning, Error, Debug, System };

    SystemEventMessage(const std::string& sender, const std::string& content,
                       EventType event_type = EventType::Info,
                       Priority priority = Priority::Normal);

    // Accessor methods
    std::string type() const { return "SystemEvent"; }
    std::string sender() const { return m_sender; }
    std::string content() const { return m_content; }
    std::chrono::system_clock::time_point timestamp() const {
        return m_timestamp;
    }

    // SystemEventMessage specific methods
    EventType event_type() const { return m_event_type; }
    Priority priority() const { return m_priority; }

private:
    std::string m_sender;
    std::string m_content;
    EventType m_event_type;
    Priority m_priority;
    std::chrono::system_clock::time_point m_timestamp;
};

}  // namespace qtplugin::examples
