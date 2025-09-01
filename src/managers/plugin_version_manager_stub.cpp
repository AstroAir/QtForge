/**
 * @file plugin_version_manager_stub.cpp
 * @brief Temporary stub implementation for plugin version manager
 * @version 3.0.0
 * @author QtPlugin Development Team
 */

#include "../../include/qtplugin/managers/plugin_version_manager.hpp"
#include "../../include/qtplugin/core/plugin_registry.hpp"
#include "../../include/qtplugin/managers/configuration_manager.hpp"
#include "../../include/qtplugin/managers/logging_manager.hpp"

#include <QObject>

namespace qtplugin {

// Temporary stub implementation to avoid MOC/vtable issues
std::unique_ptr<IPluginVersionManager> create_plugin_version_manager(
    std::shared_ptr<IPluginRegistry> registry,
    std::shared_ptr<IConfigurationManager> config_manager,
    std::shared_ptr<ILoggingManager> logger, QObject* parent) {
    // Return nullptr temporarily to avoid MOC/vtable issues
    // TODO: Implement proper version manager once MOC issues are resolved
    Q_UNUSED(registry);
    Q_UNUSED(config_manager);
    Q_UNUSED(logger);
    Q_UNUSED(parent);
    return nullptr;
}

}  // namespace qtplugin
