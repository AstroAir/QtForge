/**
 * @file test_resource_monitor.cpp
 * @brief Tests for cross-platform resource monitoring functionality
 * @version 3.2.0
 */

#include <QCoreApplication>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest/QtTest>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>

// Include the resource monitor header (would need to be accessible)
// For testing, we'll create a mock or use the actual implementation
#include "../../src/security/sandbox/resource_monitor.hpp"

using namespace qtplugin;

class TestResourceMonitor : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testResourceMonitorInitialization();
    void testResourceMonitorShutdown();
    void testPlatformSupport();

    // Process monitoring tests
    void testProcessResourceUsage();
    void testSystemResourceUsage();
    void testInvalidProcessId();

    // Cross-platform tests
    void testWindowsResourceMonitoring();
    void testLinuxResourceMonitoring();
    void testMacOSResourceMonitoring();

    // Resource usage calculation tests
    void testCpuUsageCalculation();
    void testMemoryUsageCalculation();
    void testFileHandleTracking();
    void testNetworkConnectionTracking();

    // Performance tests
    void testMonitoringOverhead();
    void testConcurrentMonitoring();

    // Utility function tests
    void testResourceMonitorUtils();

private:
    std::unique_ptr<SandboxResourceMonitor> m_monitor;
    QProcess* m_test_process;

    void createTestProcess();
    void cleanupTestProcess();
    bool isCurrentPlatformSupported();
};

void TestResourceMonitor::initTestCase() {
    // Initialize Qt application for testing
    if (!QCoreApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        new QCoreApplication(argc, argv);
    }

    m_test_process = nullptr;
}

void TestResourceMonitor::cleanupTestCase() { cleanupTestProcess(); }

void TestResourceMonitor::init() {
    m_monitor = std::make_unique<SandboxResourceMonitor>();
}

void TestResourceMonitor::cleanup() {
    if (m_monitor) {
        m_monitor->shutdown();
        m_monitor.reset();
    }
    cleanupTestProcess();
}

void TestResourceMonitor::testResourceMonitorInitialization() {
    QVERIFY(m_monitor != nullptr);

    bool init_result = m_monitor->initialize();

    if (isCurrentPlatformSupported()) {
        QVERIFY(init_result);
    } else {
        // On unsupported platforms, initialization should fail gracefully
        QVERIFY(!init_result);
    }
}

void TestResourceMonitor::testResourceMonitorShutdown() {
    bool init_result = m_monitor->initialize();

    if (isCurrentPlatformSupported()) {
        QVERIFY(init_result);

        // Shutdown should be safe to call multiple times
        m_monitor->shutdown();
        m_monitor->shutdown();

        // Should be able to reinitialize after shutdown
        bool reinit_result = m_monitor->initialize();
        QVERIFY(reinit_result);
    }
}

void TestResourceMonitor::testPlatformSupport() {
    bool is_supported = SandboxResourceMonitor::is_supported();

#ifdef Q_OS_WIN
    QVERIFY(is_supported);
#elif defined(Q_OS_LINUX)
    QVERIFY(is_supported);
#elif defined(Q_OS_MACOS)
    QVERIFY(is_supported);
#else
    QVERIFY(!is_supported);
#endif
}

void TestResourceMonitor::testProcessResourceUsage() {
    if (!isCurrentPlatformSupported()) {
        QSKIP("Resource monitoring not supported on this platform");
    }

    bool init_result = m_monitor->initialize();
    QVERIFY(init_result);

    createTestProcess();
    QVERIFY(m_test_process != nullptr);
    QVERIFY(m_test_process->state() == QProcess::Running);

    qint64 pid = m_test_process->processId();
    QVERIFY(pid > 0);

    // Get resource usage for the test process
    ResourceUsage usage = m_monitor->get_process_usage(pid);

    // Verify that we got some meaningful data
    QVERIFY(usage.cpu_time_used >= std::chrono::milliseconds(0));
    // memory_used_mb is size_t (unsigned), so always >= 0
    QVERIFY(usage.file_handles_used >= 0);
    QVERIFY(usage.network_connections_used >= 0);

    // The start time should be recent
    auto now = std::chrono::steady_clock::now();
    auto time_diff = now - usage.start_time;
    QVERIFY(time_diff < std::chrono::seconds(10));
}

void TestResourceMonitor::testSystemResourceUsage() {
    if (!isCurrentPlatformSupported()) {
        QSKIP("Resource monitoring not supported on this platform");
    }

    bool init_result = m_monitor->initialize();
    QVERIFY(init_result);

    ResourceUsage system_usage = m_monitor->get_system_usage();

    // System should have some resource usage
    QVERIFY(system_usage.memory_used_mb > 0);
    QVERIFY(system_usage.cpu_time_used >= std::chrono::milliseconds(0));

    // Start time should be recent
    auto now = std::chrono::steady_clock::now();
    auto time_diff = now - system_usage.start_time;
    QVERIFY(time_diff < std::chrono::seconds(10));
}

