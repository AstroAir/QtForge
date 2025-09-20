/**
 * @file platform_performance_monitor.hpp
 * @brief Platform-specific performance monitoring and optimization
 * @version 3.0.0
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include "../interfaces/core/plugin_interface.hpp"
#include "../utils/error_handling.hpp"

// Platform-specific includes
#ifdef Q_OS_WIN
#include <pdh.h>
#include <pdhmsg.h>
#include <psapi.h>
#include <windows.h>
#endif

#ifdef Q_OS_UNIX
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/times.h>
#include <unistd.h>
#endif

#ifdef Q_OS_MAC
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/task.h>
#include <sys/sysctl.h>
#endif

namespace qtplugin {

/**
 * @brief Performance metric types
 */
enum class PerformanceMetricType {
    CPUUsage,             ///< CPU usage percentage
    MemoryUsage,          ///< Memory usage in bytes
    DiskIO,               ///< Disk I/O operations
    NetworkIO,            ///< Network I/O operations
    ThreadCount,          ///< Number of threads
    HandleCount,          ///< Number of handles (Windows)
    FileDescriptorCount,  ///< Number of file descriptors (Unix)
    LoadTime,             ///< Plugin load time
    ExecutionTime,        ///< Method execution time
    ResponseTime,         ///< Response time
    Throughput,           ///< Operations per second
    ErrorRate,            ///< Error rate percentage
    CacheHitRate,         ///< Cache hit rate percentage
    Custom                ///< Custom metric
};

/**
 * @brief Performance monitoring configuration
 */
struct PerformanceMonitoringConfig {
    bool enable_cpu_monitoring = true;       ///< Enable CPU monitoring
    bool enable_memory_monitoring = true;    ///< Enable memory monitoring
    bool enable_io_monitoring = true;        ///< Enable I/O monitoring
    bool enable_network_monitoring = false;  ///< Enable network monitoring
    bool enable_plugin_profiling = true;     ///< Enable plugin profiling
    std::chrono::milliseconds sampling_interval{1000};  ///< Sampling interval
    int max_history_entries = 1000;        ///< Maximum history entries
    bool enable_alerts = true;             ///< Enable performance alerts
    QJsonObject alert_thresholds;          ///< Alert thresholds
    QJsonObject platform_specific_config;  ///< Platform-specific configuration

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static PerformanceMonitoringConfig from_json(const QJsonObject& json);
};

/**
 * @brief Performance metric data
 */
struct PerformanceMetric {
    PerformanceMetricType type;                       ///< Metric type
    QString name;                                     ///< Metric name
    double value = 0.0;                               ///< Metric value
    QString unit;                                     ///< Metric unit
    std::chrono::system_clock::time_point timestamp;  ///< Measurement timestamp
    QString source;        ///< Metric source (plugin ID, system, etc.)
    QJsonObject metadata;  ///< Additional metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static PerformanceMetric from_json(const QJsonObject& json);
};

/**
 * @brief Performance statistics
 */
struct PerformanceStatistics {
    QString source;  ///< Statistics source
    std::chrono::system_clock::time_point
        start_time;                                  ///< Monitoring start time
    std::chrono::system_clock::time_point end_time;  ///< Monitoring end time
    std::unordered_map<PerformanceMetricType, double>
        average_values;  ///< Average values
    std::unordered_map<PerformanceMetricType, double>
        min_values;  ///< Minimum values
    std::unordered_map<PerformanceMetricType, double>
        max_values;  ///< Maximum values
    std::unordered_map<PerformanceMetricType, double>
        current_values;             ///< Current values
    uint64_t total_samples = 0;     ///< Total samples collected
    QJsonObject custom_statistics;  ///< Custom statistics

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Performance alert information
 */
struct PerformanceAlert {
    QString alert_id;                   ///< Alert identifier
    PerformanceMetricType metric_type;  ///< Metric type that triggered alert
    QString source;                     ///< Alert source
    double threshold_value = 0.0;       ///< Threshold value
    double actual_value = 0.0;          ///< Actual value that triggered alert
    QString alert_message;              ///< Alert message
    std::chrono::system_clock::time_point timestamp;  ///< Alert timestamp
    bool is_resolved = false;  ///< Whether alert is resolved
    QJsonObject metadata;      ///< Additional metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Performance alert callback
 */
using PerformanceAlertCallback = std::function<void(const PerformanceAlert&)>;

/**
 * @brief Performance optimization suggestion
 */
struct PerformanceOptimizationSuggestion {
    QString suggestion_id;               ///< Suggestion identifier
    QString title;                       ///< Suggestion title
    QString description;                 ///< Detailed description
    QString category;                    ///< Suggestion category
    int priority = 0;                    ///< Priority (higher = more important)
    double potential_improvement = 0.0;  ///< Potential improvement percentage
    QStringList affected_plugins;        ///< Affected plugins
    QJsonObject implementation_details;  ///< Implementation details

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Platform-specific performance monitor
 *
 * This class provides comprehensive performance monitoring and optimization
 * suggestions for plugins with platform-specific optimizations.
 */
class PlatformPerformanceMonitor : public QObject {
    Q_OBJECT

public:
    explicit PlatformPerformanceMonitor(QObject* parent = nullptr);
    ~PlatformPerformanceMonitor() override;

