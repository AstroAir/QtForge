/**
 * @file test_plugin_sandbox.cpp
 * @brief Comprehensive tests for the plugin sandbox system
 * @version 3.2.0
 */

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTimer>
#include <QtTest/QtTest>
#include <memory>

#include "qtplugin/security/sandbox/plugin_sandbox.hpp"

using namespace qtplugin;

class TestPluginSandbox : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // ResourceUsage tests
    void testResourceUsageToJson();
    void testResourceUsageExceedsLimits();
    void testResourceUsageWithinLimits();

    // SecurityPolicy tests
    void testSecurityPolicyFactoryMethods();
    void testSecurityPolicyJsonSerialization();
    void testSecurityPolicyValidation();

    // PluginSandbox core tests
    void testSandboxInitialization();
    void testSandboxShutdown();
    void testSandboxExecutePlugin();
    void testSandboxResourceMonitoring();
    void testSandboxSecurityEnforcement();

    // SandboxManager tests
    void testSandboxManagerSingleton();
    void testSandboxManagerCreateSandbox();
    void testSandboxManagerPolicyManagement();
    void testSandboxManagerLifecycle();

    // Error handling tests
    void testInvalidPluginPath();
    void testDuplicateSandboxId();
    void testInvalidSecurityPolicy();

    // Signal tests
    void testResourceLimitExceededSignal();
    void testSecurityViolationSignal();
    void testExecutionCompletedSignal();

private:
    QTemporaryDir* m_temp_dir;
    QString m_test_plugin_path;

    void createTestPlugin();
    SecurityPolicy createTestPolicy();
};

void TestPluginSandbox::initTestCase() {
    // Initialize Qt application for testing
    if (!QCoreApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        new QCoreApplication(argc, argv);
    }

    m_temp_dir = new QTemporaryDir();
    QVERIFY(m_temp_dir->isValid());

    createTestPlugin();
}

void TestPluginSandbox::cleanupTestCase() { delete m_temp_dir; }

void TestPluginSandbox::init() {
    // Setup for each test
}

void TestPluginSandbox::cleanup() {
    // Cleanup after each test
}

void TestPluginSandbox::testResourceUsageToJson() {
    ResourceUsage usage;
    usage.cpu_time_used = std::chrono::milliseconds(5000);
    usage.memory_used_mb = 128;
    usage.disk_space_used_mb = 50;
    usage.file_handles_used = 25;
    usage.network_connections_used = 5;
    usage.start_time = std::chrono::steady_clock::now();

    QJsonObject json = usage.to_json();

    QCOMPARE(json["cpu_time_used"].toInt(), 5000);
    QCOMPARE(json["memory_used_mb"].toInt(), 128);
    QCOMPARE(json["disk_space_used_mb"].toInt(), 50);
    QCOMPARE(json["file_handles_used"].toInt(), 25);
    QCOMPARE(json["network_connections_used"].toInt(), 5);
    QVERIFY(json.contains("start_time"));
}

void TestPluginSandbox::testResourceUsageExceedsLimits() {
    ResourceLimits limits;
    limits.cpu_time_limit = std::chrono::milliseconds(1000);
    limits.memory_limit_mb = 100;
    limits.disk_space_limit_mb = 50;
    limits.max_file_handles = 20;
    limits.max_network_connections = 5;
    limits.execution_timeout = std::chrono::milliseconds(10000);

    ResourceUsage usage;
    usage.cpu_time_used = std::chrono::milliseconds(2000);  // Exceeds limit
    usage.memory_used_mb = 80;                              // Within limit
    usage.start_time = std::chrono::steady_clock::now();

    QVERIFY(usage.exceeds_limits(limits));
}

void TestPluginSandbox::testResourceUsageWithinLimits() {
    ResourceLimits limits;
    limits.cpu_time_limit = std::chrono::milliseconds(10000);
    limits.memory_limit_mb = 200;
    limits.disk_space_limit_mb = 100;
    limits.max_file_handles = 50;
    limits.max_network_connections = 10;
    limits.execution_timeout = std::chrono::milliseconds(60000);

    ResourceUsage usage;
    usage.cpu_time_used = std::chrono::milliseconds(5000);
    usage.memory_used_mb = 100;
    usage.disk_space_used_mb = 50;
    usage.file_handles_used = 25;
    usage.network_connections_used = 5;
    usage.start_time = std::chrono::steady_clock::now();

    QVERIFY(!usage.exceeds_limits(limits));
}

