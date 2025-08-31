/**
 * @file comprehensive_plugin.hpp
 * @brief Comprehensive plugin demonstrating ALL QtForge features
 * 
 * This plugin showcases every capability of the QtForge plugin system
 * including communication, security, monitoring, transactions, and more.
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QMutex>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>

// QtForge Core
#include <qtplugin/core/plugin_interface.hpp>
#include <qtplugin/core/service_plugin_interface.hpp>

// Communication
#include <qtplugin/communication/message_bus.hpp>
#include <qtplugin/communication/request_response_system.hpp>

// Security
#include <qtplugin/security/security_manager.hpp>

// Monitoring
#include <qtplugin/monitoring/plugin_metrics_collector.hpp>

// Utils
#include <qtplugin/utils/error_handling.hpp>
#include <qtplugin/utils/version.hpp>

#include <memory>
#include <atomic>
#include <vector>
#include <unordered_map>

/**
 * @brief Comprehensive plugin demonstrating all QtForge features
 * 
 * Features demonstrated:
 * - Core plugin interface implementation
 * - Service plugin capabilities
 * - Inter-plugin communication (message bus, request-response)
 * - Security validation and trust management
 * - Real-time monitoring and metrics
 * - Background processing and threading
 * - Configuration management
 * - Error handling with expected<T,E>
 * - Hot reload support
 * - Transaction support
 * - Network operations
 * - UI integration (optional)
 * - Python interoperability
 */
class ComprehensivePlugin : public QObject, 
                           public qtplugin::IPlugin,
                           public qtplugin::IServicePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "comprehensive_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin qtplugin::IServicePlugin)

public:
    explicit ComprehensivePlugin(QObject* parent = nullptr);
    ~ComprehensivePlugin() override;

    // IPlugin interface
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    qtplugin::Version version() const noexcept override;
    std::string_view author() const noexcept override;
    std::string id() const noexcept override;
    
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override;
    qtplugin::PluginCapabilities capabilities() const noexcept override;
    
    qtplugin::expected<QJsonObject, qtplugin::PluginError> 
    execute_command(std::string_view command, const QJsonObject& params) override;
    
    std::vector<std::string> available_commands() const override;
    
    qtplugin::expected<void, qtplugin::PluginError> 
    configure(const QJsonObject& config) override;
    
    QJsonObject get_configuration() const override;
    qtplugin::PluginMetadata metadata() const override;

    // IServicePlugin interface
    qtplugin::expected<void, qtplugin::PluginError> start_service() override;
    qtplugin::expected<void, qtplugin::PluginError> stop_service() override;
    qtplugin::ServiceStatus service_status() const override;
    QJsonObject service_info() const override;
    
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    handle_service_request(const QString& method, const QJsonObject& params) override;

public slots:
    // Communication slots
    void onMessageReceived(const QString& topic, const QJsonObject& message);
    void onServiceRequest(const QString& requestId, const QString& method, const QJsonObject& params);
    
    // Monitoring slots
    void onMetricsCollection();
    void onHealthCheck();
    
    // Background processing
    void onBackgroundTask();
    void onNetworkReply();

signals:
    // Plugin lifecycle signals
    void pluginStateChanged(qtplugin::PluginState newState);
    void configurationChanged(const QJsonObject& newConfig);
    
    // Service signals
    void serviceStarted();
    void serviceStopped();
    void serviceError(const QString& error);
    
    // Communication signals
    void messagePublished(const QString& topic, const QJsonObject& message);
    void requestProcessed(const QString& requestId, const QJsonObject& response);
    
    // Monitoring signals
    void metricsUpdated(const QJsonObject& metrics);
    void healthStatusChanged(bool healthy);

