/**
 * @file message_types.hpp
 * @brief Common message types for plugin communication
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>

#include <array>
#include <chrono>
#include <string>
#include <string_view>

#include "message_bus.hpp"

namespace qtplugin::messages {

// Helper functions for reducing code duplication
namespace detail {
/**
 * @brief Converts a timestamp to JSON-compatible string format (milliseconds since epoch)
 * @param tp The system clock time point to convert
 * @return QString representation of the timestamp
 */
inline QString timestamp_to_json_string(
    const std::chrono::system_clock::time_point& tp) {
    return QString::number(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            tp.time_since_epoch())
            .count());
}

/**
 * @brief Converts a string_view to QString efficiently using UTF-8 encoding
 * @param sv The string_view to convert
 * @return QString equivalent
 */
inline QString to_qstring(std::string_view sv) {
    return QString::fromUtf8(sv.data(), static_cast<int>(sv.size()));
}

/**
 * @brief Creates a base JSON object with common message fields
 * @param type The message type string
 * @param sender The sender identifier
 * @param timestamp The message timestamp
 * @return QJsonObject with type, sender, and timestamp fields
 */
inline QJsonObject create_base_json(
    const char* type, std::string_view sender,
    const std::chrono::system_clock::time_point& timestamp) {
    return QJsonObject{{"type", type},
                       {"sender", to_qstring(sender)},
                       {"timestamp", timestamp_to_json_string(timestamp)}};
}

/**
 * @brief Adds an optional string field to a JSON object if the value is not empty
 * @param json The JSON object to modify
 * @param key The field key
 * @param value The string value to add if non-empty
 */
inline void add_optional_field(QJsonObject& json, const char* key,
                               std::string_view value) {
    if (!value.empty()) {
        json[key] = to_qstring(value);
    }
}

/**
 * @brief Template function for converting an enum value to its string representation using a lookup array
 * @tparam EnumType The enum type
 * @tparam N The size of the string array
 * @param value The enum value to convert
 * @param strings The array of string literals for enum values
 * @return Const char* string for the enum value, or "unknown" if out of bounds
 */
template <typename EnumType, size_t N>
constexpr const char* enum_to_string(
    EnumType value, const std::array<const char*, N>& strings) {
    const auto index = static_cast<size_t>(value);
    return (index < N) ? strings[index] : "unknown";
}

// Enum-to-string mapping arrays
constexpr std::array lifecycle_event_strings = {
    "loading",  "loaded",  "initializing", "initialized", "starting", "started",
    "stopping", "stopped", "unloading",    "unloaded",    "error"};

constexpr std::array system_status_strings = {
    "starting", "running", "stopping", "stopped", "error", "maintenance"};

constexpr std::array log_level_strings = {"debug", "info", "warning", "error",
                                          "critical"};
}  // namespace detail

/**
 * @brief Plugin lifecycle event message for notifying about plugin state changes
 */
class PluginLifecycleMessage : public Message<PluginLifecycleMessage> {
public:
    /**
     * @brief Enum representing different plugin lifecycle events
     */
    enum class Event {
        Loading,      ///< Plugin is being loaded
        Loaded,       ///< Plugin has been loaded successfully
        Initializing, ///< Plugin is initializing
        Initialized,  ///< Plugin has been initialized
        Starting,     ///< Plugin is starting
        Started,      ///< Plugin has started successfully
        Stopping,     ///< Plugin is stopping
        Stopped,      ///< Plugin has stopped
        Unloading,    ///< Plugin is being unloaded
        Unloaded,     ///< Plugin has been unloaded
        Error         ///< An error occurred during lifecycle
    };

    /**
     * @brief Constructs a plugin lifecycle message
     * @param sender The sender identifier
     * @param plugin_id The ID of the affected plugin
     * @param event The lifecycle event type
     */
    PluginLifecycleMessage(std::string_view sender, std::string_view plugin_id,
                           Event event)
        : Message(sender), m_plugin_id(plugin_id), m_event(event) {}

    /**
     * @brief Gets the plugin ID
     * @return The plugin identifier
     */
    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    
    /**
     * @brief Gets the lifecycle event
     * @return The event type
     */
    Event event() const noexcept { return m_event; }

