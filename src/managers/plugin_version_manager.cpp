/**
 * @file plugin_version_manager.cpp
 * @brief Implementation of enhanced plugin version management system
 * @version 3.1.0
 * @author QtPlugin Development Team
 */

#include "../../include/qtplugin/managers/plugin_version_manager.hpp"
#include "../../include/qtplugin/core/plugin_registry.hpp"
#include "../../include/qtplugin/managers/configuration_manager.hpp"
#include "../../include/qtplugin/managers/logging_manager.hpp"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QUuid>
#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>

namespace qtplugin {

/**
 * @brief Concrete implementation of plugin version manager
 */
class PluginVersionManager : public QObject, public IPluginVersionManager {
    Q_OBJECT

public:
    explicit PluginVersionManager(
        std::shared_ptr<IPluginRegistry> registry,
        std::shared_ptr<IConfigurationManager> config_manager,
        std::shared_ptr<ILoggingManager> logger, QObject* parent = nullptr);

    ~PluginVersionManager() override;

    // === Version Installation ===
    qtplugin::expected<void, VersionError> install_version(
        std::string_view plugin_id, const Version& version,
        const std::filesystem::path& file_path,
        bool replace_existing = false) override;

    qtplugin::expected<void, VersionError> uninstall_version(
        std::string_view plugin_id, const Version& version,
        bool force = false) override;

    std::vector<PluginVersionInfo> get_installed_versions(
        std::string_view plugin_id) const override;

    std::optional<PluginVersionInfo> get_active_version(
        std::string_view plugin_id) const override;

    qtplugin::expected<void, VersionError> set_active_version(
        std::string_view plugin_id, const Version& version,
        bool migrate_data = true) override;

    // === Version Migration ===
    qtplugin::expected<void, VersionError> migrate_plugin_data(
        const MigrationContext& context) override;

    qtplugin::expected<void, VersionError> register_migration(
        std::string_view plugin_id, const Version& from_version,
        const Version& to_version,
        std::function<
            qtplugin::expected<void, PluginError>(const MigrationContext&)>
            migrator) override;

    bool is_migration_available(std::string_view plugin_id,
                                const Version& from_version,
                                const Version& to_version) const override;

    // === Version Rollback ===
    qtplugin::expected<RollbackInfo, VersionError> create_backup(
        std::string_view plugin_id, const Version& version) override;

    qtplugin::expected<void, VersionError> rollback_to_version(
        std::string_view plugin_id, const Version& target_version,
        bool preserve_user_data = true) override;

    std::vector<RollbackInfo> get_rollback_points(
        std::string_view plugin_id) const override;

    int cleanup_old_backups(std::string_view plugin_id = {},
                            int keep_count = 5) override;

    // === Compatibility Management ===
    CompatibilityLevel check_compatibility(
        std::string_view plugin_id, const Version& version,
        const Version& host_version) const override;

    std::vector<Version> get_compatible_versions(
        std::string_view plugin_id, const Version& host_version) const override;

    qtplugin::expected<void, VersionError> register_compatibility_rules(
        std::string_view plugin_id, const QJsonObject& rules) override;

    // === Version Information ===
    qtplugin::expected<PluginVersionInfo, VersionError> get_version_info(
        std::string_view plugin_id,
        std::optional<Version> version = std::nullopt) const override;

    std::vector<PluginVersionInfo> get_version_history(
        std::string_view plugin_id) const override;

    QJsonObject get_version_statistics() const override;

    // === Storage Management ===
    qtplugin::expected<void, VersionError> set_storage_directory(
        const std::filesystem::path& directory) override;

    std::filesystem::path get_storage_directory() const override;

    int cleanup_unused_versions(std::string_view plugin_id = {},
                                int keep_count = 3) override;

    QJsonObject get_storage_usage(
        std::string_view plugin_id = {}) const override;

    // === Event Notifications ===
    std::string register_version_event_callback(
        std::function<void(const std::string&, const Version&,
                           VersionInstallStatus)>
            callback) override;

    void unregister_version_event_callback(
        const std::string& subscription_id) override;

signals:
    void version_installed(const QString& plugin_id, const QString& version);
    void version_uninstalled(const QString& plugin_id, const QString& version);
    void version_activated(const QString& plugin_id, const QString& version);
    void migration_started(const QString& plugin_id,
                           const QString& from_version,
                           const QString& to_version);
    void migration_completed(const QString& plugin_id,
                             const QString& from_version,
                             const QString& to_version);
    void rollback_started(const QString& plugin_id,
                          const QString& target_version);
    void rollback_completed(const QString& plugin_id,
                            const QString& target_version);

private:
    // === Private Types ===
    struct MigrationRule {
        Version from_version;
        Version to_version;
        MigrationStrategy strategy;
        std::function<qtplugin::expected<void, PluginError>(
            const MigrationContext&)>
            migrator;
        std::optional<std::string> script_path;
    };

    struct CompatibilityRule {
        Version min_host_version;
        Version max_host_version;
        CompatibilityLevel level;
        QJsonObject metadata;
    };

    struct EventSubscription {
        std::string id;
        std::function<void(const std::string&, const Version&,
                           VersionInstallStatus)>
            callback;
    };

    // === Private Members ===
    std::shared_ptr<IPluginRegistry> registry_;
    std::shared_ptr<IConfigurationManager> config_manager_;
    std::shared_ptr<ILoggingManager> logger_;

    mutable std::shared_mutex versions_mutex_;
    std::unordered_map<std::string, std::vector<PluginVersionInfo>>
        installed_versions_;
    std::unordered_map<std::string, std::string>
        active_versions_;  // plugin_id -> version_string

    mutable std::shared_mutex migrations_mutex_;
    std::unordered_map<std::string, std::vector<MigrationRule>>
        migration_rules_;

    mutable std::shared_mutex compatibility_mutex_;
    std::unordered_map<std::string, std::vector<CompatibilityRule>>
        compatibility_rules_;

    mutable std::shared_mutex rollback_mutex_;
    std::unordered_map<std::string, std::vector<RollbackInfo>> rollback_points_;

    mutable std::shared_mutex events_mutex_;
    std::unordered_map<std::string, EventSubscription> event_subscriptions_;

    std::filesystem::path storage_directory_;
    std::atomic<bool> initialized_{false};

    // === Private Methods ===
    void initialize();
    void load_version_database();
    void save_version_database();
    void load_migration_rules();
    void save_migration_rules();
    void load_compatibility_rules();
    void save_compatibility_rules();
    void load_rollback_points();
    void save_rollback_points();

    std::filesystem::path get_plugin_version_directory(
        std::string_view plugin_id, const Version& version) const;
    std::filesystem::path get_plugin_data_directory(
        std::string_view plugin_id, const Version& version) const;
    std::filesystem::path get_backup_directory(
        std::string_view plugin_id) const;

    qtplugin::expected<void, VersionError> copy_plugin_files(
        const std::filesystem::path& source,
        const std::filesystem::path& destination);
    qtplugin::expected<void, VersionError> validate_plugin_installation(
        std::string_view plugin_id, const Version& version);

    void notify_version_event(const std::string& plugin_id,
                              const Version& version,
                              VersionInstallStatus status);

    std::string generate_subscription_id();
    std::string calculate_file_hash(const std::filesystem::path& file_path);

    qtplugin::expected<void, VersionError> perform_automatic_migration(
        const MigrationContext& context);
    qtplugin::expected<void, VersionError> perform_script_migration(
        const MigrationContext& context);
    qtplugin::expected<void, VersionError> perform_callback_migration(
        const MigrationContext& context);

