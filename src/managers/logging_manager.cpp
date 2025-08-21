/**
 * @file logging_manager.cpp
 * @brief Implementation of enhanced logging management system
 * @version 3.0.0
 */

#include "qtplugin/managers/logging_manager_impl.hpp"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>
#include <QLoggingCategory>
#include <QThread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>

Q_LOGGING_CATEGORY(loggingLog, "qtplugin.logging")

namespace qtplugin {

// === Utility Functions ===

std::string log_level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        case LogLevel::Fatal: return "FATAL";
    }
    return "UNKNOWN";
}

std::optional<LogLevel> string_to_log_level(std::string_view str) {
    if (str == "TRACE" || str == "trace") return LogLevel::Trace;
    if (str == "DEBUG" || str == "debug") return LogLevel::Debug;
    if (str == "INFO" || str == "info") return LogLevel::Info;
    if (str == "WARNING" || str == "warning" || str == "WARN" || str == "warn") return LogLevel::Warning;
    if (str == "ERROR" || str == "error") return LogLevel::Error;
    if (str == "CRITICAL" || str == "critical") return LogLevel::Critical;
    if (str == "FATAL" || str == "fatal") return LogLevel::Fatal;
    return std::nullopt;
}

std::string log_output_to_string(LogOutput output) {
    switch (output) {
        case LogOutput::Console: return "console";
        case LogOutput::File: return "file";
        case LogOutput::Network: return "network";
        case LogOutput::Database: return "database";
        case LogOutput::Custom: return "custom";
    }
    return "unknown";
}

// === Formatters Implementation ===

std::string SimpleLogFormatter::format(const LogEntry& entry) const {
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    oss << " [" << log_level_to_string(entry.level) << "]";
    oss << " [" << entry.category << "]";
    if (!entry.plugin_id.empty()) {
        oss << " [" << entry.plugin_id << "]";
    }
    oss << " " << entry.message;
    
    return oss.str();
}

std::string JsonLogFormatter::format(const LogEntry& entry) const {
    QJsonObject json;
    
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    json["timestamp"] = QDateTime::fromSecsSinceEpoch(time_t).toString(Qt::ISODate);
    json["level"] = QString::fromStdString(log_level_to_string(entry.level));
    json["category"] = QString::fromStdString(entry.category);
    json["message"] = QString::fromStdString(entry.message);
    
    if (!entry.plugin_id.empty()) {
        json["plugin_id"] = QString::fromStdString(entry.plugin_id);
    }
    
    if (!entry.thread_id.empty()) {
        json["thread_id"] = QString::fromStdString(entry.thread_id);
    }
    
    if (!entry.file.empty()) {
        json["file"] = QString::fromStdString(entry.file);
        json["line"] = entry.line;
        json["function"] = QString::fromStdString(entry.function);
    }
    
    if (!entry.context.isEmpty()) {
        json["context"] = entry.context;
    }
    
    return QJsonDocument(json).toJson(QJsonDocument::Compact).toStdString();
}

std::string DetailedLogFormatter::format(const LogEntry& entry) const {
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    oss << " [" << std::setw(8) << std::left << log_level_to_string(entry.level) << "]";
    oss << " [" << std::setw(15) << std::left << entry.category << "]";
    
    if (!entry.plugin_id.empty()) {
        oss << " [" << std::setw(20) << std::left << entry.plugin_id << "]";
    }
    
    if (!entry.thread_id.empty()) {
        oss << " [" << entry.thread_id << "]";
    }
    
    oss << " " << entry.message;
    
    if (!entry.file.empty()) {
        oss << " (" << entry.file << ":" << entry.line << " in " << entry.function << ")";
    }
    
    if (!entry.context.isEmpty()) {
        oss << " Context: " << QJsonDocument(entry.context).toJson(QJsonDocument::Compact).toStdString();
    }
    
    return oss.str();
}

// === Output Handlers Implementation ===

