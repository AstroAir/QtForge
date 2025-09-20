#include "qtplugin/remote/remote_plugin_manager.hpp"
// Remote security manager include removed
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QCryptographicHash>
#include <QTemporaryDir>
#include <QtConcurrent>
#include <QFile>
#include <QEventLoop>
#include <QUrl>
#include <QSysInfo>
#include <QJsonValue>
#include <QJsonParseError>
#include <QFutureWatcher>
#include <algorithm>

Q_LOGGING_CATEGORY(remotePluginLog, "qtforge.remote.plugin")

namespace QtPlugin {
namespace Remote {

// Private implementation class definition
class RemotePluginManager::RemotePluginManagerPrivate
{
public:
    QList<PluginRepository> repositories;
    QString cacheDir;
    QNetworkAccessManager* networkManager = nullptr;
    RemoteSecurityManager* securityManager = nullptr;
    PluginCacheSettings cacheSettings;
    FallbackSettings fallbackSettings;
    QTimer* updateTimer = nullptr;
    QHash<QString, PluginInfo> pluginCache;
    QHash<QString, QNetworkReply*> activeDownloads;
    
    void setupCacheDirectory()
    {
        cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/qtforge/remote_plugins";
        QDir dir;
        dir.mkpath(cacheDir);
    }
    
    void setupNetworkManager()
    {
        if (!networkManager) {
            networkManager = new QNetworkAccessManager();
        }
    }
    
    QString getCacheFilePath(const QString& pluginId, const QString& version)
    {
        return QString("%1/%2_%3.plugin").arg(cacheDir, pluginId, version);
    }
};

RemotePluginManager::RemotePluginManager(QObject* parent)
    : QObject(parent)
    , d_ptr(std::make_unique<RemotePluginManagerPrivate>())
{
    Q_D(RemotePluginManager);
    
    d->setupCacheDirectory();
    d->setupNetworkManager();
    
    // Security manager removed - SHA256 verification handled by core PluginManager
    
    // Setup automatic update check timer
    d->updateTimer = new QTimer(this);
    connect(d->updateTimer, &QTimer::timeout, this, &RemotePluginManager::checkForUpdates);
    d->updateTimer->start(std::chrono::milliseconds(60 * 60 * 1000)); // 1 hour
    
    // Set default cache settings
    d->cacheSettings.maxCacheSize = 1024 * 1024 * 1024; // 1GB
    d->cacheSettings.cacheExpirationTime = std::chrono::hours(24);
    d->cacheSettings.enablePersistentCache = true;
    
    // Set default fallback settings
    d->fallbackSettings.enableFallback = true;
    d->fallbackSettings.maxRetries = 3;
    d->fallbackSettings.retryDelay = std::chrono::seconds(5);
    
    qCDebug(remotePluginLog) << "Remote plugin manager initialized";
}

RemotePluginManager::~RemotePluginManager()
{
    Q_D(RemotePluginManager);
    
    // Cancel any active downloads
    for (auto* reply : d->activeDownloads) {
        reply->abort();
        reply->deleteLater();
    }
    d->activeDownloads.clear();
}

void RemotePluginManager::addRepository(const PluginRepository& repository)
{
    Q_D(RemotePluginManager);
    
    // Check if repository already exists
    auto it = std::find_if(d->repositories.begin(), d->repositories.end(),
                          [&repository](const PluginRepository& repo) {
                              return repo.url == repository.url;
                          });
    
    if (it != d->repositories.end()) {
        // Update existing repository
        *it = repository;
        qCDebug(remotePluginLog) << "Updated repository:" << repository.name;
    } else {
        // Add new repository
        d->repositories.append(repository);
        qCDebug(remotePluginLog) << "Added repository:" << repository.name;
    }
    
    emit repositoriesChanged();
}

void RemotePluginManager::removeRepository(const QString& repositoryUrl)
{
    Q_D(RemotePluginManager);
    
    auto it = std::remove_if(d->repositories.begin(), d->repositories.end(),
                            [&repositoryUrl](const PluginRepository& repo) {
                                return repo.url == repositoryUrl;
                            });
    
    if (it != d->repositories.end()) {
        d->repositories.erase(it, d->repositories.end());
        qCDebug(remotePluginLog) << "Removed repository:" << repositoryUrl;
        emit repositoriesChanged();
    }
}

QList<PluginRepository> RemotePluginManager::repositories() const
{
    Q_D(const RemotePluginManager);
    return d->repositories;
}

QFuture<QList<PluginInfo>> RemotePluginManager::discoverPlugins()
{
    return QtConcurrent::run([this]() {
        return discoverPluginsImpl();
    });
}

QList<PluginInfo> RemotePluginManager::discoverPluginsImpl()
{
    Q_D(RemotePluginManager);
    
    QList<PluginInfo> allPlugins;
    
    for (const auto& repository : d->repositories) {
        if (!repository.isEnabled) {
            continue;
        }
        
        QString manifestUrl = repository.url + "/manifest.json";
        QList<PluginInfo> repoPlugins = downloadPluginManifest(manifestUrl);
        
        // Filter plugins based on repository settings
        for (const auto& plugin : repoPlugins) {
            if (repository.supportedPlatforms.isEmpty() || 
                repository.supportedPlatforms.contains(QSysInfo::productType())) {
                allPlugins.append(plugin);
            }
        }
    }
    
    // Update plugin cache
    for (const auto& plugin : allPlugins) {
        d->pluginCache[plugin.id] = plugin;
    }
    
    qCDebug(remotePluginLog) << "Discovered" << allPlugins.size() << "plugins from" << d->repositories.size() << "repositories";
    
    return allPlugins;
}

QList<PluginInfo> RemotePluginManager::downloadPluginManifest(const QString& manifestUrl)
{
    Q_D(RemotePluginManager);
    
    QList<PluginInfo> plugins;
    
    QNetworkRequest request{QUrl(manifestUrl)};
    request.setRawHeader("User-Agent", "QtForge-RemotePluginManager/1.0");
    
    QNetworkReply* reply = d->networkManager->get(request);
    
    // Wait for completion (synchronous for simplicity)
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(remotePluginLog) << "Failed to download manifest from" << manifestUrl << ":" << reply->errorString();
        reply->deleteLater();
        return plugins;
    }
    
