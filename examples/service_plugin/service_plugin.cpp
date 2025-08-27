/**
 * @file service_plugin.cpp
 * @brief Implementation of comprehensive service plugin
 * @version 3.0.0
 *
 * This implementation demonstrates advanced service patterns with
 * background processing, MessageBus integration, and service discovery.
 */

#include "service_plugin.hpp"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <chrono>
#include <thread>

// === ServiceWorker Implementation ===

ServiceWorker::ServiceWorker(QObject* parent) : QObject(parent) {
    qDebug() << "ServiceWorker created";
}

ServiceWorker::~ServiceWorker() {
    stop_processing();
    qDebug() << "ServiceWorker destroyed";
}

void ServiceWorker::process_task(const QJsonObject& task) {
    if (!m_running.load()) {
        emit task_failed("Worker is not running");
        return;
    }

    {
        std::lock_guard lock(m_queue_mutex);
        m_task_queue.push(task);
    }

    // Process task immediately for demonstration
    QJsonObject current_task;
    {
        std::lock_guard lock(m_queue_mutex);
        if (!m_task_queue.empty()) {
            current_task = m_task_queue.front();
            m_task_queue.pop();
        }
    }

    if (!current_task.isEmpty()) {
        try {
            // Simulate processing time
            QString task_type = current_task.value("type").toString("default");
            int processing_time =
                current_task.value("processing_time").toInt(1000);

            emit status_changed(QString("Processing task: %1").arg(task_type));

            // Process task based on type
            QJsonObject task_data = current_task.value("data").toObject();
            QJsonObject result_data;

            if (task_type == "data_processing") {
                // Simulate data processing
                QThread::msleep(processing_time);
                result_data = QJsonObject{
                    {"input_data", task_data},
                    {"processed_items", task_data.size()},
                    {"processing_algorithm", "advanced_algorithm_v2"},
                    {"output_size", task_data.size() * 2},
                    {"processed_at",
                     QDateTime::currentDateTime().toString(Qt::ISODate)},
                    {"worker_id", QString::number(reinterpret_cast<quintptr>(
                                      QThread::currentThreadId()))}};
            } else if (task_type == "calculation") {
                // Simulate mathematical calculation
                QThread::msleep(processing_time / 2);
                int input_value = task_data.value("input").toInt(0);
                result_data = QJsonObject{
                    {"input_value", input_value},
                    {"calculated_result", input_value * input_value + 42},
                    {"calculation_type", "quadratic_plus_constant"},
                    {"processed_at",
                     QDateTime::currentDateTime().toString(Qt::ISODate)},
                    {"worker_id", QString::number(reinterpret_cast<quintptr>(
                                      QThread::currentThreadId()))}};
            } else {
                // Default processing
                QThread::msleep(processing_time);
                result_data = QJsonObject{
                    {"message", "Default task processing completed"},
                    {"task_data", task_data},
                    {"processed_at",
                     QDateTime::currentDateTime().toString(Qt::ISODate)},
                    {"worker_id", QString::number(reinterpret_cast<quintptr>(
                                      QThread::currentThreadId()))}};
            }

            // Create comprehensive result
            QJsonObject result{
                {"task_id", current_task.value("id")},
                {"task_type", task_type},
                {"status", "completed"},
                {"submitted_at", current_task.value("submitted_at").toString()},
                {"processed_at",
                 QDateTime::currentDateTime().toString(Qt::ISODate)},
                {"processing_time_ms", processing_time},
                {"result_data", result_data}};

            m_processed_tasks.fetch_add(1);
            emit task_completed(result);
            emit status_changed("Task completed successfully");

        } catch (const std::exception& e) {
            m_failed_tasks.fetch_add(1);

            // Create error result
            QJsonObject error_result{
                {"task_id", current_task.value("id")},
                {"task_type", current_task.value("type").toString("unknown")},
                {"status", "failed"},
                {"error", e.what()},
                {"failed_at",
                 QDateTime::currentDateTime().toString(Qt::ISODate)},
                {"worker_id", QString::number(reinterpret_cast<quintptr>(
                                  QThread::currentThreadId()))}};

            emit task_completed(
                error_result);  // Still emit completion for tracking
            emit task_failed(
                QString("Task processing failed: %1").arg(e.what()));
        } catch (...) {
            m_failed_tasks.fetch_add(1);

            // Create unknown error result
            QJsonObject error_result{
                {"task_id", current_task.value("id")},
                {"task_type", current_task.value("type").toString("unknown")},
                {"status", "failed"},
                {"error", "Unknown error occurred"},
                {"failed_at",
                 QDateTime::currentDateTime().toString(Qt::ISODate)},
                {"worker_id", QString::number(reinterpret_cast<quintptr>(
                                  QThread::currentThreadId()))}};

            emit task_completed(error_result);
            emit task_failed("Task processing failed: Unknown error");
        }
    }
}

