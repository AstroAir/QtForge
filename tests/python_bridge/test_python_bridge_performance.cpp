/**
 * @file test_python_bridge_performance.cpp
 * @brief Performance tests for Python bridge functionality
 */

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QThread>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QtTest/QtTest>

#include <qtplugin/bridges/python_plugin_bridge.hpp>

class TestPythonBridgePerformance : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Performance benchmarks
    void benchmarkPluginInitialization();
    void benchmarkMethodInvocation();
    void benchmarkPropertyAccess();
    void benchmarkEventEmission();
    void benchmarkDataTransfer();

    // Scalability tests
    void testManyMethodCalls();
    void testLargeDataHandling();
    void testHighFrequencyEvents();
    void testConcurrentAccess();

    // Memory and resource tests
    void testMemoryUsage();
    void testResourceCleanup();
    void testLongRunningOperations();

    // Stress tests
    void stressTestMethodCalls();
    void stressTestEventSystem();
    void stressTestPropertyAccess();

private:
    void createPerformanceTestPlugin();
    bool isPythonAvailable();

    QTemporaryDir* m_tempDir;
    QString m_perfPluginPath;
    std::unique_ptr<qtplugin::PythonPluginBridge> m_bridge;
    bool m_pythonAvailable;
};

void TestPythonBridgePerformance::initTestCase() {
    m_pythonAvailable = isPythonAvailable();

    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    createPerformanceTestPlugin();
}

void TestPythonBridgePerformance::cleanupTestCase() { delete m_tempDir; }

void TestPythonBridgePerformance::init() {
    if (!m_pythonAvailable) {
        QSKIP("Python not available for testing");
    }

    m_bridge = std::make_unique<qtplugin::PythonPluginBridge>(m_perfPluginPath);
}

void TestPythonBridgePerformance::cleanup() {
    if (m_bridge) {
        m_bridge->shutdown();
        m_bridge.reset();
    }
}

void TestPythonBridgePerformance::createPerformanceTestPlugin() {
    QString pluginContent = R"(
import time
import json
import threading

class PerformanceTestPlugin:
    def __init__(self):
        self.name = "Performance Test Plugin"
        self.version = "1.0.0"
        self.description = "Plugin for performance testing"
        self.counter = 0
        self.data_store = {}
        self.lock = threading.Lock()

    def initialize(self):
        return {"success": True}

    def shutdown(self):
        return {"success": True}

    def simple_method(self):
        return "result"

    def increment_counter(self):
        with self.lock:
            self.counter += 1
            return self.counter

    def get_counter(self):
        with self.lock:
            return self.counter

    def set_counter(self, value):
        with self.lock:
            self.counter = int(value)

    def process_data(self, data):
        # Simulate some processing
        if isinstance(data, str):
            return data.upper()
        elif isinstance(data, (int, float)):
            return data * 2
        elif isinstance(data, list):
            return [self.process_data(item) for item in data]
        else:
            return str(data)

    def store_data(self, key, value):
        with self.lock:
            self.data_store[key] = value
            return {"stored": True, "key": key}

    def get_data(self, key):
        with self.lock:
            return self.data_store.get(key)

    def batch_operation(self, count):
        results = []
        for i in range(count):
            results.append(f"item_{i}")
        return {"count": len(results), "results": results}

    def cpu_intensive_task(self, iterations=1000):
        # Simulate CPU-intensive work
        result = 0
        for i in range(iterations):
            result += i * i
        return {"result": result, "iterations": iterations}

    def memory_intensive_task(self, size=1000):
        # Create large data structure
        large_data = ["x" * 100 for _ in range(size)]
        return {"size": len(large_data), "total_chars": sum(len(s) for s in large_data)}

    def handle_event(self, event_name, event_data):
        return {"handled": True, "event_name": event_name}

    def sleep_task(self, seconds=0.1):
        time.sleep(seconds)
        return {"slept": seconds}

def create_plugin():
    return PerformanceTestPlugin()
)";

    m_perfPluginPath = m_tempDir->path() + "/performance_test_plugin.py";
    QFile file(m_perfPluginPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << pluginContent;
    file.close();
}

void TestPythonBridgePerformance::benchmarkPluginInitialization() {
    QBENCHMARK {
        auto bridge =
            std::make_unique<qtplugin::PythonPluginBridge>(m_perfPluginPath);
        QVERIFY(bridge->initialize().has_value());
        bridge->shutdown();
    }
}

void TestPythonBridgePerformance::benchmarkMethodInvocation() {
    QVERIFY(m_bridge->initialize().has_value());

    QBENCHMARK {
        auto result = m_bridge->invoke_method("simple_method", QVariantList());
        QVERIFY(result.has_value());
    }
}

void TestPythonBridgePerformance::benchmarkPropertyAccess() {
    QVERIFY(m_bridge->initialize().has_value());

    QBENCHMARK {
        auto result = m_bridge->get_property("counter");
        QVERIFY(result.has_value());
    }
}

