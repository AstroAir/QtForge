/**
 * @file resource_monitor_impl.hpp
 * @brief Concrete implementation of resource monitoring system
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include "resource_monitor.hpp"

namespace qtplugin {

/**
 * @brief Historical metrics entry
 */
struct HistoricalMetricsEntry {
    std::chrono::system_clock::time_point timestamp;
    ResourceMetrics metrics;

    HistoricalMetricsEntry() = default;
    HistoricalMetricsEntry(const ResourceMetrics& m)
        : timestamp(std::chrono::system_clock::now()), metrics(m) {}
};

/**
 * @brief Resource monitoring tracker
 */
struct ResourceMonitoringTracker {
    ResourceMetrics current_metrics;
    std::deque<HistoricalMetricsEntry> historical_data;
    std::chrono::system_clock::time_point last_collection;
    bool is_active = true;

    ResourceMonitoringTracker() = default;
    explicit ResourceMonitoringTracker(const ResourceHandle& handle)
        : current_metrics(handle),
          last_collection(std::chrono::system_clock::now()) {}

    void add_historical_entry(const ResourceMetrics& metrics,
                              size_t max_entries) {
        historical_data.emplace_back(metrics);

        // Keep only recent entries
        while (historical_data.size() > max_entries) {
            historical_data.pop_front();
        }
    }

    void cleanup_old_data(std::chrono::system_clock::time_point before_time) {
        historical_data.erase(
            std::remove_if(historical_data.begin(), historical_data.end(),
                           [before_time](const HistoricalMetricsEntry& entry) {
                               return entry.timestamp < before_time;
                           }),
            historical_data.end());
    }
};

/**
 * @brief Event subscription for monitoring
 */
struct MonitoringEventSubscription {
    std::string id;
    std::string type;  // "quota_violation", "performance_alert"
    std::function<void(const void*)> callback;  // Generic callback
    std::string plugin_filter;
    double severity_threshold = 0.0;

    template <typename T>
    void set_callback(std::function<void(const T&)> cb) {
        callback = [cb](const void* data) { cb(*static_cast<const T*>(data)); };
    }

    template <typename T>
    void notify(const T& event) const {
        if (callback) {
            callback(&event);
        }
    }
};

/**
 * @brief Default resource monitor implementation
 */
class ResourceMonitor : public QObject, public IResourceMonitor {
    Q_OBJECT

public:
    explicit ResourceMonitor(QObject* parent = nullptr);
    ~ResourceMonitor() override;

    // IResourceMonitor implementation
    qtplugin::expected<void, PluginError> start_monitoring(
        const ResourceHandle& handle) override;

    qtplugin::expected<void, PluginError> stop_monitoring(
        const std::string& resource_id) override;

    qtplugin::expected<void, PluginError> update_metrics(
        const std::string& resource_id,
        const ResourceMetrics& metrics) override;

    qtplugin::expected<void, PluginError> record_access(
        const std::string& resource_id,
        std::chrono::milliseconds access_duration) override;

    qtplugin::expected<void, PluginError> record_error(
        const std::string& resource_id,
        const std::string& error_message) override;

    qtplugin::expected<ResourceMetrics, PluginError> get_metrics(
        const std::string& resource_id) const override;

    qtplugin::expected<std::vector<ResourceMetrics>, PluginError>
    get_plugin_metrics(const std::string& plugin_id) const override;

    qtplugin::expected<ResourceMetrics, PluginError> get_aggregated_metrics(
        ResourceType resource_type) const override;

    qtplugin::expected<std::vector<ResourceMetrics>, PluginError>
    get_historical_metrics(
        const std::string& resource_id,
        std::chrono::system_clock::time_point start_time,
        std::chrono::system_clock::time_point end_time) const override;

    std::vector<QuotaViolation> check_quota_compliance(
        const std::string& plugin_id,
        ResourceType resource_type) const override;

    std::vector<QuotaViolation> get_quota_violations(
        std::optional<std::chrono::system_clock::time_point> since_time =
            std::nullopt) const override;

    qtplugin::expected<void, PluginError> set_custom_quota(
        const std::string& plugin_id, ResourceType resource_type,
        const std::string& quota_name, double limit) override;

