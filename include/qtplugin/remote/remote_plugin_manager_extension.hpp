/**
 * @file remote_plugin_manager_extension.hpp
 * @brief Extension to PluginManager for remote plugin support
 * @version 3.0.0
 */

#pragma once

#include <QUrl>
#include <memory>
#include <string>
#include <vector>
#include "../core/plugin_manager.hpp"
#include "../utils/error_handling.hpp"
#include "http_plugin_loader.hpp"
#include "remote_plugin_configuration.hpp"
#include "remote_plugin_loader.hpp"

namespace qtplugin {

/**
 * @brief Remote plugin loading options extending PluginLoadOptions
 */
struct RemotePluginLoadOptions : public PluginLoadOptions {
    RemoteSecurityLevel remote_security_level = RemoteSecurityLevel::Standard;
    bool validate_remote_source = true;
    bool cache_remote_plugin = true;
    bool auto_update_remote = false;
    std::chrono::seconds download_timeout{60};
    
    /**
     * @brief Convert to RemotePluginLoadOptions for remote loaders
     */
    qtplugin::RemotePluginLoadOptions to_remote_options() const;
};

/**
 * @brief Extension class for PluginManager to support remote plugins
 * 
 * This class extends the functionality of PluginManager to support loading
 * plugins from remote sources while maintaining full backward compatibility.
 */
class RemotePluginManagerExtension {
public:
    /**
     * @brief Constructor
     * @param plugin_manager Base plugin manager to extend
     */
    explicit RemotePluginManagerExtension(std::shared_ptr<PluginManager> plugin_manager);
    
    /**
     * @brief Destructor
     */
    ~RemotePluginManagerExtension();

    // === Remote Plugin Loading ===
    
    /**
     * @brief Load plugin from remote URL
     * @param url Remote plugin URL
     * @param options Loading options
     * @return Plugin ID or error information
     */
    qtplugin::expected<std::string, PluginError> load_remote_plugin(
        const QUrl& url,
        const RemotePluginLoadOptions& options = {});
    
    /**
     * @brief Load plugin from remote source
     * @param source Remote plugin source
     * @param options Loading options
     * @return Plugin ID or error information
     */
    qtplugin::expected<std::string, PluginError> load_remote_plugin(
        const RemotePluginSource& source,
        const RemotePluginLoadOptions& options = {});
    
    /**
     * @brief Load plugin asynchronously from remote URL
     * @param url Remote plugin URL
     * @param options Loading options
     * @param progress_callback Progress callback function
     * @param completion_callback Completion callback function
     * @return Operation ID for tracking
     */
    QString load_remote_plugin_async(
        const QUrl& url,
        const RemotePluginLoadOptions& options = {},
        std::function<void(const DownloadProgress&)> progress_callback = nullptr,
        std::function<void(const qtplugin::expected<std::string, PluginError>&)> completion_callback = nullptr);
    
    /**
     * @brief Cancel remote plugin loading operation
     * @param operation_id Operation identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> cancel_remote_load(const QString& operation_id);

    // === Enhanced Plugin Loading (URL Detection) ===
    
    /**
     * @brief Enhanced load_plugin that supports both local files and remote URLs
     * @param path_or_url File path or remote URL
     * @param options Loading options
     * @return Plugin ID or error information
     */
    qtplugin::expected<std::string, PluginError> load_plugin(
        const std::string& path_or_url,
        const RemotePluginLoadOptions& options = {});

    // === Remote Source Management ===
    
    /**
     * @brief Add remote plugin source
     * @param source Remote plugin source
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> add_remote_source(const RemotePluginSource& source);
    
    /**
     * @brief Remove remote plugin source
     * @param source_id Source identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> remove_remote_source(const QString& source_id);
    
    /**
     * @brief Get all configured remote sources
     * @return Vector of remote plugin sources
     */
    std::vector<RemotePluginSource> get_remote_sources() const;

    // === Plugin Discovery ===
    
    /**
     * @brief Discover plugins from remote sources
     * @param source_id Optional source ID to limit discovery
     * @return Vector of discovered plugin metadata
     */
    qtplugin::expected<std::vector<QJsonObject>, PluginError> discover_remote_plugins(
        const QString& source_id = QString());
    
    /**
     * @brief Search for plugins across remote sources
     * @param query Search query
     * @param max_results Maximum number of results
     * @return Search results
     */
    qtplugin::expected<std::vector<QJsonObject>, PluginError> search_remote_plugins(
        const QString& query, int max_results = 50);

    // === Configuration ===
    
