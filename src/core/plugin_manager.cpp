/**
 * @file plugin_manager.cpp
 * @brief Implementation of enhanced plugin manager
 * @version 3.0.0
 */

#include "../../include/qtplugin/core/plugin_manager.hpp"
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QString>
#include <QTimer>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include "../../include/qtplugin/communication/message_bus.hpp"
#include "../../include/qtplugin/core/plugin_dependency_resolver.hpp"
// #include "../../include/qtplugin/core/plugin_loader.hpp"  // Temporarily disabled
#include "../../include/qtplugin/core/plugin_registry.hpp"
#include "../../include/qtplugin/interfaces/core/service_plugin_interface.hpp"
#include "../../include/qtplugin/managers/configuration_manager_impl.hpp"
#include "../../include/qtplugin/managers/logging_manager_impl.hpp"
#include "../../include/qtplugin/managers/plugin_version_manager.hpp"
#include "../../include/qtplugin/managers/resource_lifecycle_impl.hpp"
#include "../../include/qtplugin/managers/resource_manager_impl.hpp"
#include "../../include/qtplugin/managers/resource_monitor_impl.hpp"
#include "../../include/qtplugin/monitoring/plugin_hot_reload_manager.hpp"
#include "../../include/qtplugin/monitoring/plugin_metrics_collector.hpp"


Q_LOGGING_CATEGORY(pluginManagerLog, "qtplugin.manager")

namespace qtplugin {

// PluginManager::Impl class definition
class PluginManager::Impl {
public:
    // Components
    std::unique_ptr<IPluginLoader> loader;
    std::unique_ptr<IMessageBus> message_bus;
    std::unique_ptr<IConfigurationManager> configuration_manager;
    std::unique_ptr<ILoggingManager> logging_manager;
    std::unique_ptr<IResourceManager> resource_manager;
    std::unique_ptr<IResourceLifecycleManager> resource_lifecycle_manager;
    std::unique_ptr<IResourceMonitor, detail::IResourceMonitorDeleter> resource_monitor;
    std::unique_ptr<IPluginRegistry, detail::IPluginRegistryDeleter> plugin_registry;
    std::unique_ptr<IPluginDependencyResolver> dependency_resolver;
    std::unique_ptr<IPluginHotReloadManager, detail::IPluginHotReloadManagerDeleter> hot_reload_manager;
    std::unique_ptr<IPluginMetricsCollector, detail::IPluginMetricsCollectorDeleter> metrics_collector;
    std::unique_ptr<IPluginVersionManager> version_manager;

    // Enhanced Features (v3.2.0)
    struct HookEntry {
        std::string id;
        PluginHook hook;
    };
    std::vector<HookEntry> pre_load_hooks;
    std::vector<HookEntry> post_load_hooks;
    std::vector<HookEntry> pre_unload_hooks;
    mutable std::shared_mutex hooks_mutex;

    // Health monitoring
    std::unordered_map<std::string, HealthStatus> health_status;
    std::unique_ptr<QTimer> health_timer;
    bool auto_restart_unhealthy = false;
    mutable std::shared_mutex health_mutex;

    // Transaction support
    std::vector<std::unique_ptr<PluginTransaction>> active_transactions;
    mutable std::mutex transaction_mutex;

    // Plugin storage (now handled by PluginRegistry and PluginDependencyResolver)
    mutable std::shared_mutex plugins_mutex;
    std::unordered_map<std::string, std::unique_ptr<PluginInfo>> plugins;
    std::unordered_map<std::string, DependencyNode> dependency_graph;

    // Search paths
    mutable std::shared_mutex search_paths_mutex;
    std::unordered_set<std::filesystem::path> search_paths;

    // Monitoring
    std::atomic<bool> monitoring_active{false};
    std::unique_ptr<QTimer> monitoring_timer;

    // Helper methods
    qtplugin::expected<void, PluginError> validate_plugin_file(
        const std::filesystem::path& file_path, const PluginManager* manager) const;
    qtplugin::expected<void, PluginError> check_plugin_dependencies(
        const PluginInfo& info, const PluginManager* manager) const;
    void update_dependency_graph(PluginManager* manager);
    std::vector<std::string> topological_sort() const;
    void cleanup_plugin(const std::string& plugin_id, PluginManager* manager);
    void update_plugin_metrics(const std::string& plugin_id, PluginManager* manager);

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

QJsonObject PluginInfo::to_json() const {
    QJsonObject json;
    json["id"] = QString::fromStdString(id);
    json["file_path"] = QString::fromStdString(file_path.string());
    json["state"] = static_cast<int>(state);
    json["load_time"] =
        QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                            load_time.time_since_epoch())
                            .count());
    json["last_activity"] =
        QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                            last_activity.time_since_epoch())
                            .count());
    json["hot_reload_enabled"] = hot_reload_enabled;
    json["configuration"] = configuration;
    json["metrics"] = metrics;

    // Add metadata
    if (instance) {
        auto plugin_metadata = instance->metadata();
        json["metadata"] = plugin_metadata.to_json();
    }

    // Add error log
    QJsonArray errors_array;
    for (const auto& error : error_log) {
        errors_array.append(QString::fromStdString(error));
    }
    json["error_log"] = errors_array;

    return json;
}

