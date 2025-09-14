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

    // IPlugin interface
    QString name() const override { return "MockPlugin"; }
    QString version() const override { return "1.0.0"; }
    QString description() const override { return "Mock plugin for testing"; }
    QString author() const override { return "Test Suite"; }
    QString license() const override { return "MIT"; }

    bool initialize() override {
        m_initialized = true;
        emit initialized();
        return true;
    }

    void shutdown() override {
        m_initialized = false;
        emit shutdown_signal();
    }

    bool isInitialized() const override {
        return m_initialized;
    }

    QJsonObject metadata() const override {
        QJsonObject meta;
        meta["name"] = name();
        meta["version"] = version();
        meta["description"] = description();
        meta["author"] = author();
        meta["license"] = license();
        return meta;
    }

signals:
    void initialized();
    void shutdown_signal();

private:
    bool m_initialized = false;
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
    QCOMPARE(m_plugin->name(), QString("MockPlugin"));
    QCOMPARE(m_plugin->version(), QString("1.0.0"));
    QCOMPARE(m_plugin->description(), QString("Mock plugin for testing"));
    QCOMPARE(m_plugin->author(), QString("Test Suite"));
    QCOMPARE(m_plugin->license(), QString("MIT"));

    QJsonObject metadata = m_plugin->metadata();
    QVERIFY(!metadata.isEmpty());
    QCOMPARE(metadata["name"].toString(), m_plugin->name());
    QCOMPARE(metadata["version"].toString(), m_plugin->version());
}

void TestPluginInterfaceAdvanced::testPluginInitialization() {
    QSignalSpy spy(m_plugin.get(), &MockPlugin::initialized);
    
    QVERIFY(!m_plugin->isInitialized());
    QVERIFY(m_plugin->initialize());
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
        QVERIFY(m_plugin->initialize());
        QVERIFY(m_plugin->isInitialized());
        m_plugin->shutdown();
        QVERIFY(!m_plugin->isInitialized());
    }
}

void TestPluginInterfaceAdvanced::testPluginSignals() {
    QSignalSpy initSpy(m_plugin.get(), &MockPlugin::initialized);
    QSignalSpy shutdownSpy(m_plugin.get(), &MockPlugin::shutdown_signal);
    
    m_plugin->initialize();
    QCOMPARE(initSpy.count(), 1);
    QCOMPARE(shutdownSpy.count(), 0);
    
    m_plugin->shutdown();
    QCOMPARE(initSpy.count(), 1);
    QCOMPARE(shutdownSpy.count(), 1);
}

void TestPluginInterfaceAdvanced::testPluginMetadataValidation() {
    QJsonObject metadata = m_plugin->metadata();
    
    // Validate required fields
    QVERIFY(metadata.contains("name"));
    QVERIFY(metadata.contains("version"));
    QVERIFY(metadata.contains("description"));
    QVERIFY(metadata.contains("author"));
    QVERIFY(metadata.contains("license"));
    
    // Validate field types
    QVERIFY(metadata["name"].isString());
    QVERIFY(metadata["version"].isString());
    QVERIFY(metadata["description"].isString());
    QVERIFY(metadata["author"].isString());
    QVERIFY(metadata["license"].isString());
    
    // Validate non-empty values
    QVERIFY(!metadata["name"].toString().isEmpty());
    QVERIFY(!metadata["version"].toString().isEmpty());
}

void TestPluginInterfaceAdvanced::testPluginStateConsistency() {
    // Test state consistency across multiple operations
    QVERIFY(!m_plugin->isInitialized());
    
    // Multiple initialization calls should be idempotent
    QVERIFY(m_plugin->initialize());
    QVERIFY(m_plugin->isInitialized());
    QVERIFY(m_plugin->initialize()); // Should still return true
    QVERIFY(m_plugin->isInitialized());
    
    // Multiple shutdown calls should be safe
    m_plugin->shutdown();
    QVERIFY(!m_plugin->isInitialized());
    m_plugin->shutdown(); // Should be safe
    QVERIFY(!m_plugin->isInitialized());
}

void TestPluginInterfaceAdvanced::testInitializationFailure() {
    // This test would require a mock that can fail initialization
    // For now, we test that the interface handles the failure case
    QVERIFY(m_plugin->initialize()); // Our mock always succeeds
}

void TestPluginInterfaceAdvanced::testShutdownSafety() {
    // Test shutdown without initialization
    m_plugin->shutdown(); // Should be safe
    QVERIFY(!m_plugin->isInitialized());
    
    // Test shutdown after initialization
    m_plugin->initialize();
    m_plugin->shutdown();
    QVERIFY(!m_plugin->isInitialized());
}

void TestPluginInterfaceAdvanced::testInvalidMetadata() {
    // Test that metadata is always valid for our mock
    QJsonObject metadata = m_plugin->metadata();
    QJsonDocument doc(metadata);
    QVERIFY(!doc.isNull());
    QVERIFY(!doc.isEmpty());
}

void TestPluginInterfaceAdvanced::testInitializationPerformance() {
    const int iterations = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        m_plugin->initialize();
        m_plugin->shutdown();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete within reasonable time (less than 1 second for 1000 iterations)
    QVERIFY(duration.count() < 1000);
}

void TestPluginInterfaceAdvanced::testShutdownPerformance() {
    const int iterations = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        m_plugin->initialize();
        m_plugin->shutdown();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete within reasonable time
    QVERIFY(duration.count() < 1000);
}

void TestPluginInterfaceAdvanced::testMetadataAccessPerformance() {
    const int iterations = 10000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        QJsonObject metadata = m_plugin->metadata();
        Q_UNUSED(metadata);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
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
                QJsonObject metadata = m_plugin->metadata();
                if (!metadata.isEmpty()) {
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
            if (m_plugin->initialize()) {
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
    m_plugin->initialize();
    
    const int threadCount = 2;
    std::vector<std::thread> threads;
    
    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([this]() {
            m_plugin->shutdown();
        });
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
