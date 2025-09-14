/**
 * @file plugin_loader.hpp
 * @brief Plugin loader interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QPluginLoader>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../utils/concepts.hpp"
#include "../utils/error_handling.hpp"
#include "plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Plugin loader interface
 */
class IPluginLoader {
public:
    virtual ~IPluginLoader() = default;

    /**
     * @brief Check if a file can be loaded as a plugin
     * @param file_path Path to the plugin file
     * @return true if the file can be loaded
     */
    virtual bool can_load(const std::filesystem::path& file_path) const = 0;

    /**
     * @brief Load a plugin from file
     * @param file_path Path to the plugin file
     * @return Plugin instance or error information
     */
    virtual qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> load(
        const std::filesystem::path& file_path) = 0;

    /**
     * @brief Unload a plugin
     * @param plugin_id Plugin identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> unload(
        std::string_view plugin_id) = 0;

    /**
     * @brief Get supported file extensions
     * @return Vector of supported extensions (including the dot)
     */
    virtual std::vector<std::string> supported_extensions() const = 0;

    /**
     * @brief Get loader name/type
     * @return Loader identifier
     */
    virtual std::string_view name() const noexcept = 0;

    /**
     * @brief Check if loader supports hot reloading
     * @return true if hot reloading is supported
     */
    virtual bool supports_hot_reload() const noexcept = 0;
};

/**
 * @brief Default Qt plugin loader implementation
 */
class QtPluginLoader : public IPluginLoader {
public:
    QtPluginLoader();
    ~QtPluginLoader() override;

    // IPluginLoader implementation
    bool can_load(const std::filesystem::path& file_path) const override;
    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> load(
        const std::filesystem::path& file_path) override;
    qtplugin::expected<void, PluginError> unload(
        std::string_view plugin_id) override;
    std::vector<std::string> supported_extensions() const override;
    std::string_view name() const noexcept override;
    bool supports_hot_reload() const noexcept override;

    /**
     * @brief Get loaded plugin count
     * @return Number of currently loaded plugins
     */
    size_t loaded_plugin_count() const;

    /**
     * @brief Get loaded plugin IDs
     * @return Vector of loaded plugin identifiers
     */
    std::vector<std::string> loaded_plugins() const;

    /**
     * @brief Check if a plugin is loaded
     * @param plugin_id Plugin identifier
     * @return true if plugin is loaded
     */
    bool is_loaded(std::string_view plugin_id) const;

    // === Enhanced functionality (v3.2.0) ===
    
    /**
     * @brief Enable/disable metadata caching
     * @param enabled Whether to enable caching
     */
    void set_cache_enabled(bool enabled);
    
    /**
     * @brief Get cache statistics
     * @return Cache hit rate and other statistics
     */
    struct CacheStatistics {
        size_t hits = 0;
        size_t misses = 0;
        double hit_rate = 0.0;
        size_t cache_size = 0;
    };
    CacheStatistics get_cache_statistics() const;
    
    /**
     * @brief Clear metadata cache
     */
    void clear_cache();
    
    /**
     * @brief Get detailed error report
     * @return Error report string with stack trace
     */
    std::string get_error_report() const;
    
    /**
     * @brief Clear error history
     */
    void clear_error_history();
    
    /**
     * @brief Get plugin resource usage
     * @param plugin_id Plugin identifier
     * @return Resource usage information
     */
    struct ResourceUsage {
        size_t memory_bytes = 0;
        size_t handle_count = 0;
        std::chrono::milliseconds load_time{0};
        std::chrono::system_clock::time_point last_access;
    };
    ResourceUsage get_resource_usage(std::string_view plugin_id) const;
    
    // === Parallel Loading Features (v3.2.0) ===
    
    /**
     * @brief Batch loading result structure
     */
    struct BatchLoadResult {
        std::filesystem::path path;
        qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> result;
        std::chrono::milliseconds load_time;
    };
    
    /**
     * @brief Load multiple plugins in parallel
     * @param paths Vector of plugin file paths
     * @return Vector of load results with timing
     */
    std::vector<BatchLoadResult> batch_load(
        const std::vector<std::filesystem::path>& paths);
    
    /**
     * @brief Unload multiple plugins in parallel
     * @param plugin_ids Vector of plugin identifiers
     * @return Vector of unload results
     */
    std::vector<qtplugin::expected<void, PluginError>> batch_unload(
        const std::vector<std::string>& plugin_ids);
    
