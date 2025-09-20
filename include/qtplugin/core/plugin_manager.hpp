/**
 * @file plugin_manager.hpp
 * @brief Enhanced plugin manager using modern C++ features
 * @version 3.0.0
 */

#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QPluginLoader>
#include <QString>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../managers/plugin_version_manager.hpp"

#include "../utils/concepts.hpp"
#include "../utils/error_handling.hpp"
#include "plugin_dependency_resolver.hpp"
#include "../interfaces/core/plugin_interface.hpp"
#include "plugin_loader.hpp"
// Forward-declare dependent components to reduce header coupling

namespace qtplugin {

// Forward declarations
class IPluginLoader;
class IPluginRegistry;
class IPluginDependencyResolver;
class IPluginHotReloadManager;
class IPluginMetricsCollector;
class IMessageBus;

class IConfigurationManager;
class ILoggingManager;
class IResourceManager;
class IResourceLifecycleManager;
class IResourceMonitor;
class IPluginVersionManager;
class IResourceLifecycleManager;
class IResourceMonitor;
class IPluginVersionManager;

namespace detail {
struct IPluginRegistryDeleter {
    void operator()(IPluginRegistry*) const;
};
struct IPluginHotReloadManagerDeleter {
    void operator()(IPluginHotReloadManager*) const;
};
struct IPluginMetricsCollectorDeleter {
    void operator()(IPluginMetricsCollector*) const;
};
struct IResourceMonitorDeleter {
    void operator()(IResourceMonitor*) const;
};
}  // namespace detail

/**
 * @brief Plugin loading options
 */
struct PluginLoadOptions {
    bool validate_sha256 = false;        ///< Validate plugin SHA256 checksum
    std::string expected_sha256;          ///< Expected SHA256 hash (if validation enabled)
    bool check_dependencies = true;      ///< Check plugin dependencies
    bool initialize_immediately = true;  ///< Initialize plugin after loading
    bool enable_hot_reload = false;      ///< Enable hot reloading for this plugin
    std::chrono::milliseconds timeout =
        std::chrono::seconds{30};         ///< Loading timeout
    QJsonObject configuration;           ///< Initial plugin configuration
};

/**
 * @brief Plugin information structure
 */
struct PluginInfo {
    std::string id;
    std::filesystem::path file_path;
    PluginMetadata metadata;
    PluginState state = PluginState::Unloaded;
    std::chrono::system_clock::time_point load_time;
    std::chrono::system_clock::time_point last_activity;
    std::shared_ptr<IPlugin> instance;
    std::unique_ptr<QPluginLoader> loader;
    QJsonObject configuration;
    std::vector<std::string> error_log;
    QJsonObject metrics;
    bool hot_reload_enabled = false;

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
};

/**
 * @brief Enhanced plugin manager
 *
 * This class provides comprehensive plugin management functionality including
 * loading, unloading, dependency resolution, hot reloading, and monitoring.
 */
class PluginManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor with dependency injection
     * @param loader Custom plugin loader (optional)
     * @param message_bus Custom message bus (optional)