    /**
     * @brief Serializes the message to JSON format
     * @return QJsonObject representation of the message
     */
    QJsonObject to_json() const override {
        auto json =
            detail::create_base_json("plugin_lifecycle", sender(), timestamp());
        json["plugin_id"] = detail::to_qstring(m_plugin_id);
        json["event"] =
            detail::enum_to_string(m_event, detail::lifecycle_event_strings);
        return json;
    }

private:
    std::string m_plugin_id;
    Event m_event;
};

/**
 * @brief Message indicating a change in plugin configuration
 */
class ConfigurationChangedMessage
    : public Message<ConfigurationChangedMessage> {
public:
    /**
     * @brief Constructs a configuration change message
     * @param sender The sender identifier
     * @param plugin_id The ID of the plugin whose config changed
     * @param old_config The previous configuration
     * @param new_config The updated configuration
     */
    ConfigurationChangedMessage(std::string_view sender,
                                std::string_view plugin_id,
                                const QJsonObject& old_config,
                                const QJsonObject& new_config)
        : Message(sender),
          m_plugin_id(plugin_id),
          m_old_config(old_config),
          m_new_config(new_config) {}

    /**
     * @brief Gets the plugin ID
     * @return The plugin identifier
     */
    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    
    /**
     * @brief Gets the old configuration
     * @return The previous QJsonObject config
     */
    const QJsonObject& old_configuration() const noexcept {
        return m_old_config;
    }
    
    /**
     * @brief Gets the new configuration
     * @return The updated QJsonObject config
     */
    const QJsonObject& new_configuration() const noexcept {
        return m_new_config;
    }

    /**
     * @brief Serializes the message to JSON format
     * @return QJsonObject representation of the message
     */
    QJsonObject to_json() const override {
        auto json = detail::create_base_json("configuration_changed", sender(),
                                             timestamp());
        json["plugin_id"] = detail::to_qstring(m_plugin_id);
        json["old_config"] = m_old_config;
        json["new_config"] = m_new_config;
        return json;
    }

private:
    std::string m_plugin_id;
    QJsonObject m_old_config;
    QJsonObject m_new_config;
};

/**
 * @brief Message for sending commands to plugins
 */
class PluginCommandMessage : public Message<PluginCommandMessage> {
public:
    /**
     * @brief Constructs a plugin command message
     * @param sender The sender identifier
     * @param target_plugin The ID of the target plugin
     * @param command The command string
     * @param parameters Optional JSON parameters for the command
     * @param priority Message priority (default: Normal)
     */
    PluginCommandMessage(std::string_view sender,
                         std::string_view target_plugin,
                         std::string_view command,
                         const QJsonObject& parameters = {},
                         MessagePriority priority = MessagePriority::Normal)
        : Message(sender, priority),
          m_target_plugin(target_plugin),
          m_command(command),
          m_parameters(parameters) {}

    /**
     * @brief Gets the target plugin ID
     * @return The target plugin identifier
     */
    std::string_view target_plugin() const noexcept { return m_target_plugin; }
    
    /**
     * @brief Gets the command string
     * @return The command name
     */
    std::string_view command() const noexcept { return m_command; }
    
    /**
     * @brief Gets the command parameters
     * @return The QJsonObject parameters
     */
    const QJsonObject& parameters() const noexcept { return m_parameters; }

    /**
     * @brief Serializes the message to JSON format
     * @return QJsonObject representation of the message
     */
    QJsonObject to_json() const override {
        auto json =
            detail::create_base_json("plugin_command", sender(), timestamp());
        json["target_plugin"] = detail::to_qstring(m_target_plugin);
        json["command"] = detail::to_qstring(m_command);
        json["parameters"] = m_parameters;
        json["priority"] = static_cast<int>(priority());
        return json;
    }

private:
    std::string m_target_plugin;
    std::string m_command;
    QJsonObject m_parameters;
};

/**
 * @brief Response message for plugin commands
 */
