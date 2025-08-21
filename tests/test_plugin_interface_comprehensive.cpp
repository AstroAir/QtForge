/**
 * @file test_plugin_interface_comprehensive.cpp
 * @brief Comprehensive tests for IPlugin interface implementations
 */

#include <QtTest/QtTest>
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QSignalSpy>
#include <memory>
#include <chrono>
#include <thread>

#include <qtplugin/qtplugin.hpp>

/**
 * @brief Mock plugin implementation for testing
 */
class MockPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "mock_metadata.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit MockPlugin(QObject* parent = nullptr) : QObject(parent) {}
    ~MockPlugin() override {
        if (m_state != qtplugin::PluginState::Unloaded) {
            shutdown();
        }
    }

    // IPlugin interface implementation
    std::string_view name() const noexcept override { return "Mock Plugin"; }
    std::string_view description() const noexcept override { return "A mock plugin for testing"; }
    qtplugin::Version version() const noexcept override { return {1, 2, 3}; }
    std::string_view author() const noexcept override { return "Test Author"; }
    std::string_view license() const noexcept override { return "MIT"; }
    std::string_view homepage() const noexcept override { return "https://test.example.com"; }
    std::string_view category() const noexcept override { return "Testing"; }
    std::string id() const noexcept override { return "com.test.mockplugin"; }

    qtplugin::PluginCapabilities capabilities() const noexcept override {
        return qtplugin::PluginCapability::Service | qtplugin::PluginCapability::Configuration;
    }

    std::vector<std::string> dependencies() const override {
        return {"com.test.dependency1", "com.test.dependency2"};
    }

    std::vector<std::string> optional_dependencies() const override {
        return {"com.test.optional1"};
    }

    qtplugin::expected<void, qtplugin::PluginError> initialize() override {
        if (m_state != qtplugin::PluginState::Unloaded && m_state != qtplugin::PluginState::Loaded) {
            return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError, 
                                             "Plugin is not in a state that allows initialization");
        }

        if (m_should_fail_init) {
            m_state = qtplugin::PluginState::Error;
            return qtplugin::make_error<void>(qtplugin::PluginErrorCode::InitializationFailed, 
                                             "Simulated initialization failure");
        }

        m_state = qtplugin::PluginState::Initializing;
        
        // Simulate some initialization work
        if (m_init_delay_ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_init_delay_ms));
        }

        m_state = qtplugin::PluginState::Running;
        m_init_count++;
        
        emit initialized();
        return qtplugin::make_success();
    }

    void shutdown() noexcept override {
        try {
            m_state = qtplugin::PluginState::Stopping;
            
            // Simulate cleanup work
            if (m_shutdown_delay_ms > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_shutdown_delay_ms));
            }
            
            m_state = qtplugin::PluginState::Unloaded;
            m_shutdown_count++;
            
            emit shutdown_completed();
        } catch (...) {
            // Ensure no exceptions escape
            m_state = qtplugin::PluginState::Error;
        }
    }

    qtplugin::PluginState state() const noexcept override {
        return m_state;
    }

    // Configuration management
    std::optional<QJsonObject> default_configuration() const override {
        QJsonObject config;
        config["setting1"] = "default_value";
        config["setting2"] = 42;
        config["setting3"] = true;
        return config;
    }

    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override {
        if (!validate_configuration(config)) {
            return qtplugin::make_error<void>(qtplugin::PluginErrorCode::ConfigurationError,
                                             "Configuration validation failed");
        }

        m_configuration = config;
        m_configure_count++;
        
        emit configured();
        return qtplugin::make_success();
    }

    QJsonObject current_configuration() const override {
        return m_configuration;
    }

    bool validate_configuration(const QJsonObject& config) const override {
        // Simple validation: check required keys
        return config.contains("setting1") && config.contains("setting2");
    }

    // Command execution
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    execute_command(std::string_view command, const QJsonObject& params = {}) override {
        QString cmd = QString::fromUtf8(command.data(), command.size());
        
        if (cmd == "status") {
            QJsonObject result;
            result["state"] = static_cast<int>(m_state.load());
            result["init_count"] = m_init_count;
            result["configure_count"] = m_configure_count;
            return result;
        }
        else if (cmd == "echo") {
            QJsonObject result;
            result["echoed"] = params;
            return result;
        }
        else if (cmd == "fail") {
            return qtplugin::make_error<QJsonObject>(qtplugin::PluginErrorCode::ExecutionFailed, 
                                                    "Simulated command failure");
        }
        else if (cmd == "delay") {
            int delay = params.value("ms").toInt(100);
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            QJsonObject result;
            result["delayed_ms"] = delay;
            return result;
        }
        
        return qtplugin::make_error<QJsonObject>(qtplugin::PluginErrorCode::CommandNotFound, 
                                                "Unknown command: " + cmd.toStdString());
    }

    std::vector<std::string> available_commands() const override {
        return {"status", "echo", "fail", "delay"};
    }

    // Error handling
    std::string last_error() const override {
        return m_last_error;
    }

    std::vector<std::string> error_log() const override {
        return m_error_log;
    }

    void clear_errors() override {
        m_last_error.clear();
        m_error_log.clear();
    }

    // Test control methods
    void set_should_fail_init(bool fail) { m_should_fail_init = fail; }
    void set_init_delay(int ms) { m_init_delay_ms = ms; }
    void set_shutdown_delay(int ms) { m_shutdown_delay_ms = ms; }
    int get_init_count() const { return m_init_count; }
    int get_shutdown_count() const { return m_shutdown_count; }
    int get_configure_count() const { return m_configure_count; }

