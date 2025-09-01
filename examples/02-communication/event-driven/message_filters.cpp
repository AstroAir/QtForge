/**
 * @file message_filters.cpp
 * @brief Message filtering implementations
 * @version 3.0.0
 */

#include "message_filters.hpp"

namespace qtplugin::examples {

// PriorityMessageFilter Implementation
PriorityMessageFilter::PriorityMessageFilter(SystemEventMessage::Priority min_priority)
    : m_min_priority(min_priority)
{
}

bool PriorityMessageFilter::operator()(const SystemEventMessage& message) const {
    return message.priority() >= m_min_priority;
}

// EventTypeMessageFilter Implementation
EventTypeMessageFilter::EventTypeMessageFilter(SystemEventMessage::EventType target_type)
    : m_target_type(target_type)
{
}

bool EventTypeMessageFilter::operator()(const SystemEventMessage& message) const {
    return message.event_type() == m_target_type;
}

// SenderMessageFilter Implementation
SenderMessageFilter::SenderMessageFilter(const std::string& sender_pattern)
    : m_sender_pattern(sender_pattern)
{
}

bool SenderMessageFilter::operator()(const SystemEventMessage& message) const {
    std::string sender = message.sender();

    // Simple pattern matching - check if sender contains the pattern
    return sender.find(m_sender_pattern) != std::string::npos;
}

} // namespace qtplugin::examples
