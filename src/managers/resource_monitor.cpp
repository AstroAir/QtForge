/**
 * @file resource_monitor.cpp
 * @brief Implementation of resource monitoring system
 * @version 3.0.0
 */

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <algorithm>
#include <iomanip>
#include <random>
#include <sstream>
#include "qtplugin/managers/resource_monitor_impl.hpp"

Q_LOGGING_CATEGORY(monitorLog, "qtplugin.monitor")

namespace qtplugin {

// === ResourceMonitor Implementation ===

ResourceMonitor::ResourceMonitor(QObject* parent)
    : QObject(parent),
      m_collection_timer(std::make_unique<QTimer>(this)),
      m_alert_timer(std::make_unique<QTimer>(this)) {
    // Set up collection timer
    m_collection_timer->setSingleShot(false);
    m_collection_timer->setInterval(
        static_cast<int>(m_config.collection_interval.count()));
    connect(m_collection_timer.get(), &QTimer::timeout, this,
            &ResourceMonitor::collect_metrics);
    m_collection_timer->start();

    // Set up alert checking timer
    m_alert_timer->setSingleShot(false);
    m_alert_timer->setInterval(5000);  // Check every 5 seconds
    connect(m_alert_timer.get(), &QTimer::timeout, this,
            &ResourceMonitor::check_quotas_and_alerts);
    m_alert_timer->start();

    qCDebug(monitorLog) << "Resource monitor initialized";
}

ResourceMonitor::~ResourceMonitor() {
    // Stop timers
    m_collection_timer->stop();
    m_alert_timer->stop();

    // Clean up tracked resources
    std::unique_lock<std::shared_mutex> lock(m_resources_mutex);
    m_tracked_resources.clear();

    qCDebug(monitorLog) << "Resource monitor destroyed";
}

qtplugin::expected<void, PluginError> ResourceMonitor::start_monitoring(
    const ResourceHandle& handle) {
    std::unique_lock<std::shared_mutex> lock(m_resources_mutex);

    const std::string& resource_id = handle.id();

    // Check if resource is already being monitored
    if (m_tracked_resources.find(resource_id) != m_tracked_resources.end()) {
        return make_error<void>(
            PluginErrorCode::AlreadyExists,
            "Resource already being monitored: " + resource_id);
    }

    // Create monitoring tracker
    auto tracker = std::make_unique<ResourceMonitoringTracker>(handle);
    m_tracked_resources[resource_id] = std::move(tracker);

    qCDebug(monitorLog) << "Started monitoring resource:"
                        << QString::fromStdString(resource_id);

    return make_success();
}

qtplugin::expected<void, PluginError> ResourceMonitor::stop_monitoring(
    const std::string& resource_id) {
    std::unique_lock<std::shared_mutex> lock(m_resources_mutex);

    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return make_error<void>(PluginErrorCode::NotFound,
                                "Resource not being monitored: " + resource_id);
    }

    // Mark as inactive and keep historical data for a while
    it->second->is_active = false;

    qCDebug(monitorLog) << "Stopped monitoring resource:"
                        << QString::fromStdString(resource_id);

    return make_success();
}

qtplugin::expected<void, PluginError> ResourceMonitor::update_metrics(
    const std::string& resource_id, const ResourceMetrics& metrics) {
    std::unique_lock<std::shared_mutex> lock(m_resources_mutex);

    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return make_error<void>(PluginErrorCode::NotFound,
                                "Resource not being monitored: " + resource_id);
    }

    auto& tracker = it->second;

    // Update current metrics
    tracker->current_metrics = metrics;
    tracker->last_collection = std::chrono::system_clock::now();

    // Add to historical data
    tracker->add_historical_entry(metrics, m_config.max_metrics_per_resource);

    m_total_metrics_collected.fetch_add(1);

    // Check for quota violations and performance issues
    lock.unlock();
    check_resource_quotas(resource_id, metrics);
    check_resource_performance(resource_id, metrics);

    emit metrics_updated(QString::fromStdString(resource_id));

    return make_success();
}

