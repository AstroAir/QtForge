/**
 * @file platform_plugin_loader.hpp
 * @brief Platform-specific plugin loading optimizations
 * @version 3.0.0
 */

#pragma once

#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QLibrary>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QStringList>
#include <chrono>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>
#include "../interfaces/core/plugin_interface.hpp"
#include "../core/plugin_loader.hpp"
#include "../utils/error_handling.hpp"

// Platform-specific includes
#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
// Note: Temporarily disable problematic Windows headers for compilation
// #include <psapi.h>
// #include <tlhelp32.h>
#endif

#ifdef Q_OS_UNIX
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>
#endif

namespace qtplugin {

/**
 * @brief Platform-specific loading strategies
 */
enum class PlatformLoadingStrategy {
    Default,          ///< Default Qt loading
    MemoryMapped,     ///< Memory-mapped file loading
    LazyLoading,      ///< Lazy symbol resolution
    PreloadSymbols,   ///< Preload all symbols
    OptimizedSearch,  ///< Optimized library search
    CachedMetadata    ///< Cached plugin metadata
};

/**
 * @brief Platform-specific optimization flags
 */
enum class PlatformOptimization : uint32_t {
    None = 0x0000,
    FastDiscovery = 0x0001,         ///< Fast plugin discovery
    MemoryMapping = 0x0002,         ///< Memory-mapped loading
    SymbolCaching = 0x0004,         ///< Symbol caching
    MetadataCaching = 0x0008,       ///< Metadata caching
    ParallelLoading = 0x0010,       ///< Parallel plugin loading
    LazyInitialization = 0x0020,    ///< Lazy initialization
    CompressedStorage = 0x0040,     ///< Compressed plugin storage
    SecurityValidation = 0x0080,    ///< Enhanced security validation
    PerformanceMonitoring = 0x0100  ///< Performance monitoring
};

using PlatformOptimizations = std::underlying_type_t<PlatformOptimization>;

/**
 * @brief Platform-specific plugin information
 */
struct PlatformPluginInfo {
    QString file_path;       ///< Plugin file path
    QString platform;        ///< Target platform
    QString architecture;    ///< Target architecture
    uint64_t file_size = 0;  ///< File size in bytes
    std::chrono::system_clock::time_point
        modification_time;            ///< Last modification time
    QString file_hash;                ///< File hash for integrity
    QJsonObject platform_metadata;    ///< Platform-specific metadata
    QJsonObject performance_metrics;  ///< Performance metrics
    bool is_memory_mapped = false;    ///< Whether plugin is memory-mapped
    void* memory_address = nullptr;   ///< Memory address if mapped
    size_t memory_size = 0;           ///< Memory size if mapped

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static PlatformPluginInfo from_json(const QJsonObject& json);
};

/**
 * @brief Platform-specific loading configuration
 */
struct PlatformLoadingConfig {
    PlatformLoadingStrategy strategy = PlatformLoadingStrategy::Default;
    PlatformOptimizations optimizations = 0;
    QString cache_directory;     ///< Cache directory for metadata
    int max_parallel_loads = 4;  ///< Maximum parallel loads
    std::chrono::milliseconds load_timeout{30000};  ///< Loading timeout
    bool enable_symbol_prefetch = true;    ///< Enable symbol prefetching
    bool enable_metadata_cache = true;     ///< Enable metadata caching
    bool enable_security_checks = true;    ///< Enable security checks
    QJsonObject platform_specific_config;  ///< Platform-specific configuration

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static PlatformLoadingConfig from_json(const QJsonObject& json);
};

/**
 * @brief Platform-specific loading statistics
 */
struct PlatformLoadingStatistics {
    uint64_t total_plugins_loaded = 0;    ///< Total plugins loaded
    uint64_t total_plugins_failed = 0;    ///< Total plugins failed to load
    uint64_t memory_mapped_plugins = 0;   ///< Memory-mapped plugins
    uint64_t cached_metadata_hits = 0;    ///< Metadata cache hits
    uint64_t cached_metadata_misses = 0;  ///< Metadata cache misses
    std::chrono::milliseconds total_load_time{0};    ///< Total loading time
    std::chrono::milliseconds average_load_time{0};  ///< Average loading time
    uint64_t total_memory_used = 0;                  ///< Total memory used
    std::unordered_map<QString, uint64_t>
        platform_stats;  ///< Platform-specific stats

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Platform-optimized plugin loader
 *
 * This class provides platform-specific optimizations for plugin loading
 * including memory-mapped files, symbol caching, and parallel loading.
 */
class PlatformPluginLoader : public IPluginLoader {
    Q_OBJECT

public:
    explicit PlatformPluginLoader(QObject* parent = nullptr);
    ~PlatformPluginLoader() override;

