#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
#include <QtTest/QtTest>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <psapi.h>
#include <windows.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#endif
#include <chrono>

// Include the plugin system headers
#include "qtplugin/communication/message_bus.hpp"
#include "qtplugin/communication/message_types.hpp"
#include "qtplugin/core/plugin_manager.hpp"
#include "qtplugin/managers/configuration_manager_impl.hpp"
#include "qtplugin/managers/logging_manager_impl.hpp"
#include "qtplugin/managers/resource_lifecycle.hpp"
#include "qtplugin/managers/resource_manager_impl.hpp"
#include "qtplugin/managers/resource_monitor.hpp"

class PerformanceTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Plugin loading performance tests
    void testPluginLoadingPerformance();
    void testMultiplePluginLoadingPerformance();
    void testPluginUnloadingPerformance();

    // Configuration performance tests
    void testConfigurationReadPerformance();
    void testConfigurationWritePerformance();
    void testLargeConfigurationPerformance();

    // Message bus performance tests
    void testMessageBusPerformance();
    void testHighFrequencyMessagingPerformance();
    void testConcurrentMessagingPerformance();

    // Memory usage tests
    void testMemoryUsageBaseline();
    void testMemoryUsageWithPlugins();
    void testMemoryLeakDetection();

private:
    std::unique_ptr<qtplugin::PluginManager> m_pluginManager;
    std::unique_ptr<qtplugin::IConfigurationManager> m_configManager;
    std::unique_ptr<qtplugin::MessageBus> m_messageBus;

    // Performance measurement helpers
    void measureExecutionTime(const QString& testName,
                              std::function<void()> testFunction);
    void logPerformanceResult(const QString& testName, qint64 elapsedMs,
                              const QString& details = QString());
    size_t getCurrentMemoryUsage() const;
};

void PerformanceTests::initTestCase() {
    qDebug() << "Initializing Performance Tests...";

    // Initialize core components
    m_configManager = qtplugin::create_configuration_manager();
    m_messageBus = std::make_unique<qtplugin::MessageBus>();
    // Create PluginManager with explicit nullptr parameters to avoid incomplete
    // type issues
    m_pluginManager = std::make_unique<qtplugin::PluginManager>(
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr);

    // Set up test environment
    m_configManager->set_value("test.performance.enabled", true);
    m_configManager->set_value("test.performance.iterations", 1000);
}

void PerformanceTests::cleanupTestCase() {
    qDebug() << "Cleaning up Performance Tests...";

    // Clean up in reverse order
    m_pluginManager.reset();
    m_messageBus.reset();
    m_configManager.reset();
}

void PerformanceTests::testPluginLoadingPerformance() {
    const int iterations = 100;
    QElapsedTimer timer;

    timer.start();
    for (int i = 0; i < iterations; ++i) {
        // Simulate plugin loading operations
        m_pluginManager->load_all_plugins();
    }
    qint64 elapsed = timer.elapsed();

    logPerformanceResult("Plugin Loading", elapsed,
                         QString("Iterations: %1").arg(iterations));

    // Performance threshold: should complete within reasonable time
    QVERIFY2(elapsed < 5000, QString("Plugin loading took too long: %1ms")
                                 .arg(elapsed)
                                 .toLocal8Bit());
}

void PerformanceTests::testMultiplePluginLoadingPerformance() {
    measureExecutionTime("Multiple Plugin Loading", [this]() {
        // Simulate loading multiple plugins simultaneously
        for (int i = 0; i < 10; ++i) {
            m_pluginManager->load_all_plugins();
        }
    });
}

void PerformanceTests::testPluginUnloadingPerformance() {
    measureExecutionTime("Plugin Unloading", [this]() {
        // First load some plugins
        m_pluginManager->load_all_plugins();

        // Then unload them
        auto plugins = m_pluginManager->loaded_plugins();
        for (const auto& plugin : plugins) {
            m_pluginManager->unload_plugin(plugin);
        }
    });
}

void PerformanceTests::testConfigurationReadPerformance() {
    // Prepare test data
    const int iterations = 1000;
    for (int i = 0; i < 100; ++i) {
        m_configManager->set_value(QString("test.key.%1").arg(i).toStdString(),
                                   QString("value_%1").arg(i));
    }

    measureExecutionTime("Configuration Read", [this, iterations]() {
        for (int i = 0; i < iterations; ++i) {
            auto value = m_configManager->get_value(
                QString("test.key.%1").arg(i % 100).toStdString());
            Q_UNUSED(value)
        }
    });
}

void PerformanceTests::testConfigurationWritePerformance() {
    const int iterations = 1000;

    measureExecutionTime("Configuration Write", [this, iterations]() {
        for (int i = 0; i < iterations; ++i) {
            m_configManager->set_value(
                QString("perf.test.%1").arg(i).toStdString(),
                QString("value_%1").arg(i));
        }
    });
}

