/**
 * @file monitoring_plugin.cpp
 * @brief Implementation of monitoring plugin demonstrating QtForge monitoring features
 * @version 3.0.0
 */

#include "monitoring_plugin.hpp"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QMutexLocker>
#include <QObject>
#include <chrono>
#include <thread>

MonitoringPlugin::MonitoringPlugin(QObject* parent)
    : QObject(parent),
      m_file_watcher(std::make_unique<QFileSystemWatcher>(this)),
      m_monitoring_timer(std::make_unique<QTimer>(this)),
      m_metrics_timer(std::make_unique<QTimer>(this)),
      m_alert_timer(std::make_unique<QTimer>(this)) {

    // Connect timers
    connect(m_monitoring_timer.get(), &QTimer::timeout, this,
            &MonitoringPlugin::on_monitoring_timer_timeout);
    connect(m_metrics_timer.get(), &QTimer::timeout, this,
            &MonitoringPlugin::on_metrics_collection_timeout);
    connect(m_alert_timer.get(), &QTimer::timeout, this,
            &MonitoringPlugin::on_alert_check_timeout);

    // Connect file watcher
    connect(m_file_watcher.get(), &QFileSystemWatcher::fileChanged, this,
            &MonitoringPlugin::on_file_changed);

    // Initialize dependencies
    m_required_dependencies = {"qtplugin.PluginHotReloadManager", "qtplugin.PluginMetricsCollector"};
    m_optional_dependencies = {"qtplugin.MessageBus", "qtplugin.ConfigurationManager"};

    log_info("MonitoringPlugin constructed");
}

MonitoringPlugin::~MonitoringPlugin() {
    if (m_state != qtplugin::PluginState::Unloaded) {
        shutdown();
    }
}

qtplugin::expected<void, qtplugin::PluginError> MonitoringPlugin::initialize() {
    if (m_state != qtplugin::PluginState::Unloaded &&
        m_state != qtplugin::PluginState::Loaded) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::StateError,
            "Plugin is not in a state that allows initialization");
    }

    m_state = qtplugin::PluginState::Initializing;
    m_initialization_time = std::chrono::system_clock::now();

    try {
        // Initialize monitoring components
        initialize_monitoring_components();

        // Start monitoring services
        start_monitoring();

        m_state = qtplugin::PluginState::Running;
        log_info("MonitoringPlugin initialized successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        m_state = qtplugin::PluginState::Error;
        std::string error_msg = "Initialization failed: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed, error_msg);
    }
}

void MonitoringPlugin::shutdown() noexcept {
    try {
        QWriteLocker lock(&m_state_mutex);
        m_state = qtplugin::PluginState::Stopping;
        lock.unlock();

        // Stop monitoring services
        stop_monitoring();

        lock.relock();
        m_state = qtplugin::PluginState::Stopped;
        lock.unlock();

        log_info("MonitoringPlugin shutdown completed");
    } catch (...) {
        QWriteLocker lock(&m_state_mutex);
        m_state = qtplugin::PluginState::Error;
    }
}

bool MonitoringPlugin::is_initialized() const noexcept {
    QReadLocker lock(&m_state_mutex);
    auto current_state = m_state.load();
    return current_state == qtplugin::PluginState::Running ||
           current_state == qtplugin::PluginState::Paused;
}

qtplugin::PluginMetadata MonitoringPlugin::metadata() const {
    qtplugin::PluginMetadata meta;
    meta.name = "MonitoringPlugin";
    meta.version = qtplugin::Version{3, 0, 0};
    meta.description = "Comprehensive monitoring plugin demonstrating QtForge monitoring features";
    meta.author = "QtForge Team";
    meta.license = "MIT";
    meta.category = "Monitoring";
    meta.tags = {"monitoring", "hot-reload", "metrics", "performance", "example"};

    // Monitoring-specific metadata
    QJsonObject custom_data;
    custom_data[QLatin1String("hot_reload_enabled")] = m_hot_reload_enabled;
    custom_data[QLatin1String("metrics_collection_enabled")] = m_metrics_collection_enabled;
    custom_data[QLatin1String("alerts_enabled")] = m_alerts_enabled;
    custom_data[QLatin1String("monitoring_interval")] = m_monitoring_interval;
    custom_data[QLatin1String("metrics_history_size")] = m_metrics_history_size;
    meta.custom_data = custom_data;

    return meta;
}

qtplugin::PluginCapabilities MonitoringPlugin::capabilities() const noexcept {
    return qtplugin::PluginCapability::Monitoring |
           qtplugin::PluginCapability::HotReload |
           qtplugin::PluginCapability::Configuration |
           qtplugin::PluginCapability::Logging |
           qtplugin::PluginCapability::Threading;
}

qtplugin::PluginPriority MonitoringPlugin::priority() const noexcept {
    return qtplugin::PluginPriority::High; // Monitoring plugins should have high priority
}

bool MonitoringPlugin::is_thread_safe() const noexcept {
    return true;
}

std::string_view MonitoringPlugin::thread_model() const noexcept {
    return "multi-threaded";
}

