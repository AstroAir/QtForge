/**
 * @file qtplugin.cpp
 * @brief Implementation of main QtPlugin library functions
 * @version 3.0.0
 */

#include "qtplugin/qtplugin.hpp"

#ifdef QT_CORE_LIB
#include <QCoreApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(qtpluginLog, "qtplugin")
#endif

namespace qtplugin {

bool initialize() {
#ifdef QT_CORE_LIB
    // Register Qt types for the plugin system
    qRegisterMetaType<PluginState>("PluginState");
    qRegisterMetaType<PluginCapability>("PluginCapability");
    qRegisterMetaType<PluginPriority>("PluginPriority");
    qRegisterMetaType<SecurityLevel>("SecurityLevel");

    // Set up logging
    QLoggingCategory::setFilterRules("qtplugin.debug=true");

    qCDebug(qtpluginLog) << "QtPlugin library initialized, version"
                         << version();
#endif
    return true;
}

void cleanup() {
#ifdef QT_CORE_LIB
    qCDebug(qtpluginLog) << "QtPlugin library cleanup completed";
#endif
}

}  // namespace qtplugin
