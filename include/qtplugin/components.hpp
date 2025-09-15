/**
 * @file components.hpp
 * @brief Component headers for advanced QtPlugin usage
 * @version 3.0.0
 *
 * This header provides access to all internal components for advanced users
 * who want to use individual components directly or create custom
 * implementations.
 *
 * @example
 * ```cpp
 * #include <qtplugin/components.hpp>
 *
 * // Use components directly
 * auto registry = std::make_unique<qtplugin::PluginRegistry>();
 * auto validator = std::make_unique<qtplugin::SecurityValidator>();
 * ```
 */

#pragma once

// Core components
#include "core/plugin_dependency_resolver.hpp"
#include "core/plugin_registry.hpp"

// Monitoring components
#include "monitoring/plugin_hot_reload_manager.hpp"
#include "monitoring/plugin_metrics_collector.hpp"

// Security components removed

// Configuration components
#include "managers/components/configuration_merger.hpp"
#include "managers/components/configuration_storage.hpp"
#include "managers/components/configuration_validator.hpp"
#include "managers/components/configuration_watcher.hpp"

// Resource components
#include "managers/components/resource_allocator.hpp"
#include "managers/components/resource_monitor.hpp"
#include "managers/components/resource_pool.hpp"

/**
 * @namespace qtplugin
 * @brief Main namespace for the QtPlugin library
 */
namespace qtplugin {

/**
 * @namespace qtplugin::components
 * @brief Namespace for component utilities and helpers
 */
namespace components {

/**
 * @brief Component version information
 */
struct ComponentInfo {
    const char* name;
    const char* version;
    const char* description;
};

/**
 * @brief Get information about available components
 * @return Vector of component information
 */
inline std::vector<ComponentInfo> get_available_components() {
    return {
        {"PluginRegistry", "3.0.0", "Plugin storage and lookup management"},
        {"PluginDependencyResolver", "3.0.0",
         "Plugin dependency resolution and ordering"},
        {"PluginHotReloadManager", "3.0.0",
         "Hot reload functionality for plugins"},
        {"PluginMetricsCollector", "3.0.0",
         "Plugin metrics collection and monitoring"},
        {"SecurityValidator", "3.0.0",
         "Core security validation and file integrity"},
        {"SignatureVerifier", "3.0.0", "Digital signature verification"},
        {"PermissionManager", "3.0.0", "Plugin permission management"},
        {"SecurityPolicyEngine", "3.0.0",
         "Security policy evaluation and enforcement"},
        {"ConfigurationStorage", "3.0.0",
         "Configuration file I/O and persistence"},
        {"ConfigurationValidator", "3.0.0", "Configuration schema validation"},
        {"ConfigurationMerger", "3.0.0",
         "Configuration merging and inheritance"},
        {"ConfigurationWatcher", "3.0.0", "Configuration file monitoring"},
        {"ResourcePool", "3.0.0", "Resource pooling and lifecycle management"},
        {"ResourceAllocator", "3.0.0",
         "Resource allocation strategies and policies"},
        {"ResourceMonitor", "3.0.0", "Resource usage monitoring and alerting"}};
}

/**
 * @brief Check if a component is available
 * @param component_name Name of the component to check
 * @return true if component is available
 */
inline bool is_component_available(const std::string& component_name) {
    auto components = get_available_components();
    return std::find_if(components.begin(), components.end(),
                        [&component_name](const ComponentInfo& info) {
                            return info.name == component_name;
                        }) != components.end();
}

}  // namespace components
}  // namespace qtplugin
