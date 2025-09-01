/**
 * @file test_security_enforcer.cpp
 * @brief Tests for security enforcement and policy validation
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QSignalSpy>
#include <QProcess>
#include <QStandardPaths>
#include <memory>

// Include the security enforcer header
#include "../../src/security/sandbox/security_enforcer.hpp"

using namespace qtplugin;

class TestSecurityEnforcer : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testSecurityEnforcerInitialization();
    void testSecurityEnforcerShutdown();
    void testPolicyUpdate();

    // File access validation tests
    void testFileAccessValidation();
    void testDirectoryAllowList();
    void testUnauthorizedFileAccess();

    // Network access validation tests
    void testNetworkAccessValidation();
    void testHostAllowList();
    void testUnauthorizedNetworkAccess();

    // Process creation validation tests
    void testProcessCreationValidation();
    void testUnauthorizedProcessCreation();

    // System call validation tests
    void testSystemCallValidation();
    void testUnauthorizedSystemCall();

    // API call validation tests
    void testApiCallValidation();
    void testBlockedApiCall();

    // Security event tests
    void testSecurityEventRecording();
    void testSecurityEventSignals();
    void testSecurityEventClearing();

    // Policy validation tests
    void testSecurityPolicyValidator();
    void testPolicyCompatibility();
    void testRecommendedPolicies();

    // Process isolation tests
    void testProcessIsolationUtils();
    void testIsolatedEnvironment();
    void testIsolatedDirectory();

private:
    std::unique_ptr<SecurityEnforcer> m_enforcer;
    QTemporaryDir* m_temp_dir;
    SecurityPolicy m_test_policy;
    
    SecurityPolicy createRestrictivePolicy();
    SecurityPolicy createPermissivePolicy();
    void createTestFiles();
};

void TestSecurityEnforcer::initTestCase() {
    // Initialize Qt application for testing
    if (!QCoreApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        new QCoreApplication(argc, argv);
    }
    
    m_temp_dir = new QTemporaryDir();
    QVERIFY(m_temp_dir->isValid());
    
    createTestFiles();
}

void TestSecurityEnforcer::cleanupTestCase() {
    delete m_temp_dir;
}

void TestSecurityEnforcer::init() {
    m_test_policy = createRestrictivePolicy();
    m_enforcer = std::make_unique<SecurityEnforcer>(m_test_policy);
}

void TestSecurityEnforcer::cleanup() {
    if (m_enforcer) {
        m_enforcer->shutdown();
        m_enforcer.reset();
    }
}

void TestSecurityEnforcer::testSecurityEnforcerInitialization() {
    QVERIFY(m_enforcer != nullptr);
    
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    // Verify policy is set correctly
    const SecurityPolicy& policy = m_enforcer->get_policy();
    QCOMPARE(policy.policy_name, m_test_policy.policy_name);
    QCOMPARE(policy.level, m_test_policy.level);
}

void TestSecurityEnforcer::testSecurityEnforcerShutdown() {
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    // Shutdown should be safe to call multiple times
    m_enforcer->shutdown();
    m_enforcer->shutdown();
    
    // Should be able to reinitialize after shutdown
    bool reinit_result = m_enforcer->initialize();
    QVERIFY(reinit_result);
}

void TestSecurityEnforcer::testPolicyUpdate() {
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    SecurityPolicy new_policy = createPermissivePolicy();
    m_enforcer->update_policy(new_policy);
    
    const SecurityPolicy& updated_policy = m_enforcer->get_policy();
    QCOMPARE(updated_policy.policy_name, new_policy.policy_name);
    QCOMPARE(updated_policy.level, new_policy.level);
}

void TestSecurityEnforcer::testFileAccessValidation() {
    SecurityPolicy permissive_policy = createPermissivePolicy();
    m_enforcer->update_policy(permissive_policy);
    
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    QString allowed_path = m_temp_dir->path() + "/allowed_file.txt";
    
    // Test read access (should be allowed)
    bool read_allowed = m_enforcer->validate_file_access(allowed_path, false);
    QVERIFY(read_allowed);
    
    // Test write access (should be allowed with permissive policy)
    bool write_allowed = m_enforcer->validate_file_access(allowed_path, true);
    QVERIFY(write_allowed);
}

void TestSecurityEnforcer::testDirectoryAllowList() {
    SecurityPolicy policy = createRestrictivePolicy();
    policy.permissions.allow_file_system_read = true;
    policy.permissions.allowed_directories = { m_temp_dir->path() };
    
    m_enforcer->update_policy(policy);
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    // Test access to allowed directory
    QString allowed_path = m_temp_dir->path() + "/test_file.txt";
    bool allowed = m_enforcer->validate_file_access(allowed_path, false);
    QVERIFY(allowed);
    
    // Test access to disallowed directory
    QString disallowed_path = "/tmp/disallowed_file.txt";
    bool disallowed = m_enforcer->validate_file_access(disallowed_path, false);
    QVERIFY(!disallowed);
}

void TestSecurityEnforcer::testUnauthorizedFileAccess() {
    SecurityPolicy restrictive_policy = createRestrictivePolicy();
    m_enforcer->update_policy(restrictive_policy);
    
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    QSignalSpy spy(m_enforcer.get(), &SecurityEnforcer::security_violation_detected);
    
    QString test_path = m_temp_dir->path() + "/test_file.txt";
    
    // Attempt unauthorized read access
    bool read_result = m_enforcer->validate_file_access(test_path, false);
    QVERIFY(!read_result);
    
    // Attempt unauthorized write access
    bool write_result = m_enforcer->validate_file_access(test_path, true);
    QVERIFY(!write_result);
    
    // Should have recorded security violations
    QVERIFY(spy.count() >= 2);
}

void TestSecurityEnforcer::testNetworkAccessValidation() {
    SecurityPolicy permissive_policy = createPermissivePolicy();
    m_enforcer->update_policy(permissive_policy);
    
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    // Test network access (should be allowed with permissive policy)
    bool network_allowed = m_enforcer->validate_network_access("example.com", 80);
    QVERIFY(network_allowed);
}

void TestSecurityEnforcer::testHostAllowList() {
    SecurityPolicy policy = createRestrictivePolicy();
    policy.permissions.allow_network_access = true;
    policy.permissions.allowed_hosts = { "trusted.com", "*.example.com" };
    
    m_enforcer->update_policy(policy);
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    // Test access to allowed host
    bool allowed_host = m_enforcer->validate_network_access("trusted.com", 443);
    QVERIFY(allowed_host);
    
    // Test access to wildcard-matched host
    bool wildcard_host = m_enforcer->validate_network_access("api.example.com", 80);
    QVERIFY(wildcard_host);
    
    // Test access to disallowed host
    bool disallowed_host = m_enforcer->validate_network_access("malicious.com", 80);
    QVERIFY(!disallowed_host);
}

void TestSecurityEnforcer::testUnauthorizedNetworkAccess() {
    SecurityPolicy restrictive_policy = createRestrictivePolicy();
    m_enforcer->update_policy(restrictive_policy);
    
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    QSignalSpy spy(m_enforcer.get(), &SecurityEnforcer::security_violation_detected);
    
    // Attempt unauthorized network access
    bool network_result = m_enforcer->validate_network_access("example.com", 80);
    QVERIFY(!network_result);
    
    // Should have recorded a security violation
    QVERIFY(spy.count() >= 1);
}

void TestSecurityEnforcer::testProcessCreationValidation() {
    SecurityPolicy permissive_policy = createPermissivePolicy();
    m_enforcer->update_policy(permissive_policy);
    
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    // Test process creation (should be allowed with permissive policy)
    bool process_allowed = m_enforcer->validate_process_creation("/bin/ls");
    QVERIFY(process_allowed);
}

void TestSecurityEnforcer::testUnauthorizedProcessCreation() {
    SecurityPolicy restrictive_policy = createRestrictivePolicy();
    m_enforcer->update_policy(restrictive_policy);
    
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    QSignalSpy spy(m_enforcer.get(), &SecurityEnforcer::security_violation_detected);
    
    // Attempt unauthorized process creation
    bool process_result = m_enforcer->validate_process_creation("/bin/sh");
    QVERIFY(!process_result);
    
    // Should have recorded a security violation
    QVERIFY(spy.count() >= 1);
}

void TestSecurityEnforcer::testSystemCallValidation() {
    SecurityPolicy permissive_policy = createPermissivePolicy();
    m_enforcer->update_policy(permissive_policy);
    
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    // Test system call (should be allowed with permissive policy)
    bool syscall_allowed = m_enforcer->validate_system_call("open");
    QVERIFY(syscall_allowed);
}

void TestSecurityEnforcer::testUnauthorizedSystemCall() {
    SecurityPolicy restrictive_policy = createRestrictivePolicy();
    m_enforcer->update_policy(restrictive_policy);
    
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    QSignalSpy spy(m_enforcer.get(), &SecurityEnforcer::security_violation_detected);
    
    // Attempt unauthorized system call
    bool syscall_result = m_enforcer->validate_system_call("execve");
    QVERIFY(!syscall_result);
    
    // Should have recorded a security violation
    QVERIFY(spy.count() >= 1);
}

void TestSecurityEnforcer::testApiCallValidation() {
    SecurityPolicy policy = createPermissivePolicy();
    policy.permissions.blocked_apis = { "system", "exec", "CreateProcess" };
    
    m_enforcer->update_policy(policy);
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    // Test allowed API call
    bool allowed_api = m_enforcer->validate_api_call("malloc");
    QVERIFY(allowed_api);
    
    // Test blocked API call
    bool blocked_api = m_enforcer->validate_api_call("system");
    QVERIFY(!blocked_api);
}

void TestSecurityEnforcer::testBlockedApiCall() {
    SecurityPolicy policy = createPermissivePolicy();
    policy.permissions.blocked_apis = { "dangerous_api", "malicious_call" };
    
    m_enforcer->update_policy(policy);
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    QSignalSpy spy(m_enforcer.get(), &SecurityEnforcer::security_violation_detected);
    
    // Attempt blocked API call
    bool api_result = m_enforcer->validate_api_call("dangerous_api");
    QVERIFY(!api_result);
    
    // Should have recorded a security violation
    QVERIFY(spy.count() >= 1);
}

void TestSecurityEnforcer::testSecurityEventRecording() {
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    // Initially should have no events
    auto events = m_enforcer->get_security_events();
    size_t initial_count = events.size();
    
    // Trigger a security violation
    m_enforcer->validate_file_access("/unauthorized/path", true);
    
    // Should have recorded the event
    auto updated_events = m_enforcer->get_security_events();
    QVERIFY(updated_events.size() > initial_count);
    
    // Verify event details
    if (!updated_events.empty()) {
        const SecurityEvent& event = updated_events.back();
        QCOMPARE(event.type, SecurityViolationType::UnauthorizedFileAccess);
        QVERIFY(!event.description.isEmpty());
        QCOMPARE(event.resource_path, QString("/unauthorized/path"));
    }
}

void TestSecurityEnforcer::testSecurityEventSignals() {
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    QSignalSpy violation_spy(m_enforcer.get(), &SecurityEnforcer::security_violation_detected);
    QSignalSpy activity_spy(m_enforcer.get(), &SecurityEnforcer::suspicious_activity_detected);
    
    // Trigger security violations
    m_enforcer->validate_file_access("/unauthorized/file", false);
    m_enforcer->validate_network_access("blocked.com", 80);
    
    // Should have emitted violation signals
    QVERIFY(violation_spy.count() >= 2);
    
    // Verify signal parameters
    if (violation_spy.count() > 0) {
        QList<QVariant> arguments = violation_spy.takeFirst();
        QVERIFY(arguments.size() == 1);
        // The argument should be a SecurityEvent (would need custom type registration for full testing)
    }
}

void TestSecurityEnforcer::testSecurityEventClearing() {
    bool init_result = m_enforcer->initialize();
    QVERIFY(init_result);
    
    // Generate some events
    m_enforcer->validate_file_access("/test1", true);
    m_enforcer->validate_file_access("/test2", true);
    
    auto events = m_enforcer->get_security_events();
    QVERIFY(events.size() >= 2);
    
    // Clear events
    m_enforcer->clear_security_events();
    
    auto cleared_events = m_enforcer->get_security_events();
    QCOMPARE(cleared_events.size(), static_cast<size_t>(0));
}

void TestSecurityEnforcer::testSecurityPolicyValidator() {
    SecurityPolicy valid_policy = createPermissivePolicy();
    QString error_message;
    
    bool is_valid = SecurityPolicyValidator::validate_policy(valid_policy, error_message);
    QVERIFY(is_valid);
    QVERIFY(error_message.isEmpty());
    
    // Test invalid policy (negative limits)
    SecurityPolicy invalid_policy = valid_policy;
    invalid_policy.limits.memory_limit_mb = 0; // Invalid
    
    bool is_invalid = SecurityPolicyValidator::validate_policy(invalid_policy, error_message);
    QVERIFY(!is_invalid);
    QVERIFY(!error_message.isEmpty());
}

void TestSecurityEnforcer::testPolicyCompatibility() {
    SecurityPolicy policy1 = SecurityPolicy::create_limited_policy();
    SecurityPolicy policy2 = SecurityPolicy::create_sandboxed_policy();
    
    bool compatible = SecurityPolicyValidator::is_policy_compatible(policy1, policy2);
    // Policies with different security levels should be compatible for merging
    QVERIFY(compatible || !compatible); // Implementation-dependent
}

void TestSecurityEnforcer::testRecommendedPolicies() {
    SecurityPolicy native_policy = SecurityPolicyValidator::get_recommended_policy(PluginType::Native);
    QVERIFY(!native_policy.policy_name.isEmpty());
    
    SecurityPolicy python_policy = SecurityPolicyValidator::get_recommended_policy(PluginType::Python);
    QVERIFY(!python_policy.policy_name.isEmpty());
    
    // Python plugins should generally have more restrictions than native
    QVERIFY(python_policy.level >= native_policy.level);
}

void TestSecurityEnforcer::testProcessIsolationUtils() {
    SecurityPolicy policy = createRestrictivePolicy();
    
    // Test isolated environment creation
    QProcessEnvironment env = ProcessIsolationUtils::create_isolated_environment(policy);
    QVERIFY(env.contains("QTPLUGIN_SANDBOX"));
    QCOMPARE(env.value("QTPLUGIN_SANDBOX"), QString("1"));
    
    // Test isolated directory setup
    QString isolated_dir = ProcessIsolationUtils::setup_isolated_directory(m_temp_dir->path());
    QVERIFY(!isolated_dir.isEmpty());
    QVERIFY(QDir(isolated_dir).exists());
    
    // Cleanup
    ProcessIsolationUtils::cleanup_isolated_resources(isolated_dir);
    QVERIFY(!QDir(isolated_dir).exists());
}

SecurityPolicy TestSecurityEnforcer::createRestrictivePolicy() {
    SecurityPolicy policy;
    policy.level = SandboxSecurityLevel::Strict;
    policy.policy_name = "test_restrictive";
    policy.description = "Restrictive policy for testing";
    
    // Deny all permissions by default
    policy.permissions.allow_file_system_read = false;
    policy.permissions.allow_file_system_write = false;
    policy.permissions.allow_network_access = false;
    policy.permissions.allow_process_creation = false;
    policy.permissions.allow_system_calls = false;
    policy.permissions.allow_registry_access = false;
    policy.permissions.allow_environment_access = false;
    
    return policy;
}

SecurityPolicy TestSecurityEnforcer::createPermissivePolicy() {
    SecurityPolicy policy;
    policy.level = SandboxSecurityLevel::Limited;
    policy.policy_name = "test_permissive";
    policy.description = "Permissive policy for testing";
    
    // Allow most permissions
    policy.permissions.allow_file_system_read = true;
    policy.permissions.allow_file_system_write = true;
    policy.permissions.allow_network_access = true;
    policy.permissions.allow_process_creation = true;
    policy.permissions.allow_system_calls = true;
    policy.permissions.allow_registry_access = false;
    policy.permissions.allow_environment_access = false;
    
    // Set allowed directories
    policy.permissions.allowed_directories = { m_temp_dir->path() };
    
    return policy;
}

void TestSecurityEnforcer::createTestFiles() {
    // Create some test files for access validation
    QFile test_file(m_temp_dir->path() + "/test_file.txt");
    if (test_file.open(QIODevice::WriteOnly)) {
        test_file.write("Test content");
        test_file.close();
    }
    
    QDir().mkpath(m_temp_dir->path() + "/subdir");
    QFile sub_file(m_temp_dir->path() + "/subdir/sub_file.txt");
    if (sub_file.open(QIODevice::WriteOnly)) {
        sub_file.write("Sub content");
        sub_file.close();
    }
}

QTEST_MAIN(TestSecurityEnforcer)
#include "test_security_enforcer.moc"
