/**
 * @file remote_plugin_registry_extension.hpp
 * @brief Extension to PluginRegistry for remote plugin support
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QUrl>
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/plugin_manager.hpp"
#include "../core/plugin_registry.hpp"
#include "../utils/error_handling.hpp"
#include "remote_plugin_source.hpp"

namespace qtplugin {

/**
 * @brief Extended plugin information for remote plugins
 */
struct RemotePluginInfo : public PluginInfo {
    // Remote-specific fields
    std::optional<RemotePluginSource>
        remote_source;                 ///< Source of remote plugin
    std::optional<QUrl> original_url;  ///< Original download URL
    std::optional<std::filesystem::path> cached_path;  ///< Local cache path
    std::chrono::system_clock::time_point
        download_time;  ///< When plugin was downloaded
    std::chrono::system_clock::time_point
        last_update_check;  ///< Last update check time
    std::optional<std::string>
        remote_version;                   ///< Latest available remote version
    std::optional<std::string> checksum;  ///< Plugin file checksum
    bool auto_update_enabled = false;     ///< Whether auto-update is enabled
    bool is_cached = false;               ///< Whether plugin is cached locally
    QJsonObject remote_metadata;          ///< Additional remote metadata

    /**
     * @brief Convert to JSON representation including remote fields
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static qtplugin::expected<RemotePluginInfo, PluginError> from_json(
        const QJsonObject& json);

    /**
     * @brief Check if plugin needs update
     */
    bool needs_update() const;

    /**
     * @brief Get age of cached plugin
     */
    std::chrono::seconds cache_age() const;
};

/**
 * @brief Plugin discovery result from remote sources
 */
struct RemotePluginDiscoveryResult {
    std::string plugin_id;
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::string category;
    std::vector<std::string> tags;
    QUrl download_url;
    RemotePluginSource source;
    QJsonObject metadata;
    std::optional<std::string> checksum;
    std::optional<qint64> file_size;
    std::optional<double> rating;
    std::optional<int> download_count;

    /**
     * @brief Default constructor
     */
    RemotePluginDiscoveryResult() : source(QUrl("http://example.com")) {}

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static qtplugin::expected<RemotePluginDiscoveryResult, PluginError>
    from_json(const QJsonObject& json);
};

/**
 * @brief Search criteria for remote plugin discovery
 */
struct RemotePluginSearchCriteria {
    std::optional<std::string> query;          ///< Search query
    std::optional<std::string> category;       ///< Plugin category filter
    std::vector<std::string> tags;             ///< Required tags
    std::optional<std::string> author;         ///< Author filter
    std::optional<double> min_rating;          ///< Minimum rating
    std::optional<std::string> license;        ///< License filter
    std::optional<std::string> version_range;  ///< Version range (semver)
    int max_results = 50;                      ///< Maximum results to return
    int offset = 0;                            ///< Result offset for pagination
    std::string sort_by = "relevance";         ///< Sort criteria
    bool sort_ascending = false;               ///< Sort direction

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static RemotePluginSearchCriteria from_json(const QJsonObject& json);
};

/**
 * @brief Extension interface for remote plugin registry operations
 */
class IRemotePluginRegistry : public IPluginRegistry {
public:
    virtual ~IRemotePluginRegistry() = default;

    // === Remote Plugin Registration ===

    /**
     * @brief Register a remote plugin in the registry
     * @param plugin_id Unique plugin identifier
     * @param remote_plugin_info Remote plugin information structure
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> register_remote_plugin(
        const std::string& plugin_id,
        std::unique_ptr<RemotePluginInfo> remote_plugin_info) = 0;

    /**
     * @brief Get remote plugin information by ID
     * @param plugin_id Plugin identifier
     * @return Remote plugin information or nullopt if not found
     */
    virtual std::optional<RemotePluginInfo> get_remote_plugin_info(
        const std::string& plugin_id) const = 0;

    /**
     * @brief Get all remote plugin information
     * @return Vector of remote plugin information structures
     */
    virtual std::vector<RemotePluginInfo> get_all_remote_plugin_info()
        const = 0;

    // === Remote Plugin Discovery ===

    /**
     * @brief Discover plugins from remote sources
     * @param criteria Search criteria
     * @return Vector of discovered plugins
     */
    virtual qtplugin::expected<std::vector<RemotePluginDiscoveryResult>,
                               PluginError>
    discover_remote_plugins(
        const RemotePluginSearchCriteria& criteria = {}) const = 0;

    /**
     * @brief Search for plugins across all configured remote sources
     * @param query Search query
     * @param max_results Maximum number of results
     * @return Search results
     */
    virtual qtplugin::expected<std::vector<RemotePluginDiscoveryResult>,
                               PluginError>
    search_remote_plugins(const std::string& query,
                          int max_results = 50) const = 0;

    // === Remote Source Management ===

    /**
     * @brief Add remote plugin source
     * @param source Remote plugin source
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> add_remote_source(
        const RemotePluginSource& source) = 0;

    /**
     * @brief Remove remote plugin source
     * @param source_id Source identifier
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> remove_remote_source(
        const std::string& source_id) = 0;

    /**
     * @brief Get all configured remote sources
     * @return Vector of remote plugin sources
     */
    virtual std::vector<RemotePluginSource> get_remote_sources() const = 0;