private:
    // Core functionality
    void setupCommunication();
    void setupMonitoring();
    void setupSecurity();
    void setupNetworking();
    void setupBackgroundProcessing();
    
    // Command implementations
    QJsonObject handleStatusCommand(const QJsonObject& params);
    QJsonObject handleEchoCommand(const QJsonObject& params);
    QJsonObject handleProcessDataCommand(const QJsonObject& params);
    QJsonObject handleNetworkRequestCommand(const QJsonObject& params);
    QJsonObject handleMetricsCommand(const QJsonObject& params);
    QJsonObject handleConfigCommand(const QJsonObject& params);
    QJsonObject handleSecurityCommand(const QJsonObject& params);
    QJsonObject handleTransactionCommand(const QJsonObject& params);
    QJsonObject handleWorkflowCommand(const QJsonObject& params);
    QJsonObject handlePythonCommand(const QJsonObject& params);
    
    // Service implementations
    QJsonObject handleDataProcessingService(const QJsonObject& params);
    QJsonObject handleMonitoringService(const QJsonObject& params);
    QJsonObject handleSecurityService(const QJsonObject& params);
    QJsonObject handleNetworkService(const QJsonObject& params);
    
    // Utility methods
    void updateMetrics();
    void publishEvent(const QString& event, const QJsonObject& data);
    bool validateInput(const QJsonObject& input, const QStringList& requiredFields);
    QJsonObject createErrorResponse(const QString& error, int code = -1);
    QJsonObject createSuccessResponse(const QJsonObject& data = {});
    
    // Security helpers
    bool isOperationAllowed(const QString& operation);
    void logSecurityEvent(const QString& event, const QJsonObject& details);
    
    // Performance helpers
    void startPerformanceTimer(const QString& operation);
    void endPerformanceTimer(const QString& operation);

private:
    // State management
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    std::atomic<qtplugin::ServiceStatus> m_serviceStatus{qtplugin::ServiceStatus::Stopped};
    mutable QMutex m_configMutex;
    QJsonObject m_configuration;
    
    // Communication components
    std::unique_ptr<qtplugin::MessageBus> m_messageBus;
    std::unique_ptr<qtplugin::RequestResponseSystem> m_requestResponse;
    
    // Monitoring components
    std::unique_ptr<qtplugin::PluginMetricsCollector> m_metricsCollector;
    QTimer* m_metricsTimer;
    QTimer* m_healthTimer;
    QTimer* m_backgroundTimer;
    
    // Security components
    std::unique_ptr<qtplugin::SecurityManager> m_securityManager;
    
    // Network components
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    
    // Metrics and statistics
    std::atomic<int> m_commandsExecuted{0};
    std::atomic<int> m_messagesProcessed{0};
    std::atomic<int> m_serviceRequestsHandled{0};
    std::atomic<int> m_errorsEncountered{0};
    QDateTime m_startTime;
    
    // Performance tracking
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> m_performanceTimers;
    std::unordered_map<std::string, std::vector<double>> m_performanceHistory;
    mutable QMutex m_performanceMutex;
    
    // Background processing
    std::unique_ptr<QThread> m_backgroundThread;
    std::atomic<bool> m_backgroundProcessingEnabled{false};
    
    // Feature flags
    bool m_communicationEnabled = true;
    bool m_monitoringEnabled = true;
    bool m_securityEnabled = true;
    bool m_networkingEnabled = true;
    bool m_backgroundProcessingEnabledFlag = true;
    bool m_pythonIntegrationEnabled = false;
    
    // Constants
    static constexpr int DEFAULT_METRICS_INTERVAL = 5000; // 5 seconds
    static constexpr int DEFAULT_HEALTH_CHECK_INTERVAL = 10000; // 10 seconds
    static constexpr int DEFAULT_BACKGROUND_INTERVAL = 1000; // 1 second
    static constexpr int MAX_PERFORMANCE_HISTORY = 100;
};

/**
 * @brief Background worker for processing tasks
 */
class BackgroundWorker : public QObject {
    Q_OBJECT

public:
    explicit BackgroundWorker(ComprehensivePlugin* plugin);

public slots:
    void processTask();
    void handleDataProcessing(const QJsonObject& data);
    void handleMonitoringTask();

signals:
    void taskCompleted(const QJsonObject& result);
    void taskFailed(const QString& error);

private:
    ComprehensivePlugin* m_plugin;
    std::atomic<bool> m_running{false};
};

/**
 * @brief Network request handler
 */
class NetworkHandler : public QObject {
    Q_OBJECT

public:
    explicit NetworkHandler(QNetworkAccessManager* manager, QObject* parent = nullptr);
    
    void sendRequest(const QString& url, const QJsonObject& data);

signals:
    void requestCompleted(const QJsonObject& response);
    void requestFailed(const QString& error);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager* m_manager;
    QNetworkReply* m_currentReply = nullptr;
};
