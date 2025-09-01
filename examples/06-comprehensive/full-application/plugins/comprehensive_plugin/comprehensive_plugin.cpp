/**
 * @file comprehensive_plugin.cpp
 * @brief Implementation of comprehensive plugin demonstrating ALL QtForge
 * features
 */

#include "comprehensive_plugin.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <algorithm>
#include <chrono>

Q_LOGGING_CATEGORY(comprehensivePlugin, "qtforge.plugin.comprehensive")

ComprehensivePlugin::ComprehensivePlugin(QObject* parent)
    : QObject(parent),
      m_startTime(QDateTime::currentDateTime()),
      m_metricsTimer(new QTimer(this)),
      m_healthTimer(new QTimer(this)),
      m_backgroundTimer(new QTimer(this)) {
    qCDebug(comprehensivePlugin) << "ComprehensivePlugin constructor";

    // Setup timers
    connect(m_metricsTimer, &QTimer::timeout, this,
            &ComprehensivePlugin::onMetricsCollection);
    connect(m_healthTimer, &QTimer::timeout, this,
            &ComprehensivePlugin::onHealthCheck);
    connect(m_backgroundTimer, &QTimer::timeout, this,
            &ComprehensivePlugin::onBackgroundTask);

    m_metricsTimer->setInterval(DEFAULT_METRICS_INTERVAL);
    m_healthTimer->setInterval(DEFAULT_HEALTH_CHECK_INTERVAL);
    m_backgroundTimer->setInterval(DEFAULT_BACKGROUND_INTERVAL);
}

ComprehensivePlugin::~ComprehensivePlugin() {
    qCDebug(comprehensivePlugin) << "ComprehensivePlugin destructor";
    shutdown();
}

std::string_view ComprehensivePlugin::name() const noexcept {
    return "Comprehensive Demo Plugin";
}

std::string_view ComprehensivePlugin::description() const noexcept {
    return "A comprehensive plugin demonstrating all QtForge features "
           "including "
           "communication, security, monitoring, transactions, workflows, and "
           "more.";
}

qtplugin::Version ComprehensivePlugin::version() const noexcept {
    return {3, 0, 0};
}

std::string_view ComprehensivePlugin::author() const noexcept {
    return "QtForge Development Team";
}

std::string ComprehensivePlugin::id() const noexcept {
    return "com.qtforge.comprehensive_plugin";
}

qtplugin::expected<void, qtplugin::PluginError>
ComprehensivePlugin::initialize() {
    qCInfo(comprehensivePlugin) << "Initializing comprehensive plugin...";

    try {
        m_state = qtplugin::PluginState::Loaded;

        // Initialize all subsystems
        setupCommunication();
        setupMonitoring();
        setupSecurity();
        setupNetworking();
        setupBackgroundProcessing();

        // Load default configuration
        QJsonObject defaultConfig;
        defaultConfig["communication_enabled"] = true;
        defaultConfig["monitoring_enabled"] = true;
        defaultConfig["security_enabled"] = true;
        defaultConfig["networking_enabled"] = true;
        defaultConfig["background_processing_enabled"] = true;
        defaultConfig["metrics_interval"] = DEFAULT_METRICS_INTERVAL;
        defaultConfig["health_check_interval"] = DEFAULT_HEALTH_CHECK_INTERVAL;

        auto configResult = configure(defaultConfig);
        if (!configResult.has_value()) {
            return qtplugin::make_unexpected(configResult.error());
        }

        m_state = qtplugin::PluginState::Initialized;
        emit pluginStateChanged(m_state);

        qCInfo(comprehensivePlugin)
            << "✅ Comprehensive plugin initialized successfully";
        return {};

    } catch (const std::exception& e) {
        m_state = qtplugin::PluginState::Error;
        emit pluginStateChanged(m_state);

        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::InitializationFailed,
            std::string("Initialization failed: ") + e.what()});
    }
}

