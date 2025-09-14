/**
 * @file remote_plugin_manager.hpp
 * @brief Remote plugin manager with caching, version management, and fallback mechanisms
 * @version 3.2.0
 */

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QVersionNumber>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>
#include <QReadWriteLock>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include "../../utils/error_handling.hpp"
#include "../../core/plugin_manager.hpp"
#include "../../core/plugin_interface.hpp"
#include "../security/remote_security_manager.hpp"

namespace qtplugin::remote {

// Forward declarations
class RemotePluginCache;
class RemotePluginLoader;
class RemotePluginRepository;

/**
 * @brief Remote plugin state
 */
enum class RemotePluginState {
    Unknown = 0,        ///< State unknown
    Discovering = 1,    ///< Discovering plugin metadata
    Available = 2,      ///< Plugin available for download
    Downloading = 3,    ///< Plugin being downloaded
    Cached = 4,         ///< Plugin cached locally
    Loading = 5,        ///< Plugin being loaded
    Loaded = 6,         ///< Plugin loaded and ready
    Error = 7           ///< Error state
};

/**
 * @brief Remote plugin download progress
 */
struct RemotePluginProgress {
    QString plugin_id;
    qint64 bytes_received = 0;
    qint64 bytes_total = 0;
    double percentage = 0.0;
    RemotePluginState state = RemotePluginState::Unknown;
    QString current_operation;
    QString error_message;
    QDateTime start_time;
    std::chrono::milliseconds estimated_time_remaining{0};
    
    QJsonObject to_json() const;
    static RemotePluginProgress from_json(const QJsonObject& json);
};

/**
 * @brief Remote plugin metadata
 */
struct RemotePluginMetadata {
    QString id;
    QString name;
    QVersionNumber version;
    QString description;
    QString author;
    QString license;
    QString category;
    QStringList tags;
    
    // Remote-specific fields
    QUrl download_url;
    QUrl signature_url;
    QUrl metadata_url;
    QString checksum_sha256;
    qint64 size_bytes = 0;
    QDateTime published_date;
    QDateTime last_updated;
    
    // Dependencies
    QStringList required_dependencies;
    QStringList optional_dependencies;
    QVersionNumber min_qtforge_version;
    QVersionNumber max_qtforge_version;
    
    // Security info
    QString publisher_id;
    security::PublisherTrustLevel trust_level = security::PublisherTrustLevel::Untrusted;
    bool requires_signature = true;
    
    // Repository info
    QString repository_id;
    QString repository_url;
    
    QJsonObject to_json() const;
    static RemotePluginMetadata from_json(const QJsonObject& json);
    
    bool is_compatible_version() const;
    bool has_required_dependencies() const;
};

/**
 * @brief Remote plugin cache entry
 */
struct RemotePluginCacheEntry {
    RemotePluginMetadata metadata;
    QString local_path;
    QDateTime cached_time;
    QDateTime last_accessed;
    qint64 file_size = 0;
    QString file_checksum;
    bool is_valid = false;
    int access_count = 0;
    
    QJsonObject to_json() const;
    static RemotePluginCacheEntry from_json(const QJsonObject& json);
};

/**
 * @brief Remote plugin repository configuration
 */
struct RemotePluginRepository {
    QString id;
    QString name;
    QString description;
    QUrl base_url;
    QString api_version;
    bool is_enabled = true;
    bool requires_authentication = false;
    QString authentication_token;
    security::PublisherTrustLevel default_trust_level = security::PublisherTrustLevel::Basic;
    
    // Repository capabilities
    bool supports_search = true;
    bool supports_categories = true;
    bool supports_versions = true;
    bool supports_dependencies = true;
    
    // Update settings
    std::chrono::hours update_interval{24};
    QDateTime last_updated;
    
    QJsonObject to_json() const;
    static RemotePluginRepository from_json(const QJsonObject& json);
};

/**
 * @brief Remote plugin cache manager
 */
class RemotePluginCache : public QObject {
    Q_OBJECT
    
public:
    explicit RemotePluginCache(const QString& cache_directory, QObject* parent = nullptr);
    ~RemotePluginCache() override;
    
