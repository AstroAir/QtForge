/**
 * @file plugin_hot_reload_manager.hpp
 * @brief Plugin hot reload manager interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <qtplugin/utils/error_handling.hpp>
#include <memory>
#include <string>
#include <filesystem>
#include <functional>
#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>

namespace qtplugin {

// Forward declarations
class IPluginRegistry;
class IPluginLoader;

/**
 * @brief Interface for plugin hot reload management
 * 
 * The hot reload manager handles file system monitoring and automatic
 * plugin reloading when source files change.
 */
class IPluginHotReloadManager {
public:
    virtual ~IPluginHotReloadManager() = default;
    
    /**
     * @brief Enable hot reload for a plugin
     * @param plugin_id Plugin identifier
     * @param file_path Path to the plugin file to watch
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> 
    enable_hot_reload(const std::string& plugin_id, const std::filesystem::path& file_path) = 0;
    
    /**
     * @brief Disable hot reload for a plugin
     * @param plugin_id Plugin identifier
     */
    virtual void disable_hot_reload(const std::string& plugin_id) = 0;
    
    /**
     * @brief Check if hot reload is enabled for a plugin
     * @param plugin_id Plugin identifier
     * @return true if hot reload is enabled
     */
    virtual bool is_hot_reload_enabled(const std::string& plugin_id) const = 0;
    
    /**
     * @brief Set reload callback function
     * @param callback Function to call when plugin needs reloading
     */
    virtual void set_reload_callback(std::function<void(const std::string&)> callback) = 0;
    
    /**
     * @brief Get list of plugins with hot reload enabled
     * @return Vector of plugin IDs
     */
    virtual std::vector<std::string> get_hot_reload_plugins() const = 0;
    
    /**
     * @brief Clear all hot reload watchers
     */
    virtual void clear() = 0;
    
    /**
     * @brief Enable/disable hot reload globally
     * @param enabled Global hot reload state
     */
    virtual void set_global_hot_reload_enabled(bool enabled) = 0;
    
    /**
     * @brief Check if global hot reload is enabled
     * @return true if globally enabled
     */
    virtual bool is_global_hot_reload_enabled() const = 0;
};

/**
 * @brief Plugin hot reload manager implementation
 * 
 * Monitors plugin files for changes and triggers automatic reloading.
 * Uses QFileSystemWatcher for efficient file monitoring.
 */
class PluginHotReloadManager : public QObject, public IPluginHotReloadManager {
    Q_OBJECT
    
public:
    explicit PluginHotReloadManager(QObject* parent = nullptr);
    ~PluginHotReloadManager() override;
    
    // IPluginHotReloadManager interface
    qtplugin::expected<void, PluginError> 
    enable_hot_reload(const std::string& plugin_id, const std::filesystem::path& file_path) override;
    
    void disable_hot_reload(const std::string& plugin_id) override;
    bool is_hot_reload_enabled(const std::string& plugin_id) const override;
    void set_reload_callback(std::function<void(const std::string&)> callback) override;
    std::vector<std::string> get_hot_reload_plugins() const override;
    void clear() override;
    void set_global_hot_reload_enabled(bool enabled) override;
    bool is_global_hot_reload_enabled() const override;

signals:
    /**
     * @brief Emitted when a plugin file changes and needs reloading
     * @param plugin_id Plugin identifier
     * @param file_path Path to the changed file
     */
    void plugin_file_changed(const QString& plugin_id, const QString& file_path);
    
    /**
     * @brief Emitted when hot reload is enabled for a plugin
     * @param plugin_id Plugin identifier
     */
    void hot_reload_enabled(const QString& plugin_id);
    
    /**
     * @brief Emitted when hot reload is disabled for a plugin
     * @param plugin_id Plugin identifier
     */
    void hot_reload_disabled(const QString& plugin_id);

private slots:
    void on_file_changed(const QString& path);

private:
    std::unique_ptr<QFileSystemWatcher> m_file_watcher;
    std::unordered_map<std::string, std::filesystem::path> m_watched_files;
    std::function<void(const std::string&)> m_reload_callback;
    bool m_global_enabled = true;
    
    // Helper methods
    std::string find_plugin_by_path(const std::filesystem::path& file_path) const;
};

} // namespace qtplugin
