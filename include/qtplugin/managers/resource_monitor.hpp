/**
 * @file resource_monitor.hpp
 * @brief Resource usage monitoring, quotas, and performance metrics
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>
#include "resource_manager.hpp"

namespace qtplugin {

/**
 * @brief Resource usage metrics
 */
struct ResourceMetrics {
    std::string resource_id;
    ResourceType resource_type;
    std::string plugin_id;

    // Usage statistics
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_accessed;
    std::chrono::milliseconds total_usage_time{0};
    std::chrono::milliseconds active_time{0};
    size_t access_count = 0;

    // Performance metrics
    double cpu_usage_percent = 0.0;
    size_t memory_usage_bytes = 0;
    size_t peak_memory_usage_bytes = 0;
    double io_operations_per_second = 0.0;
    double network_throughput_mbps = 0.0;

    // Error tracking
    size_t error_count = 0;
    std::chrono::system_clock::time_point last_error;
    std::string last_error_message;

    // Custom metrics
    QJsonObject custom_metrics;

    ResourceMetrics() = default;
    explicit ResourceMetrics(const ResourceHandle& handle)
        : resource_id(handle.id()),
          resource_type(handle.type()),
          plugin_id(handle.plugin_id()),
          created_at(std::chrono::system_clock::now()),
          last_accessed(std::chrono::system_clock::now()) {}

    /**
     * @brief Calculate resource efficiency score (0.0 to 1.0)
     * @return Efficiency score based on usage patterns
     */
    double calculate_efficiency_score() const {
        if (access_count == 0)
            return 0.0;

        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - created_at);

        if (age.count() == 0)
            return 1.0;

        double usage_ratio =
            static_cast<double>(total_usage_time.count()) / age.count();
        double access_frequency = static_cast<double>(access_count) /
                                  (age.count() / 1000.0);  // per second
        double error_rate =
            error_count > 0 ? static_cast<double>(error_count) / access_count
                            : 0.0;

        // Combine factors (usage ratio 50%, access frequency 30%, low error
        // rate 20%)
        return (usage_ratio * 0.5) +
               (std::min(access_frequency / 10.0, 1.0) * 0.3) +
               ((1.0 - error_rate) * 0.2);
    }

    /**
     * @brief Check if resource is underutilized
     * @return true if resource appears to be underutilized
     */
    bool is_underutilized() const { return calculate_efficiency_score() < 0.3; }

    /**
     * @brief Check if resource is overutilized
     * @return true if resource appears to be overutilized
     */
    bool is_overutilized() const {
        return cpu_usage_percent > 80.0 ||
               memory_usage_bytes > peak_memory_usage_bytes * 0.9;
    }
};

/**
 * @brief Resource quota violation
 */
struct QuotaViolation {
    std::string plugin_id;
    ResourceType resource_type;
    std::string violation_type;  // "count", "memory", "cpu", "custom"
    double current_value;
    double limit_value;
    std::chrono::system_clock::time_point timestamp;
    std::string description;

    QuotaViolation() = default;
    QuotaViolation(std::string_view plugin, ResourceType type,
                   std::string_view violation, double current, double limit,
                   std::string_view desc = {})
        : plugin_id(plugin),
          resource_type(type),
          violation_type(violation),
          current_value(current),
          limit_value(limit),
          timestamp(std::chrono::system_clock::now()),
          description(desc) {}
};

/**
 * @brief Resource performance alert
 */
struct ResourcePerformanceAlert {
    std::string resource_id;
    std::string plugin_id;
    ResourceType resource_type;
    std::string alert_type;  // "high_cpu", "high_memory", "high_errors",
                             // "low_efficiency"
    double severity;         // 0.0 to 1.0
    std::chrono::system_clock::time_point timestamp;
    std::string message;
    QJsonObject details;

    ResourcePerformanceAlert() = default;
    ResourcePerformanceAlert(std::string_view res_id, std::string_view plugin,
                     ResourceType type, std::string_view alert, double sev,
                     std::string_view msg)
        : resource_id(res_id),
          plugin_id(plugin),
          resource_type(type),
          alert_type(alert),
          severity(sev),
          timestamp(std::chrono::system_clock::now()),
          message(msg) {}
};

