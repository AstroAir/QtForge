/**
 * @file test_advanced_plugin_interface.cpp
 * @brief Tests for advanced plugin interface implementation
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QtTest/QtTest>
#include <chrono>
#include <memory>

// Advanced plugin interface merged into core plugin interface
#include <qtplugin/communication/plugin_service_contracts.hpp>
#include <qtplugin/utils/error_handling.hpp>

#include "../mock_objects.hpp"
#include "../utils/test_helpers.hpp"

using namespace qtplugin;
using namespace QtForgeTest;

/**
 * @brief Mock implementation of IAdvancedPlugin for testing
 */
class MockAdvancedPlugin : public AdvancedPluginBase {
public:
    MockAdvancedPlugin() : AdvancedPluginBase() {}

    // Override metadata to provide specific test values
    PluginMetadata metadata() const override {
        PluginMetadata meta;
        meta.name = "MockAdvancedPlugin";
        meta.version = Version(1, 0, 0);
        meta.description = "Mock advanced plugin for testing";
        meta.author = "Test Suite";
        meta.license = "MIT";
        meta.category = "test";
        meta.capabilities =
            static_cast<uint32_t>(PluginCapability::Service) |
            static_cast<uint32_t>(PluginCapability::Configuration);
        meta.priority = PluginPriority::Normal;
        return meta;
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
            return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed,
                                           "Simulated failure");
        }

        return AdvancedPluginBase::execute_command(command, params);
    }

    std::vector<std::string> available_commands() const override {
        return {"test", "fail", "status", "advanced_test", "service_call"};
    }

    // Service contracts for testing
    std::vector<contracts::ServiceContract> get_service_contracts() const {
        std::vector<contracts::ServiceContract> contracts_list;

        contracts::ServiceContract contract("test_service",
                                            contracts::ServiceVersion(1, 0, 0));
        contract.set_description("Test service for advanced plugin");

        contracts::ServiceMethod method;
        method.name = "process_data";
        method.description = "Process test data";
        contract.add_method(method);

        contracts_list.push_back(std::move(contract));
        return contracts_list;
    }

protected:
    qtplugin::expected<void, PluginError> do_initialize() { return {}; }
};

/**
 * @brief Test class for advanced plugin interface
 */
class TestAdvancedPluginInterface : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}
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

void TestAdvancedPluginInterface::init() {
    m_plugin = std::make_unique<MockAdvancedPlugin>();
}

void TestAdvancedPluginInterface::cleanup() {
    if (m_plugin) {
        m_plugin->shutdown();
        m_plugin.reset();
    }
}

void TestAdvancedPluginInterface::testPluginCreation() {
    QVERIFY(m_plugin != nullptr);
    auto meta = m_plugin->metadata();
    QCOMPARE(QString::fromStdString(meta.name), QString("MockAdvancedPlugin"));
    QCOMPARE(meta.version.to_string(), "1.0.0");
    QVERIFY(m_plugin->state() == PluginState::Unloaded);
}

void TestAdvancedPluginInterface::testPluginInitialization() {
    // Test successful initialization
    auto result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(result);
    QVERIFY(m_plugin->state() == PluginState::Running);

    // Test double initialization (should fail)
    auto double_init = m_plugin->initialize();
    QTFORGE_VERIFY_ERROR(double_init, PluginErrorCode::AlreadyExists);
}

void TestAdvancedPluginInterface::testPluginShutdown() {
    // Initialize first
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    QVERIFY(m_plugin->state() == PluginState::Running);

    // Test shutdown
    m_plugin->shutdown();
    QVERIFY(m_plugin->state() == PluginState::Stopped);

    // Test multiple shutdowns (should be safe)
    m_plugin->shutdown();
    QVERIFY(m_plugin->state() == PluginState::Stopped);
}

void TestAdvancedPluginInterface::testPluginMetadata() {
    auto metadata = m_plugin->metadata();
    QCOMPARE(QString::fromStdString(metadata.name),
             QString("MockAdvancedPlugin"));
    QVERIFY(metadata.version.major() == 1);
    QVERIFY(metadata.version.minor() == 0);
    QVERIFY(metadata.version.patch() == 0);
}