    CompatibilityLevel determine_compatibility_level(
        const Version& version1, const Version& version2) const;

    // Cleanup helper methods
    int cleanup_plugin_versions(std::vector<PluginVersionInfo>& versions,
                                int keep_count);

    // Migration helper methods
    qtplugin::expected<void, VersionError> migrate_configuration_files(
        const MigrationContext& context);
    qtplugin::expected<void, VersionError> migrate_with_transformation(
        const MigrationContext& context);
    QJsonObject transform_configuration(const QJsonObject& source_config,
                                        const Version& from_version,
                                        const Version& to_version);
};

}  // namespace qtplugin

namespace qtplugin {

// === PluginVersionInfo Implementation ===

QJsonObject PluginVersionInfo::to_json() const {
    QJsonObject json;
    json["plugin_id"] = QString::fromStdString(plugin_id);
    json["version"] = QString::fromStdString(version.to_string());
    json["installation_path"] =
        QString::fromStdString(installation_path.string());
    json["status"] = static_cast<int>(status);
    json["install_time"] = static_cast<qint64>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            install_time.time_since_epoch())
            .count());
    json["last_used"] = static_cast<qint64>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            last_used.time_since_epoch())
            .count());
    json["metadata"] = metadata;

    QJsonArray deps_array;
    for (const auto& dep : dependencies) {
        deps_array.append(QString::fromStdString(dep));
    }
    json["dependencies"] = deps_array;

    if (migration_script) {
        json["migration_script"] = QString::fromStdString(*migration_script);
    }

    json["compatibility_level"] = static_cast<int>(compatibility_level);
    json["is_active"] = is_active;
    json["usage_count"] = static_cast<qint64>(usage_count);
    json["configuration_schema"] = configuration_schema;

    return json;
}

qtplugin::expected<PluginVersionInfo, PluginError> PluginVersionInfo::from_json(
    const QJsonObject& json) {
    PluginVersionInfo info;

    if (!json.contains("plugin_id") || !json.contains("version")) {
        return qtplugin::unexpected(
            PluginError{PluginErrorCode::InvalidConfiguration,
                        "Missing required fields in PluginVersionInfo JSON"});
    }

    info.plugin_id = json["plugin_id"].toString().toStdString();

    // Parse version string manually since Version doesn't have from_string
    auto version_str = json["version"].toString().toStdString();
    try {
        // Simple version parsing - expecting "major.minor.patch" format
        std::istringstream iss(version_str);
        std::string token;
        std::vector<int> parts;

        while (std::getline(iss, token, '.') && parts.size() < 3) {
            parts.push_back(std::stoi(token));
        }

        if (parts.size() >= 3) {
            info.version = Version(parts[0], parts[1], parts[2]);
        } else {
            return qtplugin::unexpected(PluginError{
                PluginErrorCode::InvalidConfiguration,
                "Invalid version format in PluginVersionInfo JSON: " +
                    version_str});
        }
    } catch (const std::exception& e) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidConfiguration,
            "Failed to parse version: " + version_str + " - " + e.what()});
    }

    if (json.contains("installation_path")) {
        info.installation_path =
            json["installation_path"].toString().toStdString();
    }

    if (json.contains("status")) {
        info.status = static_cast<VersionInstallStatus>(json["status"].toInt());
    }

    if (json.contains("install_time")) {
        auto timestamp = std::chrono::milliseconds(
            json["install_time"].toVariant().toLongLong());
        info.install_time = std::chrono::system_clock::time_point(timestamp);
    }

    if (json.contains("last_used")) {
        auto timestamp = std::chrono::milliseconds(
            json["last_used"].toVariant().toLongLong());
        info.last_used = std::chrono::system_clock::time_point(timestamp);
    }

    if (json.contains("metadata")) {
        info.metadata = json["metadata"].toObject();
    }

    if (json.contains("dependencies")) {
        const auto deps_array = json["dependencies"].toArray();
        for (const auto& dep : deps_array) {
            info.dependencies.push_back(dep.toString().toStdString());
        }
    }

    if (json.contains("migration_script")) {
        info.migration_script =
            json["migration_script"].toString().toStdString();
    }

    if (json.contains("compatibility_level")) {
        info.compatibility_level = static_cast<CompatibilityLevel>(
            json["compatibility_level"].toInt());
    }

    if (json.contains("is_active")) {
        info.is_active = json["is_active"].toBool();
    }

    if (json.contains("usage_count")) {
        info.usage_count =
            static_cast<size_t>(json["usage_count"].toVariant().toLongLong());
    }

    if (json.contains("configuration_schema")) {
        info.configuration_schema = json["configuration_schema"].toObject();
    }

    return info;
}

// === PluginVersionManager Implementation ===

PluginVersionManager::PluginVersionManager(
    std::shared_ptr<IPluginRegistry> registry,
    std::shared_ptr<IConfigurationManager> config_manager,
    std::shared_ptr<ILoggingManager> logger, QObject* parent)
    : QObject(parent),
      registry_(std::move(registry)),
      config_manager_(std::move(config_manager)),
      logger_(std::move(logger)) {
    initialize();
}

PluginVersionManager::~PluginVersionManager() {
    if (initialized_) {
        save_version_database();
        save_migration_rules();
        save_compatibility_rules();
        save_rollback_points();
    }
}

void PluginVersionManager::initialize() {
    // Set default storage directory
    auto app_data_dir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    storage_directory_ =
        std::filesystem::path(app_data_dir.toStdString()) / "plugin_versions";

    // Create storage directory if it doesn't exist
    std::filesystem::create_directories(storage_directory_);
    std::filesystem::create_directories(storage_directory_ / "backups");
    std::filesystem::create_directories(storage_directory_ / "migrations");

    // Load existing data
    load_version_database();
    load_migration_rules();
    load_compatibility_rules();
    load_rollback_points();

    initialized_ = true;

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     "Version manager initialized with storage at: " +
                         storage_directory_.string());
    }
}

qtplugin::expected<void, VersionError> PluginVersionManager::install_version(
    std::string_view plugin_id, const Version& version,
    const std::filesystem::path& file_path, bool replace_existing) {
    std::unique_lock lock(versions_mutex_);

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     std::string("Installing plugin version: ") +
                         std::string(plugin_id) + " v" + version.to_string());
    }

    // Check if version already exists
    auto& versions = installed_versions_[std::string(plugin_id)];
    auto existing = std::find_if(versions.begin(), versions.end(),
                                 [&version](const PluginVersionInfo& info) {
                                     return info.version == version;
                                 });

    if (existing != versions.end() && !replace_existing) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::VersionAlreadyExists,
            "Version " + version.to_string() + " already exists for plugin " +
                std::string(plugin_id),
            std::string(plugin_id), version});
    }

    // Validate source file
    if (!std::filesystem::exists(file_path)) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::StorageError,
            "Source plugin file does not exist: " + file_path.string(),
            std::string(plugin_id), version});
    }

    // Create version directory
    auto version_dir = get_plugin_version_directory(plugin_id, version);
    std::filesystem::create_directories(version_dir);

    // Copy plugin files
    auto copy_result = copy_plugin_files(file_path, version_dir);
    if (!copy_result) {
        return copy_result;
    }

    // Create version info
    PluginVersionInfo version_info;
    version_info.plugin_id = plugin_id;
    version_info.version = version;
    version_info.installation_path = version_dir;
    version_info.status = VersionInstallStatus::Installed;
    version_info.install_time = std::chrono::system_clock::now();
    version_info.last_used = version_info.install_time;
    version_info.is_active = false;
    version_info.usage_count = 0;

    // Validate installation
    auto validation_result = validate_plugin_installation(plugin_id, version);
    if (!validation_result) {
        // Clean up on validation failure
        std::filesystem::remove_all(version_dir);
        return validation_result;
    }

    // Update or add version info
    if (existing != versions.end()) {
        *existing = version_info;
    } else {
        versions.push_back(version_info);
    }

    // Sort versions by version number
    std::sort(versions.begin(), versions.end(),
              [](const PluginVersionInfo& a, const PluginVersionInfo& b) {
                  return a.version < b.version;
              });

    // Save database
    save_version_database();

    // Notify event subscribers
    notify_version_event(std::string(plugin_id), version,
                         VersionInstallStatus::Installed);

    // Emit Qt signal
    emit version_installed(QString::fromStdString(std::string(plugin_id)),
                           QString::fromStdString(version.to_string()));

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     "Successfully installed plugin version: " +
                         std::string(plugin_id) + " v" + version.to_string());
    }

    return {};
}

