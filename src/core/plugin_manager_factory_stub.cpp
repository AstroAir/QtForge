/**
 * @file plugin_manager_factory_stub.cpp
 * @brief Temporary stub for PluginManager factory function
 * @version 3.0.0
 * @author QtPlugin Development Team
 */

#include "../../include/qtplugin/core/plugin_manager.hpp"
#include "../../include/qtplugin/core/plugin_registry.hpp"
#include "../../include/qtplugin/managers/configuration_manager.hpp"
#include "../../include/qtplugin/managers/logging_manager.hpp"

#include <QObject>

namespace qtplugin {

// Temporary stub implementation for PluginManager factory function
// This provides the missing factory function to complete the build
std::unique_ptr<PluginManager> create_plugin_manager(
    std::shared_ptr<IPluginRegistry> registry,
    std::shared_ptr<IConfigurationManager> config_manager,
    std::shared_ptr<ILoggingManager> logger, QObject* parent) {
    // Return nullptr temporarily to avoid MOC/vtable issues
    // TODO: Implement proper PluginManager once MOC issues are resolved
    Q_UNUSED(registry);
    Q_UNUSED(config_manager);
    Q_UNUSED(logger);
    Q_UNUSED(parent);
    return nullptr;
}

}  // namespace qtplugin
