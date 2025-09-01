# PluginHotReloadManager API Reference

!!! info "Module Information"
**Header**: `qtplugin/monitoring/plugin_hot_reload_manager.hpp`  
 **Namespace**: `qtplugin`  
 **Since**: QtForge v3.0.0  
 **Status**: Stable

## Overview

The PluginHotReloadManager provides automatic plugin reloading capabilities by monitoring plugin files for changes. It uses Qt's file system watcher to detect modifications and triggers plugin reloads without requiring application restart.

### Key Features

- **File System Monitoring**: Automatic detection of plugin file changes
- **Selective Monitoring**: Enable/disable hot reload per plugin
- **Global Control**: System-wide hot reload enable/disable
- **Callback System**: Customizable reload handling through callbacks
- **Thread Safety**: Safe concurrent access to hot reload functionality
- **Performance Optimized**: Efficient file watching with minimal overhead

### Use Cases

- **Development Workflow**: Rapid plugin development and testing
- **Production Updates**: Live plugin updates without downtime
- **Configuration Changes**: Dynamic plugin reconfiguration
- **Debugging**: Real-time plugin modification during debugging

## Quick Start

```cpp
#include <qtplugin/monitoring/plugin_hot_reload_manager.hpp>

// Create hot reload manager
auto hot_reload = PluginHotReloadManager::create();

// Set up reload callback
hot_reload->set_reload_callback([](const std::string& plugin_id) {
    qDebug() << "Plugin needs reload:" << QString::fromStdString(plugin_id);
    // Handle plugin reload logic
});

// Enable hot reload for a plugin
auto result = hot_reload->enable_hot_reload("my_plugin", "/path/to/plugin.so");
if (result) {
    qDebug() << "Hot reload enabled for my_plugin";
} else {
    qDebug() << "Failed to enable hot reload:" << result.error().message();
}

// Connect to signals for notifications
QObject::connect(hot_reload.get(), &PluginHotReloadManager::plugin_file_changed,
                 [](const QString& plugin_id, const QString& file_path) {
    qDebug() << "Plugin file changed:" << plugin_id << "at" << file_path;
});
```

## Interface: IPluginHotReloadManager

Base interface for hot reload management functionality.

### Virtual Methods

#### `enable_hot_reload()`

```cpp
virtual qtplugin::expected<void, PluginError> enable_hot_reload(
    const std::string& plugin_id,
    const std::filesystem::path& file_path) = 0;
```

Enables hot reload monitoring for a specific plugin.

**Parameters:**

- `plugin_id` - Unique plugin identifier
- `file_path` - Path to the plugin file to monitor

**Returns:**

- `expected<void, PluginError>` - Success or error

**Errors:**

- `PluginErrorCode::FileNotFound` - Plugin file does not exist
- `PluginErrorCode::InvalidPath` - Invalid file path provided
- `PluginErrorCode::AlreadyExists` - Hot reload already enabled for plugin

#### `disable_hot_reload()`

```cpp
virtual void disable_hot_reload(const std::string& plugin_id) = 0;
```

Disables hot reload monitoring for a plugin.

**Parameters:**

- `plugin_id` - Plugin identifier to disable monitoring for

#### `is_hot_reload_enabled()`

```cpp
virtual bool is_hot_reload_enabled(const std::string& plugin_id) const = 0;
```

Checks if hot reload is enabled for a specific plugin.

**Parameters:**

- `plugin_id` - Plugin identifier to check

**Returns:**

- `bool` - True if hot reload is enabled

#### `set_reload_callback()`

```cpp
virtual void set_reload_callback(
    std::function<void(const std::string&)> callback) = 0;
```

Sets the callback function to be called when a plugin needs reloading.

**Parameters:**

- `callback` - Function to call with plugin ID when reload is needed

**Example:**

```cpp
hot_reload->set_reload_callback([this](const std::string& plugin_id) {
    // Custom reload logic
    auto plugin = plugin_manager->get_plugin(plugin_id);
    if (plugin) {
        plugin_manager->unload_plugin(plugin_id);
        plugin_manager->load_plugin(plugin_id);
    }
});
```

#### `get_hot_reload_plugins()`

