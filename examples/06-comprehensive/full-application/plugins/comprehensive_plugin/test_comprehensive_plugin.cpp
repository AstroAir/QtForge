/**
 * @file test_comprehensive_plugin.cpp
 * @brief Comprehensive test suite for the comprehensive plugin
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QEventLoop>

#include "comprehensive_plugin.hpp"
#include <qtplugin/core/plugin_manager.hpp>
#include <qtplugin/utils/error_handling.hpp>

class TestComprehensivePlugin : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Core functionality tests
    void testPluginInitialization();
    void testPluginMetadata();
    void testPluginConfiguration();
    void testPluginShutdown();

    // Command execution tests
    void testStatusCommand();
    void testEchoCommand();
    void testProcessDataCommand();
    void testNetworkRequestCommand();
    void testMetricsCommand();
    void testConfigCommand();
    void testSecurityCommand();
    void testTransactionCommand();
    void testWorkflowCommand();
    void testPythonCommand();

    // Service functionality tests
    void testServiceLifecycle();
    void testServiceRequests();
    void testServiceInfo();

    // Communication tests
    void testMessageBusIntegration();
    void testEventPublishing();
    void testMessageHandling();

    // Monitoring tests
    void testMetricsCollection();
    void testHealthChecking();
    void testPerformanceTracking();

    // Security tests
    void testSecurityValidation();
    void testTrustManagement();
    void testPermissionChecking();

    // Background processing tests
    void testBackgroundTasks();
    void testThreadSafety();

    // Error handling tests
    void testInvalidCommands();
    void testInvalidParameters();
    void testErrorRecovery();

    // Performance tests
    void testCommandPerformance();
    void testMemoryUsage();
    void testConcurrentOperations();

    // Integration tests
    void testFullWorkflow();
    void testPluginInteraction();
    void testSystemIntegration();

private:
    std::unique_ptr<ComprehensivePlugin> m_plugin;
    std::unique_ptr<qtplugin::PluginManager> m_pluginManager;
    QJsonObject m_testConfig;

    // Helper methods
    void waitForSignal(QObject* sender, const char* signal, int timeout = 5000);
    QJsonObject createTestData();
    void verifyPluginState(qtplugin::PluginState expectedState);
    void verifyServiceStatus(qtplugin::ServiceStatus expectedStatus);
};

void TestComprehensivePlugin::initTestCase() {
    qDebug() << "Starting comprehensive plugin test suite...";
    
    // Initialize test environment
    m_pluginManager = std::make_unique<qtplugin::PluginManager>();
    
    // Setup test configuration
    m_testConfig["communication_enabled"] = true;
    m_testConfig["monitoring_enabled"] = true;
    m_testConfig["security_enabled"] = true;
    m_testConfig["networking_enabled"] = true;
    m_testConfig["background_processing_enabled"] = true;
    m_testConfig["metrics_interval"] = 1000;
    m_testConfig["health_check_interval"] = 2000;
}

void TestComprehensivePlugin::cleanupTestCase() {
    qDebug() << "Comprehensive plugin test suite completed.";
}

void TestComprehensivePlugin::init() {
    // Create fresh plugin instance for each test
    m_plugin = std::make_unique<ComprehensivePlugin>();
}

void TestComprehensivePlugin::cleanup() {
    // Cleanup after each test
    if (m_plugin) {
        m_plugin->shutdown();
        m_plugin.reset();
    }
}

void TestComprehensivePlugin::testPluginInitialization() {
    QVERIFY(m_plugin != nullptr);
    
    // Test initial state
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);
    
    // Test initialization
    auto result = m_plugin->initialize();
    QVERIFY(result.has_value());
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Initialized);
    
    // Test double initialization (should be safe)
    result = m_plugin->initialize();
    QVERIFY(result.has_value());
}

void TestComprehensivePlugin::testPluginMetadata() {
    auto metadata = m_plugin->metadata();
    
    QVERIFY(!metadata.id.empty());
    QVERIFY(!metadata.name.empty());
    QVERIFY(!metadata.description.empty());
    QVERIFY(!metadata.author.empty());
    
    QCOMPARE(QString::fromStdString(metadata.id), "com.qtforge.comprehensive_plugin");
    QCOMPARE(metadata.version.major, 3);
    QCOMPARE(metadata.version.minor, 0);
    QCOMPARE(metadata.version.patch, 0);
    
    // Test capabilities
    auto caps = m_plugin->capabilities();
    QVERIFY(caps & qtplugin::PluginCapability::Service);
    QVERIFY(caps & qtplugin::PluginCapability::Network);
}

void TestComprehensivePlugin::testPluginConfiguration() {
    // Initialize plugin first
    QVERIFY(m_plugin->initialize().has_value());
    
    // Test configuration
    auto configResult = m_plugin->configure(m_testConfig);
    QVERIFY(configResult.has_value());
    
    // Verify configuration was applied
    auto currentConfig = m_plugin->get_configuration();
    QCOMPARE(currentConfig["communication_enabled"].toBool(), true);
    QCOMPARE(currentConfig["monitoring_enabled"].toBool(), true);
    
    // Test invalid configuration
    QJsonObject invalidConfig;
    invalidConfig["metrics_interval"] = -1; // Invalid value
    
    auto invalidResult = m_plugin->configure(invalidConfig);
    QVERIFY(!invalidResult.has_value());
}

void TestComprehensivePlugin::testPluginShutdown() {
    // Initialize and start plugin
    QVERIFY(m_plugin->initialize().has_value());
    QVERIFY(m_plugin->configure(m_testConfig).has_value());
    QVERIFY(m_plugin->start_service().has_value());
    
    // Test shutdown
    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);
    QCOMPARE(m_plugin->service_status(), qtplugin::ServiceStatus::Stopped);
}

void TestComprehensivePlugin::testStatusCommand() {
    QVERIFY(m_plugin->initialize().has_value());
    QVERIFY(m_plugin->configure(m_testConfig).has_value());
    
    auto result = m_plugin->execute_command("status", {});
    QVERIFY(result.has_value());
    
    auto response = result.value();
    QVERIFY(response["success"].toBool());
    
    auto data = response["data"].toObject();
    QVERIFY(data.contains("plugin_id"));
    QVERIFY(data.contains("plugin_name"));
    QVERIFY(data.contains("version"));
    QVERIFY(data.contains("state"));
    QVERIFY(data.contains("features"));
    QVERIFY(data.contains("statistics"));
}

void TestComprehensivePlugin::testEchoCommand() {
    QVERIFY(m_plugin->initialize().has_value());
    
    QJsonObject params;
    params["message"] = "Hello, World!";
    params["number"] = 42;
    params["array"] = QJsonArray{1, 2, 3};
    
    auto result = m_plugin->execute_command("echo", params);
    QVERIFY(result.has_value());
    
    auto response = result.value();
    QVERIFY(response["success"].toBool());
    
    auto data = response["data"].toObject();
    auto echo = data["echo"].toObject();
    QCOMPARE(echo["message"].toString(), "Hello, World!");
    QCOMPARE(echo["number"].toInt(), 42);
}

void TestComprehensivePlugin::testProcessDataCommand() {
    QVERIFY(m_plugin->initialize().has_value());
    
    // Test string processing
    QJsonObject params;
    params["data"] = "hello";
    params["algorithm"] = "uppercase";
    
    auto result = m_plugin->execute_command("process_data", params);
    QVERIFY(result.has_value());
    
    auto response = result.value();
    QVERIFY(response["success"].toBool());
    
    auto data = response["data"].toObject();
    QCOMPARE(data["output"].toString(), "HELLO");
    
    // Test array processing
    params["data"] = QJsonArray{1, 2, 3, 4, 5};
    params["algorithm"] = "count";
    
    result = m_plugin->execute_command("process_data", params);
    QVERIFY(result.has_value());
    
    response = result.value();
    data = response["data"].toObject();
    QCOMPARE(data["output"].toInt(), 5);
    
    // Test missing data parameter
    result = m_plugin->execute_command("process_data", {});
    QVERIFY(!result.has_value());
}

void TestComprehensivePlugin::testServiceLifecycle() {
    QVERIFY(m_plugin->initialize().has_value());
    QVERIFY(m_plugin->configure(m_testConfig).has_value());
    
    // Test service start
    auto startResult = m_plugin->start_service();
    QVERIFY(startResult.has_value());
    QCOMPARE(m_plugin->service_status(), qtplugin::ServiceStatus::Running);
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Running);
    
    // Test service info
    auto serviceInfo = m_plugin->service_info();
    QVERIFY(serviceInfo.contains("service_name"));
    QVERIFY(serviceInfo.contains("status"));
    QVERIFY(serviceInfo.contains("capabilities"));
    
    // Test service stop
    auto stopResult = m_plugin->stop_service();
    QVERIFY(stopResult.has_value());
    QCOMPARE(m_plugin->service_status(), qtplugin::ServiceStatus::Stopped);
}

void TestComprehensivePlugin::testMessageBusIntegration() {
    QVERIFY(m_plugin->initialize().has_value());
    QVERIFY(m_plugin->configure(m_testConfig).has_value());
    
    // Setup signal spy for message publishing
    QSignalSpy messageSpy(m_plugin.get(), &ComprehensivePlugin::messagePublished);
    
    // Execute a command that should publish an event
    auto result = m_plugin->execute_command("status", {});
    QVERIFY(result.has_value());
    
    // Wait for message to be published
    QVERIFY(messageSpy.wait(1000));
    QVERIFY(messageSpy.count() > 0);
}

void TestComprehensivePlugin::testMetricsCollection() {
    QVERIFY(m_plugin->initialize().has_value());
    QVERIFY(m_plugin->configure(m_testConfig).has_value());
    
    // Execute some commands to generate metrics
    m_plugin->execute_command("status", {});
    m_plugin->execute_command("echo", {{"test", "data"}});
    
    // Get metrics
    auto result = m_plugin->execute_command("metrics", {});
    QVERIFY(result.has_value());
    
    auto response = result.value();
    auto data = response["data"].toObject();
    
    QVERIFY(data.contains("commands_executed"));
    QVERIFY(data.contains("uptime_seconds"));
    QVERIFY(data["commands_executed"].toInt() >= 2);
}

void TestComprehensivePlugin::testInvalidCommands() {
    QVERIFY(m_plugin->initialize().has_value());
    
    // Test invalid command
    auto result = m_plugin->execute_command("invalid_command", {});
    QVERIFY(!result.has_value());
    QVERIFY(result.error().code == qtplugin::PluginErrorCode::InvalidCommand);
    
    // Test empty command
    result = m_plugin->execute_command("", {});
    QVERIFY(!result.has_value());
}

void TestComprehensivePlugin::testCommandPerformance() {
    QVERIFY(m_plugin->initialize().has_value());
    QVERIFY(m_plugin->configure(m_testConfig).has_value());
    
    // Measure command execution time
    const int iterations = 100;
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < iterations; ++i) {
        auto result = m_plugin->execute_command("echo", {{"iteration", i}});
        QVERIFY(result.has_value());
    }
    
    qint64 elapsed = timer.elapsed();
    double avgTime = static_cast<double>(elapsed) / iterations;
    
    qDebug() << "Average command execution time:" << avgTime << "ms";
    
    // Performance should be reasonable (less than 10ms per command)
    QVERIFY(avgTime < 10.0);
}

void TestComprehensivePlugin::testFullWorkflow() {
    // Test complete plugin workflow
    QVERIFY(m_plugin->initialize().has_value());
    QVERIFY(m_plugin->configure(m_testConfig).has_value());
    QVERIFY(m_plugin->start_service().has_value());
    
    // Execute various commands
    QVERIFY(m_plugin->execute_command("status", {}).has_value());
    QVERIFY(m_plugin->execute_command("echo", {{"test", "workflow"}}).has_value());
    QVERIFY(m_plugin->execute_command("process_data", {{"data", "test"}}).has_value());
    QVERIFY(m_plugin->execute_command("metrics", {}).has_value());
    
    // Stop service and shutdown
    QVERIFY(m_plugin->stop_service().has_value());
    m_plugin->shutdown();
    
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);
}

// Helper methods
void TestComprehensivePlugin::waitForSignal(QObject* sender, const char* signal, int timeout) {
    QSignalSpy spy(sender, signal);
    QVERIFY(spy.wait(timeout));
}

QJsonObject TestComprehensivePlugin::createTestData() {
    QJsonObject data;
    data["string"] = "test";
    data["number"] = 42;
    data["boolean"] = true;
    data["array"] = QJsonArray{1, 2, 3};
    data["object"] = QJsonObject{{"nested", "value"}};
    return data;
}

void TestComprehensivePlugin::verifyPluginState(qtplugin::PluginState expectedState) {
    QCOMPARE(m_plugin->state(), expectedState);
}

void TestComprehensivePlugin::verifyServiceStatus(qtplugin::ServiceStatus expectedStatus) {
    QCOMPARE(m_plugin->service_status(), expectedStatus);
}

QTEST_MAIN(TestComprehensivePlugin)
#include "test_comprehensive_plugin.moc"
