/**
 * @file test_lifecycle_manager.cpp
 * @brief Test the Plugin Lifecycle Manager implementation
 * @version 3.0.0
 */

#include <QSignalSpy>
#include <QtTest/QtTest>
#include <algorithm>
#include <memory>
#include "qtplugin/core/plugin_lifecycle_manager.hpp"
#include "qtplugin/interfaces/core/plugin_interface.hpp"

using namespace qtplugin;

/**
 * @brief Mock plugin for testing lifecycle management
 */
class MockLifecyclePlugin : public IPlugin {
private:
    std::string m_id;
    PluginState m_state = PluginState::Unloaded;
    bool m_initialization_should_fail = false;
    QJsonObject m_config;

public:
    explicit MockLifecyclePlugin(const std::string& id) : m_id(id) {}

    // IPlugin interface implementation
    PluginMetadata metadata() const override {
        PluginMetadata meta;
        meta.name = m_id;  // Use the ID as the name for registration
        meta.version = Version{1, 0, 0};
        meta.description = "Mock plugin for testing";
        meta.author = "Test Suite";
        return meta;
    }

    PluginState state() const noexcept override { return m_state; }
    uint32_t capabilities() const noexcept override { return 0; }
    PluginPriority priority() const noexcept override {
        return PluginPriority::Normal;
    }
    bool is_initialized() const noexcept override {
        return m_state == PluginState::Running;
    }

    qtplugin::expected<void, PluginError> initialize() override {
        if (m_initialization_should_fail) {
            return make_error<void>(PluginErrorCode::InitializationFailed,
                                    "Mock initialization failure");
        }
        m_state = PluginState::Running;
        return {};
    }

    void shutdown() noexcept override { m_state = PluginState::Stopped; }

    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command, const QJsonObject& params) override {
        Q_UNUSED(command)
        Q_UNUSED(params)
        return QJsonObject{};
    }

    std::vector<std::string> available_commands() const override { return {}; }

    qtplugin::expected<void, PluginError> configure(
        const QJsonObject& config) override {
        m_config = config;
        return {};
    }

    QJsonObject get_configuration() const override { return m_config; }

    // Test control methods
    void set_initialization_should_fail(bool should_fail) {
        m_initialization_should_fail = should_fail;
    }
};

/**
 * @brief Test class for Plugin Lifecycle Manager
 */
class TestLifecycleManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testPluginStateMachine();
    void testLifecycleManagerBasic();
    void testPluginInitialization();
    void testPluginInitializationFailure();
    void testStateTransitions();

private:
    std::unique_ptr<PluginLifecycleManager> m_lifecycle_manager;
};

void TestLifecycleManager::initTestCase() {
    qDebug() << "Starting lifecycle manager tests";
    m_lifecycle_manager = std::make_unique<PluginLifecycleManager>();
}

void TestLifecycleManager::cleanupTestCase() {
    qDebug() << "Lifecycle manager tests completed";
    m_lifecycle_manager.reset();
}

void TestLifecycleManager::testPluginStateMachine() {
    // Test our custom state machine directly
    PluginStateMachine state_machine("test.plugin");

    // Test initial state
    QCOMPARE(state_machine.current_state(), PluginState::Unloaded);

    // Test valid transitions
    auto result = state_machine.transition_to(PluginState::Loading);
    QVERIFY(result.has_value());
    QCOMPARE(state_machine.current_state(), PluginState::Loading);

    result = state_machine.transition_to(PluginState::Loaded);
    QVERIFY(result.has_value());
    QCOMPARE(state_machine.current_state(), PluginState::Loaded);

    result = state_machine.transition_to(PluginState::Initializing);
    QVERIFY(result.has_value());
    QCOMPARE(state_machine.current_state(), PluginState::Initializing);

    result = state_machine.transition_to(PluginState::Running);
    QVERIFY(result.has_value());
    QCOMPARE(state_machine.current_state(), PluginState::Running);

    // Test invalid transition
    result = state_machine.transition_to(PluginState::Loading);
    QVERIFY(!result.has_value());
    QCOMPARE(state_machine.current_state(),
             PluginState::Running);  // Should remain unchanged

    // Test reset
    state_machine.reset();
    QCOMPARE(state_machine.current_state(), PluginState::Unloaded);
}