void ServiceWorker::start_processing() {
    m_running.store(true);
    emit status_changed("Worker started");
    qDebug() << "ServiceWorker started processing";
}

void ServiceWorker::stop_processing() {
    m_running.store(false);
    emit status_changed("Worker stopped");
    qDebug() << "ServiceWorker stopped processing";
}

// === ServicePlugin Implementation ===

ServicePlugin::ServicePlugin(QObject* parent)
    : QObject(parent),
      m_processing_timer(std::make_unique<QTimer>(this)),
      m_heartbeat_timer(std::make_unique<QTimer>(this)),
      m_worker_thread(std::make_unique<QThread>(this)),
      m_worker(std::make_unique<ServiceWorker>()) {
    // Initialize dependencies
    m_required_dependencies = {"qtplugin.MessageBus"};
    m_optional_dependencies = {"qtplugin.ConfigurationManager",
                               "qtplugin.PluginServiceDiscovery"};

    // Setup timers
    connect(m_processing_timer.get(), &QTimer::timeout, this,
            &ServicePlugin::on_processing_timer_timeout);
    connect(m_heartbeat_timer.get(), &QTimer::timeout, this,
            &ServicePlugin::on_heartbeat_timer_timeout);

    // Setup worker thread
    m_worker->moveToThread(m_worker_thread.get());
    connect(m_worker.get(), &ServiceWorker::task_completed, this,
            &ServicePlugin::on_task_completed);
    connect(m_worker.get(), &ServiceWorker::task_failed, this,
            &ServicePlugin::on_task_failed);
    connect(m_worker.get(), &ServiceWorker::status_changed, this,
            &ServicePlugin::on_worker_status_changed);

    log_info("ServicePlugin constructed");
}

ServicePlugin::~ServicePlugin() {
    shutdown();
    log_info("ServicePlugin destroyed");
}

qtplugin::expected<void, qtplugin::PluginError> ServicePlugin::initialize() {
    std::unique_lock lock(m_state_mutex);

    if (m_state != qtplugin::PluginState::Unloaded) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin is already initialized");
    }

    try {
        m_state = qtplugin::PluginState::Initializing;
        m_initialization_time = std::chrono::system_clock::now();
        lock.unlock();

        log_info("Initializing ServicePlugin...");

        // Initialize MessageBus (would be injected in real implementation)
        m_message_bus = new qtplugin::MessageBus(this);
        if (!m_message_bus) {
            throw std::runtime_error("Failed to create MessageBus");
        }

        // Setup message subscriptions
        setup_message_subscriptions();

        // Start worker thread
        m_worker_thread->start();
        QMetaObject::invokeMethod(m_worker.get(), "start_processing",
                                  Qt::QueuedConnection);

        // Start timers with default intervals
        m_processing_timer->setInterval(m_processing_interval);
        m_processing_timer->start();

        m_heartbeat_timer->setInterval(m_heartbeat_interval);
        m_heartbeat_timer->start();

        // Register service
        auto register_result = register_service();
        if (!register_result) {
            log_error("Failed to register service: " +
                      register_result.error().message);
            // Continue initialization even if service registration fails
        }

        lock.lock();
        m_state = qtplugin::PluginState::Running;
        lock.unlock();

        log_info("ServicePlugin initialized successfully");
        publish_status_update("initialized");

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to initialize ServicePlugin: " + std::string(e.what());
        log_error(error_msg);

        lock.lock();
        m_state = qtplugin::PluginState::Error;
        lock.unlock();

        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed, error_msg);
    }
}

