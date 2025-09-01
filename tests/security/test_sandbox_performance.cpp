/**
 * @file test_sandbox_performance.cpp
 * @brief Performance tests and benchmarks for the sandbox system
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QTimer>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

#include "qtplugin/security/sandbox/plugin_sandbox.hpp"

using namespace qtplugin;

class TestSandboxPerformance : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Initialization performance tests
    void testSandboxInitializationTime();
    void testManagerInitializationTime();
    void testResourceMonitorInitializationTime();

    // Execution performance tests
    void testPluginExecutionOverhead();
    void testResourceMonitoringOverhead();
    void testSecurityEnforcementOverhead();

    // Scalability tests
    void testMultipleSandboxPerformance();
    void testConcurrentExecutionPerformance();
    void testLargePolicySetPerformance();

    // Memory usage tests
    void testSandboxMemoryFootprint();
    void testManagerMemoryFootprint();
    void testMemoryLeakDetection();

    // Throughput tests
    void testPluginExecutionThroughput();
    void testSandboxCreationThroughput();
    void testPolicyValidationThroughput();

    // Stress tests
    void testHighFrequencyOperations();
    void testLongRunningMonitoring();
    void testResourceExhaustionHandling();

    // Cleanup performance tests
    void testSandboxShutdownTime();
    void testResourceCleanupTime();

private:
    QTemporaryDir* m_temp_dir;
    SandboxManager* m_manager;
    QStringList m_created_sandboxes;
    
    struct PerformanceMetrics {
        qint64 min_time = LLONG_MAX;
        qint64 max_time = 0;
        qint64 total_time = 0;
        int count = 0;
        
        void add_measurement(qint64 time) {
            min_time = std::min(min_time, time);
            max_time = std::max(max_time, time);
            total_time += time;
            count++;
        }
        
        double average() const {
            return count > 0 ? static_cast<double>(total_time) / count : 0.0;
        }
    };
    
    SecurityPolicy createLightweightPolicy();
    QString createSimpleTestPlugin();
    void cleanupCreatedSandboxes();
    void printMetrics(const QString& test_name, const PerformanceMetrics& metrics);
};

void TestSandboxPerformance::initTestCase() {
    // Initialize Qt application for testing
    if (!QCoreApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        new QCoreApplication(argc, argv);
    }
    
    m_temp_dir = new QTemporaryDir();
    QVERIFY(m_temp_dir->isValid());
}

void TestSandboxPerformance::cleanupTestCase() {
    delete m_temp_dir;
}

void TestSandboxPerformance::init() {
    m_manager = &SandboxManager::instance();
    m_created_sandboxes.clear();
}

void TestSandboxPerformance::cleanup() {
    cleanupCreatedSandboxes();
}

void TestSandboxPerformance::testSandboxInitializationTime() {
    const int iterations = 100;
    PerformanceMetrics metrics;
    
    SecurityPolicy policy = createLightweightPolicy();
    
    for (int i = 0; i < iterations; ++i) {
        QElapsedTimer timer;
        timer.start();
        
        auto sandbox = std::make_unique<PluginSandbox>(policy);
        auto result = sandbox->initialize();
        
        qint64 elapsed = timer.elapsed();
        
        QVERIFY(result.has_value());
        metrics.add_measurement(elapsed);
        
        sandbox->shutdown();
    }
    
    printMetrics("Sandbox Initialization", metrics);
    
    // Performance expectations
    QVERIFY(metrics.average() < 50.0); // Less than 50ms average
    QVERIFY(metrics.max_time < 200); // Less than 200ms maximum
}

void TestSandboxPerformance::testManagerInitializationTime() {
    QElapsedTimer timer;
    timer.start();
    
    // Access singleton (should be already initialized, but measure anyway)
    SandboxManager& manager = SandboxManager::instance();
    Q_UNUSED(manager);
    
    qint64 elapsed = timer.elapsed();
    
    qDebug() << "Manager initialization time:" << elapsed << "ms";
    
    // Should be very fast since it's a singleton
    QVERIFY(elapsed < 10);
}

void TestSandboxPerformance::testPluginExecutionOverhead() {
    SecurityPolicy policy = createLightweightPolicy();
    QString sandbox_id = QString("perf_test_%1").arg(QDateTime::currentMSecsSinceEpoch());
    
    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);
    
    auto sandbox = sandbox_result.value();
    
    QString plugin_path = createSimpleTestPlugin();
    if (plugin_path.isEmpty()) {
        QSKIP("Could not create test plugin");
    }
    
    const int iterations = 10;
    PerformanceMetrics metrics;
    
    for (int i = 0; i < iterations; ++i) {
        QElapsedTimer timer;
        timer.start();
        
        auto exec_result = sandbox->execute_plugin(plugin_path, PluginType::Python);
        
        if (exec_result.has_value()) {
            // Wait for completion
            QSignalSpy spy(sandbox.get(), &PluginSandbox::execution_completed);
            bool completed = spy.wait(5000);
            
            qint64 elapsed = timer.elapsed();
            
            if (completed) {
                metrics.add_measurement(elapsed);
            }
        } else {
            QSKIP("Plugin execution not available");
        }
    }
    
    if (metrics.count > 0) {
        printMetrics("Plugin Execution", metrics);
        
        // Performance expectations
        QVERIFY(metrics.average() < 1000.0); // Less than 1 second average
    }
}

void TestSandboxPerformance::testMultipleSandboxPerformance() {
    const int sandbox_count = 20;
    PerformanceMetrics creation_metrics;
    PerformanceMetrics retrieval_metrics;
    
    SecurityPolicy policy = createLightweightPolicy();
    QStringList sandbox_ids;
    
    // Test creation performance
    for (int i = 0; i < sandbox_count; ++i) {
        QString sandbox_id = QString("multi_perf_%1_%2").arg(i).arg(QDateTime::currentMSecsSinceEpoch());
        
        QElapsedTimer timer;
        timer.start();
        
        auto result = m_manager->create_sandbox(sandbox_id, policy);
        
        qint64 elapsed = timer.elapsed();
        
        QVERIFY(result.has_value());
        creation_metrics.add_measurement(elapsed);
        
        sandbox_ids.append(sandbox_id);
        m_created_sandboxes.append(sandbox_id);
    }
    
    // Test retrieval performance
    for (const QString& sandbox_id : sandbox_ids) {
        QElapsedTimer timer;
        timer.start();
        
        auto sandbox = m_manager->get_sandbox(sandbox_id);
        
        qint64 elapsed = timer.elapsed();
        
        QVERIFY(sandbox != nullptr);
        retrieval_metrics.add_measurement(elapsed);
    }
    
    printMetrics("Multiple Sandbox Creation", creation_metrics);
    printMetrics("Multiple Sandbox Retrieval", retrieval_metrics);
    
    // Performance expectations
    QVERIFY(creation_metrics.average() < 100.0); // Less than 100ms average creation
    QVERIFY(retrieval_metrics.average() < 5.0);  // Less than 5ms average retrieval
}

void TestSandboxPerformance::testConcurrentExecutionPerformance() {
    const int thread_count = 4;
    const int operations_per_thread = 5;
    
    std::vector<std::thread> threads;
    std::vector<PerformanceMetrics> thread_metrics(thread_count);
    
    SecurityPolicy policy = createLightweightPolicy();
    
    QElapsedTimer total_timer;
    total_timer.start();
    
    // Create threads that perform concurrent operations
    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([this, t, operations_per_thread, policy, &thread_metrics]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                QElapsedTimer timer;
                timer.start();
                
                QString sandbox_id = QString("concurrent_%1_%2_%3")
                    .arg(t).arg(i).arg(QDateTime::currentMSecsSinceEpoch());
                
                auto result = m_manager->create_sandbox(sandbox_id, policy);
                
                qint64 elapsed = timer.elapsed();
                
                if (result.has_value()) {
                    thread_metrics[t].add_measurement(elapsed);
                    
                    // Cleanup immediately to avoid resource buildup
                    m_manager->remove_sandbox(sandbox_id);
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    qint64 total_time = total_timer.elapsed();
    
    // Calculate overall metrics
    PerformanceMetrics overall_metrics;
    for (const auto& metrics : thread_metrics) {
        for (int i = 0; i < metrics.count; ++i) {
            // We can't get individual measurements, so use average
            overall_metrics.add_measurement(static_cast<qint64>(metrics.average()));
        }
    }
    
    qDebug() << "Concurrent execution total time:" << total_time << "ms";
    printMetrics("Concurrent Operations", overall_metrics);
    
    // Performance expectations
    QVERIFY(total_time < 5000); // Less than 5 seconds total
    QVERIFY(overall_metrics.average() < 200.0); // Less than 200ms average per operation
}

void TestSandboxPerformance::testSandboxMemoryFootprint() {
    // This is a simplified memory test - in a real scenario you'd use more sophisticated tools
    const int sandbox_count = 10;
    SecurityPolicy policy = createLightweightPolicy();
    
    // Get baseline memory usage
    size_t baseline_memory = 0; // Would need platform-specific memory measurement
    
    // Create sandboxes and measure memory growth
    for (int i = 0; i < sandbox_count; ++i) {
        QString sandbox_id = QString("memory_test_%1_%2").arg(i).arg(QDateTime::currentMSecsSinceEpoch());
        
        auto result = m_manager->create_sandbox(sandbox_id, policy);
        QVERIFY(result.has_value());
        m_created_sandboxes.append(sandbox_id);
    }
    
    size_t after_creation_memory = 0; // Would need platform-specific memory measurement
    
    // Calculate memory per sandbox (simplified)
    qDebug() << "Created" << sandbox_count << "sandboxes";
    qDebug() << "Memory baseline:" << baseline_memory << "bytes";
    qDebug() << "Memory after creation:" << after_creation_memory << "bytes";
    
    // In a real test, we would verify memory usage is reasonable
    // For now, just verify all sandboxes were created successfully
    auto active_sandboxes = m_manager->get_active_sandboxes();
    QVERIFY(active_sandboxes.size() >= sandbox_count);
}

void TestSandboxPerformance::testPluginExecutionThroughput() {
    SecurityPolicy policy = createLightweightPolicy();
    QString sandbox_id = QString("throughput_test_%1").arg(QDateTime::currentMSecsSinceEpoch());
    
    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);
    
    auto sandbox = sandbox_result.value();
    
    QString plugin_path = createSimpleTestPlugin();
    if (plugin_path.isEmpty()) {
        QSKIP("Could not create test plugin");
    }
    
    const int execution_count = 5;
    QElapsedTimer total_timer;
    total_timer.start();
    
    int successful_executions = 0;
    
    for (int i = 0; i < execution_count; ++i) {
        auto exec_result = sandbox->execute_plugin(plugin_path, PluginType::Python);
        
        if (exec_result.has_value()) {
            QSignalSpy spy(sandbox.get(), &PluginSandbox::execution_completed);
            if (spy.wait(5000)) {
                successful_executions++;
            }
        } else {
            QSKIP("Plugin execution not available");
        }
    }
    
    qint64 total_time = total_timer.elapsed();
    
    if (successful_executions > 0) {
        double throughput = (static_cast<double>(successful_executions) / total_time) * 1000.0; // executions per second
        qDebug() << "Plugin execution throughput:" << throughput << "executions/second";
        qDebug() << "Total time for" << successful_executions << "executions:" << total_time << "ms";
        
        // Performance expectation
        QVERIFY(throughput > 0.5); // At least 0.5 executions per second
    }
}

void TestSandboxPerformance::testSandboxCreationThroughput() {
    const int creation_count = 50;
    SecurityPolicy policy = createLightweightPolicy();
    
    QElapsedTimer timer;
    timer.start();
    
    int successful_creations = 0;
    
    for (int i = 0; i < creation_count; ++i) {
        QString sandbox_id = QString("throughput_create_%1_%2").arg(i).arg(QDateTime::currentMSecsSinceEpoch());
        
        auto result = m_manager->create_sandbox(sandbox_id, policy);
        if (result.has_value()) {
            successful_creations++;
            m_created_sandboxes.append(sandbox_id);
        }
    }
    
    qint64 total_time = timer.elapsed();
    
    double throughput = (static_cast<double>(successful_creations) / total_time) * 1000.0; // creations per second
    qDebug() << "Sandbox creation throughput:" << throughput << "creations/second";
    qDebug() << "Total time for" << successful_creations << "creations:" << total_time << "ms";
    
    // Performance expectations
    QVERIFY(throughput > 10.0); // At least 10 creations per second
    QCOMPARE(successful_creations, creation_count);
}

void TestSandboxPerformance::testSandboxShutdownTime() {
    const int sandbox_count = 20;
    SecurityPolicy policy = createLightweightPolicy();
    QStringList sandbox_ids;
    
    // Create sandboxes
    for (int i = 0; i < sandbox_count; ++i) {
        QString sandbox_id = QString("shutdown_test_%1_%2").arg(i).arg(QDateTime::currentMSecsSinceEpoch());
        
        auto result = m_manager->create_sandbox(sandbox_id, policy);
        QVERIFY(result.has_value());
        sandbox_ids.append(sandbox_id);
    }
    
    // Measure shutdown time
    QElapsedTimer timer;
    timer.start();
    
    for (const QString& sandbox_id : sandbox_ids) {
        m_manager->remove_sandbox(sandbox_id);
    }
    
    qint64 shutdown_time = timer.elapsed();
    
    qDebug() << "Shutdown time for" << sandbox_count << "sandboxes:" << shutdown_time << "ms";
    qDebug() << "Average shutdown time per sandbox:" << (static_cast<double>(shutdown_time) / sandbox_count) << "ms";
    
    // Performance expectations
    QVERIFY(shutdown_time < 1000); // Less than 1 second total
    QVERIFY((static_cast<double>(shutdown_time) / sandbox_count) < 50.0); // Less than 50ms per sandbox
}

SecurityPolicy TestSandboxPerformance::createLightweightPolicy() {
    SecurityPolicy policy;
    policy.level = SandboxSecurityLevel::Limited;
    policy.policy_name = "lightweight_test";
    policy.description = "Lightweight policy for performance testing";
    
    // Set reasonable limits that won't be hit during testing
    policy.limits.cpu_time_limit = std::chrono::minutes(5);
    policy.limits.memory_limit_mb = 512;
    policy.limits.disk_space_limit_mb = 100;
    policy.limits.max_file_handles = 100;
    policy.limits.max_network_connections = 20;
    policy.limits.execution_timeout = std::chrono::minutes(1);
    
    // Allow basic operations
    policy.permissions.allow_file_system_read = true;
    policy.permissions.allow_file_system_write = false;
    policy.permissions.allow_network_access = false;
    policy.permissions.allow_process_creation = false;
    policy.permissions.allow_system_calls = false;
    policy.permissions.allow_registry_access = false;
    policy.permissions.allow_environment_access = false;
    
    return policy;
}

QString TestSandboxPerformance::createSimpleTestPlugin() {
    QTemporaryFile* temp_file = new QTemporaryFile(m_temp_dir->path() + "/perf_plugin_XXXXXX.py", this);
    temp_file->setAutoRemove(false);
    
    if (temp_file->open()) {
        QTextStream stream(temp_file);
        stream << "#!/usr/bin/env python3\n";
        stream << "import sys\n";
        stream << "print('Performance test plugin')\n";
        stream << "sys.exit(0)\n";
        
        QString file_path = temp_file->fileName();
        temp_file->close();
        
        // Make executable on Unix systems
        QFile::setPermissions(file_path, 
                             QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
        
        return file_path;
    }
    
    return QString();
}

void TestSandboxPerformance::cleanupCreatedSandboxes() {
    for (const QString& sandbox_id : m_created_sandboxes) {
        m_manager->remove_sandbox(sandbox_id);
    }
    m_created_sandboxes.clear();
}

void TestSandboxPerformance::printMetrics(const QString& test_name, const PerformanceMetrics& metrics) {
    qDebug() << "=== Performance Metrics for" << test_name << "===";
    qDebug() << "Count:" << metrics.count;
    qDebug() << "Average:" << metrics.average() << "ms";
    qDebug() << "Min:" << metrics.min_time << "ms";
    qDebug() << "Max:" << metrics.max_time << "ms";
    qDebug() << "Total:" << metrics.total_time << "ms";
    qDebug() << "==========================================";
}

QTEST_MAIN(TestSandboxPerformance)
#include "test_sandbox_performance.moc"