PluginManager::PluginManager(
    std::unique_ptr<IPluginLoader> loader,
    std::unique_ptr<IMessageBus> message_bus,
    std::unique_ptr<IConfigurationManager> configuration_manager,
    std::unique_ptr<ILoggingManager> logging_manager,
    std::unique_ptr<IResourceManager> resource_manager,
    std::unique_ptr<IResourceLifecycleManager> resource_lifecycle_manager,
    std::unique_ptr<IResourceMonitor, detail::IResourceMonitorDeleter>
        resource_monitor,
    std::unique_ptr<IPluginRegistry, detail::IPluginRegistryDeleter>
        plugin_registry,
    std::unique_ptr<IPluginDependencyResolver> dependency_resolver,
    std::unique_ptr<IPluginHotReloadManager,
                    detail::IPluginHotReloadManagerDeleter>
        hot_reload_manager,
    std::unique_ptr<IPluginMetricsCollector,
                    detail::IPluginMetricsCollectorDeleter>
        metrics_collector,
    std::unique_ptr<IPluginVersionManager> version_manager, QObject* parent)
    : QObject(parent), d(std::make_unique<Impl>()) {

    // Initialize components in d-pointer
    d->loader = std::move(loader);  // Note: loader may be nullptr if QtPluginLoader is disabled
    d->message_bus = message_bus ? std::move(message_bus) : std::make_unique<MessageBus>();
    d->configuration_manager = configuration_manager ? std::move(configuration_manager) : create_configuration_manager(this);
    d->logging_manager = logging_manager ? std::move(logging_manager) : create_logging_manager(this);
    d->resource_manager = resource_manager ? std::move(resource_manager) : create_resource_manager(this);
    d->resource_lifecycle_manager = resource_lifecycle_manager ? std::move(resource_lifecycle_manager) : create_resource_lifecycle_manager(this);
    d->resource_monitor = resource_monitor ? std::move(resource_monitor) : std::unique_ptr<IResourceMonitor, detail::IResourceMonitorDeleter>(create_resource_monitor(this).release());
    d->plugin_registry = plugin_registry ? std::move(plugin_registry) : std::unique_ptr<IPluginRegistry, detail::IPluginRegistryDeleter>(new PluginRegistry(this));
    d->dependency_resolver = dependency_resolver ? std::move(dependency_resolver) : std::make_unique<PluginDependencyResolver>(this);
    d->hot_reload_manager = hot_reload_manager ? std::move(hot_reload_manager) : std::unique_ptr<IPluginHotReloadManager, detail::IPluginHotReloadManagerDeleter>(new PluginHotReloadManager(this));
    d->metrics_collector = metrics_collector ? std::move(metrics_collector) : std::unique_ptr<IPluginMetricsCollector, detail::IPluginMetricsCollectorDeleter>(new PluginMetricsCollector(this));
    d->version_manager = version_manager ? std::move(version_manager) : create_plugin_version_manager(
        std::shared_ptr<IPluginRegistry>(d->plugin_registry.get(), [](IPluginRegistry*) {}),
        std::shared_ptr<IConfigurationManager>(d->configuration_manager.get(), [](IConfigurationManager*) {}),
        std::shared_ptr<ILoggingManager>(d->logging_manager.get(), [](ILoggingManager*) {}),
        this);

    // Initialize timers
    d->monitoring_timer = std::make_unique<QTimer>(this);

    // Set up hot reload callback
    d->hot_reload_manager->set_reload_callback([this](const std::string& plugin_id) {
        reload_plugin(plugin_id, true);
    });

    // Connect monitoring timer (legacy - will be removed)
    connect(d->monitoring_timer.get(), &QTimer::timeout, this, &PluginManager::on_monitoring_timer);
}

PluginManager::~PluginManager() {
    shutdown_all_plugins();
}

// Copy constructor
PluginManager::PluginManager(const PluginManager& other) : QObject(other.parent()), d(std::make_unique<Impl>()) {
    // Deep copy implementation - note: this is complex due to Qt objects and unique_ptrs
    // For now, we'll create new instances rather than copying
    d->loader = nullptr;  // Note: loader disabled during Pimpl refactoring
    d->message_bus = std::make_unique<MessageBus>();
    d->configuration_manager = create_configuration_manager(this);
    d->logging_manager = create_logging_manager(this);
    d->resource_manager = create_resource_manager(this);
    d->resource_lifecycle_manager = create_resource_lifecycle_manager(this);
    d->resource_monitor = std::unique_ptr<IResourceMonitor, detail::IResourceMonitorDeleter>(create_resource_monitor(this).release());
    d->plugin_registry = std::unique_ptr<IPluginRegistry, detail::IPluginRegistryDeleter>(new PluginRegistry(this));
    d->dependency_resolver = std::make_unique<PluginDependencyResolver>(this);
    d->hot_reload_manager = std::unique_ptr<IPluginHotReloadManager, detail::IPluginHotReloadManagerDeleter>(new PluginHotReloadManager(this));
    d->metrics_collector = std::unique_ptr<IPluginMetricsCollector, detail::IPluginMetricsCollectorDeleter>(new PluginMetricsCollector(this));
    d->version_manager = create_plugin_version_manager(
        std::shared_ptr<IPluginRegistry>(d->plugin_registry.get(), [](IPluginRegistry*) {}),
        std::shared_ptr<IConfigurationManager>(d->configuration_manager.get(), [](IConfigurationManager*) {}),
        std::shared_ptr<ILoggingManager>(d->logging_manager.get(), [](ILoggingManager*) {}),
        this);
    d->monitoring_timer = std::make_unique<QTimer>(this);

    // Copy simple data
    d->pre_load_hooks = other.d->pre_load_hooks;
    d->post_load_hooks = other.d->post_load_hooks;
    d->pre_unload_hooks = other.d->pre_unload_hooks;
    d->health_status = other.d->health_status;
    d->auto_restart_unhealthy = other.d->auto_restart_unhealthy;
    d->search_paths = other.d->search_paths;
    d->monitoring_active = other.d->monitoring_active.load();
}