qtplugin::expected<void, VersionError> PluginVersionManager::uninstall_version(
    std::string_view plugin_id, const Version& version, bool force) {
    std::unique_lock lock(versions_mutex_);

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     std::string("Uninstalling plugin version: ") +
                         std::string(plugin_id) + " v" + version.to_string());
    }

    auto& versions = installed_versions_[std::string(plugin_id)];
    auto it = std::find_if(versions.begin(), versions.end(),
                           [&version](const PluginVersionInfo& info) {
                               return info.version == version;
                           });

    if (it == versions.end()) {
        return qtplugin::unexpected(
            VersionError{VersionErrorCode::VersionNotFound,
                         "Version " + version.to_string() +
                             " not found for plugin " + std::string(plugin_id),
                         std::string(plugin_id), version});
    }

    // Check if version is active
    if (it->is_active && !force) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::ActiveVersionConflict,
            "Cannot uninstall active version " + version.to_string() +
                " for plugin " + std::string(plugin_id),
            std::string(plugin_id), version});
    }

    // Remove files
    try {
        std::filesystem::remove_all(it->installation_path);
    } catch (const std::filesystem::filesystem_error& e) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::StorageError,
            "Failed to remove plugin files: " + std::string(e.what()),
            std::string(plugin_id), version});
    }

    // Remove from active versions if it was active
    if (it->is_active) {
        active_versions_.erase(std::string(plugin_id));
    }

    // Remove from installed versions
    versions.erase(it);

    // Clean up empty plugin entry
    if (versions.empty()) {
        installed_versions_.erase(std::string(plugin_id));
    }

    // Save database
    save_version_database();

    // Notify event subscribers
    notify_version_event(std::string(plugin_id), version,
                         VersionInstallStatus::NotInstalled);

    // Emit Qt signal
    emit version_uninstalled(QString::fromStdString(std::string(plugin_id)),
                             QString::fromStdString(version.to_string()));

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     "Successfully uninstalled plugin version: " +
                         std::string(plugin_id) + " v" + version.to_string());
    }

    return {};
}

std::vector<PluginVersionInfo> PluginVersionManager::get_installed_versions(
    std::string_view plugin_id) const {
    std::shared_lock lock(versions_mutex_);

    auto it = installed_versions_.find(std::string(plugin_id));
    if (it != installed_versions_.end()) {
        return it->second;
    }

    return {};
}

std::optional<PluginVersionInfo> PluginVersionManager::get_active_version(
    std::string_view plugin_id) const {
    std::shared_lock lock(versions_mutex_);

    auto active_it = active_versions_.find(std::string(plugin_id));
    if (active_it == active_versions_.end()) {
        return std::nullopt;
    }

    auto versions_it = installed_versions_.find(std::string(plugin_id));
    if (versions_it == installed_versions_.end()) {
        return std::nullopt;
    }

    // Parse active version string
    auto active_version_str = active_it->second;
    for (const auto& version_info : versions_it->second) {
        if (version_info.version.to_string() == active_version_str &&
            version_info.is_active) {
            return version_info;
        }
    }

    return std::nullopt;
}

qtplugin::expected<void, VersionError> PluginVersionManager::set_active_version(
    std::string_view plugin_id, const Version& version, bool migrate_data) {
    std::unique_lock lock(versions_mutex_);

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     std::string("Setting active version: ") +
                         std::string(plugin_id) + " v" + version.to_string());
    }

    auto& versions = installed_versions_[std::string(plugin_id)];
    auto target_it = std::find_if(versions.begin(), versions.end(),
                                  [&version](const PluginVersionInfo& info) {
                                      return info.version == version;
                                  });

    if (target_it == versions.end()) {
        return qtplugin::unexpected(
            VersionError{VersionErrorCode::VersionNotFound,
                         "Version " + version.to_string() +
                             " not found for plugin " + std::string(plugin_id),
                         std::string(plugin_id), version});
    }

    // Find currently active version
    auto current_active = std::find_if(
        versions.begin(), versions.end(),
        [](const PluginVersionInfo& info) { return info.is_active; });

    // Perform migration if requested and there's a current active version
    if (migrate_data && current_active != versions.end() &&
        current_active->version != version) {
        MigrationContext context(plugin_id, current_active->version, version);
        context.data_directory =
            get_plugin_data_directory(plugin_id, current_active->version);

        auto migration_result = migrate_plugin_data(context);
        if (!migration_result) {
            return qtplugin::unexpected(
                VersionError{VersionErrorCode::MigrationFailed,
                             "Failed to migrate data from " +
                                 current_active->version.to_string() + " to " +
                                 version.to_string() + ": " +
                                 migration_result.error().message,
                             std::string(plugin_id), version});
        }
    }

    // Deactivate current active version
    if (current_active != versions.end()) {
        current_active->is_active = false;
        current_active->status = VersionInstallStatus::Installed;
    }

    // Activate target version
    target_it->is_active = true;
    target_it->status = VersionInstallStatus::Active;
    target_it->last_used = std::chrono::system_clock::now();
    target_it->usage_count++;

    // Update active versions map
    active_versions_[std::string(plugin_id)] = version.to_string();

    // Save database
    save_version_database();

    // Notify event subscribers
    notify_version_event(std::string(plugin_id), version,
                         VersionInstallStatus::Active);

    // Emit Qt signal
    emit version_activated(QString::fromStdString(std::string(plugin_id)),
                           QString::fromStdString(version.to_string()));

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     "Successfully activated plugin version: " +
                         std::string(plugin_id) + " v" + version.to_string());
    }

    return {};
}

// === Private Helper Methods ===

std::filesystem::path PluginVersionManager::get_plugin_version_directory(
    std::string_view plugin_id, const Version& version) const {
    return storage_directory_ / std::string(plugin_id) / version.to_string();
}

std::filesystem::path PluginVersionManager::get_plugin_data_directory(
    std::string_view plugin_id, const Version& version) const {
    return storage_directory_ / std::string(plugin_id) / version.to_string() /
           "data";
}

std::filesystem::path PluginVersionManager::get_backup_directory(
    std::string_view plugin_id) const {
    return storage_directory_ / "backups" / std::string(plugin_id);
}

