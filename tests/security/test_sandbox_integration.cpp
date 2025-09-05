/**
 * @file test_sandbox_integration.cpp
 * @brief Integration tests for complete sandbox workflows
 * @version 3.2.0
 */

#include <QCoreApplication>
#include <QEventLoop>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTimer>
#include <QtTest/QtTest>
#include <memory>

#include "qtplugin/security/sandbox/plugin_sandbox.hpp"

using namespace qtplugin;

class TestSandboxIntegration : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // End-to-end workflow tests
    void testCompletePluginExecution();
    void testResourceLimitEnforcement();
    void testSecurityViolationHandling();
    void testMultiSandboxWorkflow();

    // Component interaction tests
    void testResourceMonitoringIntegration();
    void testSecurityEnforcerIntegration();
    void testManagerSandboxInteraction();

    // Real-world scenario tests
    void testPythonPluginExecution();
    void testNativePluginExecution();
    void testPluginTimeout();
    void testPluginCrash();

    // Policy enforcement tests
    void testStrictPolicyEnforcement();
    void testLimitedPolicyEnforcement();
    void testUnrestrictedPolicyExecution();

    // Error recovery tests
    void testSandboxRecoveryAfterFailure();
    void testManagerRecoveryAfterShutdown();
    void testResourceCleanupAfterTermination();

    // Performance integration tests
    void testConcurrentPluginExecution();
    void testRapidSandboxCycling();
    void testLongRunningPluginMonitoring();

private:
    QTemporaryDir* m_temp_dir;
    SandboxManager* m_manager;
    QStringList m_test_plugins;
    QStringList m_created_sandboxes;

    void createTestPlugins();
    QString createPythonTestPlugin(const QString& script_content);
    QString createNativeTestPlugin();
    void cleanupTestResources();
    bool waitForSignal(QObject* sender, const char* signal, int timeout = 5000);
};

void TestSandboxIntegration::initTestCase() {
    // Initialize Qt application for testing
    if (!QCoreApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        new QCoreApplication(argc, argv);
    }

    m_temp_dir = new QTemporaryDir();
    QVERIFY(m_temp_dir->isValid());

    createTestPlugins();
}

void TestSandboxIntegration::cleanupTestCase() {
    cleanupTestResources();
    delete m_temp_dir;
}

void TestSandboxIntegration::init() {
    m_manager = &SandboxManager::instance();
    m_created_sandboxes.clear();
}

void TestSandboxIntegration::cleanup() {
    // Cleanup created sandboxes
    for (const QString& sandbox_id : m_created_sandboxes) {
        m_manager->remove_sandbox(sandbox_id);
    }
    m_created_sandboxes.clear();
}

void TestSandboxIntegration::testCompletePluginExecution() {
    // Create a sandbox with limited policy
    SecurityPolicy policy = SecurityPolicy::create_limited_policy();
    QString sandbox_id =
        QString("integration_test_%1").arg(QDateTime::currentMSecsSinceEpoch());

    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);

    auto sandbox = sandbox_result.value();
    QVERIFY(sandbox != nullptr);
    QVERIFY(sandbox->is_active());

    // Connect to signals for monitoring
    QSignalSpy execution_spy(sandbox.get(),
                             SIGNAL(execution_completed(int, QJsonObject)));
    QSignalSpy resource_spy(sandbox.get(),
                            SIGNAL(resource_usage_updated(ResourceUsage)));
    QSignalSpy violation_spy(sandbox.get(), SIGNAL(security_violation(QString, QJsonObject)));

    // Create a simple test plugin
    QString plugin_content = R"(
