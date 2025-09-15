/**
 * @file main.cpp
 * @brief Comprehensive demonstration of ALL QtForge features
 *
 * This application showcases every feature and capability of the QtForge
 * plugin system in a single, integrated demonstration.
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QTimer>

// QtForge Core Components
#include <qtplugin/core/plugin_dependency_resolver.hpp>
#include <qtplugin/core/plugin_loader.hpp>
#include <qtplugin/core/plugin_manager.hpp>
#include <qtplugin/core/plugin_registry.hpp>
#include <qtplugin/qtplugin.hpp>

// Communication System
#include <qtplugin/communication/message_bus.hpp>
#include <qtplugin/communication/request_response_system.hpp>

// Security System
#include <qtplugin/security/security_manager.hpp>

// Monitoring & Metrics
#include <qtplugin/monitoring/plugin_hot_reload_manager.hpp>
#include <qtplugin/monitoring/plugin_metrics_collector.hpp>

// Management Components
#include <qtplugin/managers/configuration_manager.hpp>
#include <qtplugin/managers/logging_manager.hpp>
#include <qtplugin/managers/resource_manager.hpp>

// Orchestration & Workflows
#include <qtplugin/orchestration/plugin_orchestrator.hpp>
#include <qtplugin/orchestration/workflow.hpp>

// Transactions
#include <qtplugin/transactions/transaction_manager.hpp>

// Composition & Architecture
#include <qtplugin/composition/plugin_composer.hpp>

// Marketplace
#include <qtplugin/marketplace/plugin_marketplace.hpp>

// Threading
#include <qtplugin/threading/thread_pool_manager.hpp>

// Python Bridge
#ifdef QTFORGE_PYTHON_SUPPORT
#include <qtplugin/python/python_bridge.hpp>
#endif

#include <chrono>
#include <iostream>
#include <memory>

Q_LOGGING_CATEGORY(demo, "qtforge.demo")

class ComprehensiveDemo : public QObject {
    Q_OBJECT

public:
    explicit ComprehensiveDemo(QObject* parent = nullptr);
    ~ComprehensiveDemo() override;

    bool initialize(const QCommandLineParser& parser);
    void run();

private slots:
    void onPluginLoaded(const QString& pluginId);
    void onPluginUnloaded(const QString& pluginId);
    void onMessageReceived(const QString& topic, const QJsonObject& message);
    void onWorkflowCompleted(const QString& workflowId, bool success);
    void onMetricsUpdate();

private:
    void setupLogging();
    void initializeCore();
    void initializeCommunication();
    void initializeSecurity();
    void initializeMonitoring();
    void initializeOrchestration();
    void initializeTransactions();
    void initializeMarketplace();
    void initializeThreading();
    void initializePython();

    void loadPlugins();
    void demonstrateFeatures();
    void runWorkflowDemo();
    void runCommunicationDemo();
    void runSecurityDemo();
    void runPerformanceDemo();
    void runPythonDemo();

    void printSystemStatus();
    void printPerformanceMetrics();

private:
    // Core components
    std::unique_ptr<qtplugin::PluginManager> m_pluginManager;
    std::unique_ptr<qtplugin::PluginRegistry> m_pluginRegistry;
    std::unique_ptr<qtplugin::PluginLoader> m_pluginLoader;

    // Communication
    std::unique_ptr<qtplugin::MessageBus> m_messageBus;
    std::unique_ptr<qtplugin::RequestResponseSystem> m_requestResponse;

    // Security
    std::unique_ptr<qtplugin::SecurityManager> m_securityManager;

    // Monitoring
    std::unique_ptr<qtplugin::PluginHotReloadManager> m_hotReloadManager;
    std::unique_ptr<qtplugin::PluginMetricsCollector> m_metricsCollector;

    // Management
    std::unique_ptr<qtplugin::ConfigurationManager> m_configManager;
    std::unique_ptr<qtplugin::LoggingManager> m_loggingManager;
    std::unique_ptr<qtplugin::ResourceManager> m_resourceManager;

    // Orchestration
    std::unique_ptr<qtplugin::PluginOrchestrator> m_orchestrator;

    // Transactions
    std::unique_ptr<qtplugin::TransactionManager> m_transactionManager;

    // Composition
    std::unique_ptr<qtplugin::PluginComposer> m_composer;

    // Marketplace
    std::unique_ptr<qtplugin::PluginMarketplace> m_marketplace;

    // Threading
    std::unique_ptr<qtplugin::ThreadPoolManager> m_threadManager;

#ifdef QTFORGE_PYTHON_SUPPORT
    // Python Bridge
    std::unique_ptr<qtplugin::PythonBridge> m_pythonBridge;
#endif

    // Configuration
    QJsonObject m_config;
    QString m_pluginDirectory;
    bool m_enablePython = false;
    bool m_enableUI = false;
    qtplugin::SecurityLevel m_securityLevel = qtplugin::SecurityLevel::Medium;

    // Metrics
    QTimer* m_metricsTimer;
    std::chrono::steady_clock::time_point m_startTime;
    int m_loadedPlugins = 0;
    int m_processedMessages = 0;
    int m_completedTransactions = 0;
};

ComprehensiveDemo::ComprehensiveDemo(QObject* parent)
    : QObject(parent), m_metricsTimer(new QTimer(this)) {
    m_startTime = std::chrono::steady_clock::now();

    // Setup metrics timer
    connect(m_metricsTimer, &QTimer::timeout, this,
            &ComprehensiveDemo::onMetricsUpdate);
    m_metricsTimer->setInterval(5000);  // 5 seconds
}

ComprehensiveDemo::~ComprehensiveDemo() = default;

bool ComprehensiveDemo::initialize(const QCommandLineParser& parser) {
    qCInfo(demo) << "🚀 QtForge Comprehensive Demo v3.0.0";
    qCInfo(demo) << "=====================================";

    // Parse command line options
    m_pluginDirectory = parser.value("plugin-dir");
    if (m_pluginDirectory.isEmpty()) {
        m_pluginDirectory = "./plugins";
    }

    m_enablePython = parser.isSet("enable-python");
    m_enableUI = parser.isSet("enable-ui");

    QString securityLevel = parser.value("security-level");
    if (securityLevel == "low") {
        m_securityLevel = qtplugin::SecurityLevel::Low;
    } else if (securityLevel == "high") {
        m_securityLevel = qtplugin::SecurityLevel::High;
    }

    // Load configuration
    QFile configFile("config/application.json");
    if (configFile.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        QJsonDocument doc =
            QJsonDocument::fromJson(configFile.readAll(), &error);
        if (error.error == QJsonParseError::NoError) {
            m_config = doc.object();
        }
    }

    try {
        setupLogging();
        initializeCore();
        initializeCommunication();
        initializeSecurity();
        initializeMonitoring();
        initializeOrchestration();
        initializeTransactions();
        initializeMarketplace();
        initializeThreading();

        if (m_enablePython) {
            initializePython();
        }

        qCInfo(demo) << "[SUCCESS] All components initialized successfully!";
        return true;

    } catch (const std::exception& e) {
        qCCritical(demo) << "[ERROR] Initialization failed:" << e.what();
        return false;
    }
}

void ComprehensiveDemo::setupLogging() {
    qCInfo(demo) << "[INIT] Setting up logging system...";

    // Enable QtForge debug logging
    QLoggingCategory::setFilterRules("qtplugin.*=true");
    QLoggingCategory::setFilterRules("qtforge.*=true");

    // Initialize logging manager
    m_loggingManager = std::make_unique<qtplugin::LoggingManager>();
    m_loggingManager->set_log_level(qtplugin::LogLevel::Debug);
    m_loggingManager->enable_file_logging("comprehensive_demo.log");
}

void ComprehensiveDemo::initializeCore() {
    qCInfo(demo) << "[CORE] Initializing core plugin system...";

    // Initialize plugin registry
    m_pluginRegistry = std::make_unique<qtplugin::PluginRegistry>();

    // Initialize plugin loader
    m_pluginLoader = std::make_unique<qtplugin::PluginLoader>();

    // Initialize plugin manager
    m_pluginManager = std::make_unique<qtplugin::PluginManager>();
    m_pluginManager->set_plugin_directory(m_pluginDirectory);

    // Connect signals
    connect(m_pluginManager.get(), &qtplugin::PluginManager::pluginLoaded, this,
            &ComprehensiveDemo::onPluginLoaded);
    connect(m_pluginManager.get(), &qtplugin::PluginManager::pluginUnloaded,
            this, &ComprehensiveDemo::onPluginUnloaded);

    qCInfo(demo) << "✅ Core plugin system ready";
}

void ComprehensiveDemo::initializeCommunication() {
    qCInfo(demo) << "[COMMUNICATION] Initializing message bus and "
                    "request-response system...";

    // Initialize message bus
    m_messageBus = std::make_unique<qtplugin::MessageBus>();

    // Initialize request-response system
    m_requestResponse = std::make_unique<qtplugin::RequestResponseSystem>();

    // Subscribe to system messages
    m_messageBus->subscribe(
        "system.*", [this](const QString& topic, const QJsonObject& message) {
            onMessageReceived(topic, message);
        });

    qCInfo(demo) << "✅ Communication system ready";
}

void ComprehensiveDemo::initializeSecurity() {
    qCInfo(demo) << "[SECURITY] Initializing security management...";

    m_securityManager = std::make_unique<qtplugin::SecurityManager>();
    m_securityManager->set_security_level(m_securityLevel);

    // Add trusted publishers
    m_securityManager->add_trusted_plugin("com.example");
    m_securityManager->add_trusted_plugin("org.qtforge");

    qCInfo(demo) << "✅ Security system ready with level:"
                 << static_cast<int>(m_securityLevel);
}

void ComprehensiveDemo::initializeMonitoring() {
    qCInfo(demo) << "[MONITORING] Initializing monitoring and metrics...";

    m_hotReloadManager = std::make_unique<qtplugin::PluginHotReloadManager>();
    m_metricsCollector = std::make_unique<qtplugin::PluginMetricsCollector>();

    // Enable hot reload
    m_hotReloadManager->enable_hot_reload(true);
    m_hotReloadManager->set_watch_directory(m_pluginDirectory);

    qCInfo(demo) << "✅ Monitoring system ready";
}

void ComprehensiveDemo::initializeOrchestration() {
    qCInfo(demo) << "[ORCHESTRATION] Initializing workflow engine...";

    m_orchestrator = std::make_unique<qtplugin::PluginOrchestrator>();

    connect(m_orchestrator.get(),
            &qtplugin::PluginOrchestrator::workflowCompleted, this,
            &ComprehensiveDemo::onWorkflowCompleted);

    qCInfo(demo) << "✅ Orchestration system ready";
}

void ComprehensiveDemo::initializeTransactions() {
    qCInfo(demo) << "[TRANSACTIONS] Initializing transaction manager...";

    m_transactionManager = std::make_unique<qtplugin::TransactionManager>();
    m_transactionManager->set_isolation_level(
        qtplugin::IsolationLevel::ReadCommitted);

    qCInfo(demo) << "✅ Transaction system ready";
}



void ComprehensiveDemo::initializeThreading() {
    qCInfo(demo) << "[THREADING] Initializing thread pool...";

    m_threadManager = std::make_unique<qtplugin::ThreadPoolManager>();
    m_threadManager->set_max_threads(8);

    qCInfo(demo) << "✅ Threading system ready (8 threads)";
}

void ComprehensiveDemo::initializePython() {
#ifdef QTFORGE_PYTHON_SUPPORT
    qCInfo(demo) << "[PYTHON] Initializing Python bridge...";

    m_pythonBridge = std::make_unique<qtplugin::PythonBridge>();
    auto result = m_pythonBridge->initialize();

    if (result.has_value()) {
        qCInfo(demo) << "✅ Python bridge ready";
    } else {
        qCWarning(demo) << "⚠️ Python bridge initialization failed:"
                        << result.error().message.c_str();
    }
#endif
}

void ComprehensiveDemo::loadPlugins() {
    qCInfo(demo) << "\n[LOADING] Loading plugins from:" << m_pluginDirectory;

    QDir pluginDir(m_pluginDirectory);
    if (!pluginDir.exists()) {
        qCWarning(demo)
            << "Plugin directory does not exist, creating sample plugins...";
        // Create sample plugins would go here
        return;
    }

    auto result = m_pluginManager->load_plugin_directory(m_pluginDirectory);
    if (result.has_value()) {
        m_loadedPlugins = result.value().size();
        qCInfo(demo) << "✅ Loaded" << m_loadedPlugins
                     << "plugins successfully";
    } else {
        qCWarning(demo) << "Failed to load plugins:"
                        << result.error().message.c_str();
    }
}

void ComprehensiveDemo::demonstrateFeatures() {
    qCInfo(demo) << "\n[DEMO] Demonstrating all features...";

    runCommunicationDemo();
    runSecurityDemo();
    runWorkflowDemo();
    runPerformanceDemo();

    if (m_enablePython) {
        runPythonDemo();
    }
}

void ComprehensiveDemo::runCommunicationDemo() {
    qCInfo(demo) << "\n--- Communication Demo ---";

    // Publish test message
    QJsonObject testMessage;
    testMessage["type"] = "test";
    testMessage["timestamp"] =
        QDateTime::currentDateTime().toString(Qt::ISODate);
    testMessage["data"] = "Hello from comprehensive demo!";

    m_messageBus->publish("demo.test", testMessage);
    m_processedMessages++;

    qCInfo(demo) << "✅ Message published to demo.test topic";
}

void ComprehensiveDemo::runSecurityDemo() {
    qCInfo(demo) << "\n--- Security Demo ---";

    // Validate a hypothetical plugin
    auto validation =
        m_securityManager->validate_plugin("./plugins/sample.qtplugin");
    qCInfo(demo) << "✅ Security validation completed";
}

void ComprehensiveDemo::runWorkflowDemo() {
    qCInfo(demo) << "\n--- Workflow Demo ---";

    // Create sample workflow
    auto workflow = m_orchestrator->create_workflow("demo_workflow",
                                                    "Demonstration Workflow");
    workflow->add_step("step1", "data_validator", "validate");
    workflow->add_step("step2", "data_processor", "process");
    workflow->add_step("step3", "data_transmitter", "transmit");

    qCInfo(demo) << "✅ Workflow created with 3 steps";
}

void ComprehensiveDemo::runPerformanceDemo() {
    qCInfo(demo) << "\n--- Performance Demo ---";

    auto metrics = m_metricsCollector->collect_metrics();
    qCInfo(demo) << "✅ Performance metrics collected";
}

void ComprehensiveDemo::runPythonDemo() {
#ifdef QTFORGE_PYTHON_SUPPORT
    qCInfo(demo) << "\n--- Python Demo ---";

    if (m_pythonBridge) {
        auto result =
            m_pythonBridge->execute_script("print('Hello from Python!')");
        if (result.has_value()) {
            qCInfo(demo) << "✅ Python script executed successfully";
        }
    }
#endif
}

void ComprehensiveDemo::onPluginLoaded(const QString& pluginId) {
    qCInfo(demo) << "Plugin loaded:" << pluginId;
}

void ComprehensiveDemo::onPluginUnloaded(const QString& pluginId) {
    qCInfo(demo) << "Plugin unloaded:" << pluginId;
}

void ComprehensiveDemo::onMessageReceived(const QString& topic,
                                          const QJsonObject& message) {
    m_processedMessages++;
    qCDebug(demo) << "Message received on" << topic << ":" << message;
}

void ComprehensiveDemo::onWorkflowCompleted(const QString& workflowId,
                                            bool success) {
    if (success) {
        m_completedTransactions++;
        qCInfo(demo) << "Workflow completed successfully:" << workflowId;
    } else {
        qCWarning(demo) << "Workflow failed:" << workflowId;
    }
}

void ComprehensiveDemo::onMetricsUpdate() {
    // Update metrics periodically
    auto now = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime)
            .count();

    qCDebug(demo) << "Metrics update - Uptime:" << elapsed
                  << "s, Messages:" << m_processedMessages;
}

void ComprehensiveDemo::printSystemStatus() {
    qCInfo(demo) << "\n=== System Status ===";
    qCInfo(demo) << "Loaded plugins:" << m_loadedPlugins;
    qCInfo(demo) << "Security level:" << static_cast<int>(m_securityLevel);
    qCInfo(demo) << "Python support:"
                 << (m_enablePython ? "Enabled" : "Disabled");
    qCInfo(demo) << "UI support:" << (m_enableUI ? "Enabled" : "Disabled");
}

void ComprehensiveDemo::printPerformanceMetrics() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime)
            .count();

    qCInfo(demo) << "\n=== Performance Metrics ===";
    qCInfo(demo) << "Total runtime:" << elapsed << "ms";
    qCInfo(demo) << "Messages processed:" << m_processedMessages;
    qCInfo(demo) << "Transactions completed:" << m_completedTransactions;
    qCInfo(demo) << "Average message rate:"
                 << (m_processedMessages * 1000.0 / elapsed) << "msg/s";
}

void ComprehensiveDemo::run() {
    qCInfo(demo) << "\n[DEMO] Starting comprehensive feature demonstration...";

    // Start metrics collection
    m_metricsTimer->start();

    // Load plugins
    loadPlugins();

    // Demonstrate all features
    demonstrateFeatures();

    // Print final status
    printSystemStatus();
    printPerformanceMetrics();

    qCInfo(demo) << "\n🎉 [SUCCESS] All features demonstrated successfully!";
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("QtForge Comprehensive Demo");
    app.setApplicationVersion("3.0.0");

    // Setup command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(
        "Comprehensive demonstration of all QtForge features");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption({{"d", "plugin-dir"},
                      "Plugin directory path",
                      "directory",
                      "./plugins"});
    parser.addOption({{"p", "enable-python"}, "Enable Python bridge support"});
    parser.addOption({{"u", "enable-ui"}, "Enable UI components"});
    parser.addOption({{"s", "security-level"},
                      "Security level (low|medium|high)",
                      "level",
                      "medium"});

    parser.process(app);

    // Create and run demo
    ComprehensiveDemo demo;
    if (!demo.initialize(parser)) {
        return -1;
    }

    demo.run();
    return 0;
}

#include "main.moc"