qtplugin::expected<void, VersionError> PluginVersionManager::copy_plugin_files(
    const std::filesystem::path& source,
    const std::filesystem::path& destination) {
    try {
        if (std::filesystem::is_regular_file(source)) {
            // Single file
            auto dest_file = destination / source.filename();
            std::filesystem::copy_file(
                source, dest_file,
                std::filesystem::copy_options::overwrite_existing);
        } else if (std::filesystem::is_directory(source)) {
            // Directory
            std::filesystem::copy(
                source, destination,
                std::filesystem::copy_options::recursive |
                    std::filesystem::copy_options::overwrite_existing);
        } else {
            return qtplugin::unexpected(
                VersionError{VersionErrorCode::StorageError,
                             "Source path is neither a file nor a directory: " +
                                 source.string()});
        }
    } catch (const std::filesystem::filesystem_error& e) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::StorageError,
            "Failed to copy plugin files: " + std::string(e.what())});
    }

    return {};
}

qtplugin::expected<void, VersionError>
PluginVersionManager::validate_plugin_installation(std::string_view plugin_id,
                                                   const Version& version) {
    auto version_dir = get_plugin_version_directory(plugin_id, version);

    // Check if directory exists
    if (!std::filesystem::exists(version_dir)) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::CorruptedInstallation,
            "Plugin version directory does not exist: " + version_dir.string(),
            std::string(plugin_id), version});
    }

    // Check if directory is not empty
    if (std::filesystem::is_empty(version_dir)) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::CorruptedInstallation,
            "Plugin version directory is empty: " + version_dir.string(),
            std::string(plugin_id), version});
    }

    // Additional validation could be added here (e.g., checking for required
    // files)

    return {};
}

void PluginVersionManager::notify_version_event(const std::string& plugin_id,
                                                const Version& version,
                                                VersionInstallStatus status) {
    std::shared_lock lock(events_mutex_);

    for (const auto& [id, subscription] : event_subscriptions_) {
        try {
            subscription.callback(plugin_id, version, status);
        } catch (const std::exception& e) {
            if (logger_) {
                logger_->log(LogLevel::Warning, "PluginVersionManager",
                             "Exception in version event callback: " +
                                 std::string(e.what()));
            }
        }
    }
}

std::string PluginVersionManager::generate_subscription_id() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
}

// === Database Management Methods ===

void PluginVersionManager::load_version_database() {
    auto db_file = storage_directory_ / "versions.json";

    if (!std::filesystem::exists(db_file)) {
        return;  // No database file exists yet
    }

    try {
        QFile file(QString::fromStdString(db_file.string()));
        if (!file.open(QIODevice::ReadOnly)) {
            if (logger_) {
                logger_->log(LogLevel::Warning, "PluginVersionManager",
                             "Failed to open version database file: " +
                                 db_file.string());
            }
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject root = doc.object();

        // Load installed versions
        if (root.contains("installed_versions")) {
            QJsonObject installed = root["installed_versions"].toObject();
            for (auto it = installed.begin(); it != installed.end(); ++it) {
                std::string plugin_id = it.key().toStdString();
                QJsonArray versions_array = it.value().toArray();

                std::vector<PluginVersionInfo> versions;
                for (const auto& version_value : versions_array) {
                    auto version_result =
                        PluginVersionInfo::from_json(version_value.toObject());
                    if (version_result) {
                        versions.push_back(*version_result);
                    }
                }

                if (!versions.empty()) {
                    installed_versions_[plugin_id] = std::move(versions);
                }
            }
        }

        // Load active versions
        if (root.contains("active_versions")) {
            QJsonObject active = root["active_versions"].toObject();
            for (auto it = active.begin(); it != active.end(); ++it) {
                active_versions_[it.key().toStdString()] =
                    it.value().toString().toStdString();
            }
        }

    } catch (const std::exception& e) {
        if (logger_) {
            logger_->log(
                LogLevel::Error, "PluginVersionManager",
                "Failed to load version database: " + std::string(e.what()));
        }
    }
}

void PluginVersionManager::save_version_database() {
    auto db_file = storage_directory_ / "versions.json";

    try {
        QJsonObject root;

        // Save installed versions
        QJsonObject installed;
        for (const auto& [plugin_id, versions] : installed_versions_) {
            QJsonArray versions_array;
            for (const auto& version_info : versions) {
                versions_array.append(version_info.to_json());
            }
            installed[QString::fromStdString(plugin_id)] = versions_array;
        }
        root["installed_versions"] = installed;

        // Save active versions
        QJsonObject active;
        for (const auto& [plugin_id, version_str] : active_versions_) {
            active[QString::fromStdString(plugin_id)] =
                QString::fromStdString(version_str);
        }
        root["active_versions"] = active;

        // Write to file
        QJsonDocument doc(root);
        QFile file(QString::fromStdString(db_file.string()));
        if (!file.open(QIODevice::WriteOnly)) {
            if (logger_) {
                logger_->log(
                    LogLevel::Error, "PluginVersionManager",
                    "Failed to open version database file for writing: " +
                        db_file.string());
            }
            return;
        }

        file.write(doc.toJson());

    } catch (const std::exception& e) {
        if (logger_) {
            logger_->log(
                LogLevel::Error, "PluginVersionManager",
                "Failed to save version database: " + std::string(e.what()));
        }
    }
}

void PluginVersionManager::load_migration_rules() {
    // Implementation for loading migration rules from storage
    // This would load custom migration rules from a JSON file
}

void PluginVersionManager::save_migration_rules() {
    // Implementation for saving migration rules to storage
    // This would save custom migration rules to a JSON file
}

void PluginVersionManager::load_compatibility_rules() {
    // Implementation for loading compatibility rules from storage
    // This would load compatibility rules from a JSON file
}

void PluginVersionManager::save_compatibility_rules() {
    // Implementation for saving compatibility rules to storage
    // This would save compatibility rules to a JSON file
}

void PluginVersionManager::load_rollback_points() {
    // Implementation for loading rollback points from storage
    // This would load rollback information from a JSON file
}

void PluginVersionManager::save_rollback_points() {
    // Implementation for saving rollback points to storage
    // This would save rollback information to a JSON file
}

// === Event Management ===

std::string PluginVersionManager::register_version_event_callback(
    std::function<void(const std::string&, const Version&,
                       VersionInstallStatus)>
        callback) {
    std::unique_lock lock(events_mutex_);

    std::string subscription_id = generate_subscription_id();
    event_subscriptions_[subscription_id] =
        EventSubscription{subscription_id, std::move(callback)};

    return subscription_id;
}

void PluginVersionManager::unregister_version_event_callback(
    const std::string& subscription_id) {
    std::unique_lock lock(events_mutex_);
    event_subscriptions_.erase(subscription_id);
}

// === Storage Management ===

qtplugin::expected<void, VersionError>
PluginVersionManager::set_storage_directory(
    const std::filesystem::path& directory) {
    try {
        std::filesystem::create_directories(directory);
        std::filesystem::create_directories(directory / "backups");
        std::filesystem::create_directories(directory / "migrations");

        storage_directory_ = directory;

        if (logger_) {
            logger_->log(LogLevel::Info, "PluginVersionManager",
                         "Storage directory set to: " + directory.string());
        }

        return {};
    } catch (const std::filesystem::filesystem_error& e) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::StorageError,
            "Failed to set storage directory: " + std::string(e.what())});
    }
}

std::filesystem::path PluginVersionManager::get_storage_directory() const {
    return storage_directory_;
}

