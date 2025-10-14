/**
 * @file components.hpp
 * @brief Comprehensive component headers for QtPlugin library
 * @version 3.2.0
 *
 * This header provides access to all internal components for advanced users
 * who want to use individual components directly or create custom
 * implementations. All components are available by default in v3.2.0.
 *
 * @example Basic Component Usage
 * ```cpp
 * #include <qtplugin/components.hpp>
 *
 * // Use components directly
 * auto registry = std::make_unique<qtplugin::PluginRegistry>();
 * auto validator = std::make_unique<qtplugin::SecurityValidator>();
 * auto manager = std::make_unique<qtplugin::PermissionManager>();
 * ```
 *
 * @example Component Factory Usage
 * ```cpp
 * #include <qtplugin/components.hpp>
 *
 * // Use component factory
 * auto registry = qtplugin::ComponentFactory::create_plugin_registry();
 * auto security = qtplugin::ComponentFactory::create_security_validator();
 * auto permissions = qtplugin::ComponentFactory::create_permission_manager();
 * ```
 */

#pragma once

// Core components
#include "core/plugin_capability_discovery.hpp"
#include "core/plugin_dependency_resolver.hpp"
#include "core/plugin_lifecycle_manager.hpp"
#include "core/plugin_loader.hpp"
#include "core/plugin_manager.hpp"
#include "core/plugin_registry.hpp"
#include "interfaces/core/plugin_interface.hpp"
#include "interfaces/core/service_plugin_interface.hpp"

// Monitoring components
#include "monitoring/plugin_hot_reload_manager.hpp"
#include "monitoring/plugin_metrics_collector.hpp"

// Security components - Removed
// SHA256 verification functionality is preserved in PluginManager

// Configuration components
#include "managers/components/configuration_merger.hpp"
#include "managers/components/configuration_storage.hpp"
#include "managers/components/configuration_validator.hpp"
#include "managers/components/configuration_watcher.hpp"

// Resource components
#include "managers/components/resource_allocator.hpp"
#include "managers/components/resource_monitor.hpp"
#include "managers/components/resource_pool.hpp"

// Communication components
#include "communication/message_bus.hpp"
#include "communication/message_types.hpp"
#include "communication/plugin_service_contracts.hpp"
#include "communication/plugin_service_discovery.hpp"
#include "communication/request_response_system.hpp"
#include "communication/typed_event_system.hpp"

// Platform components
#include "platform/platform_error_handler.hpp"
#include "platform/platform_performance_monitor.hpp"
#include "platform/platform_plugin_loader.hpp"

// Interface components
#include "interfaces/data/data_processor_plugin_interface.hpp"
#include "interfaces/network/network_plugin_interface.hpp"
#include "interfaces/ui/ui_plugin_interface.hpp"
// #include "interfaces/scripting/scripting_plugin_interface.hpp" // Temporarily
// disabled - requires Qt6Qml
#include "interfaces/interface_validator.hpp"

// Manager components
#include "managers/configuration_manager.hpp"
#include "managers/logging_manager.hpp"
#include "managers/plugin_version_manager.hpp"
#include "managers/resource_lifecycle.hpp"
#include "managers/resource_manager.hpp"
#include "managers/resource_monitor.hpp"

// Threading components
#include "threading/plugin_thread_pool.hpp"

// Workflow components
#include "workflow/composition.hpp"
#include "workflow/integration.hpp"
#include "workflow/orchestration.hpp"
#include "workflow/transactions.hpp"
#include "workflow/workflow.hpp"
#include "workflow/workflow_types.hpp"

// Bridge components
// Temporarily disabled due to interface compatibility issues
// #include "bridges/lua_plugin_bridge.hpp"
// #include "bridges/python_plugin_bridge.hpp"

// Remote plugin components
#include "remote/remote_plugin_configuration.hpp"
#include "remote/remote_plugin_discovery.hpp"
#include "remote/remote_plugin_loader.hpp"
#include "remote/remote_plugin_manager.hpp"
#include "remote/remote_plugin_validator.hpp"
#include "remote/remote_security_manager.hpp"
#include "remote/unified_plugin_manager.hpp"