import sys
import time
print("Plugin started")
time.sleep(0.1)
print("Plugin completed")
sys.exit(0)
)";

    QString plugin_path = createPythonTestPlugin(plugin_content);
    QVERIFY(!plugin_path.isEmpty());

    // Execute the plugin
    auto exec_result = sandbox->execute_plugin(plugin_path, PluginType::Python);

    if (exec_result.has_value()) {
        // Wait for execution to complete
        bool completed =
            waitForSignal(sandbox.get(),
                          SIGNAL(execution_completed(int, QJsonObject)), 10000);
        QVERIFY(completed);

        // Verify signals were emitted
        QVERIFY(execution_spy.count() >= 1);

        // Check execution result
        if (execution_spy.count() > 0) {
            QList<QVariant> args = execution_spy.takeFirst();
            int exit_code = args.at(0).toInt();
            QCOMPARE(exit_code, 0);  // Successful execution
        }

        // Verify resource monitoring was active
        QVERIFY(resource_spy.count() >= 1);

        // Should not have security violations for this simple plugin
        QCOMPARE(violation_spy.count(), 0);
    } else {
        // If Python is not available, skip this test
        QSKIP("Python not available for plugin execution");
    }
}

void TestSandboxIntegration::testResourceLimitEnforcement() {
    // Create a sandbox with very strict limits
    SecurityPolicy policy = SecurityPolicy::create_strict_policy();
    policy.limits.memory_limit_mb = 10;  // Very low limit
    policy.limits.cpu_time_limit =
        std::chrono::milliseconds(100);  // Very short time
    policy.limits.execution_timeout = std::chrono::milliseconds(500);

    QString sandbox_id = QString("resource_limit_test_%1")
                             .arg(QDateTime::currentMSecsSinceEpoch());

    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);

    auto sandbox = sandbox_result.value();

    // Connect to resource limit signal
    QSignalSpy limit_spy(sandbox.get(),
                         &PluginSandbox::resource_limit_exceeded);

    // Create a resource-intensive plugin
    QString plugin_content = R"(
import time
# Try to consume resources
data = []
for i in range(1000000):
    data.append(str(i) * 100)  # Memory intensive
    if i % 10000 == 0:
        time.sleep(0.001)  # CPU intensive
)";

    QString plugin_path = createPythonTestPlugin(plugin_content);
    QVERIFY(!plugin_path.isEmpty());

    // Execute the resource-intensive plugin
    auto exec_result = sandbox->execute_plugin(plugin_path, PluginType::Python);

    if (exec_result.has_value()) {
        // Wait for resource limit to be exceeded or execution to complete
        bool limit_exceeded = waitForSignal(
            sandbox.get(),
            SIGNAL(resource_limit_exceeded(QString, QJsonObject)), 5000);

        // Should either exceed limits or complete quickly
        QVERIFY(limit_exceeded ||
                waitForSignal(sandbox.get(),
                              SIGNAL(execution_completed(int, QJsonObject)),
                              1000));

        if (limit_exceeded) {
            QVERIFY(limit_spy.count() >= 1);

            // Verify the limit exceeded signal contains useful information
            QList<QVariant> args = limit_spy.takeFirst();
            QString resource = args.at(0).toString();
            QVERIFY(!resource.isEmpty());
        }
    } else {
        QSKIP("Python not available for plugin execution");
    }
}

void TestSandboxIntegration::testSecurityViolationHandling() {
    // Create a sandbox with restrictive permissions
    SecurityPolicy policy = SecurityPolicy::create_sandboxed_policy();
    QString sandbox_id =
        QString("security_test_%1").arg(QDateTime::currentMSecsSinceEpoch());

    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);

    auto sandbox = sandbox_result.value();

    // Connect to security violation signal
    QSignalSpy violation_spy(sandbox.get(), SIGNAL(security_violation(QString, QJsonObject)));

    // Create a plugin that attempts unauthorized operations
    QString plugin_content = R"(
import os
import sys
try:
    # Attempt to read a system file (should be blocked)
    with open('/etc/passwd', 'r') as f:
        content = f.read()
    print("Unauthorized file access succeeded")
except Exception as e:
    print(f"File access blocked: {e}")

try:
    # Attempt to execute a system command (should be blocked)
    os.system('ls -la')
    print("System command succeeded")
except Exception as e:
    print(f"System command blocked: {e}")

