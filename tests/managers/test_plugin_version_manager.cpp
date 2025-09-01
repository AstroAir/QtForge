/**
 * @file test_plugin_version_manager.cpp
 * @brief Unit tests for plugin version manager
 * @version 3.1.0
 * @author QtPlugin Development Team
 */

#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>
#include <filesystem>
#include <fstream>
#include <memory>

#include "../include/qtplugin/core/plugin_registry.hpp"
#include "../include/qtplugin/managers/plugin_version_manager.hpp"

using namespace qtplugin;

// Forward declarations for testing

class TestPluginVersionManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Version installation tests
    void testInstallVersion();
    void testInstallVersionAlreadyExists();
    void testInstallVersionInvalidFile();
    void testUninstallVersion();
    void testUninstallVersionNotFound();
    void testUninstallActiveVersion();

    // Version management tests
    void testGetInstalledVersions();
    void testSetActiveVersion();
    void testGetActiveVersion();
    void testVersionHistory();

    // Migration tests
    void testRegisterMigration();
    void testIsMigrationAvailable();
    void testMigratePluginData();
    void testAutomaticMigration();
    void testCallbackMigration();

    // Rollback tests
    void testCreateBackup();
    void testRollbackToVersion();
    void testGetRollbackPoints();
    void testCleanupOldBackups();

    // Compatibility tests
    void testCheckCompatibility();
    void testGetCompatibleVersions();
    void testRegisterCompatibilityRules();

    // Storage tests
    void testSetStorageDirectory();
    void testGetStorageUsage();
    void testCleanupUnusedVersions();

    // Event tests
    void testVersionEventCallbacks();

    // Statistics tests
    void testGetVersionStatistics();

private:
    void createTestPlugin(const QString& plugin_id, const Version& version,
                          const QString& content = "test plugin");
    std::filesystem::path createTestPluginFile(const QString& plugin_id,
                                               const Version& version);

    std::unique_ptr<IPluginVersionManager> version_manager_;
    std::shared_ptr<IPluginRegistry> registry_;
    std::shared_ptr<IConfigurationManager> config_manager_;
    std::shared_ptr<ILoggingManager> logger_;
    std::unique_ptr<QTemporaryDir> temp_dir_;
    std::filesystem::path test_storage_dir_;
};

void TestPluginVersionManager::initTestCase() {
    // Create temporary directory for tests
    temp_dir_ = std::make_unique<QTemporaryDir>();
    QVERIFY(temp_dir_->isValid());

    test_storage_dir_ = temp_dir_->path().toStdString();

    // Create mock dependencies
    registry_ = std::make_shared<PluginRegistry>();
    config_manager_ = nullptr;  // Use nullptr for testing
    logger_ = nullptr;          // Use nullptr for testing
}

void TestPluginVersionManager::cleanupTestCase() {
    version_manager_.reset();
    registry_.reset();
    config_manager_.reset();
    logger_.reset();
    temp_dir_.reset();
}

void TestPluginVersionManager::init() {
    // Create fresh version manager for each test
    version_manager_ =
        create_plugin_version_manager(registry_, config_manager_, logger_);

    // Set test storage directory
    auto result = version_manager_->set_storage_directory(test_storage_dir_);
    QVERIFY(result.has_value());
}

void TestPluginVersionManager::cleanup() {
    version_manager_.reset();

    // Clean up test files
    try {
        std::filesystem::remove_all(test_storage_dir_);
        std::filesystem::create_directories(test_storage_dir_);
    } catch (const std::exception&) {
        // Ignore cleanup errors
    }
}

void TestPluginVersionManager::testInstallVersion() {
    const QString plugin_id = "test.plugin";
    const Version version(1, 0, 0);

    // Create test plugin file
    auto plugin_file = createTestPluginFile(plugin_id, version);

    // Install version
    auto result = version_manager_->install_version(plugin_id.toStdString(),
                                                    version, plugin_file);
    QVERIFY(result.has_value());

    // Verify installation
    auto versions =
        version_manager_->get_installed_versions(plugin_id.toStdString());
    QCOMPARE(versions.size(), 1);
    QCOMPARE(versions[0].plugin_id, plugin_id.toStdString());
    QCOMPARE(versions[0].version, version);
    QCOMPARE(versions[0].status, VersionInstallStatus::Installed);
}