// Copy assignment operator
PluginManager& PluginManager::operator=(const PluginManager& other) {
    if (this != &other) {
        // Shutdown current state
        shutdown_all_plugins();

        // Recreate components
        d->loader = nullptr;  // Note: loader disabled during Pimpl refactoring
        d->message_bus = std::make_unique<MessageBus>();
        d->configuration_manager = create_configuration_manager(this);
        d->logging_manager = create_logging_manager(this);
        d->resource_manager = create_resource_manager(this);
        d->resource_lifecycle_manager = create_resource_lifecycle_manager(this);
        d->resource_monitor = std::unique_ptr<IResourceMonitor, detail::IResourceMonitorDeleter>(create_resource_monitor(this).release());
        d->plugin_registry = std::unique_ptr<IPluginRegistry, detail::IPluginRegistryDeleter>(new PluginRegistry(this));
        d->dependency_resolver = std::make_unique<PluginDependencyResolver>(this);
        d->hot_reload_manager = std::unique_ptr<IPluginHotReloadManager, detail::IPluginHotReloadManagerDeleter>(new PluginHotReloadManager(this));
        d->metrics_collector = std::unique_ptr<IPluginMetricsCollector, detail::IPluginMetricsCollectorDeleter>(new PluginMetricsCollector(this));
        d->version_manager = create_plugin_version_manager(
            std::shared_ptr<IPluginRegistry>(d->plugin_registry.get(), [](IPluginRegistry*) {}),
            std::shared_ptr<IConfigurationManager>(d->configuration_manager.get(), [](IConfigurationManager*) {}),
            std::shared_ptr<ILoggingManager>(d->logging_manager.get(), [](ILoggingManager*) {}),
            this);
        d->monitoring_timer = std::make_unique<QTimer>(this);

        // Copy simple data
        d->pre_load_hooks = other.d->pre_load_hooks;
        d->post_load_hooks = other.d->post_load_hooks;
        d->pre_unload_hooks = other.d->pre_unload_hooks;
        d->health_status = other.d->health_status;
        d->auto_restart_unhealthy = other.d->auto_restart_unhealthy;
        d->search_paths = other.d->search_paths;
        d->monitoring_active = other.d->monitoring_active.load();
    }
    return *this;
}

// Move constructor
PluginManager::PluginManager(PluginManager&& other) noexcept : QObject(other.parent()), d(std::move(other.d)) {
    other.d = std::make_unique<Impl>();
}

// Move assignment operator
PluginManager& PluginManager::operator=(PluginManager&& other) noexcept {
    if (this != &other) {
        shutdown_all_plugins();
        d = std::move(other.d);
        other.d = std::make_unique<Impl>();
    }
    return *this;
}

// === Impl Helper Method Implementations ===

qtplugin::expected<void, PluginError> PluginManager::Impl::validate_plugin_file(
    const std::filesystem::path& file_path, const PluginManager* manager) const {
    Q_UNUSED(manager);

    if (!std::filesystem::exists(file_path)) {
        return make_error<void>(PluginErrorCode::FileNotFound,
                                "Plugin file not found: " + file_path.string());
    }

    // Skip loader validation if loader is disabled during Pimpl refactoring
    if (loader && !loader->can_load(file_path)) {
        return make_error<void>(PluginErrorCode::InvalidFormat,
                                "Invalid plugin file format: " + file_path.string());
    }

    return make_success();
}

void PluginManager::Impl::update_plugin_metrics(const std::string& plugin_id, PluginManager* manager) {
    Q_UNUSED(manager);

    if (metrics_collector && plugin_registry) {
        metrics_collector->update_plugin_metrics(plugin_id, plugin_registry.get());
    }
}

void PluginManager::on_file_changed(const QString& path) {
    Q_UNUSED(path);
    // Legacy stub: hot reload now handled by PluginHotReloadManager
}

void PluginManager::on_monitoring_timer() {
    // Legacy stub: metrics updated via PluginMetricsCollector
    if (d->plugin_registry) {
        auto ids = d->plugin_registry->get_all_plugin_ids();
        for (const auto& id : ids) {
            d->update_plugin_metrics(id, this);
        }
    }
}

qtplugin::expected<std::string, PluginError> PluginManager::load_plugin(
    const std::filesystem::path& file_path, const PluginLoadOptions& options) {
    // Validate plugin file
    auto validation_result = d->validate_plugin_file(file_path, this);
    if (!validation_result) {
        return qtplugin::unexpected<PluginError>{validation_result.error()};
    }

    // SHA256 validation
    if (options.validate_sha256 && !options.expected_sha256.empty()) {
        std::string calculated_hash = calculate_file_sha256(file_path);
        if (calculated_hash.empty()) {
            return make_error<std::string>(PluginErrorCode::SecurityViolation,
                                           "Failed to calculate SHA256 hash");
        }

        if (!verify_file_sha256(file_path, options.expected_sha256)) {
            return make_error<std::string>(PluginErrorCode::SecurityViolation,
                                           "SHA256 hash verification failed");
        }
    }

    // Load the plugin
    auto plugin_result = d->loader->load(file_path);
    if (!plugin_result) {
        return qtplugin::unexpected<PluginError>{plugin_result.error()};
    }

    auto plugin = plugin_result.value();
    std::string plugin_id = plugin->id();

    // Check if already loaded
    if (d->plugin_registry->is_plugin_registered(plugin_id)) {
        return make_error<std::string>(PluginErrorCode::LoadFailed,
                                       "Plugin already loaded: " + plugin_id);
    }

    // Create plugin info
    auto plugin_info = std::make_unique<PluginInfo>();
    plugin_info->id = plugin_id;
    plugin_info->file_path = file_path;
    plugin_info->metadata = plugin->metadata();
    plugin_info->state = PluginState::Loaded;
    plugin_info->load_time = std::chrono::system_clock::now();
    plugin_info->last_activity = plugin_info->load_time;
    plugin_info->instance = plugin;
    plugin_info->configuration = options.configuration;
    plugin_info->hot_reload_enabled = options.enable_hot_reload;

    // Check dependencies if requested
    if (options.check_dependencies) {
        auto dep_result =
            d->dependency_resolver->check_plugin_dependencies(*plugin_info);
        if (!dep_result) {
            return qtplugin::unexpected<PluginError>{dep_result.error()};
        }
    }

    // Configure plugin if configuration provided
    if (!options.configuration.isEmpty()) {
        auto config_result = plugin->configure(options.configuration);
        if (!config_result) {
            return qtplugin::unexpected<PluginError>{config_result.error()};
        }
    }

    // Initialize plugin if requested
    if (options.initialize_immediately) {
        plugin_info->state = PluginState::Initializing;
        auto init_result = plugin->initialize();
        if (!init_result) {
            plugin_info->state = PluginState::Error;
            plugin_info->error_log.push_back(init_result.error().message);
            return qtplugin::unexpected<PluginError>{init_result.error()};
        }
        plugin_info->state = PluginState::Running;
    }

    // Enable hot reload if requested
    if (options.enable_hot_reload) {
        d->hot_reload_manager->enable_hot_reload(plugin_id, file_path);
    }

    // Store plugin info in registry
    auto register_result =
        d->plugin_registry->register_plugin(plugin_id, std::move(plugin_info));
    if (!register_result) {
        return qtplugin::unexpected<PluginError>{register_result.error()};
    }

    // Update dependency graph
    d->dependency_resolver->update_dependency_graph(d->plugin_registry.get());

    emit plugin_loaded(QString::fromStdString(plugin_id));

    return plugin_id;
}

