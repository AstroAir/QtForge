/**
 * @file resource_monitor.cpp
 * @brief Implementation of enhanced resource monitor
 * @version 3.0.0
 */

#include "../qtplugin/managers/components/resource_monitor.hpp"
#include <QDebug>
#include <QLoggingCategory>
#include <algorithm>
#include <numeric>
#include "../qtplugin/managers/components/resource_allocator.hpp"
#include "../qtplugin/managers/components/resource_pool.hpp"

Q_LOGGING_CATEGORY(resourceMonitorLog, "qtplugin.resource.monitor")

namespace qtplugin {

ResourceMonitor::ResourceMonitor(QObject* parent)
    : QObject(parent), m_monitoring_timer(std::make_unique<QTimer>(this)) {
    // Connect timer
    connect(m_monitoring_timer.get(), &QTimer::timeout, this,
            &ResourceMonitor::on_monitoring_timer);

    qCDebug(resourceMonitorLog) << "Resource monitor initialized";
}

ResourceMonitor::~ResourceMonitor() {
    stop_monitoring();
    qCDebug(resourceMonitorLog) << "Resource monitor destroyed";
}

qtplugin::expected<void, PluginError> ResourceMonitor::start_monitoring() {
    if (m_is_monitoring.load()) {
        return make_error<void>(PluginErrorCode::StateError,
                                "Monitoring is already active");
    }

    m_monitoring_timer->start(
        static_cast<int>(m_config.monitoring_interval.count()));
    m_is_monitoring.store(true);

    qCDebug(resourceMonitorLog) << "Resource monitoring started with interval"
                                << m_config.monitoring_interval.count() << "ms";

    emit monitoring_started();

    return make_success();
}

void ResourceMonitor::stop_monitoring() {
    if (!m_is_monitoring.load()) {
        return;
    }

    m_monitoring_timer->stop();
    m_is_monitoring.store(false);

    qCDebug(resourceMonitorLog) << "Resource monitoring stopped";

    emit monitoring_stopped();
}

bool ResourceMonitor::is_monitoring() const { return m_is_monitoring.load(); }

void ResourceMonitor::set_monitoring_config(const MonitoringConfig& config) {
    std::unique_lock lock(m_mutex);
    m_config = config;

    if (m_is_monitoring.load()) {
        m_monitoring_timer->setInterval(
            static_cast<int>(config.monitoring_interval.count()));
    }

    qCDebug(resourceMonitorLog) << "Monitoring configuration updated";
}

MonitoringConfig ResourceMonitor::get_monitoring_config() const {
    std::shared_lock lock(m_mutex);
    return m_config;
}

ResourceSnapshot ResourceMonitor::get_current_snapshot() const {
    std::shared_lock lock(m_mutex);
    return create_snapshot();
}

std::vector<ResourceSnapshot> ResourceMonitor::get_usage_history(
    std::chrono::minutes duration) const {
    std::shared_lock lock(m_mutex);

    auto cutoff_time = std::chrono::system_clock::now() - duration;
    std::vector<ResourceSnapshot> filtered_history;

    for (const auto& snapshot : m_usage_history) {
        if (snapshot.timestamp >= cutoff_time) {
            filtered_history.push_back(snapshot);
        }
    }

    return filtered_history;
}

qtplugin::expected<void, PluginError> ResourceMonitor::add_alert(
    const ResourceAlert& alert) {
    if (alert.name.empty()) {
        return make_error<void>(PluginErrorCode::InvalidArgument,
                                "Alert name cannot be empty");
    }

    std::unique_lock lock(m_mutex);

    if (m_alerts.find(alert.name) != m_alerts.end()) {
        return make_error<void>(PluginErrorCode::InvalidArgument,
                                "Alert already exists: " + alert.name);
    }

    m_alerts[alert.name] = alert;

    qCDebug(resourceMonitorLog)
        << "Added resource alert:" << QString::fromStdString(alert.name);

    return make_success();
}

qtplugin::expected<void, PluginError> ResourceMonitor::remove_alert(
    const std::string& alert_name) {
    std::unique_lock lock(m_mutex);

    auto it = m_alerts.find(alert_name);
    if (it == m_alerts.end()) {
        return make_error<void>(PluginErrorCode::InvalidArgument,
                                "Alert not found: " + alert_name);
    }

    m_alerts.erase(it);

    qCDebug(resourceMonitorLog)
        << "Removed resource alert:" << QString::fromStdString(alert_name);

    return make_success();
}

std::vector<ResourceAlert> ResourceMonitor::get_active_alerts() const {
    std::shared_lock lock(m_mutex);

    std::vector<ResourceAlert> alerts;
    alerts.reserve(m_alerts.size());

    for (const auto& [name, alert] : m_alerts) {
        if (alert.enabled) {
            alerts.push_back(alert);
        }
    }

    return alerts;
}

std::vector<std::string> ResourceMonitor::detect_resource_leaks(
    std::string_view plugin_id) const {
    std::shared_lock lock(m_mutex);

    if (!m_config.enable_leak_detection) {
        return {};
    }

    std::vector<std::string> leaks;

    if (plugin_id.empty()) {
        // Check all plugins
        std::unordered_set<std::string> all_plugins;

        // Collect all plugin IDs from allocators
        for (const auto& allocator : m_monitored_allocators) {
            auto allocations = allocator->get_active_allocations();
            for (const auto& allocation : allocations) {
                all_plugins.insert(allocation.plugin_id);
            }
        }

        // Check each plugin for leaks
        for (const auto& plugin : all_plugins) {
            auto plugin_leaks = detect_leaks_for_plugin(plugin);
            leaks.insert(leaks.end(), plugin_leaks.begin(), plugin_leaks.end());
        }
    } else {
        leaks = detect_leaks_for_plugin(plugin_id);
    }

    return leaks;
}

std::unordered_map<std::string, double>
ResourceMonitor::get_performance_metrics(
    std::optional<ResourceType> resource_type,
    std::string_view plugin_id) const {
    std::shared_lock lock(m_mutex);

    std::unordered_map<std::string, double> metrics;

    if (!m_config.enable_performance_tracking) {
        return metrics;
    }

    // Calculate basic performance metrics
    if (!m_usage_history.empty()) {
        // Average memory usage
        double total_memory = 0.0;
        double total_cpu = 0.0;
        size_t count = 0;

        for (const auto& snapshot : m_usage_history) {
            total_memory += snapshot.total_memory_usage;
            total_cpu += snapshot.cpu_usage_percent;
            count++;
        }

        if (count > 0) {
            metrics["avg_memory_usage"] = total_memory / count;
            metrics["avg_cpu_usage"] = total_cpu / count;
        }

        // Peak usage
        auto max_memory_it = std::max_element(
            m_usage_history.begin(), m_usage_history.end(),
            [](const auto& a, const auto& b) {
                return a.total_memory_usage < b.total_memory_usage;
            });
        if (max_memory_it != m_usage_history.end()) {
            metrics["peak_memory_usage"] =
                static_cast<double>(max_memory_it->total_memory_usage);
        }

        auto max_cpu_it = std::max_element(
            m_usage_history.begin(), m_usage_history.end(),
            [](const auto& a, const auto& b) {
                return a.cpu_usage_percent < b.cpu_usage_percent;
            });
        if (max_cpu_it != m_usage_history.end()) {
            metrics["peak_cpu_usage"] = max_cpu_it->cpu_usage_percent;
        }
    }

    // Allocation efficiency
    if (!m_monitored_allocators.empty()) {
        size_t total_allocations = 0;
        size_t total_failures = 0;

        for (const auto& allocator : m_monitored_allocators) {
            auto stats =
                allocator->get_allocation_statistics(resource_type, plugin_id);
            total_allocations += stats.total_created;
            total_failures += stats.allocation_failures;
        }

        if (total_allocations > 0) {
            metrics["allocation_success_rate"] =
                static_cast<double>(total_allocations - total_failures) /
                total_allocations * 100.0;
        }
    }

    return metrics;
}

void ResourceMonitor::register_pool(
    std::shared_ptr<IComponentResourcePool> pool) {
    if (!pool) {
        return;
    }

    std::unique_lock lock(m_mutex);
    m_monitored_pools.push_back(pool);

    qCDebug(resourceMonitorLog) << "Registered pool for monitoring:"
                                << QString::fromStdString(pool->name());
}

void ResourceMonitor::register_allocator(
    std::shared_ptr<IResourceAllocator> allocator) {
    if (!allocator) {
        return;
    }

    std::unique_lock lock(m_mutex);
    m_monitored_allocators.push_back(allocator);

    qCDebug(resourceMonitorLog) << "Registered allocator for monitoring";
}

void ResourceMonitor::on_monitoring_timer() {
    std::unique_lock lock(m_mutex);

    // Create current snapshot
    ResourceSnapshot snapshot = create_snapshot();

    // Add to history
    m_usage_history.push_back(snapshot);

    // Cleanup old history
    cleanup_old_history();

    // Update performance metrics
    if (m_config.enable_performance_tracking) {
        update_performance_metrics(snapshot);
    }

    // Check alerts
    check_alerts(snapshot);

    lock.unlock();

    // Emit signal
    emit usage_updated(snapshot);
}

ResourceSnapshot ResourceMonitor::create_snapshot() const {
    ResourceSnapshot snapshot;
    snapshot.timestamp = std::chrono::system_clock::now();

    // Collect usage from pools
    for (const auto& pool : m_monitored_pools) {
        ResourceType type = pool->resource_type();
        ResourceUsageStats stats = pool->get_statistics();

        snapshot.usage_by_type[type] = stats;
        snapshot.active_allocations += stats.currently_active;
    }

    // Collect usage from allocators
    for (const auto& allocator : m_monitored_allocators) {
        auto stats = allocator->get_allocation_statistics();
        snapshot.failed_allocations += stats.allocation_failures;

        // Collect per-plugin statistics
        auto allocations = allocator->get_active_allocations();
        std::unordered_map<std::string, ResourceUsageStats> plugin_stats;

        for (const auto& allocation : allocations) {
            plugin_stats[allocation.plugin_id].total_created++;
            // Note: allocation_size not available in ResourceUsageStats
            // plugin_stats[allocation.plugin_id].allocation_size +=
            // allocation.allocation_size;
        }

        for (const auto& [plugin_id, stats] : plugin_stats) {
            snapshot.usage_by_plugin[plugin_id] = stats;
        }
    }

    // Calculate system metrics
    snapshot.total_memory_usage = calculate_total_memory_usage();
    snapshot.cpu_usage_percent = calculate_cpu_usage();

    return snapshot;
}

void ResourceMonitor::cleanup_old_history() {
    if (m_usage_history.size() <= m_config.max_history_entries) {
        return;
    }

    auto cutoff_time =
        std::chrono::system_clock::now() - m_config.history_retention;

    m_usage_history.erase(
        std::remove_if(m_usage_history.begin(), m_usage_history.end(),
                       [cutoff_time](const ResourceSnapshot& snapshot) {
                           return snapshot.timestamp < cutoff_time;
                       }),
        m_usage_history.end());

    // Also limit by max entries
    if (m_usage_history.size() > m_config.max_history_entries) {
        size_t to_remove =
            m_usage_history.size() - m_config.max_history_entries;
        m_usage_history.erase(m_usage_history.begin(),
                              m_usage_history.begin() + to_remove);
    }
}

void ResourceMonitor::check_alerts(const ResourceSnapshot& snapshot) {
    auto now = std::chrono::system_clock::now();

    for (auto& [name, alert] : m_alerts) {
        if (!alert.enabled) {
            continue;
        }

        // Check cooldown
        if (now - alert.last_triggered < alert.cooldown) {
            continue;
        }

        // Evaluate condition
        if (evaluate_alert_condition(alert, snapshot)) {
            alert.last_triggered = now;

            // Call callback if set
            if (alert.callback) {
                try {
                    alert.callback(snapshot);
                } catch (const std::exception& e) {
                    qCWarning(resourceMonitorLog)
                        << "Alert callback failed:" << e.what();
                }
            }

            emit alert_triggered(QString::fromStdString(name), snapshot);
        }
    }
}

bool ResourceMonitor::evaluate_alert_condition(
    const ResourceAlert& alert, const ResourceSnapshot& snapshot) const {
    // This is a simplified condition evaluator
    // In a real implementation, you would have a proper expression parser

    if (alert.condition == "memory_usage > 80%") {
        return snapshot.total_memory_usage >
               (1024 * 1024 * 1024);  // 1GB threshold
    } else if (alert.condition == "cpu_usage > 90%") {
        return snapshot.cpu_usage_percent > 90.0;
    } else if (alert.condition == "failed_allocations > 10") {
        return snapshot.failed_allocations > 10;
    }

    return false;
}

std::vector<std::string> ResourceMonitor::detect_leaks_for_plugin(
    std::string_view plugin_id) const {
    std::vector<std::string> leaks;

    // Simple leak detection: check for long-running allocations
    for (const auto& allocator : m_monitored_allocators) {
        auto allocations = allocator->get_active_allocations(plugin_id);

        auto now = std::chrono::system_clock::now();
        for (const auto& allocation : allocations) {
            auto age = now - allocation.allocated_at;
            if (age > std::chrono::hours(1)) {  // 1 hour threshold
                leaks.push_back(
                    "Long-running allocation: " + allocation.allocation_id +
                    " (age: " +
                    std::to_string(
                        std::chrono::duration_cast<std::chrono::minutes>(age)
                            .count()) +
                    " minutes)");
            }
        }
    }

    return leaks;
}

double ResourceMonitor::calculate_cpu_usage() const {
    // This is a simplified implementation
    // In a real system, you would measure actual CPU usage
    return 0.0;  // Placeholder
}

size_t ResourceMonitor::calculate_total_memory_usage() const {
    size_t total = 0;

    for (const auto& pool : m_monitored_pools) {
        auto stats = pool->get_statistics();
        // Note: memory_usage_bytes not available in ResourceUsageStats
        // Using a simple estimate based on active resources
        total += stats.currently_active * 1024;  // Rough estimate
    }

    return total;
}

void ResourceMonitor::update_performance_metrics(
    const ResourceSnapshot& snapshot) {
    // Update performance history
    m_performance_history["memory_usage"].push_back(
        static_cast<double>(snapshot.total_memory_usage));
    m_performance_history["cpu_usage"].push_back(snapshot.cpu_usage_percent);
    m_performance_history["active_allocations"].push_back(
        static_cast<double>(snapshot.active_allocations));

    // Limit history size
    const size_t max_history = 100;
    for (auto& [metric, history] : m_performance_history) {
        if (history.size() > max_history) {
            history.erase(history.begin(),
                          history.begin() + (history.size() - max_history));
        }
    }
}

}  // namespace qtplugin
