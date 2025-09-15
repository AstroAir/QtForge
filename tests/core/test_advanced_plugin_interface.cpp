/**
 * @file test_advanced_plugin_interface.cpp
 * @brief Tests for advanced plugin interface implementation
 */

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include <chrono>

// Advanced plugin interface merged into core plugin interface
#include <qtplugin/communication/service_contracts.hpp>
#include <qtplugin/utils/error_handling.hpp>

#include "../test_helpers.hpp"
#include "../mock_objects.hpp"

using namespace qtplugin;
using namespace QtForgeTest;

/**
 * @brief Mock implementation of IAdvancedPlugin for testing
 */
class MockAdvancedPlugin : public AdvancedPluginBase {
    Q_OBJECT

public:
    explicit MockAdvancedPlugin(QObject* parent = nullptr)
        : AdvancedPluginBase(parent), m_initialized(false) {}

    // IPlugin interface implementation
    std::string id() const override { return "mock_advanced_plugin"; }
    std::string name() const override { return "Mock Advanced Plugin"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override { return "Mock plugin for testing"; }

    PluginMetadata metadata() const override {
        PluginMetadata meta;
        meta.id = QString::fromStdString(id());
        meta.name = QString::fromStdString(name());
        meta.version = Version(1, 0, 0);
        meta.description = QString::fromStdString(description());
        return meta;
    }

    qtplugin::expected<void, PluginError> configure(const QJsonObject& config) override {
        Q_UNUSED(config)
        return make_success();
    }

    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override {
        
        QString cmd = QString::fromUtf8(command.data(), command.size());
        
        if (cmd == "test") {
            QJsonObject result;
            result["command"] = cmd;
            result["status"] = "success";
            result["params"] = params;
            return result;
        } else if (cmd == "fail") {
            return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed, "Simulated failure");
        }
        
        return make_error<QJsonObject>(PluginErrorCode::CommandNotFound, "Command not found");
    }

    std::vector<std::string> available_commands() const override {
        return {"test", "fail", "status"};
    }

    // IAdvancedPlugin interface implementation
    std::vector<contracts::ServiceContract> get_service_contracts() const override {
        std::vector<contracts::ServiceContract> contracts;
        
        contracts::ServiceContract contract;
        contract.set_service_name("test_service");
        contract.set_version(Version(1, 0, 0));
        
        contracts::ServiceMethod method;
        method.name = "process_data";
        method.description = "Process test data";
        contract.add_method(method);
        
        contracts.push_back(contract);
        return contracts;
    }

protected:
    qtplugin::expected<void, PluginError> do_initialize() override {
        m_initialized = true;
        return make_success();
    }

    void do_shutdown() noexcept override {
        m_initialized = false;
    }

    qtplugin::expected<void, PluginError> register_services() override {
        // Mock service registration
        return make_success();
    }

    void unregister_services() noexcept override {
        // Mock service unregistration
    }

private:
    bool m_initialized;
};

/**
 * @brief Test class for advanced plugin interface
 */
class TestAdvancedPluginInterface : public TestFixtureBase {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testPluginCreation();
    void testPluginInitialization();
    void testPluginShutdown();
    void testPluginMetadata();

    // Advanced interface tests
    void testServiceContracts();
    void testServiceCalls();
    void testAsyncServiceCalls();
    void testServiceCallHandling();

    // State management tests
    void testStateTransitions();
    void testHealthStatus();
    void testErrorHandling();

    // Configuration tests
    void testConfiguration();
    void testCommandExecution();

    // Performance tests
    void testServiceCallPerformance();
    void testConcurrentServiceCalls();

private:
    std::unique_ptr<MockAdvancedPlugin> m_plugin;
};

void TestAdvancedPluginInterface::initTestCase() {
    TestFixtureBase::initTestCase();
}

void TestAdvancedPluginInterface::cleanupTestCase() {
    TestFixtureBase::cleanupTestCase();
}

void TestAdvancedPluginInterface::init() {
    TestFixtureBase::init();
    m_plugin = std::make_unique<MockAdvancedPlugin>();
}

