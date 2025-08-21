/**
 * @file message_types.hpp
 * @brief Common message types for plugin communication
 * @version 3.0.0
 */

#pragma once

#include "message_bus.hpp"
#include "../core/plugin_interface.hpp"
#include <QJsonObject>
#include <string>
#include <string_view>
#include <chrono>

namespace qtplugin::messages {

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
    
    PluginLifecycleMessage(std::string_view sender, std::string_view plugin_id, Event event)
        : Message(sender), m_plugin_id(plugin_id), m_event(event) {}
    
    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    Event event() const noexcept { return m_event; }
    
    QJsonObject to_json() const override {
        return QJsonObject{
            {"type", "plugin_lifecycle"},
            {"sender", QString::fromStdString(std::string(sender()))},
            {"plugin_id", QString::fromStdString(m_plugin_id)},
            {"event", event_to_string(m_event)},
            {"timestamp", QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp().time_since_epoch()).count())}
        };
    }
    
private:
    std::string m_plugin_id;
    Event m_event;
    
    static const char* event_to_string(Event event) {
        switch (event) {
            case Event::Loading: return "loading";
            case Event::Loaded: return "loaded";
            case Event::Initializing: return "initializing";
            case Event::Initialized: return "initialized";
            case Event::Starting: return "starting";
            case Event::Started: return "started";
            case Event::Stopping: return "stopping";
            case Event::Stopped: return "stopped";
            case Event::Unloading: return "unloading";
            case Event::Unloaded: return "unloaded";
            case Event::Error: return "error";
        }
        return "unknown";
    }
};

/**
 * @brief Plugin configuration change message
 */
class ConfigurationChangedMessage : public Message<ConfigurationChangedMessage> {
public:
    ConfigurationChangedMessage(std::string_view sender, std::string_view plugin_id,
                               const QJsonObject& old_config, const QJsonObject& new_config)
        : Message(sender), m_plugin_id(plugin_id), m_old_config(old_config), m_new_config(new_config) {}
    
    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    const QJsonObject& old_configuration() const noexcept { return m_old_config; }
    const QJsonObject& new_configuration() const noexcept { return m_new_config; }
    
    QJsonObject to_json() const override {
        return QJsonObject{
            {"type", "configuration_changed"},
            {"sender", QString::fromStdString(std::string(sender()))},
            {"plugin_id", QString::fromStdString(m_plugin_id)},
            {"old_config", m_old_config},
            {"new_config", m_new_config},
            {"timestamp", QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp().time_since_epoch()).count())}
        };
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
    PluginCommandMessage(std::string_view sender, std::string_view target_plugin,
                        std::string_view command, const QJsonObject& parameters = {},
                        MessagePriority priority = MessagePriority::Normal)
        : Message(sender, priority), m_target_plugin(target_plugin), 
          m_command(command), m_parameters(parameters) {}
    
    std::string_view target_plugin() const noexcept { return m_target_plugin; }
    std::string_view command() const noexcept { return m_command; }
    const QJsonObject& parameters() const noexcept { return m_parameters; }
    
    QJsonObject to_json() const override {
        return QJsonObject{
            {"type", "plugin_command"},
            {"sender", QString::fromStdString(std::string(sender()))},
            {"target_plugin", QString::fromStdString(m_target_plugin)},
            {"command", QString::fromStdString(m_command)},
            {"parameters", m_parameters},
            {"priority", static_cast<int>(priority())},
            {"timestamp", QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp().time_since_epoch()).count())}
        };
    }
    
private:
    std::string m_target_plugin;
    std::string m_command;
    QJsonObject m_parameters;
};

/**
 * @brief Plugin command response message
 */
class PluginCommandResponseMessage : public Message<PluginCommandResponseMessage> {
public:
    PluginCommandResponseMessage(std::string_view sender, std::string_view request_id,
                                bool success, const QJsonObject& result = {},
                                std::string_view error_message = "")
        : Message(sender), m_request_id(request_id), m_success(success), 
          m_result(result), m_error_message(error_message) {}
    
    std::string_view request_id() const noexcept { return m_request_id; }
    bool success() const noexcept { return m_success; }
    const QJsonObject& result() const noexcept { return m_result; }
    std::string_view error_message() const noexcept { return m_error_message; }
    
    QJsonObject to_json() const override {
        QJsonObject json{
            {"type", "plugin_command_response"},
            {"sender", QString::fromStdString(std::string(sender()))},
            {"request_id", QString::fromStdString(m_request_id)},
            {"success", m_success},
            {"result", m_result},
            {"timestamp", QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp().time_since_epoch()).count())}
        };
        
        if (!m_error_message.empty()) {
            json["error_message"] = QString::fromStdString(m_error_message);
        }
        
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
    
    SystemStatusMessage(std::string_view sender, Status status, std::string_view details = "")
        : Message(sender, MessagePriority::High), m_status(status), m_details(details) {}
    
    Status status() const noexcept { return m_status; }
    std::string_view details() const noexcept { return m_details; }
    
    QJsonObject to_json() const override {
        QJsonObject json{
            {"type", "system_status"},
            {"sender", QString::fromStdString(std::string(sender()))},
            {"status", status_to_string(m_status)},
            {"timestamp", QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp().time_since_epoch()).count())}
        };
        
        if (!m_details.empty()) {
            json["details"] = QString::fromStdString(m_details);
        }
        
        return json;
    }
    
