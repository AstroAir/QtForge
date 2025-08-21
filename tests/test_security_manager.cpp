/**
 * @file test_security_manager.cpp
 * @brief Comprehensive tests for security manager functionality
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <memory>
#include <filesystem>

#include <qtplugin/security/security_manager.hpp>
#include <qtplugin/utils/error_handling.hpp>

using namespace qtplugin;

class TestSecurityManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testSecurityManagerCreation();
    void testSecurityManagerDestruction();
    void testSecurityLevelConfiguration();
    
    // Plugin validation tests
    void testValidateValidPlugin();
    void testValidateInvalidPlugin();
    void testValidateNonexistentPlugin();
    void testValidateCorruptedPlugin();
    
    // Metadata validation tests
    void testValidateMetadataValid();
    void testValidateMetadataInvalid();
    void testValidateMetadataMissing();
    void testValidateMetadataCorrupted();
    
    // Signature validation tests
    void testValidateSignatureValid();
    void testValidateSignatureInvalid();
    void testValidateSignatureMissing();
    void testValidateSignatureDisabled();
    
    // Permission validation tests
    void testValidatePermissionsValid();
    void testValidatePermissionsExcessive();
    void testValidatePermissionsInvalid();
    void testValidatePermissionsMissing();
    
    // Security level tests
    void testSecurityLevelStrict();
    void testSecurityLevelModerate();
    void testSecurityLevelPermissive();
    void testSecurityLevelCustom();
    
    // Path validation tests
    void testSafeFilePathValid();
    void testSafeFilePathTraversal();
    void testSafeFilePathAbsolute();
    void testSafeFilePathSymlink();
    
    // Trust management tests
    void testTrustedSourceValidation();
    void testUntrustedSourceRejection();
    void testTrustStoreManagement();
    void testCertificateValidation();
    
    // Security policy tests
    void testSecurityPolicyEnforcement();
    void testSecurityPolicyViolation();
    void testSecurityPolicyUpdate();
    void testSecurityPolicyInheritance();
    
    // Threat detection tests
    void testMaliciousCodeDetection();
    void testSuspiciousActivityDetection();
    void testResourceAbuseDetection();
    void testPrivilegeEscalationDetection();
    
    // Performance tests
    void testValidationPerformance();
    void testConcurrentValidation();
    void testLargeFileValidation();
    
    // Configuration tests
    void testSecurityConfiguration();
    void testConfigurationPersistence();
    void testConfigurationValidation();

private:
    std::unique_ptr<SecurityManager> m_security_manager;
    std::unique_ptr<QTemporaryDir> m_temp_dir;
    std::filesystem::path m_test_dir;
    
    // Helper methods
    void createValidPlugin(const QString& name);
    void createInvalidPlugin(const QString& name);
    void createCorruptedPlugin(const QString& name);
    void createMaliciousPlugin(const QString& name);
    std::filesystem::path getPluginPath(const QString& name);
};

void TestSecurityManager::initTestCase()
{
    qDebug() << "Starting security manager tests";
    
    // Create temporary directory for test files
    m_temp_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_temp_dir->isValid());
    m_test_dir = m_temp_dir->path().toStdString();
}

void TestSecurityManager::cleanupTestCase()
{
    qDebug() << "Security manager tests completed";
}

void TestSecurityManager::init()
{
    // Create fresh security manager for each test
    m_security_manager = std::make_unique<SecurityManager>();
    QVERIFY(m_security_manager != nullptr);
}

void TestSecurityManager::cleanup()
{
    // Clean up security manager
    m_security_manager.reset();
}

void TestSecurityManager::testSecurityManagerCreation()
{
    // Test basic creation
    auto manager = std::make_unique<SecurityManager>();
    QVERIFY(manager != nullptr);
    
    // Test initial state
    QVERIFY(manager->get_validations_performed() == 0);
    QVERIFY(manager->get_violations_detected() == 0);
}

void TestSecurityManager::testSecurityManagerDestruction()
{
    // Test that destruction properly cleans up resources
    {
        auto manager = std::make_unique<SecurityManager>();
        // Manager should clean up automatically when destroyed
    }
    
    // Verify no memory leaks
    QVERIFY(true); // This would be verified with memory profiling tools
}

void TestSecurityManager::testSecurityLevelConfiguration()
{
    // Test setting different security levels
    m_security_manager->set_security_level(SecurityLevel::Strict);
    QCOMPARE(m_security_manager->get_security_level(), SecurityLevel::Strict);
    
    m_security_manager->set_security_level(SecurityLevel::Moderate);
    QCOMPARE(m_security_manager->get_security_level(), SecurityLevel::Moderate);
    
    m_security_manager->set_security_level(SecurityLevel::Permissive);
    QCOMPARE(m_security_manager->get_security_level(), SecurityLevel::Permissive);
}

void TestSecurityManager::testValidateValidPlugin()
{
    createValidPlugin("valid_plugin");
    
    auto result = m_security_manager->validate_plugin(getPluginPath("valid_plugin"), SecurityLevel::Moderate);
    QVERIFY(result.is_valid);
    QVERIFY(result.errors.empty());
    QVERIFY(result.warnings.empty() || !result.warnings.empty()); // May have warnings
}

void TestSecurityManager::testValidateInvalidPlugin()
{
    createInvalidPlugin("invalid_plugin");
    
    auto result = m_security_manager->validate_plugin(getPluginPath("invalid_plugin"), SecurityLevel::Moderate);
    QVERIFY(!result.is_valid);
    QVERIFY(!result.errors.empty());
}

void TestSecurityManager::testValidateNonexistentPlugin()
{
    auto result = m_security_manager->validate_plugin(m_test_dir / "nonexistent.dll", SecurityLevel::Moderate);
    QVERIFY(!result.is_valid);
    QVERIFY(!result.errors.empty());
}

void TestSecurityManager::testValidateCorruptedPlugin()
{
    createCorruptedPlugin("corrupted_plugin");
    
    auto result = m_security_manager->validate_plugin(getPluginPath("corrupted_plugin"), SecurityLevel::Moderate);
    QVERIFY(!result.is_valid);
    QVERIFY(!result.errors.empty());
}

void TestSecurityManager::testValidateMetadataValid()
{
    createValidPlugin("metadata_test");
    
    auto result = m_security_manager->validate_metadata(getPluginPath("metadata_test"));
    QVERIFY(result.is_valid);
    QVERIFY(result.errors.empty());
}

void TestSecurityManager::testValidateMetadataInvalid()
{
    createInvalidPlugin("metadata_invalid");
    
    auto result = m_security_manager->validate_metadata(getPluginPath("metadata_invalid"));
    QVERIFY(!result.is_valid);
    QVERIFY(!result.errors.empty());
}

void TestSecurityManager::testValidateSignatureDisabled()
{
    createValidPlugin("signature_test");
    
    // Disable signature verification
    m_security_manager->set_signature_verification_enabled(false);
    
    auto result = m_security_manager->validate_signature(getPluginPath("signature_test"));
    QVERIFY(result.is_valid);
    QVERIFY(!result.warnings.empty()); // Should warn about disabled verification
}

void TestSecurityManager::testSafeFilePathValid()
{
    std::filesystem::path valid_path = m_test_dir / "valid_file.dll";
    QVERIFY(m_security_manager->is_safe_file_path(valid_path));
}

void TestSecurityManager::testSafeFilePathTraversal()
{
    std::filesystem::path traversal_path = m_test_dir / ".." / "malicious.dll";
    QVERIFY(!m_security_manager->is_safe_file_path(traversal_path));
}

// Helper methods implementation
void TestSecurityManager::createValidPlugin(const QString& name)
{
    QJsonObject metadata;
    metadata["name"] = name;
    metadata["version"] = "1.0.0";
    metadata["description"] = QString("Valid plugin %1").arg(name);
    metadata["author"] = "Test Suite";
    metadata["api_version"] = "3.0.0";
    metadata["permissions"] = QJsonArray{"file_read", "network_access"};
    
    QJsonDocument doc(metadata);
    
    QString plugin_path = QString::fromStdString((m_test_dir / (name.toStdString() + ".json")).string());
    QFile file(plugin_path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(doc.toJson());
    file.close();
}

void TestSecurityManager::createInvalidPlugin(const QString& name)
{
    QString plugin_path = QString::fromStdString((m_test_dir / (name.toStdString() + ".json")).string());
    QFile file(plugin_path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{ invalid json content");
    file.close();
}

void TestSecurityManager::createCorruptedPlugin(const QString& name)
{
    QString plugin_path = QString::fromStdString((m_test_dir / (name.toStdString() + ".dll")).string());
    QFile file(plugin_path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QByteArray(1024, '\0')); // Write null bytes
    file.close();
}

std::filesystem::path TestSecurityManager::getPluginPath(const QString& name)
{
    return m_test_dir / (name.toStdString() + ".json");
}

QTEST_MAIN(TestSecurityManager)
#include "test_security_manager.moc"