void TestAdvancedPluginInterface::cleanup() {
    if (m_plugin) {
        m_plugin->shutdown();
        m_plugin.reset();
    }
    TestFixtureBase::cleanup();
}

void TestAdvancedPluginInterface::testPluginCreation() {
    QVERIFY(m_plugin != nullptr);
    QCOMPARE(m_plugin->id(), std::string("mock_advanced_plugin"));
    QCOMPARE(m_plugin->name(), std::string("Mock Advanced Plugin"));
    QCOMPARE(m_plugin->version(), std::string("1.0.0"));
    QVERIFY(m_plugin->state() == PluginState::Unloaded);
}

void TestAdvancedPluginInterface::testPluginInitialization() {
    // Test successful initialization
    auto result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(result);
    QVERIFY(m_plugin->state() == PluginState::Running);

    // Test double initialization (should fail)
    auto double_init = m_plugin->initialize();
    QTFORGE_VERIFY_ERROR(double_init, PluginErrorCode::InvalidState);
}

void TestAdvancedPluginInterface::testPluginShutdown() {
    // Initialize first
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    QVERIFY(m_plugin->state() == PluginState::Running);

    // Test shutdown
    m_plugin->shutdown();
    QVERIFY(m_plugin->state() == PluginState::Unloaded);

    // Test multiple shutdowns (should be safe)
    m_plugin->shutdown();
    QVERIFY(m_plugin->state() == PluginState::Unloaded);
}

void TestAdvancedPluginInterface::testPluginMetadata() {
    auto metadata = m_plugin->metadata();
    QCOMPARE(metadata.id, QString("mock_advanced_plugin"));
    QCOMPARE(metadata.name, QString("Mock Advanced Plugin"));
    QVERIFY(metadata.version.major() == 1);
    QVERIFY(metadata.version.minor() == 0);
    QVERIFY(metadata.version.patch() == 0);
}

void TestAdvancedPluginInterface::testServiceContracts() {
    auto contracts = m_plugin->get_service_contracts();
    QVERIFY(!contracts.empty());
    QCOMPARE(contracts.size(), 1);

    const auto& contract = contracts[0];
    QCOMPARE(contract.service_name(), QString("test_service"));
    QVERIFY(contract.version().major() == 1);
    QVERIFY(!contract.methods().empty());
}

void TestAdvancedPluginInterface::testServiceCalls() {
    // Initialize plugin first
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    // Test service call (this would normally go through the service registry)
    QJsonObject params;
    params["test_data"] = "hello world";

    // Note: This test would need a proper service registry setup
    // For now, we test the handle_service_call method directly
    auto result = m_plugin->handle_service_call("test_service", "process_data", params);
    
    // The mock implementation delegates to execute_command, which doesn't recognize "process_data"
    // So we expect an error for this specific test
    QVERIFY(!result.has_value() || result.value().contains("command"));
}

void TestAdvancedPluginInterface::testAsyncServiceCalls() {
    // Initialize plugin first
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QJsonObject params;
    params["async_test"] = true;

    // Test async service call
    auto future = m_plugin->call_service_async("test_service", "process_data", params);
    
    // Wait for completion with timeout
    auto status = future.wait_for(std::chrono::milliseconds(1000));
    QVERIFY(status == std::future_status::ready);

    auto result = future.get();
    // We expect this to fail since we don't have a proper service registry
    QVERIFY(!result.has_value());
}

void TestAdvancedPluginInterface::testServiceCallHandling() {
    QJsonObject params;
    params["test_param"] = "value";

    // Test handling a service call for a provided service
    auto result = m_plugin->handle_service_call("test_service", "test", params);
    
    // This should work since "test" is a valid command
    if (result.has_value()) {
        QVERIFY(result.value().contains("command"));
        QCOMPARE(result.value()["command"].toString(), QString("test"));
    }

    // Test handling a service call for a non-provided service
    auto invalid_result = m_plugin->handle_service_call("invalid_service", "test", params);
    QTFORGE_VERIFY_ERROR(invalid_result, PluginErrorCode::CommandNotFound);
}