QJsonObject PluginVersionManager::get_version_statistics() const {
    std::shared_lock lock(versions_mutex_);

    QJsonObject stats;

    int total_plugins = static_cast<int>(installed_versions_.size());
    int total_versions = 0;
    int active_versions = static_cast<int>(active_versions_.size());

    for (const auto& [plugin_id, versions] : installed_versions_) {
        total_versions += static_cast<int>(versions.size());
    }

    stats["total_plugins"] = total_plugins;
    stats["total_versions"] = total_versions;
    stats["active_versions"] = active_versions;

    // Calculate storage usage
    try {
        uintmax_t total_size = 0;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(
                 storage_directory_)) {
            if (entry.is_regular_file()) {
                total_size += entry.file_size();
            }
        }
        stats["storage_size_bytes"] = static_cast<qint64>(total_size);
    } catch (const std::filesystem::filesystem_error&) {
        stats["storage_size_bytes"] = -1;  // Error calculating size
    }

    return stats;
}

// === Migration Implementation ===

qtplugin::expected<void, VersionError>
PluginVersionManager::migrate_plugin_data(const MigrationContext& context) {
    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     "Starting migration for plugin " + context.plugin_id +
                         " from " + context.from_version.to_string() + " to " +
                         context.to_version.to_string());
    }

    // Emit migration started signal
    emit migration_started(
        QString::fromStdString(context.plugin_id),
        QString::fromStdString(context.from_version.to_string()),
        QString::fromStdString(context.to_version.to_string()));

    // Notify event subscribers
    notify_version_event(context.plugin_id, context.from_version,
                         VersionInstallStatus::Migrating);

    qtplugin::expected<void, VersionError> result;

    switch (context.strategy) {
        case MigrationStrategy::None:
            result = {};  // No migration needed
            break;

        case MigrationStrategy::Automatic:
            result = perform_automatic_migration(context);
            break;

        case MigrationStrategy::Script:
            result = perform_script_migration(context);
            break;

        case MigrationStrategy::Callback:
            result = perform_callback_migration(context);
            break;

        case MigrationStrategy::Manual:
            result = qtplugin::unexpected(VersionError{
                VersionErrorCode::MigrationFailed,
                "Manual migration strategy requires user intervention",
                context.plugin_id, context.to_version});
            break;

        default:
            result = qtplugin::unexpected(VersionError{
                VersionErrorCode::MigrationFailed, "Unknown migration strategy",
                context.plugin_id, context.to_version});
            break;
    }

    if (result) {
        // Migration successful
        if (logger_) {
            logger_->log(LogLevel::Info, "PluginVersionManager",
                         "Migration completed successfully for plugin " +
                             context.plugin_id);
        }

        // Emit migration completed signal
        emit migration_completed(
            QString::fromStdString(context.plugin_id),
            QString::fromStdString(context.from_version.to_string()),
            QString::fromStdString(context.to_version.to_string()));
    } else {
        // Migration failed
        if (logger_) {
            logger_->log(LogLevel::Error, "PluginVersionManager",
                         "Migration failed for plugin " + context.plugin_id +
                             ": " + result.error().message);
        }
    }

    return result;
}

qtplugin::expected<void, VersionError> PluginVersionManager::register_migration(
    std::string_view plugin_id, const Version& from_version,
    const Version& to_version,
    std::function<
        qtplugin::expected<void, PluginError>(const MigrationContext&)>
        migrator) {
    std::unique_lock lock(migrations_mutex_);

    MigrationRule rule;
    rule.from_version = from_version;
    rule.to_version = to_version;
    rule.strategy = MigrationStrategy::Callback;
    rule.migrator = std::move(migrator);

    migration_rules_[std::string(plugin_id)].push_back(std::move(rule));

    // Sort migration rules by version for efficient lookup
    auto& rules = migration_rules_[std::string(plugin_id)];
    std::sort(rules.begin(), rules.end(),
              [](const MigrationRule& a, const MigrationRule& b) {
                  if (a.from_version != b.from_version) {
                      return a.from_version < b.from_version;
                  }
                  return a.to_version < b.to_version;
              });

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     "Registered migration rule for plugin " +
                         std::string(plugin_id) + " from " +
                         from_version.to_string() + " to " +
                         to_version.to_string());
    }

    return {};
}

bool PluginVersionManager::is_migration_available(
    std::string_view plugin_id, const Version& from_version,
    const Version& to_version) const {
    std::shared_lock lock(migrations_mutex_);

    auto it = migration_rules_.find(std::string(plugin_id));
    if (it == migration_rules_.end()) {
        // No custom migration rules, check if automatic migration is possible
        return determine_compatibility_level(from_version, to_version) !=
               CompatibilityLevel::Breaking;
    }

    // Check for exact migration rule
    const auto& rules = it->second;
    for (const auto& rule : rules) {
        if (rule.from_version == from_version &&
            rule.to_version == to_version) {
            return true;
        }
    }

    // Check for chain migration (from -> intermediate -> to)
    // This is a simplified implementation; a full implementation would use
    // graph algorithms
    for (const auto& rule1 : rules) {
        if (rule1.from_version == from_version) {
            for (const auto& rule2 : rules) {
                if (rule2.from_version == rule1.to_version &&
                    rule2.to_version == to_version) {
                    return true;
                }
            }
        }
    }

    return false;
}

// === Factory Function ===

std::unique_ptr<IPluginVersionManager> create_plugin_version_manager(
    std::shared_ptr<IPluginRegistry> registry,
    std::shared_ptr<IConfigurationManager> config_manager,
    std::shared_ptr<ILoggingManager> logger, QObject* parent) {
    return std::unique_ptr<IPluginVersionManager>(
        new PluginVersionManager(std::move(registry), std::move(config_manager),
                                 std::move(logger), parent));
}

// === Missing Method Implementations ===

qtplugin::expected<void, VersionError>
PluginVersionManager::perform_automatic_migration(
    const MigrationContext& context) {
    if (logger_) {
        logger_->log(
            LogLevel::Info, "PluginVersionManager",
            "Performing automatic migration for plugin " + context.plugin_id);
    }

    // For automatic migration, we copy configuration files and attempt basic
    // transformations
    auto result = migrate_configuration_files(context);
    if (!result) {
        return result;
    }

    return migrate_with_transformation(context);
}

qtplugin::expected<void, VersionError>
PluginVersionManager::perform_script_migration(
    const MigrationContext& context) {
    if (logger_) {
        logger_->log(
            LogLevel::Info, "PluginVersionManager",
            "Performing script migration for plugin " + context.plugin_id);
    }

    // Find migration script
    std::shared_lock lock(migrations_mutex_);
    auto it = migration_rules_.find(context.plugin_id);
    if (it == migration_rules_.end()) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::InvalidMigrationScript,
            "No migration rules found for plugin " + context.plugin_id,
            context.plugin_id, context.to_version});
    }

    // Find matching rule with script
    for (const auto& rule : it->second) {
        if (rule.from_version == context.from_version &&
            rule.to_version == context.to_version && rule.script_path) {
            // Execute migration script (simplified implementation)
            // In a real implementation, this would execute the script
            if (logger_) {
                logger_->log(
                    LogLevel::Info, "PluginVersionManager",
                    "Executing migration script: " + *rule.script_path);
            }

            return {};  // Assume success for now
        }
    }

    return qtplugin::unexpected(
        VersionError{VersionErrorCode::InvalidMigrationScript,
                     "No migration script found for version transition",
                     context.plugin_id, context.to_version});
}

