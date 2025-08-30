/**
 * @file monitoring_plugin.hpp
 * @brief Monitoring plugin demonstrating QtForge monitoring and hot reload features
 * @version 3.0.0
 *
 * This plugin demonstrates comprehensive monitoring functionality including:
 * - Hot reload management and file system monitoring
 * - Performance metrics collection and analysis
 * - Resource usage tracking and optimization
 * - Real-time monitoring dashboards
 * - Alert and notification systems
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QJsonObject>
#include <QMutex>
#include <QReadWriteLock>
#include <atomic>
#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>

#include "qtplugin/core/plugin_interface.hpp"
#include "qtplugin/monitoring/plugin_hot_reload_manager.hpp"
#include "qtplugin/monitoring/plugin_metrics_collector.hpp"
#include "qtplugin/communication/message_bus.hpp"
#include "qtplugin/utils/error_handling.hpp"

/**
 * @brief Monitoring plugin demonstrating QtForge monitoring features
 *
 * This plugin showcases:
 * - Hot reload functionality with file system monitoring
 * - Performance metrics collection and analysis
 * - Resource usage tracking and reporting
 * - Real-time monitoring and alerting
 * - Dashboard and visualization support
 */
class MonitoringPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.MonitoringPlugin" FILE "monitoring_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit MonitoringPlugin(QObject* parent = nullptr);
    ~MonitoringPlugin() override;

    // === IPlugin Interface ===
    // Basic plugin information
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    qtplugin::Version version() const noexcept override;
    std::string_view author() const noexcept override;
    std::string id() const noexcept override;
    qtplugin::PluginState state() const noexcept override;

    // Lifecycle methods
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    bool is_initialized() const noexcept override;

    qtplugin::PluginMetadata metadata() const override;
    qtplugin::PluginCapabilities capabilities() const noexcept override;
    qtplugin::PluginPriority priority() const noexcept override;
    bool is_thread_safe() const noexcept override;
    std::string_view thread_model() const noexcept override;

    // === Configuration Management ===
    std::optional<QJsonObject> default_configuration() const override;
    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;
    QJsonObject current_configuration() const override;
    bool validate_configuration(const QJsonObject& config) const override;

    // === Command Execution ===
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params) override;
    std::vector<std::string> available_commands() const override;

    // === Lifecycle Management ===
    qtplugin::expected<void, qtplugin::PluginError> pause() override;
    qtplugin::expected<void, qtplugin::PluginError> resume() override;
    qtplugin::expected<void, qtplugin::PluginError> restart() override;

    // === Dependency Management ===
    std::vector<std::string> dependencies() const override;
    std::vector<std::string> optional_dependencies() const override;
    bool dependencies_satisfied() const override;

    // === Monitoring ===
    std::chrono::milliseconds uptime() const override;
    QJsonObject performance_metrics() const override;
    QJsonObject resource_usage() const override;
    void clear_errors() override;

    // === Monitoring-Specific Methods ===

    /**
     * @brief Enable hot reload for a plugin
     * @param plugin_id Plugin identifier
     * @param file_path Path to plugin file
     * @return Success or error
     */
    qtplugin::expected<void, qtplugin::PluginError> enable_hot_reload(
        const QString& plugin_id, const QString& file_path);

    /**
     * @brief Disable hot reload for a plugin
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, qtplugin::PluginError> disable_hot_reload(
        const QString& plugin_id);

    /**
     * @brief Collect metrics for a specific plugin
     * @param plugin_id Plugin identifier
     * @return Collected metrics
     */
    QJsonObject collect_plugin_metrics(const QString& plugin_id);

    /**
     * @brief Get system-wide monitoring dashboard
     * @return Dashboard data
     */
    QJsonObject get_monitoring_dashboard() const;

    /**
     * @brief Set up monitoring alerts
     * @param alert_config Alert configuration
     * @return Success or error
     */
    qtplugin::expected<void, qtplugin::PluginError> setup_alerts(
        const QJsonObject& alert_config);

    /**
     * @brief Get historical metrics data
     * @param time_range Time range for metrics
     * @param plugin_id Optional plugin filter
     * @return Historical metrics
     */
    QJsonObject get_historical_metrics(
        const QJsonObject& time_range, const QString& plugin_id = QString());

