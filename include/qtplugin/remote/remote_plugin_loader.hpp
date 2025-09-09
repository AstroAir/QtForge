/**
 * @file remote_plugin_loader.hpp
 * @brief Remote plugin loader interface and implementations
 * @version 3.0.0
 */

#pragma once

#include <QUrl>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/plugin_loader.hpp"
#include "../utils/error_handling.hpp"
#include "plugin_download_manager.hpp"
#include "remote_plugin_configuration.hpp"
#include "remote_plugin_source.hpp"
#include "remote_plugin_validator.hpp"

namespace qtplugin {

/**
 * @brief Remote plugin loading options
 */
struct RemotePluginLoadOptions {
    DownloadOptions download_options;
    RemoteSecurityLevel security_level = RemoteSecurityLevel::Standard;
    bool validate_source = true;
    bool validate_plugin = true;
    bool cache_plugin = true;
    bool auto_update = false;
    std::chrono::seconds validation_timeout{30};

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static RemotePluginLoadOptions from_json(const QJsonObject& json);
};

/**
 * @brief Remote plugin loading result
 */
struct RemotePluginLoadResult {
    std::shared_ptr<IPlugin> plugin;
    RemotePluginSource source;
    DownloadResult download_result;
    ValidationResult validation_result;
    std::filesystem::path cached_path;
    std::chrono::system_clock::time_point load_time;
    QJsonObject metadata;

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
};

/**
 * @brief Interface for remote plugin loaders
 *
 * Extends the base IPluginLoader interface to support loading plugins
 * from remote sources with additional security and caching features.
 */
class IRemotePluginLoader : public IPluginLoader {
public:
    virtual ~IRemotePluginLoader() = default;

    // === Remote Loading Methods ===

    /**
     * @brief Check if a remote URL can be loaded
     * @param url Remote plugin URL
     * @return true if the URL can be loaded
     */
    virtual bool can_load_remote(const QUrl& url) const = 0;

    /**
     * @brief Load a plugin from remote source
     * @param source Remote plugin source
     * @param options Loading options
     * @return Plugin instance or error information
     */
    virtual qtplugin::expected<RemotePluginLoadResult, PluginError> load_remote(
        const RemotePluginSource& source,
        const RemotePluginLoadOptions& options = {}) = 0;

    /**
     * @brief Load a plugin from remote URL
     * @param url Remote plugin URL
     * @param options Loading options
     * @return Plugin instance or error information
     */
    virtual qtplugin::expected<RemotePluginLoadResult, PluginError> load_remote(
        const QUrl& url, const RemotePluginLoadOptions& options = {}) = 0;

    /**
     * @brief Load plugin asynchronously from remote source
     * @param source Remote plugin source
     * @param options Loading options
     * @param progress_callback Progress callback function
     * @param completion_callback Completion callback function
     * @return Loading operation ID
     */
    virtual QString load_remote_async(
        const RemotePluginSource& source,
        const RemotePluginLoadOptions& options = {},
        std::function<void(const DownloadProgress&)> progress_callback =
            nullptr,
        std::function<void(
            const qtplugin::expected<RemotePluginLoadResult, PluginError>&)>
            completion_callback = nullptr) = 0;

    /**
     * @brief Cancel remote loading operation
     * @param operation_id Loading operation ID
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> cancel_remote_load(
        const QString& operation_id) = 0;

    // === Source Management ===

    /**
     * @brief Add a remote plugin source
     * @param source Remote plugin source to add
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> add_source(
        const RemotePluginSource& source) = 0;

    /**
     * @brief Remove a remote plugin source
     * @param source_id Source identifier
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> remove_source(
        const QString& source_id) = 0;

    /**
     * @brief Get all configured sources
     * @return Vector of remote plugin sources
     */
    virtual std::vector<RemotePluginSource> get_sources() const = 0;

    /**
     * @brief Find source by URL
     * @param url Plugin URL
     * @return Source if found
     */
    virtual std::optional<RemotePluginSource> find_source_for_url(
        const QUrl& url) const = 0;

    // === Discovery Methods ===

    /**
     * @brief Discover available plugins from a source
     * @param source Remote plugin source
     * @return List of available plugins or error
     */
    virtual qtplugin::expected<std::vector<QJsonObject>, PluginError>
    discover_plugins(const RemotePluginSource& source) = 0;

    /**
     * @brief Search for plugins across all sources
     * @param query Search query
     * @param max_results Maximum number of results
     * @return Search results or error
     */
    virtual qtplugin::expected<std::vector<QJsonObject>, PluginError>
    search_plugins(const QString& query, int max_results = 50) = 0;

    // === Configuration ===

    /**
     * @brief Set remote plugin configuration
     * @param configuration Configuration to use
     */
    virtual void set_configuration(
        std::shared_ptr<RemotePluginConfiguration> configuration) = 0;

    /**
     * @brief Get remote plugin configuration
     * @return Current configuration
     */
    virtual std::shared_ptr<RemotePluginConfiguration> configuration()
        const = 0;