qtplugin::expected<void, PluginError> ResourceMonitor::record_access(
    const std::string& resource_id, std::chrono::milliseconds access_duration) {
    std::unique_lock<std::shared_mutex> lock(m_resources_mutex);

    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return make_error<void>(PluginErrorCode::NotFound,
                                "Resource not being monitored: " + resource_id);
    }

    auto& metrics = it->second->current_metrics;

    // Update access statistics
    metrics.access_count++;
    metrics.total_usage_time += access_duration;
    metrics.last_accessed = std::chrono::system_clock::now();

    qCDebug(monitorLog) << "Recorded access for resource:"
                        << QString::fromStdString(resource_id)
                        << "duration:" << access_duration.count() << "ms";

    return make_success();
}

qtplugin::expected<void, PluginError> ResourceMonitor::record_error(
    const std::string& resource_id, const std::string& error_message) {
    std::unique_lock<std::shared_mutex> lock(m_resources_mutex);

    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return make_error<void>(PluginErrorCode::NotFound,
                                "Resource not being monitored: " + resource_id);
    }

    auto& metrics = it->second->current_metrics;

    // Update error statistics
    metrics.error_count++;
    metrics.last_error = std::chrono::system_clock::now();
    metrics.last_error_message = error_message;

    qCDebug(monitorLog) << "Recorded error for resource:"
                        << QString::fromStdString(resource_id)
                        << "error:" << QString::fromStdString(error_message);

    return make_success();
}

qtplugin::expected<ResourceMetrics, PluginError> ResourceMonitor::get_metrics(
    const std::string& resource_id) const {
    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return make_error<ResourceMetrics>(
            PluginErrorCode::NotFound,
            "Resource not being monitored: " + resource_id);
    }

    return it->second->current_metrics;
}

qtplugin::expected<std::vector<ResourceMetrics>, PluginError>
ResourceMonitor::get_plugin_metrics(const std::string& plugin_id) const {
    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    std::vector<ResourceMetrics> metrics;

    for (const auto& [resource_id, tracker] : m_tracked_resources) {
        if (tracker->current_metrics.plugin_id == plugin_id &&
            tracker->is_active) {
            metrics.push_back(tracker->current_metrics);
        }
    }

    return metrics;
}

qtplugin::expected<ResourceMetrics, PluginError>
ResourceMonitor::get_aggregated_metrics(ResourceType resource_type) const {
    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    ResourceMetrics aggregated;
    aggregated.resource_type = resource_type;
    aggregated.resource_id =
        "aggregated_" + resource_type_to_string(resource_type);

    size_t count = 0;
    double total_cpu = 0.0;
    size_t total_memory = 0;
    size_t total_errors = 0;
    size_t total_accesses = 0;
    std::chrono::milliseconds total_usage{0};

    for (const auto& [resource_id, tracker] : m_tracked_resources) {
        if (tracker->current_metrics.resource_type == resource_type &&
            tracker->is_active) {
            const auto& metrics = tracker->current_metrics;

            total_cpu += metrics.cpu_usage_percent;
            total_memory += metrics.memory_usage_bytes;
            total_errors += metrics.error_count;
            total_accesses += metrics.access_count;
            total_usage += metrics.total_usage_time;

            if (count == 0) {
                aggregated.created_at = metrics.created_at;
            } else {
                aggregated.created_at =
                    std::min(aggregated.created_at, metrics.created_at);
            }

            aggregated.last_accessed =
                std::max(aggregated.last_accessed, metrics.last_accessed);
            count++;
        }
    }

    if (count > 0) {
        aggregated.cpu_usage_percent = total_cpu / count;
        aggregated.memory_usage_bytes = total_memory;
        aggregated.error_count = total_errors;
        aggregated.access_count = total_accesses;
        aggregated.total_usage_time = total_usage;
    }

    return aggregated;
}