private slots:
    void on_monitoring_timer_timeout();
    void on_file_changed(const QString& path);
    void on_metrics_collection_timeout();
    void on_alert_check_timeout();

private:
    // === Monitoring Components ===
    std::unique_ptr<qtplugin::IPluginHotReloadManager> m_hot_reload_manager;
    std::unique_ptr<qtplugin::IPluginMetricsCollector> m_metrics_collector;
    std::unique_ptr<QFileSystemWatcher> m_file_watcher;

    // === State Management ===
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    std::atomic<bool> m_dependencies_satisfied{false};
    mutable QReadWriteLock m_state_mutex;

    // === Configuration ===
    QJsonObject m_configuration;
    bool m_hot_reload_enabled{true};
    bool m_metrics_collection_enabled{true};
    bool m_alerts_enabled{true};
    int m_monitoring_interval{5000}; // 5 seconds
    int m_metrics_collection_interval{10000}; // 10 seconds
    int m_alert_check_interval{15000}; // 15 seconds
    int m_metrics_history_size{1000};

    // === Timers ===
    std::unique_ptr<QTimer> m_monitoring_timer;
    std::unique_ptr<QTimer> m_metrics_timer;
    std::unique_ptr<QTimer> m_alert_timer;
    std::chrono::system_clock::time_point m_initialization_time;

    // === Metrics Storage ===
    mutable QMutex m_metrics_mutex;
    std::vector<QJsonObject> m_metrics_history;
    std::unordered_map<QString, QJsonObject> m_plugin_metrics;
    std::unordered_map<QString, std::chrono::system_clock::time_point> m_plugin_last_seen;

    // === Hot Reload Tracking ===
    mutable QMutex m_hot_reload_mutex;
    std::unordered_map<QString, QString> m_monitored_plugins; // plugin_id -> file_path
    std::unordered_map<QString, std::chrono::system_clock::time_point> m_last_reload_times;
    std::atomic<uint64_t> m_reload_count{0};

    // === Alert System ===
    mutable QMutex m_alert_mutex;
    QJsonObject m_alert_config;
    std::vector<QJsonObject> m_active_alerts;
    std::atomic<uint64_t> m_alert_count{0};

    // === Performance Tracking ===
    std::atomic<uint64_t> m_monitoring_cycles{0};
    std::atomic<uint64_t> m_metrics_collections{0};
    std::atomic<uint64_t> m_file_changes_detected{0};

    // === Dependencies ===
    std::vector<std::string> m_required_dependencies;
    std::vector<std::string> m_optional_dependencies;

    // === Error Handling ===
    mutable QMutex m_error_mutex;
    std::vector<std::string> m_error_log;
    std::string m_last_error;
    std::atomic<uint64_t> m_error_count{0};
    static constexpr size_t MAX_ERROR_LOG_SIZE = 100;

    // === Command Handlers ===
    QJsonObject handle_hot_reload_command(const QJsonObject& params);
    QJsonObject handle_metrics_command(const QJsonObject& params);
    QJsonObject handle_dashboard_command(const QJsonObject& params);
    QJsonObject handle_alerts_command(const QJsonObject& params);
    QJsonObject handle_status_command(const QJsonObject& params);
    QJsonObject handle_history_command(const QJsonObject& params);

    // === Helper Methods ===
    void log_error(const std::string& error);
    void log_info(const std::string& message);
    void update_metrics();
    void initialize_monitoring_components();
    void start_monitoring();
    void stop_monitoring();
    void collect_system_metrics();
    void check_alerts();
    void process_file_change(const QString& file_path);
    QJsonObject create_metric_entry(const QString& plugin_id, const QJsonObject& data);
    void maintain_metrics_history();
    bool evaluate_alert_condition(const QJsonObject& condition, const QJsonObject& metrics);

public:
    // === Plugin Factory ===
    static std::unique_ptr<MonitoringPlugin> create_instance();
    static qtplugin::PluginMetadata get_static_metadata();
};