void TestAdvancedPluginInterface::testServiceContracts() {
    auto contracts = m_plugin->get_service_contracts();
    QVERIFY(!contracts.empty());
    QCOMPARE(contracts.size(), static_cast<size_t>(1));

    const auto& contract = contracts[0];
    QCOMPARE(contract.service_name(), QString("test_service"));
    QVERIFY(contract.version().major == 1);
    QVERIFY(!contract.methods().empty());
}

void TestAdvancedPluginInterface::testServiceCalls() {
    // Initialize plugin first
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    // Test command execution (service calls go through execute_command)
    QJsonObject params;
    params["test_data"] = "hello world";

    // Test valid command
    auto result = m_plugin->execute_command("test", params);
    QTFORGE_VERIFY_SUCCESS(result);
    QVERIFY(result.value().contains("command"));
    QCOMPARE(result.value()["command"].toString(), QString("test"));
}

void TestAdvancedPluginInterface::testAsyncServiceCalls() {
    // Initialize plugin first
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QJsonObject params;
    params["async_test"] = true;

    // Test command execution (async service calls would use execute_command)
    auto result = m_plugin->execute_command("advanced_test", params);
    QTFORGE_VERIFY_SUCCESS(result);
    QVERIFY(result.value().contains("status"));
    QCOMPARE(result.value()["status"].toString(), QString("advanced_success"));
}

void TestAdvancedPluginInterface::testServiceCallHandling() {
    QJsonObject params;
    params["test_param"] = "value";

    // Test handling a command for a provided service
    auto result = m_plugin->execute_command("test", params);

    // This should work since "test" is a valid command
    QTFORGE_VERIFY_SUCCESS(result);
    QVERIFY(result.value().contains("command"));
    QCOMPARE(result.value()["command"].toString(), QString("test"));

    // Test handling an invalid command
    auto invalid_result = m_plugin->execute_command("invalid_command", params);
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
    QVERIFY(m_plugin->state() == PluginState::Stopped);
}

void TestAdvancedPluginInterface::testHealthStatus() {
    // Test plugin state when unloaded
    QVERIFY(m_plugin->state() == PluginState::Unloaded);
    QVERIFY(!m_plugin->is_initialized());

    // Initialize and test state when running
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QVERIFY(m_plugin->state() == PluginState::Running);
    QVERIFY(m_plugin->is_initialized());

    // Test that service contracts are available
    auto contracts = m_plugin->get_service_contracts();
    QVERIFY(!contracts.empty());
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
    QVERIFY(std::find(commands.begin(), commands.end(), "test") !=
            commands.end());
}

void TestAdvancedPluginInterface::testServiceCallPerformance() {
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QJsonObject params;
    params["performance_test"] = true;

    // Measure command execution performance
    QElapsedTimer timer;
    timer.start();

    const int iterations = 100;
    for (int i = 0; i < iterations; ++i) {
        auto result = m_plugin->execute_command("test", params);
        // We don't check the result here, just measure timing
    }

    qint64 elapsed = timer.elapsed();
    qDebug() << "Command execution performance:" << elapsed << "ms for"
             << iterations << "calls";

    // Verify reasonable performance (less than 10ms per call on average)
    QVERIFY(elapsed < iterations * 10);
}

void TestAdvancedPluginInterface::testConcurrentServiceCalls() {
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QJsonObject params;
    params["concurrent_test"] = true;

    // Test multiple command executions
    const int concurrent_calls = 10;
    int successful_calls = 0;

    for (int i = 0; i < concurrent_calls; ++i) {
        auto result = m_plugin->execute_command("test", params);
        if (result.has_value()) {
            successful_calls++;
        }
    }

    // All calls should succeed
    QCOMPARE(successful_calls, concurrent_calls);
}

QTEST_MAIN(TestAdvancedPluginInterface)
#include "test_advanced_plugin_interface.moc"