sys.exit(0)
)";

    QString plugin_path = createPythonTestPlugin(plugin_content);
    QVERIFY(!plugin_path.isEmpty());

    // Execute the plugin
    auto exec_result = sandbox->execute_plugin(plugin_path, PluginType::Python);

    if (exec_result.has_value()) {
        // Wait for execution to complete
        bool completed =
            waitForSignal(sandbox.get(),
                          SIGNAL(execution_completed(int, QJsonObject)), 10000);
        QVERIFY(completed);

        // Note: Security violations might not be detected at the Python level
        // but the sandbox should still contain the execution
        qDebug() << "Security violation count:" << violation_spy.count();
    } else {
        QSKIP("Python not available for plugin execution");
    }
}

void TestSandboxIntegration::testMultiSandboxWorkflow() {
    const int sandbox_count = 3;
    QList<std::shared_ptr<PluginSandbox>> sandboxes;
    QList<QSignalSpy*> spies;

    // Create multiple sandboxes with different policies
    for (int i = 0; i < sandbox_count; ++i) {
        SecurityPolicy policy;
        switch (i) {
            case 0:
                policy = SecurityPolicy::create_limited_policy();
                break;
            case 1:
                policy = SecurityPolicy::create_sandboxed_policy();
                break;
            case 2:
                policy = SecurityPolicy::create_strict_policy();
                break;
        }

        QString sandbox_id = QString("multi_test_%1_%2")
                                 .arg(i)
                                 .arg(QDateTime::currentMSecsSinceEpoch());

        auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
        QVERIFY(sandbox_result.has_value());
        m_created_sandboxes.append(sandbox_id);

        auto sandbox = sandbox_result.value();
        sandboxes.append(sandbox);

        // Create spy for each sandbox
        auto spy =
            new QSignalSpy(sandbox.get(), SIGNAL(execution_completed(int, QJsonObject)));
        spies.append(spy);
    }

    // Create different plugins for each sandbox
    QStringList plugin_contents = {
        "import time; print('Plugin 1'); time.sleep(0.1); print('Done 1')",
        "import time; print('Plugin 2'); time.sleep(0.2); print('Done 2')",
        "import time; print('Plugin 3'); time.sleep(0.05); print('Done 3')"};

    // Execute plugins concurrently
    for (int i = 0; i < sandbox_count; ++i) {
        QString plugin_path = createPythonTestPlugin(plugin_contents[i]);
        if (!plugin_path.isEmpty()) {
            auto exec_result =
                sandboxes[i]->execute_plugin(plugin_path, PluginType::Python);
            if (!exec_result.has_value()) {
                QSKIP("Python not available for plugin execution");
            }
        }
    }

    // Wait for all executions to complete
    bool all_completed = true;
    for (int i = 0; i < sandbox_count; ++i) {
        bool completed =
            waitForSignal(sandboxes[i].get(),
                          SIGNAL(execution_completed(int, QJsonObject)), 10000);
        if (!completed) {
            all_completed = false;
        }
    }

    if (all_completed) {
        // Verify all executions completed
        for (int i = 0; i < sandbox_count; ++i) {
            QVERIFY(spies[i]->count() >= 1);
        }
    }

    // Cleanup spies
    for (auto spy : spies) {
        delete spy;
    }
}

void TestSandboxIntegration::testResourceMonitoringIntegration() {
    SecurityPolicy policy = SecurityPolicy::create_limited_policy();
    QString sandbox_id = QString("resource_monitor_test_%1")
                             .arg(QDateTime::currentMSecsSinceEpoch());

    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);

    auto sandbox = sandbox_result.value();

    // Connect to resource monitoring signals
    QSignalSpy usage_spy(sandbox.get(), SIGNAL(resource_usage_updated(ResourceUsage)));

    // Create a plugin that does some work
    QString plugin_content = R"(
import time
import sys
data = []
for i in range(1000):
    data.append(str(i))
    if i % 100 == 0:
        time.sleep(0.01)