void TestResourceMonitor::testInvalidProcessId() {
    if (!isCurrentPlatformSupported()) {
        QSKIP("Resource monitoring not supported on this platform");
    }

    bool init_result = m_monitor->initialize();
    QVERIFY(init_result);

    // Test with invalid PID
    ResourceUsage usage = m_monitor->get_process_usage(-1);

    // Should return empty/zero usage for invalid PID
    QCOMPARE(usage.memory_used_mb, static_cast<size_t>(0));
    QCOMPARE(usage.file_handles_used, 0);
    QCOMPARE(usage.network_connections_used, 0);
}

void TestResourceMonitor::testWindowsResourceMonitoring() {
#ifdef Q_OS_WIN
    bool init_result = m_monitor->initialize();
    QVERIFY(init_result);

    createTestProcess();
    QVERIFY(m_test_process != nullptr);

    qint64 pid = m_test_process->processId();
    ResourceUsage usage = m_monitor->get_process_usage(pid);

    // On Windows, we should get memory and handle information
    // memory_used_mb is size_t (unsigned), so always >= 0
    QVERIFY(usage.file_handles_used >= 0);

    // CPU time should be tracked
    QVERIFY(usage.cpu_time_used >= std::chrono::milliseconds(0));
#else
    QSKIP("Windows-specific test");
#endif
}

void TestResourceMonitor::testLinuxResourceMonitoring() {
#ifdef Q_OS_LINUX
    bool init_result = m_monitor->initialize();
    QVERIFY(init_result);

    createTestProcess();
    QVERIFY(m_test_process != nullptr);

    qint64 pid = m_test_process->processId();
    ResourceUsage usage = m_monitor->get_process_usage(pid);

    // On Linux, we should get information from /proc
    QVERIFY(usage.memory_used_mb >= 0);
    QVERIFY(usage.cpu_time_used >= std::chrono::milliseconds(0));

    // File descriptor count should be available
    QVERIFY(usage.file_handles_used >= 0);
#else
    QSKIP("Linux-specific test");
#endif
}

void TestResourceMonitor::testMacOSResourceMonitoring() {
#ifdef Q_OS_MACOS
    bool init_result = m_monitor->initialize();
    QVERIFY(init_result);

    createTestProcess();
    QVERIFY(m_test_process != nullptr);

    qint64 pid = m_test_process->processId();
    ResourceUsage usage = m_monitor->get_process_usage(pid);

    // On macOS, we should get resource information
    QVERIFY(usage.memory_used_mb >= 0);
    QVERIFY(usage.cpu_time_used >= std::chrono::milliseconds(0));
#else
    QSKIP("macOS-specific test");
#endif
}

void TestResourceMonitor::testCpuUsageCalculation() {
    // Test CPU usage percentage calculation
    std::chrono::milliseconds used_time(5000);
    std::chrono::milliseconds total_time(10000);

    double percentage =
        ResourceMonitorUtils::calculate_cpu_percentage(used_time, total_time);
    QCOMPARE(percentage, 50.0);

    // Test edge cases
    double zero_total = ResourceMonitorUtils::calculate_cpu_percentage(
        used_time, std::chrono::milliseconds(0));
    QCOMPARE(zero_total, 0.0);

    double zero_used = ResourceMonitorUtils::calculate_cpu_percentage(
        std::chrono::milliseconds(0), total_time);
    QCOMPARE(zero_used, 0.0);
}

void TestResourceMonitor::testMemoryUsageCalculation() {
    // Test memory usage percentage calculation
    size_t used_mb = 512;
    size_t total_mb = 1024;

    double percentage =
        ResourceMonitorUtils::calculate_memory_percentage(used_mb, total_mb);
    QCOMPARE(percentage, 50.0);

    // Test edge cases
    double zero_total =
        ResourceMonitorUtils::calculate_memory_percentage(used_mb, 0);
    QCOMPARE(zero_total, 0.0);

    double zero_used =
        ResourceMonitorUtils::calculate_memory_percentage(0, total_mb);
    QCOMPARE(zero_used, 0.0);
}

void TestResourceMonitor::testFileHandleTracking() {
    if (!isCurrentPlatformSupported()) {
        QSKIP("Resource monitoring not supported on this platform");
    }

    bool init_result = m_monitor->initialize();
    QVERIFY(init_result);

    createTestProcess();
    QVERIFY(m_test_process != nullptr);

    qint64 pid = m_test_process->processId();
    ResourceUsage usage = m_monitor->get_process_usage(pid);

    // Process should have at least some file handles (stdin, stdout, stderr)
    QVERIFY(usage.file_handles_used >= 3);
}