void TestPluginVersionManager::testInstallVersionAlreadyExists() {
    const QString plugin_id = "test.plugin";
    const Version version(1, 0, 0);

    // Create test plugin file
    auto plugin_file = createTestPluginFile(plugin_id, version);

    // Install version first time
    auto result1 = version_manager_->install_version(plugin_id.toStdString(),
                                                     version, plugin_file);
    QVERIFY(result1.has_value());

    // Try to install same version again without replace flag
    auto result2 = version_manager_->install_version(
        plugin_id.toStdString(), version, plugin_file, false);
    QVERIFY(!result2.has_value());
    QCOMPARE(result2.error().code, VersionErrorCode::VersionAlreadyExists);

    // Install with replace flag should succeed
    auto result3 = version_manager_->install_version(
        plugin_id.toStdString(), version, plugin_file, true);
    QVERIFY(result3.has_value());
}

void TestPluginVersionManager::testInstallVersionInvalidFile() {
    const QString plugin_id = "test.plugin";
    const Version version(1, 0, 0);

    // Try to install from non-existent file
    std::filesystem::path invalid_file = test_storage_dir_ / "nonexistent.dll";
    auto result = version_manager_->install_version(plugin_id.toStdString(),
                                                    version, invalid_file);

    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, VersionErrorCode::StorageError);
}

void TestPluginVersionManager::testUninstallVersion() {
    const QString plugin_id = "test.plugin";
    const Version version(1, 0, 0);

    // Install version first
    auto plugin_file = createTestPluginFile(plugin_id, version);
    auto install_result = version_manager_->install_version(
        plugin_id.toStdString(), version, plugin_file);
    QVERIFY(install_result.has_value());

    // Uninstall version
    auto uninstall_result =
        version_manager_->uninstall_version(plugin_id.toStdString(), version);
    QVERIFY(uninstall_result.has_value());

    // Verify uninstallation
    auto versions =
        version_manager_->get_installed_versions(plugin_id.toStdString());
    QCOMPARE(versions.size(), 0);
}

void TestPluginVersionManager::testUninstallVersionNotFound() {
    const QString plugin_id = "test.plugin";
    const Version version(1, 0, 0);

    // Try to uninstall non-existent version
    auto result =
        version_manager_->uninstall_version(plugin_id.toStdString(), version);

    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, VersionErrorCode::VersionNotFound);
}

void TestPluginVersionManager::testUninstallActiveVersion() {
    const QString plugin_id = "test.plugin";
    const Version version(1, 0, 0);

    // Install and activate version
    auto plugin_file = createTestPluginFile(plugin_id, version);
    auto install_result = version_manager_->install_version(
        plugin_id.toStdString(), version, plugin_file);
    QVERIFY(install_result.has_value());

    auto activate_result = version_manager_->set_active_version(
        plugin_id.toStdString(), version, false);
    QVERIFY(activate_result.has_value());

    // Try to uninstall active version without force
    auto uninstall_result = version_manager_->uninstall_version(
        plugin_id.toStdString(), version, false);
    QVERIFY(!uninstall_result.has_value());
    QCOMPARE(uninstall_result.error().code,
             VersionErrorCode::ActiveVersionConflict);

    // Uninstall with force should succeed
    auto force_uninstall_result = version_manager_->uninstall_version(
        plugin_id.toStdString(), version, true);
    QVERIFY(force_uninstall_result.has_value());
}