qtplugin::expected<void, PluginError> 
ConsoleOutputHandler::write(const std::string& formatted_message, const LogEntry& entry) {
    // Write to stderr for warnings and errors, stdout for others
    if (entry.level >= LogLevel::Warning) {
        std::cerr << formatted_message << std::endl;
    } else {
        std::cout << formatted_message << std::endl;
    }
    return make_success();
}

qtplugin::expected<void, PluginError> ConsoleOutputHandler::flush() {
    std::cout.flush();
    std::cerr.flush();
    return make_success();
}

FileOutputHandler::FileOutputHandler(const std::filesystem::path& file_path, 
                                   size_t max_size, size_t max_backups)
    : m_file_path(file_path)
    , m_max_size(max_size)
    , m_max_backups(max_backups) {
    open_file();
}

FileOutputHandler::~FileOutputHandler() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file_stream) {
        m_file_stream->flush();
        m_file_stream->close();
    }
}

qtplugin::expected<void, PluginError>
FileOutputHandler::write(const std::string& formatted_message, const LogEntry& entry) {
    Q_UNUSED(entry) // Entry is used for potential future filtering
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_file_stream || !m_file_stream->is_open()) {
        if (!open_file()) {
            return make_error<void>(PluginErrorCode::FileSystemError, 
                                   "Failed to open log file: " + m_file_path.string());
        }
    }
    
    *m_file_stream << formatted_message << std::endl;
    m_current_size += formatted_message.length() + 1; // +1 for newline
    
    // Check if rotation is needed
    if (m_current_size >= m_max_size) {
        rotate_file();
    }
    
    return make_success();
}

qtplugin::expected<void, PluginError> FileOutputHandler::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file_stream) {
        m_file_stream->flush();
    }
    return make_success();
}

bool FileOutputHandler::is_available() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_file_stream && m_file_stream->is_open();
}

void FileOutputHandler::rotate_file() {
    if (m_file_stream) {
        m_file_stream->close();
    }
    
    // Rotate existing backup files
    for (size_t i = m_max_backups; i > 1; --i) {
        std::filesystem::path old_backup = m_file_path.string() + "." + std::to_string(i - 1);
        std::filesystem::path new_backup = m_file_path.string() + "." + std::to_string(i);
        
        if (std::filesystem::exists(old_backup)) {
            std::error_code ec;
            std::filesystem::rename(old_backup, new_backup, ec);
        }
    }
    
    // Move current file to .1
    if (std::filesystem::exists(m_file_path)) {
        std::filesystem::path backup = m_file_path.string() + ".1";
        std::error_code ec;
        std::filesystem::rename(m_file_path, backup, ec);
    }
    
    // Open new file
    open_file();
}

bool FileOutputHandler::open_file() {
    // Create directory if it doesn't exist
    std::filesystem::path dir = m_file_path.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        if (ec) {
            return false;
        }
    }
    
    m_file_stream = std::make_unique<std::ofstream>(m_file_path, std::ios::app);
    m_current_size = std::filesystem::exists(m_file_path) ? std::filesystem::file_size(m_file_path) : 0;
    
    return m_file_stream->is_open();
}

// === LoggingManager Implementation ===

LoggingManager::LoggingManager(QObject* parent)
    : QObject(parent)
    , m_formatter(std::make_unique<SimpleLogFormatter>())
    , m_process_timer(std::make_unique<QTimer>(this)) {
    
    // Initialize level counters
    for (int i = 0; i <= static_cast<int>(LogLevel::Fatal); ++i) {
        m_level_counts[static_cast<LogLevel>(i)] = 0;
    }
    
    // Set up processing timer
    m_process_timer->setSingleShot(false);
    m_process_timer->setInterval(100); // Process every 100ms
    connect(m_process_timer.get(), &QTimer::timeout, this, &LoggingManager::process_log_queue);
    m_process_timer->start();
    
    // Add default console output
    m_output_handlers[LogOutput::Console] = std::make_unique<ConsoleOutputHandler>();
    
    qCDebug(loggingLog) << "Logging manager initialized";
}

LoggingManager::~LoggingManager() {
    // Process remaining entries
    process_log_queue();
    
    // Flush all handlers
    flush_all();
    
    qCDebug(loggingLog) << "Logging manager destroyed";
}