void ComprehensivePlugin::shutdown() noexcept {
    qCInfo(comprehensivePlugin) << "Shutting down comprehensive plugin...";

    try {
        // Stop all timers
        m_metricsTimer->stop();
        m_healthTimer->stop();
        m_backgroundTimer->stop();

        // Stop service if running
        if (m_serviceStatus == qtplugin::ServiceStatus::Running) {
            stop_service();
        }

        // Stop background processing
        m_backgroundProcessingEnabled = false;
        if (m_backgroundThread && m_backgroundThread->isRunning()) {
            m_backgroundThread->quit();
            m_backgroundThread->wait(5000);
        }

        // Cleanup components
        m_messageBus.reset();
        m_requestResponse.reset();
        m_metricsCollector.reset();
        m_securityManager.reset();
        m_networkManager.reset();

        m_state = qtplugin::PluginState::Unloaded;
        emit pluginStateChanged(m_state);

        qCInfo(comprehensivePlugin)
            << "✅ Comprehensive plugin shutdown completed";

    } catch (const std::exception& e) {
        qCWarning(comprehensivePlugin) << "Error during shutdown:" << e.what();
        m_state = qtplugin::PluginState::Error;
    }
}

qtplugin::PluginState ComprehensivePlugin::state() const noexcept {
    return m_state;
}

qtplugin::PluginCapabilities ComprehensivePlugin::capabilities()
    const noexcept {
    return qtplugin::PluginCapability::Service |
           qtplugin::PluginCapability::Network |
           qtplugin::PluginCapability::DataProcessor |
           qtplugin::PluginCapability::Scripting;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
ComprehensivePlugin::execute_command(std::string_view command,
                                     const QJsonObject& params) {
    qCDebug(comprehensivePlugin)
        << "Executing command:" << command.data() << "with params:" << params;

    startPerformanceTimer(std::string(command));
    m_commandsExecuted++;

    try {
        QJsonObject result;

        if (command == "status") {
            result = handleStatusCommand(params);
        } else if (command == "echo") {
            result = handleEchoCommand(params);
        } else if (command == "process_data") {
            result = handleProcessDataCommand(params);
        } else if (command == "network_request") {
            result = handleNetworkRequestCommand(params);
        } else if (command == "metrics") {
            result = handleMetricsCommand(params);
        } else if (command == "config") {
            result = handleConfigCommand(params);
        } else if (command == "security") {
            result = handleSecurityCommand(params);
        } else if (command == "transaction") {
            result = handleTransactionCommand(params);
        } else if (command == "workflow") {
            result = handleWorkflowCommand(params);
        } else if (command == "python") {
            result = handlePythonCommand(params);
        } else {
            endPerformanceTimer(std::string(command));
            m_errorsEncountered++;
            return qtplugin::make_unexpected(qtplugin::PluginError{
                qtplugin::PluginErrorCode::InvalidCommand,
                std::string("Unknown command: ") + std::string(command)});
        }

        endPerformanceTimer(std::string(command));

        // Publish command execution event
        QJsonObject eventData;
        eventData["command"] = QString::fromStdString(std::string(command));
        eventData["params"] = params;
        eventData["result"] = result;
        eventData["timestamp"] =
            QDateTime::currentDateTime().toString(Qt::ISODate);
        publishEvent("command.executed", eventData);

        return result;

    } catch (const std::exception& e) {
        endPerformanceTimer(std::string(command));
        m_errorsEncountered++;

        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::ExecutionFailed,
            std::string("Command execution failed: ") + e.what()});
    }
}

std::vector<std::string> ComprehensivePlugin::available_commands() const {
    return {
        "status",           // Get plugin status and health information
        "echo",             // Echo input parameters for testing
        "process_data",     // Process data with various algorithms
        "network_request",  // Make network requests
        "metrics",          // Get performance metrics
        "config",           // Get/set configuration
        "security",         // Security operations
        "transaction",      // Transaction operations
        "workflow",         // Workflow operations
        "python"            // Python integration operations
    };
}