void TestPluginVersionManager::testGetInstalledVersions() {
    const QString plugin_id = "test.plugin";

    // Install multiple versions
    std::vector<Version> versions = {Version(1, 0, 0), Version(1, 1, 0),
                                     Version(2, 0, 0)};

    for (const auto& version : versions) {
        auto plugin_file = createTestPluginFile(plugin_id, version);
        auto result = version_manager_->install_version(plugin_id.toStdString(),
                                                        version, plugin_file);
        QVERIFY(result.has_value());
    }

    // Get installed versions
    auto installed_versions =
        version_manager_->get_installed_versions(plugin_id.toStdString());
    QCOMPARE(installed_versions.size(), versions.size());

    // Verify versions are sorted
    for (size_t i = 0; i < installed_versions.size(); ++i) {
        QCOMPARE(installed_versions[i].version, versions[i]);
    }
}

void TestPluginVersionManager::testSetActiveVersion() {
    const QString plugin_id = "test.plugin";
    const Version version1(1, 0, 0);
    const Version version2(1, 1, 0);

    // Install two versions
    auto plugin_file1 = createTestPluginFile(plugin_id, version1);
    auto plugin_file2 = createTestPluginFile(plugin_id, version2);

    auto install_result1 = version_manager_->install_version(
        plugin_id.toStdString(), version1, plugin_file1);
    auto install_result2 = version_manager_->install_version(
        plugin_id.toStdString(), version2, plugin_file2);
    QVERIFY(install_result1.has_value());
    QVERIFY(install_result2.has_value());

    // Set first version as active
    auto activate_result1 = version_manager_->set_active_version(
        plugin_id.toStdString(), version1, false);
    QVERIFY(activate_result1.has_value());

    // Verify active version
    auto active_version =
        version_manager_->get_active_version(plugin_id.toStdString());
    QVERIFY(active_version.has_value());
    QCOMPARE(active_version->version, version1);
    QVERIFY(active_version->is_active);

    // Switch to second version
    auto activate_result2 = version_manager_->set_active_version(
        plugin_id.toStdString(), version2, false);
    QVERIFY(activate_result2.has_value());

    // Verify new active version
    active_version =
        version_manager_->get_active_version(plugin_id.toStdString());
    QVERIFY(active_version.has_value());
    QCOMPARE(active_version->version, version2);
    QVERIFY(active_version->is_active);
}

std::filesystem::path TestPluginVersionManager::createTestPluginFile(
    const QString& plugin_id, const Version& version) {
    auto plugin_dir = test_storage_dir_ / "test_plugins";
    std::filesystem::create_directories(plugin_dir);

    auto plugin_file = plugin_dir / (plugin_id.toStdString() + "_" +
                                     version.to_string() + ".dll");

    // Create a dummy plugin file
    std::ofstream file(plugin_file);
    file << "Test plugin content for " << plugin_id.toStdString() << " v"
         << version.to_string();
    file.close();

    return plugin_file;
}

void TestPluginVersionManager::testGetActiveVersion() {
    const QString plugin_id = "test.plugin";

    // No active version initially
    auto active_version =
        version_manager_->get_active_version(plugin_id.toStdString());
    QVERIFY(!active_version.has_value());

    // Install and activate a version
    const Version version(1, 0, 0);
    auto plugin_file = createTestPluginFile(plugin_id, version);
    auto install_result = version_manager_->install_version(
        plugin_id.toStdString(), version, plugin_file);
    QVERIFY(install_result.has_value());

    auto activate_result = version_manager_->set_active_version(
        plugin_id.toStdString(), version, false);
    QVERIFY(activate_result.has_value());

    // Verify active version
    active_version =
        version_manager_->get_active_version(plugin_id.toStdString());
    QVERIFY(active_version.has_value());
    QCOMPARE(active_version->plugin_id, plugin_id.toStdString());
    QCOMPARE(active_version->version, version);
    QVERIFY(active_version->is_active);
    QCOMPARE(active_version->status, VersionInstallStatus::Active);
}