signals:
    void initialized();
    void shutdown_completed();
    void configured();

private:
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    QJsonObject m_configuration;
    std::string m_last_error;
    std::vector<std::string> m_error_log;
    
    // Test control variables
    bool m_should_fail_init = false;
    int m_init_delay_ms = 0;
    int m_shutdown_delay_ms = 0;
    int m_init_count = 0;
    int m_shutdown_count = 0;
    int m_configure_count = 0;
};

/**
 * @brief Comprehensive test suite for IPlugin interface
 */
class TestPluginInterface : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Metadata tests
    void testMetadata();
    void testMetadataConsistency();
    void testCapabilities();
    void testDependencies();

    // Lifecycle tests
    void testInitialization();
    void testInitializationFailure();
    void testShutdown();
    void testStateTransitions();
    void testMultipleInitialization();
    void testInitializationTiming();

    // Configuration tests
    void testDefaultConfiguration();
    void testConfigurationValidation();
    void testConfigurationUpdate();
    void testInvalidConfiguration();
    void testConfigurationPersistence();

    // Command execution tests
    void testCommandExecution();
    void testCommandParameters();
    void testCommandFailure();
    void testUnknownCommand();
    void testAvailableCommands();
    void testCommandTiming();

    // Error handling tests
    void testErrorHandling();
    void testErrorLogging();
    void testErrorClearing();

    // Thread safety tests
    void testConcurrentAccess();
    void testConcurrentCommands();

    // Performance tests
    void testInitializationPerformance();
    void testCommandExecutionPerformance();

private:
    std::unique_ptr<MockPlugin> m_plugin;
};

void TestPluginInterface::initTestCase() {
    // Initialize QtPlugin library if needed
    qDebug() << "Starting IPlugin interface tests";
}

void TestPluginInterface::cleanupTestCase() {
    qDebug() << "IPlugin interface tests completed";
}

void TestPluginInterface::init() {
    m_plugin = std::make_unique<MockPlugin>();
}

void TestPluginInterface::cleanup() {
    if (m_plugin) {
        m_plugin->shutdown();
        m_plugin.reset();
    }
}

void TestPluginInterface::testMetadata() {
    QCOMPARE(m_plugin->name(), "Mock Plugin");
    QCOMPARE(m_plugin->description(), "A mock plugin for testing");
    QCOMPARE(m_plugin->author(), "Test Author");
    QCOMPARE(m_plugin->license(), "MIT");
    QCOMPARE(m_plugin->homepage(), "https://test.example.com");
    QCOMPARE(m_plugin->category(), "Testing");
    QCOMPARE(m_plugin->id(), "com.test.mockplugin");
    
    auto version = m_plugin->version();
    QCOMPARE(version.major(), 1);
    QCOMPARE(version.minor(), 2);
    QCOMPARE(version.patch(), 3);
}