void ServicePlugin::shutdown() noexcept {
    try {
        std::unique_lock lock(m_state_mutex);
        m_state = qtplugin::PluginState::Stopping;
        lock.unlock();

        log_info("Shutting down ServicePlugin...");

        // Stop timers
        if (m_processing_timer && m_processing_timer->isActive()) {
            m_processing_timer->stop();
        }
        if (m_heartbeat_timer && m_heartbeat_timer->isActive()) {
            m_heartbeat_timer->stop();
        }

        // Stop worker
        if (m_worker) {
            QMetaObject::invokeMethod(m_worker.get(), "stop_processing",
                                      Qt::QueuedConnection);
        }

        // Stop worker thread
        if (m_worker_thread && m_worker_thread->isRunning()) {
            m_worker_thread->quit();
            if (!m_worker_thread->wait(5000)) {
                m_worker_thread->terminate();
                m_worker_thread->wait(1000);
            }
        }

        // Unregister service
        unregister_service();

        // Publish shutdown status
        publish_status_update("shutdown");

        lock.lock();
        m_state = qtplugin::PluginState::Stopped;
        lock.unlock();

        log_info("ServicePlugin shutdown completed");

    } catch (...) {
        // Shutdown must not throw
        std::unique_lock lock(m_state_mutex);
        m_state = qtplugin::PluginState::Error;
    }
}

bool ServicePlugin::is_initialized() const noexcept {
    std::shared_lock lock(m_state_mutex);
    auto current_state = m_state.load();
    return current_state == qtplugin::PluginState::Running ||
           current_state == qtplugin::PluginState::Paused;
}

qtplugin::expected<void, qtplugin::PluginError> ServicePlugin::pause() {
    std::unique_lock lock(m_state_mutex);

    if (m_state != qtplugin::PluginState::Running) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be running to pause");
    }

    try {
        // Pause timers
        if (m_processing_timer && m_processing_timer->isActive()) {
            m_processing_timer->stop();
        }
        if (m_heartbeat_timer && m_heartbeat_timer->isActive()) {
            m_heartbeat_timer->stop();
        }

        m_state = qtplugin::PluginState::Paused;
        log_info("ServicePlugin paused successfully");
        publish_status_update("paused");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to pause ServicePlugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> ServicePlugin::resume() {
    std::unique_lock lock(m_state_mutex);

    if (m_state != qtplugin::PluginState::Paused) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be paused to resume");
    }

    try {
        // Resume timers
        if (m_processing_timer) {
            m_processing_timer->setInterval(m_processing_interval);
            m_processing_timer->start();
        }
        if (m_heartbeat_timer) {
            m_heartbeat_timer->setInterval(m_heartbeat_interval);
            m_heartbeat_timer->start();
        }

        m_state = qtplugin::PluginState::Running;
        log_info("ServicePlugin resumed successfully");
        publish_status_update("resumed");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to resume ServicePlugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> ServicePlugin::restart() {
    log_info("Restarting ServicePlugin...");

    // Shutdown first
    shutdown();

    // Wait a brief moment for cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Initialize again
    return initialize();
}

// === Configuration ===

std::optional<QJsonObject> ServicePlugin::default_configuration() const {
    return QJsonObject{
        {"processing_interval", 5000}, {"heartbeat_interval", 30000},
        {"logging_enabled", true},     {"service_name", "ExampleService"},
        {"max_concurrent_tasks", 10},  {"auto_register_service", true},
        {"message_bus_enabled", true}};
}

qtplugin::expected<void, qtplugin::PluginError> ServicePlugin::configure(
    const QJsonObject& config) {
    if (!validate_configuration(config)) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError,
            "Invalid configuration provided");
    }

    try {
        std::lock_guard lock(m_config_mutex);

        // Update configuration
        m_configuration = config;

        // Apply configuration
        m_processing_interval = config.value("processing_interval").toInt(5000);
        m_heartbeat_interval = config.value("heartbeat_interval").toInt(30000);
        m_logging_enabled = config.value("logging_enabled").toBool(true);
        m_service_name =
            config.value("service_name").toString("ExampleService");
        m_max_concurrent_tasks = config.value("max_concurrent_tasks").toInt(10);

        // Update timers if running
        if (m_processing_timer && m_processing_timer->isActive()) {
            m_processing_timer->setInterval(m_processing_interval);
        }
        if (m_heartbeat_timer && m_heartbeat_timer->isActive()) {
            m_heartbeat_timer->setInterval(m_heartbeat_interval);
        }

        log_info("ServicePlugin configured successfully");
        publish_status_update("configured", config);

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to configure ServicePlugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError, error_msg);
    }
}

