# Plugin Registry

The Plugin Registry is a central component that manages the registration, discovery, and lifecycle of plugins within the QtForge system.

## Overview

The Plugin Registry provides:
- **Plugin Registration**: Register and unregister plugins
- **Plugin Discovery**: Find and enumerate available plugins
- **Metadata Management**: Store and retrieve plugin metadata
- **Dependency Resolution**: Manage plugin dependencies
- **Lifecycle Tracking**: Monitor plugin states and transitions

## Class Reference

### PluginRegistry

```cpp
class PluginRegistry {
public:
    // Registration methods
    bool registerPlugin(const PluginMetadata& metadata);
    bool unregisterPlugin(const QString& pluginId);
    
    // Discovery methods
    QList<PluginMetadata> getAvailablePlugins() const;
    QList<PluginMetadata> getRegisteredPlugins() const;
    PluginMetadata getPluginMetadata(const QString& pluginId) const;
    
    // Query methods
    bool isPluginRegistered(const QString& pluginId) const;
    bool isPluginLoaded(const QString& pluginId) const;
    QStringList getDependencies(const QString& pluginId) const;
    QStringList getDependents(const QString& pluginId) const;
    
    // State management
    PluginState getPluginState(const QString& pluginId) const;
    void setPluginState(const QString& pluginId, PluginState state);
    
signals:
    void pluginRegistered(const QString& pluginId);
    void pluginUnregistered(const QString& pluginId);
    void pluginStateChanged(const QString& pluginId, PluginState oldState, PluginState newState);
};
```

## Usage Examples

### Registering a Plugin

```cpp
PluginMetadata metadata;
metadata.id = "com.example.myplugin";
metadata.name = "My Plugin";
metadata.version = "1.0.0";
metadata.description = "Example plugin";

PluginRegistry* registry = PluginManager::instance()->getRegistry();
if (registry->registerPlugin(metadata)) {
    qDebug() << "Plugin registered successfully";
}
```

### Discovering Plugins

```cpp
PluginRegistry* registry = PluginManager::instance()->getRegistry();
QList<PluginMetadata> plugins = registry->getAvailablePlugins();

for (const auto& plugin : plugins) {
    qDebug() << "Found plugin:" << plugin.name << "(" << plugin.id << ")";
}
```

### Checking Dependencies

```cpp
QString pluginId = "com.example.myplugin";
QStringList dependencies = registry->getDependencies(pluginId);

for (const QString& dep : dependencies) {
    if (!registry->isPluginLoaded(dep)) {
        qWarning() << "Dependency not loaded:" << dep;
    }
}
```

## Plugin States

The registry tracks the following plugin states:

- **Unknown**: Plugin state is not known
- **Registered**: Plugin is registered but not loaded
- **Loading**: Plugin is currently being loaded
- **Loaded**: Plugin is successfully loaded and active
- **Unloading**: Plugin is currently being unloaded
- **Error**: Plugin encountered an error

## Thread Safety

The Plugin Registry is thread-safe and can be accessed from multiple threads simultaneously. All operations are protected by internal synchronization mechanisms.

## See Also

- [Plugin Manager](plugin-manager.md)
- [Plugin Loader](plugin-loader.md)
- [Plugin Interface](plugin-interface.md)