class PluginCommandResponseMessage
    : public Message<PluginCommandResponseMessage> {
public:
    /**
     * @brief Constructs a plugin command response message
     * @param sender The sender identifier
     * @param request_id The ID of the original request
     * @param success Whether the command succeeded
     * @param result Optional result data
     * @param error_message Optional error description
     */
    PluginCommandResponseMessage(std::string_view sender,
                                 std::string_view request_id, bool success,
                                 const QJsonObject& result = {},
                                 std::string_view error_message = "")
        : Message(sender),
          m_request_id(request_id),
          m_success(success),
          m_result(result),
          m_error_message(error_message) {}

    /**
     * @brief Gets the request ID
     * @return The original request identifier
     */
    std::string_view request_id() const noexcept { return m_request_id; }
    
    /**
     * @brief Checks if the command was successful
     * @return True if successful
     */
    bool success() const noexcept { return m_success; }
    
    /**
     * @brief Gets the result data
     * @return The QJsonObject result
     */
    const QJsonObject& result() const noexcept { return m_result; }
    
    /**
     * @brief Gets the error message if any
     * @return The error description string
     */
    std::string_view error_message() const noexcept { return m_error_message; }

    /**
     * @brief Serializes the message to JSON format
     * @return QJsonObject representation of the message
     */
    QJsonObject to_json() const override {
        auto json = detail::create_base_json("plugin_command_response",
                                             sender(), timestamp());
        json["request_id"] = detail::to_qstring(m_request_id);
        json["success"] = m_success;
        json["result"] = m_result;
        detail::add_optional_field(json, "error_message", m_error_message);
        return json;
    }

private:
    std::string m_request_id;
    bool m_success;
    QJsonObject m_result;
    std::string m_error_message;
};

/**
 * @brief Message reporting system status updates
 */
class SystemStatusMessage : public Message<SystemStatusMessage> {
public:
    /**
     * @brief Enum representing system status levels
     */
    enum class Status {
        Starting,     ///< System is starting up
        Running,      ///< System is running normally
        Stopping,     ///< System is shutting down
        Stopped,      ///< System has stopped
        Error,        ///< System encountered an error
        Maintenance   ///< System is in maintenance mode
    };

    /**
     * @brief Constructs a system status message
     * @param sender The sender identifier
     * @param status The current system status
     * @param details Optional additional details
     */
    SystemStatusMessage(std::string_view sender, Status status,
                        std::string_view details = "")
        : Message(sender, MessagePriority::High),
          m_status(status),
          m_details(details) {}

    /**
     * @brief Gets the system status
     * @return The status enum value
     */
    Status status() const noexcept { return m_status; }
    
    /**
     * @brief Gets the status details
     * @return The details string
     */
    std::string_view details() const noexcept { return m_details; }

    /**
     * @brief Serializes the message to JSON format
     * @return QJsonObject representation of the message
     */
    QJsonObject to_json() const override {
        auto json =
            detail::create_base_json("system_status", sender(), timestamp());
        json["status"] =
            detail::enum_to_string(m_status, detail::system_status_strings);
        detail::add_optional_field(json, "details", m_details);
        return json;
    }

private:
    Status m_status;
    std::string m_details;
};

/**
 * @brief Message reporting resource usage statistics
 */
class ResourceUsageMessage : public Message<ResourceUsageMessage> {
public:
    /**
     * @brief Structure holding resource usage information
     */
    struct ResourceInfo {
        double cpu_usage = 0.0;     ///< CPU usage percentage (0.0 to 100.0)
        uint64_t memory_usage = 0;  ///< Memory usage in bytes
        uint64_t disk_usage = 0;    ///< Disk usage in bytes
        uint32_t thread_count = 0;  ///< Number of active threads
        uint32_t handle_count = 0;  ///< Number of open handles/file descriptors
    };

    /**
     * @brief Constructs a resource usage message
     * @param sender The sender identifier
     * @param plugin_id The ID of the plugin reporting usage
     * @param info The resource usage information
     */
    ResourceUsageMessage(std::string_view sender, std::string_view plugin_id,
                         const ResourceInfo& info)
        : Message(sender), m_plugin_id(plugin_id), m_resource_info(info) {}

    /**
     * @brief Gets the plugin ID
     * @return The plugin identifier
     */
    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    
    /**
     * @brief Gets the resource information
     * @return Const reference to ResourceInfo
     */
    const ResourceInfo& resource_info() const noexcept {
        return m_resource_info;
    }

    /**
     * @brief Serializes the message to JSON format
     * @return QJsonObject representation of the message
     */
    QJsonObject to_json() const override {
        auto json =
            detail::create_base_json("resource_usage", sender(), timestamp());
        json["plugin_id"] = detail::to_qstring(m_plugin_id);
        json["cpu_usage"] = m_resource_info.cpu_usage;
        json["memory_usage"] =
            static_cast<qint64>(m_resource_info.memory_usage);
        json["disk_usage"] = static_cast<qint64>(m_resource_info.disk_usage);
        json["thread_count"] = static_cast<int>(m_resource_info.thread_count);
        json["handle_count"] = static_cast<int>(m_resource_info.handle_count);
        return json;
    }

private:
    std::string m_plugin_id;
    ResourceInfo m_resource_info;
};

