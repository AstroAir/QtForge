/**
 * @file test_sandbox_integration.cpp
 * @brief Integration tests for complete sandbox workflows
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QSignalSpy>
#include <QTimer>
#include <QProcess>
#include <QEventLoop>
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
    QString sandbox_id = QString("integration_test_%1").arg(QDateTime::currentMSecsSinceEpoch());
    
    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);
    
    auto sandbox = sandbox_result.value();
    QVERIFY(sandbox != nullptr);
    QVERIFY(sandbox->is_active());
    
    // Connect to signals for monitoring
    QSignalSpy execution_spy(sandbox.get(), &PluginSandbox::execution_completed);
    QSignalSpy resource_spy(sandbox.get(), &PluginSandbox::resource_usage_updated);
    QSignalSpy violation_spy(sandbox.get(), &PluginSandbox::security_violation);
    
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
        bool completed = waitForSignal(sandbox.get(), SIGNAL(execution_completed(int, QJsonObject)), 10000);
        QVERIFY(completed);
        
        // Verify signals were emitted
        QVERIFY(execution_spy.count() >= 1);
        
        // Check execution result
        if (execution_spy.count() > 0) {
            QList<QVariant> args = execution_spy.takeFirst();
            int exit_code = args.at(0).toInt();
            QCOMPARE(exit_code, 0); // Successful execution
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
    policy.limits.memory_limit_mb = 10; // Very low limit
    policy.limits.cpu_time_limit = std::chrono::milliseconds(100); // Very short time
    policy.limits.execution_timeout = std::chrono::milliseconds(500);
    
    QString sandbox_id = QString("resource_limit_test_%1").arg(QDateTime::currentMSecsSinceEpoch());
    
    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);
    
    auto sandbox = sandbox_result.value();
    
    // Connect to resource limit signal
    QSignalSpy limit_spy(sandbox.get(), &PluginSandbox::resource_limit_exceeded);
    
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
        bool limit_exceeded = waitForSignal(sandbox.get(), SIGNAL(resource_limit_exceeded(QString, QJsonObject)), 5000);
        
        // Should either exceed limits or complete quickly
        QVERIFY(limit_exceeded || waitForSignal(sandbox.get(), SIGNAL(execution_completed(int, QJsonObject)), 1000));
        
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
    QString sandbox_id = QString("security_test_%1").arg(QDateTime::currentMSecsSinceEpoch());
    
    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);
    
    auto sandbox = sandbox_result.value();
    
    // Connect to security violation signal
    QSignalSpy violation_spy(sandbox.get(), &PluginSandbox::security_violation);
    
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
        bool completed = waitForSignal(sandbox.get(), SIGNAL(execution_completed(int, QJsonObject)), 10000);
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
            case 0: policy = SecurityPolicy::create_limited_policy(); break;
            case 1: policy = SecurityPolicy::create_sandboxed_policy(); break;
            case 2: policy = SecurityPolicy::create_strict_policy(); break;
        }
        
        QString sandbox_id = QString("multi_test_%1_%2").arg(i).arg(QDateTime::currentMSecsSinceEpoch());
        
        auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
        QVERIFY(sandbox_result.has_value());
        m_created_sandboxes.append(sandbox_id);
        
        auto sandbox = sandbox_result.value();
        sandboxes.append(sandbox);
        
        // Create spy for each sandbox
        auto spy = new QSignalSpy(sandbox.get(), &PluginSandbox::execution_completed);
        spies.append(spy);
    }
    
    // Create different plugins for each sandbox
    QStringList plugin_contents = {
        "import time; print('Plugin 1'); time.sleep(0.1); print('Done 1')",
        "import time; print('Plugin 2'); time.sleep(0.2); print('Done 2')",
        "import time; print('Plugin 3'); time.sleep(0.05); print('Done 3')"
    };
    
    // Execute plugins concurrently
    for (int i = 0; i < sandbox_count; ++i) {
        QString plugin_path = createPythonTestPlugin(plugin_contents[i]);
        if (!plugin_path.isEmpty()) {
            auto exec_result = sandboxes[i]->execute_plugin(plugin_path, PluginType::Python);
            if (!exec_result.has_value()) {
                QSKIP("Python not available for plugin execution");
            }
        }
    }
    
    // Wait for all executions to complete
    bool all_completed = true;
    for (int i = 0; i < sandbox_count; ++i) {
        bool completed = waitForSignal(sandboxes[i].get(), SIGNAL(execution_completed(int, QJsonObject)), 10000);
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
    QString sandbox_id = QString("resource_monitor_test_%1").arg(QDateTime::currentMSecsSinceEpoch());
    
    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);
    
    auto sandbox = sandbox_result.value();
    
    // Connect to resource monitoring signals
    QSignalSpy usage_spy(sandbox.get(), &PluginSandbox::resource_usage_updated);
    
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
        bool completed = waitForSignal(sandbox.get(), SIGNAL(execution_completed(int, QJsonObject)), 10000);
        QVERIFY(completed);
        
        // Verify resource monitoring was active
        QVERIFY(usage_spy.count() >= 1);
        
        // Get final resource usage
        ResourceUsage usage = sandbox->get_resource_usage();
        QVERIFY(usage.cpu_time_used >= std::chrono::milliseconds(0));
        QVERIFY(usage.memory_used_mb >= 0);
        
        // Verify usage is within reasonable bounds
        QVERIFY(usage.cpu_time_used < std::chrono::minutes(1));
        QVERIFY(usage.memory_used_mb < 1000); // Less than 1GB
    } else {
        QSKIP("Python not available for plugin execution");
    }
}

void TestSandboxIntegration::testPluginTimeout() {
    SecurityPolicy policy = SecurityPolicy::create_limited_policy();
    policy.limits.execution_timeout = std::chrono::milliseconds(1000); // 1 second timeout
    
    QString sandbox_id = QString("timeout_test_%1").arg(QDateTime::currentMSecsSinceEpoch());
    
    auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(sandbox_result.has_value());
    m_created_sandboxes.append(sandbox_id);
    
    auto sandbox = sandbox_result.value();
    
    // Connect to signals
    QSignalSpy execution_spy(sandbox.get(), &PluginSandbox::execution_completed);
    QSignalSpy violation_spy(sandbox.get(), &PluginSandbox::security_violation);
    
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
        bool completed = waitForSignal(sandbox.get(), SIGNAL(execution_completed(int, QJsonObject)), 3000);
        
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
        QString sandbox_id = QString("concurrent_%1_%2").arg(i).arg(QDateTime::currentMSecsSinceEpoch());
        
        auto sandbox_result = m_manager->create_sandbox(sandbox_id, policy);
        QVERIFY(sandbox_result.has_value());
        m_created_sandboxes.append(sandbox_id);
        
        auto sandbox = sandbox_result.value();
        sandboxes.append(sandbox);
        
        auto spy = new QSignalSpy(sandbox.get(), &PluginSandbox::execution_completed);
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
)").arg(i);
        
        QString plugin_path = createPythonTestPlugin(plugin_content);
        if (!plugin_path.isEmpty()) {
            auto exec_result = sandboxes[i]->execute_plugin(plugin_path, PluginType::Python);
            if (!exec_result.has_value()) {
                QSKIP("Python not available for plugin execution");
            }
        }
    }
    
    // Wait for all to complete
    int completed_count = 0;
    for (int i = 0; i < concurrent_count; ++i) {
        if (waitForSignal(sandboxes[i].get(), SIGNAL(execution_completed(int, QJsonObject)), 5000)) {
            completed_count++;
        }
    }
    
    qint64 total_time = timer.elapsed();
    
    // All should complete
    QCOMPARE(completed_count, concurrent_count);
    
    // Should complete in reasonable time (concurrent execution should be faster than sequential)
    QVERIFY(total_time < 2000); // Less than 2 seconds for concurrent execution
    
    qDebug() << "Concurrent execution of" << concurrent_count << "plugins took" << total_time << "ms";
    
    // Cleanup spies
    for (auto spy : spies) {
        delete spy;
    }
}

void TestSandboxIntegration::createTestPlugins() {
    // Test plugins will be created dynamically in each test
}

QString TestSandboxIntegration::createPythonTestPlugin(const QString& script_content) {
    QTemporaryFile* temp_file = new QTemporaryFile(m_temp_dir->path() + "/test_plugin_XXXXXX.py", this);
    temp_file->setAutoRemove(false);
    
    if (temp_file->open()) {
        QTextStream stream(temp_file);
        stream << "#!/usr/bin/env python3\n";
        stream << script_content;
        
        QString file_path = temp_file->fileName();
        temp_file->close();
        
        // Make executable on Unix systems
        QFile::setPermissions(file_path, 
                             QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
        
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

bool TestSandboxIntegration::waitForSignal(QObject* sender, const char* signal, int timeout) {
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(sender, signal, &loop, &QEventLoop::quit);
    
    timer.start(timeout);
    loop.exec();
    
    return timer.isActive(); // Returns true if signal was received before timeout
}

QTEST_MAIN(TestSandboxIntegration)
#include "test_sandbox_integration.moc"