void TestPluginInterface::testMetadataConsistency() {
    auto metadata = m_plugin->metadata();
    QCOMPARE(metadata.name, m_plugin->name());
    QCOMPARE(metadata.description, m_plugin->description());
    QCOMPARE(metadata.author, m_plugin->author());
    QCOMPARE(metadata.license, m_plugin->license());
    QCOMPARE(metadata.homepage, m_plugin->homepage());
    QCOMPARE(metadata.category, m_plugin->category());
    QCOMPARE(metadata.version.to_string(), m_plugin->version().to_string());
    QCOMPARE(metadata.capabilities, m_plugin->capabilities());
}

void TestPluginInterface::testCapabilities() {
    auto capabilities = m_plugin->capabilities();
    QVERIFY(capabilities & qtplugin::PluginCapability::Service);
    QVERIFY(capabilities & qtplugin::PluginCapability::Configuration);
    QVERIFY(!(capabilities & qtplugin::PluginCapability::UI));
}

void TestPluginInterface::testDependencies() {
    auto deps = m_plugin->dependencies();
    QCOMPARE(deps.size(), 2);
    QVERIFY(std::find(deps.begin(), deps.end(), "com.test.dependency1") != deps.end());
    QVERIFY(std::find(deps.begin(), deps.end(), "com.test.dependency2") != deps.end());
    
    auto optional_deps = m_plugin->optional_dependencies();
    QCOMPARE(optional_deps.size(), 1);
    QCOMPARE(optional_deps[0], "com.test.optional1");
}

void TestPluginInterface::testInitialization() {
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);
    QVERIFY(!m_plugin->is_initialized());
    
    QSignalSpy spy(m_plugin.get(), &MockPlugin::initialized);
    
    auto result = m_plugin->initialize();
    QVERIFY(result.has_value());
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Running);
    QVERIFY(m_plugin->is_initialized());
    QCOMPARE(m_plugin->get_init_count(), 1);
    QCOMPARE(spy.count(), 1);
}

void TestPluginInterface::testInitializationFailure() {
    m_plugin->set_should_fail_init(true);
    
    auto result = m_plugin->initialize();
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::InitializationFailed);
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Error);
    QVERIFY(!m_plugin->is_initialized());
}

void TestPluginInterface::testShutdown() {
    // Initialize first
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    QSignalSpy spy(m_plugin.get(), &MockPlugin::shutdown_completed);
    
    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);
    QCOMPARE(m_plugin->get_shutdown_count(), 1);
    QCOMPARE(spy.count(), 1);
}

void TestPluginInterface::testStateTransitions() {
    // Test valid state transitions
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);
    
    auto result = m_plugin->initialize();
    QVERIFY(result.has_value());
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Running);
    
    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);
}

void TestPluginInterface::testMultipleInitialization() {
    // First initialization should succeed
    auto result1 = m_plugin->initialize();
    QVERIFY(result1.has_value());
    QCOMPARE(m_plugin->get_init_count(), 1);
    
    // Second initialization should fail
    auto result2 = m_plugin->initialize();
    QVERIFY(!result2.has_value());
    QCOMPARE(result2.error().code, qtplugin::PluginErrorCode::StateError);
    QCOMPARE(m_plugin->get_init_count(), 1); // Should not increment
}

void TestPluginInterface::testInitializationTiming() {
    m_plugin->set_init_delay(50); // 50ms delay
    
    auto start = std::chrono::steady_clock::now();
    auto result = m_plugin->initialize();
    auto end = std::chrono::steady_clock::now();
    
    QVERIFY(result.has_value());
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    QVERIFY(duration.count() >= 45); // Allow some tolerance
}

