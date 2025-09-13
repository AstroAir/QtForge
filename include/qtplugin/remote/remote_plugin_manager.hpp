/**
 * @file remote_plugin_manager.hpp
 * @brief Manager for remote plugin operations
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QFuture>
#include <QtConcurrent>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include <chrono>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(remotePluginLog)

namespace QtPlugin {
namespace Remote {

// Forward declarations
class RemoteSecurityManager;

/**
 * @brief Plugin repository configuration
 */
struct PluginRepository {
    QString name;                        ///< Repository name
    QString url;                         ///< Repository URL
    QString description;                 ///< Repository description
    bool isEnabled = true;               ///< Whether repository is enabled
    QStringList supportedPlatforms;     ///< Supported platforms
    QString authenticationType;          ///< Authentication type (none, basic, token)
    QByteArray authenticationData;       ///< Authentication data
    int priority = 0;                    ///< Repository priority (higher = preferred)
};

/**
 * @brief Plugin information from repository
 */
struct PluginInfo {
    QString id;                          ///< Plugin identifier
    QString name;                        ///< Plugin display name
    QString version;                     ///< Plugin version
    QString description;                 ///< Plugin description
    QString author;                      ///< Plugin author
    QString downloadUrl;                 ///< Download URL
    qint64 fileSize = 0;                 ///< File size in bytes
    QByteArray checksum;                 ///< SHA-256 checksum
    QByteArray signature;                ///< Digital signature
    QString publisherId;                 ///< Publisher identifier
    QStringList dependencies;            ///< Plugin dependencies
    QDateTime releaseDate;               ///< Release date
    QJsonObject metadata;                ///< Additional metadata
};

/**
 * @brief Plugin cache settings
 */
struct PluginCacheSettings {
    qint64 maxCacheSize = 1024 * 1024 * 1024;  ///< Maximum cache size (1GB)
    std::chrono::hours cacheExpirationTime{24}; ///< Cache expiration time
    bool enablePersistentCache = true;          ///< Enable persistent cache
    QString cacheDirectory;                     ///< Custom cache directory
};

/**
 * @brief Fallback settings for remote operations
 */
struct FallbackSettings {
    bool enableFallback = true;                 ///< Enable fallback mechanisms
    int maxRetries = 3;                         ///< Maximum retry attempts
    std::chrono::seconds retryDelay{5};         ///< Delay between retries
    QStringList fallbackRepositories;          ///< Fallback repository URLs
};

/**
 * @brief Manager for remote plugin operations
 * 
 * This class handles downloading, caching, and managing plugins from remote repositories.
 * It provides both synchronous and asynchronous operations with progress tracking.
 */
class RemotePluginManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit RemotePluginManager(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~RemotePluginManager();

    // === Repository Management ===
    
    /**
     * @brief Add a plugin repository
     * @param repository Repository configuration
     */
    void addRepository(const PluginRepository& repository);
    
    /**
     * @brief Remove a plugin repository
     * @param repositoryUrl Repository URL to remove
     */
    void removeRepository(const QString& repositoryUrl);
    
    /**
     * @brief Get list of configured repositories
     * @return List of repositories
     */
    QList<PluginRepository> repositories() const;

    // === Plugin Discovery ===
    
    /**
     * @brief Discover available plugins from repositories
     * @return Future with list of available plugins
     */
    QFuture<QList<PluginInfo>> discoverPlugins();

    // === Plugin Download ===
    
    /**
     * @brief Download plugin asynchronously
     * @param pluginId Plugin identifier
     * @param version Specific version (empty for latest)
     * @return Future with local file path
     */
    QFuture<QString> downloadPlugin(const QString& pluginId, const QString& version = QString());

    // === Cache Management ===
    
    /**
     * @brief Clear plugin cache
     */
    void clearCache();
    
    /**
     * @brief Check if cache file is valid
     * @param filePath Path to cache file
     * @return true if cache is valid
     */
    bool isCacheValid(const QString& filePath);
    
    /**
     * @brief Set cache settings
     * @param settings Cache settings
     */
    void setCacheSettings(const PluginCacheSettings& settings);
    
    /**
     * @brief Get current cache settings
     * @return Current cache settings
     */
    PluginCacheSettings cacheSettings() const;

    // === Fallback Configuration ===
    
    /**
     * @brief Set fallback settings
     * @param settings Fallback settings
     */
    void setFallbackSettings(const FallbackSettings& settings);
    
    /**
     * @brief Get current fallback settings
     * @return Current fallback settings
     */
    FallbackSettings fallbackSettings() const;

    // === Security Integration ===
    
    /**
     * @brief Get the security manager
     * @return Pointer to security manager
     */
    RemoteSecurityManager* securityManager() const;

public slots:
    /**
     * @brief Check for plugin updates
     */
    void checkForUpdates();

signals:
    /**
     * @brief Emitted when repositories list changes
     */
    void repositoriesChanged();
    
    /**
     * @brief Emitted during plugin download
     * @param pluginId Plugin identifier
     * @param bytesReceived Bytes received so far
     * @param bytesTotal Total bytes to download
     */
    void downloadProgress(const QString& pluginId, qint64 bytesReceived, qint64 bytesTotal);
    
    /**
     * @brief Emitted when plugin download starts
     * @param pluginId Plugin identifier
     */
    void downloadStarted(const QString& pluginId);
    
    /**
     * @brief Emitted when plugin download finishes
     * @param pluginId Plugin identifier
     * @param localPath Local file path (empty on failure)
     */
    void downloadFinished(const QString& pluginId, const QString& localPath);
    
    /**
     * @brief Emitted when plugin updates are available
     * @param availablePlugins List of available plugin updates
     */
    void pluginUpdatesAvailable(const QList<PluginInfo>& availablePlugins);

private:
    // Internal implementation methods
    QList<PluginInfo> discoverPluginsImpl();
    QString downloadPluginImpl(const QString& pluginId, const QString& version);
    QList<PluginInfo> downloadPluginManifest(const QString& manifestUrl);

private:
    class RemotePluginManagerPrivate;
    std::unique_ptr<RemotePluginManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(RemotePluginManager)
};

} // namespace Remote
} // namespace QtPlugin
