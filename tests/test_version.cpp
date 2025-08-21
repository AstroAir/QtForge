/**
 * @file test_version.cpp
 * @brief Comprehensive tests for version utilities
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <memory>

#include <qtplugin/utils/version.hpp>
#include <qtplugin/utils/error_handling.hpp>

using namespace qtplugin;

class TestVersion : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Version creation tests
    void testVersionCreation();
    void testVersionFromString();
    void testVersionFromComponents();
    void testInvalidVersionString();
    
    // Version comparison tests
    void testVersionEquality();
    void testVersionInequality();
    void testVersionLessThan();
    void testVersionGreaterThan();
    void testVersionLessEqual();
    void testVersionGreaterEqual();
    
    // Version compatibility tests
    void testCompatibilityCheck();
    void testMajorVersionCompatibility();
    void testMinorVersionCompatibility();
    void testPatchVersionCompatibility();
    void testPreReleaseCompatibility();
    
    // Version parsing tests
    void testParseValidVersions();
    void testParseInvalidVersions();
    void testParseSemanticVersions();
    void testParseCustomVersions();
    
    // Version formatting tests
    void testVersionToString();
    void testVersionToStringCustomFormat();
    void testVersionSerialization();
    void testVersionDeserialization();
    
    // Version range tests
    void testVersionRange();
    void testVersionRangeInclusive();
    void testVersionRangeExclusive();
    void testVersionRangeIntersection();
    
    // Version constraint tests
    void testVersionConstraints();
    void testConstraintSatisfaction();
    void testConstraintViolation();
    void testComplexConstraints();
    
    // Pre-release version tests
    void testPreReleaseVersions();
    void testAlphaVersions();
    void testBetaVersions();
    void testReleaseCandidate();
    
    // Build metadata tests
    void testBuildMetadata();
    void testBuildMetadataComparison();
    void testBuildMetadataIgnored();
    
    // Edge cases tests
    void testZeroVersions();
    void testLargeVersionNumbers();
    void testSpecialCharacters();
    void testUnicodeVersions();
    
    // Performance tests
    void testVersionComparisonPerformance();
    void testVersionParsingPerformance();
    void testVersionCreationPerformance();
    
    // Utility function tests
    void testVersionUtilities();
    void testVersionValidation();
    void testVersionNormalization();

private:
    // Helper methods
    void verifyVersionComponents(const Version& version, int major, int minor, int patch);
    void verifyVersionString(const Version& version, const std::string& expected);
    void testVersionComparison(const std::string& v1, const std::string& v2, int expected_result);
};

void TestVersion::initTestCase()
{
    qDebug() << "Starting version utility tests";
}

void TestVersion::cleanupTestCase()
{
    qDebug() << "Version utility tests completed";
}

void TestVersion::testVersionCreation()
{
    // Test default constructor
    Version default_version;
    QCOMPARE(default_version.major(), 0);
    QCOMPARE(default_version.minor(), 0);
    QCOMPARE(default_version.patch(), 0);
    QVERIFY(default_version.prerelease().empty());
    QVERIFY(default_version.build().empty());
    
    // Test parameterized constructor
    Version version(1, 2, 3);
    QCOMPARE(version.major(), 1);
    QCOMPARE(version.minor(), 2);
    QCOMPARE(version.patch(), 3);
    
    // Test constructor with pre-release
    Version pre_release_version(2, 0, 0, "alpha.1");
    QCOMPARE(pre_release_version.major(), 2);
    QCOMPARE(pre_release_version.minor(), 0);
    QCOMPARE(pre_release_version.patch(), 0);
    QCOMPARE(pre_release_version.prerelease(), "alpha.1");
    
    // Test constructor with build metadata
    Version build_version(1, 0, 0, "", "20231201.1");
    QCOMPARE(build_version.major(), 1);
    QCOMPARE(build_version.minor(), 0);
    QCOMPARE(build_version.patch(), 0);
    QCOMPARE(build_version.build(), "20231201.1");
}

void TestVersion::testVersionFromString()
{
    // Test valid version strings
    auto result1 = Version::parse("1.2.3");
    QVERIFY(result1.has_value());
    verifyVersionComponents(result1.value(), 1, 2, 3);
    
    auto result2 = Version::parse("2.0.0-alpha.1");
    QVERIFY(result2.has_value());
    verifyVersionComponents(result2.value(), 2, 0, 0);
    QCOMPARE(result2.value().prerelease(), "alpha.1");

    auto result3 = Version::parse("1.0.0+20231201.1");
    QVERIFY(result3.has_value());
    verifyVersionComponents(result3.value(), 1, 0, 0);
    QCOMPARE(result3.value().build(), "20231201.1");

    auto result4 = Version::parse("3.1.4-beta.2+build.123");
    QVERIFY(result4.has_value());
    verifyVersionComponents(result4.value(), 3, 1, 4);
    QCOMPARE(result4.value().prerelease(), "beta.2");
    QCOMPARE(result4.value().build(), "build.123");
}

void TestVersion::testInvalidVersionString()
{
    // Test invalid version strings
    auto result1 = Version::parse("");
    QVERIFY(!result1.has_value());

    auto result2 = Version::parse("1.2");
    QVERIFY(!result2.has_value());

    auto result3 = Version::parse("1.2.3.4");
    QVERIFY(!result3.has_value());

    auto result4 = Version::parse("a.b.c");
    QVERIFY(!result4.has_value());

    auto result5 = Version::parse("1.-2.3");
    QVERIFY(!result5.has_value());
}

void TestVersion::testVersionEquality()
{
    Version v1(1, 2, 3);
    Version v2(1, 2, 3);
    Version v3(1, 2, 4);
    
    QVERIFY(v1 == v2);
    QVERIFY(!(v1 == v3));
    QVERIFY(v1 != v3);
    QVERIFY(!(v1 != v2));
}

void TestVersion::testVersionLessThan()
{
    testVersionComparison("1.0.0", "2.0.0", -1);
    testVersionComparison("1.0.0", "1.1.0", -1);
    testVersionComparison("1.0.0", "1.0.1", -1);
    testVersionComparison("1.0.0-alpha", "1.0.0", -1);
    testVersionComparison("1.0.0-alpha", "1.0.0-beta", -1);
    testVersionComparison("1.0.0-alpha.1", "1.0.0-alpha.2", -1);
}

void TestVersion::testVersionGreaterThan()
{
    testVersionComparison("2.0.0", "1.0.0", 1);
    testVersionComparison("1.1.0", "1.0.0", 1);
    testVersionComparison("1.0.1", "1.0.0", 1);
    testVersionComparison("1.0.0", "1.0.0-alpha", 1);
    testVersionComparison("1.0.0-beta", "1.0.0-alpha", 1);
    testVersionComparison("1.0.0-alpha.2", "1.0.0-alpha.1", 1);
}

void TestVersion::testCompatibilityCheck()
{
    Version v1_0_0(1, 0, 0);
    Version v1_1_0(1, 1, 0);
    Version v1_0_1(1, 0, 1);
    Version v2_0_0(2, 0, 0);
    
    // Same major version should be compatible
    QVERIFY(v1_0_0.is_compatible_with(v1_1_0));
    QVERIFY(v1_0_0.is_compatible_with(v1_0_1));
    QVERIFY(v1_1_0.is_compatible_with(v1_0_0));
    
    // Different major version should not be compatible
    QVERIFY(!v1_0_0.is_compatible_with(v2_0_0));
    QVERIFY(!v2_0_0.is_compatible_with(v1_0_0));
}

void TestVersion::testVersionToString()
{
    Version v1(1, 2, 3);
    QCOMPARE(v1.to_string(), "1.2.3");
    
    Version v2(2, 0, 0, "alpha.1");
    QCOMPARE(v2.to_string(), "2.0.0-alpha.1");
    
    Version v3(1, 0, 0, "", "20231201.1");
    QCOMPARE(v3.to_string(), "1.0.0+20231201.1");
    
    Version v4(3, 1, 4, "beta.2", "build.123");
    QCOMPARE(v4.to_string(), "3.1.4-beta.2+build.123");
}

void TestVersion::testVersionRange()
{
    VersionRange range(Version(1, 0, 0), Version(2, 0, 0));
    
    QVERIFY(range.satisfies(Version(1, 0, 0)));
    QVERIFY(range.satisfies(Version(1, 5, 0)));
    QVERIFY(range.satisfies(Version(1, 9, 9)));
    QVERIFY(!range.satisfies(Version(0, 9, 9)));
    QVERIFY(!range.satisfies(Version(2, 0, 0)));
    QVERIFY(!range.satisfies(Version(2, 0, 1)));
}

void TestVersion::testPreReleaseVersions()
{
    auto alpha = Version::parse("1.0.0-alpha");
    auto beta = Version::parse("1.0.0-beta");
    auto rc = Version::parse("1.0.0-rc.1");
    auto release = Version::parse("1.0.0");
    
    QVERIFY(alpha.has_value());
    QVERIFY(beta.has_value());
    QVERIFY(rc.has_value());
    QVERIFY(release.has_value());
    
    // Pre-release versions should be less than release version
    QVERIFY(alpha.value() < release.value());
    QVERIFY(beta.value() < release.value());
    QVERIFY(rc.value() < release.value());
    
    // Alpha < Beta < RC
    QVERIFY(alpha.value() < beta.value());
    QVERIFY(beta.value() < rc.value());
}

void TestVersion::testBuildMetadata()
{
    auto v1 = Version::parse("1.0.0+build.1");
    auto v2 = Version::parse("1.0.0+build.2");
    auto v3 = Version::parse("1.0.0");
    
    QVERIFY(v1.has_value());
    QVERIFY(v2.has_value());
    QVERIFY(v3.has_value());
    
    // Build metadata should be ignored in comparison
    QVERIFY(v1.value() == v2.value());
    QVERIFY(v1.value() == v3.value());
    QVERIFY(v2.value() == v3.value());
}

void TestVersion::testVersionValidation()
{
    // Test valid versions
    QVERIFY(Version::parse("1.0.0").has_value());
    QVERIFY(Version::parse("10.20.30").has_value());
    QVERIFY(Version::parse("1.0.0-alpha").has_value());
    QVERIFY(Version::parse("1.0.0+build").has_value());
    QVERIFY(Version::parse("1.0.0-alpha+build").has_value());

    // Test invalid versions
    QVERIFY(!Version::parse("").has_value());
    QVERIFY(!Version::parse("1.0").has_value());
    QVERIFY(!Version::parse("1.0.0.0").has_value());
    QVERIFY(!Version::parse("a.b.c").has_value());
    QVERIFY(!Version::parse("1.-1.0").has_value());
}

// Helper methods implementation
void TestVersion::verifyVersionComponents(const Version& version, int major, int minor, int patch)
{
    QCOMPARE(version.major(), major);
    QCOMPARE(version.minor(), minor);
    QCOMPARE(version.patch(), patch);
}

void TestVersion::verifyVersionString(const Version& version, const std::string& expected)
{
    QCOMPARE(version.to_string(), expected);
}

void TestVersion::testVersionComparison(const std::string& v1_str, const std::string& v2_str, int expected_result)
{
    auto v1 = Version::parse(v1_str);
    auto v2 = Version::parse(v2_str);
    
    QVERIFY(v1.has_value());
    QVERIFY(v2.has_value());
    
    if (expected_result < 0) {
        QVERIFY(v1.value() < v2.value());
        QVERIFY(!(v1.value() >= v2.value()));
    } else if (expected_result > 0) {
        QVERIFY(v1.value() > v2.value());
        QVERIFY(!(v1.value() <= v2.value()));
    } else {
        QVERIFY(v1.value() == v2.value());
        QVERIFY(!(v1.value() != v2.value()));
    }
}

QTEST_MAIN(TestVersion)
#include "test_version.moc"
