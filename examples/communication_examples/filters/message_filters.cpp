/**
 * @file message_filters.cpp
 * @brief Message filtering implementations
 * @version 3.0.0
 */

#include "message_filters.hpp"
#include <algorithm>

namespace qtplugin::examples {

// PriorityMessageFilter Implementation
PriorityMessageFilter::PriorityMessageFilter(SystemEventMessage::Priority min_priority)
    : m_min_priority(min_priority)
{
}

bool PriorityMessageFilter::operator()(const IMessage& message) const {
    // Try to cast to SystemEventMessage
    if (auto* system_msg = dynamic_cast<const SystemEventMessage*>(&message)) {
        return system_msg->priority() >= m_min_priority;
    }
    return true; // Allow non-system messages through
}

// EventTypeMessageFilter Implementation
EventTypeMessageFilter::EventTypeMessageFilter(SystemEventMessage::EventType target_type)
    : m_target_type(target_type)
{
}

bool EventTypeMessageFilter::operator()(const IMessage& message) const {
    // Try to cast to SystemEventMessage
    if (auto* system_msg = dynamic_cast<const SystemEventMessage*>(&message)) {
        return system_msg->event_type() == m_target_type;
    }
    return false; // Only allow matching system messages
}

// SenderMessageFilter Implementation
SenderMessageFilter::SenderMessageFilter(const std::string& sender_pattern)
    : m_sender_pattern(sender_pattern)
{
}

bool SenderMessageFilter::operator()(const IMessage& message) const {
    std::string sender = message.sender();
    
    // Simple pattern matching - check if sender contains the pattern
    return sender.find(m_sender_pattern) != std::string::npos;
}

} // namespace qtplugin::examples