qtplugin::expected<std::vector<ResourceMetrics>, PluginError>
ResourceMonitor::get_historical_metrics(
    const std::string& resource_id,
    std::chrono::system_clock::time_point start_time,
    std::chrono::system_clock::time_point end_time) const {
    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return make_error<std::vector<ResourceMetrics>>(
            PluginErrorCode::NotFound,
            "Resource not being monitored: " + resource_id);
    }

    std::vector<ResourceMetrics> historical;

    for (const auto& entry : it->second->historical_data) {
        if (entry.timestamp >= start_time && entry.timestamp <= end_time) {
            historical.push_back(entry.metrics);
        }
    }

    return historical;
}

std::vector<QuotaViolation> ResourceMonitor::check_quota_compliance(
    const std::string& plugin_id, ResourceType resource_type) const {
    std::vector<QuotaViolation> violations;

    // This is a simplified implementation
    // In practice, you'd check against actual quotas and current usage
    Q_UNUSED(plugin_id)
    Q_UNUSED(resource_type)

    return violations;
}

std::vector<QuotaViolation> ResourceMonitor::get_quota_violations(
    std::optional<std::chrono::system_clock::time_point> since_time) const {
    std::shared_lock<std::shared_mutex> lock(m_violations_mutex);

    std::vector<QuotaViolation> violations;

    for (const auto& violation : m_quota_violations) {
        if (!since_time || violation.timestamp >= since_time.value()) {
            violations.push_back(violation);
        }
    }

    return violations;
}

qtplugin::expected<void, PluginError> ResourceMonitor::set_custom_quota(
    const std::string& plugin_id, ResourceType resource_type,
    const std::string& quota_name, double limit) {
    std::unique_lock<std::shared_mutex> lock(m_quotas_mutex);

    m_custom_quotas[plugin_id][resource_type][quota_name] = limit;

    qCDebug(monitorLog) << "Set custom quota for plugin:"
                        << QString::fromStdString(plugin_id) << "type:"
                        << QString::fromStdString(
                               resource_type_to_string(resource_type))
                        << "quota:" << QString::fromStdString(quota_name)
                        << "limit:" << limit;

    return make_success();
}

std::vector<ResourcePerformanceAlert> ResourceMonitor::get_performance_alerts(
    double severity_threshold,
    std::optional<std::chrono::system_clock::time_point> since_time) const {
    std::shared_lock<std::shared_mutex> lock(m_violations_mutex);

    std::vector<ResourcePerformanceAlert> alerts;

    for (const auto& alert : m_performance_alerts) {
        if (alert.severity >= severity_threshold &&
            (!since_time || alert.timestamp >= since_time.value())) {
            alerts.push_back(alert);
        }
    }

    return alerts;
}

QJsonObject ResourceMonitor::get_efficiency_report(
    const std::string& plugin_id,
    std::optional<ResourceType> resource_type) const {
    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    QJsonObject report;
    QJsonArray resources_array;

    double total_efficiency = 0.0;
    size_t count = 0;

    for (const auto& [resource_id, tracker] : m_tracked_resources) {
        const auto& metrics = tracker->current_metrics;

        // Apply filters
        if (!plugin_id.empty() && metrics.plugin_id != plugin_id) {
            continue;
        }

        if (resource_type && metrics.resource_type != resource_type.value()) {
            continue;
        }

        if (!tracker->is_active) {
            continue;
        }

        QJsonObject resource_report;
        resource_report["resource_id"] = QString::fromStdString(resource_id);
        resource_report["plugin_id"] =
            QString::fromStdString(metrics.plugin_id);
        resource_report["resource_type"] = QString::fromStdString(
            resource_type_to_string(metrics.resource_type));

        double efficiency = metrics.calculate_efficiency_score();
        resource_report["efficiency_score"] = efficiency;
        resource_report["is_underutilized"] = metrics.is_underutilized();
        resource_report["is_overutilized"] = metrics.is_overutilized();
        resource_report["access_count"] =
            static_cast<qint64>(metrics.access_count);
        resource_report["error_count"] =
            static_cast<qint64>(metrics.error_count);

        resources_array.append(resource_report);

        total_efficiency += efficiency;
        count++;
    }

    report["resources"] = resources_array;
    report["average_efficiency"] = count > 0 ? total_efficiency / count : 0.0;
    report["total_resources"] = static_cast<qint64>(count);

    return report;
}