QJsonObject ServicePlugin::current_configuration() const {
    std::lock_guard lock(m_config_mutex);
    return m_configuration;
}

bool ServicePlugin::validate_configuration(const QJsonObject& config) const {
    // Validate processing interval
    if (config.contains("processing_interval")) {
        int interval = config.value("processing_interval").toInt(-1);
        if (interval < 1000 || interval > 300000) {  // 1 second to 5 minutes
            return false;
        }
    }

    // Validate heartbeat interval
    if (config.contains("heartbeat_interval")) {
        int interval = config.value("heartbeat_interval").toInt(-1);
        if (interval < 5000 || interval > 600000) {  // 5 seconds to 10 minutes
            return false;
        }
    }

    // Validate max concurrent tasks
    if (config.contains("max_concurrent_tasks")) {
        int max_tasks = config.value("max_concurrent_tasks").toInt(-1);
        if (max_tasks < 1 || max_tasks > 100) {
            return false;
        }
    }

    // Validate service name
    if (config.contains("service_name")) {
        QString name = config.value("service_name").toString();
        if (name.isEmpty() || name.length() > 100) {
            return false;
        }
    }

    return true;
}

// === Commands ===

qtplugin::expected<QJsonObject, qtplugin::PluginError>
ServicePlugin::execute_command(std::string_view command,
                               const QJsonObject& params) {
    m_command_count.fetch_add(1);

    if (command == "status") {
        return handle_status_command(params);
    } else if (command == "service") {
        return handle_service_command(params);
    } else if (command == "task") {
        return handle_task_command(params);
    } else if (command == "message") {
        return handle_message_command(params);
    } else if (command == "monitoring") {
        return handle_monitoring_command(params);
    } else {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound,
            "Unknown command: " + std::string(command));
    }
}

std::vector<std::string> ServicePlugin::available_commands() const {
    return {"status", "service", "task", "message", "monitoring"};
}

// === Dependencies ===

std::vector<std::string> ServicePlugin::dependencies() const {
    return m_required_dependencies;
}

std::vector<std::string> ServicePlugin::optional_dependencies() const {
    return m_optional_dependencies;
}

bool ServicePlugin::dependencies_satisfied() const {
    return m_dependencies_satisfied.load();
}

// === Error Handling ===

void ServicePlugin::clear_errors() {
    std::lock_guard lock(m_error_mutex);
    m_error_log.clear();
    m_last_error.clear();
}

// === Monitoring ===

std::chrono::milliseconds ServicePlugin::uptime() const {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_initialization_time);
    return duration;
}

