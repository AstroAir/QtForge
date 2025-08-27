/**
 * @file resource_monitor.hpp
 * @brief Enhanced resource monitor interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "../../utils/error_handling.hpp"
#include "../resource_manager.hpp"

namespace qtplugin {

// Forward declarations
enum class ResourceType;
struct ResourceUsageStats;
struct ResourceQuota;
class IComponentResourcePool;
class IResourceAllocator;

/**
 * @brief Resource monitoring configuration
 */
struct MonitoringConfig {
    std::chrono::milliseconds monitoring_interval{1000};
    bool enable_usage_tracking = true;
    bool enable_performance_tracking = true;
    bool enable_leak_detection = true;
    bool enable_quota_monitoring = true;
    size_t max_history_entries = 1000;
    std::chrono::minutes history_retention{60};
};

/**
 * @brief Resource usage snapshot
 */
struct ResourceSnapshot {
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<ResourceType, ResourceUsageStats> usage_by_type;
    std::unordered_map<std::string, ResourceUsageStats> usage_by_plugin;
    size_t total_memory_usage = 0;
    double cpu_usage_percent = 0.0;
    size_t active_allocations = 0;
    size_t failed_allocations = 0;
};

/**
 * @brief Resource alert configuration
 */
struct ResourceAlert {
    std::string name;
    ResourceType resource_type;
    std::string condition;  // e.g., "memory_usage > 80%"
    std::function<void(const ResourceSnapshot&)> callback;
    bool enabled = true;
    std::chrono::milliseconds cooldown{30000};
    std::chrono::system_clock::time_point last_triggered;
};

/**
 * @brief Interface for enhanced resource monitoring
 *
 * The resource monitor tracks resource usage, performance metrics,
 * and provides alerting and leak detection capabilities.
 */
class IResourceMonitor {
public:
    virtual ~IResourceMonitor() = default;

    /**
     * @brief Start monitoring
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> start_monitoring() = 0;

    /**
     * @brief Stop monitoring
     */
    virtual void stop_monitoring() = 0;

    /**
     * @brief Check if monitoring is active
     * @return true if monitoring is active
     */
    virtual bool is_monitoring() const = 0;

    /**
     * @brief Set monitoring configuration
     * @param config Monitoring configuration
     */
    virtual void set_monitoring_config(const MonitoringConfig& config) = 0;

    /**
     * @brief Get current monitoring configuration
     * @return Current configuration
     */
    virtual MonitoringConfig get_monitoring_config() const = 0;

    /**
     * @brief Get current resource snapshot
     * @return Current resource usage snapshot
     */
    virtual ResourceSnapshot get_current_snapshot() const = 0;

    /**
     * @brief Get resource usage history
     * @param duration Duration to look back
     * @return List of historical snapshots
     */
    virtual std::vector<ResourceSnapshot> get_usage_history(
        std::chrono::minutes duration) const = 0;

    /**
     * @brief Add resource alert
     * @param alert Alert configuration
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> add_alert(
        const ResourceAlert& alert) = 0;

    /**
     * @brief Remove resource alert
     * @param alert_name Name of alert to remove
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> remove_alert(
        const std::string& alert_name) = 0;

    /**
     * @brief Get active alerts
     * @return List of active alerts
     */
    virtual std::vector<ResourceAlert> get_active_alerts() const = 0;