    /**
     * @brief Initialize cache system
     * @return Success or error
     */
    expected<void, PluginError> initialize();
    
    /**
     * @brief Add plugin to cache
     * @param metadata Plugin metadata
     * @param plugin_data Plugin binary data
     * @return Success or error
     */
    expected<void, PluginError> cache_plugin(
        const RemotePluginMetadata& metadata,
        const QByteArray& plugin_data
    );
    
    /**
     * @brief Get cached plugin
     * @param plugin_id Plugin identifier
     * @return Cache entry if found
     */
    std::optional<RemotePluginCacheEntry> get_cached_plugin(const QString& plugin_id);
    
    /**
     * @brief Check if plugin is cached and valid
     * @param plugin_id Plugin identifier
     * @param version Required version (optional)
     * @return true if cached and valid
     */
    bool is_plugin_cached(const QString& plugin_id, const QVersionNumber& version = {});
    
    /**
     * @brief Remove plugin from cache
     * @param plugin_id Plugin identifier
     */
    void remove_cached_plugin(const QString& plugin_id);
    
    /**
     * @brief Clear entire cache
     */
    void clear_cache();
    
    /**
     * @brief Get cache statistics
     * @return Cache statistics JSON
     */
    QJsonObject get_cache_statistics() const;
    
    /**
     * @brief Set cache size limit
     * @param size_mb Maximum cache size in MB
     */
    void set_cache_size_limit(qint64 size_mb);
    
    /**
     * @brief Get current cache size
     * @return Cache size in bytes
     */
    qint64 get_cache_size() const;
    
    /**
     * @brief Clean up expired cache entries
     * @param max_age Maximum age for cache entries
     */
    void cleanup_expired_entries(std::chrono::hours max_age = std::chrono::hours{24 * 7});
    
signals:
    void cache_updated(const QString& plugin_id);
    void cache_cleared();
    void cache_size_limit_exceeded();
    
private slots:
    void perform_maintenance();
    
private:
    QString m_cache_directory;
    QString m_cache_index_file;
    qint64 m_cache_size_limit = 1024 * 1024 * 1024; // 1GB default
    std::unique_ptr<QTimer> m_maintenance_timer;
    
    mutable QReadWriteLock m_cache_lock;
    std::unordered_map<QString, RemotePluginCacheEntry> m_cache_index;
    
    QString get_plugin_cache_path(const QString& plugin_id) const;
    bool verify_cached_file(const RemotePluginCacheEntry& entry);
    void load_cache_index();
    void save_cache_index();
    void enforce_cache_size_limit();
    void update_access_time(const QString& plugin_id);
};

/**
 * @brief Remote plugin loader
 */
class RemotePluginLoader : public QObject {
    Q_OBJECT
    
public:
    explicit RemotePluginLoader(RemotePluginCache* cache, 
                               security::RemoteSecurityManager* security_manager,
                               QObject* parent = nullptr);
    ~RemotePluginLoader() override;
    
    /**
     * @brief Download and load plugin from URL
     * @param metadata Plugin metadata
     * @param load_options Plugin load options
     * @return Future with plugin instance
     */
    QFuture<expected<std::shared_ptr<IPlugin>, PluginError>> load_remote_plugin(
        const RemotePluginMetadata& metadata,
        const PluginLoadOptions& load_options = {}
    );
    
    /**
     * @brief Cancel plugin download/loading
     * @param plugin_id Plugin identifier
     */
    void cancel_loading(const QString& plugin_id);
    
    /**
     * @brief Get loading progress for plugin
     * @param plugin_id Plugin identifier
     * @return Progress information
     */
    std::optional<RemotePluginProgress> get_loading_progress(const QString& plugin_id);
    
    /**
     * @brief Set download timeout
     * @param timeout Timeout duration
     */
    void set_download_timeout(std::chrono::seconds timeout) { m_download_timeout = timeout; }
    