    // === IPluginLoader Implementation ===

    bool can_load(const std::filesystem::path& file_path) const override;

    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> load(
        const std::filesystem::path& file_path) override;

    qtplugin::expected<void, PluginError> unload(
        std::string_view plugin_id) override;

    std::vector<std::string> supported_extensions() const override;

    std::string_view name() const noexcept override;

    bool supports_hot_reload() const noexcept override;

    // === Platform-Specific Configuration ===

    /**
     * @brief Set platform loading configuration
     * @param config Loading configuration
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_loading_config(
        const PlatformLoadingConfig& config);

    /**
     * @brief Get platform loading configuration
     * @return Current loading configuration
     */
    PlatformLoadingConfig get_loading_config() const;

    /**
     * @brief Enable platform optimization
     * @param optimization Optimization to enable
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> enable_optimization(
        PlatformOptimization optimization);

    /**
     * @brief Disable platform optimization
     * @param optimization Optimization to disable
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> disable_optimization(
        PlatformOptimization optimization);

    /**
     * @brief Check if optimization is enabled
     * @param optimization Optimization to check
     * @return true if optimization is enabled
     */
    bool is_optimization_enabled(PlatformOptimization optimization) const;

    // === Memory-Mapped Loading ===

    /**
     * @brief Load plugin using memory mapping
     * @param file_path Plugin file path
     * @return Loaded plugin or error
     */
    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError>
    load_memory_mapped(const std::filesystem::path& file_path);

    /**
     * @brief Unload memory-mapped plugin
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unload_memory_mapped(
        std::string_view plugin_id);

    /**
     * @brief Get memory mapping information
     * @param plugin_id Plugin identifier
     * @return Memory mapping info or error
     */
    qtplugin::expected<QJsonObject, PluginError> get_memory_mapping_info(
        std::string_view plugin_id) const;

    // === Parallel Loading ===

    /**
     * @brief Load multiple plugins in parallel
     * @param file_paths Vector of plugin file paths
     * @param max_parallel Maximum parallel loads
     * @return Vector of loading results
     */
    std::vector<qtplugin::expected<std::shared_ptr<IPlugin>, PluginError>>
    load_parallel(const std::vector<std::filesystem::path>& file_paths,
                  int max_parallel = 4);

    /**
     * @brief Load plugins from directory in parallel
     * @param directory Directory path
     * @param recursive Whether to search recursively
     * @param max_parallel Maximum parallel loads
     * @return Vector of loading results
     */
    std::vector<qtplugin::expected<std::shared_ptr<IPlugin>, PluginError>>
    load_directory_parallel(const std::filesystem::path& directory,
                            bool recursive = true, int max_parallel = 4);

    // === Metadata Caching ===

    /**
     * @brief Enable metadata caching
     * @param cache_directory Cache directory path
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> enable_metadata_cache(
        const QString& cache_directory);

    /**
     * @brief Disable metadata caching
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> disable_metadata_cache();

    /**
     * @brief Clear metadata cache
     * @return Number of cleared entries
     */
    int clear_metadata_cache();

    /**
     * @brief Get cached metadata
     * @param file_path Plugin file path
     * @return Cached metadata or error
     */
    qtplugin::expected<QJsonObject, PluginError> get_cached_metadata(
        const std::filesystem::path& file_path) const;

