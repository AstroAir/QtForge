# Permission Manager

The Permission Manager handles access control and permission management for plugins and system resources.

## Overview

The Permission Manager provides:
- **Access Control**: Manage plugin permissions and access rights
- **Resource Protection**: Control access to system resources
- **Permission Inheritance**: Support for hierarchical permissions
- **Dynamic Permissions**: Runtime permission modification
- **Audit Logging**: Track permission usage and violations

## Class Reference

### PermissionManager

```cpp
class PermissionManager {
public:
    // Permission management
    bool grantPermission(const QString& pluginId, const Permission& permission);
    bool revokePermission(const QString& pluginId, const Permission& permission);
    bool hasPermission(const QString& pluginId, const Permission& permission) const;
    
    // Permission queries
    QList<Permission> getPermissions(const QString& pluginId) const;
    QList<QString> getPluginsWithPermission(const Permission& permission) const;
    
    // Permission groups
    bool createPermissionGroup(const QString& groupName, const QList<Permission>& permissions);
    bool assignToGroup(const QString& pluginId, const QString& groupName);
    bool removeFromGroup(const QString& pluginId, const QString& groupName);
    
    // Access control
    bool checkAccess(const QString& pluginId, const QString& resource) const;
    bool requestAccess(const QString& pluginId, const QString& resource);
    
    // Configuration
    void setStrictMode(bool enabled);
    void setDefaultPermissions(const QList<Permission>& permissions);
    
signals:
    void permissionGranted(const QString& pluginId, const Permission& permission);
    void permissionRevoked(const QString& pluginId, const Permission& permission);
    void accessDenied(const QString& pluginId, const QString& resource);
    void accessGranted(const QString& pluginId, const QString& resource);
};
```

### Permission

```cpp
class Permission {
public:
    enum Type {
        FileSystem,
        Network,
        SystemInfo,
        UserInterface,
        Database,
        Configuration,
        PluginManagement,
        Custom
    };
    
    Permission(Type type, const QString& resource = QString());
    Permission(const QString& customPermission);
    
    Type getType() const;
    QString getResource() const;
    QString toString() const;
    
    bool operator==(const Permission& other) const;
    bool operator<(const Permission& other) const;
};
```

## Usage Examples

### Granting Permissions

```cpp
PermissionManager* manager = PermissionManager::instance();

// Grant file system access
Permission fileAccess(Permission::FileSystem, "/tmp/plugin_data");
manager->grantPermission("com.example.plugin", fileAccess);

// Grant network access
Permission networkAccess(Permission::Network, "api.example.com");
manager->grantPermission("com.example.plugin", networkAccess);
```

### Checking Permissions

```cpp
QString pluginId = "com.example.plugin";
Permission fileAccess(Permission::FileSystem, "/tmp/plugin_data");

if (manager->hasPermission(pluginId, fileAccess)) {
    // Plugin has permission to access the file system
    qDebug() << "File access granted";
} else {
    qWarning() << "File access denied";
}
```

### Permission Groups

```cpp
// Create a permission group
QList<Permission> webPermissions = {
    Permission(Permission::Network, "*.example.com"),
    Permission(Permission::FileSystem, "/tmp/web_cache")
};

manager->createPermissionGroup("web_plugins", webPermissions);

// Assign plugin to group
manager->assignToGroup("com.example.webplugin", "web_plugins");
```

### Runtime Permission Requests

```cpp
// Plugin requests permission at runtime
bool granted = manager->requestAccess("com.example.plugin", "/home/user/documents");

if (granted) {
    // Access granted, proceed with operation
    qDebug() << "Access granted to documents folder";
} else {
    // Access denied, handle gracefully
    qWarning() << "Access denied to documents folder";
}
```

## Built-in Permissions

### File System Permissions
- **Read**: Read access to files and directories
- **Write**: Write access to files and directories
- **Execute**: Execute files and scripts

### Network Permissions
- **Connect**: Outbound network connections
- **Listen**: Inbound network connections
- **Proxy**: Use system proxy settings

### System Permissions
- **SystemInfo**: Access system information
- **Environment**: Access environment variables
- **Process**: Start and manage processes

### UI Permissions
- **ShowDialogs**: Display dialog boxes
- **SystemTray**: Access system tray
- **Notifications**: Show system notifications

## Security Policies

### Strict Mode
When strict mode is enabled, plugins must explicitly request all permissions:

```cpp
manager->setStrictMode(true);
```

### Default Permissions
Set default permissions for new plugins:

```cpp
QList<Permission> defaults = {
    Permission(Permission::FileSystem, "/tmp"),
    Permission(Permission::SystemInfo)
};
manager->setDefaultPermissions(defaults);
```

## Audit and Logging

The Permission Manager automatically logs all permission-related activities:

```cpp
connect(manager, &PermissionManager::accessDenied,
        [](const QString& pluginId, const QString& resource) {
    qWarning() << "Access denied:" << pluginId << "to" << resource;
});
```

## Thread Safety

The Permission Manager is thread-safe and can be accessed from multiple threads simultaneously.

## See Also

- [Security Manager](security-manager.md)
- [Plugin Validator](plugin-validator.md)
- [Threat Detector](threat-detector.md)