void TestPluginInterface::testDefaultConfiguration() {
    auto config = m_plugin->default_configuration();
    QVERIFY(config.has_value());

    QJsonObject default_config = config.value();
    QVERIFY(default_config.contains("setting1"));
    QVERIFY(default_config.contains("setting2"));
    QVERIFY(default_config.contains("setting3"));

    QCOMPARE(default_config["setting1"].toString(), "default_value");
    QCOMPARE(default_config["setting2"].toInt(), 42);
    QCOMPARE(default_config["setting3"].toBool(), true);
}

void TestPluginInterface::testConfigurationValidation() {
    QJsonObject valid_config;
    valid_config["setting1"] = "test_value";
    valid_config["setting2"] = 100;

    QVERIFY(m_plugin->validate_configuration(valid_config));

    QJsonObject invalid_config;
    invalid_config["setting1"] = "test_value";
    // Missing setting2

    QVERIFY(!m_plugin->validate_configuration(invalid_config));
}

void TestPluginInterface::testConfigurationUpdate() {
    QSignalSpy spy(m_plugin.get(), &MockPlugin::configured);

    QJsonObject config;
    config["setting1"] = "new_value";
    config["setting2"] = 200;
    config["setting3"] = false;

    auto result = m_plugin->configure(config);
    QVERIFY(result.has_value());
    QCOMPARE(m_plugin->get_configure_count(), 1);
    QCOMPARE(spy.count(), 1);

    auto current_config = m_plugin->current_configuration();
    QCOMPARE(current_config["setting1"].toString(), "new_value");
    QCOMPARE(current_config["setting2"].toInt(), 200);
    QCOMPARE(current_config["setting3"].toBool(), false);
}

void TestPluginInterface::testInvalidConfiguration() {
    QJsonObject invalid_config;
    invalid_config["setting1"] = "test_value";
    // Missing required setting2

    auto result = m_plugin->configure(invalid_config);
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::ConfigurationError);
    QCOMPARE(m_plugin->get_configure_count(), 0);
}

void TestPluginInterface::testConfigurationPersistence() {
    QJsonObject config1;
    config1["setting1"] = "value1";
    config1["setting2"] = 100;

    auto result1 = m_plugin->configure(config1);
    QVERIFY(result1.has_value());

    auto current1 = m_plugin->current_configuration();
    QCOMPARE(current1["setting1"].toString(), "value1");

    QJsonObject config2;
    config2["setting1"] = "value2";
    config2["setting2"] = 200;

    auto result2 = m_plugin->configure(config2);
    QVERIFY(result2.has_value());

    auto current2 = m_plugin->current_configuration();
    QCOMPARE(current2["setting1"].toString(), "value2");
    QCOMPARE(current2["setting2"].toInt(), 200);
}

void TestPluginInterface::testCommandExecution() {
    // Initialize plugin first
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    // Test status command
    auto status_result = m_plugin->execute_command("status");
    QVERIFY(status_result.has_value());

    QJsonObject status = status_result.value();
    QVERIFY(status.contains("state"));
    QVERIFY(status.contains("init_count"));
    QCOMPARE(status["state"].toInt(), static_cast<int>(qtplugin::PluginState::Running));
    QCOMPARE(status["init_count"].toInt(), 1);
}

void TestPluginInterface::testCommandParameters() {
    QJsonObject params;
    params["message"] = "Hello, World!";
    params["number"] = 42;

    auto result = m_plugin->execute_command("echo", params);
    QVERIFY(result.has_value());

    QJsonObject response = result.value();
    QVERIFY(response.contains("echoed"));

    QJsonObject echoed = response["echoed"].toObject();
    QCOMPARE(echoed["message"].toString(), "Hello, World!");
    QCOMPARE(echoed["number"].toInt(), 42);
}

void TestPluginInterface::testCommandFailure() {
    auto result = m_plugin->execute_command("fail");
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::ExecutionFailed);
    QVERIFY(result.error().message.find("Simulated command failure") != std::string::npos);
}

void TestPluginInterface::testUnknownCommand() {
    auto result = m_plugin->execute_command("unknown_command");
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::CommandNotFound);
    QVERIFY(result.error().message.find("Unknown command") != std::string::npos);
}