std::vector<std::pair<std::string, double>> ResourceMonitor::get_top_consumers(
    const std::string& metric_type, size_t count) const {
    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    std::vector<std::pair<std::string, double>> consumers;

    for (const auto& [resource_id, tracker] : m_tracked_resources) {
        if (!tracker->is_active) {
            continue;
        }

        const auto& metrics = tracker->current_metrics;
        double value = 0.0;

        if (metric_type == "cpu") {
            value = metrics.cpu_usage_percent;
        } else if (metric_type == "memory") {
            value = static_cast<double>(metrics.memory_usage_bytes);
        } else if (metric_type == "access_count") {
            value = static_cast<double>(metrics.access_count);
        } else if (metric_type == "errors") {
            value = static_cast<double>(metrics.error_count);
        }

        consumers.emplace_back(resource_id, value);
    }

    // Sort by value (descending)
    std::sort(consumers.begin(), consumers.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // Return top N
    if (consumers.size() > count) {
        consumers.resize(count);
    }

    return consumers;
}

qtplugin::expected<void, PluginError> ResourceMonitor::set_configuration(
    const MonitoringConfiguration& config) {
    {
        std::unique_lock<std::shared_mutex> lock(m_config_mutex);
        m_config = config;
    }

    // Update timer intervals
    m_collection_timer->setInterval(
        static_cast<int>(config.collection_interval.count()));

    emit monitoring_configuration_changed();

    qCDebug(monitorLog)
        << "Updated monitoring configuration - collection interval:"
        << config.collection_interval.count() << "ms";

    return make_success();
}

MonitoringConfiguration ResourceMonitor::get_configuration() const {
    std::shared_lock<std::shared_mutex> lock(m_config_mutex);
    return m_config;
}

std::string ResourceMonitor::subscribe_to_quota_violations(
    std::function<void(const QuotaViolation&)> callback,
    const std::string& plugin_filter) {
    std::unique_lock<std::shared_mutex> lock(m_subscriptions_mutex);

    std::string subscription_id = generate_subscription_id();

    auto subscription = std::make_unique<MonitoringEventSubscription>();
    subscription->id = subscription_id;
    subscription->type = "quota_violation";
    subscription->set_callback(callback);
    subscription->plugin_filter = plugin_filter;

    m_event_subscriptions[subscription_id] = std::move(subscription);

    qCDebug(monitorLog) << "Created quota violation subscription:"
                        << QString::fromStdString(subscription_id);

    return subscription_id;
}

std::string ResourceMonitor::subscribe_to_performance_alerts(
    std::function<void(const ResourcePerformanceAlert&)> callback,
    double severity_threshold) {
    std::unique_lock<std::shared_mutex> lock(m_subscriptions_mutex);

    std::string subscription_id = generate_subscription_id();

    auto subscription = std::make_unique<MonitoringEventSubscription>();
    subscription->id = subscription_id;
    subscription->type = "performance_alert";
    subscription->set_callback(callback);
    subscription->severity_threshold = severity_threshold;

    m_event_subscriptions[subscription_id] = std::move(subscription);

    qCDebug(monitorLog) << "Created performance alert subscription:"
                        << QString::fromStdString(subscription_id);

    return subscription_id;
}

qtplugin::expected<void, PluginError> ResourceMonitor::unsubscribe_from_events(
    const std::string& subscription_id) {
    std::unique_lock<std::shared_mutex> lock(m_subscriptions_mutex);

    auto it = m_event_subscriptions.find(subscription_id);
    if (it == m_event_subscriptions.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Event subscription not found: " + subscription_id);
    }

    m_event_subscriptions.erase(it);

    qCDebug(monitorLog) << "Removed event subscription:"
                        << QString::fromStdString(subscription_id);

    return make_success();
}

QJsonObject ResourceMonitor::get_monitoring_statistics() const {
    QJsonObject stats;

    stats["monitoring_enabled"] = m_monitoring_enabled.load();
    stats["total_metrics_collected"] =
        static_cast<qint64>(m_total_metrics_collected.load());
    stats["total_quota_violations"] =
        static_cast<qint64>(m_total_quota_violations.load());
    stats["total_performance_alerts"] =
        static_cast<qint64>(m_total_performance_alerts.load());

    // Resource counts
    {
        std::shared_lock<std::shared_mutex> lock(m_resources_mutex);
        stats["tracked_resources"] =
            static_cast<qint64>(m_tracked_resources.size());

        size_t active_count = 0;
        for (const auto& [resource_id, tracker] : m_tracked_resources) {
            if (tracker->is_active) {
                active_count++;
            }
        }
        stats["active_resources"] = static_cast<qint64>(active_count);
    }

    // Subscription counts
    {
        std::shared_lock<std::shared_mutex> lock(m_subscriptions_mutex);
        stats["event_subscriptions"] =
            static_cast<qint64>(m_event_subscriptions.size());
    }

    // Configuration
    {
        std::shared_lock<std::shared_mutex> lock(m_config_mutex);
        stats["collection_interval_ms"] =
            static_cast<qint64>(m_config.collection_interval.count());
        stats["retention_period_hours"] =
            static_cast<qint64>(m_config.retention_period.count() / 3600000);
    }

    return stats;
}

qtplugin::expected<std::string, PluginError> ResourceMonitor::export_metrics(
    const std::string& format, std::chrono::system_clock::time_point start_time,
    std::chrono::system_clock::time_point end_time) const {
    if (format == "json") {
        return export_to_json(start_time, end_time);
    } else if (format == "csv") {
        return export_to_csv(start_time, end_time);
    } else {
        return make_error<std::string>(PluginErrorCode::InvalidArgument,
                                       "Unsupported export format: " + format);
    }
}

size_t ResourceMonitor::clear_historical_data(
    std::chrono::system_clock::time_point before_time) {
    std::unique_lock<std::shared_mutex> lock(m_resources_mutex);

    size_t cleared = 0;

    for (auto& [resource_id, tracker] : m_tracked_resources) {
        size_t before_size = tracker->historical_data.size();
        tracker->cleanup_old_data(before_time);
        cleared += before_size - tracker->historical_data.size();
    }

    qCDebug(monitorLog) << "Cleared" << cleared << "historical data entries";

    return cleared;
}

void ResourceMonitor::set_monitoring_enabled(bool enabled) {
    m_monitoring_enabled.store(enabled);

    if (enabled) {
        m_collection_timer->start();
        m_alert_timer->start();
    } else {
        m_collection_timer->stop();
        m_alert_timer->stop();
    }

    qCDebug(monitorLog) << "Monitoring" << (enabled ? "enabled" : "disabled");
}

bool ResourceMonitor::is_monitoring_enabled() const {
    return m_monitoring_enabled.load();
}

// Slot implementations
void ResourceMonitor::collect_metrics() {
    if (!m_monitoring_enabled.load()) {
        return;
    }

    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    for (auto& [resource_id, tracker] : m_tracked_resources) {
        if (tracker->is_active) {
            // In a real implementation, you would collect actual metrics here
            // For now, we just update the last collection time
            tracker->last_collection = std::chrono::system_clock::now();
        }
    }
}

void ResourceMonitor::check_quotas_and_alerts() {
    if (!m_monitoring_enabled.load()) {
        return;
    }

    cleanup_old_violations_and_alerts();

    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    for (const auto& [resource_id, tracker] : m_tracked_resources) {
        if (tracker->is_active) {
            check_resource_quotas(resource_id, tracker->current_metrics);
            check_resource_performance(resource_id, tracker->current_metrics);
        }
    }
}

// Helper method implementations
void ResourceMonitor::notify_quota_violation(const QuotaViolation& violation) {
    {
        std::unique_lock<std::shared_mutex> lock(m_violations_mutex);
        m_quota_violations.push_back(violation);

        // Keep only recent violations
        while (m_quota_violations.size() > 1000) {
            m_quota_violations.pop_front();
        }
    }

    m_total_quota_violations.fetch_add(1);

    // Notify subscribers
    std::shared_lock<std::shared_mutex> lock(m_subscriptions_mutex);
    for (const auto& [sub_id, subscription] : m_event_subscriptions) {
        if (subscription && subscription->type == "quota_violation") {
            if (subscription->plugin_filter.empty() ||
                subscription->plugin_filter == violation.plugin_id) {
                try {
                    subscription->notify(violation);
                } catch (const std::exception& e) {
                    qCWarning(monitorLog)
                        << "Exception in quota violation callback:" << e.what();
                }
            }
        }
    }

    emit quota_violation_detected(
        QString::fromStdString(violation.plugin_id),
        static_cast<int>(violation.resource_type),
        QString::fromStdString(violation.violation_type));
}

void ResourceMonitor::notify_performance_alert(const ResourcePerformanceAlert& alert) {
    {
        std::unique_lock<std::shared_mutex> lock(m_violations_mutex);
        m_performance_alerts.push_back(alert);

        // Keep only recent alerts
        while (m_performance_alerts.size() > 1000) {
            m_performance_alerts.pop_front();
        }
    }

    m_total_performance_alerts.fetch_add(1);

    // Notify subscribers
    std::shared_lock<std::shared_mutex> lock(m_subscriptions_mutex);
    for (const auto& [sub_id, subscription] : m_event_subscriptions) {
        if (subscription && subscription->type == "performance_alert") {
            if (alert.severity >= subscription->severity_threshold) {
                try {
                    subscription->notify(alert);
                } catch (const std::exception& e) {
                    qCWarning(monitorLog)
                        << "Exception in performance alert callback:"
                        << e.what();
                }
            }
        }
    }

    emit performance_alert_triggered(QString::fromStdString(alert.resource_id),
                                     QString::fromStdString(alert.alert_type),
                                     alert.severity);
}

std::string ResourceMonitor::generate_subscription_id() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::string id;
    id.reserve(16);
    for (int i = 0; i < 16; ++i) {
        id += "0123456789abcdef"[dis(gen)];
    }
    return id;
}