void TestPluginSandbox::testSecurityPolicyFactoryMethods() {
    // Test unrestricted policy
    SecurityPolicy unrestricted = SecurityPolicy::create_unrestricted_policy();
    QCOMPARE(unrestricted.level, SandboxSecurityLevel::Unrestricted);
    QCOMPARE(unrestricted.policy_name, QString("unrestricted"));
    QVERIFY(unrestricted.permissions.allow_file_system_read);
    QVERIFY(unrestricted.permissions.allow_file_system_write);
    QVERIFY(unrestricted.permissions.allow_network_access);

    // Test limited policy
    SecurityPolicy limited = SecurityPolicy::create_limited_policy();
    QCOMPARE(limited.level, SandboxSecurityLevel::Limited);
    QCOMPARE(limited.policy_name, QString("limited"));
    QVERIFY(limited.permissions.allow_file_system_read);
    QVERIFY(!limited.permissions.allow_file_system_write);
    QVERIFY(limited.permissions.allow_network_access);
    QVERIFY(!limited.permissions.allow_process_creation);

    // Test sandboxed policy
    SecurityPolicy sandboxed = SecurityPolicy::create_sandboxed_policy();
    QCOMPARE(sandboxed.level, SandboxSecurityLevel::Sandboxed);
    QCOMPARE(sandboxed.policy_name, QString("sandboxed"));
    QVERIFY(!sandboxed.permissions.allow_file_system_read);
    QVERIFY(!sandboxed.permissions.allow_file_system_write);
    QVERIFY(!sandboxed.permissions.allow_network_access);

    // Test strict policy
    SecurityPolicy strict = SecurityPolicy::create_strict_policy();
    QCOMPARE(strict.level, SandboxSecurityLevel::Strict);
    QCOMPARE(strict.policy_name, QString("strict"));
    QVERIFY(!strict.permissions.allow_file_system_read);
    QVERIFY(!strict.permissions.allow_network_access);
    QVERIFY(!strict.permissions.allow_process_creation);
    QVERIFY(!strict.permissions.blocked_apis.isEmpty());
}

void TestPluginSandbox::testSecurityPolicyJsonSerialization() {
    SecurityPolicy original = SecurityPolicy::create_limited_policy();

    // Serialize to JSON
    QJsonObject json = original.to_json();

    // Deserialize from JSON
    auto result = SecurityPolicy::from_json(json);
    QVERIFY(result.has_value());

    SecurityPolicy deserialized = result.value();

    // Verify the deserialized policy matches the original
    QCOMPARE(deserialized.level, original.level);
    QCOMPARE(deserialized.policy_name, original.policy_name);
    QCOMPARE(deserialized.description, original.description);
    QCOMPARE(deserialized.permissions.allow_file_system_read,
             original.permissions.allow_file_system_read);
    QCOMPARE(deserialized.permissions.allow_network_access,
             original.permissions.allow_network_access);
}

void TestPluginSandbox::testSandboxInitialization() {
    SecurityPolicy policy = createTestPolicy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);

    QVERIFY(!sandbox->is_active());

    auto result = sandbox->initialize();
    QVERIFY(result.has_value());
    QVERIFY(sandbox->is_active());

    sandbox->shutdown();
    QVERIFY(!sandbox->is_active());
}

void TestPluginSandbox::testSandboxShutdown() {
    SecurityPolicy policy = createTestPolicy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);

    auto result = sandbox->initialize();
    QVERIFY(result.has_value());
    QVERIFY(sandbox->is_active());

    sandbox->shutdown();
    QVERIFY(!sandbox->is_active());

    // Test double shutdown (should be safe)
    sandbox->shutdown();
    QVERIFY(!sandbox->is_active());
}

void TestPluginSandbox::testSandboxExecutePlugin() {
    SecurityPolicy policy = SecurityPolicy::create_unrestricted_policy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Test with non-existent plugin (should fail)
    auto exec_result =
        sandbox->execute_plugin("/non/existent/plugin", PluginType::Native);
    QVERIFY(!exec_result.has_value());

    sandbox->shutdown();
}

void TestPluginSandbox::testSandboxResourceMonitoring() {
    SecurityPolicy policy = SecurityPolicy::create_limited_policy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Get initial resource usage
    ResourceUsage usage = sandbox->get_resource_usage();
    QVERIFY(usage.cpu_time_used >= std::chrono::milliseconds(0));
    // memory_used_mb is size_t (unsigned), so always >= 0

    sandbox->shutdown();
}

void TestPluginSandbox::testSandboxManagerSingleton() {
    SandboxManager& manager1 = SandboxManager::instance();
    SandboxManager& manager2 = SandboxManager::instance();

    // Verify singleton behavior
    QCOMPARE(&manager1, &manager2);
}

void TestPluginSandbox::testSandboxManagerCreateSandbox() {
    SandboxManager& manager = SandboxManager::instance();
    SecurityPolicy policy = createTestPolicy();

    QString sandbox_id =
        "test_sandbox_" + QString::number(QDateTime::currentMSecsSinceEpoch());

    // Create sandbox
    auto result = manager.create_sandbox(sandbox_id, policy);
    QVERIFY(result.has_value());

    auto sandbox = result.value();
    QVERIFY(sandbox != nullptr);
    QVERIFY(sandbox->is_active());

    // Verify we can retrieve it
    auto retrieved = manager.get_sandbox(sandbox_id);
    QVERIFY(retrieved != nullptr);
    QCOMPARE(retrieved.get(), sandbox.get());

    // Test duplicate creation (should fail)
    auto duplicate_result = manager.create_sandbox(sandbox_id, policy);
    QVERIFY(!duplicate_result.has_value());

    // Cleanup
    manager.remove_sandbox(sandbox_id);

    // Verify removal
    auto removed = manager.get_sandbox(sandbox_id);
    QVERIFY(removed == nullptr);
}