void TestPluginInterface::testAvailableCommands() {
    auto commands = m_plugin->available_commands();
    QCOMPARE(commands.size(), 4);

    std::vector<std::string> expected = {"status", "echo", "fail", "delay"};
    for (const auto& cmd : expected) {
        QVERIFY(std::find(commands.begin(), commands.end(), cmd) != commands.end());
    }
}

void TestPluginInterface::testCommandTiming() {
    QJsonObject params;
    params["ms"] = 100;

    auto start = std::chrono::steady_clock::now();
    auto result = m_plugin->execute_command("delay", params);
    auto end = std::chrono::steady_clock::now();

    QVERIFY(result.has_value());
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    QVERIFY(duration.count() >= 95); // Allow some tolerance

    QJsonObject response = result.value();
    QCOMPARE(response["delayed_ms"].toInt(), 100);
}

void TestPluginInterface::testErrorHandling() {
    // Initially no errors
    QVERIFY(m_plugin->last_error().empty());
    QVERIFY(m_plugin->error_log().empty());

    // Test that errors are properly handled in expected<T,E>
    auto result = m_plugin->execute_command("fail");
    QVERIFY(!result.has_value());
    QVERIFY(!result.error().message.empty());
}

void TestPluginInterface::testErrorLogging() {
    // Test error logging functionality if implemented
    auto initial_log = m_plugin->error_log();
    auto initial_error = m_plugin->last_error();

    // After clearing, should be empty
    m_plugin->clear_errors();
    QVERIFY(m_plugin->last_error().empty());
    QVERIFY(m_plugin->error_log().empty());
}

void TestPluginInterface::testErrorClearing() {
    m_plugin->clear_errors();
    QVERIFY(m_plugin->last_error().empty());
    QVERIFY(m_plugin->error_log().empty());
}

void TestPluginInterface::testConcurrentAccess() {
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    const int num_threads = 4;
    const int commands_per_thread = 10;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, commands_per_thread, &success_count, &failure_count]() {
            for (int j = 0; j < commands_per_thread; ++j) {
                auto result = m_plugin->execute_command("status");
                if (result.has_value()) {
                    success_count++;
                } else {
                    failure_count++;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    QCOMPARE(success_count.load(), num_threads * commands_per_thread);
    QCOMPARE(failure_count.load(), 0);
}

void TestPluginInterface::testConcurrentCommands() {
    const int num_commands = 20;
    std::vector<std::thread> threads;
    std::atomic<int> completed{0};

    for (int i = 0; i < num_commands; ++i) {
        threads.emplace_back([this, i, &completed]() {
            QJsonObject params;
            params["ms"] = 10; // Short delay
            params["thread_id"] = i;

            auto result = m_plugin->execute_command("delay", params);
            if (result.has_value()) {
                completed++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    QCOMPARE(completed.load(), num_commands);
}

void TestPluginInterface::testInitializationPerformance() {
    const int num_iterations = 100;
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < num_iterations; ++i) {
        auto plugin = std::make_unique<MockPlugin>();
        auto result = plugin->initialize();
        QVERIFY(result.has_value());
        plugin->shutdown();
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    qDebug() << "Initialization performance:" << duration.count() << "ms for" << num_iterations << "iterations";
    qDebug() << "Average per initialization:" << (duration.count() / num_iterations) << "ms";

    // Should be reasonably fast (less than 10ms per initialization on average)
    QVERIFY(duration.count() / num_iterations < 10);
}

void TestPluginInterface::testCommandExecutionPerformance() {
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    const int num_commands = 1000;
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < num_commands; ++i) {
        auto result = m_plugin->execute_command("status");
        QVERIFY(result.has_value());
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    qDebug() << "Command execution performance:" << duration.count() << "ms for" << num_commands << "commands";
    qDebug() << "Average per command:" << (static_cast<double>(duration.count()) / num_commands) << "ms";

    // Should be very fast (less than 1ms per command on average)
    QVERIFY(static_cast<double>(duration.count()) / num_commands < 1.0);
}

#include "test_plugin_interface_comprehensive.moc"

QTEST_MAIN(TestPluginInterface)
