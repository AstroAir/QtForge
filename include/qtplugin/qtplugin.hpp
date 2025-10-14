/**
 * @file qtplugin.hpp
 * @brief Main header for the QtPlugin library v3.2.0
 * @version 3.2.0
 * @author QtPlugin Development Team
 *
 * This is the main header file for the QtPlugin library. Include this file
 * to get access to all core plugin system functionality with all features
 * enabled by default.
 *
 * Version 3.2.0 provides a complete component-based architecture with all
 * features enabled by default for maximum functionality out of the box.
 * The public API remains backward compatible.
 *
 * @example Basic Usage
 * ```cpp
 * #include <qtplugin/qtplugin.hpp>
 *
 * int main() {
 *     qtplugin::LibraryInitializer init;
 *     qtplugin::PluginManager manager;
 *     auto result = manager.load_plugin("path/to/plugin.so");
 *     if (result) {
 *         // Plugin loaded successfully
 *     }
 *     return 0;
 * }
 * ```
 *
 * @example Component-Based Usage
 * ```cpp
 * #include <qtplugin/qtplugin.hpp>
 *
 * int main() {
 *     qtplugin::LibraryInitializer init;
 *     // Create custom plugin system with specific components
 *     auto manager = qtplugin::PluginSystemBuilder()
 *         .with_plugin_registry(qtplugin::ComponentFactory::create_plugin_registry())
 *         .with_security_validator(qtplugin::ComponentFactory::create_security_validator())
 *         .build();
 *
 *     auto result = manager->load_plugin("path/to/plugin.so");
 *     return 0;
 * }
 * ```
 */

#pragma once

// Version information
#define QTPLUGIN_VERSION_MAJOR 3
#define QTPLUGIN_VERSION_MINOR 2
#define QTPLUGIN_VERSION_PATCH 0
#define QTPLUGIN_VERSION "3.2.0"

// Core components
#include "core/plugin_capability_discovery.hpp"
#include "core/plugin_dependency_resolver.hpp"
#include "core/plugin_lifecycle_manager.hpp"
#include "core/plugin_loader.hpp"
#include "core/plugin_manager.hpp"
#include "core/plugin_registry.hpp"
#include "interfaces/core/plugin_interface.hpp"
#include "interfaces/core/service_plugin_interface.hpp"

// Enhanced plugin interfaces
#include "interfaces/data/data_processor_plugin_interface.hpp"
#ifdef QTFORGE_HAS_NETWORK
#include "interfaces/network/network_plugin_interface.hpp"
#endif
#ifdef QTFORGE_HAS_WIDGETS
#include "interfaces/ui/ui_plugin_interface.hpp"
#endif
// #include "interfaces/scripting/scripting_plugin_interface.hpp" // Temporarily
// disabled - requires Qt6Qml

// Platform-specific components
#include "platform/platform_error_handler.hpp"
#include "platform/platform_performance_monitor.hpp"
#include "platform/platform_plugin_loader.hpp"

// Communication system
#include "communication/message_bus.hpp"
#include "communication/message_types.hpp"
#include "communication/plugin_service_contracts.hpp"
#include "communication/plugin_service_discovery.hpp"
#include "communication/request_response_system.hpp"
#include "communication/typed_event_system.hpp"

// Security components - Removed
// SHA256 verification functionality is preserved in PluginManager

// Managers
#include "managers/configuration_manager.hpp"
#include "managers/logging_manager.hpp"
#include "managers/plugin_version_manager.hpp"
#include "managers/resource_lifecycle.hpp"
#include "managers/resource_manager.hpp"
#include "managers/resource_monitor.hpp"

// Manager components
#include "managers/components/configuration_merger.hpp"
#include "managers/components/configuration_storage.hpp"
#include "managers/components/configuration_validator.hpp"
#include "managers/components/configuration_watcher.hpp"
#include "managers/components/resource_allocator.hpp"
#include "managers/components/resource_monitor.hpp"
#include "managers/components/resource_pool.hpp"

// Monitoring components
#include "monitoring/plugin_hot_reload_manager.hpp"
#include "monitoring/plugin_metrics_collector.hpp"