    // === Cache Management ===

    /**
     * @brief Clear cached remote plugins
     * @param older_than_days Clear plugins cached longer than specified days (0
     * = all)
     * @return Number of plugins cleared
     */
    virtual int clear_remote_cache(int older_than_days = 0) = 0;

    /**
     * @brief Get cache statistics
     * @return Cache statistics as JSON object
     */
    virtual QJsonObject get_cache_statistics() const = 0;

    // === Update Management ===

    /**
     * @brief Check for updates for all remote plugins
     * @return Vector of plugin IDs that have updates available
     */
    virtual qtplugin::expected<std::vector<std::string>, PluginError>
    check_for_updates() const = 0;

    /**
     * @brief Check for update for specific plugin
     * @param plugin_id Plugin identifier
     * @return true if update is available
     */
    virtual qtplugin::expected<bool, PluginError> check_plugin_update(
        const std::string& plugin_id) const = 0;

    /**
     * @brief Enable/disable auto-update for a plugin
     * @param plugin_id Plugin identifier
     * @param enabled Whether auto-update should be enabled
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> set_auto_update(
        const std::string& plugin_id, bool enabled) = 0;
};

/**
 * @brief Enhanced plugin registry with remote plugin support
 */
class RemotePluginRegistry : public PluginRegistry,
                             public IRemotePluginRegistry {
    Q_OBJECT

public:
    explicit RemotePluginRegistry(QObject* parent = nullptr);
    ~RemotePluginRegistry() override;

    // === IRemotePluginRegistry Implementation ===

    qtplugin::expected<void, PluginError> register_remote_plugin(
        const std::string& plugin_id,
        std::unique_ptr<RemotePluginInfo> remote_plugin_info) override;

    std::optional<RemotePluginInfo> get_remote_plugin_info(
        const std::string& plugin_id) const override;

    std::vector<RemotePluginInfo> get_all_remote_plugin_info() const override;

    qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
    discover_remote_plugins(
        const RemotePluginSearchCriteria& criteria = {}) const override;

    qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
    search_remote_plugins(const std::string& query,
                          int max_results = 50) const override;

    qtplugin::expected<void, PluginError> add_remote_source(
        const RemotePluginSource& source) override;

    qtplugin::expected<void, PluginError> remove_remote_source(
        const std::string& source_id) override;

    std::vector<RemotePluginSource> get_remote_sources() const override;

    int clear_remote_cache(int older_than_days = 0) override;
    QJsonObject get_cache_statistics() const override;

    qtplugin::expected<std::vector<std::string>, PluginError>
    check_for_updates() const override;

    qtplugin::expected<bool, PluginError> check_plugin_update(
        const std::string& plugin_id) const override;

    qtplugin::expected<void, PluginError> set_auto_update(
        const std::string& plugin_id, bool enabled) override;

    // === Enhanced Base Registry Methods ===

    /**
     * @brief Enhanced plugin info retrieval that includes remote information
     */
    std::optional<PluginInfo> get_plugin_info(
        const std::string& plugin_id) const override;

    /**
     * @brief Get all plugin information including remote plugins
     */
    std::vector<PluginInfo> get_all_plugin_info() const override;

signals:
    /**
     * @brief Emitted when a remote plugin is discovered
     */
    void remote_plugin_discovered(const QString& plugin_id,
                                  const QJsonObject& metadata);

    /**
     * @brief Emitted when remote plugin update is available
     */
    void remote_plugin_update_available(const QString& plugin_id,
                                        const QString& new_version);

    /**
     * @brief Emitted when remote source is added
     */
    void remote_source_added(const QString& source_id);

    /**
     * @brief Emitted when remote source is removed
     */
    void remote_source_removed(const QString& source_id);

private:
    // Remote plugin storage
    mutable std::shared_mutex m_remote_plugins_mutex;
    std::unordered_map<std::string, std::unique_ptr<RemotePluginInfo>>
        m_remote_plugins;

    // Remote sources
    mutable std::shared_mutex m_remote_sources_mutex;
    std::unordered_map<std::string, RemotePluginSource> m_remote_sources;

    // Cache management
    std::filesystem::path m_cache_directory;
    mutable std::shared_mutex m_cache_mutex;

    // Helper methods
    RemotePluginInfo create_remote_plugin_info_copy(
        const RemotePluginInfo& original) const;
    bool is_remote_plugin(const std::string& plugin_id) const;
    void initialize_cache_directory();
    void cleanup_expired_cache_entries();

    // Discovery helpers
    qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
    discover_from_source(const RemotePluginSource& source,
                         const RemotePluginSearchCriteria& criteria) const;

    // Update checking helpers
    qtplugin::expected<std::optional<std::string>, PluginError>
    get_latest_version_from_source(const std::string& plugin_id,
                                   const RemotePluginSource& source) const;
};

}  // namespace qtplugin