std::future<qtplugin::expected<std::string, PluginError>>
PluginManager::load_plugin_async(const std::filesystem::path& file_path,
                                 const PluginLoadOptions& options) {
    return std::async(std::launch::async, [this, file_path, options]() {
        return load_plugin(file_path, options);
    });
}

qtplugin::expected<void, PluginError> PluginManager::unload_plugin(
    std::string_view plugin_id, bool force) {
    // Get plugin info from registry
    auto plugin_info_opt =
        d->plugin_registry->get_plugin_info(std::string(plugin_id));
    if (!plugin_info_opt) {
        return make_error<void>(PluginErrorCode::LoadFailed,
                                "Plugin not found: " + std::string(plugin_id));
    }

    const auto& plugin_info = plugin_info_opt.value();

    // Check if plugin can be safely unloaded
    if (!force &&
        !d->dependency_resolver->can_unload_safely(std::string(plugin_id))) {
        return make_error<void>(
            PluginErrorCode::DependencyMissing,
            "Plugin has dependents and cannot be safely unloaded");
    }

    // Shutdown plugin if running
    if (plugin_info.instance && plugin_info.state == PluginState::Running) {
        plugin_info.instance->shutdown();
    }

    // Disable hot reload
    d->hot_reload_manager->disable_hot_reload(std::string(plugin_id));

    // Unload from loader
    auto unload_result = d->loader->unload(plugin_id);
    if (!unload_result) {
        return unload_result;
    }

    // Remove from registry
    auto unregister_result =
        d->plugin_registry->unregister_plugin(std::string(plugin_id));
    if (!unregister_result) {
        return unregister_result;
    }

    // Update dependency graph
    d->dependency_resolver->update_dependency_graph(d->plugin_registry.get());

    emit plugin_unloaded(QString::fromStdString(std::string(plugin_id)));

    return make_success();
}

std::shared_ptr<IPlugin> PluginManager::get_plugin(
    std::string_view plugin_id) const {
    return d->plugin_registry->get_plugin(std::string(plugin_id));
}

std::vector<std::string> PluginManager::loaded_plugins() const {
    return d->plugin_registry->get_all_plugin_ids();
}

std::vector<PluginInfo> PluginManager::all_plugin_info() const {
    return d->plugin_registry->get_all_plugin_info();
}

std::vector<std::filesystem::path> PluginManager::discover_plugins(
    const std::filesystem::path& directory, bool recursive) const {
    std::vector<std::filesystem::path> discovered_plugins;

    if (!std::filesystem::exists(directory) ||
        !std::filesystem::is_directory(directory)) {
        return discovered_plugins;
    }

    auto extensions = d->loader->supported_extensions();

    try {
        if (recursive) {
            for (const auto& entry :
                 std::filesystem::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file() &&
                    d->loader->can_load(entry.path())) {
                    discovered_plugins.push_back(entry.path());
                }
            }
        } else {
            for (const auto& entry :
                 std::filesystem::directory_iterator(directory)) {
                if (entry.is_regular_file() &&
                    d->loader->can_load(entry.path())) {
                    discovered_plugins.push_back(entry.path());
                }
            }
        }

    } catch (const std::filesystem::filesystem_error&) {
        // Ignore filesystem errors during discovery
    }

    return discovered_plugins;
}

void PluginManager::add_search_path(const std::filesystem::path& path) {
    std::unique_lock lock(d->search_paths_mutex);
    d->search_paths.insert(path);
}

void PluginManager::remove_search_path(const std::filesystem::path& path) {
    std::unique_lock lock(d->search_paths_mutex);
    d->search_paths.erase(path);
}

std::vector<std::filesystem::path> PluginManager::search_paths() const {
    std::shared_lock lock(d->search_paths_mutex);
    return std::vector<std::filesystem::path>(d->search_paths.begin(),
                                              d->search_paths.end());
}

int PluginManager::load_all_plugins(const PluginLoadOptions& options) {
    int loaded_count = 0;

    auto paths = search_paths();
    for (const auto& search_path : paths) {
        auto discovered = discover_plugins(search_path, true);
        for (const auto& plugin_path : discovered) {
            auto result = load_plugin(plugin_path, options);
            if (result) {
                ++loaded_count;
            }
        }
    }

    return loaded_count;
}

// Hot reload functionality moved to PluginHotReloadManager
// This method is now redundant and can be removed

// Helper methods removed - these were not declared in the header

// === Missing Method Implementations ===

// System metrics collection moved to PluginMetricsCollector
QJsonObject PluginManager::system_metrics() const {
    return d->metrics_collector->get_system_metrics(d->plugin_registry.get());
}

void PluginManager::shutdown_all_plugins() {
    // Get all plugin IDs from registry
    auto plugin_ids = d->plugin_registry->get_all_plugin_ids();

    // Shutdown all plugins (order doesn't matter for shutdown)
    for (const auto& plugin_id : plugin_ids) {
        auto plugin = d->plugin_registry->get_plugin(plugin_id);
        if (plugin) {
            try {
                plugin->shutdown();
            } catch (...) {
                // Log error but continue shutdown
            }
        }
    }

    // Clear registry
    d->plugin_registry->clear();
}

int PluginManager::start_all_services() {
    std::shared_lock lock(d->plugins_mutex);
    int started_count = 0;

    for (auto& [id, info] : d->plugins) {
        if (info && info->instance) {
            // Check if plugin has Service capability
            auto capabilities = info->metadata.capabilities;
            if (capabilities &
                static_cast<uint32_t>(PluginCapability::Service)) {
                // Try to cast to service plugin and start it
                auto service_plugin =
                    std::dynamic_pointer_cast<IServicePlugin>(info->instance);
                if (service_plugin) {
                    try {
                        auto result = service_plugin->start_service();
                        if (result) {
                            started_count++;
                        }
                    } catch (...) {
                        // Log error but continue with other services
                    }
                }
            }
        }
    }

    return started_count;
}