void TestPluginSandbox::testSandboxManagerPolicyManagement() {
    SandboxManager& manager = SandboxManager::instance();

    // Test getting default policies
    auto policies = manager.get_registered_policies();
    QVERIFY(policies.size() >= 4);  // At least the 4 default policies

    // Test getting a specific policy
    auto policy_result = manager.get_policy("strict");
    QVERIFY(policy_result.has_value());

    SecurityPolicy policy = policy_result.value();
    QCOMPARE(policy.policy_name, QString("strict"));
    QCOMPARE(policy.level, SandboxSecurityLevel::Strict);

    // Test registering a custom policy
    SecurityPolicy custom_policy = createTestPolicy();
    custom_policy.policy_name = "test_custom";

    manager.register_policy("test_custom", custom_policy);

    auto custom_result = manager.get_policy("test_custom");
    QVERIFY(custom_result.has_value());
    QCOMPARE(custom_result.value().policy_name, QString("test_custom"));

    // Test getting non-existent policy
    auto missing_result = manager.get_policy("non_existent");
    QVERIFY(!missing_result.has_value());
}

void TestPluginSandbox::testInvalidPluginPath() {
    SecurityPolicy policy = createTestPolicy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Test with empty path
    auto result1 = sandbox->execute_plugin("", PluginType::Native);
    QVERIFY(!result1.has_value());

    // Test with non-existent path
    auto result2 = sandbox->execute_plugin("/path/that/does/not/exist",
                                           PluginType::Native);
    QVERIFY(!result2.has_value());

    sandbox->shutdown();
}

void TestPluginSandbox::testDuplicateSandboxId() {
    SandboxManager& manager = SandboxManager::instance();
    SecurityPolicy policy = createTestPolicy();

    QString sandbox_id = "duplicate_test_" +
                         QString::number(QDateTime::currentMSecsSinceEpoch());

    // Create first sandbox
    auto result1 = manager.create_sandbox(sandbox_id, policy);
    QVERIFY(result1.has_value());

    // Try to create duplicate (should fail)
    auto result2 = manager.create_sandbox(sandbox_id, policy);
    QVERIFY(!result2.has_value());

    // Cleanup
    manager.remove_sandbox(sandbox_id);
}

void TestPluginSandbox::testResourceLimitExceededSignal() {
    SecurityPolicy policy = SecurityPolicy::create_strict_policy();
    // Set very low limits for testing
    policy.limits.memory_limit_mb = 1;
    policy.limits.cpu_time_limit = std::chrono::milliseconds(1);

    auto sandbox = std::make_unique<PluginSandbox>(policy);

    QSignalSpy spy(sandbox.get(), &PluginSandbox::resource_limit_exceeded);

    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());

    // Note: In a real test, we would need to actually trigger resource usage
    // For now, we just verify the signal can be connected
    QVERIFY(spy.isValid());

    sandbox->shutdown();
}

void TestPluginSandbox::createTestPlugin() {
    // Create a simple test script that can be executed
    QTemporaryFile temp_file(m_temp_dir->path() + "/test_plugin_XXXXXX.py");
    temp_file.setAutoRemove(false);

    if (temp_file.open()) {
        QTextStream stream(&temp_file);
        stream << "#!/usr/bin/env python3\n";
        stream << "import sys\n";
        stream << "import time\n";
        stream << "print('Test plugin started')\n";
        stream << "time.sleep(0.1)\n";
        stream << "print('Test plugin completed')\n";
        stream << "sys.exit(0)\n";

        m_test_plugin_path = temp_file.fileName();
        temp_file.close();

        // Make executable on Unix systems
        QFile::setPermissions(m_test_plugin_path, QFile::ReadOwner |
                                                      QFile::WriteOwner |
                                                      QFile::ExeOwner);
    }
}

SecurityPolicy TestPluginSandbox::createTestPolicy() {
    SecurityPolicy policy;
    policy.level = SandboxSecurityLevel::Limited;
    policy.policy_name = "test_policy";
    policy.description = "Policy for testing";

    policy.limits.cpu_time_limit = std::chrono::minutes(1);
    policy.limits.memory_limit_mb = 256;
    policy.limits.disk_space_limit_mb = 100;
    policy.limits.max_file_handles = 50;
    policy.limits.max_network_connections = 10;
    policy.limits.execution_timeout = std::chrono::seconds(30);

    policy.permissions.allow_file_system_read = true;
    policy.permissions.allow_file_system_write = false;
    policy.permissions.allow_network_access = false;
    policy.permissions.allow_process_creation = false;
    policy.permissions.allow_system_calls = false;
    policy.permissions.allow_registry_access = false;
    policy.permissions.allow_environment_access = false;

    return policy;
}

QTEST_MAIN(TestPluginSandbox)
#include "test_plugin_sandbox.moc"