qtplugin::expected<void, PluginError>
LoggingManager::log(LogLevel level, std::string_view category, std::string_view message,
                   std::string_view plugin_id, const QJsonObject& context) {
    return log_with_location(level, category, message, "", 0, "", plugin_id, context);
}

qtplugin::expected<void, PluginError>
LoggingManager::log_with_location(LogLevel level, std::string_view category, std::string_view message,
                                 std::string_view file, int line, std::string_view function,
                                 std::string_view plugin_id, const QJsonObject& context) {
    // Create log entry
    LogEntry entry(level, category, plugin_id, message, context);
    entry.file = file;
    entry.line = line;
    entry.function = function;
    entry.thread_id = get_thread_id();

    // Check if we should log this entry
    if (!should_log_entry(entry)) {
        return make_success();
    }

    // Add to queue for processing
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);

        // Drop entries if queue is full
        if (m_log_queue.size() >= MAX_QUEUE_SIZE) {
            m_log_queue.pop_front();
            m_dropped_entries.fetch_add(1);
        }

        m_log_queue.push_back(entry);
    }

    m_queue_condition.notify_one();

    return make_success();
}

// Convenience methods
qtplugin::expected<void, PluginError> LoggingManager::trace(std::string_view category, std::string_view message, std::string_view plugin_id, const QJsonObject& context) {
    return log(LogLevel::Trace, category, message, plugin_id, context);
}

qtplugin::expected<void, PluginError> LoggingManager::debug(std::string_view category, std::string_view message, std::string_view plugin_id, const QJsonObject& context) {
    return log(LogLevel::Debug, category, message, plugin_id, context);
}

qtplugin::expected<void, PluginError> LoggingManager::info(std::string_view category, std::string_view message, std::string_view plugin_id, const QJsonObject& context) {
    return log(LogLevel::Info, category, message, plugin_id, context);
}

qtplugin::expected<void, PluginError> LoggingManager::warning(std::string_view category, std::string_view message, std::string_view plugin_id, const QJsonObject& context) {
    return log(LogLevel::Warning, category, message, plugin_id, context);
}

qtplugin::expected<void, PluginError> LoggingManager::error(std::string_view category, std::string_view message, std::string_view plugin_id, const QJsonObject& context) {
    return log(LogLevel::Error, category, message, plugin_id, context);
}

qtplugin::expected<void, PluginError> LoggingManager::critical(std::string_view category, std::string_view message, std::string_view plugin_id, const QJsonObject& context) {
    return log(LogLevel::Critical, category, message, plugin_id, context);
}

qtplugin::expected<void, PluginError> LoggingManager::fatal(std::string_view category, std::string_view message, std::string_view plugin_id, const QJsonObject& context) {
    return log(LogLevel::Fatal, category, message, plugin_id, context);
}

qtplugin::expected<void, PluginError>
LoggingManager::set_configuration(const LoggingConfiguration& config) {
    {
        std::unique_lock<std::shared_mutex> lock(m_config_mutex);
        m_config = config;
    }

    emit configuration_changed();
    return make_success();
}

LoggingConfiguration LoggingManager::get_configuration() const {
    std::shared_lock<std::shared_mutex> lock(m_config_mutex);
    return m_config;
}

void LoggingManager::set_global_level(LogLevel level) {
    std::unique_lock<std::shared_mutex> lock(m_config_mutex);
    m_config.global_level = level;
}

void LoggingManager::set_category_level(std::string_view category, LogLevel level) {
    std::unique_lock<std::shared_mutex> lock(m_config_mutex);
    m_config.category_levels[std::string(category)] = level;
}

void LoggingManager::set_plugin_level(std::string_view plugin_id, LogLevel level) {
    std::unique_lock<std::shared_mutex> lock(m_config_mutex);
    m_config.plugin_levels[std::string(plugin_id)] = level;
}