int PluginManager::stop_all_services() {
    std::shared_lock lock(d->plugins_mutex);
    int stopped_count = 0;

    for (auto& [id, info] : d->plugins) {
        if (info && info->instance) {
            // Check if plugin has Service capability
            auto capabilities = info->metadata.capabilities;
            if (capabilities &
                static_cast<uint32_t>(PluginCapability::Service)) {
                // Try to cast to service plugin and stop it
                auto service_plugin =
                    std::dynamic_pointer_cast<IServicePlugin>(info->instance);
                if (service_plugin) {
                    try {
                        auto result = service_plugin->stop_service();
                        if (result) {
                            stopped_count++;
                        }
                    } catch (...) {
                        // Log error but continue with other services
                    }
                }
            }
        }
    }

    return stopped_count;
}

// Hot reload functionality moved to PluginHotReloadManager
qtplugin::expected<void, PluginError> PluginManager::enable_hot_reload(
    std::string_view plugin_id) {
    return d->hot_reload_manager->enable_hot_reload(std::string(plugin_id),
                                                   std::filesystem::path{});
}

// Dependency checking moved to PluginDependencyResolver
bool PluginManager::can_unload_safely(std::string_view plugin_id) const {
    return d->dependency_resolver->can_unload_safely(std::string(plugin_id));
}

// Hot reload functionality moved to PluginHotReloadManager
void PluginManager::disable_hot_reload(std::string_view plugin_id) {
    d->hot_reload_manager->disable_hot_reload(std::string(plugin_id));
}

qtplugin::expected<void, PluginError> PluginManager::reload_plugin(
    std::string_view plugin_id, bool preserve_state) {
    std::unique_lock lock(d->plugins_mutex);

    auto it = d->plugins.find(std::string(plugin_id));
    if (it == d->plugins.end()) {
        return make_error<void>(PluginErrorCode::LoadFailed,
                                "Plugin not found");
    }

    if (!it->second) {
        return make_error<void>(PluginErrorCode::LoadFailed,
                                "Plugin info is null");
    }

    // Save state if requested
    QJsonObject saved_state;
    if (preserve_state && it->second->instance) {
        try {
            // Try to get state from plugin using standard command
            auto state_result =
                it->second->instance->execute_command("save_state");
            if (state_result) {
                saved_state = state_result.value();
            } else {
                // Fallback: save current configuration as state
                saved_state = it->second->configuration;
                saved_state["_fallback_state"] = true;
            }

            // Also save plugin metrics and runtime information
            saved_state["_runtime_info"] = QJsonObject{
                {"load_time",
                 QString::number(
                     std::chrono::duration_cast<std::chrono::milliseconds>(
                         it->second->load_time.time_since_epoch())
                         .count())},
                {"last_activity",
                 QString::number(
                     std::chrono::duration_cast<std::chrono::milliseconds>(
                         it->second->last_activity.time_since_epoch())
                         .count())},
                {"error_count",
                 static_cast<int>(it->second->error_log.size())}};
        } catch (...) {
            qCWarning(pluginManagerLog)
                << "Failed to save state for plugin:"
                << QString::fromStdString(std::string(plugin_id));
        }
    }

    // Unload current plugin
    if (it->second->instance) {
        it->second->instance->shutdown();
    }

    // Reload plugin
    auto plugin_result = d->loader->load(it->second->file_path);
    if (!plugin_result) {
        return make_error<void>(plugin_result.error().code,
                                "Failed to reload plugin");
    }

    it->second->instance = plugin_result.value();

    // Initialize plugin
    auto init_result = it->second->instance->initialize();
    if (!init_result) {
        return make_error<void>(init_result.error().code,
                                "Failed to initialize reloaded plugin");
    }

    // Restore state if requested
    if (preserve_state && !saved_state.isEmpty()) {
        try {
            // Check if this was a fallback state save
            bool is_fallback = saved_state.contains("_fallback_state") &&
                               saved_state["_fallback_state"].toBool();

            if (is_fallback) {
                // Restore configuration
                QJsonObject config = saved_state;
                config.remove("_fallback_state");
                config.remove("_runtime_info");

                auto config_result = it->second->instance->configure(config);
                if (!config_result) {
                    qCWarning(pluginManagerLog)
                        << "Failed to restore configuration for plugin:"
                        << QString::fromStdString(std::string(plugin_id));
                }
            } else {
                // Try to restore state using standard command
                auto restore_result = it->second->instance->execute_command(
                    "restore_state", saved_state);
                if (!restore_result) {
                    qCWarning(pluginManagerLog)
                        << "Failed to restore state for plugin:"
                        << QString::fromStdString(std::string(plugin_id));

                    // Fallback: try to restore as configuration
                    auto config_result =
                        it->second->instance->configure(saved_state);
                    if (!config_result) {
                        qCWarning(pluginManagerLog)
                            << "Failed to restore state as configuration for "
                               "plugin:"
                            << QString::fromStdString(std::string(plugin_id));
                    }
                }
            }

            // Update plugin info with restored state
            it->second->configuration = saved_state;

        } catch (...) {
            qCWarning(pluginManagerLog)
                << "Exception during state restoration for plugin:"
                << QString::fromStdString(std::string(plugin_id));
        }
    }

    return make_success();
}

qtplugin::expected<void, PluginError> PluginManager::configure_plugin(
    std::string_view plugin_id, const QJsonObject& configuration) {
    std::unique_lock lock(d->plugins_mutex);
    auto it = d->plugins.find(std::string(plugin_id));
    if (it == d->plugins.end()) {
        return make_error<void>(PluginErrorCode::StateError,
                                "Plugin not found");
    }

    // Store configuration
    it->second->configuration = configuration;

    // Apply configuration to plugin if it's loaded
    if (it->second->instance) {
        auto result = it->second->instance->configure(configuration);
        if (!result) {
            return make_error<void>(result.error().code,
                                    "Failed to configure plugin");
        }
    }

    return make_success();
}