QJsonObject ServicePlugin::performance_metrics() const {
    std::lock_guard lock(m_metrics_mutex);

    auto current_uptime = uptime();
    auto commands_per_second =
        current_uptime.count() > 0
            ? (m_command_count.load() * 1000.0) / current_uptime.count()
            : 0.0;

    return QJsonObject{
        {"uptime_ms", static_cast<qint64>(current_uptime.count())},
        {"command_count", static_cast<qint64>(m_command_count.load())},
        {"message_count", static_cast<qint64>(m_message_count.load())},
        {"error_count", static_cast<qint64>(m_error_count.load())},
        {"tasks_processed", static_cast<qint64>(m_tasks_processed.load())},
        {"tasks_failed", static_cast<qint64>(m_tasks_failed.load())},
        {"commands_per_second", commands_per_second},
        {"state", static_cast<int>(m_state.load())},
        {"worker_thread_running",
         m_worker_thread && m_worker_thread->isRunning()},
        {"processing_timer_active",
         m_processing_timer && m_processing_timer->isActive()},
        {"heartbeat_timer_active",
         m_heartbeat_timer && m_heartbeat_timer->isActive()},
        {"service_registered", !m_registered_service_id.isEmpty()}};
}

QJsonObject ServicePlugin::resource_usage() const {
    std::lock_guard lock(m_metrics_mutex);

    // Enhanced resource usage tracking
    auto memory_estimate =
        1024 + (m_error_log.size() * 50);  // Base + error log
    auto cpu_estimate = 0.5;  // Service plugins typically use more CPU
    if (m_processing_timer && m_processing_timer->isActive())
        cpu_estimate += 0.3;
    if (m_heartbeat_timer && m_heartbeat_timer->isActive())
        cpu_estimate += 0.1;
    if (m_worker_thread && m_worker_thread->isRunning())
        cpu_estimate += 0.5;

    QJsonObject result;
    result["estimated_memory_kb"] = static_cast<qint64>(memory_estimate);
    result["estimated_cpu_percent"] = cpu_estimate;
    result["thread_count"] = m_worker_thread && m_worker_thread->isRunning() ? 2 : 1;
    result["timer_count"] = 2;
    result["active_timers"] = (m_processing_timer && m_processing_timer->isActive() ? 1 : 0) +
                              (m_heartbeat_timer && m_heartbeat_timer->isActive() ? 1 : 0);
    result["message_bus_connected"] = m_message_bus != nullptr;
    result["service_discovery_connected"] = m_service_discovery != nullptr;
    result["error_log_size"] = static_cast<qint64>(m_error_log.size());
    result["dependencies_satisfied"] = dependencies_satisfied();
    return result;
}

// === Slot Handlers ===

void ServicePlugin::on_processing_timer_timeout() {
    try {
        // Perform periodic processing tasks
        update_metrics();

        // Create a sample task for demonstration
        QJsonObject sample_task{
            {"id", QString("task_%1").arg(QDateTime::currentMSecsSinceEpoch())},
            {"type", "periodic_processing"},
            {"processing_time", 500},
            {"data",
             QJsonObject{{"timestamp", QDateTime::currentDateTime().toString(
                                           Qt::ISODate)}}}};

        // Send task to worker
        QMetaObject::invokeMethod(m_worker.get(), "process_task",
                                  Qt::QueuedConnection,
                                  Q_ARG(QJsonObject, sample_task));

        log_info("Periodic processing task queued");

    } catch (const std::exception& e) {
        log_error("Processing timer error: " + std::string(e.what()));
    }
}

void ServicePlugin::on_heartbeat_timer_timeout() {
    try {
        // Send heartbeat status
        QJsonObject heartbeat_data{
            {"uptime_ms", static_cast<qint64>(uptime().count())},
            {"state", static_cast<int>(m_state.load())},
            {"tasks_processed", static_cast<qint64>(m_tasks_processed.load())},
            {"worker_active", m_worker_thread && m_worker_thread->isRunning()}};

        publish_status_update("heartbeat", heartbeat_data);
        log_info("Heartbeat sent");

    } catch (const std::exception& e) {
        log_error("Heartbeat timer error: " + std::string(e.what()));
    }
}

void ServicePlugin::on_task_completed(const QJsonObject& result) {
    m_tasks_processed.fetch_add(1);

    // Publish task completion status
    publish_status_update("task_completed", result);

    log_info("Task completed: " +
             result.value("task_id").toString().toStdString());
}

