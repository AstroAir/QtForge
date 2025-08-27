/**
 * @file configuration_watcher.hpp
 * @brief Configuration watcher interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include "../../utils/error_handling.hpp"
#include "../configuration_manager.hpp"

namespace qtplugin {

// Forward declarations
enum class ConfigurationScope;
struct ConfigurationChangeEvent;

/**
 * @brief Configuration file watch information
 */
struct ConfigurationWatch {
    std::filesystem::path file_path;
    ConfigurationScope scope;
    std::string plugin_id;
    bool auto_reload = true;
    std::chrono::system_clock::time_point last_modified;
};

/**
 * @brief Interface for configuration file watching and hot reload
 *
 * The configuration watcher monitors configuration files for changes
 * and provides hot reload functionality.
 */
class IConfigurationWatcher {
public:
    virtual ~IConfigurationWatcher() = default;

    /**
     * @brief Start watching configuration file
     * @param file_path Path to configuration file
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     * @param auto_reload Whether to automatically reload on changes
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> watch_file(
        const std::filesystem::path& file_path, ConfigurationScope scope,
        std::string_view plugin_id = {}, bool auto_reload = true) = 0;

    /**
     * @brief Stop watching configuration file
     * @param file_path Path to configuration file
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> unwatch_file(
        const std::filesystem::path& file_path) = 0;

    /**
     * @brief Check if file is being watched
     * @param file_path Path to configuration file
     * @return true if file is being watched
     */
    virtual bool is_watching(const std::filesystem::path& file_path) const = 0;

    /**
     * @brief Get all watched files
     * @return List of watched file paths
     */
    virtual std::vector<std::filesystem::path> get_watched_files() const = 0;

    /**
     * @brief Set auto-reload for watched file
     * @param file_path Path to configuration file
     * @param auto_reload Whether to automatically reload
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> set_auto_reload(
        const std::filesystem::path& file_path, bool auto_reload) = 0;

    /**
     * @brief Manually reload configuration file
     * @param file_path Path to configuration file
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> reload_file(
        const std::filesystem::path& file_path) = 0;

    /**
     * @brief Clear all watches
     */
    virtual void clear_watches() = 0;

    /**
     * @brief Set file change callback
     * @param callback Callback function for file changes
     */
    virtual void set_change_callback(
        std::function<void(const std::filesystem::path&, ConfigurationScope,
                           const std::string&)>
            callback) = 0;
};

/**
 * @brief Configuration watcher implementation
 *
 * Monitors configuration files for changes and provides hot reload
 * functionality using Qt's file system watcher.
 */
class ConfigurationWatcher : public QObject, public IConfigurationWatcher {
    Q_OBJECT

public:
    explicit ConfigurationWatcher(QObject* parent = nullptr);
    ~ConfigurationWatcher() override;

    // IConfigurationWatcher interface
    qtplugin::expected<void, PluginError> watch_file(
        const std::filesystem::path& file_path, ConfigurationScope scope,
        std::string_view plugin_id = {}, bool auto_reload = true) override;

    qtplugin::expected<void, PluginError> unwatch_file(
        const std::filesystem::path& file_path) override;

    bool is_watching(const std::filesystem::path& file_path) const override;

    std::vector<std::filesystem::path> get_watched_files() const override;

    qtplugin::expected<void, PluginError> set_auto_reload(
        const std::filesystem::path& file_path, bool auto_reload) override;

    qtplugin::expected<void, PluginError> reload_file(
        const std::filesystem::path& file_path) override;

    void clear_watches() override;

    void set_change_callback(
        std::function<void(const std::filesystem::path&, ConfigurationScope,
                           const std::string&)>
            callback) override;

signals:
    /**
     * @brief Emitted when configuration file changes
     * @param file_path Path to changed file
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     */
    void file_changed(const QString& file_path, int scope,
                      const QString& plugin_id);

    /**
     * @brief Emitted when configuration is reloaded
     * @param file_path Path to reloaded file
     * @param success Whether reload was successful
     */
    void file_reloaded(const QString& file_path, bool success);

    /**
     * @brief Emitted when watch error occurs
     * @param file_path Path where error occurred
     * @param error Error message
     */
    void watch_error(const QString& file_path, const QString& error);

private slots:
    void on_file_changed(const QString& path);
    void on_directory_changed(const QString& path);

private:
    std::unique_ptr<QFileSystemWatcher> m_file_watcher;
    std::unordered_map<std::string, std::unique_ptr<ConfigurationWatch>>
        m_watches;
    std::unordered_set<std::string> m_watched_directories;
    std::function<void(const std::filesystem::path&, ConfigurationScope,
                       const std::string&)>
        m_change_callback;

    // Helper methods
    std::string path_to_string(const std::filesystem::path& path) const;
    std::filesystem::path string_to_path(const std::string& path_str) const;
    void ensure_directory_watched(const std::filesystem::path& file_path);
    bool is_file_modified(
        const std::filesystem::path& file_path,
        const std::chrono::system_clock::time_point& last_known) const;
    std::chrono::system_clock::time_point get_file_modification_time(
        const std::filesystem::path& file_path) const;
};

}  // namespace qtplugin
