/**
 * @file version_management_example.cpp
 * @brief Example demonstrating plugin version management features
 * @version 3.1.0
 * @author QtPlugin Development Team
 */

#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include <memory>

#include "qtplugin/core/plugin_manager.hpp"
#include "qtplugin/managers/plugin_version_manager.hpp"
#include "qtplugin/utils/version.hpp"

using namespace qtplugin;

class VersionManagementExample {
public:
    VersionManagementExample() {
        // Create plugin manager with version management support
        plugin_manager_ = std::make_unique<PluginManager>();

        std::cout << "=== QtForge Plugin Version Management Example ==="
                  << std::endl;
    }

    void run() {
        demonstrateBasicVersionManagement();
        demonstrateVersionMigration();
        demonstrateRollbackFeatures();
        demonstrateCompatibilityManagement();
        demonstrateVersionStatistics();
    }

private:
    void demonstrateBasicVersionManagement() {
        std::cout << "\n1. Basic Version Management" << std::endl;
        std::cout << "=============================" << std::endl;

        const std::string plugin_id = "example.calculator";

        // Install multiple versions of a plugin
        std::vector<Version> versions = {Version(1, 0, 0), Version(1, 1, 0),
                                         Version(1, 2, 0), Version(2, 0, 0)};

        for (const auto& version : versions) {
            // In a real scenario, you would have actual plugin files
            std::filesystem::path plugin_file =
                createMockPluginFile(plugin_id, version);

            auto result = plugin_manager_->install_plugin_version(
                plugin_id, version, plugin_file);

            if (result) {
                std::cout << "✓ Installed " << plugin_id << " v"
                          << version.to_string() << std::endl;
            } else {
                std::cout << "✗ Failed to install " << plugin_id << " v"
                          << version.to_string() << ": "
                          << result.error().message << std::endl;
            }
        }

        // List installed versions
        auto installed_versions =
            plugin_manager_->get_plugin_versions(plugin_id);
        std::cout << "\nInstalled versions:" << std::endl;
        for (const auto& version_info : installed_versions) {
            std::cout << "  - v" << version_info.version.to_string()
                      << " (Status: " << static_cast<int>(version_info.status)
                      << ")" << std::endl;
        }

        // Set active version
        auto activate_result = plugin_manager_->set_plugin_active_version(
            plugin_id, Version(1, 2, 0), false);

        if (activate_result) {
            std::cout << "✓ Activated version 1.2.0" << std::endl;
        }

        // Get active version
        auto active_version =
            plugin_manager_->get_plugin_active_version(plugin_id);
        if (active_version) {
            std::cout << "Active version: v"
                      << active_version->version.to_string() << std::endl;
        }
    }

    void demonstrateVersionMigration() {
        std::cout << "\n2. Version Migration" << std::endl;
        std::cout << "====================" << std::endl;

        const std::string plugin_id = "example.texteditor";
        auto& version_manager = plugin_manager_->version_manager();

        // Register a custom migration
        auto migration_result = version_manager.register_migration(
            plugin_id, Version(1, 0, 0), Version(2, 0, 0),
            [](const MigrationContext& context)
                -> qtplugin::expected<void, PluginError> {
                std::cout << "  Performing custom migration from "
                          << context.from_version.to_string() << " to "
                          << context.to_version.to_string() << std::endl;

                // Simulate migration work
                std::cout << "  - Converting configuration format..."
                          << std::endl;
                std::cout << "  - Migrating user preferences..." << std::endl;
                std::cout << "  - Updating data structures..." << std::endl;

                return {};
            });

        if (migration_result) {
            std::cout << "✓ Registered custom migration for " << plugin_id
                      << std::endl;
        }

        // Check if migration is available
        bool migration_available = version_manager.is_migration_available(
            plugin_id, Version(1, 0, 0), Version(2, 0, 0));

        std::cout << "Migration available: "
                  << (migration_available ? "Yes" : "No") << std::endl;

        // Perform migration
        MigrationContext context(plugin_id, Version(1, 0, 0), Version(2, 0, 0));
        context.strategy = MigrationStrategy::Callback;

        auto migrate_result = version_manager.migrate_plugin_data(context);
        if (migrate_result) {
            std::cout << "✓ Migration completed successfully" << std::endl;
        } else {
            std::cout << "✗ Migration failed: "
                      << migrate_result.error().message << std::endl;
        }
    }

