/**
 * @file unified_plugin_manager.hpp
 * @brief Unified plugin manager integrating remote and local plugin systems
 * @version 3.2.0
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVersionNumber>
#include <QJsonObject>
#include <QFuture>
#include <QMutex>
#include <QReadWriteLock>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../../utils/error_handling.hpp"
#include "../../core/plugin_manager.hpp"
#include "../../core/plugin_interface.hpp"
#include "../core/remote_plugin_manager.hpp"
#include "../security/remote_security_manager.hpp"

namespace qtplugin::integration {

/**
 * @brief Plugin source type
 */
enum class PluginSource {
    Unknown = 0,
    Local = 1,      ///< Local plugin loaded from filesystem
    Remote = 2,     ///< Remote plugin downloaded and cached
    Fallback = 3    ///< Fallback to local when remote unavailable
};

/**
 * @brief Unified plugin information
 */
struct UnifiedPluginInfo {
    QString id;
    QString name;
    QVersionNumber version;
    QString description;
    QString author;
    QString license;
    QString category;
    QStringList tags;
    
    PluginSource source;
    QString source_location;  ///< Local file path or remote URL
    PluginState state;
    QDateTime load_time;
    QDateTime last_activity;
    
    // Remote-specific info (when applicable)
    std::optional<remote::RemotePluginMetadata> remote_metadata;
    bool has_updates = false;
    bool is_cached = false;
    
    // Local-specific info (when applicable)  
    std::optional<PluginInfo> local_info;
    
    QJsonObject to_json() const;
    static UnifiedPluginInfo from_json(const QJsonObject& json);
};

/**
 * @brief Plugin load strategy
 */
enum class LoadStrategy {
    PreferLocal = 0,    ///< Prefer local plugins over remote
    PreferRemote = 1,   ///< Prefer remote plugins over local
    LocalOnly = 2,      ///< Only load local plugins
    RemoteOnly = 3,     ///< Only load remote plugins
    BestVersion = 4     ///< Load best version (highest) regardless of source
};

/**
 * @brief Unified plugin load options
 */
struct UnifiedPluginLoadOptions {
    LoadStrategy strategy = LoadStrategy::PreferLocal;
    bool allow_fallback = true;
    bool cache_remote = true;
    bool check_for_updates = true;
    remote::RemoteSecurityLevel min_security_level = remote::RemoteSecurityLevel::Standard;
    
    // Pass-through to underlying managers
    PluginLoadOptions local_options;
    
    QJsonObject to_json() const;
    static UnifiedPluginLoadOptions from_json(const QJsonObject& json);
};

/**
 * @brief Plugin repository manager for unified access
 */
class UnifiedRepositoryManager : public QObject {
    Q_OBJECT
    
public:
    explicit UnifiedRepositoryManager(QObject* parent = nullptr);
    ~UnifiedRepositoryManager() override;
    
    /**
     * @brief Add plugin source directory (local)
     * @param directory Directory path
     * @param recursive Search recursively
     * @return Success or error
     */
    expected<void, PluginError> add_local_directory(const QString& directory, bool recursive = true);
    
    /**
     * @brief Add remote repository
     * @param repository Repository configuration
     * @return Success or error
     */
    expected<void, PluginError> add_remote_repository(const remote::RemotePluginRepository& repository);
    
    /**
     * @brief Remove source by ID
     * @param source_id Source identifier
     */
    void remove_source(const QString& source_id);
    
    /**
     * @brief Update all remote repositories
     * @return Future with update results
     */
    QFuture<std::vector<QString>> update_all_repositories();
    
    /**
     * @brief Search for plugins across all sources
     * @param query Search query
     * @param category Category filter
     * @param source_filter Source type filter
     * @return Future with unified search results
     */
    QFuture<std::vector<UnifiedPluginInfo>> search_plugins(
        const QString& query,
        const QString& category = {},
        PluginSource source_filter = PluginSource::Unknown
    );
    