private:
    Status m_status;
    std::string m_details;
    
    static const char* status_to_string(Status status) {
        switch (status) {
            case Status::Starting: return "starting";
            case Status::Running: return "running";
            case Status::Stopping: return "stopping";
            case Status::Stopped: return "stopped";
            case Status::Error: return "error";
            case Status::Maintenance: return "maintenance";
        }
        return "unknown";
    }
};

/**
 * @brief Resource usage message
 */
class ResourceUsageMessage : public Message<ResourceUsageMessage> {
public:
    struct ResourceInfo {
        double cpu_usage = 0.0;        ///< CPU usage percentage
        uint64_t memory_usage = 0;     ///< Memory usage in bytes
        uint64_t disk_usage = 0;       ///< Disk usage in bytes
        uint32_t thread_count = 0;     ///< Number of threads
        uint32_t handle_count = 0;     ///< Number of handles/file descriptors
    };
    
    ResourceUsageMessage(std::string_view sender, std::string_view plugin_id, const ResourceInfo& info)
        : Message(sender), m_plugin_id(plugin_id), m_resource_info(info) {}
    
    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    const ResourceInfo& resource_info() const noexcept { return m_resource_info; }
    
    QJsonObject to_json() const override {
        return QJsonObject{
            {"type", "resource_usage"},
            {"sender", QString::fromStdString(std::string(sender()))},
            {"plugin_id", QString::fromStdString(m_plugin_id)},
            {"cpu_usage", m_resource_info.cpu_usage},
            {"memory_usage", static_cast<qint64>(m_resource_info.memory_usage)},
            {"disk_usage", static_cast<qint64>(m_resource_info.disk_usage)},
            {"thread_count", static_cast<int>(m_resource_info.thread_count)},
            {"handle_count", static_cast<int>(m_resource_info.handle_count)},
            {"timestamp", QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp().time_since_epoch()).count())}
        };
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
                     const QJsonObject& data, MessagePriority priority = MessagePriority::Normal)
        : Message(sender, priority), m_data_type(data_type), m_data(data) {}
    
    std::string_view data_type() const noexcept { return m_data_type; }
    const QJsonObject& data() const noexcept { return m_data; }
    
    QJsonObject to_json() const override {
        return QJsonObject{
            {"type", "custom_data"},
            {"sender", QString::fromStdString(std::string(sender()))},
            {"data_type", QString::fromStdString(m_data_type)},
            {"data", m_data},
            {"priority", static_cast<int>(priority())},
            {"timestamp", QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp().time_since_epoch()).count())}
        };
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
        : Message(sender, MessagePriority::High), m_plugin_id(plugin_id), m_error(error) {}
    
    std::string_view plugin_id() const noexcept { return m_plugin_id; }
    const PluginError& error() const noexcept { return m_error; }
    
    QJsonObject to_json() const override {
        return QJsonObject{
            {"type", "error"},
            {"sender", QString::fromStdString(std::string(sender()))},
            {"plugin_id", QString::fromStdString(m_plugin_id)},
            {"error_code", static_cast<int>(m_error.code)},
            {"error_message", QString::fromStdString(m_error.message)},
            {"error_details", QString::fromStdString(m_error.details)},
            {"timestamp", QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp().time_since_epoch()).count())}
        };
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
    enum class Level {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };
    
    LogMessage(std::string_view sender, Level level, std::string_view message,
              std::string_view category = "")
        : Message(sender), m_level(level), m_message(message), m_category(category) {}
    
    Level level() const noexcept { return m_level; }
    std::string_view message() const noexcept { return m_message; }
    std::string_view category() const noexcept { return m_category; }
    
    QJsonObject to_json() const override {
        QJsonObject json{
            {"type", "log"},
            {"sender", QString::fromStdString(std::string(sender()))},
            {"level", level_to_string(m_level)},
            {"message", QString::fromStdString(m_message)},
            {"timestamp", QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp().time_since_epoch()).count())}
        };
        
        if (!m_category.empty()) {
            json["category"] = QString::fromStdString(m_category);
        }
        
        return json;
    }
    
private:
    Level m_level;
    std::string m_message;
    std::string m_category;
    
    static const char* level_to_string(Level level) {
        switch (level) {
            case Level::Debug: return "debug";
            case Level::Info: return "info";
            case Level::Warning: return "warning";
            case Level::Error: return "error";
            case Level::Critical: return "critical";
        }
        return "unknown";
    }
};

} // namespace qtplugin::messages