     * @param configuration_manager Custom configuration manager (optional)
     * @param logging_manager Custom logging manager (optional)
     * @param resource_manager Custom resource manager (optional)
     * @param resource_lifecycle_manager Custom resource lifecycle manager
     * (optional)
     * @param resource_monitor Custom resource monitor (optional)
     * @param parent Parent QObject (optional)
     */
    explicit PluginManager(
        std::unique_ptr<IPluginLoader> loader = nullptr,
        std::unique_ptr<IMessageBus> message_bus = nullptr,

        std::unique_ptr<IConfigurationManager> configuration_manager = nullptr,
        std::unique_ptr<ILoggingManager> logging_manager = nullptr,
        std::unique_ptr<IResourceManager> resource_manager = nullptr,
        std::unique_ptr<IResourceLifecycleManager> resource_lifecycle_manager =
            nullptr,
        std::unique_ptr<IResourceMonitor, detail::IResourceMonitorDeleter>
            resource_monitor = nullptr,
        std::unique_ptr<IPluginRegistry, detail::IPluginRegistryDeleter>
            plugin_registry = nullptr,
        std::unique_ptr<IPluginDependencyResolver> dependency_resolver =
            nullptr,
        std::unique_ptr<IPluginHotReloadManager,
                        detail::IPluginHotReloadManagerDeleter>
            hot_reload_manager = nullptr,
        std::unique_ptr<IPluginMetricsCollector,
                        detail::IPluginMetricsCollectorDeleter>
            metrics_collector = nullptr,
        std::unique_ptr<IPluginVersionManager> version_manager = nullptr,
        QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~PluginManager();  // Removed override since not inheriting from QObject

    // === Plugin Loading ===

    /**
     * @brief Load a plugin from file
     * @param file_path Path to the plugin file
     * @param options Loading options
     * @return Plugin ID or error information
     */
    qtplugin::expected<std::string, PluginError> load_plugin(
        const std::filesystem::path& file_path,
        const PluginLoadOptions& options = {});

    /**
     * @brief Load plugin asynchronously
     * @param file_path Path to the plugin file
     * @param options Loading options
     * @return Future with plugin ID or error information
     */
    std::future<qtplugin::expected<std::string, PluginError>> load_plugin_async(
        const std::filesystem::path& file_path,
        const PluginLoadOptions& options = {});

    /**
     * @brief Unload a plugin
     * @param plugin_id Plugin identifier
     * @param force Force unload even if other plugins depend on it
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> unload_plugin(
        std::string_view plugin_id, bool force = false);

    /**
     * @brief Reload a plugin
     * @param plugin_id Plugin identifier
     * @param preserve_state Whether to preserve plugin state
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> reload_plugin(
        std::string_view plugin_id, bool preserve_state = true);

    // === Plugin Discovery ===

    /**
     * @brief Discover plugins in a directory
     * @param directory Directory to search
     * @param recursive Whether to search recursively
     * @return Vector of discovered plugin file paths
     */
    std::vector<std::filesystem::path> discover_plugins(
        const std::filesystem::path& directory, bool recursive = false) const;

    /**
     * @brief Add plugin search path
     * @param path Path to add to search paths
     */
    void add_search_path(const std::filesystem::path& path);

    /**
     * @brief Remove plugin search path
     * @param path Path to remove from search paths
     */
    void remove_search_path(const std::filesystem::path& path);

    /**
     * @brief Get all plugin search paths
     * @return Vector of search paths
     */
    std::vector<std::filesystem::path> search_paths() const;

    /**
     * @brief Load all plugins from search paths
     * @param options Loading options to apply to all plugins
     * @return Number of successfully loaded plugins
     */
    int load_all_plugins(const PluginLoadOptions& options = {});

    // === Enhanced Features (v3.2.0) ===

    /**
     * @brief Transaction for atomic plugin operations
     */
    class PluginTransaction {
    public:
        using Operation = std::function<qtplugin::expected<void, PluginError>()>;
        using Rollback = std::function<void()>;

        void add_load(const std::filesystem::path& path,
                     const PluginLoadOptions& options = {});
        void add_unload(std::string_view plugin_id, bool force = false);
        void add_reload(std::string_view plugin_id, bool preserve_state = true);
        void add_operation(Operation op, Rollback rollback);

        qtplugin::expected<void, PluginError> commit();
        void rollback();

        bool is_committed() const { return m_committed; }
        bool is_rolled_back() const { return m_rolled_back; }
        std::vector<std::string> loaded_plugins() const { return m_loaded_plugins; }

    private:
        friend class PluginManager;
        PluginTransaction(PluginManager* manager);

        PluginManager* m_manager;
        std::vector<std::pair<Operation, Rollback>> m_operations;
        std::vector<std::string> m_loaded_plugins;
        size_t m_completed = 0;
        bool m_committed = false;
        bool m_rolled_back = false;
    };

    /**
     * @brief Begin a new transaction
     * @return Unique pointer to transaction
     */
    std::unique_ptr<PluginTransaction> begin_transaction();

    /**
     * @brief Batch load multiple plugins
     * @param paths Plugin file paths
     * @param options Loading options for all plugins
     * @return Map of paths to results (plugin ID or error)
     */
    std::unordered_map<std::filesystem::path,
                       qtplugin::expected<std::string, PluginError>>
    batch_load(const std::vector<std::filesystem::path>& paths,
               const PluginLoadOptions& options = {});

    /**
     * @brief Batch unload multiple plugins
     * @param plugin_ids Plugin identifiers
     * @param force Force unload even with dependencies
     * @return Map of plugin IDs to results
     */
    std::unordered_map<std::string, qtplugin::expected<void, PluginError>>
    batch_unload(const std::vector<std::string>& plugin_ids, bool force = false);

    // === Plugin Lifecycle Hooks ===

    using PluginHook = std::function<qtplugin::expected<void, PluginError>(
        const std::string& plugin_id, std::shared_ptr<IPlugin>)>;

    /**
     * @brief Register pre-load hook
     * @param hook Function called before plugin load
     * @return Hook ID for unregistration
     */
    std::string register_pre_load_hook(PluginHook hook);

    /**
     * @brief Register post-load hook
     * @param hook Function called after successful plugin load
     * @return Hook ID for unregistration
     */
    std::string register_post_load_hook(PluginHook hook);

    /**
     * @brief Register pre-unload hook
     * @param hook Function called before plugin unload
     * @return Hook ID for unregistration
     */
    std::string register_pre_unload_hook(PluginHook hook);

    /**
     * @brief Unregister hook
     * @param hook_id Hook identifier
     */
    void unregister_hook(const std::string& hook_id);

    // === Health Monitoring ===

    /**
     * @brief Plugin health status
     */
    struct HealthStatus {
        bool is_healthy = true;
        std::string status_message;
        std::chrono::system_clock::time_point last_check;
        int consecutive_failures = 0;
        QJsonObject diagnostics;
    };

    /**
     * @brief Perform health check on plugin
     * @param plugin_id Plugin identifier
     * @return Health status
     */
    HealthStatus check_plugin_health(std::string_view plugin_id);

    /**
     * @brief Perform health check on all plugins
     * @return Map of plugin IDs to health status
     */
    std::unordered_map<std::string, HealthStatus> check_all_plugin_health();

    /**
     * @brief Enable automatic health monitoring
     * @param interval Check interval
     * @param auto_restart Whether to auto-restart unhealthy plugins
     */
    void enable_health_monitoring(std::chrono::milliseconds interval,
                                 bool auto_restart = false);

    /**
     * @brief Disable automatic health monitoring
     */
    void disable_health_monitoring();

    // === Configuration Hot Reload ===

    /**
     * @brief Update plugin configuration without restart
     * @param plugin_id Plugin identifier
     * @param config New configuration
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> update_plugin_config(
        std::string_view plugin_id,
        const QJsonObject& config);

    /**
     * @brief Batch update plugin configurations
     * @param configs Map of plugin IDs to new configurations
     * @return Map of plugin IDs to results
     */
    std::unordered_map<std::string, qtplugin::expected<void, PluginError>>
    batch_update_configs(
        const std::unordered_map<std::string, QJsonObject>& configs);

    // === Plugin Access ===

    /**
     * @brief Get plugin by ID
     * @param plugin_id Plugin identifier
     * @return Shared pointer to plugin, or nullptr if not found
     */
    std::shared_ptr<IPlugin> get_plugin(std::string_view plugin_id) const;

    /**
     * @brief Get plugin with specific interface type
     * @tparam PluginType Plugin interface type
     * @param plugin_id Plugin identifier
     * @return Shared pointer to plugin with specified type, or nullptr if not
     * found or wrong type
     */
    template <concepts::Plugin PluginType>
    std::shared_ptr<PluginType> get_plugin(std::string_view plugin_id) const {
        auto plugin = get_plugin(plugin_id);
        return std::dynamic_pointer_cast<PluginType>(plugin);
    }

    /**
     * @brief Get all loaded plugins
     * @return Vector of plugin IDs
     */
    std::vector<std::string> loaded_plugins() const;

    /**
     * @brief Get plugins by capability
     * @param capability Capability to filter by
     * @return Vector of plugin IDs that have the specified capability
     */
    std::vector<std::string> plugins_with_capability(
        PluginCapability capability) const;

    /**
     * @brief Get plugins by category
     * @param category Category to filter by
     * @return Vector of plugin IDs in the specified category
     */
    std::vector<std::string> plugins_in_category(
        std::string_view category) const;

    /**
     * @brief Get plugin information
     * @param plugin_id Plugin identifier
     * @return Plugin information, or nullopt if not found
     */
    std::optional<PluginInfo> get_plugin_info(std::string_view plugin_id) const;

    /**
     * @brief Get all plugin information
     * @return Vector of plugin information for all loaded plugins
     */
    std::vector<PluginInfo> all_plugin_info() const;

    // === Plugin State Management ===

    /**
     * @brief Initialize all loaded plugins
     * @return Number of successfully initialized plugins
     */
    int initialize_all_plugins();

    /**
     * @brief Shutdown all plugins
     */
    void shutdown_all_plugins();

    /**
     * @brief Start all service plugins
     * @return Number of successfully started services
     */
    int start_all_services();

    /**
     * @brief Stop all service plugins
     * @return Number of successfully stopped services
     */
    int stop_all_services();

    // === Dependency Management ===

    /**
     * @brief Resolve plugin dependencies
     * @return Success or error information with details about unresolved
     * dependencies
     */
    qtplugin::expected<void, PluginError> resolve_dependencies();

    /**
     * @brief Get dependency graph
     * @return Map of plugin IDs to their dependency information
     */
    std::unordered_map<std::string, DependencyNode> dependency_graph() const;

    /**
     * @brief Get load order for plugins based on dependencies
     * @return Vector of plugin IDs in load order
     */
    std::vector<std::string> get_load_order() const;

    /**
     * @brief Check if plugin can be safely unloaded
     * @param plugin_id Plugin identifier
     * @return true if plugin can be unloaded without breaking dependencies
     */
    bool can_unload_safely(std::string_view plugin_id) const;

    // === Hot Reloading ===

    /**
     * @brief Enable hot reloading for a plugin
     * @param plugin_id Plugin identifier
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> enable_hot_reload(
        std::string_view plugin_id);

    /**
     * @brief Disable hot reloading for a plugin
     * @param plugin_id Plugin identifier
     */
    void disable_hot_reload(std::string_view plugin_id);

    /**
     * @brief Check if hot reloading is enabled for a plugin
     * @param plugin_id Plugin identifier
     * @return true if hot reloading is enabled
     */
    bool is_hot_reload_enabled(std::string_view plugin_id) const;

    /**
     * @brief Enable global hot reloading
     * @param watch_directories Directories to watch for changes
     */
    void enable_global_hot_reload(
        const std::vector<std::filesystem::path>& watch_directories = {});

    /**
     * @brief Disable global hot reloading
     */
    void disable_global_hot_reload();

    // === Configuration Management ===

    /**
     * @brief Configure a plugin
     * @param plugin_id Plugin identifier
     * @param configuration Configuration data
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> configure_plugin(
        std::string_view plugin_id, const QJsonObject& configuration);

    /**
     * @brief Get plugin configuration
     * @param plugin_id Plugin identifier
     * @return Plugin configuration, or empty object if not found
     */
    QJsonObject get_plugin_configuration(std::string_view plugin_id) const;

    /**
     * @brief Save all plugin configurations
     * @param file_path File to save configurations to
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> save_configurations(
        const std::filesystem::path& file_path) const;

    /**
     * @brief Load plugin configurations
     * @param file_path File to load configurations from
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> load_configurations(
        const std::filesystem::path& file_path);

    /**
     * @brief Get configuration manager
     * @return Reference to configuration manager
     */
    IConfigurationManager& configuration_manager() const;

    /**
     * @brief Get logging manager
     * @return Reference to logging manager
     */
    ILoggingManager& logging_manager() const;

    /**
     * @brief Get resource manager
     * @return Reference to resource manager
     */
    IResourceManager& resource_manager() const;

    /**
     * @brief Get resource lifecycle manager
     * @return Reference to resource lifecycle manager
     */
    IResourceLifecycleManager& resource_lifecycle_manager() const;

    /**
     * @brief Get resource monitor
     * @return Reference to resource monitor
     */
    IResourceMonitor& resource_monitor() const;

    // === Communication ===

    /**
     * @brief Send command to a plugin
     * @param plugin_id Plugin identifier
     * @param command Command name
     * @param parameters Command parameters
     * @return Command result or error information
     */
    qtplugin::expected<QJsonObject, PluginError> send_command(
        std::string_view plugin_id, std::string_view command,
        const QJsonObject& parameters = {});

    /**
     * @brief Broadcast message to all plugins
     * @tparam MessageType Type of message to broadcast
     * @param message Message to broadcast
     */
    template <typename MessageType>
    void broadcast_message(const MessageType& message);

    /**
     * @brief Get message bus
     * @return Reference to the message bus
     */
    IMessageBus& message_bus() const { return *m_message_bus; }

    // === Monitoring and Metrics ===

    /**
     * @brief Get system metrics
     * @return System-wide plugin metrics
     */
    QJsonObject system_metrics() const;

    /**
     * @brief Get plugin metrics
     * @param plugin_id Plugin identifier
     * @return Plugin-specific metrics
     */
    QJsonObject plugin_metrics(std::string_view plugin_id) const;

    /**
     * @brief Start performance monitoring
     * @param interval Monitoring interval
     */
    void start_monitoring(
        std::chrono::milliseconds interval = std::chrono::seconds{60});

    /**
     * @brief Stop performance monitoring
     */
    void stop_monitoring();

    /**
     * @brief Check if monitoring is active
     * @return true if monitoring is running
     */
    bool is_monitoring_active() const noexcept { return m_monitoring_active; }

    // === Version Management ===

    /**
     * @brief Get version manager
     * @return Reference to version manager
     */
    IPluginVersionManager& version_manager() const;

    /**
     * @brief Install a specific version of a plugin
     * @param plugin_id Plugin identifier
     * @param version Version to install
     * @param file_path Path to plugin file
     * @param replace_existing Whether to replace existing version
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> install_plugin_version(
        std::string_view plugin_id, const Version& version,
        const std::filesystem::path& file_path, bool replace_existing = false);

    /**
     * @brief Uninstall a specific version of a plugin
     * @param plugin_id Plugin identifier
     * @param version Version to uninstall
     * @param force Force uninstall even if active
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> uninstall_plugin_version(
        std::string_view plugin_id, const Version& version, bool force = false);

    /**
     * @brief Get all installed versions of a plugin
     * @param plugin_id Plugin identifier
     * @return Vector of installed versions
     */
    std::vector<PluginVersionInfo> get_plugin_versions(
        std::string_view plugin_id) const;

    /**
     * @brief Set active version for a plugin
     * @param plugin_id Plugin identifier
     * @param version Version to activate
     * @param migrate_data Whether to migrate data from previous version
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> set_plugin_active_version(
        std::string_view plugin_id, const Version& version,
        bool migrate_data = true);

    /**
     * @brief Get currently active version of a plugin
     * @param plugin_id Plugin identifier
     * @return Active version info, or nullopt if not active
     */
    std::optional<PluginVersionInfo> get_plugin_active_version(
        std::string_view plugin_id) const;

    // === SHA256 Validation ===

    /**
     * @brief Calculate SHA256 hash of a file
     * @param file_path Path to the file
     * @return SHA256 hash as hex string, empty if error
     */
    std::string calculate_file_sha256(const std::filesystem::path& file_path) const;

    /**
     * @brief Verify file SHA256 hash
     * @param file_path Path to the file
     * @param expected_hash Expected SHA256 hash
     * @return true if hash matches, false otherwise
     */
    bool verify_file_sha256(const std::filesystem::path& file_path,
                            const std::string& expected_hash) const;

signals:
    void plugin_loaded(const QString& plugin_id);
    void plugin_unloaded(const QString& plugin_id);
    void plugin_state_changed(const QString& plugin_id, PluginState old_state,
                              PluginState new_state);
    void plugin_error(const QString& plugin_id, const QString& error);
    void plugin_metrics_updated(const QString& plugin_id,
                                const QJsonObject& metrics);

private slots:
    void on_file_changed(const QString& path);
    void on_monitoring_timer();

private:
    // Components
    std::unique_ptr<IPluginLoader> m_loader;
    std::unique_ptr<IMessageBus> m_message_bus;

    std::unique_ptr<IConfigurationManager> m_configuration_manager;
    std::unique_ptr<ILoggingManager> m_logging_manager;
    std::unique_ptr<IResourceManager> m_resource_manager;
    std::unique_ptr<IResourceLifecycleManager> m_resource_lifecycle_manager;
    std::unique_ptr<IResourceMonitor, detail::IResourceMonitorDeleter>
        m_resource_monitor;
    std::unique_ptr<IPluginRegistry, detail::IPluginRegistryDeleter>
        m_plugin_registry;
    std::unique_ptr<IPluginDependencyResolver> m_dependency_resolver;
    std::unique_ptr<IPluginHotReloadManager,
                    detail::IPluginHotReloadManagerDeleter>
        m_hot_reload_manager;
    std::unique_ptr<IPluginMetricsCollector,
                    detail::IPluginMetricsCollectorDeleter>
        m_metrics_collector;
    std::unique_ptr<IPluginVersionManager> m_version_manager;

    // === Enhanced Features (v3.2.0) ===

    // Lifecycle hooks
    struct HookEntry {
        std::string id;
        PluginHook hook;
    };
    std::vector<HookEntry> m_pre_load_hooks;
    std::vector<HookEntry> m_post_load_hooks;
    std::vector<HookEntry> m_pre_unload_hooks;
    mutable std::shared_mutex m_hooks_mutex;

    // Health monitoring
    std::unordered_map<std::string, HealthStatus> m_health_status;
    std::unique_ptr<QTimer> m_health_timer;
    bool m_auto_restart_unhealthy = false;
    mutable std::shared_mutex m_health_mutex;

    // Transaction support
    std::vector<std::unique_ptr<PluginTransaction>> m_active_transactions;
    mutable std::mutex m_transaction_mutex;

    // Plugin storage (now handled by PluginRegistry and
    // PluginDependencyResolver)
    // TODO: Remove after refactoring is complete
    mutable std::shared_mutex m_plugins_mutex;
    std::unordered_map<std::string, std::unique_ptr<PluginInfo>> m_plugins;
    std::unordered_map<std::string, DependencyNode> m_dependency_graph;

    // Search paths
    mutable std::shared_mutex m_search_paths_mutex;
    std::unordered_set<std::filesystem::path> m_search_paths;

    // Hot reloading (now handled by PluginHotReloadManager)
    // TODO: Remove after refactoring is complete
    // std::unique_ptr<QFileSystemWatcher> m_file_watcher;
    // std::unordered_map<std::string, std::filesystem::path> m_watched_files;

    // Monitoring (now handled by PluginMetricsCollector)
    // TODO: Remove after refactoring is complete
    std::atomic<bool> m_monitoring_active{false};
    std::unique_ptr<QTimer> m_monitoring_timer;



    // Helper methods
    qtplugin::expected<void, PluginError> validate_plugin_file(
        const std::filesystem::path& file_path) const;
    qtplugin::expected<void, PluginError> check_plugin_dependencies(
        const PluginInfo& info) const;
    void update_dependency_graph();
    std::vector<std::string> topological_sort() const;
    void cleanup_plugin(const std::string& plugin_id);
    void update_plugin_metrics(const std::string& plugin_id);

    // Dependency graph helpers
    int calculate_dependency_level(
        const std::string& plugin_id,
        const std::vector<std::string>& dependencies) const;
    void detect_circular_dependencies() const;
    bool has_circular_dependency(
        const std::string& plugin_id, std::unordered_set<std::string>& visited,
        std::unordered_set<std::string>& recursion_stack) const;

    // Utility helpers
    std::string plugin_state_to_string(PluginState state) const;
};

}  // namespace qtplugin
