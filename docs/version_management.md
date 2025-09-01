# Plugin Version Management

QtForge 3.1.0 introduces comprehensive plugin version management capabilities, allowing applications to install, manage, and migrate between different versions of plugins seamlessly.

## Overview

The version management system provides:

- **Multi-version Support**: Install and maintain multiple versions of the same plugin
- **Version Migration**: Automatic and custom data migration between plugin versions
- **Rollback Capabilities**: Create backups and rollback to previous versions
- **Compatibility Management**: Check and enforce version compatibility
- **Storage Management**: Efficient storage and cleanup of plugin versions

## Key Components

### IPluginVersionManager

The main interface for version management operations:

```cpp
#include "qtplugin/managers/plugin_version_manager.hpp"

// Create version manager
auto version_manager = create_plugin_version_manager(
    registry, config_manager, logger);
```

### Version Class

Represents semantic versions (major.minor.patch):

```cpp
#include "qtplugin/utils/version.hpp"

Version v1(1, 0, 0);        // 1.0.0
Version v2(1, 2, 3);        // 1.2.3
Version v3(2, 0, 0, "beta"); // 2.0.0-beta
```

## Basic Usage

### Installing Plugin Versions

```cpp
// Install a specific version
auto result = version_manager->install_version(
    "my.plugin", Version(1, 2, 0), "/path/to/plugin.dll");

if (result) {
    std::cout << "Plugin installed successfully" << std::endl;
} else {
    std::cout << "Installation failed: " << result.error().message << std::endl;
}
```

### Managing Active Versions

```cpp
// Set active version
auto activate_result = version_manager->set_active_version(
    "my.plugin", Version(1, 2, 0), true); // true = migrate data

// Get active version
auto active_version = version_manager->get_active_version("my.plugin");
if (active_version) {
    std::cout << "Active version: " << active_version->version.to_string() << std::endl;
}

// List all installed versions
auto versions = version_manager->get_installed_versions("my.plugin");
for (const auto& version_info : versions) {
    std::cout << "Version: " << version_info.version.to_string()
              << " Status: " << static_cast<int>(version_info.status) << std::endl;
}
```

## Version Migration

### Automatic Migration

The system automatically handles migration for compatible version changes:

```cpp
// Minor and patch version changes are automatically migrated
auto result = version_manager->set_active_version(
    "my.plugin", Version(1, 2, 1), true); // Automatic migration
```

### Custom Migration

Register custom migration logic for complex version changes:

```cpp
// Register custom migration
auto migration_result = version_manager->register_migration(
    "my.plugin",
    Version(1, 0, 0),  // from version
    Version(2, 0, 0),  // to version
    [](const MigrationContext& context) -> qtplugin::expected<void, PluginError> {
        // Custom migration logic
        std::cout << "Migrating from " << context.from_version.to_string()
                  << " to " << context.to_version.to_string() << std::endl;

        // Perform migration tasks
        // - Convert configuration files
        // - Update data structures
        // - Migrate user preferences

        return {}; // Success
    });
```

### Migration Context

The `MigrationContext` provides information and control over the migration process:

```cpp
struct MigrationContext {
    std::string plugin_id;              // Plugin being migrated
    Version from_version;               // Source version
    Version to_version;                 // Target version
    MigrationStrategy strategy;         // Migration strategy
    std::filesystem::path data_directory; // Plugin data directory
    QJsonObject old_configuration;     // Old configuration
    QJsonObject new_configuration;     // New configuration
    bool preserve_user_data;           // Whether to preserve user data
    bool create_backup;                // Whether to create backup
};
```

## Rollback and Backup

### Creating Backups

```cpp
// Create backup before major changes
auto backup_result = version_manager->create_backup("my.plugin", Version(1, 2, 0));
if (backup_result) {
    std::cout << "Backup created at: " << backup_result->backup_path.string() << std::endl;
}
```

### Rolling Back

```cpp
// Rollback to previous version
auto rollback_result = version_manager->rollback_to_version(
    "my.plugin", Version(1, 1, 0), true); // true = preserve user data

if (rollback_result) {
    std::cout << "Rollback successful" << std::endl;
}
```

### Managing Rollback Points

```cpp
// Get available rollback points
auto rollback_points = version_manager->get_rollback_points("my.plugin");
for (const auto& point : rollback_points) {
    std::cout << "Rollback point: v" << point.current_version.to_string()
              << " (created: " << /* format timestamp */ ")" << std::endl;
}

// Clean up old backups (keep only 5 most recent)
int cleaned = version_manager->cleanup_old_backups("my.plugin", 5);
std::cout << "Cleaned up " << cleaned << " old backups" << std::endl;
```

