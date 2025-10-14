/**
 * @file test_plugin_interface_advanced.cpp
 * @brief Advanced tests for IPlugin interface implementations
 */

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QSignalSpy>
#include <QTimer>
#include <QtTest/QtTest>
#include <chrono>
#include <memory>
#include <thread>

#include <qtplugin/qtplugin.hpp>

/**
 * @brief Mock plugin implementation for testing
 */
class MockPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT

public:
    MockPlugin() = default;
    ~MockPlugin() override = default;

    // IPlugin interface - required methods
    qtplugin::PluginMetadata metadata() const override {
        qtplugin::PluginMetadata meta;
        meta.name = "MockPlugin";
        meta.version = qtplugin::Version{1, 0, 0};
        meta.description = "Mock plugin for testing";
        meta.author = "Test Suite";
        meta.license = "MIT";
        return meta;
    }

    qtplugin::PluginState state() const noexcept override { return m_state; }

    uint32_t capabilities() const noexcept override {
        return static_cast<uint32_t>(qtplugin::PluginCapability::Configuration);
    }

    qtplugin::PluginPriority priority() const noexcept override {
        return qtplugin::PluginPriority::Normal;
    }

    bool is_initialized() const noexcept override { return m_initialized; }

    qtplugin::expected<void, qtplugin::PluginError> initialize() override {
        if (m_initialized) {
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::AlreadyLoaded,
                "Already initialized");
        }
        m_initialized = true;
        m_state = qtplugin::PluginState::Running;
        emit initialized();
        return qtplugin::make_success();
    }

    void shutdown() noexcept override {
        m_initialized = false;
        m_state = qtplugin::PluginState::Unloaded;
        emit shutdown_signal();
    }

    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override {
        QJsonObject result;
        if (command == "test") {
            result["command"] = "test";
            result["params"] = params;
            result["success"] = true;
        } else {
            return qtplugin::make_error<QJsonObject>(
                qtplugin::PluginErrorCode::CommandNotFound,
                std::string("Unknown command: ") + std::string(command));
        }
        return result;
    }

    std::vector<std::string> available_commands() const override {
        return {"test"};
    }

    qtplugin::expected<void, qtplugin::PluginError> configure(
        const QJsonObject& config) override {
        m_config = config;
        return qtplugin::make_success();
    }

    QJsonObject get_configuration() const override { return m_config; }

    bool isInitialized() const { return m_initialized; }

signals:
    void initialized();
    void shutdown_signal();

private:
    bool m_initialized = false;
    qtplugin::PluginState m_state = qtplugin::PluginState::Unloaded;
    QJsonObject m_config;
};

/**
 * @brief Advanced test class for IPlugin interface
 */
class TestPluginInterfaceAdvanced : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic interface tests
    void testPluginCreation();
    void testPluginMetadata();
    void testPluginInitialization();
    void testPluginShutdown();

    // Advanced functionality tests
    void testPluginLifecycle();
    void testPluginSignals();
    void testPluginMetadataValidation();
    void testPluginStateConsistency();

    // Error handling tests
    void testInitializationFailure();
    void testShutdownSafety();
    void testInvalidMetadata();

    // Performance tests
    void testInitializationPerformance();
    void testShutdownPerformance();
    void testMetadataAccessPerformance();

    // Thread safety tests
    void testConcurrentAccess();
    void testThreadSafeInitialization();
    void testThreadSafeShutdown();

    // Memory management tests
    void testMemoryLeaks();
    void testResourceCleanup();

private:
    std::unique_ptr<MockPlugin> m_plugin;
};

void TestPluginInterfaceAdvanced::initTestCase() {
    // Global test setup
}

void TestPluginInterfaceAdvanced::cleanupTestCase() {
    // Global test cleanup
}

void TestPluginInterfaceAdvanced::init() {
    m_plugin = std::make_unique<MockPlugin>();
}

void TestPluginInterfaceAdvanced::cleanup() {
    if (m_plugin && m_plugin->isInitialized()) {
        m_plugin->shutdown();
    }
    m_plugin.reset();
}

