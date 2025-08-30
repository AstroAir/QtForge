/**
 * @file permission_manager.cpp
 * @brief Implementation of permission manager
 * @version 3.0.0
 */

#include "../../../include/qtplugin/security/components/permission_manager.hpp"
#include <QDebug>
#include <QLoggingCategory>
#include <algorithm>
#include <mutex>
#include "../../../include/qtplugin/security/security_manager.hpp"

Q_LOGGING_CATEGORY(permissionManagerLog, "qtplugin.security.permissions")

namespace qtplugin {

PermissionManager::PermissionManager(QObject* parent) : QObject(parent) {
    qCDebug(permissionManagerLog) << "Permission manager initialized";
}

PermissionManager::~PermissionManager() {
    qCDebug(permissionManagerLog) << "Permission manager destroyed";
}

SecurityValidationResult PermissionManager::validate_permissions(
    const std::filesystem::path& file_path) const {
    SecurityValidationResult result;
    result.is_valid = false;
    result.validated_level = SecurityLevel::None;

    try {
        // Parse permissions from plugin metadata
        auto requested_permissions = parse_plugin_permissions(file_path);

        // Check each requested permission
        for (auto permission : requested_permissions) {
            // For strict security level, check if permission is allowed
            if (!is_permission_allowed(permission, SecurityLevel::Strict)) {
                result.errors.push_back("Permission not allowed: " +
                                        permission_to_string(permission));
                return result;
            }

            // Add warning for potentially dangerous permissions
            if (permission == PluginPermission::FileSystemWrite ||
                permission == PluginPermission::ProcessCreation ||
                permission == PluginPermission::RegistryAccess) {
                result.warnings.push_back(
                    "Plugin requests potentially dangerous permission: " +
                    permission_to_string(permission));
            }
        }

        result.is_valid = true;
        result.validated_level = SecurityLevel::Strict;

    } catch (const std::exception& e) {
        result.errors.push_back("Exception during permission validation: " +
                                std::string(e.what()));
    } catch (...) {
        result.errors.push_back(
            "Unknown exception during permission validation");
    }

    return result;
}

bool PermissionManager::has_permission(const std::string& plugin_id,
                                       PluginPermission permission) const {
    std::shared_lock lock(m_permissions_mutex);

    auto it = m_plugin_permissions.find(plugin_id);
    if (it == m_plugin_permissions.end()) {
        return false;
    }

    return it->second.find(permission) != it->second.end();
}

qtplugin::expected<void, PluginError> PermissionManager::grant_permission(
    const std::string& plugin_id, PluginPermission permission) {
    if (plugin_id.empty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Plugin ID cannot be empty");
    }

    std::unique_lock lock(m_permissions_mutex);

    m_plugin_permissions[plugin_id].insert(permission);

    lock.unlock();

    qCDebug(permissionManagerLog)
        << "Permission granted to plugin" << QString::fromStdString(plugin_id)
        << ":" << permission_to_string(permission).c_str();

    emit permission_granted(QString::fromStdString(plugin_id),
                            static_cast<int>(permission));

    return make_success();
}

void PermissionManager::revoke_permission(const std::string& plugin_id,
                                          PluginPermission permission) {
    std::unique_lock lock(m_permissions_mutex);

    auto it = m_plugin_permissions.find(plugin_id);
    if (it != m_plugin_permissions.end()) {
        it->second.erase(permission);

        // Remove plugin entry if no permissions left
        if (it->second.empty()) {
            m_plugin_permissions.erase(it);
        }
    }

    lock.unlock();

    qCDebug(permissionManagerLog)
        << "Permission revoked from plugin" << QString::fromStdString(plugin_id)
        << ":" << permission_to_string(permission).c_str();

    emit permission_revoked(QString::fromStdString(plugin_id),
                            static_cast<int>(permission));
}

std::unordered_set<PluginPermission> PermissionManager::get_permissions(
    const std::string& plugin_id) const {
    std::shared_lock lock(m_permissions_mutex);

    auto it = m_plugin_permissions.find(plugin_id);
    if (it != m_plugin_permissions.end()) {
        return it->second;
    }

    return {};
}

void PermissionManager::clear_permissions(const std::string& plugin_id) {
    std::unique_lock lock(m_permissions_mutex);

    auto it = m_plugin_permissions.find(plugin_id);
    if (it != m_plugin_permissions.end()) {
        size_t count = it->second.size();
        m_plugin_permissions.erase(it);

        qCDebug(permissionManagerLog)
            << "Cleared" << count << "permissions for plugin"
            << QString::fromStdString(plugin_id);
    }
}

std::unordered_set<PluginPermission>
PermissionManager::get_required_permissions(SecurityLevel level) const {
    std::unordered_set<PluginPermission> required;

    switch (level) {
        case SecurityLevel::None:
            // No restrictions
            break;

        case SecurityLevel::Basic:
            required.insert(PluginPermission::SystemInfo);
            break;

        case SecurityLevel::Standard:
            required.insert(PluginPermission::SystemInfo);
            required.insert(PluginPermission::FileSystemRead);
            break;

        case SecurityLevel::Strict:
            required.insert(PluginPermission::SystemInfo);
            required.insert(PluginPermission::FileSystemRead);
            required.insert(PluginPermission::FileSystemWrite);
            break;

        case SecurityLevel::Maximum:
            // All permissions required for maximum security
            required.insert(PluginPermission::FileSystemRead);
            required.insert(PluginPermission::FileSystemWrite);
            required.insert(PluginPermission::NetworkAccess);
            required.insert(PluginPermission::RegistryAccess);
            required.insert(PluginPermission::ProcessCreation);
            required.insert(PluginPermission::SystemInfo);
            required.insert(PluginPermission::HardwareAccess);
            required.insert(PluginPermission::DatabaseAccess);
            break;
    }

    return required;
}

std::vector<PluginPermission> PermissionManager::parse_plugin_permissions(
    const std::filesystem::path& file_path) const {
    // This is a simplified implementation
    // In a real system, you would parse the plugin's manifest or metadata
    // to extract the requested permissions

    Q_UNUSED(file_path);

    // For demonstration, return a default set of permissions
    return {PluginPermission::SystemInfo, PluginPermission::FileSystemRead};
}

bool PermissionManager::is_permission_allowed(PluginPermission permission,
                                              SecurityLevel level) const {
    // Define which permissions are allowed at each security level
    switch (level) {
        case SecurityLevel::None:
            return true;  // All permissions allowed

        case SecurityLevel::Basic:
            return permission == PluginPermission::SystemInfo;

        case SecurityLevel::Standard:
            return permission == PluginPermission::SystemInfo ||
                   permission == PluginPermission::FileSystemRead;

        case SecurityLevel::Strict:
            return permission != PluginPermission::ProcessCreation &&
                   permission != PluginPermission::RegistryAccess;

        case SecurityLevel::Maximum:
            return false;  // No permissions allowed by default
    }

    return false;
}

std::string PermissionManager::permission_to_string(
    PluginPermission permission) const {
    switch (permission) {
        case PluginPermission::FileSystemRead:
            return "FileSystemRead";
        case PluginPermission::FileSystemWrite:
            return "FileSystemWrite";
        case PluginPermission::NetworkAccess:
            return "NetworkAccess";
        case PluginPermission::RegistryAccess:
            return "RegistryAccess";
        case PluginPermission::ProcessCreation:
            return "ProcessCreation";
        case PluginPermission::SystemInfo:
            return "SystemInfo";
        case PluginPermission::HardwareAccess:
            return "HardwareAccess";
        case PluginPermission::DatabaseAccess:
            return "DatabaseAccess";
        default:
            return "Unknown";
    }
}

SecurityValidationResult PermissionManager::create_permission_result(
    bool is_valid, const std::vector<std::string>& errors,
    const std::vector<std::string>& warnings) const {
    SecurityValidationResult result;
    result.is_valid = is_valid;
    result.errors = errors;
    result.warnings = warnings;
    result.validated_level =
        is_valid ? SecurityLevel::Strict : SecurityLevel::None;
    return result;
}

}  // namespace qtplugin