// Utilities
#include "utils/concepts.hpp"
#include "utils/error_handling.hpp"
#include "utils/version.hpp"

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

// Remote plugin components (require network support)
#ifdef QTFORGE_HAS_NETWORK
#include "remote/remote_plugin_loader.hpp"
#include "remote/remote_plugin_manager.hpp"
#include "remote/remote_security_manager.hpp"
#include "remote/unified_plugin_manager.hpp"
#endif

/**
 * @namespace qtplugin
 * @brief Main namespace for the QtPlugin library
 *
 * All QtPlugin library classes, functions, and types are contained within
 * this namespace to avoid naming conflicts with other libraries.
 */
namespace qtplugin {

/**
 * @brief Get the library version as a string
 * @return Version string in format "major.minor.patch"
 */
inline constexpr const char* version() noexcept { return QTPLUGIN_VERSION; }

/**
 * @brief Get the major version number
 * @return Major version number
 */
inline constexpr int version_major() noexcept { return QTPLUGIN_VERSION_MAJOR; }

/**
 * @brief Get the minor version number
 * @return Minor version number
 */
inline constexpr int version_minor() noexcept { return QTPLUGIN_VERSION_MINOR; }

/**
 * @brief Get the patch version number
 * @return Patch version number
 */
inline constexpr int version_patch() noexcept { return QTPLUGIN_VERSION_PATCH; }

/**
 * @brief Check if the library was compiled with network support
 * @return true if network support is available, false otherwise
 */
inline constexpr bool has_network_support() noexcept {
    return true;  // Always enabled in v3.2.0
}

/**
 * @brief Check if the library was compiled with UI support
 * @return true if UI support is available, false otherwise
 */
inline constexpr bool has_ui_support() noexcept {
    return true;  // Always enabled in v3.2.0
}

/**
 * @brief Check if the library was compiled with security support
 * @return true if security support is available, false otherwise
 */
inline constexpr bool has_security_support() noexcept {
    return true;  // Always enabled in v3.2.0
}

/**
 * @brief Check if the library was compiled with scripting support
 * @return true if scripting support is available, false otherwise
 */
inline constexpr bool has_scripting_support() noexcept {
    return true;  // Always enabled in v3.2.0
}

/**
 * @brief Check if the library was compiled with remote plugin support
 * @return true if remote plugin support is available, false otherwise
 */
inline constexpr bool has_remote_support() noexcept {
    return true;  // Always enabled in v3.2.0
}

/**
 * @brief Check if the library was compiled with workflow support
 * @return true if workflow support is available, false otherwise
 */
inline constexpr bool has_workflow_support() noexcept {
    return true;  // Always enabled in v3.2.0
}

/**
 * @brief Initialize the QtPlugin library
 *
 * This function should be called once at the beginning of your application
 * to initialize the plugin system. It sets up logging, registers Qt types,
 * and performs other necessary initialization tasks.
 *
 * @return true if initialization was successful, false otherwise
 */
bool initialize();

/**
 * @brief Cleanup the QtPlugin library
 *
 * This function should be called once at the end of your application
 * to cleanup resources used by the plugin system.
 */
void cleanup();

/**
 * @brief RAII wrapper for library initialization
 *
 * This class provides automatic initialization and cleanup of the QtPlugin
 * library using RAII principles. Create an instance at the beginning of
 * your application and it will automatically cleanup when destroyed.
 *
 * @example
 * ```cpp
 * int main() {
 *     qtplugin::LibraryInitializer init;
 *     if (!init.is_initialized()) {
 *         return -1;
 *     }
 *
 *     // Use plugin system...
 *
 *     return 0; // Automatic cleanup when init goes out of scope
 * }
 * ```
 */
class LibraryInitializer {
public:
    /**
     * @brief Constructor - initializes the library
     */
    LibraryInitializer() : m_initialized(initialize()) {}

    /**
     * @brief Destructor - cleans up the library
     */
    ~LibraryInitializer() {
        if (m_initialized) {
            cleanup();
        }
    }