    /**
     * @brief Set maximum concurrent downloads
     * @param max_downloads Maximum number of concurrent downloads
     */
    void set_max_concurrent_downloads(int max_downloads) { m_max_concurrent_downloads = max_downloads; }
    
signals:
    void download_progress(const QString& plugin_id, const RemotePluginProgress& progress);
    void download_finished(const QString& plugin_id, bool success, const QString& error);
    void plugin_loaded(const QString& plugin_id, std::shared_ptr<IPlugin> plugin);
    void loading_error(const QString& plugin_id, const QString& error);
    
private slots:
    void handle_download_progress(qint64 bytes_received, qint64 bytes_total);
    void handle_download_finished();
    void handle_download_error(QNetworkReply::NetworkError error);
    
private:
    RemotePluginCache* m_cache;
    security::RemoteSecurityManager* m_security_manager;
    std::unique_ptr<QNetworkAccessManager> m_network_manager;
    std::unique_ptr<qtplugin::PluginManager> m_local_plugin_manager;
    
    // Download management
    std::chrono::seconds m_download_timeout{300}; // 5 minutes
    int m_max_concurrent_downloads = 3;
    mutable QMutex m_downloads_mutex;
    std::unordered_map<QString, std::unique_ptr<QNetworkReply>> m_active_downloads;
    std::unordered_map<QString, RemotePluginProgress> m_download_progress;
    std::unordered_map<QString, QFutureInterface<expected<std::shared_ptr<IPlugin>, PluginError>>> m_loading_futures;
    
    expected<QByteArray, PluginError> download_plugin_data(const RemotePluginMetadata& metadata);
    expected<std::shared_ptr<IPlugin>, PluginError> load_plugin_from_data(
        const RemotePluginMetadata& metadata,
        const QByteArray& plugin_data,
        const PluginLoadOptions& options
    );
    void update_progress(const QString& plugin_id, RemotePluginState state, 
                        const QString& operation = {}, const QString& error = {});
    QString get_temp_file_path(const QString& plugin_id) const;
};

/**
 * @brief Main remote plugin manager
 */
class RemotePluginManager : public QObject {
    Q_OBJECT
    
public:
    explicit RemotePluginManager(QObject* parent = nullptr);
    ~RemotePluginManager() override;
    
    /**
     * @brief Get singleton instance
     * @return Remote plugin manager instance
     */
    static RemotePluginManager& instance();
    
    /**
     * @brief Initialize remote plugin system
     * @param cache_directory Cache directory path
     * @param security_config Security configuration
     * @return Success or error
     */
    expected<void, PluginError> initialize(
        const QString& cache_directory,
        const security::RemoteSecurityConfig& security_config = {}
    );
    
    /**
     * @brief Add plugin repository
     * @param repository Repository configuration
     * @return Success or error
     */
    expected<void, PluginError> add_repository(const RemotePluginRepository& repository);
    
    /**
     * @brief Remove plugin repository
     * @param repository_id Repository identifier
     */
    void remove_repository(const QString& repository_id);
    
    /**
     * @brief Get all repositories
     * @return List of repositories
     */
    std::vector<RemotePluginRepository> get_repositories() const;
    
    /**
     * @brief Update repository metadata
     * @param repository_id Repository identifier
     * @return Future with success result
     */
    QFuture<expected<void, PluginError>> update_repository(const QString& repository_id);
    
    /**
     * @brief Search for plugins
     * @param query Search query
     * @param category Category filter (optional)
     * @param repository_id Repository filter (optional)
     * @return Future with search results
     */
    QFuture<std::vector<RemotePluginMetadata>> search_plugins(
        const QString& query,
        const QString& category = {},
        const QString& repository_id = {}
    );
    
    /**
     * @brief Get plugin metadata by ID
     * @param plugin_id Plugin identifier
     * @param repository_id Repository to search (optional)
     * @return Future with plugin metadata
     */
    QFuture<std::optional<RemotePluginMetadata>> get_plugin_metadata(
        const QString& plugin_id,
        const QString& repository_id = {}
    );
    
