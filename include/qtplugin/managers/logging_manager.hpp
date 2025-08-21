/**
 * @file logging_manager.hpp
 * @brief Enhanced logging management system for plugins
 * @version 3.0.0
 */

#pragma once

#include "../utils/error_handling.hpp"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonValue>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>
#include <chrono>
#include <filesystem>
#include <shared_mutex>
#include <atomic>

namespace qtplugin {

/**
 * @brief Log levels for structured logging
 */
enum class LogLevel {
    Trace = 0,    ///< Detailed trace information
    Debug = 1,    ///< Debug information
    Info = 2,     ///< General information
    Warning = 3,  ///< Warning messages
    Error = 4,    ///< Error messages
    Critical = 5, ///< Critical errors
    Fatal = 6     ///< Fatal errors (application termination)
};

/**
 * @brief Log output destinations
 */
enum class LogOutput {
    Console,    ///< Console output (stdout/stderr)
    File,       ///< File output
    Network,    ///< Network output (TCP/UDP)
    Database,   ///< Database output
    Custom      ///< Custom output handler
};

/**
 * @brief Log entry structure
 */
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string category;
    std::string plugin_id;
    std::string message;
    QJsonObject context;
    std::string thread_id;
    std::string file;
    int line = 0;
    std::string function;
    
    LogEntry() = default;
    LogEntry(LogLevel lvl, std::string_view cat, std::string_view plugin, 
             std::string_view msg, const QJsonObject& ctx = {})
        : timestamp(std::chrono::system_clock::now())
        , level(lvl)
        , category(cat)
        , plugin_id(plugin)
        , message(msg)
        , context(ctx) {}
};

/**
 * @brief Log formatter interface
 */
class ILogFormatter {
public:
    virtual ~ILogFormatter() = default;
    
    /**
     * @brief Format a log entry
     * @param entry Log entry to format
     * @return Formatted log message
     */
    virtual std::string format(const LogEntry& entry) const = 0;
    
    /**
     * @brief Get formatter name
     * @return Formatter name
     */
    virtual std::string name() const = 0;
};

/**
 * @brief Log output handler interface
 */
class ILogOutputHandler {
public:
    virtual ~ILogOutputHandler() = default;
    
    /**
     * @brief Write formatted log message
     * @param formatted_message Formatted log message
     * @param entry Original log entry
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> 
    write(const std::string& formatted_message, const LogEntry& entry) = 0;
    
    /**
     * @brief Flush any buffered output
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> flush() = 0;
    
    /**
     * @brief Get handler name
     * @return Handler name
     */
    virtual std::string name() const = 0;
    
    /**
     * @brief Check if handler is available
     * @return true if handler can be used
     */
    virtual bool is_available() const = 0;
};

/**
 * @brief Log filter interface
 */
class ILogFilter {
public:
    virtual ~ILogFilter() = default;
    
    /**
     * @brief Check if log entry should be processed
     * @param entry Log entry to check
     * @return true if entry should be processed
     */
    virtual bool should_log(const LogEntry& entry) const = 0;
    
    /**
     * @brief Get filter name
     * @return Filter name
     */
    virtual std::string name() const = 0;
};

/**
 * @brief Logging configuration
 */
struct LoggingConfiguration {
    LogLevel global_level = LogLevel::Info;
    std::unordered_map<std::string, LogLevel> category_levels;
    std::unordered_map<std::string, LogLevel> plugin_levels;
    std::vector<LogOutput> enabled_outputs;
    std::string log_file_path;
    size_t max_file_size = 10 * 1024 * 1024; // 10MB
    size_t max_backup_files = 5;
    bool auto_flush = true;
    bool include_context = true;
    bool include_source_location = false;
    std::string date_format = "yyyy-MM-dd hh:mm:ss.zzz";
};

/**
 * @brief Enhanced logging manager interface
 * 
 * Provides comprehensive logging functionality with multiple outputs,
 * structured logging, filtering, and per-plugin log level management.
 */
class ILoggingManager {
public:
    virtual ~ILoggingManager() = default;
    
    // === Basic Logging ===
    
    /**
     * @brief Log a message
     * @param level Log level
     * @param category Log category
     * @param message Log message
     * @param plugin_id Plugin ID (optional)
     * @param context Additional context (optional)
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    log(LogLevel level, std::string_view category, std::string_view message,
        std::string_view plugin_id = {}, const QJsonObject& context = {}) = 0;
    
    /**
     * @brief Log with source location
     * @param level Log level
     * @param category Log category
     * @param message Log message
     * @param file Source file
     * @param line Source line
     * @param function Source function
     * @param plugin_id Plugin ID (optional)
     * @param context Additional context (optional)
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    log_with_location(LogLevel level, std::string_view category, std::string_view message,
                     std::string_view file, int line, std::string_view function,
                     std::string_view plugin_id = {}, const QJsonObject& context = {}) = 0;
    
    // === Convenience Methods ===
    
    virtual qtplugin::expected<void, PluginError> trace(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) = 0;
    virtual qtplugin::expected<void, PluginError> debug(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) = 0;
    virtual qtplugin::expected<void, PluginError> info(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) = 0;
    virtual qtplugin::expected<void, PluginError> warning(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) = 0;
    virtual qtplugin::expected<void, PluginError> error(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) = 0;
    virtual qtplugin::expected<void, PluginError> critical(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) = 0;
    virtual qtplugin::expected<void, PluginError> fatal(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) = 0;
    
    // === Configuration ===
    
    /**
     * @brief Set logging configuration
     * @param config Logging configuration
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    set_configuration(const LoggingConfiguration& config) = 0;
    
    /**
     * @brief Get current logging configuration
     * @return Current configuration
     */
    virtual LoggingConfiguration get_configuration() const = 0;
    
