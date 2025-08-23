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
 * @brief Convert timestamp to JSON string format
 */
inline QString timestamp_to_json_string(
    const std::chrono::system_clock::time_point& tp) {
    return QString::number(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            tp.time_since_epoch())
            .count());
}

/**
 * @brief Convert string_view to QString efficiently
 */
inline QString to_qstring(std::string_view sv) {
    return QString::fromUtf8(sv.data(), static_cast<int>(sv.size()));
}

/**
 * @brief Create base JSON object with common fields
 */
inline QJsonObject create_base_json(
    const char* type, std::string_view sender,
    const std::chrono::system_clock::time_point& timestamp) {
    return QJsonObject{{"type", type},
                       {"sender", to_qstring(sender)},
                       {"timestamp", timestamp_to_json_string(timestamp)}};
}

/**
 * @brief Add optional string field to JSON if value is not empty
 */
inline void add_optional_field(QJsonObject& json, const char* key,
                               std::string_view value) {
    if (!value.empty()) {
        json[key] = to_qstring(value);
    }
}

/**
 * @brief Template for enum-to-string conversion using lookup arrays
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
 * @brief Plugin lifecycle event message
 */
class PluginLifecycleMessage : public Message<PluginLifecycleMessage> {
public:
    enum class Event {
        Loading,
        Loaded,
        Initializing,
        Initialized,
        Starting,
        Started,
        Stopping,
        Stopped,
        Unloading,
        Unloaded,
        Error
    };

    PluginLifecycleMessage(std::string_view sender, std::string_view plugin_id,
                           Event event)
        : Message(sender), m_plugin_id(plugin_id), m_event(event) {}

    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    Event event() const noexcept { return m_event; }

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
 * @brief Plugin configuration change message
 */
class ConfigurationChangedMessage
    : public Message<ConfigurationChangedMessage> {
public:
    ConfigurationChangedMessage(std::string_view sender,
                                std::string_view plugin_id,
                                const QJsonObject& old_config,
                                const QJsonObject& new_config)
        : Message(sender),
          m_plugin_id(plugin_id),
          m_old_config(old_config),
          m_new_config(new_config) {}

    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    const QJsonObject& old_configuration() const noexcept {
        return m_old_config;
    }
    const QJsonObject& new_configuration() const noexcept {
        return m_new_config;
    }

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
 * @brief Plugin command message
 */
class PluginCommandMessage : public Message<PluginCommandMessage> {
public:
    PluginCommandMessage(std::string_view sender,
                         std::string_view target_plugin,
                         std::string_view command,
                         const QJsonObject& parameters = {},
                         MessagePriority priority = MessagePriority::Normal)
        : Message(sender, priority),
          m_target_plugin(target_plugin),
          m_command(command),
          m_parameters(parameters) {}

    std::string_view target_plugin() const noexcept { return m_target_plugin; }
    std::string_view command() const noexcept { return m_command; }
    const QJsonObject& parameters() const noexcept { return m_parameters; }

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
 * @brief Plugin command response message
 */
class PluginCommandResponseMessage
    : public Message<PluginCommandResponseMessage> {
public:
    PluginCommandResponseMessage(std::string_view sender,
                                 std::string_view request_id, bool success,
                                 const QJsonObject& result = {},
                                 std::string_view error_message = "")
        : Message(sender),
          m_request_id(request_id),
          m_success(success),
          m_result(result),
          m_error_message(error_message) {}

    std::string_view request_id() const noexcept { return m_request_id; }
    bool success() const noexcept { return m_success; }
    const QJsonObject& result() const noexcept { return m_result; }
    std::string_view error_message() const noexcept { return m_error_message; }

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
 * @brief System status message
 */
class SystemStatusMessage : public Message<SystemStatusMessage> {
public:
    enum class Status {
        Starting,
        Running,
        Stopping,
        Stopped,
        Error,
        Maintenance
    };

    SystemStatusMessage(std::string_view sender, Status status,
                        std::string_view details = "")
        : Message(sender, MessagePriority::High),
          m_status(status),
          m_details(details) {}

    Status status() const noexcept { return m_status; }
    std::string_view details() const noexcept { return m_details; }

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
 * @brief Resource usage message
 */
class ResourceUsageMessage : public Message<ResourceUsageMessage> {
public:
    struct ResourceInfo {
        double cpu_usage = 0.0;     ///< CPU usage percentage
        uint64_t memory_usage = 0;  ///< Memory usage in bytes
        uint64_t disk_usage = 0;    ///< Disk usage in bytes
        uint32_t thread_count = 0;  ///< Number of threads
        uint32_t handle_count = 0;  ///< Number of handles/file descriptors
    };

    ResourceUsageMessage(std::string_view sender, std::string_view plugin_id,
                         const ResourceInfo& info)
        : Message(sender), m_plugin_id(plugin_id), m_resource_info(info) {}

    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    const ResourceInfo& resource_info() const noexcept {
        return m_resource_info;
    }

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
 * @brief Custom data message for plugin-specific communication
 */
class CustomDataMessage : public Message<CustomDataMessage> {
public:
    CustomDataMessage(std::string_view sender, std::string_view data_type,
                      const QJsonObject& data,
                      MessagePriority priority = MessagePriority::Normal)
        : Message(sender, priority), m_data_type(data_type), m_data(data) {}

    std::string_view data_type() const noexcept { return m_data_type; }
    const QJsonObject& data() const noexcept { return m_data; }

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
 * @brief Error message for reporting plugin errors
 */
class ErrorMessage : public Message<ErrorMessage> {
public:
    ErrorMessage(std::string_view sender, std::string_view plugin_id,
                 const PluginError& error)
        : Message(sender, MessagePriority::High),
          m_plugin_id(plugin_id),
          m_error(error) {}

    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    const PluginError& error() const noexcept { return m_error; }

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
 * @brief Log message for centralized logging
 */
class LogMessage : public Message<LogMessage> {
public:
    enum class Level { Debug, Info, Warning, Error, Critical };

    LogMessage(std::string_view sender, Level level, std::string_view message,
               std::string_view category = "")
        : Message(sender),
          m_level(level),
          m_message(message),
          m_category(category) {}

    Level level() const noexcept { return m_level; }
    std::string_view message() const noexcept { return m_message; }
    std::string_view category() const noexcept { return m_category; }

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
