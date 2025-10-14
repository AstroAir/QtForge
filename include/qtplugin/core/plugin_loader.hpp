/**
 * @file plugin_loader.hpp
 * @brief Plugin loader interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "../interfaces/core/plugin_interface.hpp"
#include "../utils/concepts.hpp"
#include "../utils/error_handling.hpp"

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

    // Copy constructor and assignment operator (deleted - plugin loaders should
    // not be copied)
    QtPluginLoader(const QtPluginLoader& other) = delete;
    QtPluginLoader& operator=(const QtPluginLoader& other) = delete;

    // Move constructor and assignment operator
    QtPluginLoader(QtPluginLoader&& other) noexcept;
    QtPluginLoader& operator=(QtPluginLoader&& other) noexcept;

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
    class Impl;
    std::unique_ptr<Impl> d;
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