    /**
     * @brief Read metadata for multiple plugins in parallel
     * @param paths Vector of plugin file paths
     * @return Vector of metadata read results
     */
    std::vector<qtplugin::expected<QJsonObject, PluginError>> 
    batch_read_metadata(const std::vector<std::filesystem::path>& paths);
    
    /**
     * @brief Thread pool statistics
     */
    struct ThreadPoolStats {
        size_t queue_size = 0;
        size_t max_threads = 0;
    };
    
    /**
     * @brief Get thread pool statistics
     * @return Current thread pool statistics
     */
    ThreadPoolStats get_thread_pool_stats() const;
    
    /**
     * @brief Configure maximum loading threads
     * @param count Maximum number of concurrent loading threads
     */
    void set_max_loading_threads(size_t count);

private:
    struct LoadedPlugin {
        std::string id;
        std::filesystem::path file_path;
        std::unique_ptr<QPluginLoader> qt_loader;
        std::shared_ptr<IPlugin> instance;
        std::chrono::steady_clock::time_point load_time;
        std::atomic<int> ref_count{1};
        size_t estimated_memory = 0;
    };

    // === Error tracking ===
    struct ErrorEntry {
        std::chrono::system_clock::time_point timestamp;
        std::string function;
        std::string message;
        PluginErrorCode code;
    };
    
    // === Metadata cache ===
    struct CacheEntry {
        QJsonObject metadata;
        std::filesystem::file_time_type file_time;
        size_t file_size;
        std::chrono::steady_clock::time_point cache_time;
    };

    std::unordered_map<std::string, std::unique_ptr<LoadedPlugin>>
        m_loaded_plugins;
    mutable std::shared_mutex m_plugins_mutex;
    
    // Enhanced features
    mutable std::unordered_map<std::string, CacheEntry> m_metadata_cache;
    mutable std::shared_mutex m_cache_mutex;
    mutable std::vector<ErrorEntry> m_error_history;
    mutable std::mutex m_error_mutex;
    mutable std::atomic<size_t> m_cache_hits{0};
    mutable std::atomic<size_t> m_cache_misses{0};
    bool m_cache_enabled = true;
    static constexpr size_t MAX_ERROR_HISTORY = 100;
    static constexpr size_t MAX_CACHE_SIZE = 100;
    static constexpr auto CACHE_EXPIRY = std::chrono::minutes(10);

    // Helper methods
    qtplugin::expected<QJsonObject, PluginError> read_metadata(
        const std::filesystem::path& file_path) const;
    qtplugin::expected<QJsonObject, PluginError> read_metadata_cached(
        const std::filesystem::path& file_path) const;
    qtplugin::expected<QJsonObject, PluginError> read_metadata_impl(
        const std::filesystem::path& file_path) const;
    qtplugin::expected<std::string, PluginError> extract_plugin_id(
        const QJsonObject& metadata) const;
    bool is_valid_plugin_file(const std::filesystem::path& file_path) const;
    void track_error(const std::string& function, const std::string& message,
                     PluginErrorCode code = PluginErrorCode::Unknown) const;
    bool is_cache_valid(const std::filesystem::path& path, 
                        const CacheEntry& entry) const;
    void evict_oldest_cache_entry() const;
    
    // Parallel loading helpers
    std::vector<BatchLoadResult> batch_load_parallel(
        const std::vector<std::filesystem::path>& paths);
    void load_persistent_cache();
    void save_persistent_cache() const;
};

/**
 * @brief Plugin loader factory
 */
class PluginLoaderFactory {
public:
    /**
     * @brief Create default plugin loader
     * @return Unique pointer to plugin loader
     */
    static std::unique_ptr<IPluginLoader> create_default_loader();

    /**
     * @brief Create Qt plugin loader
     * @return Unique pointer to Qt plugin loader
     */
    static std::unique_ptr<QtPluginLoader> create_qt_loader();

    /**
     * @brief Register custom loader type
     * @param name Loader type name
     * @param factory Factory function for creating the loader
     */
    static void register_loader_type(
        std::string_view name,
        std::function<std::unique_ptr<IPluginLoader>()> factory);

    /**
     * @brief Create loader by name
     * @param name Loader type name
     * @return Unique pointer to plugin loader, or nullptr if not found
     */
    static std::unique_ptr<IPluginLoader> create_loader(std::string_view name);

    /**
     * @brief Get available loader types
     * @return Vector of available loader type names
     */
    static std::vector<std::string> available_loaders();

private:
    static std::unordered_map<std::string,
                              std::function<std::unique_ptr<IPluginLoader>()>>
        s_loader_factories;
    static std::mutex s_factory_mutex;
};

}  // namespace qtplugin