// Plugin metrics collection moved to PluginMetricsCollector
QJsonObject PluginManager::plugin_metrics(std::string_view plugin_id) const {
    return d->metrics_collector->get_plugin_metrics(std::string(plugin_id),
                                                   d->plugin_registry.get());
}

// Monitoring functionality moved to PluginMetricsCollector
void PluginManager::start_monitoring(std::chrono::milliseconds interval) {
    d->metrics_collector->start_monitoring(interval);
}

// Plugin info retrieval moved to PluginRegistry
std::optional<PluginInfo> PluginManager::get_plugin_info(
    std::string_view plugin_id) const {
    return d->plugin_registry->get_plugin_info(std::string(plugin_id));
}

QJsonObject PluginManager::get_plugin_configuration(
    std::string_view plugin_id) const {
    std::shared_lock lock(d->plugins_mutex);
    auto it = d->plugins.find(std::string(plugin_id));
    if (it == d->plugins.end()) {
        return QJsonObject();
    }

    return it->second->configuration;
}

IConfigurationManager& PluginManager::configuration_manager() const {
    return *d->configuration_manager;
}

ILoggingManager& PluginManager::logging_manager() const {
    return *d->logging_manager;
}

IResourceManager& PluginManager::resource_manager() const {
    return *d->resource_manager;
}

IResourceLifecycleManager& PluginManager::resource_lifecycle_manager() const {
    return *d->resource_lifecycle_manager;
}

IResourceMonitor& PluginManager::resource_monitor() const {
    return *d->resource_monitor;
}

// === Helper Methods ===

// Dependency level calculation moved to PluginDependencyResolver
// This method is now redundant and can be removed

// Helper methods removed - these were not declared in the header

// === Version Management Implementation ===

IPluginVersionManager& PluginManager::version_manager() const {
    return *d->version_manager;
}

qtplugin::expected<void, PluginError> PluginManager::install_plugin_version(
    std::string_view plugin_id, const Version& version,
    const std::filesystem::path& file_path, bool replace_existing) {
    auto result = d->version_manager->install_version(
        plugin_id, version, file_path, replace_existing);
    if (!result) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::LoadFailed,
            "Failed to install plugin version: " + result.error().message,
            std::string(plugin_id)});
    }
    return {};
}

qtplugin::expected<void, PluginError> PluginManager::uninstall_plugin_version(
    std::string_view plugin_id, const Version& version, bool force) {
    auto result =
        d->version_manager->uninstall_version(plugin_id, version, force);
    if (!result) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::UnloadFailed,
            "Failed to uninstall plugin version: " + result.error().message,
            std::string(plugin_id)});
    }
    return {};
}

std::vector<PluginVersionInfo> PluginManager::get_plugin_versions(
    std::string_view plugin_id) const {
    return d->version_manager->get_installed_versions(plugin_id);
}

qtplugin::expected<void, PluginError> PluginManager::set_plugin_active_version(
    std::string_view plugin_id, const Version& version, bool migrate_data) {
    auto result =
        d->version_manager->set_active_version(plugin_id, version, migrate_data);
    if (!result) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::StateError,
            "Failed to set active plugin version: " + result.error().message,
            std::string(plugin_id)});
    }
    return {};
}

std::optional<PluginVersionInfo> PluginManager::get_plugin_active_version(
    std::string_view plugin_id) const {
    return d->version_manager->get_active_version(plugin_id);
}

// === Missing Methods Implementation ===

std::vector<std::string> PluginManager::plugins_with_capability(
    PluginCapability capability) const {
    std::vector<std::string> result;
    std::shared_lock lock(d->plugins_mutex);

    for (const auto& [id, info] : d->plugins) {
        if (info->instance) {
            auto metadata = info->instance->metadata();
            if (metadata.capabilities & static_cast<PluginCapabilities>(capability)) {
                result.push_back(id);
            }
        }
    }

    return result;
}

std::vector<std::string> PluginManager::plugins_in_category(
    std::string_view category) const {
    std::vector<std::string> result;
    std::shared_lock lock(d->plugins_mutex);

    for (const auto& [id, info] : d->plugins) {
        if (info->instance) {
            auto metadata = info->instance->metadata();
            if (metadata.category == category) {
                result.push_back(id);
            }
        }
    }

    return result;
}

qtplugin::expected<void, PluginError> PluginManager::resolve_dependencies() {
    if (!d->dependency_resolver) {
        return make_error<void>(PluginErrorCode::StateError,
                               "Dependency resolver not available");
    }

    // Update dependency graph from plugin registry
    auto result = d->dependency_resolver->update_dependency_graph(d->plugin_registry.get());
    if (!result) {
        return qtplugin::unexpected(result.error());
    }

    // Check for circular dependencies
    if (d->dependency_resolver->has_circular_dependencies()) {
        return make_error<void>(PluginErrorCode::CircularDependency,
                               "Circular dependencies detected");
    }

    return make_success();
}

std::unordered_map<std::string, DependencyNode> PluginManager::dependency_graph() const {
    if (!d->dependency_resolver) {
        return {};
    }

    // Delegate to the dependency resolver
    return d->dependency_resolver->get_dependency_graph();
}

std::vector<std::string> PluginManager::get_load_order() const {
    if (!d->dependency_resolver) {
        // Fallback: return plugins in registration order
        std::vector<std::string> load_order;
        std::shared_lock lock(d->plugins_mutex);
        for (const auto& [id, info] : d->plugins) {
            load_order.push_back(id);
        }
        return load_order;
    }

    // Delegate to the dependency resolver
    return d->dependency_resolver->get_load_order();
}

namespace detail {
void IPluginRegistryDeleter::operator()(IPluginRegistry* p) const { delete p; }
void IPluginHotReloadManagerDeleter::operator()(
    IPluginHotReloadManager* p) const {
    delete p;
}
void IPluginMetricsCollectorDeleter::operator()(
    IPluginMetricsCollector* p) const {
    delete p;
}
void IResourceMonitorDeleter::operator()(IResourceMonitor* p) const {
    delete p;
}
}  // namespace detail