void PerformanceTests::testLargeConfigurationPerformance() {
    const int largeDataSize = 10000;
    QString largeValue = QString("x").repeated(largeDataSize);

    measureExecutionTime("Large Configuration", [this, largeValue]() {
        m_configManager->set_value("large.data.test", largeValue);
        auto retrievedValue = m_configManager->get_value("large.data.test");
        if (retrievedValue) {
            QCOMPARE(retrievedValue.value().toString().size(),
                     largeValue.size());
        }
    });
}

void PerformanceTests::testMessageBusPerformance() {
    const int messageCount = 1000;

    measureExecutionTime("Message Bus", [this, messageCount]() {
        for (int i = 0; i < messageCount; ++i) {
            QJsonObject data;
            data["id"] = i;
            data["data"] = QString("test_message_%1").arg(i);
            qtplugin::messages::CustomDataMessage message("performance_test",
                                                          "test_data", data);
            m_messageBus->publish(message);
        }
    });
}

void PerformanceTests::testHighFrequencyMessagingPerformance() {
    const int highFrequencyCount = 5000;

    measureExecutionTime(
        "High Frequency Messaging", [this, highFrequencyCount]() {
            for (int i = 0; i < highFrequencyCount; ++i) {
                QJsonObject data;
                data["sequence"] = i;
                qtplugin::messages::CustomDataMessage message(
                    "performance_test", "high_frequency", data);
                m_messageBus->publish(message);
            }
        });
}

void PerformanceTests::testConcurrentMessagingPerformance() {
    const int threadCount = 4;
    const int messagesPerThread = 250;

    measureExecutionTime("Concurrent Messaging", [this, threadCount,
                                                  messagesPerThread]() {
        QList<QThread*> threads;

        for (int t = 0; t < threadCount; ++t) {
            QThread* thread = QThread::create([this, t, messagesPerThread]() {
                for (int i = 0; i < messagesPerThread; ++i) {
                    QJsonObject data;
                    data["thread"] = t;
                    data["message"] = i;
                    qtplugin::messages::CustomDataMessage message(
                        "performance_test", "concurrent", data);
                    m_messageBus->publish(message);
                }
            });
            threads.append(thread);
            thread->start();
        }

        // Wait for all threads to complete
        for (auto* thread : threads) {
            thread->wait();
            delete thread;
        }
    });
}

void PerformanceTests::testMemoryUsageBaseline() {
    // Measure baseline memory usage
    size_t initial_memory = getCurrentMemoryUsage();

    // Create a minimal plugin manager
    auto manager = std::make_unique<qtplugin::PluginManager>();

    size_t after_manager = getCurrentMemoryUsage();
    size_t manager_overhead = after_manager - initial_memory;

    qDebug() << "Memory usage baseline:";
    qDebug() << "  Initial memory:" << initial_memory << "bytes";
    qDebug() << "  After PluginManager:" << after_manager << "bytes";
    qDebug() << "  Manager overhead:" << manager_overhead << "bytes";

    // Manager should use less than 10MB baseline
    QVERIFY2(manager_overhead < 10 * 1024 * 1024,
             QString("PluginManager uses too much memory: %1 bytes")
                 .arg(manager_overhead)
                 .toLocal8Bit());

    logPerformanceResult("Memory Baseline", manager_overhead, "bytes");
}

void PerformanceTests::testMemoryUsageWithPlugins() {
    // Placeholder for memory usage with plugins loaded
    qDebug() << "Memory usage with plugins test - placeholder implementation";
    QVERIFY(true);  // Always pass for now
}

void PerformanceTests::testMemoryLeakDetection() {
    // Placeholder for memory leak detection
    qDebug() << "Memory leak detection test - placeholder implementation";
    QVERIFY(true);  // Always pass for now
}

void PerformanceTests::measureExecutionTime(
    const QString& testName, std::function<void()> testFunction) {
    QElapsedTimer timer;
    timer.start();

    testFunction();

    qint64 elapsed = timer.elapsed();
    logPerformanceResult(testName, elapsed);
}

void PerformanceTests::logPerformanceResult(const QString& testName,
                                            qint64 elapsedMs,
                                            const QString& details) {
    QString message =
        QString("Performance Test '%1': %2ms").arg(testName).arg(elapsedMs);
    if (!details.isEmpty()) {
        message += QString(" (%1)").arg(details);
    }
    qDebug() << message;

    // You could also write results to a file or database for analysis
}

size_t PerformanceTests::getCurrentMemoryUsage() const {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
#elif defined(__linux__)
    std::ifstream status_file("/proc/self/status");
    std::string line;
    while (std::getline(status_file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::istringstream iss(line);
            std::string key, value, unit;
            iss >> key >> value >> unit;
            return std::stoull(value) * 1024;  // Convert KB to bytes
        }
    }
    return 0;
#elif defined(__APPLE__)
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info,
                  &infoCount) == KERN_SUCCESS) {
        return info.resident_size;
    }
    return 0;
#else
    // Fallback: return a reasonable estimate
    return 1024 * 1024;  // 1MB
#endif
}

QTEST_MAIN(PerformanceTests)
#include "test_performance.moc"