void ResourceMonitor::check_resource_quotas(const std::string& resource_id,
                                            const ResourceMetrics& metrics) {
    // Simplified quota checking implementation
    Q_UNUSED(resource_id)
    Q_UNUSED(metrics)

    // In a real implementation, you would check against configured quotas
}

void ResourceMonitor::check_resource_performance(
    const std::string& resource_id, const ResourceMetrics& metrics) {
    std::shared_lock<std::shared_mutex> config_lock(m_config_mutex);

    // Check CPU usage
    if (metrics.cpu_usage_percent > m_config.cpu_usage_alert_threshold) {
        ResourcePerformanceAlert alert(
            resource_id, metrics.plugin_id, metrics.resource_type, "high_cpu",
            metrics.cpu_usage_percent / 100.0, "High CPU usage detected");
        config_lock.unlock();
        notify_performance_alert(alert);
        return;
    }

    // Check memory usage
    if (metrics.memory_usage_bytes > m_config.memory_usage_alert_threshold) {
        ResourcePerformanceAlert alert(resource_id, metrics.plugin_id,
                               metrics.resource_type, "high_memory",
                               static_cast<double>(metrics.memory_usage_bytes) /
                                   m_config.memory_usage_alert_threshold,
                               "High memory usage detected");
        config_lock.unlock();
        notify_performance_alert(alert);
        return;
    }

    // Check error rate
    if (metrics.access_count > 0) {
        double error_rate =
            static_cast<double>(metrics.error_count) / metrics.access_count;
        if (error_rate > m_config.error_rate_alert_threshold) {
            ResourcePerformanceAlert alert(resource_id, metrics.plugin_id,
                                   metrics.resource_type, "high_errors",
                                   error_rate, "High error rate detected");
            config_lock.unlock();
            notify_performance_alert(alert);
            return;
        }
    }

    // Check efficiency
    double efficiency = metrics.calculate_efficiency_score();
    if (efficiency < m_config.efficiency_alert_threshold) {
        ResourcePerformanceAlert alert(resource_id, metrics.plugin_id,
                               metrics.resource_type, "low_efficiency",
                               1.0 - efficiency,
                               "Low resource efficiency detected");
        config_lock.unlock();
        notify_performance_alert(alert);
        return;
    }
}