    /**
     * @brief Get available plugin versions
     * @param plugin_id Plugin identifier
     * @return Available versions from all sources
     */
    QFuture<std::vector<std::pair<QVersionNumber, PluginSource>>> get_available_versions(
        const QString& plugin_id
    );
    
signals:
    void source_added(const QString& source_id, PluginSource type);
    void source_removed(const QString& source_id);
    void repositories_updated(const QStringList& successful, const QStringList& failed);
    
private:
    struct LocalSource {
        QString id;
        QString directory;
        bool recursive;
        QDateTime last_scanned;
    };
    
    mutable QReadWriteLock m_sources_lock;
    std::unordered_map<QString, LocalSource> m_local_sources;
    std::unordered_map<QString, remote::RemotePluginRepository> m_remote_repositories;
    
    QString generate_source_id(const QString& path_or_url) const;
};

/**
 * @brief Main unified plugin manager
 */
class UnifiedPluginManager : public QObject {
    Q_OBJECT
    
public:
    explicit UnifiedPluginManager(QObject* parent = nullptr);
    ~UnifiedPluginManager() override;
    
    /**
     * @brief Get singleton instance
     * @return Unified plugin manager instance
     */
    static UnifiedPluginManager& instance();
    
    /**
     * @brief Initialize unified plugin system
     * @param local_plugin_dirs Local plugin directories
     * @param remote_cache_dir Remote plugin cache directory
     * @param security_config Remote security configuration
     * @return Success or error
     */
    expected<void, PluginError> initialize(
        const QStringList& local_plugin_dirs,
        const QString& remote_cache_dir,
        const remote::security::RemoteSecurityConfig& security_config = {}
    );
    
    // === Core Plugin Management (Unified API) ===
    
    /**
     * @brief Load plugin with unified options
     * @param plugin_id Plugin identifier or path
     * @param options Unified load options
     * @return Future with plugin instance
     */
    QFuture<expected<std::shared_ptr<IPlugin>, PluginError>> load_plugin(
        const QString& plugin_id,
        const UnifiedPluginLoadOptions& options = {}
    );
    
    /**
     * @brief Unload plugin regardless of source
     * @param plugin_id Plugin identifier
     * @param force Force unload even if dependencies exist
     * @return Success or error
     */
    expected<void, PluginError> unload_plugin(const QString& plugin_id, bool force = false);
    
    /**
     * @brief Reload plugin with same options
     * @param plugin_id Plugin identifier
     * @param preserve_state Preserve plugin state
     * @return Future with reloaded plugin
     */
    QFuture<expected<std::shared_ptr<IPlugin>, PluginError>> reload_plugin(
        const QString& plugin_id,
        bool preserve_state = true
    );
    
    /**
     * @brief Get loaded plugin
     * @param plugin_id Plugin identifier
     * @return Plugin instance if loaded
     */
    std::shared_ptr<IPlugin> get_plugin(const QString& plugin_id);
    
    /**
     * @brief Check if plugin is loaded
     * @param plugin_id Plugin identifier
     * @return true if plugin is loaded
     */
    bool has_plugin(const QString& plugin_id);
    
    /**
     * @brief Get unified plugin information
     * @param plugin_id Plugin identifier
     * @return Plugin information
     */
    std::optional<UnifiedPluginInfo> get_plugin_info(const QString& plugin_id);
    
    /**
     * @brief Get all loaded plugins
     * @return Map of plugin ID to plugin instance
     */
    std::unordered_map<QString, std::shared_ptr<IPlugin>> get_all_plugins();
    
    /**
     * @brief Get loaded plugin IDs
     * @return List of loaded plugin IDs
     */
    QStringList get_plugin_ids();
    
    /**
     * @brief Get plugin count
     * @return Number of loaded plugins
     */
    size_t get_plugin_count() const;
    
    // === Plugin Discovery ===
    
    /**
     * @brief Discover plugins from local directories
     * @param directory Directory to search
     * @param recursive Search recursively
     * @return Discovered plugin paths
     */
    std::vector<std::filesystem::path> discover_local_plugins(
        const std::filesystem::path& directory,
        bool recursive = true
    );
    