qtplugin::expected<void, qtplugin::PluginError> ComprehensivePlugin::configure(
    const QJsonObject& config) {
    qCDebug(comprehensivePlugin) << "Configuring plugin with:" << config;

    QMutexLocker locker(&m_configMutex);

    try {
        // Validate configuration
        if (config.contains("metrics_interval")) {
            int interval = config["metrics_interval"].toInt();
            if (interval < 1000 || interval > 60000) {
                return qtplugin::make_unexpected(qtplugin::PluginError{
                    qtplugin::PluginErrorCode::ConfigurationError,
                    "metrics_interval must be between 1000 and 60000 ms"});
            }
            m_metricsTimer->setInterval(interval);
        }

        if (config.contains("health_check_interval")) {
            int interval = config["health_check_interval"].toInt();
            if (interval < 5000 || interval > 300000) {
                return qtplugin::make_unexpected(qtplugin::PluginError{
                    qtplugin::PluginErrorCode::ConfigurationError,
                    "health_check_interval must be between 5000 and 300000 "
                    "ms"});
            }
            m_healthTimer->setInterval(interval);
        }

        // Update feature flags
        if (config.contains("communication_enabled")) {
            m_communicationEnabled = config["communication_enabled"].toBool();
        }

        if (config.contains("monitoring_enabled")) {
            m_monitoringEnabled = config["monitoring_enabled"].toBool();
            if (m_monitoringEnabled) {
                m_metricsTimer->start();
                m_healthTimer->start();
            } else {
                m_metricsTimer->stop();
                m_healthTimer->stop();
            }
        }

        if (config.contains("security_enabled")) {
            m_securityEnabled = config["security_enabled"].toBool();
        }

        if (config.contains("networking_enabled")) {
            m_networkingEnabled = config["networking_enabled"].toBool();
        }

        if (config.contains("background_processing_enabled")) {
            m_backgroundProcessingEnabledFlag =
                config["background_processing_enabled"].toBool();
            if (m_backgroundProcessingEnabledFlag) {
                m_backgroundTimer->start();
            } else {
                m_backgroundTimer->stop();
            }
        }

        if (config.contains("python_integration_enabled")) {
            m_pythonIntegrationEnabled =
                config["python_integration_enabled"].toBool();
        }

        // Merge with existing configuration
        for (auto it = config.begin(); it != config.end(); ++it) {
            m_configuration[it.key()] = it.value();
        }

        emit configurationChanged(m_configuration);

        qCInfo(comprehensivePlugin)
            << "✅ Plugin configuration updated successfully";
        return {};

    } catch (const std::exception& e) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::ConfigurationError,
            std::string("Configuration failed: ") + e.what()});
    }
}

QJsonObject ComprehensivePlugin::get_configuration() const {
    QMutexLocker locker(&m_configMutex);
    return m_configuration;
}

qtplugin::PluginMetadata ComprehensivePlugin::metadata() const {
    qtplugin::PluginMetadata meta;
    meta.id = id();
    meta.name = std::string(name());
    meta.description = std::string(description());
    meta.version = version();
    meta.author = std::string(author());
    meta.capabilities = capabilities();
    meta.dependencies = {};  // No dependencies for this demo

    // Add custom metadata
    meta.custom_data["features"] = QJsonArray{"communication",
                                              "security",
                                              "monitoring",
                                              "networking",
                                              "background_processing",
                                              "transactions",
                                              "workflows",
                                              "python_integration"};

    meta.custom_data["supported_commands"] = QJsonArray();
    for (const auto& cmd : available_commands()) {
        meta.custom_data["supported_commands"].toArray().append(
            QString::fromStdString(cmd));
    }

    return meta;
}

void ComprehensivePlugin::setupCommunication() {
    if (!m_communicationEnabled)
        return;

    qCDebug(comprehensivePlugin) << "Setting up communication subsystem...";

    // Initialize message bus
    m_messageBus = std::make_unique<qtplugin::MessageBus>();

    // Subscribe to relevant topics
    m_messageBus->subscribe(
        "system.*", [this](const QString& topic, const QJsonObject& message) {
            onMessageReceived(topic, message);
        });

    m_messageBus->subscribe(
        "plugin.*", [this](const QString& topic, const QJsonObject& message) {
            onMessageReceived(topic, message);
        });

    // Initialize request-response system
    m_requestResponse = std::make_unique<qtplugin::RequestResponseSystem>();

    qCDebug(comprehensivePlugin) << "✅ Communication subsystem ready";
}

void ComprehensivePlugin::setupMonitoring() {
    if (!m_monitoringEnabled)
        return;

    qCDebug(comprehensivePlugin) << "Setting up monitoring subsystem...";

    // Initialize metrics collector
    m_metricsCollector = std::make_unique<qtplugin::PluginMetricsCollector>();

    qCDebug(comprehensivePlugin) << "✅ Monitoring subsystem ready";
}