```cpp
virtual std::vector<std::string> get_hot_reload_plugins() const = 0;
```

Gets list of all plugins with hot reload enabled.

**Returns:**

- `std::vector<std::string>` - Vector of plugin IDs

#### `clear()`

```cpp
virtual void clear() = 0;
```

Clears all hot reload watchers and disables monitoring for all plugins.

#### `set_global_hot_reload_enabled()`

```cpp
virtual void set_global_hot_reload_enabled(bool enabled) = 0;
```

Enables or disables hot reload globally for all plugins.

**Parameters:**

- `enabled` - Global hot reload state

#### `is_global_hot_reload_enabled()`

```cpp
virtual bool is_global_hot_reload_enabled() const = 0;
```

Checks if global hot reload is enabled.

**Returns:**

- `bool` - True if globally enabled

## Class: PluginHotReloadManager

Concrete implementation of the hot reload manager using Qt's file system watcher.

### Constructor

```cpp
explicit PluginHotReloadManager(QObject* parent = nullptr);
```

### Static Methods

#### `create()`

```cpp
static std::shared_ptr<PluginHotReloadManager> create();
```

Creates a new hot reload manager instance.

**Returns:**

- `std::shared_ptr<PluginHotReloadManager>` - Shared pointer to new instance

**Example:**

```cpp
auto hot_reload = PluginHotReloadManager::create();
```

### Signals

The PluginHotReloadManager emits the following Qt signals:

#### `plugin_file_changed`

```cpp
void plugin_file_changed(const QString& plugin_id, const QString& file_path);
```

Emitted when a monitored plugin file changes.

**Parameters:**

- `plugin_id` - Plugin identifier
- `file_path` - Path to the changed file

#### `hot_reload_enabled`

```cpp
void hot_reload_enabled(const QString& plugin_id);
```

Emitted when hot reload is enabled for a plugin.

#### `hot_reload_disabled`

```cpp
void hot_reload_disabled(const QString& plugin_id);
```

Emitted when hot reload is disabled for a plugin.

## Error Handling

Common error codes and their meanings:

| Error Code         | Description                          | Resolution                           |
| ------------------ | ------------------------------------ | ------------------------------------ |
| `FileNotFound`     | Plugin file does not exist           | Verify file path is correct          |
| `InvalidPath`      | Invalid file path provided           | Check path format and accessibility  |
| `AlreadyExists`    | Hot reload already enabled           | Disable first or check current state |
| `PermissionDenied` | Cannot watch file due to permissions | Check file system permissions        |

## Thread Safety

- **Thread-safe methods**: All public methods are thread-safe
- **Signal emissions**: Signals are emitted from the main thread
- **File watching**: File system monitoring runs in background threads
- **Callback execution**: Reload callbacks are executed in the main thread

## Performance Considerations

- **Memory usage**: Approximately 100-200 bytes per monitored plugin
- **CPU usage**: Minimal overhead, only active during file changes
- **File system load**: Uses efficient native file watching APIs
- **Scalability**: Can monitor hundreds of plugins simultaneously

## Integration Examples

### Basic Hot Reload Setup