void TestPythonBridgePerformance::benchmarkEventEmission() {
    QVERIFY(m_bridge->initialize().has_value());

    QJsonObject eventData;
    eventData["test"] = "benchmark";

    QBENCHMARK {
        auto result = m_bridge->emit_event("benchmark_event", eventData);
        QVERIFY(result.has_value());
    }
}

void TestPythonBridgePerformance::benchmarkDataTransfer() {
    QVERIFY(m_bridge->initialize().has_value());

    QString largeString = QString("x").repeated(1000);
    QVariantList params;
    params << largeString;

    QBENCHMARK {
        auto result = m_bridge->invoke_method("process_data", params);
        QVERIFY(result.has_value());
    }
}

void TestPythonBridgePerformance::testManyMethodCalls() {
    QVERIFY(m_bridge->initialize().has_value());

    QElapsedTimer timer;
    timer.start();

    const int numCalls = 1000;
    int successCount = 0;

    for (int i = 0; i < numCalls; ++i) {
        auto result =
            m_bridge->invoke_method("increment_counter", QVariantList());
        if (result.has_value()) {
            successCount++;
        }
    }

    qint64 elapsed = timer.elapsed();

    QCOMPARE(successCount, numCalls);
    qDebug() << "Made" << numCalls << "method calls in" << elapsed << "ms";
    qDebug() << "Average time per call:" << (double)elapsed / numCalls << "ms";

    // Verify final counter value
    auto counter_result = m_bridge->get_property("counter");
    QVERIFY(counter_result.has_value());
    QCOMPARE(counter_result.value().toInt(), numCalls);
}

void TestPythonBridgePerformance::testLargeDataHandling() {
    QVERIFY(m_bridge->initialize().has_value());

    // Test with increasingly large data
    QList<int> dataSizes = {100, 1000, 10000, 100000};

    for (int size : dataSizes) {
        QElapsedTimer timer;
        timer.start();

        QVariantList params;
        params << size;

        auto result = m_bridge->invoke_method("memory_intensive_task", params);

        qint64 elapsed = timer.elapsed();

        QVERIFY(result.has_value());
        qDebug() << "Processed" << size << "items in" << elapsed << "ms";
    }
}

void TestPythonBridgePerformance::testHighFrequencyEvents() {
    QVERIFY(m_bridge->initialize().has_value());

    const int numEvents = 1000;
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < numEvents; ++i) {
        QJsonObject eventData;
        eventData["index"] = i;
        eventData["timestamp"] = QDateTime::currentMSecsSinceEpoch();

        auto result = m_bridge->emit_event("high_freq_event", eventData);
        QVERIFY(result.has_value());
    }

    qint64 elapsed = timer.elapsed();

    qDebug() << "Emitted" << numEvents << "events in" << elapsed << "ms";
    qDebug() << "Average time per event:" << (double)elapsed / numEvents
             << "ms";
}

void TestPythonBridgePerformance::testConcurrentAccess() {
    QVERIFY(m_bridge->initialize().has_value());

    const int numThreads = 4;
    const int callsPerThread = 100;

    QElapsedTimer timer;
    timer.start();

    QList<QFuture<int>> futures;

    for (int t = 0; t < numThreads; ++t) {
        auto future = QtConcurrent::run([this, callsPerThread]() -> int {
            int successCount = 0;
            for (int i = 0; i < callsPerThread; ++i) {
                auto result = m_bridge->invoke_method("increment_counter",
                                                      QVariantList());
                if (result.has_value()) {
                    successCount++;
                }
            }
            return successCount;
        });
        futures.append(future);
    }

    int totalSuccess = 0;
    for (auto& future : futures) {
        future.waitForFinished();
        totalSuccess += future.result();
    }

    qint64 elapsed = timer.elapsed();

    QCOMPARE(totalSuccess, numThreads * callsPerThread);
    qDebug() << "Concurrent test:" << totalSuccess << "calls in" << elapsed
             << "ms";
    qDebug() << "Calls per second:" << (totalSuccess * 1000.0) / elapsed;
}

void TestPythonBridgePerformance::testMemoryUsage() {
    QVERIFY(m_bridge->initialize().has_value());

    // Test memory usage with repeated operations
    const int iterations = 100;

    for (int i = 0; i < iterations; ++i) {
        // Store some data
        QVariantList store_params;
        store_params << QString("key_%1").arg(i) << QString("value_%1").arg(i);

        auto store_result = m_bridge->invoke_method("store_data", store_params);
        QVERIFY(store_result.has_value());

        // Retrieve the data
        QVariantList get_params;
        get_params << QString("key_%1").arg(i);

        auto get_result = m_bridge->invoke_method("get_data", get_params);
        QVERIFY(get_result.has_value());
    }

    qDebug() << "Completed" << iterations << "store/retrieve cycles";
}

