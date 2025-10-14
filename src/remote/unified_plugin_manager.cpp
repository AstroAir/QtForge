#include "qtplugin/remote/unified_plugin_manager.hpp"
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QStringList>
#include <QtConcurrent>
#include <algorithm>
#include <filesystem>
#include "qtplugin/communication/message_bus.hpp"
#include "qtplugin/core/plugin_manager.hpp"
#include "qtplugin/managers/configuration_manager.hpp"
#include "qtplugin/managers/logging_manager.hpp"
#include "qtplugin/managers/resource_lifecycle.hpp"
#include "qtplugin/managers/resource_manager.hpp"
#include "qtplugin/remote/remote_plugin_manager.hpp"

Q_LOGGING_CATEGORY(unifiedPluginLog, "qtforge.unified.plugin")

namespace qtplugin {

// Private implementation class definition
class UnifiedPluginManager::UnifiedPluginManagerPrivate {
public:
    qtplugin::PluginManager* localPluginManager = nullptr;
    RemotePluginManager* remotePluginManager = nullptr;
    UnifiedLoadStrategy loadStrategy = UnifiedLoadStrategy::FavorLocal;
    bool autoUpdateEnabled = true;
    QHash<QString, PluginSource> pluginSources;
    QHash<QString, QString> pluginPaths;  // pluginId -> local path
    QStringList searchPaths;              // Search paths for local plugins

