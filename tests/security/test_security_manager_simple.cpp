/**
 * @file test_security_manager_simple.cpp
 * @brief Simple security manager tests
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <filesystem>
#include <memory>

#include "qtplugin/security/security_manager.hpp"

using namespace qtplugin;

class TestSecurityManagerSimple : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Basic functionality tests
    void testSecurityManagerCreation();
    void testSecurityLevelManagement();
    void testBasicValidation();
    void testStatistics();

private:
    std::unique_ptr<SecurityManager> m_security_manager;
};

void TestSecurityManagerSimple::initTestCase() {
    qDebug() << "Starting simple security manager tests";
}

void TestSecurityManagerSimple::cleanupTestCase() {
    qDebug() << "Simple security manager tests completed";
}

void TestSecurityManagerSimple::testSecurityManagerCreation() {
    // Test basic creation
    auto manager = std::make_unique<SecurityManager>();
    QVERIFY(manager != nullptr);

    // Test default security level
    QCOMPARE(manager->security_level(), SecurityLevel::Basic);
    QCOMPARE(manager->get_security_level(), SecurityLevel::Basic);
}

void TestSecurityManagerSimple::testSecurityLevelManagement() {
    auto manager = std::make_unique<SecurityManager>();

    // Test setting different security levels
    manager->set_security_level(SecurityLevel::Basic);
    QCOMPARE(manager->security_level(), SecurityLevel::Basic);
    QCOMPARE(manager->get_security_level(), SecurityLevel::Basic);

    manager->set_security_level(SecurityLevel::Strict);
    QCOMPARE(manager->security_level(), SecurityLevel::Strict);

    manager->set_security_level(SecurityLevel::Maximum);
    QCOMPARE(manager->security_level(), SecurityLevel::Maximum);

    // Test backward compatibility aliases
    manager->set_security_level(SecurityLevel::Moderate);
    QCOMPARE(manager->security_level(), SecurityLevel::Moderate);

    manager->set_security_level(SecurityLevel::Permissive);
    QCOMPARE(manager->security_level(), SecurityLevel::Permissive);
}

void TestSecurityManagerSimple::testBasicValidation() {
    auto manager = std::make_unique<SecurityManager>();

    // Test validation of a non-existent file (should fail)
    std::filesystem::path non_existent_file = "non_existent_plugin.dll";
    auto result =
        manager->validate_plugin(non_existent_file, SecurityLevel::Standard);
    QVERIFY(!result.is_valid);
    QVERIFY(result.has_errors());
    QVERIFY(!result.errors.empty());

    // Test validation with different security levels
    manager->set_security_level(SecurityLevel::None);
    auto result_none =
        manager->validate_plugin(non_existent_file, SecurityLevel::None);
    QVERIFY(!result_none.is_valid);  // Should still fail for non-existent file

    manager->set_security_level(SecurityLevel::Maximum);
    auto result_max =
        manager->validate_plugin(non_existent_file, SecurityLevel::Maximum);
    QVERIFY(!result_max.is_valid);  // Should still fail for non-existent file
}

void TestSecurityManagerSimple::testStatistics() {
    auto manager = std::make_unique<SecurityManager>();

    // Test initial statistics
    QCOMPARE(manager->get_validations_performed(), 0ULL);
    QCOMPARE(manager->get_violations_detected(), 0ULL);

    // Perform some validations to update statistics
    std::filesystem::path non_existent_file = "test_plugin.dll";
    manager->validate_plugin(non_existent_file, SecurityLevel::Standard);

    // Statistics should be updated
    QVERIFY(manager->get_validations_performed() > 0);

    // Test security statistics
    auto stats = manager->security_statistics();
    QVERIFY(stats.contains("validations_performed"));
    QVERIFY(stats.contains("validations_passed"));
    QVERIFY(stats.contains("validations_failed"));

    // Verify statistics are consistent
    auto validations_performed = stats["validations_performed"].toInt();
    auto validations_passed = stats["validations_passed"].toInt();
    auto validations_failed = stats["validations_failed"].toInt();

    QCOMPARE(validations_performed, validations_passed + validations_failed);
    QVERIFY(validations_performed > 0);
}

QTEST_MAIN(TestSecurityManagerSimple)
#include "test_security_manager_simple.moc"