void ComprehensivePlugin::setupSecurity() {
    if (!m_securityEnabled)
        return;

    qCDebug(comprehensivePlugin) << "Setting up security subsystem...";

    // Initialize security manager
    m_securityManager = std::make_unique<qtplugin::SecurityManager>();
    m_securityManager->set_security_level(qtplugin::SecurityLevel::Medium);

    qCDebug(comprehensivePlugin) << "✅ Security subsystem ready";
}

void ComprehensivePlugin::setupNetworking() {
    if (!m_networkingEnabled)
        return;

    qCDebug(comprehensivePlugin) << "Setting up networking subsystem...";

    // Initialize network manager
    m_networkManager = std::make_unique<QNetworkAccessManager>(this);

    qCDebug(comprehensivePlugin) << "✅ Networking subsystem ready";
}

void ComprehensivePlugin::setupBackgroundProcessing() {
    if (!m_backgroundProcessingEnabledFlag)
        return;

    qCDebug(comprehensivePlugin) << "Setting up background processing...";

    // Initialize background thread
    m_backgroundThread = std::make_unique<QThread>();
    m_backgroundProcessingEnabled = true;

    qCDebug(comprehensivePlugin) << "✅ Background processing ready";
}

// Command implementations
QJsonObject ComprehensivePlugin::handleStatusCommand(
    const QJsonObject& params) {
    Q_UNUSED(params)

    QJsonObject status;
    status["plugin_id"] = QString::fromStdString(id());
    status["plugin_name"] = QString::fromStdString(std::string(name()));
    status["version"] = QString("%1.%2.%3")
                            .arg(version().major)
                            .arg(version().minor)
                            .arg(version().patch);
    status["state"] = static_cast<int>(m_state.load());
    status["service_status"] = static_cast<int>(m_serviceStatus.load());
    status["uptime_seconds"] = m_startTime.secsTo(QDateTime::currentDateTime());

    // Feature status
    QJsonObject features;
    features["communication"] = m_communicationEnabled;
    features["monitoring"] = m_monitoringEnabled;
    features["security"] = m_securityEnabled;
    features["networking"] = m_networkingEnabled;
    features["background_processing"] = m_backgroundProcessingEnabledFlag;
    features["python_integration"] = m_pythonIntegrationEnabled;
    status["features"] = features;

    // Statistics
    QJsonObject stats;
    stats["commands_executed"] = m_commandsExecuted.load();
    stats["messages_processed"] = m_messagesProcessed.load();
    stats["service_requests_handled"] = m_serviceRequestsHandled.load();
    stats["errors_encountered"] = m_errorsEncountered.load();
    status["statistics"] = stats;

    return createSuccessResponse(status);
}

QJsonObject ComprehensivePlugin::handleEchoCommand(const QJsonObject& params) {
    QJsonObject response;
    response["echo"] = params;
    response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    response["plugin_id"] = QString::fromStdString(id());

    return createSuccessResponse(response);
}