qtplugin::expected<void, VersionError>
PluginVersionManager::perform_callback_migration(
    const MigrationContext& context) {
    if (logger_) {
        logger_->log(
            LogLevel::Info, "PluginVersionManager",
            "Performing callback migration for plugin " + context.plugin_id);
    }

    // Find migration callback
    std::shared_lock lock(migrations_mutex_);
    auto it = migration_rules_.find(context.plugin_id);
    if (it == migration_rules_.end()) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::MigrationFailed,
            "No migration rules found for plugin " + context.plugin_id,
            context.plugin_id, context.to_version});
    }

    // Find matching rule with callback
    for (const auto& rule : it->second) {
        if (rule.from_version == context.from_version &&
            rule.to_version == context.to_version && rule.migrator) {
            // Execute migration callback
            auto result = rule.migrator(context);
            if (!result) {
                return qtplugin::unexpected(VersionError{
                    VersionErrorCode::MigrationFailed,
                    "Migration callback failed: " + result.error().message,
                    context.plugin_id, context.to_version});
            }

            return {};
        }
    }

    return qtplugin::unexpected(
        VersionError{VersionErrorCode::MigrationFailed,
                     "No migration callback found for version transition",
                     context.plugin_id, context.to_version});
}

CompatibilityLevel PluginVersionManager::determine_compatibility_level(
    const Version& version1, const Version& version2) const {
    if (version1.major() != version2.major()) {
        return CompatibilityLevel::Breaking;
    }

    if (version1.minor() != version2.minor()) {
        return CompatibilityLevel::Major;
    }

    if (version1.patch() != version2.patch()) {
        return CompatibilityLevel::Minor;
    }

    return CompatibilityLevel::Patch;
}

qtplugin::expected<void, VersionError>
PluginVersionManager::migrate_configuration_files(
    const MigrationContext& context) {
    auto source_config_dir =
        get_plugin_data_directory(context.plugin_id, context.from_version);
    auto target_config_dir =
        get_plugin_data_directory(context.plugin_id, context.to_version);

    try {
        if (std::filesystem::exists(source_config_dir)) {
            std::filesystem::create_directories(target_config_dir);
            std::filesystem::copy(
                source_config_dir, target_config_dir,
                std::filesystem::copy_options::recursive |
                    std::filesystem::copy_options::overwrite_existing);
        }
        return {};
    } catch (const std::filesystem::filesystem_error& e) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::MigrationFailed,
            "Failed to migrate configuration files: " + std::string(e.what()),
            context.plugin_id, context.to_version});
    }
}

qtplugin::expected<void, VersionError>
PluginVersionManager::migrate_with_transformation(
    const MigrationContext& context) {
    // Apply configuration transformations based on version differences
    auto config_file =
        get_plugin_data_directory(context.plugin_id, context.to_version) /
        "config.json";

    if (std::filesystem::exists(config_file)) {
        try {
            QFile file(QString::fromStdString(config_file.string()));
            if (file.open(QIODevice::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
                file.close();

                QJsonObject transformed = transform_configuration(
                    doc.object(), context.from_version, context.to_version);

                if (file.open(QIODevice::WriteOnly)) {
                    file.write(QJsonDocument(transformed).toJson());
                }
            }
        } catch (const std::exception& e) {
            return qtplugin::unexpected(VersionError{
                VersionErrorCode::MigrationFailed,
                "Failed to transform configuration: " + std::string(e.what()),
                context.plugin_id, context.to_version});
        }
    }

    return {};
}

QJsonObject PluginVersionManager::transform_configuration(
    const QJsonObject& source_config, const Version& from_version,
    const Version& to_version) {
    // Basic configuration transformation
    // In a real implementation, this would apply version-specific
    // transformations
    QJsonObject transformed = source_config;

    // Add version metadata
    transformed["migrated_from"] =
        QString::fromStdString(from_version.to_string());
    transformed["migrated_to"] = QString::fromStdString(to_version.to_string());
    transformed["migration_timestamp"] =
        QDateTime::currentDateTime().toString(Qt::ISODate);

    return transformed;
}

// === Rollback Implementation ===

qtplugin::expected<RollbackInfo, VersionError>
PluginVersionManager::create_backup(std::string_view plugin_id,
                                    const Version& version) {
    std::unique_lock lock(rollback_mutex_);

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     "Creating backup for plugin " + std::string(plugin_id) +
                         " v" + version.to_string());
    }

    // Create backup info - use constructor
    RollbackInfo backup_info(
        plugin_id, version, version);  // current and target are same for backup
    backup_info.backup_time = std::chrono::system_clock::now();

    // Generate unique backup directory name
    std::string backup_id = generate_subscription_id();

    // Create backup directory
    auto backup_dir = get_backup_directory(plugin_id) / backup_id;

    try {
        std::filesystem::create_directories(backup_dir);

        // Copy current version files
        auto version_dir = get_plugin_version_directory(plugin_id, version);
        if (std::filesystem::exists(version_dir)) {
            std::filesystem::copy(version_dir, backup_dir / "version",
                                  std::filesystem::copy_options::recursive);
        }

        // Copy data directory
        auto data_dir = get_plugin_data_directory(plugin_id, version);
        if (std::filesystem::exists(data_dir)) {
            std::filesystem::copy(data_dir, backup_dir / "data",
                                  std::filesystem::copy_options::recursive);
        }

        backup_info.backup_path = backup_dir;

        // Store affected files
        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(backup_dir)) {
            if (entry.is_regular_file()) {
                backup_info.affected_files.push_back(entry.path().string());
            }
        }

    } catch (const std::filesystem::filesystem_error& e) {
        return qtplugin::unexpected(
            VersionError{VersionErrorCode::BackupFailed,
                         "Failed to create backup: " + std::string(e.what()),
                         std::string(plugin_id), version});
    }

    // Store backup info
    rollback_points_[std::string(plugin_id)].push_back(backup_info);

    // Sort by timestamp (newest first)
    auto& backups = rollback_points_[std::string(plugin_id)];
    std::sort(backups.begin(), backups.end(),
              [](const RollbackInfo& a, const RollbackInfo& b) {
                  return a.backup_time > b.backup_time;
              });

    save_rollback_points();

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     "Successfully created backup " + backup_id +
                         " for plugin " + std::string(plugin_id));
    }

    return backup_info;
}