void ServicePlugin::on_task_failed(const QString& error) {
    m_tasks_failed.fetch_add(1);

    // Publish task failure status
    QJsonObject failure_data{
        {"error", error},
        {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}};

    publish_status_update("task_failed", failure_data);

    log_error("Task failed: " + error.toStdString());
}

void ServicePlugin::on_worker_status_changed(const QString& status) {
    log_info("Worker status: " + status.toStdString());

    QJsonObject status_data{
        {"worker_status", status},
        {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}};

    publish_status_update("worker_status", status_data);
}

void ServicePlugin::on_service_request_received(
    std::shared_ptr<ServiceRequestMessage> message) {
    m_message_count.fetch_add(1);

    try {
        log_info("Service request received: " +
                 message->operation().toStdString());

        // Handle different service operations
        QString operation = message->operation();
        QJsonObject params = message->params();
        QJsonObject response;

        if (operation == "get_status") {
            response = QJsonObject{{"operation", "get_status"},
                                   {"status", "success"},
                                   {"data", performance_metrics()}};
        } else if (operation == "submit_task") {
            // Submit task to worker
            QJsonObject task = params.value("task").toObject();
            if (!task.isEmpty()) {
                QMetaObject::invokeMethod(m_worker.get(), "process_task",
                                          Qt::QueuedConnection,
                                          Q_ARG(QJsonObject, task));
                response = QJsonObject{{"operation", "submit_task"},
                                       {"status", "accepted"},
                                       {"task_id", task.value("id")}};
            } else {
                response = QJsonObject{{"operation", "submit_task"},
                                       {"status", "error"},
                                       {"error", "Invalid task data"}};
            }
        } else {
            response = QJsonObject{{"operation", operation},
                                   {"status", "error"},
                                   {"error", "Unknown operation"}};
        }

        // Send response (in a real implementation, this would use
        // request-response system)
        publish_status_update("service_response", response);

    } catch (const std::exception& e) {
        log_error("Service request handling error: " + std::string(e.what()));
    }
}

// === Helper Methods ===

void ServicePlugin::log_error(const std::string& error) {
    {
        std::lock_guard lock(m_error_mutex);
        m_last_error = error;
        m_error_log.push_back(error);

        // Maintain error log size
        if (m_error_log.size() > MAX_ERROR_LOG_SIZE) {
            m_error_log.erase(m_error_log.begin());
        }
    }

    m_error_count.fetch_add(1);

    if (m_logging_enabled) {
        qWarning() << "ServicePlugin Error:" << QString::fromStdString(error);
    }
}

void ServicePlugin::log_info(const std::string& message) {
    if (m_logging_enabled) {
        qInfo() << "ServicePlugin:" << QString::fromStdString(message);
    }
}

void ServicePlugin::update_metrics() {
    // Update internal metrics
    // This could include collecting system metrics, updating counters, etc.
    // For demonstration, we'll just log the current state

    if (m_logging_enabled) {
        auto metrics = performance_metrics();
        qDebug() << "ServicePlugin metrics updated:"
                 << "Commands:" << metrics.value("command_count").toInt()
                 << "Tasks:" << metrics.value("tasks_processed").toInt()
                 << "Errors:" << metrics.value("error_count").toInt();
    }
}

// === Service Management ===