    // === Configuration ===

    /**
     * @brief Set monitoring configuration
     * @param config Monitoring configuration
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_monitoring_config(
        const PerformanceMonitoringConfig& config);

    /**
     * @brief Get monitoring configuration
     * @return Current monitoring configuration
     */
    PerformanceMonitoringConfig get_monitoring_config() const;

    /**
     * @brief Enable monitoring
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> enable_monitoring();

    /**
     * @brief Disable monitoring
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> disable_monitoring();

    /**
     * @brief Check if monitoring is enabled
     * @return true if monitoring is enabled
     */
    bool is_monitoring_enabled() const;

    // === Plugin Performance Monitoring ===

    /**
     * @brief Start monitoring plugin
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> start_plugin_monitoring(
        const QString& plugin_id);

    /**
     * @brief Stop monitoring plugin
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> stop_plugin_monitoring(
        const QString& plugin_id);

    /**
     * @brief Get plugin performance statistics
     * @param plugin_id Plugin identifier
     * @return Performance statistics or error
     */
    qtplugin::expected<PerformanceStatistics, PluginError>
    get_plugin_statistics(const QString& plugin_id) const;

    /**
     * @brief Get all monitored plugins
     * @return Vector of monitored plugin IDs
     */
    std::vector<QString> get_monitored_plugins() const;

    // === System Performance Monitoring ===

    /**
     * @brief Get system performance metrics
     * @return Current system metrics
     */
    std::vector<PerformanceMetric> get_system_metrics();

    /**
     * @brief Get system performance statistics
     * @return System performance statistics
     */
    PerformanceStatistics get_system_statistics() const;

    /**
     * @brief Get CPU usage
     * @return CPU usage percentage
     */
    double get_cpu_usage();

    /**
     * @brief Get memory usage
     * @return Memory usage in bytes
     */
    uint64_t get_memory_usage();

    /**
     * @brief Get available memory
     * @return Available memory in bytes
     */
    uint64_t get_available_memory();

    /**
     * @brief Get disk I/O statistics
     * @return Disk I/O statistics
     */
    QJsonObject get_disk_io_statistics();

    /**
     * @brief Get network I/O statistics
     * @return Network I/O statistics
     */
    QJsonObject get_network_io_statistics();

    // === Performance Profiling ===

    /**
     * @brief Start profiling session
     * @param session_name Profiling session name
     * @param target_plugins Target plugins to profile
     * @return Session ID or error
     */
    qtplugin::expected<QString, PluginError> start_profiling_session(
        const QString& session_name,
        const std::vector<QString>& target_plugins = {});

    /**
     * @brief Stop profiling session
     * @param session_id Session identifier
     * @return Profiling results or error
     */
    qtplugin::expected<QJsonObject, PluginError> stop_profiling_session(
        const QString& session_id);

    /**
     * @brief Get active profiling sessions
     * @return Vector of active session IDs
     */
    std::vector<QString> get_active_profiling_sessions() const;

    /**
     * @brief Profile method execution
     * @param plugin_id Plugin identifier
     * @param method_name Method name
     * @param execution_time Execution time
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> profile_method_execution(
        const QString& plugin_id, const QString& method_name,
        std::chrono::microseconds execution_time);

    // === Performance Alerts ===

    /**
     * @brief Set performance alert threshold
     * @param metric_type Metric type
     * @param threshold_value Threshold value
     * @param source Optional source filter
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_alert_threshold(
        PerformanceMetricType metric_type, double threshold_value,
        const QString& source = QString());

    /**
     * @brief Remove performance alert threshold
     * @param metric_type Metric type
     * @param source Optional source filter
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> remove_alert_threshold(
        PerformanceMetricType metric_type, const QString& source = QString());

    /**
     * @brief Register alert callback
     * @param callback Alert callback function
     * @return Callback ID for unregistration
     */
    QString register_alert_callback(PerformanceAlertCallback callback);

