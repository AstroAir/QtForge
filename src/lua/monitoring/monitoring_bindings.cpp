/**
 * @file monitoring_bindings.cpp
 * @brief Monitoring bindings for Lua
 * @version 3.2.0
 */

#include <QDebug>
#include <QLoggingCategory>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include "../qtplugin/monitoring/plugin_hot_reload_manager.hpp"
#include "../qtplugin/monitoring/plugin_metrics_collector.hpp"
#include "../qt_conversions.cpp"

Q_LOGGING_CATEGORY(monitoringBindingsLog, "qtforge.lua.monitoring");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register IPluginHotReloadManager interface with Lua
 */
void register_hot_reload_manager_bindings(sol::state& lua) {
    auto hot_reload_type = lua.new_usertype<qtplugin::IPluginHotReloadManager>("IPluginHotReloadManager");

    hot_reload_type["enable_hot_reload"] = &qtplugin::IPluginHotReloadManager::enable_hot_reload;
    hot_reload_type["disable_hot_reload"] = &qtplugin::IPluginHotReloadManager::disable_hot_reload;
    hot_reload_type["is_hot_reload_enabled"] = &qtplugin::IPluginHotReloadManager::is_hot_reload_enabled;
    hot_reload_type["get_hot_reload_plugins"] = &qtplugin::IPluginHotReloadManager::get_hot_reload_plugins;
    hot_reload_type["clear"] = &qtplugin::IPluginHotReloadManager::clear;
    hot_reload_type["set_global_hot_reload_enabled"] = &qtplugin::IPluginHotReloadManager::set_global_hot_reload_enabled;
    hot_reload_type["is_global_hot_reload_enabled"] = &qtplugin::IPluginHotReloadManager::is_global_hot_reload_enabled;

    qCDebug(monitoringBindingsLog) << "IPluginHotReloadManager bindings registered";
}

/**
 * @brief Register IPluginMetricsCollector interface with Lua
 */
void register_metrics_collector_bindings(sol::state& lua) {
    auto metrics_type = lua.new_usertype<qtplugin::IPluginMetricsCollector>("IPluginMetricsCollector");

    metrics_type["start_monitoring"] = &qtplugin::IPluginMetricsCollector::start_monitoring;
    metrics_type["stop_monitoring"] = &qtplugin::IPluginMetricsCollector::stop_monitoring;
    metrics_type["is_monitoring_active"] = &qtplugin::IPluginMetricsCollector::is_monitoring_active;
    metrics_type["clear_metrics"] = &qtplugin::IPluginMetricsCollector::clear_metrics;
    metrics_type["set_monitoring_interval"] = &qtplugin::IPluginMetricsCollector::set_monitoring_interval;
    metrics_type["get_monitoring_interval"] = &qtplugin::IPluginMetricsCollector::get_monitoring_interval;

    qCDebug(monitoringBindingsLog) << "IPluginMetricsCollector bindings registered";
}

void register_monitoring_bindings(sol::state& lua) {
    qCDebug(monitoringBindingsLog) << "Registering monitoring bindings...";

    // Create qtforge.monitoring namespace
    sol::table qtforge = lua["qtforge"];
    sol::table monitoring = qtforge.get_or_create<sol::table>("monitoring");

    // Register monitoring interfaces
    register_hot_reload_manager_bindings(lua);
    register_metrics_collector_bindings(lua);

    // Factory functions
    monitoring["create_hot_reload_manager"] = []() {
        return std::make_shared<qtplugin::PluginHotReloadManager>();
    };

    monitoring["create_metrics_collector"] = []() {
        return std::make_shared<qtplugin::PluginMetricsCollector>();
    };

    qCDebug(monitoringBindingsLog) << "Monitoring bindings registered successfully";
}

#else // QTFORGE_LUA_BINDINGS not defined

namespace sol { class state; }

void register_monitoring_bindings(sol::state& lua) {
    (void)lua;
    qCWarning(monitoringBindingsLog) << "Monitoring bindings not available - Lua support not compiled";
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
