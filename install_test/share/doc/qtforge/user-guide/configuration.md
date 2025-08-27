# Configuration Guide

This guide covers how to configure QtForge applications and plugins for optimal performance and functionality.

## Overview

QtForge provides a flexible configuration system that supports:

- Application-wide settings
- Plugin-specific configuration
- Environment-based configuration
- Runtime configuration changes
- Configuration validation

## Configuration Files

### Main Configuration File

The main application configuration is typically stored in `qtforge.json`:

```json
{
    "application": {
        "name": "MyApp",
        "version": "1.0.0",
        "log_level": "info"
    },
    "plugin_system": {
        "plugin_directories": [
            "./plugins",
            "~/.qtforge/plugins"
        ],
        "auto_load": true,
        "dependency_resolution": true
    },
    "security": {
        "enable_signature_verification": true,
        "trusted_publishers": [
            "com.example.trusted"
        ]
    }
}
```

### Plugin Configuration

Individual plugins can have their own configuration files:

```json
{
    "plugin_id": "com.example.myplugin",
    "enabled": true,
    "settings": {
        "server_url": "https://api.example.com",
        "timeout": 30000,
        "retry_count": 3,
        "features": {
            "feature_a": true,
            "feature_b": false
        }
    }
}
```

## Configuration Locations

### Search Order

QtForge searches for configuration files in the following order:

1. Command line arguments
2. Environment variables
3. Current directory (`./qtforge.json`)
4. User configuration directory
   - Linux/macOS: `~/.config/qtforge/qtforge.json`
   - Windows: `%APPDATA%\QtForge\qtforge.json`
5. System configuration directory
   - Linux: `/etc/qtforge/qtforge.json`
   - Windows: `C:\ProgramData\QtForge\qtforge.json`
   - macOS: `/Library/Application Support/QtForge/qtforge.json`

### Environment Variables

Configuration can be overridden using environment variables:

```bash
# Set log level
export QTFORGE_LOG_LEVEL=debug

# Set plugin directory
export QTFORGE_PLUGIN_DIR=/custom/plugin/path

# Disable auto-loading
export QTFORGE_AUTO_LOAD=false
```

## Programmatic Configuration

### Loading Configuration

```cpp
#include <QtForge/ConfigurationManager>

auto* config = QtForge::ConfigurationManager::instance();

// Load from default locations
config->load();

// Load from specific file
config->loadFromFile("/path/to/config.json");

// Load from JSON string
config->loadFromJson(jsonString);
```

### Accessing Configuration Values

```cpp
// Get simple values
QString appName = config->value("application.name").toString();
int timeout = config->value("plugin.timeout", 5000).toInt();

// Get complex objects
QJsonObject security = config->object("security");
QStringList directories = config->stringList("plugin_system.plugin_directories");
```

### Setting Configuration Values

```cpp
// Set simple values
config->setValue("application.log_level", "debug");

// Set complex objects
QJsonObject newSettings;
newSettings["enabled"] = true;
newSettings["timeout"] = 10000;
config->setObject("plugin.settings", newSettings);

// Save changes
config->save();
```

## Configuration Schema

### Validation

QtForge supports JSON Schema validation for configuration files:

```json
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "properties": {
        "application": {
            "type": "object",
            "properties": {
                "name": {"type": "string"},
                "version": {"type": "string"},
                "log_level": {
                    "type": "string",
                    "enum": ["trace", "debug", "info", "warn", "error", "fatal"]
                }
            },
            "required": ["name", "version"]
        }
    }
}
```

### Custom Validation

```cpp
#include <QtForge/ConfigurationValidator>

auto validator = std::make_unique<ConfigurationValidator>();
validator->setSchema(schemaJson);

if (!validator->validate(config->toJson())) {
    qWarning() << "Configuration validation failed:" 
               << validator->lastError();
}
```

## Plugin Configuration

### Plugin-Specific Settings

```cpp
#include <QtForge/PluginConfiguration>

// Get plugin configuration
auto* pluginConfig = config->pluginConfiguration("com.example.myplugin");

// Access plugin settings
QString serverUrl = pluginConfig->value("server_url").toString();
bool featureEnabled = pluginConfig->value("features.feature_a", false).toBool();

// Update plugin settings
pluginConfig->setValue("timeout", 60000);
pluginConfig->save();
```

### Configuration Templates

Create configuration templates for common plugin types:

```json
{
    "templates": {
        "network_plugin": {
            "timeout": 30000,
            "retry_count": 3,
            "ssl_verify": true
        },
        "ui_plugin": {
            "theme": "default",
            "animations": true,
            "shortcuts": {}
        }
    }
}
```

## Runtime Configuration

### Dynamic Updates

```cpp
// Watch for configuration changes
connect(config, &ConfigurationManager::configurationChanged,
        [](const QString& key, const QVariant& value) {
    qDebug() << "Configuration changed:" << key << "=" << value;
});

// Enable hot-reloading
config->setWatchEnabled(true);
```

### Configuration Signals

```cpp
// React to specific configuration changes
connect(config, &ConfigurationManager::valueChanged,
        this, [this](const QString& key) {
    if (key == "application.log_level") {
        updateLogLevel();
    }
});
```

## Environment-Specific Configuration

### Development Configuration

```json
{
    "environment": "development",
    "application": {
        "log_level": "debug"
    },
    "plugin_system": {
        "auto_load": false,
        "development_mode": true
    }
}
```

### Production Configuration

```json
{
    "environment": "production",
    "application": {
        "log_level": "warn"
    },
    "security": {
        "enable_signature_verification": true,
        "strict_mode": true
    }
}
```

## Configuration Best Practices

1. **Use meaningful defaults** for all configuration options
2. **Validate configuration** before using values
3. **Document all configuration options** with examples
4. **Use environment variables** for deployment-specific settings
5. **Implement configuration migration** for version updates
6. **Secure sensitive configuration** data (passwords, keys)
7. **Test configuration changes** in development environment

## Troubleshooting

### Common Issues

1. **Configuration not found**: Check file paths and permissions
2. **Invalid JSON**: Validate JSON syntax
3. **Schema validation errors**: Check against schema requirements
4. **Permission denied**: Ensure write access for configuration updates

### Debug Configuration

```cpp
// Enable configuration debugging
config->setDebugEnabled(true);

// Dump current configuration
qDebug() << config->toJson();

// Check configuration sources
auto sources = config->configurationSources();
for (const auto& source : sources) {
    qDebug() << "Source:" << source.path << "Priority:" << source.priority;
}
```

## See Also

- [Plugin Management](plugin-management.md)
- [Security Guidelines](security.md)
- [Performance Optimization](performance.md)
- [Troubleshooting Guide](troubleshooting.md)
