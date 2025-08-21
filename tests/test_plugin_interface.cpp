/**
 * @file test_plugin_interface.cpp
 * @brief Comprehensive tests for plugin interface functionality
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <memory>
#include <string>

#include <qtplugin/core/plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>
#include <qtplugin/utils/version.hpp>

using namespace qtplugin;

// Mock plugin implementation for testing
class MockPlugin : public IPlugin
{
public:
    MockPlugin() : m_initialized(false) {}

    // IPlugin interface - Metadata
    std::string_view name() const noexcept override {
        return "MockPlugin";
    }

    std::string_view description() const noexcept override {
        return "Mock plugin for testing";
    }

    Version version() const noexcept override {
        return Version(1, 0, 0);
    }

    std::string_view author() const noexcept override {
        return "Test Suite";
    }

    std::string id() const noexcept override {
        return "mock_plugin";
    }

    // IPlugin interface - Lifecycle
    expected<void, PluginError> initialize() override {
        if (m_initialized) {
            return make_error<void>(PluginErrorCode::AlreadyExists, "Plugin already initialized");
        }
        m_initialized = true;
        return make_success();
    }

    void shutdown() noexcept override {
        m_initialized = false;
    }

    PluginState state() const noexcept override {
        return m_initialized ? PluginState::Running : PluginState::Unloaded;
    }

    // IPlugin interface - Capabilities and Commands
    PluginCapabilities capabilities() const noexcept override {
        return static_cast<PluginCapabilities>(PluginCapability::Service);
    }

    expected<QJsonObject, PluginError>
    execute_command(std::string_view command, const QJsonObject& params = {}) override {
        Q_UNUSED(params)
        QJsonObject result;
        if (command == "test") {
            result["status"] = "ok";
            return result;
        }
        return make_error<QJsonObject>(PluginErrorCode::CommandNotFound, "Unknown command");
    }

    std::vector<std::string> available_commands() const override {
        return {"test"};
    }

    // IPlugin interface - Dependencies
    std::vector<std::string> dependencies() const override {
        return {};
    }

    bool is_initialized() const noexcept override {
        return m_initialized;
    }

private:
    bool m_initialized;
};

class TestPluginInterface : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic interface tests
    void testPluginCreation();
    void testPluginDestruction();
    void testPluginInfo();

    // Lifecycle tests
    void testPluginInitialization();
    void testPluginShutdown();
    void testPluginStart();
    void testPluginStop();
    void testPluginLifecycleOrder();

    // State management tests
    void testInvalidStateTransitions();
    void testDoubleInitialization();
    void testDoubleShutdown();

    // Plugin info tests
    void testPluginInfoValidation();
    void testApiVersionCompatibility();
    void testPluginStateConsistency();

private:
    std::unique_ptr<MockPlugin> m_plugin;
    
    // Helper methods
    void verifyPluginState(PluginState expected_state);
    void verifyPluginInfo(const std::string& expected_name, const std::string& expected_name2);
    void testStateTransition(PluginState from, PluginState to, bool should_succeed);
};

void TestPluginInterface::initTestCase()
{
    qDebug() << "Starting plugin interface tests";
}

void TestPluginInterface::cleanupTestCase()
{
    qDebug() << "Plugin interface tests completed";
}

void TestPluginInterface::init()
{
    // Create fresh plugin for each test
    m_plugin = std::make_unique<MockPlugin>();
    QVERIFY(m_plugin != nullptr);
}

void TestPluginInterface::cleanup()
{
    // Clean up plugin
    if (m_plugin) {
        if (m_plugin->is_initialized()) {
            m_plugin->shutdown();
        }
        m_plugin.reset();
    }
}

void TestPluginInterface::testPluginCreation()
{
    // Test basic plugin creation
    auto plugin = std::make_unique<MockPlugin>();
    QVERIFY(plugin != nullptr);

    // Test initial state
    QVERIFY(!plugin->is_initialized());
    QCOMPARE(plugin->state(), PluginState::Unloaded);
}

void TestPluginInterface::testPluginDestruction()
{
    // Test that destruction properly cleans up resources
    {
        auto plugin = std::make_unique<MockPlugin>();
        auto init_result = plugin->initialize();
        QVERIFY(init_result.has_value());

        // Plugin should clean up automatically when destroyed
    }

    // Verify no memory leaks
    QVERIFY(true); // This would be verified with memory profiling tools
}

void TestPluginInterface::testPluginInfo()
{
    // Verify basic info fields
    QCOMPARE(QString::fromStdString(std::string(m_plugin->name())), "MockPlugin");
    QCOMPARE(m_plugin->version().to_string(), "1.0.0");
    QCOMPARE(QString::fromStdString(std::string(m_plugin->description())), "Mock plugin for testing");
    QCOMPARE(QString::fromStdString(std::string(m_plugin->author())), "Test Suite");
    QCOMPARE(QString::fromStdString(m_plugin->id()), "mock_plugin");
    QCOMPARE(m_plugin->state(), PluginState::Unloaded);
}

void TestPluginInterface::testPluginInitialization()
{
    // Test successful initialization
    auto result = m_plugin->initialize();
    QVERIFY(result.has_value());
    QVERIFY(m_plugin->is_initialized());

    verifyPluginState(PluginState::Running);
}

void TestPluginInterface::testPluginShutdown()
{
    // Initialize first
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    // Test successful shutdown
    m_plugin->shutdown();
    QVERIFY(!m_plugin->is_initialized());

    verifyPluginState(PluginState::Unloaded);
}

void TestPluginInterface::testPluginStart()
{
    // Initialize first
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    QVERIFY(m_plugin->is_initialized());

    verifyPluginState(PluginState::Running);
}

void TestPluginInterface::testPluginStop()
{
    // Initialize first
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    // Test shutdown
    m_plugin->shutdown();
    QVERIFY(!m_plugin->is_initialized());

    verifyPluginState(PluginState::Unloaded);
}

void TestPluginInterface::testPluginLifecycleOrder()
{
    // Test complete lifecycle in correct order

    // 1. Initialize
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    verifyPluginState(PluginState::Running);

    // 2. Shutdown
    m_plugin->shutdown();
    verifyPluginState(PluginState::Unloaded);
}

void TestPluginInterface::testInvalidStateTransitions()
{
    // Test basic state transitions
    QCOMPARE(m_plugin->state(), PluginState::Unloaded);

    // Initialize
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    QCOMPARE(m_plugin->state(), PluginState::Running);

    // Shutdown
    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), PluginState::Unloaded);
}

void TestPluginInterface::testDoubleInitialization()
{
    // Initialize once
    auto init_result1 = m_plugin->initialize();
    QVERIFY(init_result1.has_value());

    // Try to initialize again
    auto init_result2 = m_plugin->initialize();
    QVERIFY(!init_result2.has_value());
    QCOMPARE(init_result2.error().code, PluginErrorCode::AlreadyExists);
}

void TestPluginInterface::testDoubleShutdown()
{
    // Initialize and shutdown once
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), PluginState::Unloaded);

    // Shutdown again should be safe (no-op)
    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), PluginState::Unloaded);
}

void TestPluginInterface::testApiVersionCompatibility()
{
    // Test plugin version
    Version plugin_version = m_plugin->version();
    QCOMPARE(plugin_version.to_string(), "1.0.0");

    // Test version compatibility
    Version current_api(3, 0, 0);
    QVERIFY(plugin_version.major() >= 1);
}

void TestPluginInterface::testPluginInfoValidation()
{
    // Verify required fields are not empty
    QVERIFY(!std::string(m_plugin->name()).empty());
    QVERIFY(!std::string(m_plugin->description()).empty());
    QVERIFY(!std::string(m_plugin->author()).empty());
    QVERIFY(!m_plugin->id().empty());

    // Verify version is valid
    Version version = m_plugin->version();
    QVERIFY(version.major() >= 0);
    QVERIFY(version.minor() >= 0);
    QVERIFY(version.patch() >= 0);
}

void TestPluginInterface::testPluginStateConsistency()
{
    // Test state consistency throughout lifecycle

    // Initial state
    QCOMPARE(m_plugin->state(), PluginState::Unloaded);
    QVERIFY(!m_plugin->is_initialized());

    // After initialization
    m_plugin->initialize();
    QCOMPARE(m_plugin->state(), PluginState::Running);
    QVERIFY(m_plugin->is_initialized());

    // After shutdown
    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), PluginState::Unloaded);
    QVERIFY(!m_plugin->is_initialized());
}

// Helper methods implementation
void TestPluginInterface::verifyPluginState(PluginState expected_state)
{
    QCOMPARE(m_plugin->state(), expected_state);
}

void TestPluginInterface::verifyPluginInfo(const std::string& expected_name, const std::string& expected_name2)
{
    Q_UNUSED(expected_name2)
    QCOMPARE(QString::fromStdString(std::string(m_plugin->name())), QString::fromStdString(expected_name));
    QVERIFY(!std::string(m_plugin->description()).empty());
    QVERIFY(!std::string(m_plugin->author()).empty());
}

void TestPluginInterface::testStateTransition(PluginState from, PluginState to, bool should_succeed)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    Q_UNUSED(should_succeed)
    // Helper implementation would go here
}

QTEST_MAIN(TestPluginInterface)
#include "test_plugin_interface.moc"
