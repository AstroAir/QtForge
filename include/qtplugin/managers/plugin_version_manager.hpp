/**
 * @file plugin_version_manager.hpp
 * @brief Enhanced plugin version management system
 * @version 3.1.0
 * @author QtPlugin Development Team
 *
 * This file defines the enhanced version management system that provides
 * multi-version plugin support, version migration, rollback capabilities,
 * and compatibility management.
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../core/plugin_interface.hpp"
#include "../utils/error_handling.hpp"
#include "../utils/version.hpp"

namespace qtplugin {

// Forward declarations
class IPluginRegistry;
class IConfigurationManager;
class ILoggingManager;

/**
 * @brief Plugin version installation status
 */
enum class VersionInstallStatus {
    NotInstalled,   ///< Version is not installed
    Installing,     ///< Version is being installed
    Installed,      ///< Version is installed and available
    Active,         ///< Version is currently active/loaded
    Deprecated,     ///< Version is deprecated but still available
    Corrupted,      ///< Version installation is corrupted
    Migrating,      ///< Version is being migrated
    RollingBack     ///< Version is being rolled back
};

/**
 * @brief Version migration strategy
 */
enum class MigrationStrategy {
    None,           ///< No migration needed
    Automatic,      ///< Automatic migration using built-in rules
    Manual,         ///< Manual migration with user intervention
    Script,         ///< Migration using custom script
    Callback        ///< Migration using callback function
};

/**
 * @brief Version compatibility level
 */
enum class CompatibilityLevel {
    Breaking,       ///< Breaking changes, manual migration required
    Major,          ///< Major changes, automatic migration possible
    Minor,          ///< Minor changes, backward compatible
    Patch,          ///< Patch changes, fully compatible
    Build           ///< Build changes, no migration needed
};

/**
 * @brief Plugin version information
 */
struct PluginVersionInfo {
    std::string plugin_id;                              ///< Plugin identifier
    Version version;                                    ///< Version number
    std::filesystem::path installation_path;           ///< Installation directory
    VersionInstallStatus status;                        ///< Installation status
    std::chrono::system_clock::time_point install_time; ///< Installation timestamp
    std::chrono::system_clock::time_point last_used;   ///< Last usage timestamp
    QJsonObject metadata;                               ///< Version-specific metadata
    std::vector<std::string> dependencies;             ///< Version dependencies
    std::optional<std::string> migration_script;       ///< Migration script path
    CompatibilityLevel compatibility_level;            ///< Compatibility with previous version
    bool is_active;                                     ///< Whether this version is currently active
    size_t usage_count;                                 ///< Number of times this version was used
    QJsonObject configuration_schema;                   ///< Configuration schema for this version

    PluginVersionInfo()
        : status(VersionInstallStatus::NotInstalled)
        , install_time(std::chrono::system_clock::now())
        , last_used(std::chrono::system_clock::now())
        , compatibility_level(CompatibilityLevel::Minor)
        , is_active(false)
        , usage_count(0) {}

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static qtplugin::expected<PluginVersionInfo, PluginError> from_json(
        const QJsonObject& json);
};

/**
 * @brief Version migration context
 */
struct MigrationContext {
    std::string plugin_id;                              ///< Plugin being migrated
    Version from_version;                               ///< Source version
    Version to_version;                                 ///< Target version
    MigrationStrategy strategy;                         ///< Migration strategy
    std::filesystem::path data_directory;              ///< Plugin data directory
    QJsonObject old_configuration;                     ///< Old configuration
    QJsonObject new_configuration;                     ///< New configuration
    std::vector<std::string> backup_files;            ///< Files to backup
    std::function<qtplugin::expected<void, PluginError>(const MigrationContext&)>
        custom_migrator;                                ///< Custom migration function
    bool preserve_user_data;                           ///< Whether to preserve user data
    bool create_backup;                                ///< Whether to create backup

    MigrationContext(std::string_view id, const Version& from, const Version& to)
        : plugin_id(id)
        , from_version(from)
        , to_version(to)
        , strategy(MigrationStrategy::Automatic)
        , preserve_user_data(true)
        , create_backup(true) {}
};

/**
 * @brief Version rollback information
 */
struct RollbackInfo {
    std::string plugin_id;                              ///< Plugin identifier
    Version current_version;                            ///< Current version
    Version target_version;                             ///< Target rollback version
    std::filesystem::path backup_path;                 ///< Backup location
    std::chrono::system_clock::time_point backup_time; ///< Backup timestamp
    QJsonObject backup_metadata;                       ///< Backup metadata
    std::vector<std::string> affected_files;          ///< Files affected by rollback
    bool data_migration_required;                      ///< Whether data migration is needed