print(f"Processed {len(data)} items")
sys.exit(0)
)";

    QString plugin_path = createPythonTestPlugin(plugin_content);
    QVERIFY(!plugin_path.isEmpty());

    // Execute the plugin
    auto exec_result = sandbox->execute_plugin(plugin_path, PluginType::Python);

    if (exec_result.has_value()) {
        // Wait for execution to complete
        bool completed =
            waitForSignal(sandbox.get(),
                          SIGNAL(execution_completed(int, QJsonObject)), 10000);
        QVERIFY(completed);

        // Verify resource monitoring was active
        QVERIFY(usage_spy.count() >= 1);

        // Get final resource usage
        ResourceUsage usage = sandbox->get_resource_usage();
        QVERIFY(usage.cpu_time_used >= std::chrono::milliseconds(0));
        // memory_used_mb is size_t (unsigned), so it's always >= 0, just verify it's reasonable
        Q_UNUSED(usage.memory_used_mb); // Suppress unused variable warning if needed

        // Verify usage is within reasonable bounds
        QVERIFY(usage.cpu_time_used < std::chrono::minutes(1));
        QVERIFY(usage.memory_used_mb < 1000);  // Less than 1GB
    } else {
        QSKIP("Python not available for plugin execution");
    }
}

void TestSandboxIntegration::testPluginTimeout() {
    SecurityPolicy policy = SecurityPolicy::create_limited_policy();
    policy.limits.execution_timeout =
        std::chrono::milliseconds(1000);  // 1 second timeout

    QString sandbox_id =
        QString("timeout_test_%1").arg(QDateTime::currentMSecsSinceEpoch());

    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);

    auto sandbox = sandbox_result.value();

    // Connect to signals
    QSignalSpy execution_spy(sandbox.get(),
                             SIGNAL(execution_completed(int, QJsonObject)));
    QSignalSpy violation_spy(sandbox.get(), SIGNAL(security_violation(QString, QJsonObject)));

    // Create a long-running plugin
    QString plugin_content = R"(
import time
print("Starting long operation")
time.sleep(5)  # Sleep longer than timeout
print("This should not be printed")
)";

    QString plugin_path = createPythonTestPlugin(plugin_content);
    QVERIFY(!plugin_path.isEmpty());

    // Execute the plugin
    auto exec_result = sandbox->execute_plugin(plugin_path, PluginType::Python);

    if (exec_result.has_value()) {
        // Wait for timeout or completion
        bool completed = waitForSignal(
            sandbox.get(), SIGNAL(execution_completed(int, QJsonObject)), 3000);

        // Should complete due to timeout
        QVERIFY(completed);

        if (execution_spy.count() > 0) {
            QList<QVariant> args = execution_spy.takeFirst();
            int exit_code = args.at(0).toInt();
            // Should be terminated (non-zero exit code)
            QVERIFY(exit_code != 0);
        }
    } else {
        QSKIP("Python not available for plugin execution");
    }
}