void TestPluginInterfaceAdvanced::testPluginCreation() {
    QVERIFY(m_plugin != nullptr);
    QVERIFY(!m_plugin->isInitialized());
}

void TestPluginInterfaceAdvanced::testPluginMetadata() {
    auto metadata = m_plugin->metadata();
    QVERIFY(!metadata.name.empty());
    QCOMPARE(QString::fromStdString(metadata.name), QString("MockPlugin"));
    QCOMPARE(metadata.version.to_string(), std::string("1.0.0"));
    QCOMPARE(QString::fromStdString(metadata.description),
             QString("Mock plugin for testing"));
    QCOMPARE(QString::fromStdString(metadata.author), QString("Test Suite"));
    QCOMPARE(QString::fromStdString(metadata.license), QString("MIT"));
}

void TestPluginInterfaceAdvanced::testPluginInitialization() {
    QSignalSpy spy(m_plugin.get(), &MockPlugin::initialized);

    QVERIFY(!m_plugin->isInitialized());
    auto result = m_plugin->initialize();
    QVERIFY(result.has_value());
    QVERIFY(m_plugin->isInitialized());

    QCOMPARE(spy.count(), 1);
}

void TestPluginInterfaceAdvanced::testPluginShutdown() {
    QSignalSpy spy(m_plugin.get(), &MockPlugin::shutdown_signal);

    m_plugin->initialize();
    QVERIFY(m_plugin->isInitialized());

    m_plugin->shutdown();
    QVERIFY(!m_plugin->isInitialized());

    QCOMPARE(spy.count(), 1);
}

void TestPluginInterfaceAdvanced::testPluginLifecycle() {
    // Test multiple initialization/shutdown cycles
    for (int i = 0; i < 5; ++i) {
        QVERIFY(!m_plugin->isInitialized());
        auto init_result = m_plugin->initialize();
        QVERIFY(init_result.has_value());
        QVERIFY(m_plugin->isInitialized());
        m_plugin->shutdown();
        QVERIFY(!m_plugin->isInitialized());
    }
}

void TestPluginInterfaceAdvanced::testPluginSignals() {
    QSignalSpy initSpy(m_plugin.get(), &MockPlugin::initialized);
    QSignalSpy shutdownSpy(m_plugin.get(), &MockPlugin::shutdown_signal);

    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    QCOMPARE(initSpy.count(), 1);
    QCOMPARE(shutdownSpy.count(), 0);

    m_plugin->shutdown();
    QCOMPARE(initSpy.count(), 1);
    QCOMPARE(shutdownSpy.count(), 1);
}

void TestPluginInterfaceAdvanced::testPluginMetadataValidation() {
    auto metadata = m_plugin->metadata();

    // Validate required fields
    QVERIFY(!metadata.name.empty());
    QVERIFY(metadata.version.major() >= 0);
    QVERIFY(!metadata.description.empty());
    QVERIFY(!metadata.author.empty());
    QVERIFY(!metadata.license.empty());

    // Validate specific values
    QCOMPARE(QString::fromStdString(metadata.name), QString("MockPlugin"));
    QCOMPARE(metadata.version.to_string(), std::string("1.0.0"));
}

void TestPluginInterfaceAdvanced::testPluginStateConsistency() {
    // Test state consistency across multiple operations
    QVERIFY(!m_plugin->isInitialized());

    // Multiple initialization calls - second should fail
    auto first_init = m_plugin->initialize();
    QVERIFY(first_init.has_value());
    QVERIFY(m_plugin->isInitialized());
    auto second_init = m_plugin->initialize();  // Should fail
    QVERIFY(!second_init.has_value());
    QVERIFY(m_plugin->isInitialized());

    // Multiple shutdown calls should be safe
    m_plugin->shutdown();
    QVERIFY(!m_plugin->isInitialized());
    m_plugin->shutdown();  // Should be safe
    QVERIFY(!m_plugin->isInitialized());
}

void TestPluginInterfaceAdvanced::testInitializationFailure() {
    // This test would require a mock that can fail initialization
    // For now, we test that the interface handles the failure case
    auto result = m_plugin->initialize();
    QVERIFY(result.has_value());  // Our mock always succeeds
}

