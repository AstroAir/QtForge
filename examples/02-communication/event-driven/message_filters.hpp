/**
 * @file message_filters.hpp
 * @brief Message filtering implementations
 * @version 3.0.0
 */

#pragma once

#include "../messages/system_event_message.hpp"
#include <string>

namespace qtplugin::examples {

/**
 * @brief Message filter for priority-based filtering
 */
class PriorityMessageFilter {
public:
    explicit PriorityMessageFilter(SystemEventMessage::Priority min_priority);

    bool operator()(const SystemEventMessage& message) const;

private:
    SystemEventMessage::Priority m_min_priority;
};

/**
 * @brief Message filter for event type filtering
 */
class EventTypeMessageFilter {
public:
    explicit EventTypeMessageFilter(SystemEventMessage::EventType target_type);

    bool operator()(const SystemEventMessage& message) const;

private:
    SystemEventMessage::EventType m_target_type;
};

/**
 * @brief Message filter for sender-based filtering
 */
class SenderMessageFilter {
public:
    explicit SenderMessageFilter(const std::string& sender_pattern);

    bool operator()(const SystemEventMessage& message) const;

private:
    std::string m_sender_pattern;
};

} // namespace qtplugin::examples