```cpp
#include <qtplugin/monitoring/plugin_hot_reload_manager.hpp>
#include <qtplugin/core/plugin_manager.hpp>

class DevelopmentEnvironment {
private:
    std::shared_ptr<PluginManager> m_plugin_manager;
    std::shared_ptr<PluginHotReloadManager> m_hot_reload;

public:
    bool initialize() {
        m_plugin_manager = PluginManager::create();
        m_hot_reload = PluginHotReloadManager::create();

        // Set up automatic reload handling
        m_hot_reload->set_reload_callback([this](const std::string& plugin_id) {
            handle_plugin_reload(plugin_id);
        });

        // Connect to signals for logging
        connect(m_hot_reload.get(), &PluginHotReloadManager::plugin_file_changed,
                this, &DevelopmentEnvironment::on_plugin_file_changed);

        return true;
    }

    bool enable_development_mode(const QString& plugin_id, const QString& plugin_path) {
        // Enable hot reload for development
        auto result = m_hot_reload->enable_hot_reload(
            plugin_id.toStdString(),
            plugin_path.toStdString()
        );

        if (result) {
            qDebug() << "Development mode enabled for" << plugin_id;
            return true;
        } else {
            qDebug() << "Failed to enable development mode:" << result.error().message();
            return false;
        }
    }

private:
    void handle_plugin_reload(const std::string& plugin_id) {
        qDebug() << "Reloading plugin:" << QString::fromStdString(plugin_id);

        // Unload current plugin
        auto unload_result = m_plugin_manager->unload_plugin(plugin_id);
        if (!unload_result) {
            qWarning() << "Failed to unload plugin:" << unload_result.error().message();
            return;
        }

        // Small delay to ensure file operations complete
        QThread::msleep(100);

        // Reload plugin
        auto load_result = m_plugin_manager->load_plugin(plugin_id);
        if (load_result) {
            qDebug() << "Plugin reloaded successfully:" << QString::fromStdString(plugin_id);
        } else {
            qWarning() << "Failed to reload plugin:" << load_result.error().message();
        }
    }

private slots:
    void on_plugin_file_changed(const QString& plugin_id, const QString& file_path) {
        qDebug() << "Plugin file changed:" << plugin_id << "at" << file_path;
        // Additional logging or notification logic
    }
};
```

### Production Hot Reload with Validation

```cpp
class ProductionHotReload {
private:
    std::shared_ptr<PluginHotReloadManager> m_hot_reload;
    std::shared_ptr<SecurityManager> m_security;

public:
    bool setup_production_reload() {
        m_hot_reload = PluginHotReloadManager::create();

        // Set up validated reload callback
        m_hot_reload->set_reload_callback([this](const std::string& plugin_id) {
            handle_validated_reload(plugin_id);
        });

        return true;
    }

private:
    void handle_validated_reload(const std::string& plugin_id) {
        // Validate plugin before reloading
        auto plugin_path = get_plugin_path(plugin_id);

        // Security validation
        auto validation_result = m_security->validate_plugin(plugin_path);
        if (!validation_result || !validation_result.value().is_valid) {
            qWarning() << "Plugin validation failed, skipping reload:"
                       << QString::fromStdString(plugin_id);
            return;
        }

        // Proceed with reload
        perform_safe_reload(plugin_id);
    }

    void perform_safe_reload(const std::string& plugin_id) {
        // Implementation of safe reload with rollback capability
        // Save current state, attempt reload, rollback on failure
    }
};
```

## Python Bindings

!!! note "Python Support"
This component is available in Python through the `qtforge.monitoring` module.

```python
import qtforge

# Create hot reload manager
hot_reload = qtforge.monitoring.PluginHotReloadManager()

# Set up reload callback
def handle_reload(plugin_id):
    print(f"Plugin {plugin_id} needs reload")
    # Handle reload logic

hot_reload.set_reload_callback(handle_reload)

# Enable hot reload for a plugin
result = hot_reload.enable_hot_reload("my_plugin", "/path/to/plugin.so")
if result:
    print("Hot reload enabled")

# Check status
plugins = hot_reload.get_hot_reload_plugins()
print(f"Monitored plugins: {plugins}")

# Global control
hot_reload.set_global_hot_reload_enabled(True)
```

## Related Components

- **[PluginManager](../core/plugin-manager.md)**: Core plugin management for reload operations
- **[PluginMetricsCollector](plugin-metrics-collector.md)**: Performance monitoring during reloads
- **[SecurityManager](../security/security-manager.md)**: Plugin validation before reload
- **[ResourceMonitor](resource-monitor.md)**: Resource usage monitoring during reloads

## Migration Notes

### From v2.x to v3.0

- **New Features**: Global hot reload control, enhanced error handling
- **API Changes**: None (backward compatible)
- **Deprecated**: None

## See Also

- [Monitoring User Guide](../../user-guide/monitoring-optimization.md)
- [Development Workflow Guide](../../user-guide/development-workflow.md)
- [Hot Reload Examples](../../examples/hot-reload-examples.md)
- [Architecture Overview](../../architecture/system-design.md)

---

_Last updated: December 2024 | QtForge v3.0.0_