    QByteArray manifestData = reply->readAll();
    reply->deleteLater();
    
    // Parse JSON manifest
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(manifestData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(remotePluginLog) << "Failed to parse manifest JSON:" << parseError.errorString();
        return plugins;
    }
    
    QJsonObject manifestObj = doc.object();
    QJsonArray pluginsArray = manifestObj["plugins"].toArray();
    
    for (const QJsonValue& pluginValue : pluginsArray) {
        QJsonObject pluginObj = pluginValue.toObject();
        
        PluginInfo plugin;
        plugin.id = pluginObj["id"].toString();
        plugin.name = pluginObj["name"].toString();
        plugin.version = pluginObj["version"].toString();
        plugin.description = pluginObj["description"].toString();
        plugin.author = pluginObj["author"].toString();
        plugin.downloadUrl = pluginObj["downloadUrl"].toString();
        plugin.fileSize = pluginObj["fileSize"].toInteger();
        plugin.checksum = pluginObj["checksum"].toString().toUtf8();
        plugin.signature = pluginObj["signature"].toString().toUtf8();
        plugin.publisherId = pluginObj["publisherId"].toString();
        
        // Parse dependencies
        QJsonArray depsArray = pluginObj["dependencies"].toArray();
        for (const QJsonValue& depValue : depsArray) {
            plugin.dependencies.append(depValue.toString());
        }
        
        plugins.append(plugin);
    }
    
    qCDebug(remotePluginLog) << "Downloaded manifest with" << plugins.size() << "plugins from" << manifestUrl;
    
    return plugins;
}

QFuture<QString> RemotePluginManager::downloadPlugin(const QString& pluginId, const QString& version)
{
    return QtConcurrent::run([this, pluginId, version]() {
        return downloadPluginImpl(pluginId, version);
    });
}

QString RemotePluginManager::downloadPluginImpl(const QString& pluginId, const QString& version)
{
    Q_D(RemotePluginManager);
    
    // Check if plugin is already cached
    QString cacheFilePath = d->getCacheFilePath(pluginId, version);
    if (QFileInfo::exists(cacheFilePath) && isCacheValid(cacheFilePath)) {
        qCDebug(remotePluginLog) << "Plugin found in cache:" << cacheFilePath;
        emit downloadProgress(pluginId, 100, 100);
        return cacheFilePath;
    }
    
    // Find plugin info
    PluginInfo pluginInfo;
    bool pluginFound = false;
    
    if (d->pluginCache.contains(pluginId)) {
        pluginInfo = d->pluginCache[pluginId];
        if (pluginInfo.version == version || version.isEmpty()) {
            pluginFound = true;
        }
    }
    
    if (!pluginFound) {
        // Refresh plugin discovery and try again
        discoverPluginsImpl();
        
        if (d->pluginCache.contains(pluginId)) {
            pluginInfo = d->pluginCache[pluginId];
            pluginFound = true;
        }
    }
    
    if (!pluginFound) {
        qCWarning(remotePluginLog) << "Plugin not found:" << pluginId << "version:" << version;
        return QString();
    }
    
    // Publisher trust verification removed - use SHA256 verification instead
    qCDebug(remotePluginLog) << "Downloading plugin from publisher:" << pluginInfo.publisherId;
    
    // Download plugin file
    QNetworkRequest request{QUrl(pluginInfo.downloadUrl)};
    request.setRawHeader("User-Agent", "QtForge-RemotePluginManager/1.0");
    
    QNetworkReply* reply = d->networkManager->get(request);
    d->activeDownloads[pluginId] = reply;
    
    // Track download progress
    connect(reply, &QNetworkReply::downloadProgress, this, [this, pluginId](qint64 bytesReceived, qint64 bytesTotal) {
        emit downloadProgress(pluginId, bytesReceived, bytesTotal);
    });
    
    // Wait for completion
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    d->activeDownloads.remove(pluginId);
    
    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(remotePluginLog) << "Failed to download plugin" << pluginId << ":" << reply->errorString();
        reply->deleteLater();
        return QString();
    }
    