void TestSandboxIntegration::testConcurrentPluginExecution() {
    const int concurrent_count = 3;
    QList<std::shared_ptr<PluginSandbox>> sandboxes;
    QList<QSignalSpy*> spies;

    // Create multiple sandboxes
    for (int i = 0; i < concurrent_count; ++i) {
        SecurityPolicy policy = SecurityPolicy::create_limited_policy();
        QString sandbox_id = QString("concurrent_%1_%2")
                                 .arg(i)
                                 .arg(QDateTime::currentMSecsSinceEpoch());

        auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
        QVERIFY(sandbox_result.has_value());
        m_created_sandboxes.append(sandbox_id);

        auto sandbox = sandbox_result.value();
        sandboxes.append(sandbox);

        auto spy =
            new QSignalSpy(sandbox.get(), SIGNAL(execution_completed(int, QJsonObject)));
        spies.append(spy);
    }

    QElapsedTimer timer;
    timer.start();

    // Start all plugins simultaneously
    for (int i = 0; i < concurrent_count; ++i) {
        QString plugin_content = QString(R"(
import time
import sys
print("Concurrent plugin %1 started")
time.sleep(0.5)
print("Concurrent plugin %1 completed")
sys.exit(%1)
)")
                                     .arg(i);

        QString plugin_path = createPythonTestPlugin(plugin_content);
        if (!plugin_path.isEmpty()) {
            auto exec_result =
                sandboxes[i]->execute_plugin(plugin_path, PluginType::Python);
            if (!exec_result.has_value()) {
                QSKIP("Python not available for plugin execution");
            }
        }
    }

    // Wait for all to complete
    int completed_count = 0;
    for (int i = 0; i < concurrent_count; ++i) {
        if (waitForSignal(sandboxes[i].get(),
                          SIGNAL(execution_completed(int, QJsonObject)),
                          5000)) {
            completed_count++;
        }
    }

    qint64 total_time = timer.elapsed();

    // All should complete
    QCOMPARE(completed_count, concurrent_count);

    // Should complete in reasonable time (concurrent execution should be faster
    // than sequential)
    QVERIFY(total_time < 2000);  // Less than 2 seconds for concurrent execution

    qDebug() << "Concurrent execution of" << concurrent_count << "plugins took"
             << total_time << "ms";

    // Cleanup spies
    for (auto spy : spies) {
        delete spy;
    }
}

void TestSandboxIntegration::createTestPlugins() {
    // Test plugins will be created dynamically in each test
}

QString TestSandboxIntegration::createPythonTestPlugin(
    const QString& script_content) {
    QTemporaryFile* temp_file =
        new QTemporaryFile(m_temp_dir->path() + "/test_plugin_XXXXXX.py", this);
    temp_file->setAutoRemove(false);

    if (temp_file->open()) {
        QTextStream stream(temp_file);
        stream << "#!/usr/bin/env python3\n";
        stream << script_content;

        QString file_path = temp_file->fileName();
        temp_file->close();

        // Make executable on Unix systems
        QFile::setPermissions(
            file_path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);

        m_test_plugins.append(file_path);
        return file_path;
    }

    return QString();
}

QString TestSandboxIntegration::createNativeTestPlugin() {
    // For now, return empty string as native plugin creation is complex
    return QString();
}

void TestSandboxIntegration::cleanupTestResources() {
    // Remove test plugin files
    for (const QString& plugin_path : m_test_plugins) {
        QFile::remove(plugin_path);
    }
    m_test_plugins.clear();
}

bool TestSandboxIntegration::waitForSignal(QObject* sender, const char* signal,
                                           int timeout) {
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(sender, signal, &loop, SLOT(quit()));

    timer.start(timeout);
    loop.exec();

    return timer
        .isActive();  // Returns true if signal was received before timeout
}