LogLevel LoggingManager::get_effective_level(std::string_view category, std::string_view plugin_id) const {
    std::shared_lock<std::shared_mutex> lock(m_config_mutex);

    // Check plugin-specific level first
    if (!plugin_id.empty()) {
        auto plugin_it = m_config.plugin_levels.find(std::string(plugin_id));
        if (plugin_it != m_config.plugin_levels.end()) {
            return plugin_it->second;
        }
    }

    // Check category-specific level
    auto category_it = m_config.category_levels.find(std::string(category));
    if (category_it != m_config.category_levels.end()) {
        return category_it->second;
    }

    // Return global level
    return m_config.global_level;
}

qtplugin::expected<void, PluginError>
LoggingManager::add_output_handler(LogOutput output_type, std::unique_ptr<ILogOutputHandler> handler) {
    if (!handler) {
        return make_error<void>(PluginErrorCode::InvalidArgument, "Output handler cannot be null");
    }

    if (!handler->is_available()) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                               "Output handler is not available: " + handler->name());
    }

    {
        std::unique_lock<std::shared_mutex> lock(m_handlers_mutex);
        m_output_handlers[output_type] = std::move(handler);
    }

    emit output_handler_added(output_type);
    return make_success();
}

qtplugin::expected<void, PluginError>
LoggingManager::remove_output_handler(LogOutput output_type) {
    {
        std::unique_lock<std::shared_mutex> lock(m_handlers_mutex);
        auto it = m_output_handlers.find(output_type);
        if (it == m_output_handlers.end()) {
            return make_error<void>(PluginErrorCode::NotFound,
                                   "Output handler not found: " + log_output_to_string(output_type));
        }
        m_output_handlers.erase(it);
    }

    emit output_handler_removed(output_type);
    return make_success();
}

void LoggingManager::set_formatter(std::unique_ptr<ILogFormatter> formatter) {
    if (!formatter) {
        formatter = std::make_unique<SimpleLogFormatter>();
    }

    std::unique_lock<std::shared_mutex> lock(m_handlers_mutex);
    m_formatter = std::move(formatter);
}

std::string LoggingManager::add_filter(std::unique_ptr<ILogFilter> filter) {
    if (!filter) {
        return {};
    }

    std::string filter_id = generate_filter_id();

    {
        std::unique_lock<std::shared_mutex> lock(m_handlers_mutex);
        m_filters[filter_id] = std::move(filter);
    }

    return filter_id;
}

qtplugin::expected<void, PluginError>
LoggingManager::remove_filter(const std::string& filter_id) {
    std::unique_lock<std::shared_mutex> lock(m_handlers_mutex);
    auto it = m_filters.find(filter_id);
    if (it == m_filters.end()) {
        return make_error<void>(PluginErrorCode::NotFound, "Filter not found: " + filter_id);
    }

    m_filters.erase(it);
    return make_success();
}

qtplugin::expected<void, PluginError> LoggingManager::flush_all() {
    std::shared_lock<std::shared_mutex> lock(m_handlers_mutex);

    for (auto& [output_type, handler] : m_output_handlers) {
        if (handler) {
            auto result = handler->flush();
            if (!result) {
                qCWarning(loggingLog) << "Failed to flush output handler:"
                                     << QString::fromStdString(handler->name())
                                     << QString::fromStdString(result.error().message);
            }
        }
    }

    return make_success();
}

QJsonObject LoggingManager::get_statistics() const {
    QJsonObject stats;

    stats["total_entries"] = static_cast<qint64>(m_total_entries.load());
    stats["dropped_entries"] = static_cast<qint64>(m_dropped_entries.load());

    // Level counts
    QJsonObject level_counts;
    for (const auto& [level, count] : m_level_counts) {
        level_counts[QString::fromStdString(log_level_to_string(level))] = static_cast<qint64>(count.load());
    }
    stats["level_counts"] = level_counts;

    // Queue status
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        stats["queue_size"] = static_cast<qint64>(m_log_queue.size());
        stats["recent_entries_count"] = static_cast<qint64>(m_recent_entries.size());
    }

    // Configuration
    {
        std::shared_lock<std::shared_mutex> lock(m_config_mutex);
        stats["global_level"] = QString::fromStdString(log_level_to_string(m_config.global_level));
        stats["category_levels_count"] = static_cast<qint64>(m_config.category_levels.size());
        stats["plugin_levels_count"] = static_cast<qint64>(m_config.plugin_levels.size());
    }

    // Output handlers
    {
        std::shared_lock<std::shared_mutex> lock(m_handlers_mutex);
        stats["output_handlers_count"] = static_cast<qint64>(m_output_handlers.size());
        stats["filters_count"] = static_cast<qint64>(m_filters.size());
        stats["formatter"] = QString::fromStdString(m_formatter ? m_formatter->name() : "none");
    }

    return stats;
}