void TestPythonBridgePerformance::testResourceCleanup() {
    // Test multiple initialization/cleanup cycles
    const int cycles = 10;

    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < cycles; ++i) {
        auto bridge =
            std::make_unique<qtplugin::PythonPluginBridge>(m_perfPluginPath);
        QVERIFY(bridge->initialize().has_value());

        // Perform some operations
        auto result = m_bridge->invoke_method("simple_method", QVariantList());
        QVERIFY(result.has_value());

        bridge->shutdown();
    }

    qint64 elapsed = timer.elapsed();

    qDebug() << "Completed" << cycles << "init/cleanup cycles in" << elapsed
             << "ms";
    qDebug() << "Average cycle time:" << (double)elapsed / cycles << "ms";
}

void TestPythonBridgePerformance::testLongRunningOperations() {
    QVERIFY(m_bridge->initialize().has_value());

    QElapsedTimer timer;
    timer.start();

    QVariantList params;
    params << 0.1;  // 100ms sleep

    auto result = m_bridge->invoke_method("sleep_task", params);

    qint64 elapsed = timer.elapsed();

    QVERIFY(result.has_value());
    QVERIFY(elapsed >= 100);  // Should take at least 100ms

    qDebug() << "Long running operation took" << elapsed << "ms";
}

void TestPythonBridgePerformance::stressTestMethodCalls() {
    QVERIFY(m_bridge->initialize().has_value());

    const int stressIterations = 5000;
    QElapsedTimer timer;
    timer.start();

    int successCount = 0;
    int errorCount = 0;

    for (int i = 0; i < stressIterations; ++i) {
        auto result = m_bridge->invoke_method("simple_method", QVariantList());
        if (result.has_value()) {
            successCount++;
        } else {
            errorCount++;
        }
    }

    qint64 elapsed = timer.elapsed();

    qDebug() << "Stress test results:";
    qDebug() << "  Successful calls:" << successCount;
    qDebug() << "  Failed calls:" << errorCount;
    qDebug() << "  Total time:" << elapsed << "ms";
    qDebug() << "  Calls per second:" << (successCount * 1000.0) / elapsed;

    // Allow some failures under stress, but most should succeed
    QVERIFY(successCount > stressIterations * 0.95);
}

void TestPythonBridgePerformance::stressTestEventSystem() {
    QVERIFY(m_bridge->initialize().has_value());

    const int stressEvents = 2000;
    QElapsedTimer timer;
    timer.start();

    int successCount = 0;

    for (int i = 0; i < stressEvents; ++i) {
        QJsonObject eventData;
        eventData["stress_index"] = i;

        auto result = m_bridge->emit_event("stress_event", eventData);
        if (result.has_value()) {
            successCount++;
        }
    }

    qint64 elapsed = timer.elapsed();

    qDebug() << "Event stress test results:";
    qDebug() << "  Successful events:" << successCount;
    qDebug() << "  Total time:" << elapsed << "ms";
    qDebug() << "  Events per second:" << (successCount * 1000.0) / elapsed;

    QVERIFY(successCount > stressEvents * 0.95);
}

void TestPythonBridgePerformance::stressTestPropertyAccess() {
    QVERIFY(m_bridge->initialize().has_value());

    const int stressAccesses = 3000;
    QElapsedTimer timer;
    timer.start();

    int getSuccessCount = 0;
    int setSuccessCount = 0;

    for (int i = 0; i < stressAccesses; ++i) {
        // Alternate between get and set operations
        if (i % 2 == 0) {
            auto result = m_bridge->get_property("counter");
            if (result.has_value()) {
                getSuccessCount++;
            }
        } else {
            auto result = m_bridge->set_property("counter", QVariant(i));
            if (result.has_value()) {
                setSuccessCount++;
            }
        }
    }

    qint64 elapsed = timer.elapsed();

    qDebug() << "Property stress test results:";
    qDebug() << "  Successful gets:" << getSuccessCount;
    qDebug() << "  Successful sets:" << setSuccessCount;
    qDebug() << "  Total time:" << elapsed << "ms";

    QVERIFY(getSuccessCount > (stressAccesses / 2) * 0.95);
    QVERIFY(setSuccessCount > (stressAccesses / 2) * 0.95);
}

bool TestPythonBridgePerformance::isPythonAvailable() {
    QProcess process;
    QStringList python_names = {"python3",   "python",     "python3.8",
                                "python3.9", "python3.10", "python3.11",
                                "python3.12"};

    for (const QString& name : python_names) {
        process.start(name, QStringList() << "--version");

        if (!process.waitForFinished(3000)) {
            continue;
        }

        if (process.exitCode() != 0) {
            continue;
        }

        QString output = process.readAllStandardOutput();
        if (output.contains("Python", Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

QTEST_MAIN(TestPythonBridgePerformance)
#include "test_python_bridge_performance.moc"