    QString getPluginId(const QString& path) {
        // Extract plugin ID from path
        QFileInfo fileInfo(path);
        return fileInfo.baseName();
    }
};

UnifiedPluginManager::UnifiedPluginManager(QObject* parent)
    : QObject(parent), d_ptr(std::make_unique<UnifiedPluginManagerPrivate>()) {
    Q_D(UnifiedPluginManager);

    // Initialize local plugin manager
    d->localPluginManager = new qtplugin::PluginManager(
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, this);
    // Set default search paths
    d->searchPaths << "./plugins" << "./";
    d->searchPaths << QStandardPaths::writableLocation(
                          QStandardPaths::ApplicationsLocation) +
                          "/plugins";

    // Initialize remote plugin manager
    d->remotePluginManager = new RemotePluginManager(this);

    // Connect signals from local plugin manager
    connect(d->localPluginManager, &qtplugin::PluginManager::plugin_loaded,
            this, [this](const QString& plugin_id) {
                Q_D(UnifiedPluginManager);
                d->pluginSources[plugin_id] = PluginSource::Local;
                // Get plugin info to find the path
                auto plugin_info = d->localPluginManager->get_plugin_info(
                    plugin_id.toStdString());
                if (plugin_info) {
                    QString path =
                        QString::fromStdString(plugin_info->file_path.string());
                    d->pluginPaths[plugin_id] = path;
                    emit pluginLoaded(plugin_id, path);
                }
            });

    connect(d->localPluginManager, &qtplugin::PluginManager::plugin_unloaded,
            this, [this](const QString& plugin_id) {
                Q_D(UnifiedPluginManager);
                d->pluginSources.remove(plugin_id);
                QString path = d->pluginPaths.take(plugin_id);
                emit pluginUnloaded(plugin_id, path);
            });

    connect(d->localPluginManager, &qtplugin::PluginManager::plugin_error, this,
            [this](const QString& plugin_id, const QString& error) {
                Q_D(UnifiedPluginManager);
                QString path = d->pluginPaths.value(plugin_id);
                emit pluginLoadFailed(plugin_id, path, error);
            });

    // Connect signals from remote plugin manager
    connect(d->remotePluginManager, &RemotePluginManager::downloadProgress,
            this, &UnifiedPluginManager::downloadProgress);
    connect(d->remotePluginManager,
            &RemotePluginManager::pluginUpdatesAvailable, this,
            &UnifiedPluginManager::pluginUpdatesAvailable);

    qCDebug(unifiedPluginLog) << "Unified plugin manager initialized";
}

UnifiedPluginManager::~UnifiedPluginManager() = default;

void UnifiedPluginManager::setLoadStrategy(UnifiedLoadStrategy strategy) {
    Q_D(UnifiedPluginManager);
    if (d->loadStrategy != strategy) {
        d->loadStrategy = strategy;
        qCDebug(unifiedPluginLog)
            << "Load strategy changed to:" << static_cast<int>(strategy);
        emit loadStrategyChanged(strategy);
    }
}

UnifiedLoadStrategy UnifiedPluginManager::loadStrategy() const {
    Q_D(const UnifiedPluginManager);
    return d->loadStrategy;
}

void UnifiedPluginManager::setAutoUpdateEnabled(bool enabled) {
    Q_D(UnifiedPluginManager);
    if (d->autoUpdateEnabled != enabled) {
        d->autoUpdateEnabled = enabled;
        qCDebug(unifiedPluginLog)
            << "Auto update" << (enabled ? "enabled" : "disabled");
        emit autoUpdateEnabledChanged(enabled);
    }
}

bool UnifiedPluginManager::isAutoUpdateEnabled() const {
    Q_D(const UnifiedPluginManager);
    return d->autoUpdateEnabled;
}

QFuture<bool> UnifiedPluginManager::loadPlugin(const QString& pluginIdOrPath) {
    return QtConcurrent::run(
        [this, pluginIdOrPath]() { return loadPluginImpl(pluginIdOrPath); });
}

bool UnifiedPluginManager::loadPluginImpl(const QString& pluginIdOrPath) {
    Q_D(UnifiedPluginManager);

    // Determine if this is a path or plugin ID
    bool isPath = pluginIdOrPath.contains('/') ||
                  pluginIdOrPath.contains('\\') ||
                  QFileInfo::exists(pluginIdOrPath);

    if (isPath) {
        // Direct path - load locally
        return loadLocalPlugin(pluginIdOrPath);
    }

    // Plugin ID - apply load strategy
    return loadPluginById(pluginIdOrPath);
}

bool UnifiedPluginManager::loadPluginById(const QString& pluginId) {
    Q_D(UnifiedPluginManager);

    bool localAvailable = isLocalPluginAvailable(pluginId);
    bool remoteAvailable = isRemotePluginAvailable(pluginId);

    switch (d->loadStrategy) {
        case UnifiedLoadStrategy::FavorLocal:
            if (localAvailable) {
                return loadLocalPluginById(pluginId);
            } else if (remoteAvailable) {
                return loadRemotePlugin(pluginId);
            }
            break;

        case UnifiedLoadStrategy::FavorRemote:
            if (remoteAvailable) {
                return loadRemotePlugin(pluginId);
            } else if (localAvailable) {
                return loadLocalPluginById(pluginId);
            }
            break;

        case UnifiedLoadStrategy::LocalOnly:
            if (localAvailable) {
                return loadLocalPluginById(pluginId);
            }
            break;

        case UnifiedLoadStrategy::RemoteOnly:
            if (remoteAvailable) {
                return loadRemotePlugin(pluginId);
            }
            break;
    }

    qCWarning(unifiedPluginLog) << "Plugin not found:" << pluginId;
    emit pluginLoadFailed(pluginId, QString(), "Plugin not available");
    return false;
}

bool UnifiedPluginManager::loadLocalPlugin(const QString& path) {
    Q_D(UnifiedPluginManager);

    if (!QFileInfo::exists(path)) {
        qCWarning(unifiedPluginLog)
            << "Local plugin file does not exist:" << path;
        return false;
    }

    try {
        // Implement actual local plugin loading
        if (!d->localPluginManager) {
            qCWarning(unifiedPluginLog)
                << "Local plugin manager not initialized";
            return false;
        }

        // Use the actual PluginManager interface
        std::filesystem::path fsPath(path.toStdString());
        auto result = d->localPluginManager->load_plugin(fsPath);
        if (result.has_value()) {
            QString pluginId = d->getPluginId(path);
            d->pluginSources[pluginId] = PluginSource::Local;
            d->pluginPaths[pluginId] = path;
            emit pluginLoaded(pluginId, path);
            return true;
        } else {
            qCWarning(unifiedPluginLog)
                << "Failed to load local plugin:" << path;
            return false;
        }
    } catch (const std::exception& e) {
        qCWarning(unifiedPluginLog)
            << "Failed to load local plugin:" << path << "-" << e.what();
        return false;
    }
}

bool UnifiedPluginManager::loadLocalPluginById(const QString& pluginId) {
    Q_D(UnifiedPluginManager);

    // Search for plugin in local directories
    QStringList searchPaths = d->searchPaths;

    for (const QString& searchPath : searchPaths) {
        QDir dir(searchPath);
        QStringList filters;
        filters << QString("%1.*").arg(pluginId);

        QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

        for (const QFileInfo& fileInfo : files) {
            if (fileInfo.suffix() == "dll" || fileInfo.suffix() == "so" ||
                fileInfo.suffix() == "dylib" || fileInfo.suffix() == "plugin") {
                return loadLocalPlugin(fileInfo.absoluteFilePath());
            }
        }
    }

    qCWarning(unifiedPluginLog) << "Local plugin not found:" << pluginId;
    return false;
}

bool UnifiedPluginManager::loadRemotePlugin(const QString& pluginId) {
    Q_D(UnifiedPluginManager);

    qCDebug(unifiedPluginLog) << "Loading remote plugin:" << pluginId;

    // Download plugin
    auto downloadFuture = d->remotePluginManager->downloadPlugin(pluginId);

    // Wait for download completion
    downloadFuture.waitForFinished();
    QString localPath = downloadFuture.result();

    if (localPath.isEmpty()) {
        qCWarning(unifiedPluginLog)
            << "Failed to download remote plugin:" << pluginId;
        return false;
    }

    // Load the downloaded plugin
    bool success = loadLocalPlugin(localPath);

    if (success) {
        d->pluginSources[pluginId] = PluginSource::Remote;
        d->pluginPaths[pluginId] = localPath;
    }

    return success;
}

bool UnifiedPluginManager::unloadPlugin(const QString& pluginId) {
    Q_D(UnifiedPluginManager);

    if (!d->pluginPaths.contains(pluginId)) {
        qCWarning(unifiedPluginLog) << "Plugin not loaded:" << pluginId;
        return false;
    }

    QString path = d->pluginPaths[pluginId];
    // Use the actual PluginManager interface
    bool success = false;
    if (d->localPluginManager) {
        auto result =
            d->localPluginManager->unload_plugin(pluginId.toStdString());
        success = result.has_value();
    } else {
        qCWarning(unifiedPluginLog) << "Local plugin manager not initialized";
    }

    if (success) {
        d->pluginSources.remove(pluginId);
        d->pluginPaths.remove(pluginId);
        qCDebug(unifiedPluginLog) << "Plugin unloaded:" << pluginId;
    }

    return success;
}

QFuture<bool> UnifiedPluginManager::updatePlugin(const QString& pluginId) {
    return QtConcurrent::run(
        [this, pluginId]() { return updatePluginImpl(pluginId); });
}

bool UnifiedPluginManager::updatePluginImpl(const QString& pluginId) {
    Q_D(UnifiedPluginManager);

    if (!d->pluginSources.contains(pluginId)) {
        qCWarning(unifiedPluginLog)
            << "Plugin not loaded, cannot update:" << pluginId;
        return false;
    }

    PluginSource source = d->pluginSources[pluginId];

    if (source == PluginSource::Local) {
        // For local plugins, check if a newer version is available remotely
        if (d->loadStrategy != UnifiedLoadStrategy::LocalOnly &&
            isRemotePluginAvailable(pluginId)) {
            // Unload current plugin
            unloadPlugin(pluginId);

            // Load from remote
            return loadRemotePlugin(pluginId);
        } else {
            qCDebug(unifiedPluginLog)
                << "Local plugin update not implemented:" << pluginId;
            return false;
        }
    } else {
        // For remote plugins, download latest version
        unloadPlugin(pluginId);
        return loadRemotePlugin(pluginId);
    }

    // Should never reach here, but add return to avoid compiler warning
    return false;
}

bool UnifiedPluginManager::isPluginLoaded(const QString& pluginId) const {
    Q_D(const UnifiedPluginManager);
    return d->pluginSources.contains(pluginId);
}

PluginSource UnifiedPluginManager::getPluginSource(
    const QString& pluginId) const {
    Q_D(const UnifiedPluginManager);
    return d->pluginSources.value(pluginId, PluginSource::Unknown);
}

QStringList UnifiedPluginManager::loadedPlugins() const {
    Q_D(const UnifiedPluginManager);
    return d->pluginSources.keys();
}

QFuture<QStringList> UnifiedPluginManager::discoverPlugins() {
    return QtConcurrent::run([this]() { return discoverPluginsImpl(); });
}

QStringList UnifiedPluginManager::discoverPluginsImpl() {
    Q_D(UnifiedPluginManager);

    QStringList allPlugins;

    // Discover local plugins
    QStringList localPlugins = discoverLocalPlugins();
    allPlugins.append(localPlugins);

    // Discover remote plugins
    if (d->loadStrategy != UnifiedLoadStrategy::LocalOnly) {
        QStringList remotePlugins = discoverRemotePlugins();

        // Remove duplicates (favor local for discovery)
        for (const QString& remotePlugin : remotePlugins) {
            if (!allPlugins.contains(remotePlugin)) {
                allPlugins.append(remotePlugin);
            }
        }
    }

    qCDebug(unifiedPluginLog) << "Discovered" << allPlugins.size() << "plugins";

    return allPlugins;
}

QStringList UnifiedPluginManager::discoverLocalPlugins() {
    Q_D(UnifiedPluginManager);

    QStringList plugins;
    QStringList searchPaths = d->searchPaths;

    for (const QString& searchPath : searchPaths) {
        QDir dir(searchPath);
        QStringList filters;
        filters << "*.dll" << "*.so" << "*.dylib" << "*.plugin";

        QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

        for (const QFileInfo& fileInfo : files) {
            QString pluginId = fileInfo.baseName();
            if (!plugins.contains(pluginId)) {
                plugins.append(pluginId);
            }
        }
    }

    qCDebug(unifiedPluginLog)
        << "Discovered" << plugins.size() << "local plugins";

    return plugins;
}

QStringList UnifiedPluginManager::discoverRemotePlugins() {
    Q_D(UnifiedPluginManager);

    // Discover remote plugins
    auto future = d->remotePluginManager->discoverPlugins();
    future.waitForFinished();

    auto pluginInfos = future.result();
    QStringList plugins;

    for (const auto& pluginInfo : pluginInfos) {
        plugins.append(pluginInfo.id);
    }

    qCDebug(unifiedPluginLog)
        << "Discovered" << plugins.size() << "remote plugins";

    return plugins;
}

bool UnifiedPluginManager::isLocalPluginAvailable(const QString& pluginId) {
    Q_D(UnifiedPluginManager);

    QStringList searchPaths = d->searchPaths;

    for (const QString& searchPath : searchPaths) {
        QDir dir(searchPath);
        QStringList filters;
        filters << QString("%1.*").arg(pluginId);

        QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

        for (const QFileInfo& fileInfo : files) {
            if (fileInfo.suffix() == "dll" || fileInfo.suffix() == "so" ||
                fileInfo.suffix() == "dylib" || fileInfo.suffix() == "plugin") {
                return true;
            }
        }
    }

    return false;
}

bool UnifiedPluginManager::isRemotePluginAvailable(const QString& pluginId) {
    Q_D(UnifiedPluginManager);

    // This is a simplified check - in practice, you'd want to cache the results
    // from the last discovery operation
    auto future = d->remotePluginManager->discoverPlugins();
    future.waitForFinished();

    auto pluginInfos = future.result();

    for (const auto& pluginInfo : pluginInfos) {
        if (pluginInfo.id == pluginId) {
            return true;
        }
    }

    return false;
}

qtplugin::PluginManager* UnifiedPluginManager::localPluginManager() const {
    Q_D(const UnifiedPluginManager);
    return d->localPluginManager;
}

RemotePluginManager* UnifiedPluginManager::remotePluginManager() const {
    Q_D(const UnifiedPluginManager);
    return d->remotePluginManager;
}

void UnifiedPluginManager::checkForUpdates() {
    Q_D(UnifiedPluginManager);

    if (!d->autoUpdateEnabled) {
        qCDebug(unifiedPluginLog)
            << "Auto update disabled, skipping update check";
        return;
    }

    qCDebug(unifiedPluginLog) << "Checking for plugin updates";

    // Check for updates to currently loaded plugins
    QStringList loadedPlugins = d->pluginSources.keys();

    for (const QString& pluginId : loadedPlugins) {
        auto updateFuture = updatePlugin(pluginId);
        // Don't wait for completion - let updates happen asynchronously
        Q_UNUSED(updateFuture)
    }
}

}  // namespace qtplugin