// === Enhanced Features Implementation (v3.2.0) ===

// PluginTransaction implementation
PluginManager::PluginTransaction::PluginTransaction(PluginManager* manager)
    : m_manager(manager) {
}

void PluginManager::PluginTransaction::add_load(
    const std::filesystem::path& path,
    const PluginLoadOptions& options) {

    if (m_committed || m_rolled_back) {
        throw std::runtime_error("Cannot modify completed transaction");
    }

    add_operation(
        [this, path, options]() -> qtplugin::expected<void, PluginError> {
            auto result = m_manager->load_plugin(path, options);
            if (result) {
                m_loaded_plugins.push_back(result.value());
                return make_success();
            }
            return qtplugin::unexpected<PluginError>{result.error()};
        },
        [this]() {
            if (!m_loaded_plugins.empty()) {
                auto plugin_id = m_loaded_plugins.back();
                m_manager->unload_plugin(plugin_id, true);
                m_loaded_plugins.pop_back();
            }
        }
    );
}

void PluginManager::PluginTransaction::add_unload(
    std::string_view plugin_id, bool force) {

    if (m_committed || m_rolled_back) {
        throw std::runtime_error("Cannot modify completed transaction");
    }

    // Store plugin info for potential rollback
    auto plugin_info = m_manager->get_plugin_info(plugin_id);
    auto file_path = plugin_info ? plugin_info->file_path : std::filesystem::path{};

    add_operation(
        [this, plugin_id = std::string(plugin_id), force]() {
            return m_manager->unload_plugin(plugin_id, force);
        },
        [this, file_path]() {
            if (!file_path.empty()) {
                // Attempt to reload the plugin
                m_manager->load_plugin(file_path);
            }
        }
    );
}

void PluginManager::PluginTransaction::add_operation(
    Operation op, Rollback rollback) {

    if (m_committed || m_rolled_back) {
        throw std::runtime_error("Cannot modify completed transaction");
    }

    m_operations.emplace_back(std::move(op), std::move(rollback));
}

qtplugin::expected<void, PluginError> PluginManager::PluginTransaction::commit() {
    if (m_committed) {
        return make_success();
    }

    if (m_rolled_back) {
        return make_error<void>(PluginErrorCode::InvalidState,
                               "Transaction already rolled back");
    }

    for (size_t i = m_completed; i < m_operations.size(); ++i) {
        auto result = m_operations[i].first();
        if (!result) {
            // Rollback on failure
            rollback();
            return result;
        }
        ++m_completed;
    }

    m_committed = true;
    return make_success();
}

void PluginManager::PluginTransaction::rollback() {
    if (m_committed || m_rolled_back) {
        return;
    }

    // Rollback in reverse order
    for (size_t i = m_completed; i > 0; --i) {
        m_operations[i - 1].second();
    }

    m_completed = 0;
    m_rolled_back = true;
    m_loaded_plugins.clear();
}

// Transaction support
std::unique_ptr<PluginManager::PluginTransaction>
PluginManager::begin_transaction() {
    std::lock_guard<std::mutex> lock(d->transaction_mutex);

    auto transaction = std::unique_ptr<PluginTransaction>(
        new PluginTransaction(this));

    d->active_transactions.push_back(
        std::unique_ptr<PluginTransaction>(new PluginTransaction(this)));

    return transaction;
}

// Batch operations
std::unordered_map<std::filesystem::path,
                   qtplugin::expected<std::string, PluginError>>
PluginManager::batch_load(const std::vector<std::filesystem::path>& paths,
                          const PluginLoadOptions& options) {

    std::unordered_map<std::filesystem::path,
                       qtplugin::expected<std::string, PluginError>> results;

    auto transaction = begin_transaction();

    for (const auto& path : paths) {
        transaction->add_load(path, options);
    }

    auto commit_result = transaction->commit();

    if (commit_result) {
        // All succeeded, populate results
        const auto& loaded = transaction->loaded_plugins();
        for (size_t i = 0; i < paths.size() && i < loaded.size(); ++i) {
            results[paths[i]] = loaded[i];
        }
    } else {
        // Some failed, get individual results
        for (const auto& path : paths) {
            results[path] = load_plugin(path, options);
        }
    }

    return results;
}

std::unordered_map<std::string, qtplugin::expected<void, PluginError>>
PluginManager::batch_unload(const std::vector<std::string>& plugin_ids,
                            bool force) {

    std::unordered_map<std::string, qtplugin::expected<void, PluginError>> results;

    auto transaction = begin_transaction();

    for (const auto& id : plugin_ids) {
        transaction->add_unload(id, force);
    }

    auto commit_result = transaction->commit();

    if (commit_result) {
        // All succeeded
        for (const auto& id : plugin_ids) {
            results[id] = make_success();
        }
    } else {
        // Some failed, get individual results
        for (const auto& id : plugin_ids) {
            results[id] = unload_plugin(id, force);
        }
    }

    return results;
}

// Lifecycle hooks
std::string PluginManager::register_pre_load_hook(PluginHook hook) {
    std::unique_lock lock(d->hooks_mutex);

    std::string hook_id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    d->pre_load_hooks.push_back({hook_id, std::move(hook)});

    return hook_id;
}

std::string PluginManager::register_post_load_hook(PluginHook hook) {
    std::unique_lock lock(d->hooks_mutex);

    std::string hook_id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    d->post_load_hooks.push_back({hook_id, std::move(hook)});

    return hook_id;
}

std::string PluginManager::register_pre_unload_hook(PluginHook hook) {
    std::unique_lock lock(d->hooks_mutex);

    std::string hook_id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    d->pre_unload_hooks.push_back({hook_id, std::move(hook)});

    return hook_id;
}

void PluginManager::unregister_hook(const std::string& hook_id) {
    std::unique_lock lock(d->hooks_mutex);

    auto remove_hook = [&hook_id](std::vector<Impl::HookEntry>& hooks) {
        hooks.erase(
            std::remove_if(hooks.begin(), hooks.end(),
                [&hook_id](const Impl::HookEntry& entry) {
                    return entry.id == hook_id;
                }),
            hooks.end()
        );
    };

    remove_hook(d->pre_load_hooks);
    remove_hook(d->post_load_hooks);
    remove_hook(d->pre_unload_hooks);
}