    std::vector<PerformanceAlert> get_performance_alerts(
        double severity_threshold = 0.5,
        std::optional<std::chrono::system_clock::time_point> since_time =
            std::nullopt) const override;

    QJsonObject get_efficiency_report(
        const std::string& plugin_id = {},
        std::optional<ResourceType> resource_type =
            std::nullopt) const override;

    std::vector<std::pair<std::string, double>> get_top_consumers(
        const std::string& metric_type, size_t count = 10) const override;

    qtplugin::expected<void, PluginError> set_configuration(
        const MonitoringConfiguration& config) override;

    MonitoringConfiguration get_configuration() const override;

    std::string subscribe_to_quota_violations(
        std::function<void(const QuotaViolation&)> callback,
        const std::string& plugin_filter = {}) override;

    std::string subscribe_to_performance_alerts(
        std::function<void(const PerformanceAlert&)> callback,
        double severity_threshold = 0.5) override;

    qtplugin::expected<void, PluginError> unsubscribe_from_events(
        const std::string& subscription_id) override;

    QJsonObject get_monitoring_statistics() const override;

    qtplugin::expected<std::string, PluginError> export_metrics(
        const std::string& format,
        std::chrono::system_clock::time_point start_time,
        std::chrono::system_clock::time_point end_time) const override;

    size_t clear_historical_data(
        std::chrono::system_clock::time_point before_time) override;
    void set_monitoring_enabled(bool enabled) override;
    bool is_monitoring_enabled() const override;

signals:
    void quota_violation_detected(const QString& plugin_id, int resource_type,
                                  const QString& violation_type);
    void performance_alert_triggered(const QString& resource_id,
                                     const QString& alert_type,
                                     double severity);
    void metrics_updated(const QString& resource_id);
    void monitoring_configuration_changed();

private slots:
    void collect_metrics();
    void check_quotas_and_alerts();

private:
    // Resource tracking
    std::unordered_map<std::string, std::unique_ptr<ResourceMonitoringTracker>>
        m_tracked_resources;
    mutable std::shared_mutex m_resources_mutex;

    // Configuration
    MonitoringConfiguration m_config;
    mutable std::shared_mutex m_config_mutex;

    // Quota violations and alerts
    std::deque<QuotaViolation> m_quota_violations;
    std::deque<PerformanceAlert> m_performance_alerts;
    mutable std::shared_mutex m_violations_mutex;

    // Custom quotas
    std::unordered_map<
        std::string, std::unordered_map<
                         ResourceType, std::unordered_map<std::string, double>>>
        m_custom_quotas;
    mutable std::shared_mutex m_quotas_mutex;

    // Event subscriptions
    std::unordered_map<std::string,
                       std::unique_ptr<MonitoringEventSubscription>>
        m_event_subscriptions;
    mutable std::shared_mutex m_subscriptions_mutex;

    // Timers
    std::unique_ptr<QTimer> m_collection_timer;
    std::unique_ptr<QTimer> m_alert_timer;

    // Settings
    std::atomic<bool> m_monitoring_enabled{true};

    // Statistics
    std::atomic<size_t> m_total_metrics_collected{0};
    std::atomic<size_t> m_total_quota_violations{0};
    std::atomic<size_t> m_total_performance_alerts{0};

    // Helper methods
    void notify_quota_violation(const QuotaViolation& violation);
    void notify_performance_alert(const PerformanceAlert& alert);
    std::string generate_subscription_id() const;
    void check_resource_quotas(const std::string& resource_id,
                               const ResourceMetrics& metrics);
    void check_resource_performance(const std::string& resource_id,
                                    const ResourceMetrics& metrics);
    void cleanup_old_violations_and_alerts();
    std::string export_to_json(
        std::chrono::system_clock::time_point start_time,
        std::chrono::system_clock::time_point end_time) const;
    std::string export_to_csv(
        std::chrono::system_clock::time_point start_time,
        std::chrono::system_clock::time_point end_time) const;
};

/**
 * @brief Create a default resource monitor instance
 * @param parent Parent QObject
 * @return Unique pointer to resource monitor
 */
std::unique_ptr<IResourceMonitor> create_resource_monitor(
    QObject* parent = nullptr);

}  // namespace qtplugin
