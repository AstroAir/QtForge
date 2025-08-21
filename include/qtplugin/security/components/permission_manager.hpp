/**
 * @file permission_manager.hpp
 * @brief Permission manager interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <filesystem>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../../utils/error_handling.hpp"

namespace qtplugin {

// Forward declarations
struct SecurityValidationResult;
enum class SecurityLevel;

/**
 * @brief Permission types for plugins
 */
enum class PluginPermission {
    FileSystemRead,
    FileSystemWrite,
    NetworkAccess,
    RegistryAccess,
    ProcessCreation,
    SystemInfo,
    HardwareAccess,
    DatabaseAccess
};

/**
 * @brief Interface for permission management
 *
 * The permission manager handles access control, permission validation,
 * and enforcement of security policies for plugins.
 */
class IPermissionManager {
public:
    virtual ~IPermissionManager() = default;

    /**
     * @brief Validate plugin permissions
     * @param file_path Path to plugin file
     * @return Validation result
     */
    virtual SecurityValidationResult validate_permissions(
        const std::filesystem::path& file_path) const = 0;

    /**
     * @brief Check if plugin has specific permission
     * @param plugin_id Plugin identifier
     * @param permission Permission to check
     * @return true if permission is granted
     */
    virtual bool has_permission(const std::string& plugin_id,
                                PluginPermission permission) const = 0;

    /**
     * @brief Grant permission to plugin
     * @param plugin_id Plugin identifier
     * @param permission Permission to grant
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> grant_permission(
        const std::string& plugin_id, PluginPermission permission) = 0;

    /**
     * @brief Revoke permission from plugin
     * @param plugin_id Plugin identifier
     * @param permission Permission to revoke
     */
    virtual void revoke_permission(const std::string& plugin_id,
                                   PluginPermission permission) = 0;

    /**
     * @brief Get all permissions for plugin
     * @param plugin_id Plugin identifier
     * @return Set of granted permissions
     */
    virtual std::unordered_set<PluginPermission> get_permissions(
        const std::string& plugin_id) const = 0;

    /**
     * @brief Clear all permissions for plugin
     * @param plugin_id Plugin identifier
     */
    virtual void clear_permissions(const std::string& plugin_id) = 0;

    /**
     * @brief Get required permissions for security level
     * @param level Security level
     * @return Set of required permissions
     */
    virtual std::unordered_set<PluginPermission> get_required_permissions(
        SecurityLevel level) const = 0;
};

/**
 * @brief Permission manager implementation
 *
 * Manages plugin permissions and access control policies.
 * Provides thread-safe permission management and validation.
 */
class PermissionManager : public QObject, public IPermissionManager {
    Q_OBJECT

public:
    explicit PermissionManager(QObject* parent = nullptr);
    ~PermissionManager() override;

    // IPermissionManager interface
    SecurityValidationResult validate_permissions(
        const std::filesystem::path& file_path) const override;
    bool has_permission(const std::string& plugin_id,
                        PluginPermission permission) const override;
    qtplugin::expected<void, PluginError> grant_permission(
        const std::string& plugin_id, PluginPermission permission) override;
    void revoke_permission(const std::string& plugin_id,
                           PluginPermission permission) override;
    std::unordered_set<PluginPermission> get_permissions(
        const std::string& plugin_id) const override;
    void clear_permissions(const std::string& plugin_id) override;
    std::unordered_set<PluginPermission> get_required_permissions(
        SecurityLevel level) const override;

signals:
    /**
     * @brief Emitted when permission is granted
     * @param plugin_id Plugin identifier
     * @param permission Permission that was granted
     */
    void permission_granted(const QString& plugin_id, int permission);

    /**
     * @brief Emitted when permission is revoked
     * @param plugin_id Plugin identifier
     * @param permission Permission that was revoked
     */
    void permission_revoked(const QString& plugin_id, int permission);

    /**
     * @brief Emitted when permission violation is detected
     * @param plugin_id Plugin identifier
     * @param permission Permission that was violated
     */
    void permission_violation(const QString& plugin_id, int permission);

private:
    mutable std::shared_mutex m_permissions_mutex;
    std::unordered_map<std::string, std::unordered_set<PluginPermission>>
        m_plugin_permissions;

    // Helper methods
    std::vector<PluginPermission> parse_plugin_permissions(
        const std::filesystem::path& file_path) const;
    bool is_permission_allowed(PluginPermission permission,
                               SecurityLevel level) const;
    std::string permission_to_string(PluginPermission permission) const;
    SecurityValidationResult create_permission_result(
        bool is_valid, const std::vector<std::string>& errors = {},
        const std::vector<std::string>& warnings = {}) const;
};

}  // namespace qtplugin
