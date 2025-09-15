/**
 * @file qtplugin.hpp
 * @brief Main header for the QtPlugin library v3.0.0
 * @version 3.0.0
 * @author QtPlugin Development Team
 *
 * This is the main header file for the QtPlugin library. Include this file
 * to get access to all core plugin system functionality.
 *
 * Version 3.0.0 introduces a new component-based architecture for improved
 * modularity, testability, and maintainability. The public API remains
 * backward compatible.
 *
 * @example Basic Usage
 * ```cpp
 * #include <qtplugin/qtplugin.hpp>
 *
 * int main() {
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
 * #include <qtplugin/qtplugin_components.hpp>
 *
 * int main() {
 *     // Create custom plugin system with specific components
 *     auto manager = qtplugin::PluginSystemBuilder()
 *         .with_plugin_registry(qtplugin::ComponentFactory::create_plugin_registry())
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
// #include "core/plugin_interface.hpp" - included by plugin_lifecycle_manager.hpp
#include "core/plugin_lifecycle_manager.hpp"  // Re-enabled with custom state machine
#include "core/plugin_loader.hpp"
#include "core/plugin_manager.hpp"  // Re-enabled after fixing MOC issues
#include "core/service_plugin_interface.hpp"

// Qt-dependent components (conditionally included)
#ifdef QT_CORE_LIB
#include "core/plugin_capability_discovery.hpp"
#endif

// Enhanced plugin interfaces
#include "interfaces/data_processor_plugin_interface.hpp"
#ifdef QTPLUGIN_BUILD_NETWORK
#include "interfaces/network_plugin_interface.hpp"
#endif
#ifdef QTPLUGIN_BUILD_UI
#include "interfaces/ui_plugin_interface.hpp"
#endif
// #include "interfaces/scripting_plugin_interface.hpp"  // Temporarily disabled
// due to QJSEngine dependency

// Platform-specific components (temporarily disabled due to conflicts)
// #include "platform/platform_plugin_loader.hpp"
// #include "platform/platform_error_handler.hpp"
// #include "platform/platform_performance_monitor.hpp"

// Communication system
#include "communication/message_bus.hpp"
#include "communication/message_types.hpp"
// #include "communication/typed_event_system.hpp"  // Temporarily disabled
// #include "communication/request_response_system.hpp"  // Temporarily disabled
// Network-dependent headers are conditionally included
#ifdef QT_NETWORK_LIB
#include "communication/plugin_service_discovery.hpp"
#endif

// Managers
#include "managers/configuration_manager.hpp"
#include "managers/logging_manager.hpp"
#include "managers/resource_lifecycle.hpp"
#include "managers/resource_manager.hpp"
// #include "managers/resource_monitor.hpp"  // Temporarily disabled due to
// conflicts

// For specialized component usage, include:
// #include <qtplugin/components.hpp>

// Utilities
#include "utils/concepts.hpp"
#include "utils/error_handling.hpp"
#include "utils/version.hpp"

// Security components removed

// UI components (if available) - using comprehensive interface from interfaces/
// #ifdef QTPLUGIN_BUILD_UI
// #include "ui/ui_plugin_interface.hpp"  // Disabled to avoid duplicate with
// interfaces/ui_plugin_interface.hpp #endif

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
#ifdef QTPLUGIN_NETWORK_SUPPORT
    return true;
#else
    return false;
#endif
}

/**
 * @brief Check if the library was compiled with UI support
 * @return true if UI support is available, false otherwise
 */
inline constexpr bool has_ui_support() noexcept {
#ifdef QTPLUGIN_UI_SUPPORT
    return true;
#else
    return false;
#endif
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