void TestLifecycleManager::testLifecycleManagerBasic() {
    // Test basic lifecycle manager functionality
    QVERIFY(m_lifecycle_manager != nullptr);

    // Create a mock plugin
    auto plugin = std::make_shared<MockLifecyclePlugin>("test.basic");

    // Register plugin
    auto result = m_lifecycle_manager->register_plugin(plugin);
    QVERIFY(result.has_value());

    // Check if plugin is registered
    QVERIFY(m_lifecycle_manager->is_plugin_registered("test.basic"));

    // Get registered plugins
    auto registered_plugins = m_lifecycle_manager->get_registered_plugins();
    QVERIFY(std::find(registered_plugins.begin(), registered_plugins.end(),
                      "test.basic") != registered_plugins.end());

    // Unregister plugin
    result = m_lifecycle_manager->unregister_plugin("test.basic");
    QVERIFY(result.has_value());

    // Check if plugin is no longer registered
    QVERIFY(!m_lifecycle_manager->is_plugin_registered("test.basic"));
}

void TestLifecycleManager::testPluginInitialization() {
    // Test successful plugin initialization
    auto plugin = std::make_shared<MockLifecyclePlugin>("test.init.success");

    // Register plugin
    auto result = m_lifecycle_manager->register_plugin(plugin);
    QVERIFY(result.has_value());

    // Initialize plugin
    result = m_lifecycle_manager->initialize_plugin("test.init.success");
    QVERIFY(result.has_value());

    // Check plugin state
    QCOMPARE(plugin->state(), PluginState::Running);

    // Cleanup
    m_lifecycle_manager->unregister_plugin("test.init.success");
}

void TestLifecycleManager::testPluginInitializationFailure() {
    // Test plugin initialization failure
    auto plugin = std::make_shared<MockLifecyclePlugin>("test.init.failure");
    plugin->set_initialization_should_fail(true);

    // Register plugin
    auto result = m_lifecycle_manager->register_plugin(plugin);
    QVERIFY(result.has_value());

    // Try to initialize plugin (should fail)
    result = m_lifecycle_manager->initialize_plugin("test.init.failure");
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, PluginErrorCode::InitializationFailed);

    // Cleanup
    m_lifecycle_manager->unregister_plugin("test.init.failure");
}

void TestLifecycleManager::testStateTransitions() {
    // Test state transition validation
    QVERIFY(PluginStateMachine::is_valid_transition(PluginState::Unloaded,
                                                    PluginState::Loading));
    QVERIFY(PluginStateMachine::is_valid_transition(PluginState::Loading,
                                                    PluginState::Loaded));
    QVERIFY(PluginStateMachine::is_valid_transition(PluginState::Loaded,
                                                    PluginState::Initializing));
    QVERIFY(PluginStateMachine::is_valid_transition(PluginState::Initializing,
                                                    PluginState::Running));
    QVERIFY(PluginStateMachine::is_valid_transition(PluginState::Running,
                                                    PluginState::Paused));
    QVERIFY(PluginStateMachine::is_valid_transition(PluginState::Paused,
                                                    PluginState::Running));
    QVERIFY(PluginStateMachine::is_valid_transition(PluginState::Running,
                                                    PluginState::Stopping));
    QVERIFY(PluginStateMachine::is_valid_transition(PluginState::Stopping,
                                                    PluginState::Stopped));
    QVERIFY(PluginStateMachine::is_valid_transition(PluginState::Stopped,
                                                    PluginState::Unloaded));

    // Test invalid transitions
    QVERIFY(!PluginStateMachine::is_valid_transition(PluginState::Unloaded,
                                                     PluginState::Running));
    QVERIFY(!PluginStateMachine::is_valid_transition(PluginState::Running,
                                                     PluginState::Loading));
    QVERIFY(!PluginStateMachine::is_valid_transition(PluginState::Stopped,
                                                     PluginState::Running));
}

QTEST_MAIN(TestLifecycleManager)
#include "test_lifecycle_manager.moc"
