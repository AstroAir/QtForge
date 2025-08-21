/**
 * @file qtplugin.cpp
 * @brief Implementation of main QtPlugin library functions
 * @version 3.0.0
 */

#include "qtplugin/qtplugin.hpp"
#include <QCoreApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(qtpluginLog, "qtplugin")

namespace qtplugin {

bool initialize() {
    // Register Qt types for the plugin system
    qRegisterMetaType<PluginState>("PluginState");
    qRegisterMetaType<PluginCapability>("PluginCapability");
    qRegisterMetaType<PluginPriority>("PluginPriority");
    qRegisterMetaType<SecurityLevel>("SecurityLevel");

    // Set up logging
    QLoggingCategory::setFilterRules("qtplugin.debug=true");

    qCDebug(qtpluginLog) << "QtPlugin library initialized, version"
                         << version();

    return true;
}

void cleanup() { qCDebug(qtpluginLog) << "QtPlugin library cleanup completed"; }

}  // namespace qtplugin