    /**
     * @brief Set download manager
     * @param download_manager Download manager to use
     */
    virtual void set_download_manager(
        std::shared_ptr<PluginDownloadManager> download_manager) = 0;

    /**
     * @brief Get download manager
     * @return Current download manager
     */
    virtual std::shared_ptr<PluginDownloadManager> download_manager() const = 0;

    /**
     * @brief Set validator
     * @param validator Validator to use
     */
    virtual void set_validator(
        std::shared_ptr<RemotePluginValidator> validator) = 0;

    /**
     * @brief Get validator
     * @return Current validator
     */
    virtual std::shared_ptr<RemotePluginValidator> validator() const = 0;

    // === Statistics and Monitoring ===

    /**
     * @brief Get loading statistics
     * @return Statistics as JSON object
     */
    virtual QJsonObject get_statistics() const = 0;

    /**
     * @brief Reset statistics
     */
    virtual void reset_statistics() = 0;

    /**
     * @brief Get active loading operations
     * @return List of active operation IDs
     */
    virtual std::vector<QString> get_active_operations() const = 0;
};

/**
 * @brief Base implementation of remote plugin loader
 */
class RemotePluginLoaderBase : public IRemotePluginLoader {
public:
    /**
     * @brief Constructor
     * @param configuration Remote plugin configuration
     * @param download_manager Download manager
     * @param validator Plugin validator
     */
    explicit RemotePluginLoaderBase(
        std::shared_ptr<RemotePluginConfiguration> configuration = nullptr,
        std::shared_ptr<PluginDownloadManager> download_manager = nullptr,
        std::shared_ptr<RemotePluginValidator> validator = nullptr);

    /**
     * @brief Destructor
     */
    ~RemotePluginLoaderBase() override;

    // === IPluginLoader Implementation ===

    bool can_load(const std::filesystem::path& file_path) const override;
    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> load(
        const std::filesystem::path& file_path) override;
    qtplugin::expected<void, PluginError> unload(
        std::string_view plugin_id) override;
    std::vector<std::string> supported_extensions() const override;
    std::string_view name() const noexcept override;
    bool supports_hot_reload() const noexcept override;

    // === IRemotePluginLoader Implementation ===

    bool can_load_remote(const QUrl& url) const override;
    qtplugin::expected<RemotePluginLoadResult, PluginError> load_remote(
        const RemotePluginSource& source,
        const RemotePluginLoadOptions& options = {}) override;
    qtplugin::expected<RemotePluginLoadResult, PluginError> load_remote(
        const QUrl& url, const RemotePluginLoadOptions& options = {}) override;
    qtplugin::expected<void, PluginError> add_source(
        const RemotePluginSource& source) override;
    qtplugin::expected<void, PluginError> remove_source(
        const QString& source_id) override;
    std::vector<RemotePluginSource> get_sources() const override;
    std::optional<RemotePluginSource> find_source_for_url(
        const QUrl& url) const override;

    void set_configuration(
        std::shared_ptr<RemotePluginConfiguration> configuration) override;
    std::shared_ptr<RemotePluginConfiguration> configuration() const override;
    void set_download_manager(
        std::shared_ptr<PluginDownloadManager> download_manager) override;
    std::shared_ptr<PluginDownloadManager> download_manager() const override;
    void set_validator(
        std::shared_ptr<RemotePluginValidator> validator) override;
    std::shared_ptr<RemotePluginValidator> validator() const override;

    QJsonObject get_statistics() const override;
    void reset_statistics() override;
    std::vector<QString> get_active_operations() const override;

protected:
    // Core components
    std::shared_ptr<RemotePluginConfiguration> m_configuration;
    std::shared_ptr<PluginDownloadManager> m_download_manager;
    std::shared_ptr<RemotePluginValidator> m_validator;

    // Local plugin loader for handling downloaded plugins
    std::unique_ptr<IPluginLoader> m_local_loader;

    // Active operations tracking
    mutable std::mutex m_operations_mutex;
    std::unordered_map<QString, QJsonObject> m_active_operations;

    // Statistics
    mutable std::mutex m_stats_mutex;
    std::atomic<int> m_remote_loads_attempted{0};
    std::atomic<int> m_remote_loads_successful{0};
    std::atomic<int> m_remote_loads_failed{0};
    std::atomic<int> m_cache_hits{0};

    // Helper methods
    virtual qtplugin::expected<RemotePluginLoadResult, PluginError>
    perform_remote_load(const RemotePluginSource& source,
                        const RemotePluginLoadOptions& options) = 0;

    QString generate_operation_id() const;
    void track_operation(const QString& operation_id, const QJsonObject& info);
    void untrack_operation(const QString& operation_id);

    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> load_from_cache(
        const std::filesystem::path& cached_path);

    void initialize_components();
    void validate_configuration() const;
};

}  // namespace qtplugin