std::optional<QJsonObject> MonitoringPlugin::default_configuration() const {
    return QJsonObject{
        {QLatin1String("hot_reload_enabled"), true},
        {QLatin1String("metrics_collection_enabled"), true},
        {QLatin1String("alerts_enabled"), true},
        {QLatin1String("monitoring_interval"), 5000},
        {QLatin1String("metrics_collection_interval"), 10000},
        {QLatin1String("alert_check_interval"), 15000},
        {QLatin1String("metrics_history_size"), 1000},
        {QLatin1String("watched_directories"), QJsonArray{
            QString(QCoreApplication::applicationDirPath() + QLatin1String("/plugins"))
        }},
        {QLatin1String("metric_types"), QJsonArray{
            QLatin1String("cpu_usage"), QLatin1String("memory_usage"), QLatin1String("plugin_count"), QLatin1String("error_rate")
        }},
        {QLatin1String("alert_thresholds"), QJsonObject{
            {QLatin1String("cpu_usage_max"), 80.0},
            {QLatin1String("memory_usage_max"), 1024.0}, // MB
            {QLatin1String("error_rate_max"), 5.0}, // errors per minute
            {QLatin1String("plugin_load_time_max"), 5000} // milliseconds
        }},
        {QLatin1String("dashboard_refresh_rate"), 2000},
        {QLatin1String("enable_file_monitoring"), true},
        {QLatin1String("enable_performance_tracking"), true}
    };
}

qtplugin::expected<void, qtplugin::PluginError> MonitoringPlugin::configure(
    const QJsonObject& config) {
    if (!validate_configuration(config)) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError,
            "Invalid configuration provided");
    }

    // Store old configuration for comparison
    QJsonObject old_config = m_configuration;
    m_configuration = config;

    // Apply configuration changes
    if (config.contains(QLatin1String("hot_reload_enabled"))) {
        m_hot_reload_enabled = config[QLatin1String("hot_reload_enabled")].toBool();
    }

    if (config.contains(QLatin1String("metrics_collection_enabled"))) {
        m_metrics_collection_enabled = config[QLatin1String("metrics_collection_enabled")].toBool();
    }

    if (config.contains(QLatin1String("alerts_enabled"))) {
        m_alerts_enabled = config[QLatin1String("alerts_enabled")].toBool();
    }

    if (config.contains(QLatin1String("monitoring_interval"))) {
        m_monitoring_interval = config[QLatin1String("monitoring_interval")].toInt();
        if (m_monitoring_timer && m_monitoring_timer->isActive()) {
            m_monitoring_timer->setInterval(m_monitoring_interval);
        }
    }

    if (config.contains(QLatin1String("metrics_collection_interval"))) {
        m_metrics_collection_interval = config[QLatin1String("metrics_collection_interval")].toInt();
        if (m_metrics_timer && m_metrics_timer->isActive()) {
            m_metrics_timer->setInterval(m_metrics_collection_interval);
        }
    }

    if (config.contains(QLatin1String("alert_check_interval"))) {
        m_alert_check_interval = config[QLatin1String("alert_check_interval")].toInt();
        if (m_alert_timer && m_alert_timer->isActive()) {
            m_alert_timer->setInterval(m_alert_check_interval);
        }
    }

    if (config.contains(QLatin1String("metrics_history_size"))) {
        m_metrics_history_size = config[QLatin1String("metrics_history_size")].toInt();
        maintain_metrics_history(); // Trim if necessary
    }

    // Update watched directories
    if (config.contains(QLatin1String("watched_directories"))) {
        QJsonArray dirs = config[QLatin1String("watched_directories")].toArray();
        QStringList dir_list;
        for (const auto& dir : dirs) {
            dir_list.append(dir.toString());
        }

        // Update file watcher
        if (!m_file_watcher->directories().isEmpty()) {
            m_file_watcher->removePaths(m_file_watcher->directories());
        }
        m_file_watcher->addPaths(dir_list);
    }

    // Update alert configuration
    if (config.contains(QLatin1String("alert_thresholds"))) {
        QMutexLocker lock(&m_alert_mutex);
        m_alert_config = config[QLatin1String("alert_thresholds")].toObject();
    }

    log_info("Monitoring configuration updated successfully");
    return qtplugin::make_success();
}

QJsonObject MonitoringPlugin::current_configuration() const {
    return m_configuration;
}

bool MonitoringPlugin::validate_configuration(const QJsonObject& config) const {
    // Validate monitoring_interval
    if (config.contains(QLatin1String("monitoring_interval"))) {
        if (!config[QLatin1String("monitoring_interval")].isDouble()) {
            return false;
        }
        int interval = config[QLatin1String("monitoring_interval")].toInt();
        if (interval < 1000 || interval > 60000) { // 1 second to 1 minute
            return false;
        }
    }

    // Validate metrics_history_size
    if (config.contains(QLatin1String("metrics_history_size"))) {
        if (!config[QLatin1String("metrics_history_size")].isDouble()) {
            return false;
        }
        int size = config[QLatin1String("metrics_history_size")].toInt();
        if (size < 100 || size > 10000) {
            return false;
        }
    }

    // Validate boolean flags
    const std::vector<QString> boolean_flags = {
        QLatin1String("hot_reload_enabled"),
        QLatin1String("metrics_collection_enabled"),
        QLatin1String("alerts_enabled")
    };
    for (const QString& flag : boolean_flags) {
        if (config.contains(flag)) {
            if (!config[flag].isBool()) {
                return false;
            }
        }
    }

    return true;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError> MonitoringPlugin::execute_command(
    std::string_view command, const QJsonObject& params) {

    m_monitoring_cycles.fetch_add(1);

    if (command == "hot_reload") {
        return handle_hot_reload_command(params);
    } else if (command == "metrics") {
        return handle_metrics_command(params);
    } else if (command == "dashboard") {
        return handle_dashboard_command(params);
    } else if (command == "alerts") {
        return handle_alerts_command(params);
    } else if (command == "status") {
        return handle_status_command(params);
    } else if (command == "history") {
        return handle_history_command(params);
    } else {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound,
            "Unknown command: " + std::string(command));
    }
}

