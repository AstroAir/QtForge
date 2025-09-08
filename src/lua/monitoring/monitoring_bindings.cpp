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

#include <qtplugin/monitoring/plugin_hot_reload_manager.hpp>
#include <qtplugin/monitoring/plugin_metrics_collector.hpp>
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

/**
 * @brief Register PluginMetrics struct with Lua
 */
void register_plugin_metrics_bindings(sol::state& lua) {
    auto metrics_type = lua.new_usertype<qtplugin::PluginMetrics>("PluginMetrics");

    metrics_type["plugin_id"] = &qtplugin::PluginMetrics::plugin_id;
    metrics_type["load_time_ms"] = &qtplugin::PluginMetrics::load_time_ms;
    metrics_type["initialization_time_ms"] = &qtplugin::PluginMetrics::initialization_time_ms;
    metrics_type["total_execution_time_ms"] = &qtplugin::PluginMetrics::total_execution_time_ms;
    metrics_type["command_count"] = &qtplugin::PluginMetrics::command_count;
    metrics_type["error_count"] = &qtplugin::PluginMetrics::error_count;
    metrics_type["memory_usage_bytes"] = &qtplugin::PluginMetrics::memory_usage_bytes;
    metrics_type["cpu_usage_percent"] = &qtplugin::PluginMetrics::cpu_usage_percent;

    // Timestamp
    metrics_type["last_activity"] = sol::property(
        [](const qtplugin::PluginMetrics& metrics) -> double {
            auto duration = metrics.last_activity.time_since_epoch();
            return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
        }
    );

    qCDebug(monitoringBindingsLog) << "PluginMetrics bindings registered";
}

/**
 * @brief Register PluginHealthStatus enum and struct
 */
void register_plugin_health_bindings(sol::state& lua) {
    // Health status enum
    lua.new_enum<qtplugin::PluginHealthStatus>("PluginHealthStatus", {
        {"Unknown", qtplugin::PluginHealthStatus::Unknown},
        {"Healthy", qtplugin::PluginHealthStatus::Healthy},
        {"Warning", qtplugin::PluginHealthStatus::Warning},
        {"Critical", qtplugin::PluginHealthStatus::Critical},
        {"Unhealthy", qtplugin::PluginHealthStatus::Unhealthy}
    });

    auto health_type = lua.new_usertype<qtplugin::PluginHealth>("PluginHealth");
    health_type["plugin_id"] = &qtplugin::PluginHealth::plugin_id;
    health_type["status"] = &qtplugin::PluginHealth::status;
    health_type["message"] = &qtplugin::PluginHealth::message;
    health_type["score"] = &qtplugin::PluginHealth::score;

    // Timestamp
    health_type["last_check"] = sol::property(
        [](const qtplugin::PluginHealth& health) -> double {
            auto duration = health.last_check.time_since_epoch();
            return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
        }
    );

    qCDebug(monitoringBindingsLog) << "PluginHealth bindings registered";
}

void register_monitoring_bindings(sol::state& lua) {
    qCDebug(monitoringBindingsLog) << "Registering monitoring bindings...";

    // Create qtforge.monitoring namespace
    sol::table qtforge = lua["qtforge"];
    sol::table monitoring = qtforge.get_or_create<sol::table>("monitoring");

    // Register monitoring interfaces and types
    register_hot_reload_manager_bindings(lua);
    register_metrics_collector_bindings(lua);
    register_plugin_metrics_bindings(lua);
    register_plugin_health_bindings(lua);

    // Factory functions
    monitoring["create_hot_reload_manager"] = []() {
        return std::make_shared<qtplugin::PluginHotReloadManager>();
    };

    monitoring["create_metrics_collector"] = []() {
        return std::make_shared<qtplugin::PluginMetricsCollector>();
    };

    // Utility functions
    monitoring["health_status_to_string"] = [](qtplugin::PluginHealthStatus status) -> std::string {
        switch (status) {
            case qtplugin::PluginHealthStatus::Unknown: return "Unknown";
            case qtplugin::PluginHealthStatus::Healthy: return "Healthy";
            case qtplugin::PluginHealthStatus::Warning: return "Warning";
            case qtplugin::PluginHealthStatus::Critical: return "Critical";
            case qtplugin::PluginHealthStatus::Unhealthy: return "Unhealthy";
            default: return "Unknown";
        }
    };

    monitoring["create_metrics"] = [](const std::string& plugin_id) {
        qtplugin::PluginMetrics metrics;
        metrics.plugin_id = plugin_id;
        metrics.last_activity = std::chrono::system_clock::now();
        return metrics;
    };

    monitoring["create_health"] = [](const std::string& plugin_id, qtplugin::PluginHealthStatus status) {
        qtplugin::PluginHealth health;
        health.plugin_id = plugin_id;
        health.status = status;
        health.last_check = std::chrono::system_clock::now();
        return health;
    };

    // Performance monitoring utilities
    monitoring["get_current_memory_usage"] = []() -> size_t {
        // Basic implementation - in real scenario would use platform-specific APIs
        return 0; // Placeholder
    };

    monitoring["get_current_cpu_usage"] = []() -> double {
        // Basic implementation - in real scenario would use platform-specific APIs
        return 0.0; // Placeholder
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