    /**
     * @brief Unregister alert callback
     * @param callback_id Callback identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unregister_alert_callback(
        const QString& callback_id);

    /**
     * @brief Get active alerts
     * @return Vector of active alerts
     */
    std::vector<PerformanceAlert> get_active_alerts() const;

    /**
     * @brief Resolve alert
     * @param alert_id Alert identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> resolve_alert(
        const QString& alert_id);

    // === Performance Optimization ===

    /**
     * @brief Analyze performance and generate suggestions
     * @param target_plugins Target plugins to analyze
     * @return Vector of optimization suggestions
     */
    std::vector<PerformanceOptimizationSuggestion> analyze_performance(
        const std::vector<QString>& target_plugins = {});

    /**
     * @brief Get optimization suggestions for plugin
     * @param plugin_id Plugin identifier
     * @return Vector of suggestions for the plugin
     */
    std::vector<PerformanceOptimizationSuggestion>
    get_plugin_optimization_suggestions(const QString& plugin_id);

    /**
     * @brief Apply optimization suggestion
     * @param suggestion_id Suggestion identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> apply_optimization_suggestion(
        const QString& suggestion_id);

    // === Data Export and Reporting ===

    /**
     * @brief Export performance data
     * @param file_path Export file path
     * @param format Export format (json, csv, xml)
     * @param time_range Optional time range filter
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> export_performance_data(
        const QString& file_path, const QString& format = "json",
        const QJsonObject& time_range = {});

    /**
     * @brief Generate performance report
     * @param report_type Report type (summary, detailed, comparison)
     * @param target_plugins Target plugins for report
     * @return Performance report or error
     */
    qtplugin::expected<QJsonObject, PluginError> generate_performance_report(
        const QString& report_type = "summary",
        const std::vector<QString>& target_plugins = {});

    // === Platform-Specific Methods ===

#ifdef Q_OS_WIN
    /**
     * @brief Get Windows performance counters
     * @return Windows performance counters
     */
    QJsonObject get_windows_performance_counters();

    /**
     * @brief Get Windows process information
     * @param process_id Process ID
     * @return Process information
     */
    QJsonObject get_windows_process_info(DWORD process_id = 0);
#endif

#ifdef Q_OS_UNIX
    /**
     * @brief Get Unix system statistics
     * @return Unix system statistics
     */
    QJsonObject get_unix_system_statistics();

    /**
     * @brief Get Unix process statistics
     * @param process_id Process ID
     * @return Process statistics
     */
    QJsonObject get_unix_process_statistics(pid_t process_id = 0);
#endif

#ifdef Q_OS_MAC
    /**
     * @brief Get macOS system statistics
     * @return macOS system statistics
     */
    QJsonObject get_macos_system_statistics();

    /**
     * @brief Get macOS task information
     * @param task Task port
     * @return Task information
     */
    QJsonObject get_macos_task_info(mach_port_t task = mach_task_self());
#endif

signals:
    /**
     * @brief Emitted when performance metric is collected
     * @param metric Performance metric
     */
    void metric_collected(const PerformanceMetric& metric);

    /**
     * @brief Emitted when performance alert is triggered
     * @param alert Performance alert
     */
    void alert_triggered(const PerformanceAlert& alert);

    /**
     * @brief Emitted when performance alert is resolved
     * @param alert_id Alert identifier
     */
    void alert_resolved(const QString& alert_id);

    /**
     * @brief Emitted when optimization suggestion is generated
     * @param suggestion Optimization suggestion
     */
    void optimization_suggestion_generated(
        const PerformanceOptimizationSuggestion& suggestion);

private slots:
    void on_monitoring_timer();
    void on_profiling_timer();
    void check_performance_alerts();

private:
    class Private;
    std::unique_ptr<Private> d;

    void initialize_platform_monitoring();
    void cleanup_platform_monitoring();
    void collect_system_metrics();
    void collect_plugin_metrics();
    void analyze_performance_trends();
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::PerformanceMetricType)
Q_DECLARE_METATYPE(qtplugin::PerformanceMonitoringConfig)
Q_DECLARE_METATYPE(qtplugin::PerformanceMetric)
Q_DECLARE_METATYPE(qtplugin::PerformanceStatistics)
Q_DECLARE_METATYPE(qtplugin::PerformanceAlert)
Q_DECLARE_METATYPE(qtplugin::PerformanceOptimizationSuggestion)