    /**
     * @brief Cache plugin metadata
     * @param file_path Plugin file path
     * @param metadata Metadata to cache
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> cache_metadata(
        const std::filesystem::path& file_path, const QJsonObject& metadata);

    // === Platform-Specific Discovery ===

    /**
     * @brief Discover plugins using platform-specific methods
     * @param search_paths Search paths
     * @param recursive Whether to search recursively
     * @return Vector of discovered plugin paths
     */
    std::vector<std::filesystem::path> discover_plugins_platform_optimized(
        const std::vector<std::filesystem::path>& search_paths,
        bool recursive = true);

    /**
     * @brief Get platform-specific plugin information
     * @param file_path Plugin file path
     * @return Platform plugin information or error
     */
    qtplugin::expected<PlatformPluginInfo, PluginError>
    get_platform_plugin_info(const std::filesystem::path& file_path) const;

    /**
     * @brief Validate plugin file integrity
     * @param file_path Plugin file path
     * @return Validation result or error
     */
    qtplugin::expected<bool, PluginError> validate_plugin_integrity(
        const std::filesystem::path& file_path) const;

    // === Performance Monitoring ===

    /**
     * @brief Get loading statistics
     * @return Platform loading statistics
     */
    PlatformLoadingStatistics get_loading_statistics() const;

    /**
     * @brief Reset loading statistics
     */
    void reset_statistics();

    /**
     * @brief Get plugin loading performance metrics
     * @param plugin_id Plugin identifier
     * @return Performance metrics or error
     */
    qtplugin::expected<QJsonObject, PluginError> get_plugin_performance_metrics(
        std::string_view plugin_id) const;

    /**
     * @brief Enable performance monitoring
     * @param enabled Whether to enable monitoring
     */
    void set_performance_monitoring_enabled(bool enabled);

    /**
     * @brief Check if performance monitoring is enabled
     * @return true if monitoring is enabled
     */
    bool is_performance_monitoring_enabled() const;

    // === Platform-Specific Methods ===

#ifdef Q_OS_WIN
    /**
     * @brief Windows-specific plugin loading
     * @param file_path Plugin file path
     * @return Loaded plugin or error
     */
    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError>
    load_windows_specific(const std::filesystem::path& file_path);

    /**
     * @brief Get Windows module information
     * @param file_path Plugin file path
     * @return Module information
     */
    QJsonObject get_windows_module_info(
        const std::filesystem::path& file_path) const;
#endif

#ifdef Q_OS_UNIX
    /**
     * @brief Unix-specific plugin loading
     * @param file_path Plugin file path
     * @return Loaded plugin or error
     */
    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError>
    load_unix_specific(const std::filesystem::path& file_path);

    /**
     * @brief Get Unix shared library information
     * @param file_path Plugin file path
     * @return Library information
     */
    QJsonObject get_unix_library_info(
        const std::filesystem::path& file_path) const;
#endif

#ifdef Q_OS_MAC
    /**
     * @brief macOS-specific plugin loading
     * @param file_path Plugin file path
     * @return Loaded plugin or error
     */
    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError>
    load_macos_specific(const std::filesystem::path& file_path);

    /**
     * @brief Get macOS bundle information
     * @param file_path Plugin file path
     * @return Bundle information
     */
    QJsonObject get_macos_bundle_info(
        const std::filesystem::path& file_path) const;
#endif

signals:
    /**
     * @brief Emitted when plugin loading starts
     * @param file_path Plugin file path
     */
    void loading_started(const QString& file_path);

    /**
     * @brief Emitted when plugin loading completes
     * @param file_path Plugin file path
     * @param success Whether loading was successful
     * @param load_time Loading time in milliseconds
     */
    void loading_completed(const QString& file_path, bool success,
                           qint64 load_time);

    /**
     * @brief Emitted when metadata is cached
     * @param file_path Plugin file path
     * @param cache_hit Whether it was a cache hit
     */
    void metadata_cached(const QString& file_path, bool cache_hit);

private:
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::PlatformLoadingStrategy)
Q_DECLARE_METATYPE(qtplugin::PlatformOptimization)
Q_DECLARE_METATYPE(qtplugin::PlatformPluginInfo)
Q_DECLARE_METATYPE(qtplugin::PlatformLoadingConfig)
Q_DECLARE_METATYPE(qtplugin::PlatformLoadingStatistics)