std::vector<LogEntry> LoggingManager::get_recent_entries(size_t count, std::optional<LogLevel> level_filter) const {
    std::lock_guard<std::mutex> lock(m_queue_mutex);

    std::vector<LogEntry> result;
    result.reserve(std::min(count, m_recent_entries.size()));

    auto it = m_recent_entries.rbegin(); // Start from most recent
    while (it != m_recent_entries.rend() && result.size() < count) {
        if (!level_filter || it->level >= level_filter.value()) {
            result.push_back(*it);
        }
        ++it;
    }

    return result;
}

void LoggingManager::process_log_queue() {
    std::vector<LogEntry> entries_to_process;

    // Extract entries from queue
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        if (m_log_queue.empty()) {
            return;
        }

        // Process up to 100 entries at a time
        size_t process_count = std::min(m_log_queue.size(), size_t(100));
        entries_to_process.reserve(process_count);

        for (size_t i = 0; i < process_count; ++i) {
            entries_to_process.push_back(m_log_queue.front());
            m_log_queue.pop_front();
        }
    }

    // Process entries outside of lock
    for (const auto& entry : entries_to_process) {
        process_single_entry(entry);
        update_statistics(entry);
        add_to_recent_entries(entry);
        emit log_entry_added(entry);
    }
}

bool LoggingManager::should_log_entry(const LogEntry& entry) const {
    // Check effective log level
    LogLevel effective_level = get_effective_level(entry.category, entry.plugin_id);
    if (entry.level < effective_level) {
        return false;
    }

    // Check filters
    std::shared_lock<std::shared_mutex> lock(m_handlers_mutex);
    for (const auto& [filter_id, filter] : m_filters) {
        if (filter && !filter->should_log(entry)) {
            return false;
        }
    }

    return true;
}

void LoggingManager::process_single_entry(const LogEntry& entry) {
    std::shared_lock<std::shared_mutex> lock(m_handlers_mutex);

    if (!m_formatter) {
        return;
    }

    std::string formatted_message = m_formatter->format(entry);

    for (auto& [output_type, handler] : m_output_handlers) {
        if (handler && handler->is_available()) {
            auto result = handler->write(formatted_message, entry);
            if (!result) {
                // Log handler error to Qt's logging system
                qCWarning(loggingLog) << "Failed to write to output handler:"
                                     << QString::fromStdString(handler->name())
                                     << QString::fromStdString(result.error().message);
            }
        }
    }

    // Auto-flush if enabled
    if (m_config.auto_flush) {
        for (auto& [output_type, handler] : m_output_handlers) {
            if (handler && handler->is_available()) {
                handler->flush();
            }
        }
    }
}

std::string LoggingManager::generate_filter_id() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::string id;
    id.reserve(16);
    for (int i = 0; i < 16; ++i) {
        id += "0123456789abcdef"[dis(gen)];
    }
    return id;
}

void LoggingManager::update_statistics(const LogEntry& entry) {
    m_total_entries.fetch_add(1);
    m_level_counts[entry.level].fetch_add(1);
}

void LoggingManager::add_to_recent_entries(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(m_queue_mutex);

    m_recent_entries.push_back(entry);

    // Keep only the most recent entries
    while (m_recent_entries.size() > MAX_RECENT_ENTRIES) {
        m_recent_entries.pop_front();
    }
}

std::string LoggingManager::get_thread_id() const {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

// Factory function
std::unique_ptr<ILoggingManager> create_logging_manager(QObject* parent) {
    return std::make_unique<LoggingManager>(parent);
}

} // namespace qtplugin