    /**
     * @brief Install remote plugin
     * @param plugin_id Plugin identifier
     * @param version Specific version (optional)
     * @param repository_id Repository to install from (optional)
     * @return Future with plugin instance
     */
    QFuture<expected<std::shared_ptr<IPlugin>, PluginError>> install_plugin(
        const QString& plugin_id,
        const QVersionNumber& version = {},
        const QString& repository_id = {}
    );
    
    /**
     * @brief Uninstall remote plugin
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    expected<void, PluginError> uninstall_plugin(const QString& plugin_id);
    
    /**
     * @brief Update plugin to latest version
     * @param plugin_id Plugin identifier
     * @return Future with updated plugin instance
     */
    QFuture<expected<std::shared_ptr<IPlugin>, PluginError>> update_plugin(const QString& plugin_id);
    
    /**
     * @brief Get installed remote plugins
     * @return List of installed plugin IDs
     */
    QStringList get_installed_plugins() const;
    
    /**
     * @brief Check for plugin updates
     * @return Future with list of plugins that have updates
     */
    QFuture<QStringList> check_for_updates();
    
    /**
     * @brief Set fallback local plugin manager
     * @param local_manager Local plugin manager for fallback
     */
    void set_fallback_manager(qtplugin::PluginManager* local_manager);
    
    /**
     * @brief Get cache manager
     * @return Cache manager instance
     */
    RemotePluginCache* get_cache() const { return m_cache.get(); }
    
    /**
     * @brief Get loader
     * @return Plugin loader instance
     */
    RemotePluginLoader* get_loader() const { return m_loader.get(); }
    
    /**
     * @brief Get security manager
     * @return Security manager instance
     */
    security::RemoteSecurityManager* get_security_manager() const { return m_security_manager.get(); }
    
signals:
    void repository_added(const QString& repository_id);
    void repository_removed(const QString& repository_id);
    void repository_updated(const QString& repository_id, bool success);
    void plugin_installed(const QString& plugin_id, const QVersionNumber& version);
    void plugin_uninstalled(const QString& plugin_id);
    void plugin_updated(const QString& plugin_id, const QVersionNumber& old_version, const QVersionNumber& new_version);
    void installation_progress(const QString& plugin_id, const RemotePluginProgress& progress);
    void installation_error(const QString& plugin_id, const QString& error);
    
private slots:
    void handle_repository_update_finished();
    void handle_plugin_installation_finished();
    void cleanup_temporary_files();
    
private:
    std::unique_ptr<RemotePluginCache> m_cache;
    std::unique_ptr<RemotePluginLoader> m_loader;
    std::unique_ptr<security::RemoteSecurityManager> m_security_manager;
    std::unique_ptr<QNetworkAccessManager> m_network_manager;
    std::unique_ptr<QTimer> m_cleanup_timer;
    qtplugin::PluginManager* m_fallback_manager = nullptr;
    
    // Repository management
    mutable QReadWriteLock m_repositories_lock;
    std::unordered_map<QString, RemotePluginRepository> m_repositories;
    std::unordered_map<QString, std::vector<RemotePluginMetadata>> m_repository_plugins;
    
    // Installation tracking
    mutable QMutex m_installations_mutex;
    std::unordered_set<QString> m_installed_plugins;
    std::unordered_map<QString, QVersionNumber> m_installed_versions;
    
    bool m_initialized = false;
    QString m_cache_directory;
    
    expected<RemotePluginMetadata, PluginError> fetch_plugin_metadata(
        const QString& plugin_id, const RemotePluginRepository& repository);
    expected<std::vector<RemotePluginMetadata>, PluginError> fetch_repository_catalog(
        const RemotePluginRepository& repository);
    void save_installation_state();
    void load_installation_state();
    QString get_repositories_config_path() const;
    QString get_installation_state_path() const;
};

} // namespace qtplugin::remote

Q_DECLARE_METATYPE(qtplugin::remote::RemotePluginState)
Q_DECLARE_METATYPE(qtplugin::remote::RemotePluginProgress)
Q_DECLARE_METATYPE(qtplugin::remote::RemotePluginMetadata)
Q_DECLARE_METATYPE(qtplugin::remote::RemotePluginRepository)