    QByteArray pluginData = reply->readAll();
    reply->deleteLater();
    
    // Verify checksum
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(pluginData);
    QByteArray calculatedChecksum = hash.result().toHex();
    
    if (calculatedChecksum != pluginInfo.checksum) {
        qCWarning(remotePluginLog) << "Checksum mismatch for plugin" << pluginId;
        return QString();
    }
    
    // Save to cache
    QFile cacheFile(cacheFilePath);
    if (!cacheFile.open(QIODevice::WriteOnly)) {
        qCWarning(remotePluginLog) << "Failed to write plugin to cache:" << cacheFilePath;
        return QString();
    }
    
    cacheFile.write(pluginData);
    cacheFile.close();
    
    // Signature verification removed - SHA256 verification will be handled by core PluginManager
    qCDebug(remotePluginLog) << "Plugin downloaded, SHA256 verification will be performed during loading";
    
    qCDebug(remotePluginLog) << "Successfully downloaded and cached plugin:" << pluginId;
    
    return cacheFilePath;
}

bool RemotePluginManager::isCacheValid(const QString& filePath)
{
    Q_D(RemotePluginManager);
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }
    
    // Check cache expiration
    QDateTime lastModified = fileInfo.lastModified();
    QDateTime now = QDateTime::currentDateTime();
    
    if (now.toSecsSinceEpoch() - lastModified.toSecsSinceEpoch() > d->cacheSettings.cacheExpirationTime.count()) {
        return false;
    }
    
    return true;
}

void RemotePluginManager::clearCache()
{
    Q_D(RemotePluginManager);
    
    QDir cacheDir(d->cacheDir);
    if (cacheDir.exists()) {
        cacheDir.removeRecursively();
        d->setupCacheDirectory();
        qCDebug(remotePluginLog) << "Plugin cache cleared";
    }
}

void RemotePluginManager::setCacheSettings(const PluginCacheSettings& settings)
{
    Q_D(RemotePluginManager);
    d->cacheSettings = settings;
    qCDebug(remotePluginLog) << "Cache settings updated";
}

PluginCacheSettings RemotePluginManager::cacheSettings() const
{
    Q_D(const RemotePluginManager);
    return d->cacheSettings;
}

void RemotePluginManager::setFallbackSettings(const FallbackSettings& settings)
{
    Q_D(RemotePluginManager);
    d->fallbackSettings = settings;
    qCDebug(remotePluginLog) << "Fallback settings updated";
}

FallbackSettings RemotePluginManager::fallbackSettings() const
{
    Q_D(const RemotePluginManager);
    return d->fallbackSettings;
}

RemoteSecurityManager* RemotePluginManager::securityManager() const
{
    Q_D(const RemotePluginManager);
    return d->securityManager;
}

void RemotePluginManager::checkForUpdates()
{
    qCDebug(remotePluginLog) << "Checking for plugin updates";
    
    auto future = discoverPlugins();
    auto watcher = new QFutureWatcher<QList<PluginInfo>>(this);
    
    connect(watcher, &QFutureWatcher<QList<PluginInfo>>::finished, this, [this, watcher]() {
        auto plugins = watcher->result();
        emit pluginUpdatesAvailable(plugins);
        watcher->deleteLater();
    });
    
    watcher->setFuture(future);
}

} // namespace Remote
} // namespace QtPlugin