void TestResourceMonitor::testMonitoringOverhead() {
    if (!isCurrentPlatformSupported()) {
        QSKIP("Resource monitoring not supported on this platform");
    }

    bool init_result = m_monitor->initialize();
    QVERIFY(init_result);

    createTestProcess();
    QVERIFY(m_test_process != nullptr);

    qint64 pid = m_test_process->processId();

    // Measure time for multiple resource usage calls
    QElapsedTimer timer;
    timer.start();

    const int iterations = 100;
    for (int i = 0; i < iterations; ++i) {
        ResourceUsage usage = m_monitor->get_process_usage(pid);
        Q_UNUSED(usage);
    }

    qint64 elapsed = timer.elapsed();
    double avg_time = static_cast<double>(elapsed) / iterations;

    // Resource monitoring should be reasonably fast (less than 10ms per call on
    // average)
    QVERIFY(avg_time < 10.0);

    qDebug() << "Average resource monitoring time:" << avg_time << "ms";
}

void TestResourceMonitor::testResourceMonitorUtils() {
    // Test utility functions
    size_t bytes = 1024 * 1024 * 512;  // 512 MB
    size_t mb = ResourceMonitorUtils::bytes_to_mb(bytes);
    QCOMPARE(mb, static_cast<size_t>(512));

    std::chrono::milliseconds ms(5500);
    double seconds = ResourceMonitorUtils::ms_to_seconds(ms);
    QCOMPARE(seconds, 5.5);

    // Test threshold checking
    ResourceUsage usage;
    usage.memory_used_mb = 80;
    usage.cpu_time_used = std::chrono::milliseconds(4000);

    ResourceLimits limits;
    limits.memory_limit_mb = 100;
    limits.cpu_time_limit = std::chrono::milliseconds(5000);

    // Should exceed 80% threshold
    bool exceeds_80 =
        ResourceMonitorUtils::exceeds_threshold(usage, limits, 80.0);
    QVERIFY(exceeds_80);

    // Should not exceed 90% threshold
    bool exceeds_90 =
        ResourceMonitorUtils::exceeds_threshold(usage, limits, 90.0);
    QVERIFY(!exceeds_90);
}

void TestResourceMonitor::createTestProcess() {
    cleanupTestProcess();

    m_test_process = new QProcess(this);

    // Create a simple long-running process for testing
#ifdef Q_OS_WIN
    m_test_process->start("ping", QStringList() << "-t" << "127.0.0.1");
#else
    m_test_process->start("sleep", QStringList() << "30");
#endif

    bool started = m_test_process->waitForStarted(5000);
    if (!started) {
        delete m_test_process;
        m_test_process = nullptr;
    }
}

void TestResourceMonitor::cleanupTestProcess() {
    if (m_test_process) {
        if (m_test_process->state() == QProcess::Running) {
            m_test_process->terminate();
            if (!m_test_process->waitForFinished(3000)) {
                m_test_process->kill();
                m_test_process->waitForFinished(1000);
            }
        }
        delete m_test_process;
        m_test_process = nullptr;
    }
}

bool TestResourceMonitor::isCurrentPlatformSupported() {
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    return true;
#else
    return false;
#endif
}

void TestResourceMonitor::testNetworkConnectionTracking() {
    if (!SandboxResourceMonitor::is_supported()) {
        QSKIP("Resource monitoring not supported on this platform");
    }

    SandboxResourceMonitor monitor;
    QVERIFY(monitor.initialize());

    // Test network connection tracking
    ResourceUsage usage = monitor.get_system_usage();

    // Network connections should be tracked (even if 0)
    QVERIFY(usage.network_connections_used >= 0);

    // Test with process usage (if we have a test process)
    if (m_test_process && m_test_process->state() == QProcess::Running) {
        ResourceUsage process_usage = monitor.get_process_usage(m_test_process->processId());
        QVERIFY(process_usage.network_connections_used >= 0);
    }

    monitor.shutdown();
}

void TestResourceMonitor::testConcurrentMonitoring() {
    if (!SandboxResourceMonitor::is_supported()) {
        QSKIP("Resource monitoring not supported on this platform");
    }

    // Test concurrent access to resource monitoring
    const int thread_count = 3;
    const int iterations_per_thread = 5;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([this, iterations_per_thread, &success_count]() {
            SandboxResourceMonitor monitor;
            if (monitor.initialize()) {
                for (int i = 0; i < iterations_per_thread; ++i) {
                    ResourceUsage usage = monitor.get_system_usage();
                    if (usage.memory_used_mb > 0) {
                        success_count.fetch_add(1);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                monitor.shutdown();
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify that most monitoring operations succeeded
    QVERIFY(success_count.load() > (thread_count * iterations_per_thread) / 2);
}

QTEST_MAIN(TestResourceMonitor)
#include "test_resource_monitor.moc"