    RollbackInfo(std::string_view id, const Version& current, const Version& target)
        : plugin_id(id)
        , current_version(current)
        , target_version(target)
        , backup_time(std::chrono::system_clock::now())
        , data_migration_required(false) {}
};

/**
 * @brief Version management error codes
 */
enum class VersionErrorCode {
    Unknown,                ///< Unknown error
    VersionNotFound,        ///< Requested version not found
    VersionAlreadyExists,   ///< Version already installed
    IncompatibleVersion,    ///< Version is incompatible
    MigrationFailed,        ///< Migration process failed
    RollbackFailed,         ///< Rollback process failed
    BackupFailed,           ///< Backup creation failed
    CorruptedInstallation,  ///< Installation is corrupted
    DependencyConflict,     ///< Version dependency conflict
    InsufficientPermissions, ///< Insufficient permissions for operation
    StorageError,           ///< Storage/filesystem error
    InvalidMigrationScript, ///< Migration script is invalid
    ActiveVersionConflict   ///< Cannot modify active version
};

/**
 * @brief Version management error
 */
struct VersionError {
    VersionErrorCode code = VersionErrorCode::Unknown;
    std::string message = "Unknown error";
    std::string plugin_id;
    std::optional<Version> version;
    QJsonObject details;

    // Default constructor
    VersionError() = default;

    VersionError(VersionErrorCode c, std::string_view msg,
                std::string_view id = {}, std::optional<Version> ver = std::nullopt)
        : code(c), message(msg), plugin_id(id), version(ver) {}
};

/**
 * @brief Plugin version manager interface
 *
 * This interface provides comprehensive version management capabilities
 * including multi-version support, migration, rollback, and compatibility
 * management for plugins.
 */
class IPluginVersionManager {
public:
    virtual ~IPluginVersionManager() = default;

    // === Version Installation ===

    /**
     * @brief Install a specific version of a plugin
     * @param plugin_id Plugin identifier
     * @param version Version to install
     * @param file_path Path to plugin file
     * @param replace_existing Whether to replace existing version
     * @return Success or error information
     */
    virtual qtplugin::expected<void, VersionError> install_version(
        std::string_view plugin_id, const Version& version,
        const std::filesystem::path& file_path, bool replace_existing = false) = 0;

    /**
     * @brief Uninstall a specific version of a plugin
     * @param plugin_id Plugin identifier
     * @param version Version to uninstall
     * @param force Force uninstall even if active
     * @return Success or error information
     */
    virtual qtplugin::expected<void, VersionError> uninstall_version(
        std::string_view plugin_id, const Version& version, bool force = false) = 0;

    /**
     * @brief Get all installed versions of a plugin
     * @param plugin_id Plugin identifier
     * @return Vector of installed versions
     */
    virtual std::vector<PluginVersionInfo> get_installed_versions(
        std::string_view plugin_id) const = 0;

    /**
     * @brief Get currently active version of a plugin
     * @param plugin_id Plugin identifier
     * @return Active version info, or nullopt if not active
     */
    virtual std::optional<PluginVersionInfo> get_active_version(
        std::string_view plugin_id) const = 0;

    /**
     * @brief Set active version for a plugin
     * @param plugin_id Plugin identifier
     * @param version Version to activate
     * @param migrate_data Whether to migrate data from previous version
     * @return Success or error information
     */
    virtual qtplugin::expected<void, VersionError> set_active_version(
        std::string_view plugin_id, const Version& version,
        bool migrate_data = true) = 0;

    // === Version Migration ===

    /**
     * @brief Migrate plugin data between versions
     * @param context Migration context with source and target versions
     * @return Success or error information
     */
    virtual qtplugin::expected<void, VersionError> migrate_plugin_data(
        const MigrationContext& context) = 0;

    /**
     * @brief Register custom migration function
     * @param plugin_id Plugin identifier
     * @param from_version Source version
     * @param to_version Target version
     * @param migrator Custom migration function
     * @return Success or error information
     */
    virtual qtplugin::expected<void, VersionError> register_migration(
        std::string_view plugin_id, const Version& from_version,
        const Version& to_version,
        std::function<qtplugin::expected<void, PluginError>(const MigrationContext&)> migrator) = 0;

    /**
     * @brief Check if migration is available between versions
     * @param plugin_id Plugin identifier
     * @param from_version Source version
     * @param to_version Target version
     * @return true if migration is available
     */
    virtual bool is_migration_available(
        std::string_view plugin_id, const Version& from_version,
        const Version& to_version) const = 0;

    // === Version Rollback ===

    /**
     * @brief Create backup of current plugin version
     * @param plugin_id Plugin identifier
     * @param version Version to backup
     * @return Backup information or error
     */
    virtual qtplugin::expected<RollbackInfo, VersionError> create_backup(
        std::string_view plugin_id, const Version& version) = 0;

