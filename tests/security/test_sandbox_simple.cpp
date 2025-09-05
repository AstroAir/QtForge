/**
 * @file test_sandbox_simple.cpp
 * @brief Simple test to verify sandbox functionality is enabled and working
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <memory>

#include "qtplugin/security/sandbox/plugin_sandbox.hpp"

using namespace qtplugin;

class TestSandboxSimple : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testSandboxCreation();
    void testSandboxInitialization();
    void testSecurityPolicyCreation();
    void testResourceLimitsCreation();
    void testSandboxManagerSingleton();
};

void TestSandboxSimple::initTestCase() {
    // Initialize Qt application for testing
    if (!QCoreApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        new QCoreApplication(argc, argv);
    }
}

void TestSandboxSimple::testSandboxCreation() {
    // Test that we can create a sandbox with a basic policy
    SecurityPolicy policy = SecurityPolicy::create_unrestricted_policy();
    QVERIFY(!policy.policy_name.isEmpty());
    
    auto sandbox = std::make_unique<PluginSandbox>(policy);
    QVERIFY(sandbox != nullptr);
}

void TestSandboxSimple::testSandboxInitialization() {
    // Test that sandbox can be initialized
    SecurityPolicy policy = SecurityPolicy::create_unrestricted_policy();
    auto sandbox = std::make_unique<PluginSandbox>(policy);
    
    auto init_result = sandbox->initialize();
    QVERIFY(init_result.has_value());
    
    sandbox->shutdown();
}

void TestSandboxSimple::testSecurityPolicyCreation() {
    // Test different security policy creation methods
    auto unrestricted = SecurityPolicy::create_unrestricted_policy();
    QCOMPARE(unrestricted.level, SandboxSecurityLevel::Unrestricted);
    
    auto limited = SecurityPolicy::create_limited_policy();
    QCOMPARE(limited.level, SandboxSecurityLevel::Limited);
    
    auto sandboxed = SecurityPolicy::create_sandboxed_policy();
    QCOMPARE(sandboxed.level, SandboxSecurityLevel::Sandboxed);
    
    auto strict = SecurityPolicy::create_strict_policy();
    QCOMPARE(strict.level, SandboxSecurityLevel::Strict);
}

void TestSandboxSimple::testResourceLimitsCreation() {
    // Test resource limits creation and JSON serialization
    ResourceLimits limits;
    limits.memory_limit_mb = 512;
    limits.cpu_time_limit = std::chrono::minutes(5);
    
    auto json = limits.to_json();
    QVERIFY(!json.isEmpty());
    QVERIFY(json.contains("memory_limit_mb"));
    QVERIFY(json.contains("cpu_time_limit"));
    
    auto restored_result = ResourceLimits::from_json(json);
    QVERIFY(restored_result.has_value());
    
    auto restored = restored_result.value();
    QCOMPARE(restored.memory_limit_mb, limits.memory_limit_mb);
    QCOMPARE(restored.cpu_time_limit, limits.cpu_time_limit);
}

void TestSandboxSimple::testSandboxManagerSingleton() {
    // Test that SandboxManager is a proper singleton
    SandboxManager& manager1 = SandboxManager::instance();
    SandboxManager& manager2 = SandboxManager::instance();
    
    QCOMPARE(&manager1, &manager2);
    
    // Test that we can get default policies
    auto unrestricted_policy = manager1.get_policy("unrestricted");
    QVERIFY(unrestricted_policy.has_value());
    
    auto limited_policy = manager1.get_policy("limited");
    QVERIFY(limited_policy.has_value());
}

QTEST_MAIN(TestSandboxSimple)
#include "test_sandbox_simple.moc"