## Compatibility Management

### Checking Compatibility

```cpp
// Check compatibility between plugin and host versions
auto compatibility = version_manager->check_compatibility(
    "my.plugin", Version(1, 5, 0), Version(1, 0, 0)); // plugin vs host

switch (compatibility) {
    case CompatibilityLevel::Breaking:
        std::cout << "Breaking changes - manual migration required" << std::endl;
        break;
    case CompatibilityLevel::Major:
        std::cout << "Major changes - automatic migration possible" << std::endl;
        break;
    case CompatibilityLevel::Minor:
        std::cout << "Minor changes - backward compatible" << std::endl;
        break;
    case CompatibilityLevel::Patch:
        std::cout << "Patch changes - fully compatible" << std::endl;
        break;
    case CompatibilityLevel::Build:
        std::cout << "Build changes - no migration needed" << std::endl;
        break;
}
```

### Getting Compatible Versions

```cpp
// Get all versions compatible with host version
auto compatible_versions = version_manager->get_compatible_versions(
    "my.plugin", Version(1, 0, 0)); // host version

for (const auto& version : compatible_versions) {
    std::cout << "Compatible version: " << version.to_string() << std::endl;
}
```

## Storage Management

### Storage Configuration

```cpp
// Set custom storage directory
auto result = version_manager->set_storage_directory("/custom/plugin/storage");

// Get current storage directory
auto storage_dir = version_manager->get_storage_directory();
std::cout << "Storage directory: " << storage_dir.string() << std::endl;
```

### Storage Cleanup

```cpp
// Clean up unused versions (keep only 3 most recent)
int cleaned_versions = version_manager->cleanup_unused_versions("my.plugin", 3);

// Get storage usage information
auto usage = version_manager->get_storage_usage("my.plugin");
std::cout << "Storage usage: " << usage["total_size"].toVariant().toLongLong()
          << " bytes" << std::endl;
```

## Event Handling

### Version Events

```cpp
// Register for version events
auto subscription_id = version_manager->register_version_event_callback(
    [](const std::string& plugin_id, const Version& version, VersionInstallStatus status) {
        std::cout << "Plugin " << plugin_id << " v" << version.to_string()
                  << " status changed to " << static_cast<int>(status) << std::endl;
    });

// Unregister when done
version_manager->unregister_version_event_callback(subscription_id);
```

## Statistics and Monitoring

### Version Statistics

```cpp
// Get overall version management statistics
auto stats = version_manager->get_version_statistics();

std::cout << "Total plugins: " << stats["total_plugins"].toInt() << std::endl;
std::cout << "Total versions: " << stats["total_versions"].toInt() << std::endl;
std::cout << "Active versions: " << stats["active_versions"].toInt() << std::endl;
std::cout << "Storage usage: " << stats["storage_size_bytes"].toVariant().toLongLong()
          << " bytes" << std::endl;
```

## Integration with PluginManager

The version management system is fully integrated with the main `PluginManager`:

```cpp
#include "qtplugin/core/plugin_manager.hpp"

// Create plugin manager (includes version management)
auto plugin_manager = std::make_unique<PluginManager>();

// Access version management through plugin manager
auto& version_manager = plugin_manager->version_manager();

// Or use plugin manager's version management methods directly
auto result = plugin_manager->install_plugin_version(
    "my.plugin", Version(1, 2, 0), "/path/to/plugin.dll");
```

## Best Practices

1. **Always create backups** before major version upgrades
2. **Test migrations** in a development environment first
3. **Use semantic versioning** for your plugins
4. **Register custom migrations** for breaking changes
5. **Clean up old versions** regularly to save storage space
6. **Monitor version events** for debugging and logging
7. **Check compatibility** before installing new versions

## Error Handling

All version management operations return `qtplugin::expected<T, VersionError>` for robust error handling:

```cpp
auto result = version_manager->install_version("my.plugin", version, path);
if (!result) {
    switch (result.error().code) {
        case VersionErrorCode::VersionAlreadyExists:
            std::cout << "Version already installed" << std::endl;
            break;
        case VersionErrorCode::StorageError:
            std::cout << "Storage error: " << result.error().message << std::endl;
            break;
        case VersionErrorCode::MigrationFailed:
            std::cout << "Migration failed: " << result.error().message << std::endl;
            break;
        // Handle other error codes...
    }
}
```

## See Also

- [Plugin Development Guide](plugin_development.md)
- [API Reference](api/README.md)
- [Examples](../examples/version_management/)
- [Testing Guide](testing.md)
