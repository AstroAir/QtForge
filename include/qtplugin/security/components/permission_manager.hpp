#pragma once

#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <memory>
#include <set>
#include <unordered_map>

namespace qtplugin {
namespace security {
namespace components {

/**
 * @brief Permission manager for plugin access control
 *
 * This is a stub implementation to satisfy test compilation requirements.
 * The actual permission manager would implement comprehensive permission
 * management including role-based access control, resource permissions,
 * and security policy enforcement.
 */
class PermissionManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Permission types
     */
    enum class Permission {
        FileRead,
        FileWrite,
        NetworkAccess,
        SystemCall,
        DatabaseRead,
        DatabaseWrite,
        ConfigurationAccess
    };

    explicit PermissionManager(QObject* parent = nullptr);
    ~PermissionManager() override = default;

    /**
     * @brief Initialize the permission manager
     * @return True if initialization succeeded
     */
    bool initialize();

    /**
     * @brief Grant permission to a plugin
     * @param pluginId Plugin identifier
     * @param permission Permission to grant
     * @param resource Optional resource specification
     * @return True if permission was granted
     */
    bool grantPermission(const QString& pluginId, Permission permission,
                         const QString& resource = {});

    /**
     * @brief Revoke permission from a plugin
     * @param pluginId Plugin identifier
     * @param permission Permission to revoke
     * @param resource Optional resource specification
     * @return True if permission was revoked
     */
    bool revokePermission(const QString& pluginId, Permission permission,
                          const QString& resource = {});

    /**
     * @brief Check if plugin has permission
     * @param pluginId Plugin identifier
     * @param permission Permission to check
     * @param resource Optional resource specification
     * @return True if plugin has permission
     */
    bool hasPermission(const QString& pluginId, Permission permission,
                       const QString& resource = {}) const;

    /**
     * @brief Get all permissions for a plugin
     * @param pluginId Plugin identifier
     * @return List of permissions
     */
    QStringList getPluginPermissions(const QString& pluginId) const;

    /**
     * @brief Clear all permissions for a plugin
     * @param pluginId Plugin identifier
     */
    void clearPluginPermissions(const QString& pluginId);

    /**
     * @brief Convert permission enum to string
     * @param permission Permission enum value
     * @return String representation
     */
    static QString permissionToString(Permission permission);

    /**
     * @brief Convert string to permission enum
     * @param permissionStr String representation
     * @return Permission enum value or nullopt if invalid
     */
    static std::optional<Permission> stringToPermission(
        const QString& permissionStr);

private:
    struct PermissionEntry {
        Permission permission;
        QString resource;

        bool operator<(const PermissionEntry& other) const {
            if (permission != other.permission) {
                return static_cast<int>(permission) <
                       static_cast<int>(other.permission);
            }
            return resource < other.resource;
        }
    };

    std::unordered_map<QString, std::set<PermissionEntry>> m_pluginPermissions;
    mutable QMutex m_mutex;
};

}  // namespace components
}  // namespace security
}  // namespace qtplugin