std::vector<std::string> MonitoringPlugin::available_commands() const {
    return {"hot_reload", "metrics", "dashboard", "alerts", "status", "history"};
}

// === Lifecycle Management ===

qtplugin::expected<void, qtplugin::PluginError> MonitoringPlugin::pause() {
    QWriteLocker lock(&m_state_mutex);

    if (m_state != qtplugin::PluginState::Running) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be running to pause");
    }

    try {
        // Pause monitoring timers
        if (m_monitoring_timer && m_monitoring_timer->isActive()) {
            m_monitoring_timer->stop();
        }
        if (m_metrics_timer && m_metrics_timer->isActive()) {
            m_metrics_timer->stop();
        }
        if (m_alert_timer && m_alert_timer->isActive()) {
            m_alert_timer->stop();
        }

        m_state = qtplugin::PluginState::Paused;
        log_info("MonitoringPlugin paused successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg = "Failed to pause plugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> MonitoringPlugin::resume() {
    QWriteLocker lock(&m_state_mutex);

    if (m_state != qtplugin::PluginState::Paused) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be paused to resume");
    }

    try {
        // Resume monitoring timers
        if (m_monitoring_timer) {
            m_monitoring_timer->setInterval(m_monitoring_interval);
            m_monitoring_timer->start();
        }
        if (m_metrics_timer) {
            m_metrics_timer->setInterval(m_metrics_collection_interval);
            m_metrics_timer->start();
        }
        if (m_alert_timer) {
            m_alert_timer->setInterval(m_alert_check_interval);
            m_alert_timer->start();
        }

        m_state = qtplugin::PluginState::Running;
        log_info("MonitoringPlugin resumed successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg = "Failed to resume plugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> MonitoringPlugin::restart() {
    log_info("Restarting MonitoringPlugin...");

    // Shutdown first
    shutdown();

    // Wait a brief moment for cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Initialize again
    return initialize();
}

// === Dependency Management ===

std::vector<std::string> MonitoringPlugin::dependencies() const {
    return m_required_dependencies;
}

std::vector<std::string> MonitoringPlugin::optional_dependencies() const {
    return m_optional_dependencies;
}

bool MonitoringPlugin::dependencies_satisfied() const {
    return m_dependencies_satisfied.load();
}

// === Monitoring ===

std::chrono::milliseconds MonitoringPlugin::uptime() const {
    if (m_state == qtplugin::PluginState::Running) {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_initialization_time);
    }
    return std::chrono::milliseconds{0};
}

QJsonObject MonitoringPlugin::performance_metrics() const {
    QMutexLocker lock(&m_metrics_mutex);

    auto current_uptime = uptime();
    auto monitoring_rate =
        current_uptime.count() > 0
            ? (m_monitoring_cycles.load() * 1000.0) / current_uptime.count()
            : 0.0;

    return QJsonObject{
        {QLatin1String("uptime_ms"), static_cast<qint64>(current_uptime.count())},
        {QLatin1String("monitoring_cycles"), static_cast<qint64>(m_monitoring_cycles.load())},
        {QLatin1String("metrics_collections"), static_cast<qint64>(m_metrics_collections.load())},
        {QLatin1String("file_changes_detected"), static_cast<qint64>(m_file_changes_detected.load())},
        {QLatin1String("reload_count"), static_cast<qint64>(m_reload_count.load())},
        {QLatin1String("alert_count"), static_cast<qint64>(m_alert_count.load())},
        {QLatin1String("monitoring_rate_per_second"), monitoring_rate},
        {QLatin1String("state"), static_cast<int>(m_state.load())},
        {QLatin1String("hot_reload_enabled"), m_hot_reload_enabled},
        {QLatin1String("metrics_collection_enabled"), m_metrics_collection_enabled},
        {QLatin1String("alerts_enabled"), m_alerts_enabled},
        {QLatin1String("monitored_plugins_count"), static_cast<int>(m_monitored_plugins.size())},
        {QLatin1String("metrics_history_size"), static_cast<int>(m_metrics_history.size())}
    };
}

QJsonObject MonitoringPlugin::resource_usage() const {
    QMutexLocker lock(&m_metrics_mutex);

    // Estimate resource usage
    auto memory_estimate = 2048 + (m_metrics_history.size() * 200) +
                          (m_monitored_plugins.size() * 100);
    auto cpu_estimate = (m_monitoring_timer && m_monitoring_timer->isActive()) ? 2.0 : 0.1;

    return QJsonObject{
        {QLatin1String("estimated_memory_kb"), static_cast<qint64>(memory_estimate)},
        {QLatin1String("estimated_cpu_percent"), cpu_estimate},
        {QLatin1String("thread_count"), 1},
        {QLatin1String("monitoring_timer_active"), m_monitoring_timer && m_monitoring_timer->isActive()},
        {QLatin1String("metrics_timer_active"), m_metrics_timer && m_metrics_timer->isActive()},
        {QLatin1String("alert_timer_active"), m_alert_timer && m_alert_timer->isActive()},
        {QLatin1String("watched_files_count"), m_file_watcher->files().size()},
        {QLatin1String("watched_directories_count"), m_file_watcher->directories().size()},
        {QLatin1String("error_log_size"), static_cast<qint64>(m_error_log.size())},
        {QLatin1String("dependencies_satisfied"), dependencies_satisfied()}
    };
}

void MonitoringPlugin::clear_errors() {
    QMutexLocker lock(&m_error_mutex);
    m_error_log.clear();
    m_last_error.clear();
    m_error_count = 0;
}

// === Monitoring-Specific Methods ===

qtplugin::expected<void, qtplugin::PluginError> MonitoringPlugin::enable_hot_reload(
    const QString& plugin_id, const QString& file_path) {

    if (!m_hot_reload_manager) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed,
            "Hot reload manager not initialized");
    }

    try {
        std::filesystem::path path = file_path.toStdString();
        auto result = m_hot_reload_manager->enable_hot_reload(plugin_id.toStdString(), path);

        if (result) {
            QMutexLocker lock(&m_hot_reload_mutex);
            m_monitored_plugins[plugin_id] = file_path;
            m_last_reload_times[plugin_id] = std::chrono::system_clock::now();

            // Add to file watcher
            if (!m_file_watcher->files().contains(file_path)) {
                m_file_watcher->addPath(file_path);
            }

            log_info("Hot reload enabled for plugin: " + plugin_id.toStdString());
        }

        return result;
    } catch (const std::exception& e) {
        std::string error_msg = "Failed to enable hot reload: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> MonitoringPlugin::disable_hot_reload(
    const QString& plugin_id) {

    if (!m_hot_reload_manager) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed,
            "Hot reload manager not initialized");
    }

    try {
        m_hot_reload_manager->disable_hot_reload(plugin_id.toStdString());

        {
            QMutexLocker lock(&m_hot_reload_mutex);
            auto it = m_monitored_plugins.find(plugin_id);
            QString file_path = (it != m_monitored_plugins.end()) ? it->second : QString();
            m_monitored_plugins.erase(plugin_id);
            m_last_reload_times.erase(plugin_id);

            // Remove from file watcher if no other plugins are watching this file
            if (!file_path.isEmpty()) {
                bool still_watched = false;
                for (const auto& pair : m_monitored_plugins) {
                    if (pair.second == file_path) {
                        still_watched = true;
                        break;
                    }
                }
                if (!still_watched) {
                    m_file_watcher->removePath(file_path);
                }
            }

            log_info("Hot reload disabled for plugin: " + plugin_id.toStdString());
        }

        return {};
    } catch (const std::exception& e) {
        std::string error_msg = "Failed to disable hot reload: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

QJsonObject MonitoringPlugin::collect_plugin_metrics(const QString& plugin_id) {
    if (!m_metrics_collector) {
        return QJsonObject{{QLatin1String("error"), QLatin1String("Metrics collector not initialized")}};
    }

    try {
        auto metrics = m_metrics_collector->get_plugin_metrics(plugin_id.toStdString(), nullptr);

        // Convert to QJsonObject
        QJsonObject result;
        result[QLatin1String("plugin_id")] = plugin_id;
        result[QLatin1String("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        result[QLatin1String("metrics")] = metrics; // Assuming metrics is already a QJsonObject

        // Store in plugin metrics cache
        {
            QMutexLocker lock(&m_metrics_mutex);
            m_plugin_metrics[plugin_id] = result;
            m_plugin_last_seen[plugin_id] = std::chrono::system_clock::now();
        }

        return result;
    } catch (const std::exception& e) {
        log_error("Failed to collect plugin metrics: " + std::string(e.what()));
        return QJsonObject{
            {QLatin1String("error"), QString::fromStdString(e.what())},
            {QLatin1String("plugin_id"), plugin_id},
            {QLatin1String("timestamp"), QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    }
}

QJsonObject MonitoringPlugin::get_monitoring_dashboard() const {
    QMutexLocker lock(&m_metrics_mutex);

    QJsonObject dashboard;
    dashboard[QLatin1String("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
    dashboard[QLatin1String("uptime_ms")] = static_cast<qint64>(uptime().count());

    // System overview
    QJsonObject system_overview;
    system_overview[QLatin1String("monitored_plugins")] = static_cast<int>(m_monitored_plugins.size());
    system_overview[QLatin1String("total_reloads")] = static_cast<qint64>(m_reload_count.load());
    system_overview[QLatin1String("active_alerts")] = static_cast<int>(m_active_alerts.size());
    system_overview[QLatin1String("metrics_collections")] = static_cast<qint64>(m_metrics_collections.load());
    dashboard[QLatin1String("system_overview")] = system_overview;

    // Plugin metrics summary
    QJsonArray plugin_summaries;
    for (const auto& [plugin_id, metrics] : m_plugin_metrics) {
        QJsonObject summary;
        summary[QLatin1String("plugin_id")] = plugin_id;
        summary[QLatin1String("last_update")] = metrics.value(QLatin1String("timestamp"));
        summary[QLatin1String("status")] = QLatin1String("active"); // Could be enhanced with actual status
        plugin_summaries.append(summary);
    }
    dashboard[QLatin1String("plugins")] = plugin_summaries;

    // Recent metrics (last 10 entries)
    QJsonArray recent_metrics;
    int start = std::max(0, static_cast<int>(m_metrics_history.size()) - 10);
    for (int i = start; i < static_cast<int>(m_metrics_history.size()); ++i) {
        recent_metrics.append(m_metrics_history[i]);
    }
    dashboard[QLatin1String("recent_metrics")] = recent_metrics;

    // Active alerts
    QJsonArray alerts_array;
    for (const auto& alert : m_active_alerts) {
        alerts_array.append(alert);
    }
    dashboard[QLatin1String("active_alerts")] = alerts_array;

    // Performance summary
    dashboard[QLatin1String("performance")] = performance_metrics();
    dashboard[QLatin1String("resource_usage")] = resource_usage();

    return dashboard;
}

qtplugin::expected<void, qtplugin::PluginError> MonitoringPlugin::setup_alerts(
    const QJsonObject& alert_config) {

    try {
        QMutexLocker lock(&m_alert_mutex);
        m_alert_config = alert_config;

        log_info("Alert configuration updated");
        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg = "Failed to setup alerts: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

QJsonObject MonitoringPlugin::get_historical_metrics(
    const QJsonObject& time_range, const QString& plugin_id) {

    QMutexLocker lock(&m_metrics_mutex);

    QJsonObject result;
    result[QLatin1String("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
    result[QLatin1String("time_range")] = time_range;

    if (!plugin_id.isEmpty()) {
        result[QLatin1String("plugin_filter")] = plugin_id;
    }

    // Parse time range
    QDateTime start_time = QDateTime::fromString(
        time_range.value(QLatin1String("start")).toString(), Qt::ISODate);
    QDateTime end_time = QDateTime::fromString(
        time_range.value(QLatin1String("end")).toString(), Qt::ISODate);

    if (!start_time.isValid()) {
        start_time = QDateTime::currentDateTime().addSecs(-3600); // Last hour
    }
    if (!end_time.isValid()) {
        end_time = QDateTime::currentDateTime();
    }

    // Filter metrics by time range and plugin
    QJsonArray filtered_metrics;
    for (const auto& metric : m_metrics_history) {
        QDateTime metric_time = QDateTime::fromString(
            metric.value(QLatin1String("timestamp")).toString(), Qt::ISODate);

        if (metric_time >= start_time && metric_time <= end_time) {
            if (plugin_id.isEmpty() ||
                metric.value(QLatin1String("plugin_id")).toString() == plugin_id) {
                filtered_metrics.append(metric);
            }
        }
    }

    result[QLatin1String("metrics")] = filtered_metrics;
    result[QLatin1String("count")] = filtered_metrics.size();

    return result;
}

// === Private Slots ===

void MonitoringPlugin::on_monitoring_timer_timeout() {
    // Perform periodic monitoring tasks
    update_metrics();
    collect_system_metrics();

    log_info("Monitoring cycle completed");
}

void MonitoringPlugin::on_file_changed(const QString& path) {
    m_file_changes_detected.fetch_add(1);

    log_info("File change detected: " + path.toStdString());
    process_file_change(path);
}

void MonitoringPlugin::on_metrics_collection_timeout() {
    if (!m_metrics_collection_enabled) {
        return;
    }

    m_metrics_collections.fetch_add(1);

    // Collect metrics for all monitored plugins
    QMutexLocker lock(&m_hot_reload_mutex);
    for (const auto& pair : m_monitored_plugins) {
        collect_plugin_metrics(pair.first);
    }
    lock.unlock();

    // Maintain metrics history
    maintain_metrics_history();

    log_info("Metrics collection completed");
}

void MonitoringPlugin::on_alert_check_timeout() {
    if (!m_alerts_enabled) {
        return;
    }

    check_alerts();
}

// === Command Handlers ===

QJsonObject MonitoringPlugin::handle_hot_reload_command(const QJsonObject& params) {
    QString action = params.value(QLatin1String("action")).toString(QLatin1String("status"));

    if (action == QLatin1String("enable")) {
        if (!params.contains(QLatin1String("plugin_id")) || !params.contains(QLatin1String("file_path"))) {
            return QJsonObject{
                {QLatin1String("error"), QLatin1String("Missing required parameters: plugin_id, file_path")},
                {QLatin1String("success"), false}
            };
        }

        QString plugin_id = params[QLatin1String("plugin_id")].toString();
        QString file_path = params[QLatin1String("file_path")].toString();

        auto result = enable_hot_reload(plugin_id, file_path);

        return QJsonObject{
            {QLatin1String("action"), QLatin1String("enable")},
            {QLatin1String("plugin_id"), plugin_id},
            {QLatin1String("file_path"), file_path},
            {QLatin1String("success"), result.has_value()},
            {QLatin1String("error"), result ? QLatin1String("") : QString::fromStdString(result.error().message)},
            {QLatin1String("timestamp"), QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    } else if (action == QLatin1String("disable")) {
        if (!params.contains(QLatin1String("plugin_id"))) {
            return QJsonObject{
                {QLatin1String("error"), QLatin1String("Missing required parameter: plugin_id")},
                {QLatin1String("success"), false}
            };
        }

        QString plugin_id = params[QLatin1String("plugin_id")].toString();
        auto result = disable_hot_reload(plugin_id);

        return QJsonObject{
            {QLatin1String("action"), QLatin1String("disable")},
            {QLatin1String("plugin_id"), plugin_id},
            {QLatin1String("success"), result.has_value()},
            {QLatin1String("error"), result ? QLatin1String("") : QString::fromStdString(result.error().message)},
            {QLatin1String("timestamp"), QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    } else if (action == QLatin1String("status")) {
        QMutexLocker lock(&m_hot_reload_mutex);

        QJsonArray monitored_plugins;
        for (auto it = m_monitored_plugins.begin(); it != m_monitored_plugins.end(); ++it) {
            QJsonObject plugin_info;
            plugin_info[QLatin1String("plugin_id")] = it->first;
            plugin_info[QLatin1String("file_path")] = it->second;

            auto reload_time_it = m_last_reload_times.find(it->first);
            if (reload_time_it != m_last_reload_times.end()) {
                auto time_point = reload_time_it->second;
                auto time_t = std::chrono::system_clock::to_time_t(time_point);
                plugin_info[QLatin1String("last_reload")] = QDateTime::fromSecsSinceEpoch(time_t).toString(Qt::ISODate);
            }

            monitored_plugins.append(plugin_info);
        }

        return QJsonObject{
            {QLatin1String("action"), QLatin1String("status")},
            {QLatin1String("hot_reload_enabled"), m_hot_reload_enabled},
            {QLatin1String("monitored_plugins"), monitored_plugins},
            {QLatin1String("total_reloads"), static_cast<qint64>(m_reload_count.load())},
            {QLatin1String("success"), true},
            {QLatin1String("timestamp"), QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    } else {
        return QJsonObject{
            {QLatin1String("error"), QLatin1String("Invalid action. Supported: enable, disable, status")},
            {QLatin1String("success"), false}
        };
    }
}

QJsonObject MonitoringPlugin::handle_metrics_command(const QJsonObject& params) {
    QString plugin_id = params.value(QLatin1String("plugin_id")).toString();

    if (plugin_id.isEmpty()) {
        // Return all plugin metrics
        QMutexLocker lock(&m_metrics_mutex);

        QJsonArray all_metrics;
        for (const auto& [id, metrics] : m_plugin_metrics) {
            all_metrics.append(metrics);
        }

        return QJsonObject{
            {QLatin1String("action"), QLatin1String("get_all")},
            {QLatin1String("metrics"), all_metrics},
            {QLatin1String("count"), all_metrics.size()},
            {QLatin1String("timestamp"), QDateTime::currentDateTime().toString(Qt::ISODate)},
            {QLatin1String("success"), true}
        };
    } else {
        // Return specific plugin metrics
        auto metrics = collect_plugin_metrics(plugin_id);

        return QJsonObject{
            {QLatin1String("action"), QLatin1String("get_plugin")},
            {QLatin1String("plugin_id"), plugin_id},
            {QLatin1String("metrics"), metrics},
            {QLatin1String("timestamp"), QDateTime::currentDateTime().toString(Qt::ISODate)},
            {QLatin1String("success"), !metrics.contains(QLatin1String("error"))}
        };
    }
}

QJsonObject MonitoringPlugin::handle_dashboard_command(const QJsonObject& params) {
    Q_UNUSED(params)

    auto dashboard = get_monitoring_dashboard();

    return QJsonObject{
        {QLatin1String("action"), QLatin1String("dashboard")},
        {QLatin1String("dashboard"), dashboard},
        {QLatin1String("success"), true}
    };
}

QJsonObject MonitoringPlugin::handle_alerts_command(const QJsonObject& params) {
    QString action = params.value(QLatin1String("action")).toString(QLatin1String("get"));

    if (action == QLatin1String("get")) {
        QMutexLocker lock(&m_alert_mutex);

        QJsonArray alerts_array;
        for (const auto& alert : m_active_alerts) {
            alerts_array.append(alert);
        }

        return QJsonObject{
            {QLatin1String("action"), QLatin1String("get")},
            {QLatin1String("alerts"), alerts_array},
            {QLatin1String("count"), alerts_array.size()},
            {QLatin1String("alert_config"), m_alert_config},
            {QLatin1String("timestamp"), QDateTime::currentDateTime().toString(Qt::ISODate)},
            {QLatin1String("success"), true}
        };
    } else if (action == QLatin1String("setup")) {
        QJsonObject alert_config = params.value(QLatin1String("config")).toObject();
        auto result = setup_alerts(alert_config);

        return QJsonObject{
            {QLatin1String("action"), QLatin1String("setup")},
            {QLatin1String("config"), alert_config},
            {QLatin1String("success"), result.has_value()},
            {QLatin1String("error"), result ? QLatin1String("") : QString::fromStdString(result.error().message)},
            {QLatin1String("timestamp"), QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    } else if (action == QLatin1String("clear")) {
        QMutexLocker lock(&m_alert_mutex);
        m_active_alerts.clear();

        return QJsonObject{
            {QLatin1String("action"), QLatin1String("clear")},
            {QLatin1String("success"), true},
            {QLatin1String("timestamp"), QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    } else {
        return QJsonObject{
            {QLatin1String("error"), QLatin1String("Invalid action. Supported: get, setup, clear")},
            {QLatin1String("success"), false}
        };
    }
}

QJsonObject MonitoringPlugin::handle_status_command(const QJsonObject& params) {
    Q_UNUSED(params)

    QJsonObject status;
    status[QLatin1String("plugin_name")] = QLatin1String("MonitoringPlugin");
    status[QLatin1String("state")] = static_cast<int>(m_state.load());
    status[QLatin1String("uptime_ms")] = static_cast<qint64>(uptime().count());
    status[QLatin1String("hot_reload_enabled")] = m_hot_reload_enabled;
    status[QLatin1String("metrics_collection_enabled")] = m_metrics_collection_enabled;
    status[QLatin1String("alerts_enabled")] = m_alerts_enabled;

    // Component status
    QJsonObject components;
    components[QLatin1String("hot_reload_manager")] = m_hot_reload_manager != nullptr;
    components[QLatin1String("metrics_collector")] = m_metrics_collector != nullptr;
    components[QLatin1String("file_watcher")] = m_file_watcher != nullptr;
    status[QLatin1String("components")] = components;

    // Statistics
    QJsonObject stats;
    stats[QLatin1String("monitoring_cycles")] = static_cast<qint64>(m_monitoring_cycles.load());
    stats[QLatin1String("metrics_collections")] = static_cast<qint64>(m_metrics_collections.load());
    stats[QLatin1String("file_changes_detected")] = static_cast<qint64>(m_file_changes_detected.load());
    stats[QLatin1String("reload_count")] = static_cast<qint64>(m_reload_count.load());
    stats[QLatin1String("alert_count")] = static_cast<qint64>(m_alert_count.load());
    status[QLatin1String("statistics")] = stats;

    // Current monitoring state
    QJsonObject monitoring_state;
    monitoring_state[QLatin1String("monitored_plugins")] = static_cast<int>(m_monitored_plugins.size());
    monitoring_state[QLatin1String("active_alerts")] = static_cast<int>(m_active_alerts.size());
    monitoring_state[QLatin1String("metrics_history_size")] = static_cast<int>(m_metrics_history.size());
    monitoring_state[QLatin1String("watched_files")] = m_file_watcher->files().size();
    monitoring_state[QLatin1String("watched_directories")] = m_file_watcher->directories().size();
    status[QLatin1String("monitoring_state")] = monitoring_state;

    status[QLatin1String("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
    status[QLatin1String("success")] = true;

    return status;
}

QJsonObject MonitoringPlugin::handle_history_command(const QJsonObject& params) {
    QJsonObject time_range = params.value(QLatin1String("time_range")).toObject();
    QString plugin_id = params.value(QLatin1String("plugin_id")).toString();

    auto history = get_historical_metrics(time_range, plugin_id);

    return QJsonObject{
        {QLatin1String("action"), QLatin1String("history")},
        {QLatin1String("history"), history},
        {QLatin1String("success"), true}
    };
}

// === Helper Methods ===

void MonitoringPlugin::log_error(const std::string& error) {
    {
        QMutexLocker lock(&m_error_mutex);
        m_last_error = error;
        m_error_log.push_back(error);

        // Maintain error log size
        if (m_error_log.size() > MAX_ERROR_LOG_SIZE) {
            m_error_log.erase(m_error_log.begin());
        }
    }

    m_error_count.fetch_add(1);
    qWarning() << "MonitoringPlugin Error:" << QString::fromStdString(error);
}

void MonitoringPlugin::log_info(const std::string& message) {
    qInfo() << "MonitoringPlugin:" << QString::fromStdString(message);
}

void MonitoringPlugin::update_metrics() {
    // Update internal metrics - could be expanded for more detailed monitoring
}

void MonitoringPlugin::initialize_monitoring_components() {
    // Initialize hot reload manager
    m_hot_reload_manager = std::make_unique<qtplugin::PluginHotReloadManager>();

    // Initialize metrics collector
    m_metrics_collector = std::make_unique<qtplugin::PluginMetricsCollector>();

    log_info("Monitoring components initialized");
}

void MonitoringPlugin::start_monitoring() {
    if (m_monitoring_timer) {
        m_monitoring_timer->setInterval(m_monitoring_interval);
        m_monitoring_timer->start();
    }

    if (m_metrics_timer && m_metrics_collection_enabled) {
        m_metrics_timer->setInterval(m_metrics_collection_interval);
        m_metrics_timer->start();
    }

    if (m_alert_timer && m_alerts_enabled) {
        m_alert_timer->setInterval(m_alert_check_interval);
        m_alert_timer->start();
    }

    log_info("Monitoring services started");
}

void MonitoringPlugin::stop_monitoring() {
    if (m_monitoring_timer && m_monitoring_timer->isActive()) {
        m_monitoring_timer->stop();
    }

    if (m_metrics_timer && m_metrics_timer->isActive()) {
        m_metrics_timer->stop();
    }

    if (m_alert_timer && m_alert_timer->isActive()) {
        m_alert_timer->stop();
    }

    log_info("Monitoring services stopped");
}

void MonitoringPlugin::collect_system_metrics() {
    if (!m_metrics_collection_enabled) {
        return;
    }

    // Create system-wide metric entry
    QJsonObject system_metrics;
    system_metrics[QLatin1String("type")] = QLatin1String("system");
    system_metrics[QLatin1String("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
    system_metrics[QLatin1String("uptime_ms")] = static_cast<qint64>(uptime().count());
    system_metrics[QLatin1String("monitored_plugins")] = static_cast<int>(m_monitored_plugins.size());
    system_metrics[QLatin1String("active_alerts")] = static_cast<int>(m_active_alerts.size());
    system_metrics[QLatin1String("file_changes")] = static_cast<qint64>(m_file_changes_detected.load());
    system_metrics[QLatin1String("reload_count")] = static_cast<qint64>(m_reload_count.load());

    // Add to metrics history
    {
        QMutexLocker lock(&m_metrics_mutex);
        m_metrics_history.push_back(system_metrics);
    }
}

void MonitoringPlugin::check_alerts() {
    if (!m_alerts_enabled) {
        return;
    }

    QMutexLocker lock(&m_alert_mutex);

    // Get current metrics for alert evaluation
    auto current_metrics = performance_metrics();

    // Check each alert condition
    for (auto it = m_alert_config.begin(); it != m_alert_config.end(); ++it) {
        QString condition_name = it.key();
        QJsonValue condition_value = it.value();

        if (condition_value.isObject()) {
            QJsonObject condition = condition_value.toObject();
            if (evaluate_alert_condition(condition, current_metrics)) {
                // Create alert
                QJsonObject alert;
                alert[QLatin1String("condition")] = condition_name;
                alert[QLatin1String("triggered_at")] = QDateTime::currentDateTime().toString(Qt::ISODate);
                alert[QLatin1String("metrics")] = current_metrics;
                alert[QLatin1String("severity")] = condition.value(QLatin1String("severity")).toString(QLatin1String("warning"));

                // Check if this alert is already active
                bool already_active = false;
                for (const auto& active_alert : m_active_alerts) {
                    if (active_alert.value(QLatin1String("condition")).toString() == condition_name) {
                        already_active = true;
                        break;
                    }
                }

                if (!already_active) {
                    m_active_alerts.push_back(alert);
                    m_alert_count.fetch_add(1);
                    log_error("Alert triggered: " + condition_name.toStdString());
                }
            }
        }
    }
}

void MonitoringPlugin::process_file_change(const QString& file_path) {
    // Find which plugin this file belongs to
    QMutexLocker lock(&m_hot_reload_mutex);

    for (auto it = m_monitored_plugins.begin(); it != m_monitored_plugins.end(); ++it) {
        if (it->second == file_path) {
            QString plugin_id = it->first;

            // Trigger hot reload if enabled
            if (m_hot_reload_enabled && m_hot_reload_manager) {
                // Hot reload manager uses callback mechanism, so we just log the file change
                // The actual reload will be handled by the callback set during initialization
                m_reload_count.fetch_add(1);
                m_last_reload_times[plugin_id] = std::chrono::system_clock::now();
                log_info("File change detected for plugin: " + plugin_id.toStdString());
            }
            break;
        }
    }
}

QJsonObject MonitoringPlugin::create_metric_entry(const QString& plugin_id, const QJsonObject& data) {
    QJsonObject entry;
    entry[QLatin1String("plugin_id")] = plugin_id;
    entry[QLatin1String("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
    entry[QLatin1String("data")] = data;
    entry[QLatin1String("source")] = QLatin1String("MonitoringPlugin");

    return entry;
}

void MonitoringPlugin::maintain_metrics_history() {
    QMutexLocker lock(&m_metrics_mutex);

    // Trim metrics history if it exceeds the configured size
    while (static_cast<int>(m_metrics_history.size()) > m_metrics_history_size) {
        m_metrics_history.erase(m_metrics_history.begin());
    }
}

bool MonitoringPlugin::evaluate_alert_condition(const QJsonObject& condition, const QJsonObject& metrics) {
    QString metric_name = condition.value(QLatin1String("metric")).toString();
    QString operator_str = condition.value(QLatin1String("operator")).toString();
    double threshold = condition.value(QLatin1String("threshold")).toDouble();

    if (metric_name.isEmpty() || operator_str.isEmpty()) {
        return false;
    }

    // Get the metric value
    QJsonValue metric_value = metrics.value(metric_name);
    if (!metric_value.isDouble()) {
        return false;
    }

    double value = metric_value.toDouble();

    // Evaluate condition
    if (operator_str == QLatin1String("greater_than") || operator_str == QLatin1String(">")) {
        return value > threshold;
    } else if (operator_str == QLatin1String("less_than") || operator_str == QLatin1String("<")) {
        return value < threshold;
    } else if (operator_str == QLatin1String("equals") || operator_str == QLatin1String("==")) {
        return qAbs(value - threshold) < 0.001; // Float comparison with tolerance
    } else if (operator_str == QLatin1String("greater_equal") || operator_str == QLatin1String(">=")) {
        return value >= threshold;
    } else if (operator_str == QLatin1String("less_equal") || operator_str == QLatin1String("<=")) {
        return value <= threshold;
    }

    return false;
}

// === IPlugin Basic Interface Implementation ===
std::string_view MonitoringPlugin::name() const noexcept {
    return "MonitoringPlugin";
}

std::string_view MonitoringPlugin::description() const noexcept {
    return "Advanced monitoring plugin with hot reload, metrics collection, and alerting";
}

qtplugin::Version MonitoringPlugin::version() const noexcept {
    return qtplugin::Version{3, 0, 0};
}

std::string_view MonitoringPlugin::author() const noexcept {
    return "QtForge Monitoring Team";
}

std::string MonitoringPlugin::id() const noexcept {
    return "qtforge.monitoring.plugin";
}

qtplugin::PluginState MonitoringPlugin::state() const noexcept {
    return m_state.load();
}

// === Plugin Factory ===

std::unique_ptr<MonitoringPlugin> MonitoringPlugin::create_instance() {
    return std::make_unique<MonitoringPlugin>();
}

qtplugin::PluginMetadata MonitoringPlugin::get_static_metadata() {
    qtplugin::PluginMetadata meta;
    meta.name = "MonitoringPlugin";
    meta.version = qtplugin::Version{3, 0, 0};
    meta.description = "Comprehensive monitoring plugin demonstrating QtForge monitoring features";
    meta.author = "QtForge Team";
    meta.license = "MIT";
    meta.category = "Monitoring";
    meta.tags = {"monitoring", "hot-reload", "metrics", "performance", "example"};
    return meta;
}

// #include "monitoring_plugin.moc" // Removed - not needed
