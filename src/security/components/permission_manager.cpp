#include "qtplugin/security/components/permission_manager.hpp"
#include <QtCore/QDebug>
#include <QtCore/QMutexLocker>

namespace qtplugin {
namespace security {
namespace components {

PermissionManager::PermissionManager(QObject* parent) : QObject(parent) {}

bool PermissionManager::initialize() {
    qDebug() << "PermissionManager initialized";
    return true;
}

bool PermissionManager::grantPermission(const QString& pluginId,
                                        Permission permission,
                                        const QString& resource) {
    QMutexLocker locker(&m_mutex);

    PermissionEntry entry{permission, resource};
    m_pluginPermissions[pluginId].insert(entry);

    qDebug() << "Granted permission" << permissionToString(permission)
             << "to plugin" << pluginId;
    return true;
}

bool PermissionManager::revokePermission(const QString& pluginId,
                                         Permission permission,
                                         const QString& resource) {
    QMutexLocker locker(&m_mutex);

    auto it = m_pluginPermissions.find(pluginId);
    if (it == m_pluginPermissions.end()) {
        return false;
    }

    PermissionEntry entry{permission, resource};
    bool removed = it->second.erase(entry) > 0;

    if (removed) {
        qDebug() << "Revoked permission" << permissionToString(permission)
                 << "from plugin" << pluginId;
    }

    return removed;
}

bool PermissionManager::hasPermission(const QString& pluginId,
                                      Permission permission,
                                      const QString& resource) const {
    QMutexLocker locker(&m_mutex);

    auto it = m_pluginPermissions.find(pluginId);
    if (it == m_pluginPermissions.end()) {
        return false;
    }

    PermissionEntry entry{permission, resource};
    return it->second.find(entry) != it->second.end();
}

QStringList PermissionManager::getPluginPermissions(
    const QString& pluginId) const {
    QMutexLocker locker(&m_mutex);

    QStringList permissions;
    auto it = m_pluginPermissions.find(pluginId);
    if (it != m_pluginPermissions.end()) {
        for (const auto& entry : it->second) {
            QString permStr = permissionToString(entry.permission);
            if (!entry.resource.isEmpty()) {
                permStr += ":" + entry.resource;
            }
            permissions.append(permStr);
        }
    }

    return permissions;
}

void PermissionManager::clearPluginPermissions(const QString& pluginId) {
    QMutexLocker locker(&m_mutex);

    m_pluginPermissions.erase(pluginId);
    qDebug() << "Cleared all permissions for plugin" << pluginId;
}

QString PermissionManager::permissionToString(Permission permission) {
    switch (permission) {
        case Permission::FileRead:
            return "FileRead";
        case Permission::FileWrite:
            return "FileWrite";
        case Permission::NetworkAccess:
            return "NetworkAccess";
        case Permission::SystemCall:
            return "SystemCall";
        case Permission::DatabaseRead:
            return "DatabaseRead";
        case Permission::DatabaseWrite:
            return "DatabaseWrite";
        case Permission::ConfigurationAccess:
            return "ConfigurationAccess";
        default:
            return "Unknown";
    }
}

std::optional<PermissionManager::Permission>
PermissionManager::stringToPermission(const QString& permissionStr) {
    if (permissionStr == "FileRead")
        return Permission::FileRead;
    if (permissionStr == "FileWrite")
        return Permission::FileWrite;
    if (permissionStr == "NetworkAccess")
        return Permission::NetworkAccess;
    if (permissionStr == "SystemCall")
        return Permission::SystemCall;
    if (permissionStr == "DatabaseRead")
        return Permission::DatabaseRead;
    if (permissionStr == "DatabaseWrite")
        return Permission::DatabaseWrite;
    if (permissionStr == "ConfigurationAccess")
        return Permission::ConfigurationAccess;
    return std::nullopt;
}

}  // namespace components
}  // namespace security
}  // namespace qtplugin