    /**
     * @brief Rollback plugin to previous version
     * @param plugin_id Plugin identifier
     * @param target_version Target version to rollback to
     * @param preserve_user_data Whether to preserve user data
     * @return Success or error information
     */
    virtual qtplugin::expected<void, VersionError> rollback_to_version(
        std::string_view plugin_id, const Version& target_version,
        bool preserve_user_data = true) = 0;

    /**
     * @brief Get available rollback points
     * @param plugin_id Plugin identifier
     * @return Vector of available rollback versions
     */
    virtual std::vector<RollbackInfo> get_rollback_points(
        std::string_view plugin_id) const = 0;

    /**
     * @brief Clean up old backups
     * @param plugin_id Plugin identifier (empty for all plugins)
     * @param keep_count Number of backups to keep
     * @return Number of backups cleaned up
     */
    virtual int cleanup_old_backups(std::string_view plugin_id = {},
                                   int keep_count = 5) = 0;

    // === Compatibility Management ===

    /**
     * @brief Check version compatibility
     * @param plugin_id Plugin identifier
     * @param version Version to check
     * @param host_version Host application version
     * @return Compatibility level
     */
    virtual CompatibilityLevel check_compatibility(
        std::string_view plugin_id, const Version& version,
        const Version& host_version) const = 0;

    /**
     * @brief Get compatible versions for host
     * @param plugin_id Plugin identifier
     * @param host_version Host application version
     * @return Vector of compatible versions
     */
    virtual std::vector<Version> get_compatible_versions(
        std::string_view plugin_id, const Version& host_version) const = 0;

    /**
     * @brief Register compatibility rules
     * @param plugin_id Plugin identifier
     * @param rules Compatibility rules as JSON
     * @return Success or error information
     */
    virtual qtplugin::expected<void, VersionError> register_compatibility_rules(
        std::string_view plugin_id, const QJsonObject& rules) = 0;

    // === Version Information ===

    /**
     * @brief Get version information
     * @param plugin_id Plugin identifier
     * @param version Specific version (optional, gets active if not specified)
     * @return Version information or error
     */
    virtual qtplugin::expected<PluginVersionInfo, VersionError> get_version_info(
        std::string_view plugin_id, std::optional<Version> version = std::nullopt) const = 0;

    /**
     * @brief Get version history for a plugin
     * @param plugin_id Plugin identifier
     * @return Vector of version history entries
     */
    virtual std::vector<PluginVersionInfo> get_version_history(
        std::string_view plugin_id) const = 0;

    /**
     * @brief Get version statistics
     * @return Version management statistics as JSON
     */
    virtual QJsonObject get_version_statistics() const = 0;

    // === Storage Management ===

    /**
     * @brief Set version storage directory
     * @param directory Directory to store plugin versions
     * @return Success or error information
     */
    virtual qtplugin::expected<void, VersionError> set_storage_directory(
        const std::filesystem::path& directory) = 0;

    /**
     * @brief Get version storage directory
     * @return Current storage directory
     */
    virtual std::filesystem::path get_storage_directory() const = 0;

    /**
     * @brief Clean up unused versions
     * @param plugin_id Plugin identifier (empty for all plugins)
     * @param keep_count Number of versions to keep
     * @return Number of versions cleaned up
     */
    virtual int cleanup_unused_versions(std::string_view plugin_id = {},
                                       int keep_count = 3) = 0;

    /**
     * @brief Get storage usage information
     * @param plugin_id Plugin identifier (empty for all plugins)
     * @return Storage usage information as JSON
     */
    virtual QJsonObject get_storage_usage(std::string_view plugin_id = {}) const = 0;

    // === Event Notifications ===

    /**
     * @brief Register version event callback
     * @param callback Callback function for version events
     * @return Subscription ID for unregistering
     */
    virtual std::string register_version_event_callback(
        std::function<void(const std::string&, const Version&, VersionInstallStatus)> callback) = 0;

    /**
     * @brief Unregister version event callback
     * @param subscription_id Subscription ID from register_version_event_callback
     */
    virtual void unregister_version_event_callback(const std::string& subscription_id) = 0;
};

/**
 * @brief Factory function to create a plugin version manager
 * @param registry Plugin registry instance
 * @param config_manager Configuration manager instance
 * @param logger Logging manager instance
 * @param parent Parent QObject (optional)
 * @return Unique pointer to plugin version manager
 */
std::unique_ptr<IPluginVersionManager> create_plugin_version_manager(
    std::shared_ptr<IPluginRegistry> registry,
    std::shared_ptr<IConfigurationManager> config_manager,
    std::shared_ptr<ILoggingManager> logger,
    QObject* parent = nullptr);

}  // namespace qtplugin