    /**
     * @brief Search for plugins across all sources
     * @param query Search query
     * @param category Category filter
     * @param source_filter Source type filter
     * @return Future with search results
     */
    QFuture<std::vector<UnifiedPluginInfo>> search_plugins(
        const QString& query,
        const QString& category = {},
        PluginSource source_filter = PluginSource::Unknown
    );
    
    // === Plugin Installation and Updates ===
    
    /**
     * @brief Install plugin from best available source
     * @param plugin_id Plugin identifier
     * @param version Preferred version (latest if empty)
     * @param options Installation options
     * @return Future with installed plugin
     */
    QFuture<expected<std::shared_ptr<IPlugin>, PluginError>> install_plugin(
        const QString& plugin_id,
        const QVersionNumber& version = {},
        const UnifiedPluginLoadOptions& options = {}
    );
    
    /**
     * @brief Update plugin to latest version
     * @param plugin_id Plugin identifier
     * @return Future with updated plugin
     */
    QFuture<expected<std::shared_ptr<IPlugin>, PluginError>> update_plugin(const QString& plugin_id);
    
    /**
     * @brief Check for plugin updates
     * @param plugin_id Specific plugin (empty for all)
     * @return Future with list of plugins that have updates
     */
    QFuture<QStringList> check_for_updates(const QString& plugin_id = {});
    
    // === Repository Management ===
    
    /**
     * @brief Get repository manager
     * @return Repository manager instance
     */
    UnifiedRepositoryManager* get_repository_manager() const { return m_repository_manager.get(); }
    
    /**
     * @brief Add local plugin directory
     * @param directory Directory path
     * @param recursive Search recursively
     * @return Success or error
     */
    expected<void, PluginError> add_local_directory(const QString& directory, bool recursive = true);
    
    /**
     * @brief Add remote repository
     * @param repository Repository configuration
     * @return Success or error
     */
    expected<void, PluginError> add_remote_repository(const remote::RemotePluginRepository& repository);
    
    // === Advanced Features ===
    
    /**
     * @brief Set default load strategy
     * @param strategy Default strategy for plugin loading
     */
    void set_default_load_strategy(LoadStrategy strategy) { m_default_strategy = strategy; }
    
    /**
     * @brief Get default load strategy
     * @return Current default strategy
     */
    LoadStrategy get_default_load_strategy() const { return m_default_strategy; }
    
    /**
     * @brief Enable/disable automatic updates
     * @param enabled true to enable automatic updates
     * @param check_interval Update check interval
     */
    void set_automatic_updates(bool enabled, std::chrono::hours check_interval = std::chrono::hours{24});
    
    /**
     * @brief Get underlying local plugin manager
     * @return Local plugin manager instance
     */
    qtplugin::PluginManager* get_local_manager() const { return m_local_manager.get(); }
    
    /**
     * @brief Get underlying remote plugin manager
     * @return Remote plugin manager instance
     */
    remote::RemotePluginManager* get_remote_manager() const { return m_remote_manager.get(); }
    
    /**
     * @brief Get security manager
     * @return Remote security manager instance
     */
    remote::security::RemoteSecurityManager* get_security_manager() const;
    
    // === Compatibility API (matches existing PluginManager) ===
    
    /**
     * @brief Load plugins from directory (backward compatibility)
     * @param directory Directory path
     * @param options Load options
     * @return Number of successfully loaded plugins
     */
    size_t load_plugins_from_directory(
        const std::filesystem::path& directory,
        const PluginLoadOptions& options = {}
    );
    
    /**
     * @brief Shutdown all plugins
     */
    void shutdown_all_plugins();
    
    /**
     * @brief Enable hot reload for plugin
     * @param plugin_id Plugin identifier
     */
    void enable_hot_reload(const QString& plugin_id);
    
    /**
     * @brief Disable hot reload for plugin
     * @param plugin_id Plugin identifier  
     */
    void disable_hot_reload(const QString& plugin_id);
    
