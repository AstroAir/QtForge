/**
 * @file plugin_metrics_collector.cpp
 * @brief Implementation of plugin metrics collector
 * @version 3.0.0
 */

#include "../../include/qtplugin/monitoring/plugin_metrics_collector.hpp"
#include <QDebug>
#include <QLoggingCategory>
#include <algorithm>
#include "../../include/qtplugin/core/plugin_manager.hpp"
#include "../../include/qtplugin/core/plugin_registry.hpp"

Q_LOGGING_CATEGORY(metricsCollectorLog, "qtplugin.metrics")

namespace qtplugin {

PluginMetricsCollector::PluginMetricsCollector(QObject* parent)
    : QObject(parent), m_monitoring_timer(std::make_unique<QTimer>(this)) {
    // Connect monitoring timer
    connect(m_monitoring_timer.get(), &QTimer::timeout, this,
            &PluginMetricsCollector::on_monitoring_timer);

    qCDebug(metricsCollectorLog) << "Plugin metrics collector initialized";
}

PluginMetricsCollector::~PluginMetricsCollector() {
    stop_monitoring();
    qCDebug(metricsCollectorLog) << "Plugin metrics collector destroyed";
}

void PluginMetricsCollector::start_monitoring(
    std::chrono::milliseconds interval) {
    if (m_monitoring_active.load()) {
        qCDebug(metricsCollectorLog) << "Monitoring already active";
        return;
    }

    m_monitoring_interval = interval;
    m_monitoring_active = true;

    m_monitoring_timer->start(static_cast<int>(interval.count()));

    qCDebug(metricsCollectorLog)
        << "Monitoring started with interval:" << interval.count() << "ms";
    emit monitoring_started();
}

void PluginMetricsCollector::stop_monitoring() {
    if (!m_monitoring_active.load()) {
        return;
    }

    m_monitoring_active = false;
    m_monitoring_timer->stop();

    qCDebug(metricsCollectorLog) << "Monitoring stopped";
    emit monitoring_stopped();
}

bool PluginMetricsCollector::is_monitoring_active() const {
    return m_monitoring_active.load();
}

qtplugin::expected<void, PluginError>
PluginMetricsCollector::update_plugin_metrics(
    const std::string& plugin_id, IPluginRegistry* plugin_registry) {
    if (!plugin_registry) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Plugin registry cannot be null");
    }

    auto plugin_info_opt = plugin_registry->get_plugin_info(plugin_id);
    if (!plugin_info_opt) {
        return make_error<void>(PluginErrorCode::NotFound,
                                "Plugin not found: " + plugin_id);
    }

    const auto& plugin_info = plugin_info_opt.value();
    if (!plugin_info.instance) {
        return make_error<void>(PluginErrorCode::StateError,
                                "Plugin instance is null");
    }

    // Calculate and update metrics
    QJsonObject metrics = calculate_plugin_metrics(plugin_id, plugin_registry);

    // Update plugin info with new metrics (this would need to be done through
    // the registry)
    auto update_result =
        plugin_registry->update_plugin_info(plugin_id, plugin_info);
    if (!update_result) {
        return update_result;
    }

    qCDebug(metricsCollectorLog)
        << "Updated metrics for plugin:" << QString::fromStdString(plugin_id);
    emit plugin_metrics_updated(QString::fromStdString(plugin_id));

    return make_success();
}

QJsonObject PluginMetricsCollector::get_plugin_metrics(
    const std::string& plugin_id, IPluginRegistry* plugin_registry) const {
    if (!plugin_registry) {
        return QJsonObject();
    }

    auto plugin_info_opt = plugin_registry->get_plugin_info(plugin_id);
    if (!plugin_info_opt) {
        return QJsonObject();
    }

    return plugin_info_opt.value().metrics;
}

