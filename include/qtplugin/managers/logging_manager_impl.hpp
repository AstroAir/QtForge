/**
 * @file logging_manager_impl.hpp
 * @brief Concrete implementation of enhanced logging management system
 * @version 3.0.0
 */

#pragma once

#include "logging_manager.hpp"
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <deque>
#include <thread>
#include <condition_variable>
#include <unordered_set>
#include <fstream>

namespace qtplugin {

// === Built-in Formatters ===

/**
 * @brief Simple text formatter
 */
class SimpleLogFormatter : public ILogFormatter {
public:
    std::string format(const LogEntry& entry) const override;
    std::string name() const override { return "simple"; }
};

/**
 * @brief JSON formatter
 */
class JsonLogFormatter : public ILogFormatter {
public:
    std::string format(const LogEntry& entry) const override;
    std::string name() const override { return "json"; }
};

/**
 * @brief Detailed formatter with context
 */
class DetailedLogFormatter : public ILogFormatter {
public:
    std::string format(const LogEntry& entry) const override;
    std::string name() const override { return "detailed"; }
};

// === Built-in Output Handlers ===

/**
 * @brief Console output handler
 */
class ConsoleOutputHandler : public ILogOutputHandler {
public:
    qtplugin::expected<void, PluginError> 
    write(const std::string& formatted_message, const LogEntry& entry) override;
    
    qtplugin::expected<void, PluginError> flush() override;
    std::string name() const override { return "console"; }
    bool is_available() const override { return true; }
};

/**
 * @brief File output handler with rotation
 */
class FileOutputHandler : public ILogOutputHandler {
public:
    explicit FileOutputHandler(const std::filesystem::path& file_path, 
                              size_t max_size = 10 * 1024 * 1024, 
                              size_t max_backups = 5);
    ~FileOutputHandler();
    
    qtplugin::expected<void, PluginError> 
    write(const std::string& formatted_message, const LogEntry& entry) override;
    
    qtplugin::expected<void, PluginError> flush() override;
    std::string name() const override { return "file"; }
    bool is_available() const override;
    
    void set_max_size(size_t max_size) { m_max_size = max_size; }
    void set_max_backups(size_t max_backups) { m_max_backups = max_backups; }

private:
    std::filesystem::path m_file_path;
    size_t m_max_size;
    size_t m_max_backups;
    size_t m_current_size = 0;
    std::unique_ptr<std::ofstream> m_file_stream;
    mutable std::mutex m_mutex;
    
    void rotate_file();
    bool open_file();
};

// === Built-in Filters ===

/**
 * @brief Level-based filter
 */
class LevelLogFilter : public ILogFilter {
public:
    explicit LevelLogFilter(LogLevel min_level) : m_min_level(min_level) {}
    
    bool should_log(const LogEntry& entry) const override {
        return entry.level >= m_min_level;
    }
    
    std::string name() const override { return "level"; }
    
    void set_min_level(LogLevel level) { m_min_level = level; }
    LogLevel get_min_level() const { return m_min_level; }

private:
    LogLevel m_min_level;
};

/**
 * @brief Category-based filter
 */
class CategoryLogFilter : public ILogFilter {
public:
    explicit CategoryLogFilter(const std::vector<std::string>& allowed_categories)
        : m_allowed_categories(allowed_categories.begin(), allowed_categories.end()) {}
    
    bool should_log(const LogEntry& entry) const override {
        return m_allowed_categories.empty() || 
               m_allowed_categories.find(entry.category) != m_allowed_categories.end();
    }
    
    std::string name() const override { return "category"; }
    
    void add_category(const std::string& category) { m_allowed_categories.insert(category); }
    void remove_category(const std::string& category) { m_allowed_categories.erase(category); }
    void clear_categories() { m_allowed_categories.clear(); }

private:
    std::unordered_set<std::string> m_allowed_categories;
};

/**
 * @brief Plugin-based filter
 */
class PluginLogFilter : public ILogFilter {
public:
    explicit PluginLogFilter(const std::vector<std::string>& allowed_plugins)
        : m_allowed_plugins(allowed_plugins.begin(), allowed_plugins.end()) {}
    
    bool should_log(const LogEntry& entry) const override {
        return m_allowed_plugins.empty() || 
               m_allowed_plugins.find(entry.plugin_id) != m_allowed_plugins.end();
    }
    
    std::string name() const override { return "plugin"; }
    