qtplugin::expected<void, VersionError>
PluginVersionManager::rollback_to_version(std::string_view plugin_id,
                                          const Version& target_version,
                                          bool preserve_user_data) {
    std::unique_lock lock(rollback_mutex_);

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     "Rolling back plugin " + std::string(plugin_id) + " to v" +
                         target_version.to_string());
    }

    // Emit rollback started signal
    emit rollback_started(QString::fromStdString(std::string(plugin_id)),
                          QString::fromStdString(target_version.to_string()));

    // Find backup for target version
    auto it = rollback_points_.find(std::string(plugin_id));
    if (it == rollback_points_.end()) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::RollbackFailed,
            "No rollback points found for plugin " + std::string(plugin_id),
            std::string(plugin_id), target_version});
    }

    auto backup_it =
        std::find_if(it->second.begin(), it->second.end(),
                     [&target_version](const RollbackInfo& info) {
                         return info.current_version == target_version ||
                                info.target_version == target_version;
                     });

    if (backup_it == it->second.end()) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::RollbackFailed,
            "No backup found for version " + target_version.to_string(),
            std::string(plugin_id), target_version});
    }

    try {
        // Restore from backup
        auto version_dir =
            get_plugin_version_directory(plugin_id, target_version);
        auto backup_version_dir = backup_it->backup_path / "version";

        if (std::filesystem::exists(backup_version_dir)) {
            std::filesystem::remove_all(version_dir);
            std::filesystem::copy(backup_version_dir, version_dir,
                                  std::filesystem::copy_options::recursive);
        }

        // Restore data if not preserving user data
        if (!preserve_user_data) {
            auto data_dir =
                get_plugin_data_directory(plugin_id, target_version);
            auto backup_data_dir = backup_it->backup_path / "data";

            if (std::filesystem::exists(backup_data_dir)) {
                std::filesystem::remove_all(data_dir);
                std::filesystem::copy(backup_data_dir, data_dir,
                                      std::filesystem::copy_options::recursive);
            }
        }

    } catch (const std::filesystem::filesystem_error& e) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::RollbackFailed,
            "Failed to restore from backup: " + std::string(e.what()),
            std::string(plugin_id), target_version});
    }

    // Update version status - deactivate all versions first, then activate
    // target
    {
        std::unique_lock versions_lock(versions_mutex_);
        auto& versions = installed_versions_[std::string(plugin_id)];

        // Deactivate all versions first
        for (auto& version_info : versions) {
            version_info.is_active = false;
            if (version_info.status == VersionInstallStatus::Active) {
                version_info.status = VersionInstallStatus::Installed;
            }
        }

        // Activate target version
        auto version_it =
            std::find_if(versions.begin(), versions.end(),
                         [&target_version](const PluginVersionInfo& info) {
                             return info.version == target_version;
                         });

        if (version_it != versions.end()) {
            version_it->status = VersionInstallStatus::Active;
            version_it->is_active = true;
            version_it->last_used = std::chrono::system_clock::now();

            // Update active_versions_ map as well
            active_versions_[std::string(plugin_id)] =
                target_version.to_string();
        }
    }

    // Emit rollback completed signal
    emit rollback_completed(QString::fromStdString(std::string(plugin_id)),
                            QString::fromStdString(target_version.to_string()));

    if (logger_) {
        logger_->log(LogLevel::Info, "PluginVersionManager",
                     "Successfully rolled back plugin " +
                         std::string(plugin_id) + " to v" +
                         target_version.to_string());
    }

    return {};
}

std::vector<RollbackInfo> PluginVersionManager::get_rollback_points(
    std::string_view plugin_id) const {
    std::shared_lock lock(rollback_mutex_);

    auto it = rollback_points_.find(std::string(plugin_id));
    if (it != rollback_points_.end()) {
        return it->second;
    }

    return {};
}

int PluginVersionManager::cleanup_old_backups(std::string_view plugin_id,
                                              int keep_count) {
    std::unique_lock lock(rollback_mutex_);

    int cleaned_count = 0;

    if (plugin_id.empty()) {
        // Clean up all plugins
        for (auto& [id, backups] : rollback_points_) {
            if (static_cast<int>(backups.size()) > keep_count) {
                // Sort by backup time (newest first)
                std::sort(backups.begin(), backups.end(),
                          [](const RollbackInfo& a, const RollbackInfo& b) {
                              return a.backup_time > b.backup_time;
                          });

                // Remove old backups
                int to_remove = static_cast<int>(backups.size()) - keep_count;
                for (int i = 0; i < to_remove; ++i) {
                    try {
                        std::filesystem::remove_all(backups.back().backup_path);
                        backups.pop_back();
                        cleaned_count++;
                    } catch (const std::filesystem::filesystem_error&) {
                        // Continue with other backups
                    }
                }
            }
        }
    } else {
        // Clean up specific plugin
        auto it = rollback_points_.find(std::string(plugin_id));
        if (it != rollback_points_.end()) {
            auto& backups = it->second;
            if (static_cast<int>(backups.size()) > keep_count) {
                // Sort by backup time (newest first)
                std::sort(backups.begin(), backups.end(),
                          [](const RollbackInfo& a, const RollbackInfo& b) {
                              return a.backup_time > b.backup_time;
                          });

                // Remove old backups
                int to_remove = static_cast<int>(backups.size()) - keep_count;
                for (int i = 0; i < to_remove; ++i) {
                    try {
                        std::filesystem::remove_all(backups.back().backup_path);
                        backups.pop_back();
                        cleaned_count++;
                    } catch (const std::filesystem::filesystem_error&) {
                        // Continue with other backups
                    }
                }
            }
        }
    }

    if (cleaned_count > 0) {
        save_rollback_points();

        if (logger_) {
            logger_->log(
                LogLevel::Info, "PluginVersionManager",
                "Cleaned up " + std::to_string(cleaned_count) + " old backups");
        }
    }

    return cleaned_count;
}

// === Compatibility Management Implementation ===

CompatibilityLevel PluginVersionManager::check_compatibility(
    std::string_view plugin_id, const Version& version,
    const Version& host_version) const {
    std::shared_lock lock(compatibility_mutex_);

    // Check for custom compatibility rules
    auto it = compatibility_rules_.find(std::string(plugin_id));
    if (it != compatibility_rules_.end()) {
        for (const auto& rule : it->second) {
            if (version >= rule.min_host_version &&
                version <= rule.max_host_version) {
                return rule.level;
            }
        }
    }

    // Default compatibility check based on semantic versioning
    return determine_compatibility_level(version, host_version);
}

std::vector<Version> PluginVersionManager::get_compatible_versions(
    std::string_view plugin_id, const Version& host_version) const {
    std::shared_lock versions_lock(versions_mutex_);
    std::shared_lock compat_lock(compatibility_mutex_);

    std::vector<Version> compatible_versions;

    auto versions_it = installed_versions_.find(std::string(plugin_id));
    if (versions_it == installed_versions_.end()) {
        return compatible_versions;
    }

    for (const auto& version_info : versions_it->second) {
        auto compat_level =
            check_compatibility(plugin_id, version_info.version, host_version);
        if (compat_level != CompatibilityLevel::Breaking) {
            compatible_versions.push_back(version_info.version);
        }
    }

    // Sort by version (newest first)
    std::sort(compatible_versions.begin(), compatible_versions.end(),
              [](const Version& a, const Version& b) { return a > b; });

    return compatible_versions;
}

qtplugin::expected<void, VersionError>
PluginVersionManager::register_compatibility_rules(std::string_view plugin_id,
                                                   const QJsonObject& rules) {
    std::unique_lock lock(compatibility_mutex_);

    try {
        auto& plugin_rules = compatibility_rules_[std::string(plugin_id)];
        plugin_rules.clear();  // Replace existing rules

        if (rules.contains("rules") && rules["rules"].isArray()) {
            QJsonArray rules_array = rules["rules"].toArray();

            for (const auto& rule_value : rules_array) {
                if (!rule_value.isObject())
                    continue;

                QJsonObject rule_obj = rule_value.toObject();
                CompatibilityRule rule;

                // Parse version ranges
                if (rule_obj.contains("min_host_version")) {
                    auto min_ver_str =
                        rule_obj["min_host_version"].toString().toStdString();
                    auto min_ver = Version::parse(min_ver_str);
                    if (min_ver) {
                        rule.min_host_version = *min_ver;
                    }
                }

                if (rule_obj.contains("max_host_version")) {
                    auto max_ver_str =
                        rule_obj["max_host_version"].toString().toStdString();
                    auto max_ver = Version::parse(max_ver_str);
                    if (max_ver) {
                        rule.max_host_version = *max_ver;
                    }
                }

                // Parse compatibility level
                if (rule_obj.contains("level")) {
                    rule.level = static_cast<CompatibilityLevel>(
                        rule_obj["level"].toInt());
                }

                if (rule_obj.contains("metadata")) {
                    rule.metadata = rule_obj["metadata"].toObject();
                }

                plugin_rules.push_back(rule);
            }
        }

        save_compatibility_rules();

        if (logger_) {
            logger_->log(LogLevel::Info, "PluginVersionManager",
                         "Registered " + std::to_string(plugin_rules.size()) +
                             " compatibility rules for plugin " +
                             std::string(plugin_id));
        }

        return {};

    } catch (const std::exception& e) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::StorageError,
            "Failed to register compatibility rules: " + std::string(e.what()),
            std::string(plugin_id)});
    }
}