/**
 * @brief Monitoring configuration
 */
struct MonitoringConfiguration {
    std::chrono::milliseconds collection_interval{1000};  // 1 second
    std::chrono::milliseconds retention_period{
        std::chrono::hours(24)};  // 24 hours
    size_t max_metrics_per_resource = 1000;

    // Alert thresholds
    double cpu_usage_alert_threshold = 80.0;                  // percent
    size_t memory_usage_alert_threshold = 100 * 1024 * 1024;  // 100MB
    double error_rate_alert_threshold = 0.1;                  // 10%
    double efficiency_alert_threshold = 0.2;                  // 20%

    // Quota enforcement
    bool enforce_quotas = true;
    bool auto_cleanup_violations = true;

    // Custom metric collection
    bool collect_custom_metrics = true;
    std::vector<std::string> enabled_metric_types;
};

/**
 * @brief Resource monitor interface
 */
class IResourceMonitor {
public:
    virtual ~IResourceMonitor() = default;

    // === Metrics Collection ===

    /**
     * @brief Start monitoring a resource
     * @param handle Resource handle
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> start_monitoring(
        const ResourceHandle& handle) = 0;

    /**
     * @brief Stop monitoring a resource
     * @param resource_id Resource ID
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> stop_monitoring(
        const std::string& resource_id) = 0;

    /**
     * @brief Update resource metrics
     * @param resource_id Resource ID
     * @param metrics Updated metrics
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> update_metrics(
        const std::string& resource_id, const ResourceMetrics& metrics) = 0;

    /**
     * @brief Record resource access
     * @param resource_id Resource ID
     * @param access_duration Duration of access
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> record_access(
        const std::string& resource_id,
        std::chrono::milliseconds access_duration) = 0;

    /**
     * @brief Record resource error
     * @param resource_id Resource ID
     * @param error_message Error message
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> record_error(
        const std::string& resource_id, const std::string& error_message) = 0;

    // === Metrics Retrieval ===

    /**
     * @brief Get current metrics for a resource
     * @param resource_id Resource ID
     * @return Current metrics or error
     */
    virtual qtplugin::expected<ResourceMetrics, PluginError> get_metrics(
        const std::string& resource_id) const = 0;

    /**
     * @brief Get metrics for all resources of a plugin
     * @param plugin_id Plugin ID
     * @return List of metrics or error
     */
    virtual qtplugin::expected<std::vector<ResourceMetrics>, PluginError>
    get_plugin_metrics(const std::string& plugin_id) const = 0;

    /**
     * @brief Get aggregated metrics by resource type
     * @param resource_type Resource type
     * @return Aggregated metrics or error
     */
    virtual qtplugin::expected<ResourceMetrics, PluginError>
    get_aggregated_metrics(ResourceType resource_type) const = 0;

    /**
     * @brief Get historical metrics for a resource
     * @param resource_id Resource ID
     * @param start_time Start time for history
     * @param end_time End time for history
     * @return Historical metrics or error
     */
    virtual qtplugin::expected<std::vector<ResourceMetrics>, PluginError>
    get_historical_metrics(
        const std::string& resource_id,
        std::chrono::system_clock::time_point start_time,
        std::chrono::system_clock::time_point end_time) const = 0;

    // === Quota Management ===

    /**
     * @brief Check quota compliance for a plugin
     * @param plugin_id Plugin ID
     * @param resource_type Resource type
     * @return List of quota violations
     */
    virtual std::vector<QuotaViolation> check_quota_compliance(
        const std::string& plugin_id, ResourceType resource_type) const = 0;

    /**
     * @brief Get all quota violations
     * @param since_time Only violations since this time (optional)
     * @return List of quota violations
     */
    virtual std::vector<QuotaViolation> get_quota_violations(
        std::optional<std::chrono::system_clock::time_point> since_time =
            std::nullopt) const = 0;