// Health monitoring
PluginManager::HealthStatus
PluginManager::check_plugin_health(std::string_view plugin_id) {

    auto plugin = get_plugin(plugin_id);
    if (!plugin) {
        return HealthStatus{
            false,
            "Plugin not found",
            std::chrono::system_clock::now(),
            0,
            QJsonObject()
        };
    }

    HealthStatus status;
    status.last_check = std::chrono::system_clock::now();

    // Basic health check - plugin is loaded and responding
    try {
        // Check if plugin is initialized
        auto state = plugin->state();
        if (state != PluginState::Running && state != PluginState::Loaded) {
            status.is_healthy = false;
            status.status_message = "Plugin not in running state";
        } else {
            status.is_healthy = true;
            status.status_message = "Plugin is healthy";
        }

        // Get plugin metrics if available
        if (d->metrics_collector) {
            status.diagnostics = d->metrics_collector->get_plugin_metrics(
                std::string(plugin_id), d->plugin_registry.get());
        }

    } catch (const std::exception& e) {
        status.is_healthy = false;
        status.status_message = std::string("Health check failed: ") + e.what();
    }

    // Update health status
    {
        std::unique_lock lock(d->health_mutex);
        auto& stored_status = d->health_status[std::string(plugin_id)];

        if (!status.is_healthy) {
            stored_status.consecutive_failures++;
        } else {
            stored_status.consecutive_failures = 0;
        }

        stored_status = status;
        stored_status.consecutive_failures =
            status.is_healthy ? 0 : stored_status.consecutive_failures;
    }

    return status;
}

std::unordered_map<std::string, PluginManager::HealthStatus>
PluginManager::check_all_plugin_health() {

    std::unordered_map<std::string, HealthStatus> results;

    auto plugins = loaded_plugins();
    for (const auto& plugin_id : plugins) {
        results[plugin_id] = check_plugin_health(plugin_id);
    }

    return results;
}

void PluginManager::enable_health_monitoring(
    std::chrono::milliseconds interval,
    bool auto_restart) {

    if (!d->health_timer) {
        d->health_timer = std::make_unique<QTimer>(this);
    }

    d->auto_restart_unhealthy = auto_restart;

    connect(d->health_timer.get(), &QTimer::timeout, this, [this]() {
        auto health_results = check_all_plugin_health();

        if (d->auto_restart_unhealthy) {
            for (const auto& [plugin_id, status] : health_results) {
                if (!status.is_healthy && status.consecutive_failures >= 3) {
                    qWarning() << "Auto-restarting unhealthy plugin:"
                              << QString::fromStdString(plugin_id);

                    // Try to reload the plugin
                    reload_plugin(plugin_id, true);
                }
            }
        }
    });

    d->health_timer->start(interval.count());

    qCDebug(pluginManagerLog) << "Health monitoring enabled with interval:"
                       << interval.count() << "ms";
}

void PluginManager::disable_health_monitoring() {
    if (d->health_timer) {
        d->health_timer->stop();
        disconnect(d->health_timer.get(), nullptr, this, nullptr);
    }

    d->auto_restart_unhealthy = false;

    qCDebug(pluginManagerLog) << "Health monitoring disabled";
}

// Configuration hot reload
qtplugin::expected<void, PluginError>
PluginManager::update_plugin_config(
    std::string_view plugin_id,
    const QJsonObject& config) {

    auto plugin = get_plugin(plugin_id);
    if (!plugin) {
        return make_error<void>(PluginErrorCode::LoadFailed,
                               "Plugin not found: " + std::string(plugin_id));
    }

    // Call plugin's configure method
    auto result = plugin->configure(config);

    if (result) {
        // Update stored configuration
        auto plugin_info_opt = d->plugin_registry->get_plugin_info(std::string(plugin_id));
        if (plugin_info_opt) {
            auto& plugin_info = const_cast<PluginInfo&>(plugin_info_opt.value());
            plugin_info.configuration = config;
            plugin_info.last_activity = std::chrono::system_clock::now();
        }

        qCDebug(pluginManagerLog) << "Updated configuration for plugin:"
                          << QString::fromStdString(std::string(plugin_id));
    }

    return result;
}

std::unordered_map<std::string, qtplugin::expected<void, PluginError>>
PluginManager::batch_update_configs(
    const std::unordered_map<std::string, QJsonObject>& configs) {

    std::unordered_map<std::string, qtplugin::expected<void, PluginError>> results;

    for (const auto& [plugin_id, config] : configs) {
        results[plugin_id] = update_plugin_config(plugin_id, config);
    }

    return results;
}

// === SHA256 Validation Implementation ===

std::string PluginManager::calculate_file_sha256(const std::filesystem::path& file_path) const {
    try {
        QFile file(QString::fromStdString(file_path.string()));
        if (!file.open(QIODevice::ReadOnly)) {
            return {};
        }

        QCryptographicHash hash(QCryptographicHash::Sha256);
        if (hash.addData(&file)) {
            return hash.result().toHex().toStdString();
        }

    } catch (...) {
        // Return empty string on error
    }

    return {};
}

bool PluginManager::verify_file_sha256(const std::filesystem::path& file_path,
                                       const std::string& expected_hash) const {
    std::string calculated_hash = calculate_file_sha256(file_path);

    if (calculated_hash.empty()) {
        return false;
    }

    // Case-insensitive comparison
    std::string expected_lower = expected_hash;
    std::string calculated_lower = calculated_hash;

    std::transform(expected_lower.begin(), expected_lower.end(),
                   expected_lower.begin(), ::tolower);
    std::transform(calculated_lower.begin(), calculated_lower.end(),
                   calculated_lower.begin(), ::tolower);

    return expected_lower == calculated_lower;
}

// Implementation of methods that were moved from inline to out-of-line
IMessageBus& PluginManager::message_bus() const {
    return *d->message_bus;
}

bool PluginManager::is_monitoring_active() const noexcept {
    return d->monitoring_active;
}

}  // namespace qtplugin