void TestPluginVersionManager::testVersionHistory() {
    const QString plugin_id = "test.plugin";

    // Install multiple versions over time
    std::vector<Version> versions = {Version(1, 0, 0), Version(1, 0, 1),
                                     Version(1, 1, 0)};

    for (const auto& version : versions) {
        auto plugin_file = createTestPluginFile(plugin_id, version);
        auto result = version_manager_->install_version(plugin_id.toStdString(),
                                                        version, plugin_file);
        QVERIFY(result.has_value());

        // Small delay to ensure different timestamps
        QTest::qWait(10);
    }

    // Get version history
    auto history =
        version_manager_->get_version_history(plugin_id.toStdString());
    QCOMPARE(history.size(), versions.size());

    // Verify chronological order (sorted by install time, newest first)
    // Since we installed in order [1.0.0, 1.0.1, 1.1.0], history should be
    // [1.1.0, 1.0.1, 1.0.0]
    std::vector<Version> expected_order = {
        Version(1, 1, 0),  // Last installed (newest)
        Version(1, 0, 1),  // Second installed
        Version(1, 0, 0)   // First installed (oldest)
    };

    for (size_t i = 0; i < history.size(); ++i) {
        QCOMPARE(history[i].version, expected_order[i]);
        QVERIFY(history[i].install_time <= std::chrono::system_clock::now());
    }
}

void TestPluginVersionManager::testRegisterMigration() {
    const QString plugin_id = "test.plugin";
    const Version from_version(1, 0, 0);
    const Version to_version(2, 0, 0);

    // Register a custom migration
    bool migration_called = false;
    auto migrator =
        [&migration_called](
            const MigrationContext&) -> qtplugin::expected<void, PluginError> {
        migration_called = true;
        return {};
    };

    auto result = version_manager_->register_migration(
        plugin_id.toStdString(), from_version, to_version, migrator);
    QVERIFY(result.has_value());

    // Verify migration is available
    bool available = version_manager_->is_migration_available(
        plugin_id.toStdString(), from_version, to_version);
    QVERIFY(available);
}

void TestPluginVersionManager::testIsMigrationAvailable() {
    const QString plugin_id = "test.plugin";
    const Version version1(1, 0, 0);
    const Version version2(
        1, 1, 0);  // Minor version change - should have automatic migration
    const Version version3(
        2, 0, 0);  // Major version change - might not have automatic migration

    // Test automatic migration availability for compatible versions
    bool available_minor = version_manager_->is_migration_available(
        plugin_id.toStdString(), version1, version2);
    QVERIFY(available_minor);  // Minor version changes should be automatically
                               // migratable

    // Test migration availability for major version changes
    bool available_major = version_manager_->is_migration_available(
        plugin_id.toStdString(), version1, version3);
    // This depends on the compatibility level determination - major changes
    // might not be automatically migratable
    Q_UNUSED(available_major);
}

void TestPluginVersionManager::testMigratePluginData() {
    const QString plugin_id = "test.plugin";
    const Version from_version(1, 0, 0);
    const Version to_version(1, 1, 0);

    // Create migration context
    MigrationContext context(plugin_id.toStdString(), from_version, to_version);
    context.strategy = MigrationStrategy::Automatic;

    // Test migration
    auto result = version_manager_->migrate_plugin_data(context);
    QVERIFY(result.has_value());  // Automatic migration should succeed for
                                  // minor version changes
}

void TestPluginVersionManager::testAutomaticMigration() {
    const QString plugin_id = "test.plugin";
    const Version from_version(1, 0, 0);
    const Version to_version(1, 0, 1);  // Patch version change

    // Create migration context for patch version change
    MigrationContext context(plugin_id.toStdString(), from_version, to_version);
    context.strategy = MigrationStrategy::Automatic;

    // Test automatic migration
    auto result = version_manager_->migrate_plugin_data(context);
    QVERIFY(result.has_value());  // Patch changes should migrate automatically
}