    /**
     * @brief Set remote plugin configuration
     * @param configuration Remote plugin configuration
     */
    void set_remote_configuration(std::shared_ptr<RemotePluginConfiguration> configuration);
    
    /**
     * @brief Get remote plugin configuration
     * @return Current remote plugin configuration
     */
    std::shared_ptr<RemotePluginConfiguration> remote_configuration() const;
    
    /**
     * @brief Enable/disable remote plugin support
     * @param enabled Whether remote plugin support is enabled
     */
    void set_remote_plugins_enabled(bool enabled);
    
    /**
     * @brief Check if remote plugin support is enabled
     * @return true if remote plugin support is enabled
     */
    bool is_remote_plugins_enabled() const;

    // === Statistics and Monitoring ===
    
    /**
     * @brief Get remote plugin statistics
     * @return Statistics as JSON object
     */
    QJsonObject get_remote_statistics() const;
    
    /**
     * @brief Get active remote operations
     * @return List of active operation IDs
     */
    std::vector<QString> get_active_remote_operations() const;

    // === Access to Base Manager ===
    
    /**
     * @brief Get the base plugin manager
     * @return Shared pointer to base plugin manager
     */
    std::shared_ptr<PluginManager> base_manager() const { return m_plugin_manager; }

private:
    // Core components
    std::shared_ptr<PluginManager> m_plugin_manager;
    std::shared_ptr<RemotePluginConfiguration> m_remote_configuration;
    std::unique_ptr<HttpPluginLoader> m_http_loader;
    std::shared_ptr<PluginDownloadManager> m_download_manager;
    std::shared_ptr<RemotePluginValidator> m_validator;
    
    // Remote plugin tracking
    mutable std::mutex m_remote_plugins_mutex;
    std::unordered_map<std::string, RemotePluginSource> m_remote_plugin_sources;  // plugin_id -> source
    std::unordered_map<QString, std::string> m_async_operations;  // operation_id -> plugin_id
    
    // Configuration
    bool m_remote_plugins_enabled = true;
    
    // Helper methods
    bool is_url(const std::string& path_or_url) const;
    QUrl parse_url(const std::string& url_string) const;
    qtplugin::expected<std::string, PluginError> load_from_cached_file(
        const std::filesystem::path& cached_path, const RemotePluginLoadOptions& options);
    
    void initialize_remote_components();
    void register_remote_loaders();
    void setup_async_callbacks();
    
    // Async operation management
    QString generate_operation_id() const;
    void track_async_operation(const QString& operation_id, const std::string& plugin_id);
    void untrack_async_operation(const QString& operation_id);
    
    // Integration helpers
    PluginLoadOptions convert_to_base_options(const RemotePluginLoadOptions& remote_options) const;
    RemotePluginLoadOptions convert_from_base_options(const PluginLoadOptions& base_options) const;
};

/**
 * @brief Factory for creating enhanced plugin managers with remote support
 */
class RemotePluginManagerFactory {
public:
    /**
     * @brief Create plugin manager with default remote support
     * @return Enhanced plugin manager
     */
    static std::unique_ptr<RemotePluginManagerExtension> create_with_remote_support();
    
    /**
     * @brief Create plugin manager with custom remote configuration
     * @param remote_config Remote plugin configuration
     * @return Enhanced plugin manager
     */
    static std::unique_ptr<RemotePluginManagerExtension> create_with_remote_config(
        std::shared_ptr<RemotePluginConfiguration> remote_config);
    
    /**
     * @brief Enhance existing plugin manager with remote support
     * @param base_manager Existing plugin manager
     * @return Enhanced plugin manager
     */
    static std::unique_ptr<RemotePluginManagerExtension> enhance_existing_manager(
        std::shared_ptr<PluginManager> base_manager);
    
    /**
     * @brief Create enterprise plugin manager with remote support
     * @return Enhanced plugin manager with enterprise configuration
     */
    static std::unique_ptr<RemotePluginManagerExtension> create_enterprise();
};

/**
 * @brief Convenience function to check if a string is a URL
 * @param path_or_url String to check
 * @return true if the string appears to be a URL
 */
bool is_plugin_url(const std::string& path_or_url);

/**
 * @brief Convenience function to load plugin from path or URL
 * @param manager Plugin manager (with or without remote support)
 * @param path_or_url File path or remote URL
 * @param options Loading options
 * @return Plugin ID or error information
 */
qtplugin::expected<std::string, PluginError> load_plugin_from_path_or_url(
    PluginManager& manager,
    const std::string& path_or_url,
    const PluginLoadOptions& options = {});

}  // namespace qtplugin