    void add_plugin(const std::string& plugin_id) { m_allowed_plugins.insert(plugin_id); }
    void remove_plugin(const std::string& plugin_id) { m_allowed_plugins.erase(plugin_id); }
    void clear_plugins() { m_allowed_plugins.clear(); }

private:
    std::unordered_set<std::string> m_allowed_plugins;
};

// === Main Implementation ===

/**
 * @brief Default logging manager implementation
 */
class LoggingManager : public QObject, public ILoggingManager {
    Q_OBJECT

public:
    explicit LoggingManager(QObject* parent = nullptr);
    ~LoggingManager() override;

    // ILoggingManager implementation
    qtplugin::expected<void, PluginError>
    log(LogLevel level, std::string_view category, std::string_view message,
        std::string_view plugin_id = {}, const QJsonObject& context = {}) override;

    qtplugin::expected<void, PluginError>
    log_with_location(LogLevel level, std::string_view category, std::string_view message,
                     std::string_view file, int line, std::string_view function,
                     std::string_view plugin_id = {}, const QJsonObject& context = {}) override;

    // Convenience methods
    qtplugin::expected<void, PluginError> trace(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) override;
    qtplugin::expected<void, PluginError> debug(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) override;
    qtplugin::expected<void, PluginError> info(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) override;
    qtplugin::expected<void, PluginError> warning(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) override;
    qtplugin::expected<void, PluginError> error(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) override;
    qtplugin::expected<void, PluginError> critical(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) override;
    qtplugin::expected<void, PluginError> fatal(std::string_view category, std::string_view message, std::string_view plugin_id = {}, const QJsonObject& context = {}) override;

    // Configuration
    qtplugin::expected<void, PluginError>
    set_configuration(const LoggingConfiguration& config) override;
    
    LoggingConfiguration get_configuration() const override;
    void set_global_level(LogLevel level) override;
    void set_category_level(std::string_view category, LogLevel level) override;
    void set_plugin_level(std::string_view plugin_id, LogLevel level) override;
    LogLevel get_effective_level(std::string_view category, std::string_view plugin_id = {}) const override;

    // Output management
    qtplugin::expected<void, PluginError>
    add_output_handler(LogOutput output_type, std::unique_ptr<ILogOutputHandler> handler) override;
    
    qtplugin::expected<void, PluginError>
    remove_output_handler(LogOutput output_type) override;
    
    void set_formatter(std::unique_ptr<ILogFormatter> formatter) override;
    std::string add_filter(std::unique_ptr<ILogFilter> filter) override;
    
    qtplugin::expected<void, PluginError>
    remove_filter(const std::string& filter_id) override;

    // Utility
    qtplugin::expected<void, PluginError> flush_all() override;
    QJsonObject get_statistics() const override;
    std::vector<LogEntry> get_recent_entries(size_t count, std::optional<LogLevel> level_filter = std::nullopt) const override;

signals:
    void log_entry_added(const LogEntry& entry);
    void configuration_changed();
    void output_handler_added(LogOutput output_type);
    void output_handler_removed(LogOutput output_type);

private slots:
    void process_log_queue();

private:
    // Configuration
    LoggingConfiguration m_config;
    mutable std::shared_mutex m_config_mutex;

    // Output handling
    std::unordered_map<LogOutput, std::unique_ptr<ILogOutputHandler>> m_output_handlers;
    std::unique_ptr<ILogFormatter> m_formatter;
    std::unordered_map<std::string, std::unique_ptr<ILogFilter>> m_filters;
    mutable std::shared_mutex m_handlers_mutex;

    // Log processing
    std::deque<LogEntry> m_log_queue;
    std::deque<LogEntry> m_recent_entries;
    mutable std::mutex m_queue_mutex;
    std::condition_variable m_queue_condition;
    std::unique_ptr<QTimer> m_process_timer;
    
    // Statistics
    std::atomic<size_t> m_total_entries{0};
    std::atomic<size_t> m_dropped_entries{0};
    std::unordered_map<LogLevel, std::atomic<size_t>> m_level_counts;
    
    // Settings
    static constexpr size_t MAX_QUEUE_SIZE = 10000;
    static constexpr size_t MAX_RECENT_ENTRIES = 1000;
    
    // Helper methods
    bool should_log_entry(const LogEntry& entry) const;
    void process_single_entry(const LogEntry& entry);
    std::string generate_filter_id() const;
    void update_statistics(const LogEntry& entry);
    void add_to_recent_entries(const LogEntry& entry);
    std::string get_thread_id() const;
};

/**
 * @brief Create a default logging manager instance
 * @param parent Parent QObject
 * @return Unique pointer to logging manager
 */
std::unique_ptr<ILoggingManager> create_logging_manager(QObject* parent = nullptr);

} // namespace qtplugin