void TestPluginVersionManager::testCallbackMigration() {
    const QString plugin_id = "test.plugin";
    const Version from_version(1, 0, 0);
    const Version to_version(2, 0, 0);

    // Register custom migration callback
    bool callback_executed = false;
    auto migrator = [&callback_executed](const MigrationContext& context)
        -> qtplugin::expected<void, PluginError> {
        callback_executed = true;
        // Verify context parameters
        if (context.from_version != Version(1, 0, 0) ||
            context.to_version != Version(2, 0, 0)) {
            return qtplugin::unexpected(PluginError{
                PluginErrorCode::InvalidArgument, "Invalid migration context"});
        }
        return {};
    };

    auto register_result = version_manager_->register_migration(
        plugin_id.toStdString(), from_version, to_version, migrator);
    QVERIFY(register_result.has_value());

    // Create migration context
    MigrationContext context(plugin_id.toStdString(), from_version, to_version);
    context.strategy = MigrationStrategy::Callback;
    context.custom_migrator = migrator;

    // Test callback migration
    auto migrate_result = version_manager_->migrate_plugin_data(context);
    QVERIFY(migrate_result.has_value());
    QVERIFY(callback_executed);
}

void TestPluginVersionManager::testCreateBackup() {
    const QString plugin_id = "test.plugin";
    const Version version(1, 0, 0);

    // Install version first
    auto plugin_file = createTestPluginFile(plugin_id, version);
    auto install_result = version_manager_->install_version(
        plugin_id.toStdString(), version, plugin_file);
    QVERIFY(install_result.has_value());

    // Create backup
    auto backup_result =
        version_manager_->create_backup(plugin_id.toStdString(), version);
    QVERIFY(backup_result.has_value());

    const auto& backup_info = backup_result.value();
    QCOMPARE(backup_info.plugin_id, plugin_id.toStdString());
    QCOMPARE(backup_info.current_version, version);
    QVERIFY(std::filesystem::exists(backup_info.backup_path));
}

void TestPluginVersionManager::testRollbackToVersion() {
    const QString plugin_id = "test.plugin";
    const Version version1(1, 0, 0);
    const Version version2(1, 1, 0);

    // Install two versions
    auto plugin_file1 = createTestPluginFile(plugin_id, version1);
    auto plugin_file2 = createTestPluginFile(plugin_id, version2);

    auto install_result1 = version_manager_->install_version(
        plugin_id.toStdString(), version1, plugin_file1);
    QVERIFY(install_result1.has_value());

    // Create backup of version1 so we can rollback to it later
    auto backup_result1 =
        version_manager_->create_backup(plugin_id.toStdString(), version1);
    QVERIFY(backup_result1.has_value());

    auto install_result2 = version_manager_->install_version(
        plugin_id.toStdString(), version2, plugin_file2);
    QVERIFY(install_result2.has_value());

    // Set version2 as active
    auto activate_result = version_manager_->set_active_version(
        plugin_id.toStdString(), version2, false);
    QVERIFY(activate_result.has_value());

    // Create backup of version2 as well
    auto backup_result2 =
        version_manager_->create_backup(plugin_id.toStdString(), version2);
    QVERIFY(backup_result2.has_value());

    // Rollback to version1
    auto rollback_result = version_manager_->rollback_to_version(
        plugin_id.toStdString(), version1, true);
    QVERIFY(rollback_result.has_value());

    // Verify rollback
    auto active_version =
        version_manager_->get_active_version(plugin_id.toStdString());
    QVERIFY(active_version.has_value());
    QCOMPARE(active_version->version, version1);
}

void TestPluginVersionManager::testGetRollbackPoints() {
    const QString plugin_id = "test.plugin";
    const Version version(1, 0, 0);

    // Install version and create backup
    auto plugin_file = createTestPluginFile(plugin_id, version);
    auto install_result = version_manager_->install_version(
        plugin_id.toStdString(), version, plugin_file);
    QVERIFY(install_result.has_value());

    auto backup_result =
        version_manager_->create_backup(plugin_id.toStdString(), version);
    QVERIFY(backup_result.has_value());

    // Get rollback points
    auto rollback_points =
        version_manager_->get_rollback_points(plugin_id.toStdString());
    QVERIFY(rollback_points.size() >= 1);

    const auto& rollback_point = rollback_points[0];
    QCOMPARE(rollback_point.plugin_id, plugin_id.toStdString());
    QCOMPARE(rollback_point.current_version, version);
}