qtplugin::expected<void, qtplugin::PluginError>
ServicePlugin::register_service() {
    try {
        // Create service registration
        qtplugin::ServiceRegistration registration;
        registration.service_id =
            QString("service_%1").arg(QDateTime::currentMSecsSinceEpoch());
        registration.plugin_id = QString::fromStdString(id());
        registration.service_name = m_service_name;
        registration.service_version = "1.0.0";
        registration.description =
            "Example service plugin demonstrating background processing";
        registration.tags = QStringList{"example", "service", "background"};
        registration.categories = QStringList{"processing", "messaging"};
        registration.availability = qtplugin::ServiceAvailability::Available;
        registration.registration_time = std::chrono::system_clock::now();
        registration.last_heartbeat = std::chrono::system_clock::now();

        // Add endpoints
        registration.endpoints = QJsonObject{{"status", "/service/status"},
                                             {"submit_task", "/service/task"},
                                             {"metrics", "/service/metrics"}};

        // Add configuration
        registration.configuration = current_configuration();

        // Store registration ID
        m_registered_service_id = registration.service_id;

        log_info("Service registered with ID: " +
                 registration.service_id.toStdString());

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to register service: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError>
ServicePlugin::unregister_service() {
    try {
        if (!m_registered_service_id.isEmpty()) {
            log_info("Service unregistered: " +
                     m_registered_service_id.toStdString());
            m_registered_service_id.clear();
        }

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to unregister service: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

void ServicePlugin::publish_status_update(const QString& status,
                                          const QJsonObject& data) {
    if (!m_message_bus) {
        return;
    }

    try {
        auto message = std::make_shared<ServiceStatusMessage>(
            id(), m_service_name, status, data);

        // Publish message (in a real implementation, this would use the actual
        // MessageBus)
        log_info("Status update published: " + status.toStdString());

    } catch (const std::exception& e) {
        log_error("Failed to publish status update: " + std::string(e.what()));
    }
}

void ServicePlugin::setup_message_subscriptions() {
    if (!m_message_bus) {
        return;
    }

    try {
        // Subscribe to service request messages
        // In a real implementation, this would use the actual MessageBus
        // subscription API
        log_info("Message subscriptions set up");

    } catch (const std::exception& e) {
        log_error("Failed to setup message subscriptions: " +
                  std::string(e.what()));
    }
}

// === Command Handlers ===

QJsonObject ServicePlugin::handle_status_command(const QJsonObject& params) {
    Q_UNUSED(params)

    return QJsonObject{
        {"plugin_name", QString::fromStdString(std::string(name()))},
        {"plugin_id", QString::fromStdString(id())},
        {"state", static_cast<int>(m_state.load())},
        {"uptime_ms", static_cast<qint64>(uptime().count())},
        {"service_name", m_service_name},
        {"service_registered", !m_registered_service_id.isEmpty()},
        {"worker_running", m_worker_thread && m_worker_thread->isRunning()},
        {"processing_active",
         m_processing_timer && m_processing_timer->isActive()},
        {"heartbeat_active",
         m_heartbeat_timer && m_heartbeat_timer->isActive()},
        {"message_bus_connected", m_message_bus != nullptr},
        {"dependencies_satisfied", dependencies_satisfied()}};
}

QJsonObject ServicePlugin::handle_service_command(const QJsonObject& params) {
    QString action = params.value("action").toString();

    if (action == "register") {
        auto result = register_service();
        return QJsonObject{
            {"action", "register"},
            {"success", result.has_value()},
            {"error",
             result ? "" : QString::fromStdString(result.error().message)},
            {"service_id", m_registered_service_id}};
    } else if (action == "unregister") {
        auto result = unregister_service();
        return QJsonObject{
            {"action", "unregister"},
            {"success", result.has_value()},
            {"error",
             result ? "" : QString::fromStdString(result.error().message)}};
    } else if (action == "info") {
        return QJsonObject{
            {"action", "info"},
            {"service_name", m_service_name},
            {"service_id", m_registered_service_id},
            {"registration_status", !m_registered_service_id.isEmpty()
                                        ? "registered"
                                        : "not_registered"},
            {"endpoints", QJsonObject{{"status", "/service/status"},
                                      {"submit_task", "/service/task"},
                                      {"metrics", "/service/metrics"}}}};
    } else {
        return QJsonObject{
            {"error", "Invalid action. Supported: register, unregister, info"},
            {"success", false}};
    }
}

QJsonObject ServicePlugin::handle_task_command(const QJsonObject& params) {
    QString action = params.value("action").toString();

    if (action == "submit") {
        QJsonObject task_data = params.value("task").toObject();
        if (task_data.isEmpty()) {
            return QJsonObject{{"action", "submit"},
                               {"success", false},
                               {"error", "Task data is required"}};
        }

        // Add task ID if not present
        if (!task_data.contains("id")) {
            task_data["id"] =
                QString("task_%1").arg(QDateTime::currentMSecsSinceEpoch());
        }

        // Add submission timestamp
        task_data["submitted_at"] =
            QDateTime::currentDateTime().toString(Qt::ISODate);

        // Add default processing time if not specified
        if (!task_data.contains("processing_time")) {
            task_data["processing_time"] = 1000;  // Default 1 second
        }

        // Add default type if not specified
        if (!task_data.contains("type")) {
            task_data["type"] = "default";
        }

        // Submit task to worker
        QMetaObject::invokeMethod(m_worker.get(), "process_task",
                                  Qt::QueuedConnection,
                                  Q_ARG(QJsonObject, task_data));

        m_tasks_submitted.fetch_add(1);

        return QJsonObject{{"action", "submit"},
                           {"success", true},
                           {"task_id", task_data.value("id")},
                           {"task_type", task_data.value("type")},
                           {"status", "queued"},
                           {"submitted_at", task_data.value("submitted_at")},
                           {"estimated_processing_time_ms",
                            task_data.value("processing_time")}};

    } else if (action == "stats") {
        return QJsonObject{
            {"action", "stats"},
            {"tasks_processed", static_cast<qint64>(m_tasks_processed.load())},
            {"tasks_failed", static_cast<qint64>(m_tasks_failed.load())},
            {"worker_running", m_worker_thread && m_worker_thread->isRunning()},
            {"success_rate",
             m_tasks_processed.load() > 0
                 ? (double)(m_tasks_processed.load() - m_tasks_failed.load()) /
                       m_tasks_processed.load() * 100.0
                 : 0.0}};

    } else {
        return QJsonObject{
            {"error", "Invalid action. Supported: submit, stats"},
            {"success", false}};
    }
}

QJsonObject ServicePlugin::handle_message_command(const QJsonObject& params) {
    QString action = params.value("action").toString();

    if (action == "publish") {
        QString status = params.value("status").toString("custom_status");
        QJsonObject data = params.value("data").toObject();

        publish_status_update(status, data);

        return QJsonObject{
            {"action", "publish"},
            {"success", true},
            {"status", status},
            {"message_count", static_cast<qint64>(m_message_count.load())}};

    } else if (action == "stats") {
        return QJsonObject{
            {"action", "stats"},
            {"message_count", static_cast<qint64>(m_message_count.load())},
            {"message_bus_connected", m_message_bus != nullptr},
            {"subscriptions_active", true}  // Simplified for demonstration
        };

    } else {
        return QJsonObject{
            {"error", "Invalid action. Supported: publish, stats"},
            {"success", false}};
    }
}

QJsonObject ServicePlugin::handle_monitoring_command(
    const QJsonObject& params) {
    QString type = params.value("type").toString("all");

    if (type == "performance") {
        return QJsonObject{{"type", "performance"},
                           {"data", performance_metrics()}};
    } else if (type == "resources") {
        return QJsonObject{{"type", "resources"}, {"data", resource_usage()}};
    } else if (type == "service") {
        return QJsonObject{
            {"type", "service"},
            {"data",
             QJsonObject{
                 {"service_name", m_service_name},
                 {"service_id", m_registered_service_id},
                 {"registration_status", !m_registered_service_id.isEmpty()},
                 {"worker_status",
                  m_worker_thread && m_worker_thread->isRunning() ? "running"
                                                                  : "stopped"},
                 {"processing_interval", m_processing_interval},
                 {"heartbeat_interval", m_heartbeat_interval}}}};
    } else if (type == "all") {
        return QJsonObject{
            {"type", "all"},
            {"performance", performance_metrics()},
            {"resources", resource_usage()},
            {"service_info",
             QJsonObject{{"service_name", m_service_name},
                         {"service_id", m_registered_service_id},
                         {"worker_running",
                          m_worker_thread && m_worker_thread->isRunning()}}}};
    } else {
        return QJsonObject{
            {"error",
             "Invalid type. Supported: performance, resources, service, all"},
            {"success", false}};
    }
}
