/**
 * @file test_metrics_collection_advanced.cpp
 * @brief Advanced tests for plugin metrics collection
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>

#include <qtplugin/monitoring/plugin_metrics_collector.hpp>
#include <qtplugin/core/plugin_manager.hpp>
#include <qtplugin/core/plugin_registry.hpp>
#include <qtplugin/core/plugin_interface.hpp>

#include <memory>
#include <chrono>
#include <atomic>
#include <thread>

using namespace qtplugin;

// Mock plugin for testing metrics collection
class MockMetricsPlugin : public IPlugin
{
public:
    MockMetricsPlugin() : m_command_count(0), m_error_count(0) {}

    std::string id() const override { return "mock_metrics_plugin"; }
    std::string name() const override { return "Mock Metrics Plugin"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override { return "Plugin for testing metrics collection"; }

    PluginCapabilities capabilities() const override {
        return static_cast<PluginCapabilities>(
            PluginCapability::Monitoring |
            PluginCapability::Configuration
        );
    }

    qtplugin::expected<void, PluginError> initialize() override {
        m_initialized = true;
        return make_success();
    }

    qtplugin::expected<void, PluginError> shutdown() override {
        m_initialized = false;
        return make_success();
    }

    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override {

        m_command_count.fetch_add(1);

        if (command == "get_metrics") {
            QJsonObject metrics;
            metrics["command_count"] = static_cast<int>(m_command_count.load());
            metrics["error_count"] = static_cast<int>(m_error_count.load());
            metrics["uptime_ms"] = static_cast<qint64>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - m_start_time
                ).count()
            );
            metrics["memory_usage_kb"] = 1024; // Mock memory usage
            metrics["cpu_usage_percent"] = 5.5; // Mock CPU usage
            return metrics;
        } else if (command == "simulate_error") {
            m_error_count.fetch_add(1);
            return qtplugin::make_error<QJsonObject>(
                PluginErrorCode::ExecutionFailed, "Simulated error"
            );
        } else if (command == "heavy_operation") {
            // Simulate heavy operation
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return QJsonObject{{"result", "heavy_operation_completed"}};
        }

        return qtplugin::make_error<QJsonObject>(
            PluginErrorCode::CommandNotFound, "Unknown command"
        );
    }

    // Test helper methods
    int getCommandCount() const { return m_command_count.load(); }
    int getErrorCount() const { return m_error_count.load(); }
    bool isInitialized() const { return m_initialized; }

private:
    std::atomic<int> m_command_count;
    std::atomic<int> m_error_count;
    bool m_initialized = false;
    std::chrono::steady_clock::time_point m_start_time = std::chrono::steady_clock::now();
};

class MetricsCollectionComprehensiveTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic metrics collection functionality
    void testMetricsCollectorCreation();
    void testStartStopMonitoring();
    void testMetricsCollectionInterval();
    void testPluginMetricsUpdate();

    // Plugin-specific metrics
    void testPluginSpecificMetrics();
    void testMetricsFromPluginCommands();
    void testMetricsErrorHandling();
    void testMetricsWithPluginErrors();

    // System-wide metrics
    void testSystemMetricsCollection();
    void testMultiplePluginMetrics();
    void testMetricsAggregation();
    void testMetricsHistory();

    // Performance and reliability
    void testMetricsCollectionPerformance();
    void testHighFrequencyMetricsCollection();
    void testMetricsCollectionUnderLoad();
    void testMemoryUsageDuringCollection();

    // Integration scenarios
    void testMetricsWithPluginLifecycle();
    void testMetricsWithHotReload();
    void testMetricsExportAndReporting();

    // Error handling and edge cases
    void testMetricsCollectionWithInvalidPlugins();
    void testMetricsCollectionFailureRecovery();
    void testConcurrentMetricsAccess();

private:
    std::unique_ptr<PluginMetricsCollector> m_metrics_collector;
    std::unique_ptr<PluginManager> m_plugin_manager;
    std::unique_ptr<PluginRegistry> m_plugin_registry;
    std::shared_ptr<MockMetricsPlugin> m_mock_plugin;

    void setupMockPlugin();
    bool waitForMetricsUpdate(int timeout_ms = 5000);
};

void MetricsCollectionComprehensiveTest::initTestCase()
{
    qDebug() << "Starting comprehensive metrics collection tests";
}

void MetricsCollectionComprehensiveTest::cleanupTestCase()
{
    qDebug() << "Comprehensive metrics collection tests completed";
}

void MetricsCollectionComprehensiveTest::init()
{
    // Create fresh instances for each test
    m_metrics_collector = std::make_unique<PluginMetricsCollector>();
    m_plugin_manager = std::make_unique<PluginManager>();
    m_plugin_registry = std::make_unique<PluginRegistry>();
    m_mock_plugin = std::make_shared<MockMetricsPlugin>();

    setupMockPlugin();
}

void MetricsCollectionComprehensiveTest::cleanup()
{
    // Stop monitoring and clean up
    if (m_metrics_collector && m_metrics_collector->is_monitoring_active()) {
        m_metrics_collector->stop_monitoring();
    }

    m_mock_plugin.reset();
    m_plugin_registry.reset();
    m_plugin_manager.reset();
    m_metrics_collector.reset();
}

void MetricsCollectionComprehensiveTest::testMetricsCollectorCreation()
{
    // Test basic creation and initialization
    QVERIFY(m_metrics_collector != nullptr);

    // Test initial state
    QVERIFY(!m_metrics_collector->is_monitoring_active());

    // Test metrics retrieval when not active
    auto metrics = m_metrics_collector->get_system_metrics();
    QVERIFY(!metrics.isEmpty());
    QVERIFY(metrics.contains("monitoring_active"));
    QCOMPARE(metrics["monitoring_active"].toBool(), false);
}

void MetricsCollectionComprehensiveTest::testStartStopMonitoring()
{
    // Test starting monitoring
    auto start_result = m_metrics_collector->start_monitoring(std::chrono::milliseconds(1000));
    QVERIFY(start_result.has_value());
    QVERIFY(m_metrics_collector->is_monitoring_active());

    // Test stopping monitoring
    auto stop_result = m_metrics_collector->stop_monitoring();
    QVERIFY(stop_result.has_value());
    QVERIFY(!m_metrics_collector->is_monitoring_active());
}

void MetricsCollectionComprehensiveTest::testPluginMetricsUpdate()
{
    // Start monitoring
    auto start_result = m_metrics_collector->start_monitoring(std::chrono::milliseconds(500));
    QVERIFY(start_result.has_value());

    // Set up signal spy for metrics updates
    QSignalSpy spy(m_metrics_collector.get(), &PluginMetricsCollector::plugin_metrics_updated);

    // Update metrics for mock plugin
    auto update_result = m_metrics_collector->update_plugin_metrics(
        m_mock_plugin->id(), m_plugin_registry.get()
    );
    QVERIFY(update_result.has_value());

    // Verify signal was emitted
    QVERIFY(spy.wait(2000));
    QCOMPARE(spy.count(), 1);

    // Verify signal parameters
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QString::fromStdString(m_mock_plugin->id()));
}

void MetricsCollectionComprehensiveTest::testPluginSpecificMetrics()
{
    // Execute some commands on the mock plugin to generate metrics
    auto result1 = m_mock_plugin->execute_command("get_metrics");
    QVERIFY(result1.has_value());

    auto result2 = m_mock_plugin->execute_command("heavy_operation");
    QVERIFY(result2.has_value());

    // Get metrics from plugin
    auto metrics_result = m_mock_plugin->execute_command("get_metrics");
    QVERIFY(metrics_result.has_value());

    QJsonObject metrics = metrics_result.value();
    QVERIFY(metrics.contains("command_count"));
    QVERIFY(metrics.contains("uptime_ms"));
    QVERIFY(metrics.contains("memory_usage_kb"));
    QVERIFY(metrics.contains("cpu_usage_percent"));

    // Verify command count increased
    QVERIFY(metrics["command_count"].toInt() >= 2);

    qDebug() << "Plugin metrics:" << QJsonDocument(metrics).toJson(QJsonDocument::Compact);
}

void MetricsCollectionComprehensiveTest::testMetricsCollectionPerformance()
{
    // Start high-frequency monitoring
    auto start_result = m_metrics_collector->start_monitoring(std::chrono::milliseconds(100));
    QVERIFY(start_result.has_value());

    // Measure performance of metrics collection
    QElapsedTimer timer;
    timer.start();

    const int collection_cycles = 10;
    QSignalSpy spy(m_metrics_collector.get(), &PluginMetricsCollector::system_metrics_updated);

    // Wait for multiple collection cycles
    while (spy.count() < collection_cycles && timer.elapsed() < 5000) {
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }

    qint64 elapsed = timer.elapsed();

    // Performance expectations
    QVERIFY(elapsed < 3000); // Should complete within 3 seconds
    QVERIFY(spy.count() >= collection_cycles);

    double avg_cycle_time = static_cast<double>(elapsed) / spy.count();
    QVERIFY(avg_cycle_time < 200); // Each cycle should be under 200ms

    qDebug() << "Metrics collection performance:" << elapsed << "ms for" << spy.count() << "cycles";
    qDebug() << "Average cycle time:" << avg_cycle_time << "ms";
}

void MetricsCollectionComprehensiveTest::testSystemMetricsCollection()
{
    // Start monitoring
    auto start_result = m_metrics_collector->start_monitoring(std::chrono::milliseconds(500));
    QVERIFY(start_result.has_value());

    // Get system metrics
    auto system_metrics = m_metrics_collector->get_system_metrics();
    QVERIFY(!system_metrics.isEmpty());

    // Verify required system metrics fields
    QVERIFY(system_metrics.contains("monitoring_active"));
    QVERIFY(system_metrics.contains("monitoring_interval_ms"));
    QCOMPARE(system_metrics["monitoring_active"].toBool(), true);
    QCOMPARE(system_metrics["monitoring_interval_ms"].toInt(), 500);

    qDebug() << "System metrics:" << QJsonDocument(system_metrics).toJson(QJsonDocument::Compact);
}

void MetricsCollectionComprehensiveTest::testMetricsErrorHandling()
{
    // Test metrics collection with plugin that generates errors
    auto error_result = m_mock_plugin->execute_command("simulate_error");
    QVERIFY(!error_result.has_value());
    QCOMPARE(error_result.error().code, PluginErrorCode::ExecutionFailed);

    // Verify error count increased
    QCOMPARE(m_mock_plugin->getErrorCount(), 1);

    // Test metrics collection continues despite errors
    auto start_result = m_metrics_collector->start_monitoring(std::chrono::milliseconds(500));
    QVERIFY(start_result.has_value());

    // Update metrics should still work
    auto update_result = m_metrics_collector->update_plugin_metrics(
        m_mock_plugin->id(), m_plugin_registry.get()
    );
    QVERIFY(update_result.has_value());
}

void MetricsCollectionComprehensiveTest::testConcurrentMetricsAccess()
{
    // Start monitoring
    auto start_result = m_metrics_collector->start_monitoring(std::chrono::milliseconds(200));
    QVERIFY(start_result.has_value());

    // Simulate concurrent access to metrics
    std::atomic<int> successful_reads{0};
    std::atomic<int> failed_reads{0};

    const int thread_count = 5;
    const int reads_per_thread = 10;

    std::vector<std::thread> threads;

    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < reads_per_thread; ++j) {
                try {
                    auto metrics = m_metrics_collector->get_system_metrics();
                    if (!metrics.isEmpty()) {
                        successful_reads.fetch_add(1);
                    } else {
                        failed_reads.fetch_add(1);
                    }
                } catch (...) {
                    failed_reads.fetch_add(1);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify concurrent access worked correctly
    int total_reads = successful_reads.load() + failed_reads.load();
    QCOMPARE(total_reads, thread_count * reads_per_thread);
    QVERIFY(successful_reads.load() > 0);

    // Most reads should succeed (allow some failures due to timing)
    double success_rate = static_cast<double>(successful_reads.load()) / total_reads;
    QVERIFY(success_rate > 0.8); // At least 80% success rate

    qDebug() << "Concurrent metrics access:" << successful_reads.load() << "successful,"
             << failed_reads.load() << "failed";
}

void MetricsCollectionComprehensiveTest::setupMockPlugin()
{
    // Initialize the mock plugin
    auto init_result = m_mock_plugin->initialize();
    QVERIFY(init_result.has_value());
    QVERIFY(m_mock_plugin->isInitialized());

    // Add plugin to registry (if registry supports it)
    // Note: This would require registry implementation details
}

bool MetricsCollectionComprehensiveTest::waitForMetricsUpdate(int timeout_ms)
{
    QSignalSpy spy(m_metrics_collector.get(), &PluginMetricsCollector::system_metrics_updated);
    return spy.wait(timeout_ms);
}

QTEST_MAIN(MetricsCollectionComprehensiveTest)
#include "test_metrics_collection_comprehensive.moc"