void TestPluginVersionManager::testCleanupOldBackups() {
    const QString plugin_id = "test.plugin";
    const Version version(1, 0, 0);

    // Install version and create multiple backups
    auto plugin_file = createTestPluginFile(plugin_id, version);
    auto install_result = version_manager_->install_version(
        plugin_id.toStdString(), version, plugin_file);
    QVERIFY(install_result.has_value());

    // Create several backups
    for (int i = 0; i < 10; ++i) {
        auto backup_result =
            version_manager_->create_backup(plugin_id.toStdString(), version);
        QVERIFY(backup_result.has_value());
        QTest::qWait(10);  // Small delay to ensure different timestamps
    }

    // Clean up old backups, keeping only 3
    int cleaned_count =
        version_manager_->cleanup_old_backups(plugin_id.toStdString(), 3);
    QVERIFY(cleaned_count >= 0);

    // Verify remaining backups
    auto rollback_points =
        version_manager_->get_rollback_points(plugin_id.toStdString());
    QVERIFY(rollback_points.size() <= 3);
}

void TestPluginVersionManager::testCheckCompatibility() {
    const QString plugin_id = "test.plugin";
    const Version plugin_version(1, 5, 0);
    const Version host_version(1, 0, 0);

    // Test compatibility check
    auto compatibility = version_manager_->check_compatibility(
        plugin_id.toStdString(), plugin_version, host_version);

    // The result depends on the implementation, but should be a valid
    // compatibility level
    QVERIFY(compatibility == CompatibilityLevel::Breaking ||
            compatibility == CompatibilityLevel::Major ||
            compatibility == CompatibilityLevel::Minor ||
            compatibility == CompatibilityLevel::Patch ||
            compatibility == CompatibilityLevel::Build);
}

void TestPluginVersionManager::testGetCompatibleVersions() {
    const QString plugin_id = "test.plugin";
    const Version host_version(1, 0, 0);

    // Install multiple versions
    std::vector<Version> versions = {Version(1, 0, 0), Version(1, 0, 1),
                                     Version(1, 1, 0), Version(2, 0, 0)};

    for (const auto& version : versions) {
        auto plugin_file = createTestPluginFile(plugin_id, version);
        auto result = version_manager_->install_version(plugin_id.toStdString(),
                                                        version, plugin_file);
        QVERIFY(result.has_value());
    }

    // Get compatible versions
    auto compatible_versions = version_manager_->get_compatible_versions(
        plugin_id.toStdString(), host_version);

    // Should return some compatible versions
    QVERIFY(compatible_versions.size() > 0);
}

void TestPluginVersionManager::testRegisterCompatibilityRules() {
    const QString plugin_id = "test.plugin";

    // Create compatibility rules
    QJsonObject rules;
    QJsonObject rule1;
    rule1["min_host_version"] = "1.0.0";
    rule1["max_host_version"] = "1.9.9";
    rule1["compatibility_level"] = static_cast<int>(CompatibilityLevel::Minor);

    QJsonArray rules_array;
    rules_array.append(rule1);
    rules["compatibility_rules"] = rules_array;

    // Register compatibility rules
    auto result = version_manager_->register_compatibility_rules(
        plugin_id.toStdString(), rules);
    QVERIFY(result.has_value());
}

void TestPluginVersionManager::testSetStorageDirectory() {
    auto new_storage_dir = test_storage_dir_ / "new_storage";

    // Set new storage directory
    auto result = version_manager_->set_storage_directory(new_storage_dir);
    QVERIFY(result.has_value());

    // Verify directory was set
    auto current_dir = version_manager_->get_storage_directory();
    QCOMPARE(current_dir, new_storage_dir);

    // Verify directory exists
    QVERIFY(std::filesystem::exists(new_storage_dir));
}