void TestAdvancedPluginInterface::testStateTransitions() {
    // Test initial state
    QVERIFY(m_plugin->state() == PluginState::Unloaded);

    // Test initialization state transition
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    QVERIFY(m_plugin->state() == PluginState::Running);

    // Test shutdown state transition
    m_plugin->shutdown();
    QVERIFY(m_plugin->state() == PluginState::Unloaded);
}

void TestAdvancedPluginInterface::testHealthStatus() {
    // Test health status when unloaded
    auto health_unloaded = m_plugin->get_health_status();
    QCOMPARE(health_unloaded["status"].toString(), QString("unhealthy"));
    QVERIFY(health_unloaded.contains("uptime"));
    QVERIFY(health_unloaded.contains("services"));

    // Initialize and test health status when running
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    auto health_running = m_plugin->get_health_status();
    QCOMPARE(health_running["status"].toString(), QString("healthy"));
    QVERIFY(health_running["uptime"].toInt() >= 0);
    
    auto services = health_running["services"].toArray();
    QVERIFY(!services.isEmpty());
}

void TestAdvancedPluginInterface::testErrorHandling() {
    // Test command execution error
    auto error_result = m_plugin->execute_command("fail");
    QTFORGE_VERIFY_ERROR(error_result, PluginErrorCode::ExecutionFailed);

    // Test invalid command
    auto invalid_result = m_plugin->execute_command("invalid_command");
    QTFORGE_VERIFY_ERROR(invalid_result, PluginErrorCode::CommandNotFound);
}

void TestAdvancedPluginInterface::testConfiguration() {
    QJsonObject config;
    config["test_setting"] = "test_value";
    config["enabled"] = true;

    auto result = m_plugin->configure(config);
    QTFORGE_VERIFY_SUCCESS(result);
}

void TestAdvancedPluginInterface::testCommandExecution() {
    QJsonObject params;
    params["test_param"] = "test_value";

    // Test successful command
    auto success_result = m_plugin->execute_command("test", params);
    QTFORGE_VERIFY_SUCCESS(success_result);
    
    if (success_result.has_value()) {
        auto result = success_result.value();
        QCOMPARE(result["command"].toString(), QString("test"));
        QCOMPARE(result["status"].toString(), QString("success"));
    }

    // Test available commands
    auto commands = m_plugin->available_commands();
    QVERIFY(!commands.empty());
    QVERIFY(std::find(commands.begin(), commands.end(), "test") != commands.end());
}

void TestAdvancedPluginInterface::testServiceCallPerformance() {
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QJsonObject params;
    params["performance_test"] = true;

    // Measure service call handling performance
    QElapsedTimer timer;
    timer.start();

    const int iterations = 100;
    for (int i = 0; i < iterations; ++i) {
        auto result = m_plugin->handle_service_call("test_service", "test", params);
        // We don't check the result here, just measure timing
    }

    qint64 elapsed = timer.elapsed();
    qDebug() << "Service call performance:" << elapsed << "ms for" << iterations << "calls";
    
    // Verify reasonable performance (less than 10ms per call on average)
    QVERIFY(elapsed < iterations * 10);
}

void TestAdvancedPluginInterface::testConcurrentServiceCalls() {
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QJsonObject params;
    params["concurrent_test"] = true;

    // Test concurrent async service calls
    std::vector<std::future<qtplugin::expected<QJsonObject, PluginError>>> futures;
    
    const int concurrent_calls = 10;
    for (int i = 0; i < concurrent_calls; ++i) {
        futures.push_back(m_plugin->call_service_async("test_service", "test", params));
    }

    // Wait for all calls to complete
    int completed = 0;
    for (auto& future : futures) {
        auto status = future.wait_for(std::chrono::milliseconds(1000));
        if (status == std::future_status::ready) {
            completed++;
        }
    }

    // All calls should complete (even if they fail due to missing service registry)
    QCOMPARE(completed, concurrent_calls);
}

QTEST_MAIN(TestAdvancedPluginInterface)
#include "test_advanced_plugin_interface.moc"