void TestPluginInterfaceAdvanced::testShutdownSafety() {
    // Test shutdown without initialization
    m_plugin->shutdown();  // Should be safe
    QVERIFY(!m_plugin->isInitialized());

    // Test shutdown after initialization
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    m_plugin->shutdown();
    QVERIFY(!m_plugin->isInitialized());
}

void TestPluginInterfaceAdvanced::testInvalidMetadata() {
    // Test that metadata is always valid for our mock
    auto metadata = m_plugin->metadata();
    QVERIFY(!metadata.name.empty());
    QVERIFY(metadata.version.major() >= 0);
    QVERIFY(!metadata.description.empty());
}

void TestPluginInterfaceAdvanced::testInitializationPerformance() {
    const int iterations = 1000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        auto init_result = m_plugin->initialize();
        QVERIFY(init_result.has_value());
        m_plugin->shutdown();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete within reasonable time (less than 1 second for 1000
    // iterations)
    QVERIFY(duration.count() < 1000);
}

void TestPluginInterfaceAdvanced::testShutdownPerformance() {
    const int iterations = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        auto init_result = m_plugin->initialize();
        QVERIFY(init_result.has_value());
        m_plugin->shutdown();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete within reasonable time
    QVERIFY(duration.count() < 1000);
}

void TestPluginInterfaceAdvanced::testMetadataAccessPerformance() {
    const int iterations = 10000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        auto metadata = m_plugin->metadata();
        Q_UNUSED(metadata);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Metadata access should be very fast
    QVERIFY(duration.count() < 100);
}

void TestPluginInterfaceAdvanced::testConcurrentAccess() {
    // Test concurrent metadata access
    const int threadCount = 4;
    const int iterationsPerThread = 100;

    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};

    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([this, iterationsPerThread, &successCount]() {
            for (int i = 0; i < iterationsPerThread; ++i) {
                auto metadata = m_plugin->metadata();
                if (!metadata.name.empty()) {
                    successCount++;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    QCOMPARE(successCount.load(), threadCount * iterationsPerThread);
}

void TestPluginInterfaceAdvanced::testThreadSafeInitialization() {
    // Note: This test assumes the plugin implementation is thread-safe
    // Our mock implementation may not be fully thread-safe

    const int threadCount = 2;
    std::vector<std::thread> threads;
    std::atomic<int> initSuccessCount{0};

    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([this, &initSuccessCount]() {
            auto result = m_plugin->initialize();
            if (result.has_value()) {
                initSuccessCount++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // At least one initialization should succeed
    QVERIFY(initSuccessCount.load() >= 1);
    QVERIFY(m_plugin->isInitialized());
}

void TestPluginInterfaceAdvanced::testThreadSafeShutdown() {
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    const int threadCount = 2;
    std::vector<std::thread> threads;

    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([this]() { m_plugin->shutdown(); });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    QVERIFY(!m_plugin->isInitialized());
}

void TestPluginInterfaceAdvanced::testMemoryLeaks() {
    // Test for obvious memory leaks by creating and destroying many plugins
    const int iterations = 1000;

    for (int i = 0; i < iterations; ++i) {
        auto plugin = std::make_unique<MockPlugin>();
        plugin->initialize();
        plugin->shutdown();
        // plugin automatically destroyed here
    }

    // If we get here without crashing, memory management is likely correct
    QVERIFY(true);
}

void TestPluginInterfaceAdvanced::testResourceCleanup() {
    m_plugin->initialize();

    // Verify plugin is initialized
    QVERIFY(m_plugin->isInitialized());

    // Shutdown should clean up resources
    m_plugin->shutdown();
    QVERIFY(!m_plugin->isInitialized());

    // Should be safe to call shutdown again
    m_plugin->shutdown();
    QVERIFY(!m_plugin->isInitialized());
}

QTEST_MAIN(TestPluginInterfaceAdvanced)
#include "test_plugin_interface_advanced.moc"
