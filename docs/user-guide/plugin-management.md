# Plugin Management

This guide covers how to manage plugins in QtForge applications, including installation, configuration, and lifecycle management.

## Overview

The QtForge plugin system provides comprehensive tools for managing plugins throughout their lifecycle. This includes:

- Plugin discovery and loading
- Configuration management
- Runtime control (enable/disable)
- Dependency resolution
- Version management

## Plugin Discovery

QtForge automatically discovers plugins in several locations:

### Default Plugin Directories

```cpp
// System-wide plugins
/usr/lib/qtforge/plugins/          // Linux
C:\Program Files\QtForge\plugins\  // Windows
/Applications/QtForge/plugins/     // macOS

// User-specific plugins
~/.qtforge/plugins/                // Linux/macOS
%APPDATA%\QtForge\plugins\         // Windows

// Application-specific plugins
./plugins/                         // Relative to executable
```

### Custom Plugin Directories

You can specify additional plugin directories:

```cpp
#include <QtForge/PluginManager>

auto* manager = QtForge::PluginManager::instance();
manager->addPluginDirectory("/custom/plugin/path");
```

## Plugin Installation

### Manual Installation

1. Copy plugin files to a plugin directory
2. Ensure all dependencies are available
3. Restart the application or reload plugins

### Programmatic Installation

```cpp
#include <QtForge/PluginManager>

auto* manager = QtForge::PluginManager::instance();
bool success = manager->installPlugin("/path/to/plugin.so");
if (success) {
    qDebug() << "Plugin installed successfully";
}
```

## Plugin Configuration

### Configuration Files

Plugins can be configured using JSON configuration files:

```json
{
    "plugin_id": "com.example.myplugin",
    "enabled": true,
    "settings": {
        "option1": "value1",
        "option2": 42
    },
    "dependencies": [
        "com.example.dependency1",
        "com.example.dependency2"
    ]
}
```

### Runtime Configuration

```cpp
#include <QtForge/PluginManager>

auto* manager = QtForge::PluginManager::instance();
auto* plugin = manager->getPlugin("com.example.myplugin");
if (plugin) {
    plugin->configure({
        {"option1", "new_value"},
        {"option2", 100}
    });
}
```

## Plugin Lifecycle Management

### Loading Plugins

```cpp
// Load all discovered plugins
manager->loadAllPlugins();

// Load specific plugin
manager->loadPlugin("com.example.myplugin");
```

### Enabling/Disabling Plugins

```cpp
// Enable plugin
manager->enablePlugin("com.example.myplugin");

// Disable plugin
manager->disablePlugin("com.example.myplugin");

// Check if plugin is enabled
bool enabled = manager->isPluginEnabled("com.example.myplugin");
```

### Unloading Plugins

```cpp
// Unload specific plugin
manager->unloadPlugin("com.example.myplugin");

// Unload all plugins
manager->unloadAllPlugins();
```

## Dependency Management

QtForge automatically handles plugin dependencies:

### Dependency Resolution

- Automatic loading of required dependencies
- Circular dependency detection
- Version compatibility checking
- Graceful handling of missing dependencies

### Dependency Configuration

```json
{
    "dependencies": [
        {
            "id": "com.example.core",
            "version": ">=1.0.0",
            "required": true
        },
        {
            "id": "com.example.optional",
            "version": "^2.0.0",
            "required": false
        }
    ]
}
```

## Plugin Information

### Querying Plugin Information

```cpp
#include <QtForge/PluginInfo>

auto plugins = manager->getAvailablePlugins();
for (const auto& info : plugins) {
    qDebug() << "Plugin:" << info.id();
    qDebug() << "Version:" << info.version();
    qDebug() << "Description:" << info.description();
    qDebug() << "Enabled:" << info.isEnabled();
}
```

### Plugin Metadata

Each plugin provides metadata including:

- Unique identifier
- Version information
- Description and author
- Dependencies
- Capabilities and interfaces

## Error Handling

### Common Issues

1. **Plugin Not Found**: Check plugin directories and file permissions
2. **Dependency Missing**: Ensure all required dependencies are installed
3. **Version Conflicts**: Update plugins to compatible versions
4. **Loading Failures**: Check plugin compatibility and system requirements

### Error Reporting

```cpp
connect(manager, &PluginManager::pluginLoadFailed,
        [](const QString& pluginId, const QString& error) {
    qWarning() << "Failed to load plugin" << pluginId << ":" << error;
});
```

## Best Practices

1. **Always check return values** when loading/unloading plugins
2. **Handle plugin failures gracefully** in your application
3. **Use dependency injection** to decouple your code from specific plugins
4. **Implement proper error handling** for plugin operations
5. **Test plugin combinations** to ensure compatibility

## See Also

- [Configuration Guide](configuration.md)
- [Security Guidelines](security.md)
- [Troubleshooting](troubleshooting.md)
- [Plugin Development Guide](../developer-guide/plugin-development.md)
