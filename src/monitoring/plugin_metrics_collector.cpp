/**
 * @file plugin_metrics_collector.cpp
 * @brief Implementation of plugin metrics collector
 * @version 3.0.0
 */

#include "qtplugin/monitoring/plugin_metrics_collector.hpp"
#include "qtplugin/core/plugin_manager.hpp"
#include "qtplugin/core/plugin_registry.hpp"

#include <QDebug>
#include <QLoggingCategory>
#include <QMutexLocker>
#include <limits>

Q_LOGGING_CATEGORY(metricsCollectorLog, "qtplugin.metrics")

namespace qtplugin {

PluginMetricsCollector::PluginMetricsCollector(QObject* parent)
    : QObject(parent),
      m_monitoring_timer(std::make_unique<QTimer>(this)),
      m_last_cleanup_time(std::chrono::system_clock::now()) {
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
    if (interval.count() <= 0) {
        qCWarning(metricsCollectorLog)
            << "Invalid monitoring interval:" << interval.count() << "ms";
        return;
    }

    if (m_monitoring_active.load()) {
        qCDebug(metricsCollectorLog) << "Monitoring already active";
        return;
    }

    if (!is_ready_for_monitoring()) {
        qCWarning(metricsCollectorLog)
            << "Cannot start monitoring: plugin registry not set";
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
    if (plugin_id.empty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Plugin ID cannot be empty");
    }

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

    // Store metrics in history
    store_metrics_in_history(plugin_id, metrics);

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
    if (plugin_id.empty()) {
        qCWarning(metricsCollectorLog) << "Plugin ID cannot be empty";
        return QJsonObject();
    }

    // Try to get from cache first for better performance
    {
        QMutexLocker locker(&m_metrics_mutex);
        auto cache_it = m_plugin_metrics_cache.find(plugin_id);
        if (cache_it != m_plugin_metrics_cache.end()) {
            return cache_it->second;
        }
    }

    if (!plugin_registry) {
        qCWarning(metricsCollectorLog) << "Plugin registry cannot be null";
        return QJsonObject();
    }

    auto plugin_info_opt = plugin_registry->get_plugin_info(plugin_id);
    if (!plugin_info_opt) {
        qCDebug(metricsCollectorLog) << "Plugin not found:" << QString::fromStdString(plugin_id);
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
    QMutexLocker locker(&m_metrics_mutex);

    // Clear all cached metrics
    m_plugin_metrics_cache.clear();

    // Clear metrics history
    m_metrics_history.clear();

    // Reset cleanup time
    m_last_cleanup_time = std::chrono::system_clock::now();

    qCDebug(metricsCollectorLog) << "All metrics data cleared";
}

void PluginMetricsCollector::set_monitoring_interval(
    std::chrono::milliseconds interval) {
    if (interval.count() <= 0) {
        qCWarning(metricsCollectorLog)
            << "Invalid monitoring interval:" << interval.count() << "ms, ignoring";
        return;
    }

    // Limit maximum interval to prevent overflow
    const auto max_interval = std::chrono::milliseconds(std::numeric_limits<int>::max());
    if (interval > max_interval) {
        qCWarning(metricsCollectorLog)
            << "Monitoring interval too large:" << interval.count()
            << "ms, clamping to:" << max_interval.count() << "ms";
        interval = max_interval;
    }

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

void PluginMetricsCollector::set_plugin_registry(IPluginRegistry* plugin_registry) {
    if (m_plugin_registry == plugin_registry) {
        qCDebug(metricsCollectorLog) << "Plugin registry already set to the same instance";
        return;
    }

    // If we're currently monitoring and changing registry, we should clear metrics
    if (m_plugin_registry != nullptr && plugin_registry != m_plugin_registry) {
        qCDebug(metricsCollectorLog) << "Plugin registry changed, clearing existing metrics";
        clear_metrics();
    }

    m_plugin_registry = plugin_registry;

    if (plugin_registry) {
        qCDebug(metricsCollectorLog) << "Plugin registry set for metrics collection";
    } else {
        qCDebug(metricsCollectorLog) << "Plugin registry cleared";
    }
}

IPluginRegistry* PluginMetricsCollector::get_plugin_registry() const {
    return m_plugin_registry;
}

std::vector<QJsonObject> PluginMetricsCollector::get_plugin_metrics_history(
    const std::string& plugin_id, size_t max_entries) const {
    if (plugin_id.empty()) {
        qCWarning(metricsCollectorLog) << "Plugin ID cannot be empty";
        return {};
    }

    QMutexLocker locker(&m_metrics_mutex);

    auto it = m_metrics_history.find(plugin_id);
    if (it == m_metrics_history.end()) {
        return {};
    }

    const auto& history = it->second;

    if (max_entries == 0 || max_entries >= history.size()) {
        return history;
    }

    // Return the most recent entries
    size_t start_index = history.size() - max_entries;
    return std::vector<QJsonObject>(history.begin() + start_index, history.end());
}

void PluginMetricsCollector::set_max_history_size(size_t max_size) {
    if (max_size == 0) {
        qCWarning(metricsCollectorLog) << "Maximum history size cannot be zero";
        return;
    }

    QMutexLocker locker(&m_metrics_mutex);
    m_max_history_size = max_size;

    qCDebug(metricsCollectorLog) << "Maximum history size set to:" << max_size;

    // Trigger cleanup to apply new limit
    cleanup_old_metrics();
}

size_t PluginMetricsCollector::get_max_history_size() const {
    QMutexLocker locker(&m_metrics_mutex);
    return m_max_history_size;
}

bool PluginMetricsCollector::is_ready_for_monitoring() const {
    return m_plugin_registry != nullptr;
}

void PluginMetricsCollector::on_monitoring_timer() {
    if (!m_monitoring_active.load()) {
        qCDebug(metricsCollectorLog) << "Monitoring timer fired but monitoring is not active";
        return;
    }

    if (!m_plugin_registry) {
        qCWarning(metricsCollectorLog) << "Monitoring timer fired but no plugin registry is set";
        return;
    }

    try {
        update_all_metrics(m_plugin_registry);

        // Periodically cleanup old metrics
        cleanup_old_metrics();
    } catch (const std::exception& e) {
        qCWarning(metricsCollectorLog) << "Exception during metrics collection:" << e.what();
    } catch (...) {
        qCWarning(metricsCollectorLog) << "Unknown exception during metrics collection";
    }
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

void PluginMetricsCollector::cleanup_old_metrics() {
    QMutexLocker locker(&m_metrics_mutex);

    auto now = std::chrono::system_clock::now();

    // Only cleanup every 5 minutes to avoid excessive overhead
    if (now - m_last_cleanup_time < std::chrono::minutes(5)) {
        return;
    }

    m_last_cleanup_time = now;

    // Remove old metrics history entries
    for (auto& [plugin_id, history] : m_metrics_history) {
        if (history.size() > m_max_history_size) {
            // Keep only the most recent entries
            size_t excess = history.size() - m_max_history_size;
            history.erase(history.begin(), history.begin() + excess);
        }
    }

    qCDebug(metricsCollectorLog) << "Metrics cleanup completed";
}

void PluginMetricsCollector::store_metrics_in_history(
    const std::string& plugin_id, const QJsonObject& metrics) {
    QMutexLocker locker(&m_metrics_mutex);

    // Store in cache
    m_plugin_metrics_cache[plugin_id] = metrics;

    // Add timestamp to metrics for history
    QJsonObject timestamped_metrics = metrics;
    timestamped_metrics["collection_timestamp"] =
        QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

    // Store in history
    auto& history = m_metrics_history[plugin_id];
    history.push_back(timestamped_metrics);

    // Trigger cleanup if needed
    if (history.size() > m_max_history_size * 1.2) {
        cleanup_old_metrics();
    }
}

}  // namespace qtplugin
