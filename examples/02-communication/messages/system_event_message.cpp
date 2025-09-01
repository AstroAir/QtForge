/**
 * @file system_event_message.cpp
 * @brief System event message implementation for communication examples
 * @version 3.0.0
 */

#include "system_event_message.hpp"

namespace qtplugin::examples {

SystemEventMessage::SystemEventMessage(const std::string& sender,
                                      const std::string& content,
                                      EventType event_type,
                                      Priority priority)
    : m_sender(sender)
    , m_content(content)
    , m_event_type(event_type)
    , m_priority(priority)
    , m_timestamp(std::chrono::system_clock::now())
{
}

} // namespace qtplugin::examples
