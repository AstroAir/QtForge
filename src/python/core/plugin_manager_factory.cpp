/**
 * @file plugin_manager_factory.cpp
 * @brief Factory for creating PluginManager instances for Python bindings
 * @version 3.0.0
 *
 * This file provides a factory function that creates PluginManager instances
 * by passing nullptr for all dependencies to avoid incomplete type issues.
 */

#include "plugin_manager_factory.hpp"
#include <qtplugin/core/plugin_manager.hpp>

namespace qtforge_python {

std::unique_ptr<qtplugin::PluginManager> create_plugin_manager_for_python() {
    // Create PluginManager with all nullptr dependencies
    // This should work because the constructor should handle nullptr gracefully
    return std::make_unique<qtplugin::PluginManager>(
        nullptr, // loader
        nullptr, // message_bus
        nullptr, // security_manager
        nullptr, // config_manager
        nullptr, // logging_manager
        nullptr, // resource_manager
        nullptr, // lifecycle_manager
        nullptr, // resource_monitor
        nullptr, // plugin_registry
        nullptr, // dependency_resolver
        nullptr, // hot_reload_manager
        nullptr, // metrics_collector
        nullptr, // version_manager
        nullptr  // parent
    );
}

} // namespace qtforge_python