    void demonstrateRollbackFeatures() {
        std::cout << "\n3. Rollback Features" << std::endl;
        std::cout << "===================" << std::endl;

        const std::string plugin_id = "example.calculator";
        auto& version_manager = plugin_manager_->version_manager();

        // Create backup
        auto backup_result =
            version_manager.create_backup(plugin_id, Version(1, 2, 0));
        if (backup_result) {
            std::cout << "✓ Created backup for v1.2.0" << std::endl;
            std::cout << "  Backup location: "
                      << backup_result->backup_path.string() << std::endl;
        }

        // Get rollback points
        auto rollback_points = version_manager.get_rollback_points(plugin_id);
        std::cout << "Available rollback points: " << rollback_points.size()
                  << std::endl;

        for (const auto& point : rollback_points) {
            std::cout << "  - v" << point.current_version.to_string()
                      << " (backup: "
                      << point.backup_time.time_since_epoch().count() << ")"
                      << std::endl;
        }

        // Simulate rollback
        if (!rollback_points.empty()) {
            auto rollback_result = version_manager.rollback_to_version(
                plugin_id, Version(1, 1, 0), true);

            if (rollback_result) {
                std::cout << "✓ Rolled back to v1.1.0" << std::endl;
            }
        }

        // Clean up old backups
        int cleaned_count = version_manager.cleanup_old_backups(plugin_id, 3);
        std::cout << "Cleaned up " << cleaned_count << " old backups"
                  << std::endl;
    }

    void demonstrateCompatibilityManagement() {
        std::cout << "\n4. Compatibility Management" << std::endl;
        std::cout << "===========================" << std::endl;

        const std::string plugin_id = "example.calculator";
        auto& version_manager = plugin_manager_->version_manager();

        // Check compatibility
        Version host_version(1, 0, 0);
        Version plugin_version(1, 2, 0);

        auto compatibility = version_manager.check_compatibility(
            plugin_id, plugin_version, host_version);

        std::cout << "Compatibility level: ";
        switch (compatibility) {
            case CompatibilityLevel::Breaking:
                std::cout << "Breaking (manual migration required)"
                          << std::endl;
                break;
            case CompatibilityLevel::Major:
                std::cout << "Major (automatic migration possible)"
                          << std::endl;
                break;
            case CompatibilityLevel::Minor:
                std::cout << "Minor (backward compatible)" << std::endl;
                break;
            case CompatibilityLevel::Patch:
                std::cout << "Patch (fully compatible)" << std::endl;
                break;
            case CompatibilityLevel::Build:
                std::cout << "Build (no migration needed)" << std::endl;
                break;
        }

        // Get compatible versions
        auto compatible_versions =
            version_manager.get_compatible_versions(plugin_id, host_version);
        std::cout << "Compatible versions with host v"
                  << host_version.to_string() << ":" << std::endl;
        for (const auto& version : compatible_versions) {
            std::cout << "  - v" << version.to_string() << std::endl;
        }
    }

    void demonstrateVersionStatistics() {
        std::cout << "\n5. Version Statistics" << std::endl;
        std::cout << "=====================" << std::endl;

        auto& version_manager = plugin_manager_->version_manager();

        // Get version statistics
        auto stats = version_manager.get_version_statistics();

        std::cout << "Version Management Statistics:" << std::endl;
        std::cout << "  Total plugins: " << stats["total_plugins"].toInt()
                  << std::endl;
        std::cout << "  Total versions: " << stats["total_versions"].toInt()
                  << std::endl;
        std::cout << "  Active versions: " << stats["active_versions"].toInt()
                  << std::endl;

        if (stats.contains("storage_size_bytes")) {
            auto storage_size =
                stats["storage_size_bytes"].toVariant().toLongLong();
            std::cout << "  Storage usage: " << storage_size << " bytes"
                      << std::endl;
        }

        // Get storage usage for specific plugin
        auto storage_usage =
            version_manager.get_storage_usage("example.calculator");
        if (storage_usage.contains("total_size")) {
            std::cout << "  Calculator plugin storage: "
                      << storage_usage["total_size"].toVariant().toLongLong()
                      << " bytes" << std::endl;
        }
    }

    std::filesystem::path createMockPluginFile(const std::string& plugin_id,
                                               const Version& version) {
        // Create a temporary mock plugin file for demonstration
        auto temp_dir =
            std::filesystem::temp_directory_path() / "qtforge_example";
        std::filesystem::create_directories(temp_dir);

        auto plugin_file =
            temp_dir / (plugin_id + "_v" + version.to_string() + ".dll");

        // Create a dummy file
        std::ofstream file(plugin_file);
        file << "Mock plugin: " << plugin_id << " v" << version.to_string();
        file.close();

        return plugin_file;
    }

    std::unique_ptr<PluginManager> plugin_manager_;
};

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    try {
        VersionManagementExample example;
        example.run();

        std::cout << "\n=== Example completed successfully ===" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