QJsonObject ComprehensivePlugin::handleProcessDataCommand(
    const QJsonObject& params) {
    if (!validateInput(params, {"data"})) {
        return createErrorResponse("Missing required field: data");
    }

    QJsonValue inputData = params["data"];
    QString algorithm = params.value("algorithm").toString("default");

    QJsonObject result;
    result["input"] = inputData;
    result["algorithm"] = algorithm;
    result["processed_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    // Simulate data processing based on algorithm
    if (algorithm == "reverse" && inputData.isString()) {
        QString str = inputData.toString();
        std::reverse(str.begin(), str.end());
        result["output"] = str;
    } else if (algorithm == "uppercase" && inputData.isString()) {
        result["output"] = inputData.toString().toUpper();
    } else if (algorithm == "count" && inputData.isArray()) {
        result["output"] = inputData.toArray().size();
    } else {
        // Default processing - just return the input with metadata
        result["output"] = inputData;
        result["note"] = "Default processing applied";
    }

    return createSuccessResponse(result);
}

QJsonObject ComprehensivePlugin::handleNetworkRequestCommand(
    const QJsonObject& params) {
    if (!m_networkingEnabled) {
        return createErrorResponse("Networking is disabled");
    }

    if (!validateInput(params, {"url"})) {
        return createErrorResponse("Missing required field: url");
    }

    QString url = params["url"].toString();
    QString method = params.value("method").toString("GET");

    QJsonObject response;
    response["url"] = url;
    response["method"] = method;
    response["status"] = "request_initiated";
    response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    // In a real implementation, this would make an actual network request
    // For demo purposes, we'll simulate a successful response
    response["simulated_response"] =
        QJsonObject{{"status_code", 200},
                    {"content_type", "application/json"},
                    {"response_time_ms", 150}};

    return createSuccessResponse(response);
}

QJsonObject ComprehensivePlugin::handleMetricsCommand(
    const QJsonObject& params) {
    Q_UNUSED(params)

    if (!m_monitoringEnabled) {
        return createErrorResponse("Monitoring is disabled");
    }

    updateMetrics();

    QJsonObject metrics;
    metrics["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metrics["uptime_seconds"] =
        m_startTime.secsTo(QDateTime::currentDateTime());
    metrics["commands_executed"] = m_commandsExecuted.load();
    metrics["messages_processed"] = m_messagesProcessed.load();
    metrics["service_requests_handled"] = m_serviceRequestsHandled.load();
    metrics["errors_encountered"] = m_errorsEncountered.load();

    // Performance metrics
    QMutexLocker perfLocker(&m_performanceMutex);
    QJsonObject performance;
    for (const auto& [operation, times] : m_performanceHistory) {
        if (!times.empty()) {
            double avg =
                std::accumulate(times.begin(), times.end(), 0.0) / times.size();
            double min = *std::min_element(times.begin(), times.end());
            double max = *std::max_element(times.begin(), times.end());

            QJsonObject opMetrics;
            opMetrics["average_ms"] = avg;
            opMetrics["min_ms"] = min;
            opMetrics["max_ms"] = max;
            opMetrics["count"] = static_cast<int>(times.size());

            performance[QString::fromStdString(operation)] = opMetrics;
        }
    }
    metrics["performance"] = performance;

    return createSuccessResponse(metrics);
}

// Utility method implementations
void ComprehensivePlugin::updateMetrics() {
    // This would collect real metrics in a production implementation
    emit metricsUpdated(handleMetricsCommand({}).value("data").toObject());
}

void ComprehensivePlugin::publishEvent(const QString& event,
                                       const QJsonObject& data) {
    if (m_communicationEnabled && m_messageBus) {
        QJsonObject eventMessage;
        eventMessage["event"] = event;
        eventMessage["plugin_id"] = QString::fromStdString(id());
        eventMessage["timestamp"] =
            QDateTime::currentDateTime().toString(Qt::ISODate);
        eventMessage["data"] = data;

        m_messageBus->publish("plugin.events", eventMessage);
        emit messagePublished("plugin.events", eventMessage);
    }
}

bool ComprehensivePlugin::validateInput(const QJsonObject& input,
                                        const QStringList& requiredFields) {
    for (const QString& field : requiredFields) {
        if (!input.contains(field)) {
            return false;
        }
    }
    return true;
}

QJsonObject ComprehensivePlugin::createErrorResponse(const QString& error,
                                                     int code) {
    QJsonObject response;
    response["success"] = false;
    response["error"] = error;
    response["error_code"] = code;
    response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    response["plugin_id"] = QString::fromStdString(id());
    return response;
}

QJsonObject ComprehensivePlugin::createSuccessResponse(
    const QJsonObject& data) {
    QJsonObject response;
    response["success"] = true;
    response["data"] = data;
    response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    response["plugin_id"] = QString::fromStdString(id());
    return response;
}

void ComprehensivePlugin::startPerformanceTimer(const QString& operation) {
    QMutexLocker locker(&m_performanceMutex);
    m_performanceTimers[operation.toStdString()] =
        std::chrono::steady_clock::now();
}

void ComprehensivePlugin::endPerformanceTimer(const QString& operation) {
    QMutexLocker locker(&m_performanceMutex);
    auto it = m_performanceTimers.find(operation.toStdString());
    if (it != m_performanceTimers.end()) {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - it->second);
        double ms = duration.count() / 1000.0;

        auto& history = m_performanceHistory[operation.toStdString()];
        history.push_back(ms);

        // Keep only recent history
        if (history.size() > MAX_PERFORMANCE_HISTORY) {
            history.erase(history.begin());
        }

        m_performanceTimers.erase(it);
    }
}

// Slot implementations
void ComprehensivePlugin::onMessageReceived(const QString& topic,
                                            const QJsonObject& message) {
    m_messagesProcessed++;
    qCDebug(comprehensivePlugin)
        << "Message received on topic" << topic << ":" << message;

    // Handle system messages
    if (topic.startsWith("system.")) {
        if (topic == "system.shutdown") {
            qCInfo(comprehensivePlugin) << "Received shutdown signal";
            shutdown();
        }
    }
}

void ComprehensivePlugin::onMetricsCollection() { updateMetrics(); }

void ComprehensivePlugin::onHealthCheck() {
    bool healthy = (m_state == qtplugin::PluginState::Running ||
                    m_state == qtplugin::PluginState::Initialized);

    QJsonObject healthData;
    healthData["healthy"] = healthy;
    healthData["state"] = static_cast<int>(m_state.load());
    healthData["uptime"] = m_startTime.secsTo(QDateTime::currentDateTime());

    publishEvent("health.check", healthData);
    emit healthStatusChanged(healthy);
}

void ComprehensivePlugin::onBackgroundTask() {
    if (m_backgroundProcessingEnabled) {
        // Simulate background processing
        QJsonObject taskData;
        taskData["task_id"] = QUuid::createUuid().toString();
        taskData["type"] = "background_processing";
        taskData["timestamp"] =
            QDateTime::currentDateTime().toString(Qt::ISODate);

        publishEvent("background.task", taskData);
    }
}

// Service Plugin Interface Implementation
qtplugin::expected<void, qtplugin::PluginError>
ComprehensivePlugin::start_service() {
    qCInfo(comprehensivePlugin) << "Starting service...";

    if (m_state != qtplugin::PluginState::Initialized &&
        m_state != qtplugin::PluginState::Running) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::InvalidState,
            "Plugin must be initialized before starting service"});
    }

    try {
        // Start monitoring timers
        if (m_monitoringEnabled) {
            m_metricsTimer->start();
            m_healthTimer->start();
        }

        // Start background processing
        if (m_backgroundProcessingEnabledFlag) {
            m_backgroundTimer->start();
        }

        m_state = qtplugin::PluginState::Running;
        m_serviceStatus = qtplugin::ServiceStatus::Running;

        emit pluginStateChanged(m_state);
        emit serviceStarted();

        publishEvent("service.started", {});

        qCInfo(comprehensivePlugin) << "✅ Service started successfully";
        return {};

    } catch (const std::exception& e) {
        m_serviceStatus = qtplugin::ServiceStatus::Error;
        emit serviceError(QString::fromStdString(e.what()));

        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::ServiceError,
            std::string("Failed to start service: ") + e.what()});
    }
}

