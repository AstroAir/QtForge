/**
 * @file plugin_metrics_collector.hpp
 * @brief Plugin metrics collector interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <memory>
#include <qtplugin/utils/error_handling.hpp>
#include <string>

namespace qtplugin {

// Forward declarations
class IPluginRegistry;

/**
 * @brief Interface for plugin metrics collection
 *
 * The metrics collector handles performance monitoring, metrics aggregation,
 * and system-wide statistics collection for plugins.
 */
class IPluginMetricsCollector {
public:
    virtual ~IPluginMetricsCollector() = default;

    /**
     * @brief Start monitoring with specified interval
     * @param interval Monitoring interval in milliseconds
     */
    virtual void start_monitoring(std::chrono::milliseconds interval) = 0;

    /**
     * @brief Stop monitoring
     */
    virtual void stop_monitoring() = 0;

    /**
     * @brief Check if monitoring is active
     * @return true if monitoring is active
     */
    virtual bool is_monitoring_active() const = 0;

    /**
     * @brief Update metrics for a specific plugin
     * @param plugin_id Plugin identifier
     * @param plugin_registry Plugin registry to read from
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> update_plugin_metrics(
        const std::string& plugin_id, IPluginRegistry* plugin_registry) = 0;

    /**
     * @brief Get metrics for a specific plugin
     * @param plugin_id Plugin identifier
     * @param plugin_registry Plugin registry to read from
     * @return Plugin metrics or empty object if not found
     */
    virtual QJsonObject get_plugin_metrics(
        const std::string& plugin_id,
        IPluginRegistry* plugin_registry) const = 0;

    /**
     * @brief Get system-wide metrics
     * @param plugin_registry Plugin registry to read from
     * @return System metrics
     */
    virtual QJsonObject get_system_metrics(
        IPluginRegistry* plugin_registry) const = 0;

    /**
     * @brief Update all plugin metrics
     * @param plugin_registry Plugin registry to read from
     */
    virtual void update_all_metrics(IPluginRegistry* plugin_registry) = 0;

    /**
     * @brief Clear all collected metrics
     */
    virtual void clear_metrics() = 0;

    /**
     * @brief Set monitoring interval
     * @param interval New monitoring interval
     */
    virtual void set_monitoring_interval(
        std::chrono::milliseconds interval) = 0;

    /**
     * @brief Get current monitoring interval
     * @return Current monitoring interval
     */
    virtual std::chrono::milliseconds get_monitoring_interval() const = 0;
};

/**
 * @brief Plugin metrics collector implementation
 *
 * Collects and aggregates performance metrics for plugins and the system.
 * Provides periodic monitoring and real-time metrics updates.
 */
class PluginMetricsCollector : public QObject, public IPluginMetricsCollector {
    Q_OBJECT

public:
    explicit PluginMetricsCollector(QObject* parent = nullptr);
    ~PluginMetricsCollector() override;

    // IPluginMetricsCollector interface
    void start_monitoring(std::chrono::milliseconds interval) override;
    void stop_monitoring() override;
    bool is_monitoring_active() const override;

    qtplugin::expected<void, PluginError> update_plugin_metrics(
        const std::string& plugin_id,
        IPluginRegistry* plugin_registry) override;

    QJsonObject get_plugin_metrics(
        const std::string& plugin_id,
        IPluginRegistry* plugin_registry) const override;
    QJsonObject get_system_metrics(
        IPluginRegistry* plugin_registry) const override;
    void update_all_metrics(IPluginRegistry* plugin_registry) override;
    void clear_metrics() override;
    void set_monitoring_interval(std::chrono::milliseconds interval) override;
    std::chrono::milliseconds get_monitoring_interval() const override;

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
     * @brief Emitted when plugin metrics are updated
     * @param plugin_id Plugin identifier
     */
    void plugin_metrics_updated(const QString& plugin_id);

    /**
     * @brief Emitted when system metrics are updated
     */
    void system_metrics_updated();

private slots:
    void on_monitoring_timer();

private:
    std::unique_ptr<QTimer> m_monitoring_timer;
    std::atomic<bool> m_monitoring_active{false};
    std::chrono::milliseconds m_monitoring_interval{1000};
    IPluginRegistry* m_plugin_registry = nullptr;

    // Helper methods
    std::string plugin_state_to_string(int state) const;
    QJsonObject calculate_plugin_metrics(
        const std::string& plugin_id, IPluginRegistry* plugin_registry) const;
};

}  // namespace qtplugin
