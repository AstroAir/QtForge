/**
 * @file message_utils.hpp
 * @brief Message utility functions for communication examples
 * @version 3.0.0
 */

#pragma once

#include <string>
#include <chrono>

namespace qtplugin::examples {

/**
 * @brief Utility functions for message handling
 */
class MessageUtils {
public:
    // Message ID generation
    static std::string generate_message_id();
    
    // Timestamp utilities
    static std::string timestamp_to_string(const std::chrono::system_clock::time_point& tp);
    static std::chrono::system_clock::time_point string_to_timestamp(const std::string& str);
    
    // Message validation
    static bool is_valid_sender(const std::string& sender);
    static bool is_valid_content(const std::string& content);
    
    // String utilities
    static std::string trim(const std::string& str);
    static std::string to_lower(const std::string& str);

private:
    MessageUtils() = default; // Static utility class
};

} // namespace qtplugin::examples