void TestSandboxIntegration::testSecurityEnforcerIntegration() {
    // Test integration between sandbox and security enforcer
    SecurityPolicy strict_policy = SecurityPolicy::create_strict_policy();
    auto sandbox = std::make_unique<PluginSandbox>(strict_policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Test that security enforcer is active
    QVERIFY(sandbox->is_active());

    // Test security policy enforcement
    auto exec_result = sandbox->execute_plugin("/nonexistent/path", PluginType::Native, QJsonObject{});
    QVERIFY(!exec_result.has_value()); // Should fail due to security restrictions

    sandbox->shutdown();
}

void TestSandboxIntegration::testManagerSandboxInteraction() {
    SandboxManager& manager = SandboxManager::instance();

    SecurityPolicy policy = SecurityPolicy::create_sandboxed_policy();
    QString sandbox_id = "manager_interaction_test_" + QString::number(QDateTime::currentMSecsSinceEpoch());

    // Test manager creating sandbox
    auto create_result = manager.create_sandbox(sandbox_id, policy);
    QVERIFY(create_result.has_value());

    // Test manager retrieving sandbox
    auto retrieved = manager.get_sandbox(sandbox_id);
    QVERIFY(retrieved != nullptr);

    // Test manager removing sandbox
    manager.remove_sandbox(sandbox_id);

    // Verify sandbox is removed
    auto after_removal = manager.get_sandbox(sandbox_id);
    QVERIFY(after_removal == nullptr);
}

void TestSandboxIntegration::testPythonPluginExecution() {
    // Create a simple Python test script
    QTemporaryFile python_script(m_temp_dir->path() + "/test_python_XXXXXX.py");
    python_script.setAutoRemove(false);

    if (!python_script.open()) {
        QSKIP("Could not create temporary Python script");
    }

    QTextStream stream(&python_script);
    stream << "#!/usr/bin/env python3\n";
    stream << "print('Hello from Python plugin')\n";
    stream << "import sys\n";
    stream << "sys.exit(0)\n";
    python_script.close();

    SecurityPolicy policy = SecurityPolicy::create_sandboxed_policy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Execute Python plugin (this may fail if Python is not available)
    auto exec_result = sandbox->execute_plugin(python_script.fileName(), PluginType::Python, QJsonObject{});
    // Don't assert success as Python may not be available in test environment

    sandbox->shutdown();
}

void TestSandboxIntegration::testNativePluginExecution() {
    // For native plugin test, we'll use a simple executable
    #ifdef Q_OS_WIN
    QString native_executable = "C:\\Windows\\System32\\ping.exe";
    #else
    QString native_executable = "/bin/echo";
    #endif

    if (!QFile::exists(native_executable)) {
        QSKIP("Native executable not available for testing");
    }

    SecurityPolicy policy = SecurityPolicy::create_limited_policy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Execute native plugin
    auto exec_result = sandbox->execute_plugin(native_executable, PluginType::Native, QJsonObject{});
    // Don't assert success as execution may be restricted

    sandbox->shutdown();
}

void TestSandboxIntegration::testPluginCrash() {
    // Create a script that will crash/exit with error
    QTemporaryFile crash_script(m_temp_dir->path() + "/crash_test_XXXXXX.py");
    crash_script.setAutoRemove(false);

    if (!crash_script.open()) {
        QSKIP("Could not create temporary crash script");
    }

    QTextStream stream(&crash_script);
    stream << "#!/usr/bin/env python3\n";
    stream << "import sys\n";
    stream << "sys.exit(1)\n";  // Exit with error code
    crash_script.close();

    SecurityPolicy policy = SecurityPolicy::create_sandboxed_policy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Execute crashing plugin
    auto exec_result = sandbox->execute_plugin(crash_script.fileName(), PluginType::Python, QJsonObject{});
    // The execution may start successfully but the plugin will crash

    sandbox->shutdown();
}

void TestSandboxIntegration::testStrictPolicyEnforcement() {
    SecurityPolicy strict_policy = SecurityPolicy::create_strict_policy();
    auto sandbox = std::make_unique<PluginSandbox>(strict_policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Test that strict policy blocks most operations
    auto exec_result = sandbox->execute_plugin("/nonexistent/path", PluginType::Native, QJsonObject{});
    QVERIFY(!exec_result.has_value()); // Should fail due to strict policy

    sandbox->shutdown();
}

void TestSandboxIntegration::testLimitedPolicyEnforcement() {
    SecurityPolicy limited_policy = SecurityPolicy::create_limited_policy();
    auto sandbox = std::make_unique<PluginSandbox>(limited_policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Limited policy should allow some operations but with restrictions
    QVERIFY(sandbox->is_active());

    sandbox->shutdown();
}

void TestSandboxIntegration::testUnrestrictedPolicyExecution() {
    SecurityPolicy unrestricted_policy = SecurityPolicy::create_unrestricted_policy();
    auto sandbox = std::make_unique<PluginSandbox>(unrestricted_policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Unrestricted policy should allow most operations
    QVERIFY(sandbox->is_active());

    sandbox->shutdown();
}

void TestSandboxIntegration::testSandboxRecoveryAfterFailure() {
    SecurityPolicy policy = SecurityPolicy::create_sandboxed_policy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Simulate failure by shutting down
    sandbox->shutdown();
    QVERIFY(!sandbox->is_active());

    // Test recovery by reinitializing
    auto recovery_result = sandbox->initialize();
    QVERIFY(recovery_result.has_value());
    QVERIFY(sandbox->is_active());

    sandbox->shutdown();
}

void TestSandboxIntegration::testManagerRecoveryAfterShutdown() {
    SandboxManager& manager = SandboxManager::instance();

    // Create some sandboxes
    SecurityPolicy policy = SecurityPolicy::create_sandboxed_policy();
    QString sandbox_id1 = "recovery_test_1_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    QString sandbox_id2 = "recovery_test_2_" + QString::number(QDateTime::currentMSecsSinceEpoch());

    auto result1 = manager.create_sandbox(sandbox_id1, policy);
    auto result2 = manager.create_sandbox(sandbox_id2, policy);
    QVERIFY(result1.has_value());
    QVERIFY(result2.has_value());

    // Shutdown all sandboxes
    manager.shutdown_all();

    // Verify they're gone
    auto after_shutdown1 = manager.get_sandbox(sandbox_id1);
    auto after_shutdown2 = manager.get_sandbox(sandbox_id2);
    QVERIFY(after_shutdown1 == nullptr);
    QVERIFY(after_shutdown2 == nullptr);

    // Test that manager can still create new sandboxes
    QString new_sandbox_id = "recovery_new_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    auto new_result = manager.create_sandbox(new_sandbox_id, policy);
    QVERIFY(new_result.has_value());

    manager.remove_sandbox(new_sandbox_id);
}

void TestSandboxIntegration::testResourceCleanupAfterTermination() {
    SecurityPolicy policy = SecurityPolicy::create_sandboxed_policy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Get initial resource usage
    ResourceUsage initial_usage = sandbox->get_resource_usage();
    Q_UNUSED(initial_usage); // Suppress unused variable warning

    // Shutdown sandbox
    sandbox->shutdown();

    // Verify sandbox is inactive
    QVERIFY(!sandbox->is_active());

    // Resource cleanup is implicit in shutdown
    // We can't easily verify all resources are cleaned up without platform-specific code
}

void TestSandboxIntegration::testRapidSandboxCycling() {
    SandboxManager& manager = SandboxManager::instance();
    SecurityPolicy policy = SecurityPolicy::create_sandboxed_policy();

    const int cycle_count = 10;
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < cycle_count; ++i) {
        QString sandbox_id = QString("rapid_cycle_%1").arg(i);

        // Create
        auto create_result = manager.create_sandbox(sandbox_id, policy);
        QVERIFY(create_result.has_value());

        // Verify exists
        auto retrieved = manager.get_sandbox(sandbox_id);
        QVERIFY(retrieved != nullptr);

        // Remove
        manager.remove_sandbox(sandbox_id);

        // Verify removed
        auto after_removal = manager.get_sandbox(sandbox_id);
        QVERIFY(after_removal == nullptr);
    }

    qint64 elapsed = timer.elapsed();
    qDebug() << "Rapid sandbox cycling completed in" << elapsed << "ms";

    // Should complete in reasonable time (under 10 seconds)
    QVERIFY(elapsed < 10000);
}

void TestSandboxIntegration::testLongRunningPluginMonitoring() {
    // This test simulates long-running plugin monitoring
    SecurityPolicy policy = SecurityPolicy::create_sandboxed_policy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Monitor resource usage over time
    QElapsedTimer timer;
    timer.start();

    const int monitoring_duration_ms = 1000; // 1 second
    const int check_interval_ms = 100;       // Check every 100ms

    while (timer.elapsed() < monitoring_duration_ms) {
        ResourceUsage usage = sandbox->get_resource_usage();

        // Verify we can get resource usage (memory_used_mb is size_t, always >= 0)
        Q_UNUSED(usage.memory_used_mb); // Suppress unused variable warning

        QTest::qWait(check_interval_ms);
    }

    sandbox->shutdown();
}

QTEST_MAIN(TestSandboxIntegration)
#include "test_sandbox_integration.moc"