/**
 * @brief Generic message for custom plugin-specific data exchange
 */
class CustomDataMessage : public Message<CustomDataMessage> {
public:
    /**
     * @brief Constructs a custom data message
     * @param sender The sender identifier
     * @param data_type The type identifier for the custom data
     * @param data The JSON data payload
     * @param priority Message priority (default: Normal)
     */
    CustomDataMessage(std::string_view sender, std::string_view data_type,
                      const QJsonObject& data,
                      MessagePriority priority = MessagePriority::Normal)
        : Message(sender, priority), m_data_type(data_type), m_data(data) {}

    /**
     * @brief Gets the data type
     * @return The custom data type string
     */
    std::string_view data_type() const noexcept { return m_data_type; }
    
    /**
     * @brief Gets the data payload
     * @return The QJsonObject data
     */
    const QJsonObject& data() const noexcept { return m_data; }

    /**
     * @brief Serializes the message to JSON format
     * @return QJsonObject representation of the message
     */
    QJsonObject to_json() const override {
        auto json =
            detail::create_base_json("custom_data", sender(), timestamp());
        json["data_type"] = detail::to_qstring(m_data_type);
        json["data"] = m_data;
        json["priority"] = static_cast<int>(priority());
        return json;
    }

private:
    std::string m_data_type;
    QJsonObject m_data;
};

/**
 * @brief Message for reporting errors from plugins
 */
class ErrorMessage : public Message<ErrorMessage> {
public:
    /**
     * @brief Constructs an error message
     * @param sender The sender identifier
     * @param plugin_id The ID of the plugin reporting the error
     * @param error The PluginError details
     */
    ErrorMessage(std::string_view sender, std::string_view plugin_id,
                 const PluginError& error)
        : Message(sender, MessagePriority::High),
          m_plugin_id(plugin_id),
          m_error(error) {}

    /**
     * @brief Gets the plugin ID
     * @return The plugin identifier
     */
    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    
    /**
     * @brief Gets the error details
     * @return Const reference to PluginError
     */
    const PluginError& error() const noexcept { return m_error; }

    /**
     * @brief Serializes the message to JSON format
     * @return QJsonObject representation of the message
     */
    QJsonObject to_json() const override {
        auto json = detail::create_base_json("error", sender(), timestamp());
        json["plugin_id"] = detail::to_qstring(m_plugin_id);
        json["error_code"] = static_cast<int>(m_error.code);
        json["error_message"] = detail::to_qstring(m_error.message);
        json["error_details"] = detail::to_qstring(m_error.details);
        return json;
    }

private:
    std::string m_plugin_id;
    PluginError m_error;
};

/**
 * @brief Message for logging events across the system
 */
class LogMessage : public Message<LogMessage> {
public:
    /**
     * @brief Enum representing log levels
     */
    enum class Level {
        Debug,     ///< Debug level logging
        Info,      ///< Informational logging
        Warning,   ///< Warning level logging
        Error,     ///< Error level logging
        Critical   ///< Critical error logging
    };

    /**
     * @brief Constructs a log message
     * @param sender The sender identifier
     * @param level The log level
     * @param message The log message content
     * @param category Optional log category
     */
    LogMessage(std::string_view sender, Level level, std::string_view message,
               std::string_view category = "")
        : Message(sender),
          m_level(level),
          m_message(message),
          m_category(category) {}

    /**
     * @brief Gets the log level
     * @return The Level enum value
     */
    Level level() const noexcept { return m_level; }
    
    /**
     * @brief Gets the log message
     * @return The message string
     */
    std::string_view message() const noexcept { return m_message; }
    
    /**
     * @brief Gets the log category
     * @return The category string
     */
    std::string_view category() const noexcept { return m_category; }

    /**
     * @brief Serializes the message to JSON format
     * @return QJsonObject representation of the message
     */
    QJsonObject to_json() const override {
        auto json = detail::create_base_json("log", sender(), timestamp());
        json["level"] =
            detail::enum_to_string(m_level, detail::log_level_strings);
        json["message"] = detail::to_qstring(m_message);
        detail::add_optional_field(json, "category", m_category);
        return json;
    }

private:
    Level m_level;
    std::string m_message;
    std::string m_category;
};

}  // namespace qtplugin::messages