// === Version Information Implementation ===

qtplugin::expected<PluginVersionInfo, VersionError>
PluginVersionManager::get_version_info(std::string_view plugin_id,
                                       std::optional<Version> version) const {
    std::shared_lock lock(versions_mutex_);

    auto it = installed_versions_.find(std::string(plugin_id));
    if (it == installed_versions_.end()) {
        return qtplugin::unexpected(VersionError{
            VersionErrorCode::VersionNotFound,
            "No versions found for plugin " + std::string(plugin_id),
            std::string(plugin_id)});
    }

    if (!version) {
        // Return active version if no specific version requested
        auto active_it = std::find_if(
            it->second.begin(), it->second.end(),
            [](const PluginVersionInfo& info) { return info.is_active; });

        if (active_it != it->second.end()) {
            return *active_it;
        }

        // If no active version, return latest version
        if (!it->second.empty()) {
            auto latest = std::max_element(
                it->second.begin(), it->second.end(),
                [](const PluginVersionInfo& a, const PluginVersionInfo& b) {
                    return a.version < b.version;
                });
            return *latest;
        }
    } else {
        // Return specific version
        auto version_it =
            std::find_if(it->second.begin(), it->second.end(),
                         [&version](const PluginVersionInfo& info) {
                             return info.version == *version;
                         });

        if (version_it != it->second.end()) {
            return *version_it;
        }
    }

    return qtplugin::unexpected(VersionError{
        VersionErrorCode::VersionNotFound,
        version ? "Version " + version->to_string() + " not found for plugin " +
                      std::string(plugin_id)
                : "No versions available for plugin " + std::string(plugin_id),
        std::string(plugin_id), version});
}

std::vector<PluginVersionInfo> PluginVersionManager::get_version_history(
    std::string_view plugin_id) const {
    std::shared_lock lock(versions_mutex_);

    auto it = installed_versions_.find(std::string(plugin_id));
    if (it != installed_versions_.end()) {
        auto history = it->second;

        // Sort by installation time (newest first)
        std::sort(history.begin(), history.end(),
                  [](const PluginVersionInfo& a, const PluginVersionInfo& b) {
                      return a.install_time > b.install_time;
                  });

        return history;
    }

    return {};
}

// === Storage Management Implementation ===

int PluginVersionManager::cleanup_unused_versions(std::string_view plugin_id,
                                                  int keep_count) {
    std::unique_lock lock(versions_mutex_);

    int cleaned_count = 0;

    if (plugin_id.empty()) {
        // Clean up all plugins
        for (auto& [id, versions] : installed_versions_) {
            cleaned_count += cleanup_plugin_versions(versions, keep_count);
        }
    } else {
        // Clean up specific plugin
        auto it = installed_versions_.find(std::string(plugin_id));
        if (it != installed_versions_.end()) {
            cleaned_count = cleanup_plugin_versions(it->second, keep_count);
        }
    }

    if (cleaned_count > 0) {
        save_version_database();

        if (logger_) {
            logger_->log(LogLevel::Info, "PluginVersionManager",
                         "Cleaned up " + std::to_string(cleaned_count) +
                             " unused versions");
        }
    }

    return cleaned_count;
}

QJsonObject PluginVersionManager::get_storage_usage(
    std::string_view plugin_id) const {
    std::shared_lock lock(versions_mutex_);

    QJsonObject usage;

    try {
        if (plugin_id.empty()) {
            // Calculate total storage usage
            uintmax_t total_size = 0;
            int total_versions = 0;

            for (const auto& [id, versions] : installed_versions_) {
                for (const auto& version_info : versions) {
                    if (std::filesystem::exists(
                            version_info.installation_path)) {
                        for (const auto& entry :
                             std::filesystem::recursive_directory_iterator(
                                 version_info.installation_path)) {
                            if (entry.is_regular_file()) {
                                total_size += entry.file_size();
                            }
                        }
                    }
                    total_versions++;
                }
            }

            usage["total_size_bytes"] = static_cast<qint64>(total_size);
            usage["total_versions"] = total_versions;
            usage["total_plugins"] =
                static_cast<int>(installed_versions_.size());

        } else {
            // Calculate storage usage for specific plugin
            auto it = installed_versions_.find(std::string(plugin_id));
            if (it != installed_versions_.end()) {
                uintmax_t plugin_size = 0;

                for (const auto& version_info : it->second) {
                    if (std::filesystem::exists(
                            version_info.installation_path)) {
                        for (const auto& entry :
                             std::filesystem::recursive_directory_iterator(
                                 version_info.installation_path)) {
                            if (entry.is_regular_file()) {
                                plugin_size += entry.file_size();
                            }
                        }
                    }
                }

                usage["plugin_id"] =
                    QString::fromStdString(std::string(plugin_id));
                usage["size_bytes"] = static_cast<qint64>(plugin_size);
                usage["version_count"] = static_cast<int>(it->second.size());
            }
        }

    } catch (const std::filesystem::filesystem_error& e) {
        usage["error"] = QString::fromStdString(e.what());
    }

    return usage;
}

// === Helper Methods Implementation ===

int PluginVersionManager::cleanup_plugin_versions(
    std::vector<PluginVersionInfo>& versions, int keep_count) {
    int cleaned_count = 0;

    if (static_cast<int>(versions.size()) <= keep_count) {
        return 0;  // Nothing to clean up
    }

    // Sort by last used time (newest first), but keep active versions
    std::sort(versions.begin(), versions.end(),
              [](const PluginVersionInfo& a, const PluginVersionInfo& b) {
                  // Active versions always come first
                  if (a.is_active != b.is_active) {
                      return a.is_active > b.is_active;
                  }
                  // Then sort by last used time
                  return a.last_used > b.last_used;
              });

    // Count active versions
    int active_count = 0;
    for (const auto& version : versions) {
        if (version.is_active) {
            active_count++;
        }
    }

    // Calculate how many to remove (never remove active versions)
    int total_to_keep = std::max(keep_count, active_count);
    if (static_cast<int>(versions.size()) <= total_to_keep) {
        return 0;
    }

    int to_remove = static_cast<int>(versions.size()) - total_to_keep;

    // Remove oldest unused versions
    for (int i = 0; i < to_remove; ++i) {
        auto& version_to_remove = versions[versions.size() - 1 - i];

        if (!version_to_remove.is_active) {
            try {
                std::filesystem::remove_all(
                    version_to_remove.installation_path);
                cleaned_count++;
            } catch (const std::filesystem::filesystem_error&) {
                // Continue with other versions
            }
        }
    }

    // Remove from vector
    versions.erase(versions.end() - to_remove, versions.end());

    return cleaned_count;
}

}  // namespace qtplugin

#include "plugin_version_manager.moc"