    /**
     * @brief Get system metrics
     * @return System metrics JSON
     */
    QJsonObject system_metrics();
    
signals:
    // Plugin lifecycle signals
    void plugin_loaded(const QString& plugin_id, PluginSource source);
    void plugin_unloaded(const QString& plugin_id);
    void plugin_reloaded(const QString& plugin_id);
    void plugin_error(const QString& plugin_id, const QString& error);
    
    // Installation signals
    void plugin_installed(const QString& plugin_id, const QVersionNumber& version, PluginSource source);
    void plugin_updated(const QString& plugin_id, const QVersionNumber& old_version, 
                       const QVersionNumber& new_version);
    void installation_progress(const QString& plugin_id, int percentage, const QString& status);
    
    // Repository signals
    void repositories_updated();
    void update_check_completed(const QStringList& available_updates);
    
    // Fallback signals
    void fallback_activated(const QString& plugin_id, const QString& reason);
    
private slots:
    void handle_local_plugin_loaded(const QString& plugin_id);
    void handle_local_plugin_unloaded(const QString& plugin_id);
    void handle_local_plugin_error(const QString& plugin_id, const QString& error);
    
    void handle_remote_plugin_installed(const QString& plugin_id, const QVersionNumber& version);
    void handle_remote_plugin_error(const QString& plugin_id, const QString& error);
    void handle_remote_installation_progress(const QString& plugin_id, const remote::RemotePluginProgress& progress);
    
    void perform_automatic_update_check();
    void cleanup_inactive_plugins();
    
private:
    std::unique_ptr<qtplugin::PluginManager> m_local_manager;
    std::unique_ptr<remote::RemotePluginManager> m_remote_manager;
    std::unique_ptr<UnifiedRepositoryManager> m_repository_manager;
    std::unique_ptr<QTimer> m_update_timer;
    std::unique_ptr<QTimer> m_cleanup_timer;
    
    // Plugin tracking
    mutable QReadWriteLock m_plugins_lock;
    std::unordered_map<QString, UnifiedPluginInfo> m_plugin_info;
    std::unordered_map<QString, PluginSource> m_plugin_sources;
    std::unordered_map<QString, UnifiedPluginLoadOptions> m_load_options;
    
    // Configuration
    LoadStrategy m_default_strategy = LoadStrategy::PreferLocal;
    bool m_automatic_updates_enabled = false;
    std::chrono::hours m_update_check_interval{24};
    
    bool m_initialized = false;
    
    // Helper methods
    expected<std::shared_ptr<IPlugin>, PluginError> try_load_local(
        const QString& plugin_id,
        const UnifiedPluginLoadOptions& options
    );
    
    QFuture<expected<std::shared_ptr<IPlugin>, PluginError>> try_load_remote(
        const QString& plugin_id,
        const UnifiedPluginLoadOptions& options
    );
    
    expected<std::shared_ptr<IPlugin>, PluginError> apply_load_strategy(
        const QString& plugin_id,
        const UnifiedPluginLoadOptions& options
    );
    
    void register_plugin(const QString& plugin_id, std::shared_ptr<IPlugin> plugin, 
                        PluginSource source, const UnifiedPluginLoadOptions& options);
    void unregister_plugin(const QString& plugin_id);
    
    UnifiedPluginInfo create_plugin_info(const QString& plugin_id, std::shared_ptr<IPlugin> plugin,
                                        PluginSource source);
    
    void setup_automatic_updates();
    void setup_cleanup_timer();
    
    QString get_plugin_cache_key(const QString& plugin_id, const QVersionNumber& version) const;
};

} // namespace qtplugin::integration

Q_DECLARE_METATYPE(qtplugin::integration::PluginSource)
Q_DECLARE_METATYPE(qtplugin::integration::LoadStrategy)
Q_DECLARE_METATYPE(qtplugin::integration::UnifiedPluginInfo)
Q_DECLARE_METATYPE(qtplugin::integration::UnifiedPluginLoadOptions)