// Utility components
#include "utils/concepts.hpp"
#include "utils/error_handling.hpp"
#include "utils/version.hpp"

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
        // Core components
        {"PluginRegistry", "3.2.0", "Plugin storage and lookup management"},
        {"PluginDependencyResolver", "3.2.0",
         "Plugin dependency resolution and ordering"},
        {"PluginManager", "3.2.0", "Central plugin management system"},
        {"PluginLoader", "3.2.0", "Plugin loading and unloading functionality"},
        {"PluginLifecycleManager", "3.2.0",
         "Plugin lifecycle state management"},
        {"ServicePluginInterface", "3.2.0",
         "Service-oriented plugin interface"},
        {"PluginCapabilityDiscovery", "3.2.0",
         "Plugin capability discovery system"},

        // Monitoring components
        {"PluginHotReloadManager", "3.2.0",
         "Hot reload functionality for plugins"},
        {"PluginMetricsCollector", "3.2.0",
         "Plugin metrics collection and monitoring"},

        // Security components
        {"SecurityManager", "3.2.0", "Central security management system"},
        {"SecurityValidator", "3.2.0",
         "Core security validation and file integrity"},
        {"SignatureVerifier", "3.2.0", "Digital signature verification"},
        {"PermissionManager", "3.2.0", "Plugin permission management"},
        {"SecurityPolicyEngine", "3.2.0",
         "Security policy evaluation and enforcement"},

        // Configuration components
        {"ConfigurationStorage", "3.2.0",
         "Configuration file I/O and persistence"},
        {"ConfigurationValidator", "3.2.0", "Configuration schema validation"},
        {"ConfigurationMerger", "3.2.0",
         "Configuration merging and inheritance"},
        {"ConfigurationWatcher", "3.2.0", "Configuration file monitoring"},
        {"ConfigurationManager", "3.2.0", "Central configuration management"},

        // Resource components
        {"ResourcePool", "3.2.0", "Resource pooling and lifecycle management"},
        {"ResourceAllocator", "3.2.0",
         "Resource allocation strategies and policies"},
        {"ResourceMonitor", "3.2.0", "Resource usage monitoring and alerting"},
        {"ResourceManager", "3.2.0", "Central resource management system"},
        {"ResourceLifecycle", "3.2.0", "Resource lifecycle management"},

        // Communication components
        {"MessageBus", "3.2.0", "Inter-plugin message bus system"},
        {"TypedEventSystem", "3.2.0", "Type-safe event system"},
        {"RequestResponseSystem", "3.2.0",
         "Request-response communication system"},
        {"PluginServiceDiscovery", "3.2.0",
         "Plugin service discovery mechanism"},

        // Platform components
        {"PlatformPluginLoader", "3.2.0", "Platform-specific plugin loading"},
        {"PlatformErrorHandler", "3.2.0", "Platform-specific error handling"},
        {"PlatformPerformanceMonitor", "3.2.0",
         "Platform performance monitoring"},

        // Interface components
        {"DataProcessorPluginInterface", "3.2.0",
         "Data processing plugin interface"},
        {"NetworkPluginInterface", "3.2.0", "Network plugin interface"},
        {"UIPluginInterface", "3.2.0", "User interface plugin interface"},
        {"ScriptingPluginInterface", "3.2.0", "Scripting plugin interface"},
        {"InterfaceValidator", "3.2.0", "Plugin interface validation"},

        // Threading components
        {"PluginThreadPool", "3.2.0", "Thread pool for plugin operations"},

        // Workflow components
        {"WorkflowEngine", "3.2.0", "Plugin workflow orchestration"},
        {"WorkflowComposition", "3.2.0", "Workflow composition system"},
        {"WorkflowIntegration", "3.2.0", "Workflow integration framework"},
        {"WorkflowOrchestration", "3.2.0", "Advanced workflow orchestration"},
        {"WorkflowTransactions", "3.2.0", "Transactional workflow support"},

        // Bridge components
        {"PythonPluginBridge", "3.2.0", "Python plugin bridge"},
        {"LuaPluginBridge", "3.2.0", "Lua plugin bridge"},

        // Remote plugin components
        {"RemotePluginManager", "3.2.0", "Remote plugin management"},
        {"RemotePluginLoader", "3.2.0", "Remote plugin loading"},
        {"RemoteSecurityManager", "3.2.0", "Remote plugin security"},
        {"UnifiedPluginManager", "3.2.0",
         "Unified local/remote plugin management"},
        {"RemotePluginConfiguration", "3.2.0", "Remote plugin configuration"},
        {"RemotePluginDiscovery", "3.2.0", "Remote plugin discovery"},
        {"RemotePluginValidator", "3.2.0", "Remote plugin validation"}};
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