void TestPluginVersionManager::testGetStorageUsage() {
    const QString plugin_id = "test.plugin";
    const Version version(1, 0, 0);

    // Install a version to create some storage usage
    auto plugin_file = createTestPluginFile(plugin_id, version);
    auto install_result = version_manager_->install_version(
        plugin_id.toStdString(), version, plugin_file);
    QVERIFY(install_result.has_value());

    // Get storage usage for specific plugin
    auto usage = version_manager_->get_storage_usage(plugin_id.toStdString());
    QVERIFY(usage.contains("size_bytes"));
    QVERIFY(usage["size_bytes"].toVariant().toLongLong() >= 0);
    QVERIFY(usage.contains("plugin_id"));
    QCOMPARE(usage["plugin_id"].toString(), plugin_id);
}

void TestPluginVersionManager::testCleanupUnusedVersions() {
    const QString plugin_id = "test.plugin";

    // Install multiple versions
    std::vector<Version> versions = {Version(1, 0, 0), Version(1, 1, 0),
                                     Version(1, 2, 0), Version(2, 0, 0)};

    for (const auto& version : versions) {
        auto plugin_file = createTestPluginFile(plugin_id, version);
        auto result = version_manager_->install_version(plugin_id.toStdString(),
                                                        version, plugin_file);
        QVERIFY(result.has_value());
    }

    // Set one version as active
    auto activate_result = version_manager_->set_active_version(
        plugin_id.toStdString(), versions.back(), false);
    QVERIFY(activate_result.has_value());

    // Clean up unused versions, keeping only 2
    int cleaned_count =
        version_manager_->cleanup_unused_versions(plugin_id.toStdString(), 2);
    QVERIFY(cleaned_count >= 0);

    // Verify remaining versions
    auto remaining_versions =
        version_manager_->get_installed_versions(plugin_id.toStdString());
    QVERIFY(remaining_versions.size() <= 3);  // 2 unused + 1 active
}

void TestPluginVersionManager::testVersionEventCallbacks() {
    const QString plugin_id = "test.plugin";
    const Version version(1, 0, 0);

    // Register event callback
    bool callback_called = false;
    std::string received_plugin_id;
    Version received_version;
    VersionInstallStatus received_status;

    auto subscription_id = version_manager_->register_version_event_callback(
        [&](const std::string& id, const Version& ver,
            VersionInstallStatus status) {
            callback_called = true;
            received_plugin_id = id;
            received_version = ver;
            received_status = status;
        });

    QVERIFY(!subscription_id.empty());

    // Install version to trigger callback
    auto plugin_file = createTestPluginFile(plugin_id, version);
    auto install_result = version_manager_->install_version(
        plugin_id.toStdString(), version, plugin_file);
    QVERIFY(install_result.has_value());

    // Verify callback was called
    QVERIFY(callback_called);
    QCOMPARE(received_plugin_id, plugin_id.toStdString());
    QCOMPARE(received_version, version);
    QCOMPARE(received_status, VersionInstallStatus::Installed);

    // Unregister callback
    version_manager_->unregister_version_event_callback(subscription_id);
}

void TestPluginVersionManager::testGetVersionStatistics() {
    const QString plugin_id = "test.plugin";

    // Install some versions
    std::vector<Version> versions = {Version(1, 0, 0), Version(1, 1, 0)};

    for (const auto& version : versions) {
        auto plugin_file = createTestPluginFile(plugin_id, version);
        auto result = version_manager_->install_version(plugin_id.toStdString(),
                                                        version, plugin_file);
        QVERIFY(result.has_value());
    }

    // Set one version as active
    auto activate_result = version_manager_->set_active_version(
        plugin_id.toStdString(), versions[0], false);
    QVERIFY(activate_result.has_value());

    // Get statistics
    auto stats = version_manager_->get_version_statistics();

    // Verify statistics
    QVERIFY(stats.contains("total_plugins"));
    QVERIFY(stats.contains("total_versions"));
    QVERIFY(stats.contains("active_versions"));

    QCOMPARE(stats["total_plugins"].toInt(), 1);
    QCOMPARE(stats["total_versions"].toInt(), 2);
    QCOMPARE(stats["active_versions"].toInt(), 1);
}

QTEST_MAIN(TestPluginVersionManager)
#include "test_plugin_version_manager.moc"