    /**
     * @brief Set custom quota for plugin
     * @param plugin_id Plugin ID
     * @param resource_type Resource type
     * @param quota_name Quota name (e.g., "cpu_percent", "memory_mb")
     * @param limit Quota limit
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> set_custom_quota(
        const std::string& plugin_id, ResourceType resource_type,
        const std::string& quota_name, double limit) = 0;

    // === Performance Monitoring ===

    /**
     * @brief Get performance alerts
     * @param severity_threshold Minimum severity (0.0 to 1.0)
     * @param since_time Only alerts since this time (optional)
     * @return List of performance alerts
     */
    virtual std::vector<ResourcePerformanceAlert> get_performance_alerts(
        double severity_threshold = 0.5,
        std::optional<std::chrono::system_clock::time_point> since_time =
            std::nullopt) const = 0;

    /**
     * @brief Get resource efficiency report
     * @param plugin_id Plugin ID filter (optional)
     * @param resource_type Resource type filter (optional)
     * @return Efficiency report as JSON
     */
    virtual QJsonObject get_efficiency_report(
        const std::string& plugin_id = {},
        std::optional<ResourceType> resource_type = std::nullopt) const = 0;

    /**
     * @brief Get top resource consumers
     * @param metric_type Metric type ("cpu", "memory", "access_count",
     * "errors")
     * @param count Number of top consumers to return
     * @return List of top consuming resources
     */
    virtual std::vector<std::pair<std::string, double>> get_top_consumers(
        const std::string& metric_type, size_t count = 10) const = 0;

    // === Configuration ===

    /**
     * @brief Set monitoring configuration
     * @param config Monitoring configuration
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> set_configuration(
        const MonitoringConfiguration& config) = 0;

    /**
     * @brief Get current monitoring configuration
     * @return Current configuration
     */
    virtual MonitoringConfiguration get_configuration() const = 0;

    // === Event Subscriptions ===

    /**
     * @brief Subscribe to quota violation events
     * @param callback Event callback function
     * @param plugin_filter Plugin ID filter (optional)
     * @return Subscription ID for unsubscribing
     */
    virtual std::string subscribe_to_quota_violations(
        std::function<void(const QuotaViolation&)> callback,
        const std::string& plugin_filter = {}) = 0;

    /**
     * @brief Subscribe to performance alert events
     * @param callback Event callback function
     * @param severity_threshold Minimum severity threshold
     * @return Subscription ID for unsubscribing
     */
    virtual std::string subscribe_to_performance_alerts(
        std::function<void(const ResourcePerformanceAlert&)> callback,
        double severity_threshold = 0.5) = 0;

    /**
     * @brief Unsubscribe from events
     * @param subscription_id Subscription ID
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> unsubscribe_from_events(
        const std::string& subscription_id) = 0;

    // === Utility ===

    /**
     * @brief Get monitoring statistics
     * @return Statistics as JSON object
     */
    virtual QJsonObject get_monitoring_statistics() const = 0;

    /**
     * @brief Export metrics data
     * @param format Export format ("json", "csv")
     * @param start_time Start time for export
     * @param end_time End time for export
     * @return Exported data as string or error
     */
    virtual qtplugin::expected<std::string, PluginError> export_metrics(
        const std::string& format,
        std::chrono::system_clock::time_point start_time,
        std::chrono::system_clock::time_point end_time) const = 0;

    /**
     * @brief Clear historical data
     * @param before_time Clear data before this time
     * @return Number of records cleared
     */
    virtual size_t clear_historical_data(
        std::chrono::system_clock::time_point before_time) = 0;

    /**
     * @brief Enable or disable monitoring
     * @param enabled Whether to enable monitoring
     */
    virtual void set_monitoring_enabled(bool enabled) = 0;

    /**
     * @brief Check if monitoring is enabled
     * @return true if monitoring is enabled
     */
    virtual bool is_monitoring_enabled() const = 0;
};

}  // namespace qtplugin