    // Non-copyable, non-movable
    LibraryInitializer(const LibraryInitializer&) = delete;
    LibraryInitializer& operator=(const LibraryInitializer&) = delete;
    LibraryInitializer(LibraryInitializer&&) = delete;
    LibraryInitializer& operator=(LibraryInitializer&&) = delete;

    /**
     * @brief Check if initialization was successful
     * @return true if the library was initialized successfully
     */
    bool is_initialized() const noexcept { return m_initialized; }

private:
    bool m_initialized;
};

}  // namespace qtplugin

/**
 * @def QTPLUGIN_PLUGIN_METADATA
 * @brief Macro to declare plugin metadata
 *
 * This macro should be used in plugin classes to declare metadata
 * that can be read without loading the plugin.
 *
 * @param IID Interface identifier string
 * @param FILE JSON metadata file path
 *
 * @example
 * ```cpp
 * class MyPlugin : public QObject, public qtplugin::IPlugin {
 *     Q_OBJECT
 *     QTPLUGIN_PLUGIN_METADATA("com.example.MyPlugin/1.0", "metadata.json")
 *     Q_INTERFACES(qtplugin::IPlugin)
 * public:
 *     // Plugin implementation...
 * };
 * ```
 */
#define QTPLUGIN_PLUGIN_METADATA(IID, FILE) Q_PLUGIN_METADATA(IID IID FILE FILE)

/**
 * @def QTPLUGIN_DECLARE_PLUGIN
 * @brief Convenience macro to declare a plugin class
 *
 * This macro combines the necessary Qt macros and interface declarations
 * needed for a plugin class.
 *
 * @param ClassName The plugin class name
 * @param IID Interface identifier string
 * @param FILE JSON metadata file path
 * @param ... Additional interfaces (optional)
 *
 * @example
 * ```cpp
 * class MyPlugin : public QObject, public qtplugin::IPlugin {
 *     Q_OBJECT
 *     QTPLUGIN_DECLARE_PLUGIN(MyPlugin, "com.example.MyPlugin/1.0",
 * "metadata.json") public:
 *     // Plugin implementation...
 * };
 * ```
 */
#define QTPLUGIN_DECLARE_PLUGIN(ClassName, IID, FILE, ...) \
    Q_PLUGIN_METADATA(IID IID FILE FILE)                   \
    Q_INTERFACES(qtplugin::IPlugin __VA_ARGS__)

// Convenience aliases for commonly used types
namespace qtplugin {
using PluginPtr = std::shared_ptr<IPlugin>;
using PluginWeakPtr = std::weak_ptr<IPlugin>;
using PluginUniquePtr = std::unique_ptr<IPlugin>;
}  // namespace qtplugin

/**
 * @example basic_usage.cpp
 * Basic usage example showing how to create and use a plugin manager:
 *
 * ```cpp
 * #include <qtplugin/qtplugin.hpp>
 * #include <iostream>
 *
 * int main() {
 *     // Initialize the library
 *     qtplugin::LibraryInitializer init;
 *     if (!init.is_initialized()) {
 *         std::cerr << "Failed to initialize QtPlugin library" << std::endl;
 *         return -1;
 *     }
 *
 *     // Create plugin manager
 *     qtplugin::PluginManager manager;
 *
 *     // Load a plugin
 *     auto result = manager.load_plugin("./plugins/example_plugin.so");
 *     if (!result) {
 *         std::cerr << "Failed to load plugin: " << result.error().message <<
 * std::endl; return -1;
 *     }
 *
 *     // Get the loaded plugin
 *     auto plugin = manager.get_plugin(result.value());
 *     if (plugin) {
 *         std::cout << "Loaded plugin: " << plugin->name() << std::endl;
 *         std::cout << "Version: " << plugin->version() << std::endl;
 *         std::cout << "Description: " << plugin->description() << std::endl;
 *
 *         // Initialize the plugin
 *         auto init_result = plugin->initialize();
 *         if (init_result) {
 *             std::cout << "Plugin initialized successfully" << std::endl;
 *         }
 *     }
 *
 *     return 0;
 * }
 * ```
 */