qtplugin::expected<void, qtplugin::PluginError>
ComprehensivePlugin::stop_service() {
    qCInfo(comprehensivePlugin) << "Stopping service...";

    try {
        // Stop all timers
        m_metricsTimer->stop();
        m_healthTimer->stop();
        m_backgroundTimer->stop();

        m_serviceStatus = qtplugin::ServiceStatus::Stopped;

        emit serviceStopped();
        publishEvent("service.stopped", {});

        qCInfo(comprehensivePlugin) << "✅ Service stopped successfully";
        return {};

    } catch (const std::exception& e) {
        m_serviceStatus = qtplugin::ServiceStatus::Error;
        emit serviceError(QString::fromStdString(e.what()));

        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::ServiceError,
            std::string("Failed to stop service: ") + e.what()});
    }
}

qtplugin::ServiceStatus ComprehensivePlugin::service_status() const {
    return m_serviceStatus;
}

QJsonObject ComprehensivePlugin::service_info() const {
    QJsonObject info;
    info["service_name"] = "Comprehensive Demo Service";
    info["status"] = static_cast<int>(m_serviceStatus.load());
    info["uptime_seconds"] = m_startTime.secsTo(QDateTime::currentDateTime());
    info["requests_handled"] = m_serviceRequestsHandled.load();

    QJsonArray capabilities;
    capabilities.append("data_processing");
    capabilities.append("monitoring");
    capabilities.append("security");
    capabilities.append("networking");
    info["capabilities"] = capabilities;

    return info;
}

#include "comprehensive_plugin.moc"