    /**
     * @brief Detect resource leaks
     * @param plugin_id Optional plugin ID filter
     * @return List of potential leak descriptions
     */
    virtual std::vector<std::string> detect_resource_leaks(
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Get performance metrics
     * @param resource_type Optional resource type filter
     * @param plugin_id Optional plugin ID filter
     * @return Performance metrics
     */
    virtual std::unordered_map<std::string, double> get_performance_metrics(
        std::optional<ResourceType> resource_type = std::nullopt,
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Register resource pool for monitoring
     * @param pool Resource pool to monitor
     */
    virtual void register_pool(
        std::shared_ptr<IComponentResourcePool> pool) = 0;

    /**
     * @brief Register resource allocator for monitoring
     * @param allocator Resource allocator to monitor
     */
    virtual void register_allocator(
        std::shared_ptr<IResourceAllocator> allocator) = 0;
};

/**
 * @brief Enhanced resource monitor implementation
 *
 * Provides comprehensive resource monitoring with usage tracking,
 * performance metrics, leak detection, and alerting capabilities.
 */
class ResourceMonitor : public QObject, public IResourceMonitor {
    Q_OBJECT

public:
    explicit ResourceMonitor(QObject* parent = nullptr);
    ~ResourceMonitor() override;

    // IResourceMonitor interface
    qtplugin::expected<void, PluginError> start_monitoring() override;
    void stop_monitoring() override;
    bool is_monitoring() const override;

    void set_monitoring_config(const MonitoringConfig& config) override;
    MonitoringConfig get_monitoring_config() const override;

    ResourceSnapshot get_current_snapshot() const override;
    std::vector<ResourceSnapshot> get_usage_history(
        std::chrono::minutes duration) const override;

    qtplugin::expected<void, PluginError> add_alert(
        const ResourceAlert& alert) override;
    qtplugin::expected<void, PluginError> remove_alert(
        const std::string& alert_name) override;
    std::vector<ResourceAlert> get_active_alerts() const override;

    std::vector<std::string> detect_resource_leaks(
        std::string_view plugin_id = {}) const override;

    std::unordered_map<std::string, double> get_performance_metrics(
        std::optional<ResourceType> resource_type = std::nullopt,
        std::string_view plugin_id = {}) const override;

    void register_pool(std::shared_ptr<IComponentResourcePool> pool) override;
    void register_allocator(
        std::shared_ptr<IResourceAllocator> allocator) override;

signals:
    /**
     * @brief Emitted when monitoring starts
     */
    void monitoring_started();

    /**
     * @brief Emitted when monitoring stops
     */
    void monitoring_stopped();

    /**
     * @brief Emitted when resource usage is updated
     * @param snapshot Current usage snapshot
     */
    void usage_updated(const ResourceSnapshot& snapshot);

    /**
     * @brief Emitted when resource alert is triggered
     * @param alert_name Name of triggered alert
     * @param snapshot Current snapshot that triggered alert
     */
    void alert_triggered(const QString& alert_name,
                         const ResourceSnapshot& snapshot);

    /**
     * @brief Emitted when resource leak is detected
     * @param plugin_id Plugin with potential leak
     * @param description Leak description
     */
    void leak_detected(const QString& plugin_id, const QString& description);

private slots:
    void on_monitoring_timer();

private:
    MonitoringConfig m_config;
    std::unique_ptr<QTimer> m_monitoring_timer;
    std::atomic<bool> m_is_monitoring{false};

    mutable std::shared_mutex m_mutex;
    std::vector<std::shared_ptr<IComponentResourcePool>> m_monitored_pools;
    std::vector<std::shared_ptr<IResourceAllocator>> m_monitored_allocators;
    std::vector<ResourceSnapshot> m_usage_history;
    std::unordered_map<std::string, ResourceAlert> m_alerts;

    // Performance tracking
    std::unordered_map<std::string, std::chrono::system_clock::time_point>
        m_allocation_times;
    std::unordered_map<std::string, std::vector<double>> m_performance_history;

    // Helper methods
    ResourceSnapshot create_snapshot() const;
    void cleanup_old_history();
    void check_alerts(const ResourceSnapshot& snapshot);
    bool evaluate_alert_condition(const ResourceAlert& alert,
                                  const ResourceSnapshot& snapshot) const;
    std::vector<std::string> detect_leaks_for_plugin(
        std::string_view plugin_id) const;
    double calculate_cpu_usage() const;
    size_t calculate_total_memory_usage() const;
    void update_performance_metrics(const ResourceSnapshot& snapshot);
};

}  // namespace qtplugin
