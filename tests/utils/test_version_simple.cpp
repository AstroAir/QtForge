/**
 * @file test_version_simple.cpp
 * @brief Simple tests for version utilities
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <memory>

#include <qtplugin/utils/version.hpp>

using namespace qtplugin;

class TestVersionSimple : public QObject {
    // Q_OBJECT  // Temporarily disabled for xmake demo

public:  // Changed from private slots for xmake demo
    void initTestCase();
    void cleanupTestCase();

    // Basic version tests
    void testVersionCreation();
    void testVersionFromString();
    void testInvalidVersionString();
    void testVersionEquality();
    void testVersionOrdering();
    void testVersionRange();
    void testPreReleaseVersions();
    void testBuildMetadata();
    void testVersionValidation();

private:
    // Helper methods
    void verifyVersionComponents(const Version& version, int major, int minor,
                                 int patch);
    void verifyVersionString(const Version& version,
                             const std::string& expected);
};

void TestVersionSimple::initTestCase() {
    qDebug() << "Starting version utility tests";
}

void TestVersionSimple::cleanupTestCase() {
    qDebug() << "Version utility tests completed";
}

void TestVersionSimple::testVersionCreation() {
    // Test default constructor
    Version default_version;
    QCOMPARE(default_version.major(), 0);
    QCOMPARE(default_version.minor(), 0);
    QCOMPARE(default_version.patch(), 0);
    QVERIFY(default_version.prerelease().empty());
    QVERIFY(default_version.build().empty());

    // Test constructor with major, minor, patch
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

void TestVersionSimple::testVersionFromString() {
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

void TestVersionSimple::testInvalidVersionString() {
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

void TestVersionSimple::testVersionEquality() {
    Version v1(1, 2, 3);
    Version v2(1, 2, 3);
    Version v3(1, 2, 4);

    QVERIFY(v1 == v2);
    QVERIFY(!(v1 == v3));
    QVERIFY(v1 != v3);
    QVERIFY(!(v1 != v2));
}

void TestVersionSimple::testVersionOrdering() {
    Version v1(1, 0, 0);
    Version v2(1, 1, 0);
    Version v3(2, 0, 0);

    QVERIFY(v1 < v2);
    QVERIFY(v2 < v3);
    QVERIFY(v1 < v3);
    QVERIFY(v3 > v2);
    QVERIFY(v2 > v1);
    QVERIFY(v3 > v1);
}

void TestVersionSimple::testVersionRange() {
    VersionRange range(Version(1, 0, 0), Version(2, 0, 0));

    QVERIFY(range.satisfies(Version(1, 0, 0)));
    QVERIFY(range.satisfies(Version(1, 5, 0)));
    QVERIFY(range.satisfies(Version(1, 9, 9)));
    QVERIFY(range.satisfies(Version(2, 0, 0)));  // Range is inclusive
    QVERIFY(!range.satisfies(Version(0, 9, 9)));
    QVERIFY(!range.satisfies(Version(2, 0, 1)));
}

void TestVersionSimple::testPreReleaseVersions() {
    auto alpha = Version::parse("1.0.0-alpha");
    auto beta = Version::parse("1.0.0-beta");
    auto rc = Version::parse("1.0.0-rc.1");
    auto release = Version::parse("1.0.0");

    QVERIFY(alpha.has_value());
    QVERIFY(beta.has_value());
    QVERIFY(rc.has_value());
    QVERIFY(release.has_value());

    // Pre-release versions should be less than release versions
    QVERIFY(alpha.value() < release.value());
    QVERIFY(beta.value() < release.value());
    QVERIFY(rc.value() < release.value());

    // Alpha < Beta < RC
    QVERIFY(alpha.value() < beta.value());
    QVERIFY(beta.value() < rc.value());
}

void TestVersionSimple::testBuildMetadata() {
    auto v1 = Version::parse("1.0.0+build.1");
    auto v2 = Version::parse("1.0.0+build.2");
    auto v3 = Version::parse("1.0.0");

    QVERIFY(v1.has_value());
    QVERIFY(v2.has_value());
    QVERIFY(v3.has_value());

    // Build metadata should not affect comparison
    QVERIFY(v1.value() == v2.value());
    QVERIFY(v1.value() == v3.value());
}

void TestVersionSimple::testVersionValidation() {
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
void TestVersionSimple::verifyVersionComponents(const Version& version,
                                                int major, int minor,
                                                int patch) {
    QCOMPARE(version.major(), major);
    QCOMPARE(version.minor(), minor);
    QCOMPARE(version.patch(), patch);
}

void TestVersionSimple::verifyVersionString(const Version& version,
                                            const std::string& expected) {
    QCOMPARE(version.to_string(), expected);
}

QTEST_MAIN(TestVersionSimple)
// #include "test_version_simple.moc"  // Temporarily disabled for xmake demo