    /**
     * @brief Set global log level
     * @param level Global log level
     */
    virtual void set_global_level(LogLevel level) = 0;
    
    /**
     * @brief Set log level for category
     * @param category Category name
     * @param level Log level
     */
    virtual void set_category_level(std::string_view category, LogLevel level) = 0;
    
    /**
     * @brief Set log level for plugin
     * @param plugin_id Plugin ID
     * @param level Log level
     */
    virtual void set_plugin_level(std::string_view plugin_id, LogLevel level) = 0;
    
    /**
     * @brief Get effective log level for category and plugin
     * @param category Category name
     * @param plugin_id Plugin ID
     * @return Effective log level
     */
    virtual LogLevel get_effective_level(std::string_view category, std::string_view plugin_id = {}) const = 0;
    
    // === Output Management ===
    
    /**
     * @brief Add log output handler
     * @param output_type Output type
     * @param handler Output handler
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    add_output_handler(LogOutput output_type, std::unique_ptr<ILogOutputHandler> handler) = 0;
    
    /**
     * @brief Remove log output handler
     * @param output_type Output type
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    remove_output_handler(LogOutput output_type) = 0;
    
    /**
     * @brief Set log formatter
     * @param formatter Log formatter
     */
    virtual void set_formatter(std::unique_ptr<ILogFormatter> formatter) = 0;
    
    /**
     * @brief Add log filter
     * @param filter Log filter
     * @return Filter ID for removal
     */
    virtual std::string add_filter(std::unique_ptr<ILogFilter> filter) = 0;
    
    /**
     * @brief Remove log filter
     * @param filter_id Filter ID
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    remove_filter(const std::string& filter_id) = 0;
    
    // === Utility ===
    
    /**
     * @brief Flush all output handlers
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> flush_all() = 0;
    
    /**
     * @brief Get logging statistics
     * @return Statistics as JSON object
     */
    virtual QJsonObject get_statistics() const = 0;
    
    /**
     * @brief Get recent log entries
     * @param count Maximum number of entries
     * @param level_filter Minimum log level (optional)
     * @return Recent log entries
     */
    virtual std::vector<LogEntry> get_recent_entries(size_t count, std::optional<LogLevel> level_filter = std::nullopt) const = 0;
};

// === Utility Functions ===

/**
 * @brief Convert log level to string
 * @param level Log level
 * @return String representation
 */
std::string log_level_to_string(LogLevel level);

/**
 * @brief Convert string to log level
 * @param str String representation
 * @return Log level or nullopt if invalid
 */
std::optional<LogLevel> string_to_log_level(std::string_view str);

/**
 * @brief Convert log output to string
 * @param output Log output
 * @return String representation
 */
std::string log_output_to_string(LogOutput output);

} // namespace qtplugin

// === Logging Macros ===

#define QTPLUGIN_LOG_MSG(manager, level, category, message, plugin_id, context) \
    (manager).log_with_location((level), (category), (message), __FILE__, __LINE__, __FUNCTION__, (plugin_id), (context))

#define QTPLUGIN_LOG_TRACE(manager, category, message, plugin_id, context) \
    QTPLUGIN_LOG_MSG(manager, qtplugin::LogLevel::Trace, category, message, plugin_id, context)

#define QTPLUGIN_LOG_DEBUG(manager, category, message, plugin_id, context) \
    QTPLUGIN_LOG_MSG(manager, qtplugin::LogLevel::Debug, category, message, plugin_id, context)

#define QTPLUGIN_LOG_INFO(manager, category, message, plugin_id, context) \
    QTPLUGIN_LOG_MSG(manager, qtplugin::LogLevel::Info, category, message, plugin_id, context)

#define QTPLUGIN_LOG_WARNING(manager, category, message, plugin_id, context) \
    QTPLUGIN_LOG_MSG(manager, qtplugin::LogLevel::Warning, category, message, plugin_id, context)

#define QTPLUGIN_LOG_ERROR(manager, category, message, plugin_id, context) \
    QTPLUGIN_LOG_MSG(manager, qtplugin::LogLevel::Error, category, message, plugin_id, context)

#define QTPLUGIN_LOG_CRITICAL(manager, category, message, plugin_id, context) \
    QTPLUGIN_LOG_MSG(manager, qtplugin::LogLevel::Critical, category, message, plugin_id, context)

#define QTPLUGIN_LOG_FATAL(manager, category, message, plugin_id, context) \
    QTPLUGIN_LOG_MSG(manager, qtplugin::LogLevel::Fatal, category, message, plugin_id, context)