void ResourceMonitor::cleanup_old_violations_and_alerts() {
    std::unique_lock<std::shared_mutex> lock(m_violations_mutex);

    auto cutoff_time =
        std::chrono::system_clock::now() - std::chrono::hours(24);

    // Clean up old quota violations
    m_quota_violations.erase(
        std::remove_if(m_quota_violations.begin(), m_quota_violations.end(),
                       [cutoff_time](const QuotaViolation& violation) {
                           return violation.timestamp < cutoff_time;
                       }),
        m_quota_violations.end());

    // Clean up old performance alerts
    m_performance_alerts.erase(
        std::remove_if(m_performance_alerts.begin(), m_performance_alerts.end(),
                       [cutoff_time](const ResourcePerformanceAlert& alert) {
                           return alert.timestamp < cutoff_time;
                       }),
        m_performance_alerts.end());
}

std::string ResourceMonitor::export_to_json(
    std::chrono::system_clock::time_point start_time,
    std::chrono::system_clock::time_point end_time) const {
    QJsonObject export_data;
    QJsonArray resources_array;

    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    for (const auto& [resource_id, tracker] : m_tracked_resources) {
        QJsonObject resource_data;
        resource_data["resource_id"] = QString::fromStdString(resource_id);
        resource_data["plugin_id"] =
            QString::fromStdString(tracker->current_metrics.plugin_id);
        resource_data["resource_type"] = QString::fromStdString(
            resource_type_to_string(tracker->current_metrics.resource_type));

        QJsonArray historical_array;
        for (const auto& entry : tracker->historical_data) {
            if (entry.timestamp >= start_time && entry.timestamp <= end_time) {
                QJsonObject metrics_data;
                metrics_data["timestamp"] = QString::number(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        entry.timestamp.time_since_epoch())
                        .count());
                metrics_data["cpu_usage_percent"] =
                    entry.metrics.cpu_usage_percent;
                metrics_data["memory_usage_bytes"] =
                    static_cast<qint64>(entry.metrics.memory_usage_bytes);
                metrics_data["access_count"] =
                    static_cast<qint64>(entry.metrics.access_count);
                metrics_data["error_count"] =
                    static_cast<qint64>(entry.metrics.error_count);

                historical_array.append(metrics_data);
            }
        }

        resource_data["historical_data"] = historical_array;
        resources_array.append(resource_data);
    }

    export_data["resources"] = resources_array;
    export_data["export_start_time"] =
        QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                            start_time.time_since_epoch())
                            .count());
    export_data["export_end_time"] =
        QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                            end_time.time_since_epoch())
                            .count());

    return QJsonDocument(export_data).toJson().toStdString();
}

std::string ResourceMonitor::export_to_csv(
    std::chrono::system_clock::time_point start_time,
    std::chrono::system_clock::time_point end_time) const {
    std::ostringstream csv;

    // CSV header
    csv << "timestamp,resource_id,plugin_id,resource_type,cpu_usage_percent,"
           "memory_usage_bytes,access_count,error_count\n";

    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    for (const auto& [resource_id, tracker] : m_tracked_resources) {
        for (const auto& entry : tracker->historical_data) {
            if (entry.timestamp >= start_time && entry.timestamp <= end_time) {
                auto timestamp_ms =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        entry.timestamp.time_since_epoch())
                        .count();

                csv << timestamp_ms << "," << resource_id << ","
                    << entry.metrics.plugin_id << ","
                    << resource_type_to_string(entry.metrics.resource_type)
                    << "," << entry.metrics.cpu_usage_percent << ","
                    << entry.metrics.memory_usage_bytes << ","
                    << entry.metrics.access_count << ","
                    << entry.metrics.error_count << "\n";
            }
        }
    }

    return csv.str();
}

// Factory function
std::unique_ptr<IResourceMonitor> create_resource_monitor(QObject* parent) {
    return std::make_unique<ResourceMonitor>(parent);
}

}  // namespace qtplugin