QJsonObject PluginMetricsCollector::get_system_metrics(
    IPluginRegistry* plugin_registry) const {
    if (!plugin_registry) {
        return QJsonObject();
    }

    QJsonObject metrics;

    // Get all plugin information
    auto all_plugin_info = plugin_registry->get_all_plugin_info();

    // Count plugins by state
    int total_plugins = static_cast<int>(all_plugin_info.size());
    int loaded_plugins = 0;
    int failed_plugins = 0;
    int unloaded_plugins = 0;
    int initializing_plugins = 0;

    for (const auto& plugin_info : all_plugin_info) {
        switch (plugin_info.state) {
            case PluginState::Loaded:
            case PluginState::Running:
            case PluginState::Paused:
                loaded_plugins++;
                break;
            case PluginState::Error:
                failed_plugins++;
                break;
            case PluginState::Unloaded:
            case PluginState::Stopped:
                unloaded_plugins++;
                break;
            case PluginState::Initializing:
            case PluginState::Loading:
            case PluginState::Reloading:
                initializing_plugins++;
                break;
            case PluginState::Stopping:
                // Plugins that are stopping are transitioning to unloaded
                unloaded_plugins++;
                break;
        }
    }

    metrics["total_plugins"] = total_plugins;
    metrics["loaded_plugins"] = loaded_plugins;
    metrics["failed_plugins"] = failed_plugins;
    metrics["unloaded_plugins"] = unloaded_plugins;
    metrics["initializing_plugins"] = initializing_plugins;

    // Calculate memory usage (basic estimation)
    size_t estimated_memory = 0;
    for (const auto& plugin_info : all_plugin_info) {
        // Basic estimation: plugin info + metadata + configuration
        estimated_memory += sizeof(PluginInfo);
        estimated_memory += plugin_info.metadata.name.size();
        estimated_memory += plugin_info.metadata.description.size();
        estimated_memory +=
            plugin_info.error_log.size() * 100;  // Rough estimate
    }
    metrics["estimated_memory_bytes"] = static_cast<qint64>(estimated_memory);

    // System uptime (time since first plugin was loaded)
    if (!all_plugin_info.empty()) {
        auto earliest_load_time = std::chrono::system_clock::now();
        for (const auto& plugin_info : all_plugin_info) {
            if (plugin_info.load_time < earliest_load_time) {
                earliest_load_time = plugin_info.load_time;
            }
        }

        auto uptime_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - earliest_load_time)
                .count();
        metrics["system_uptime_ms"] = static_cast<qint64>(uptime_ms);
    } else {
        metrics["system_uptime_ms"] = 0;
    }

    // Monitoring status
    metrics["monitoring_active"] = m_monitoring_active.load();
    metrics["monitoring_interval_ms"] =
        static_cast<qint64>(m_monitoring_interval.count());

    return metrics;
}

void PluginMetricsCollector::update_all_metrics(
    IPluginRegistry* plugin_registry) {
    if (!plugin_registry) {
        return;
    }

    auto plugin_ids = plugin_registry->get_all_plugin_ids();

    for (const auto& plugin_id : plugin_ids) {
        update_plugin_metrics(plugin_id, plugin_registry);
    }

    emit system_metrics_updated();
}

void PluginMetricsCollector::clear_metrics() {
    qCDebug(metricsCollectorLog) << "Metrics cleared";
}

void PluginMetricsCollector::set_monitoring_interval(
    std::chrono::milliseconds interval) {
    m_monitoring_interval = interval;

    if (m_monitoring_active.load()) {
        m_monitoring_timer->setInterval(static_cast<int>(interval.count()));
    }

    qCDebug(metricsCollectorLog)
        << "Monitoring interval set to:" << interval.count() << "ms";
}

std::chrono::milliseconds PluginMetricsCollector::get_monitoring_interval()
    const {
    return m_monitoring_interval;
}

void PluginMetricsCollector::on_monitoring_timer() {
    if (!m_monitoring_active.load() || !m_plugin_registry) {
        return;
    }

    update_all_metrics(m_plugin_registry);
}

std::string PluginMetricsCollector::plugin_state_to_string(int state) const {
    switch (static_cast<PluginState>(state)) {
        case PluginState::Unloaded:
            return "Unloaded";
        case PluginState::Loading:
            return "Loading";
        case PluginState::Loaded:
            return "Loaded";
        case PluginState::Initializing:
            return "Initializing";
        case PluginState::Running:
            return "Running";
        case PluginState::Stopping:
            return "Stopping";
        case PluginState::Stopped:
            return "Stopped";
        case PluginState::Error:
            return "Error";
        default:
            return "Unknown";
    }
}

QJsonObject PluginMetricsCollector::calculate_plugin_metrics(
    const std::string& plugin_id, IPluginRegistry* plugin_registry) const {
    QJsonObject metrics;

    auto plugin_info_opt = plugin_registry->get_plugin_info(plugin_id);
    if (!plugin_info_opt) {
        return metrics;
    }

    const auto& plugin_info = plugin_info_opt.value();

    // Update basic metrics
    auto now = std::chrono::system_clock::now();

    // Calculate uptime
    auto uptime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now - plugin_info.load_time)
                         .count();
    metrics["uptime_ms"] = static_cast<qint64>(uptime_ms);

    // Update activity timestamp
    metrics["last_activity"] =
        QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch())
                            .count());

    // Get plugin-specific metrics if available
    try {
        if (plugin_info.instance &&
            plugin_info.instance->capabilities() &
                static_cast<PluginCapabilities>(PluginCapability::Monitoring)) {
            // Try to get metrics from plugin
            auto plugin_metrics_result =
                plugin_info.instance->execute_command("get_metrics");
            if (plugin_metrics_result) {
                metrics["plugin_metrics"] = plugin_metrics_result.value();
            }
        }
    } catch (...) {
        // Ignore errors in metrics collection
    }

    // Update error count
    metrics["error_count"] = static_cast<int>(plugin_info.error_log.size());

    // Update state information
    metrics["state"] = static_cast<int>(plugin_info.state);
    metrics["state_name"] = QString::fromStdString(
        plugin_state_to_string(static_cast<int>(plugin_info.state)));

    return metrics;
}

}  // namespace qtplugin
