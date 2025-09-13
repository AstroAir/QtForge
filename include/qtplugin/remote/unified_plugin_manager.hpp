/**
 * @file unified_plugin_manager.hpp
 * @brief Unified manager combining local and remote plugin capabilities
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <QFuture>
#include <QtConcurrent>
#include <QList>
#include <QString>
#include <QStringList>
#include <memory>
#include "remote_plugin_manager.hpp"

Q_DECLARE_LOGGING_CATEGORY(unifiedPluginLog)

namespace qtplugin {
class PluginManager; // Forward declaration
}

namespace QtPlugin {
namespace Remote {

// Forward declarations
class RemotePluginManager;

/**
 * @brief Plugin load strategies
 */
enum class UnifiedLoadStrategy {
    FavorLocal,     ///< Try local first, fallback to remote
    FavorRemote,    ///< Try remote first, fallback to local
    LocalOnly,      ///< Only load local plugins
    RemoteOnly      ///< Only load remote plugins
};

/**
 * @brief Plugin source information
 */
enum class PluginSource {
    Unknown,        ///< Source unknown
    Local,          ///< Plugin loaded from local file
    Remote          ///< Plugin loaded from remote repository
};

/**
 * @brief Unified plugin manager combining local and remote capabilities
 * 
 * This class provides a unified interface for managing both local and remote plugins.
 * It supports configurable load strategies, automatic updates, and seamless fallback
 * between local and remote sources.
 */
class UnifiedPluginManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit UnifiedPluginManager(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~UnifiedPluginManager();

    // === Load Strategy Configuration ===
    
    /**
     * @brief Set the plugin load strategy
     * @param strategy Load strategy to use
     */
    void setLoadStrategy(UnifiedLoadStrategy strategy);
    
    /**
     * @brief Get current load strategy
     * @return Current load strategy
     */
    UnifiedLoadStrategy loadStrategy() const;

    // === Auto Update Configuration ===
    
    /**
     * @brief Enable or disable automatic updates
     * @param enabled Whether auto updates are enabled
     */
    void setAutoUpdateEnabled(bool enabled);
    
    /**
     * @brief Check if automatic updates are enabled
     * @return true if auto updates are enabled
     */
    bool isAutoUpdateEnabled() const;

    // === Plugin Management ===
    
    /**
     * @brief Load plugin by ID or path
     * @param pluginIdOrPath Plugin identifier or file path
     * @return Future with load result
     */
    QFuture<bool> loadPlugin(const QString& pluginIdOrPath);
    
    /**
     * @brief Unload plugin
     * @param pluginId Plugin identifier
     * @return true if successful
     */
    bool unloadPlugin(const QString& pluginId);
    
    /**
     * @brief Update plugin to latest version
     * @param pluginId Plugin identifier
     * @return Future with update result
     */
    QFuture<bool> updatePlugin(const QString& pluginId);

    // === Plugin Query ===
    
    /**
     * @brief Check if plugin is loaded
     * @param pluginId Plugin identifier
     * @return true if plugin is loaded
     */
    bool isPluginLoaded(const QString& pluginId) const;
    
    /**
     * @brief Get plugin source
     * @param pluginId Plugin identifier
     * @return Plugin source (Local, Remote, or Unknown)
     */
    PluginSource getPluginSource(const QString& pluginId) const;
    
    /**
     * @brief Get list of loaded plugins
     * @return List of loaded plugin identifiers
     */
    QStringList loadedPlugins() const;

    // === Plugin Discovery ===
    
    /**
     * @brief Discover available plugins
     * @return Future with list of available plugin identifiers
     */
    QFuture<QStringList> discoverPlugins();

    // === Manager Access ===
    
    /**
     * @brief Get the local plugin manager
     * @return Pointer to local plugin manager
     */
    qtplugin::PluginManager* localPluginManager() const;
    
    /**
     * @brief Get the remote plugin manager
     * @return Pointer to remote plugin manager
     */
    RemotePluginManager* remotePluginManager() const;

public slots:
    /**
     * @brief Check for plugin updates
     */
    void checkForUpdates();

signals:
    /**
     * @brief Emitted when load strategy changes
     * @param strategy New load strategy
     */
    void loadStrategyChanged(UnifiedLoadStrategy strategy);
    
    /**
     * @brief Emitted when auto update setting changes
     * @param enabled New auto update setting
     */
    void autoUpdateEnabledChanged(bool enabled);
    
    /**
     * @brief Emitted when a plugin is loaded
     * @param pluginId Plugin identifier
     * @param localPath Local file path
     */
    void pluginLoaded(const QString& pluginId, const QString& localPath);
    
    /**
     * @brief Emitted when a plugin is unloaded
     * @param pluginId Plugin identifier
     * @param localPath Local file path
     */
    void pluginUnloaded(const QString& pluginId, const QString& localPath);
    
    /**
     * @brief Emitted when plugin loading fails
     * @param pluginId Plugin identifier
     * @param localPath Local file path (may be empty)
     * @param errorMessage Error message
     */
    void pluginLoadFailed(const QString& pluginId, const QString& localPath, const QString& errorMessage);
    
    /**
     * @brief Emitted during remote plugin download
     * @param pluginId Plugin identifier
     * @param bytesReceived Bytes received so far
     * @param bytesTotal Total bytes to download
     */
    void downloadProgress(const QString& pluginId, qint64 bytesReceived, qint64 bytesTotal);
    
    /**
     * @brief Emitted when plugin updates are available
     * @param availablePlugins List of available plugin updates
     */
    void pluginUpdatesAvailable(const QList<PluginInfo>& availablePlugins);

private:
    // Internal implementation methods
    bool loadPluginImpl(const QString& pluginIdOrPath);
    bool loadPluginById(const QString& pluginId);
    bool loadLocalPlugin(const QString& path);
    bool loadLocalPluginById(const QString& pluginId);
    bool loadRemotePlugin(const QString& pluginId);
    bool updatePluginImpl(const QString& pluginId);
    
    QStringList discoverPluginsImpl();
    QStringList discoverLocalPlugins();
    QStringList discoverRemotePlugins();
    
    bool isLocalPluginAvailable(const QString& pluginId);
    bool isRemotePluginAvailable(const QString& pluginId);

private:
    class UnifiedPluginManagerPrivate;
    std::unique_ptr<UnifiedPluginManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(UnifiedPluginManager)
};

} // namespace Remote
} // namespace QtPlugin
