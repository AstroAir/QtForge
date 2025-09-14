# Plugin Interface

The `IPlugin` interface is the foundation of the QtPlugin system. All plugins must implement this interface to be compatible with the plugin manager.

## Overview

```cpp
#include <qtplugin/core/plugin_interface.hpp>

class IPlugin {
public:
    virtual ~IPlugin() = default;

    // Lifecycle management
    virtual expected<void, PluginError> initialize() = 0;
    virtual void shutdown() noexcept = 0;
    virtual PluginState state() const noexcept = 0;
    virtual bool is_initialized() const noexcept = 0;

    // Metadata and identification
    virtual PluginMetadata metadata() const = 0;
    virtual std::string id() const noexcept = 0;
    virtual std::string name() const noexcept = 0;
    virtual std::string version() const noexcept = 0;
    virtual std::string description() const noexcept = 0;
    virtual std::string author() const noexcept = 0;

    // Configuration management
    virtual expected<void, PluginError> configure(const QJsonObject& config) = 0;
    virtual QJsonObject current_configuration() const = 0;

    // Command execution
    virtual expected<QJsonObject, PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}
    ) = 0;

    // Capability queries
    virtual std::vector<std::string> supported_commands() const = 0;
    virtual bool supports_command(std::string_view command) const = 0;
    virtual PluginCapabilities capabilities() const noexcept = 0;
};
```

## Core Methods

### Lifecycle Management

#### `initialize()`

Initializes the plugin and prepares it for use.

```cpp
virtual expected<void, PluginError> initialize() = 0;
```

**Returns:**

- `expected<void, PluginError>`: Success or error information

**Example:**

```cpp
expected<void, PluginError> MyPlugin::initialize() {
    if (m_initialized) {
        return make_unexpected(PluginError{
            PluginErrorCode::AlreadyInitialized,
            "Plugin is already initialized"
        });
    }

    // Perform initialization
    try {
        setup_resources();
        connect_signals();
        m_initialized = true;
        m_state = PluginState::Running;
        return {};
    } catch (const std::exception& e) {
        return make_unexpected(PluginError{
            PluginErrorCode::InitializationFailed,
            std::string("Initialization failed: ") + e.what()
        });
    }
}
```

#### `shutdown()`

Cleanly shuts down the plugin and releases resources.

```cpp
virtual void shutdown() noexcept = 0;
```

**Notes:**

- Must be `noexcept` - cannot throw exceptions
- Should handle cleanup gracefully even if initialization failed
- Called automatically during plugin unloading

**Example:**

```cpp
void MyPlugin::shutdown() noexcept {
    try {
        if (m_initialized) {
            cleanup_resources();
            disconnect_signals();
            m_initialized = false;
        }
        m_state = PluginState::Unloaded;
    } catch (...) {
        // Log error but don't throw
        qWarning() << "Error during plugin shutdown";
    }
}
```

#### `state()`

Returns the current state of the plugin.

```cpp
virtual PluginState state() const noexcept = 0;
```

**Returns:**

- `PluginState`: Current plugin state

**Plugin States:**

```cpp
enum class PluginState {
    Unloaded,    // Plugin not loaded or shut down
    Loaded,      // Plugin loaded but not initialized
    Running,     // Plugin initialized and running
    Error        // Plugin in error state
};
```

#### `is_initialized()`

Quick check if the plugin is initialized and ready to use.

```cpp
virtual bool is_initialized() const noexcept = 0;
```

**Returns:**

- `bool`: `true` if plugin is initialized

## Metadata Methods

### `metadata()`

Returns comprehensive plugin metadata.

```cpp
virtual PluginMetadata metadata() const = 0;
```

**Returns:**

- `PluginMetadata`: Complete plugin information

**PluginMetadata Structure:**

```cpp
struct PluginMetadata {
    std::string id;                          // Unique plugin identifier
    std::string name;                        // Human-readable name
    std::string version;                     // Version string
    std::string description;                 // Plugin description
    std::string author;                      // Author information
    std::string license;                     // License information
    std::vector<std::string> dependencies;   // Required dependencies
    std::vector<std::string> tags;           // Classification tags
    QJsonObject custom_data;                 // Custom metadata

    // Version information
    Version semantic_version;                // Parsed version
    Version min_qtplugin_version;           // Minimum QtPlugin version
    Version min_qt_version;                 // Minimum Qt version

    // Capabilities
    PluginCapabilities capabilities;         // Plugin capabilities
    std::vector<std::string> interfaces;    // Implemented interfaces
};
```

### Individual Metadata Methods

```cpp
virtual std::string id() const noexcept = 0;
virtual std::string name() const noexcept = 0;
virtual std::string version() const noexcept = 0;
virtual std::string description() const noexcept = 0;
virtual std::string author() const noexcept = 0;
```

These provide quick access to common metadata without creating the full structure.

## Configuration Management

### `configure()`

Applies configuration to the plugin.

```cpp
virtual expected<void, PluginError> configure(const QJsonObject& config) = 0;
```

**Parameters:**

- `config`: JSON configuration object

**Returns:**

- `expected<void, PluginError>`: Success or error information

**Example:**

```cpp
expected<void, PluginError> MyPlugin::configure(const QJsonObject& config) {
    try {
        // Validate configuration
        if (!config.contains("required_setting")) {
            return make_unexpected(PluginError{
                PluginErrorCode::InvalidConfiguration,
                "Missing required setting: required_setting"
            });
        }

        // Apply configuration
        m_setting1 = config["setting1"].toString();
        m_setting2 = config["setting2"].toInt();

        // Store current configuration
        m_current_config = config;

        return {};
    } catch (const std::exception& e) {
        return make_unexpected(PluginError{
            PluginErrorCode::ConfigurationFailed,
            std::string("Configuration failed: ") + e.what()
        });
    }
}
```

### `current_configuration()`

Returns the current plugin configuration.

```cpp
virtual QJsonObject current_configuration() const = 0;
```

**Returns:**

- `QJsonObject`: Current configuration

## Command Execution

### `execute_command()`

Executes a plugin-specific command.

```cpp
virtual expected<QJsonObject, PluginError> execute_command(
    std::string_view command,
    const QJsonObject& params = {}
) = 0;
```

**Parameters:**

- `command`: Command name to execute
- `params`: Command parameters (optional)

**Returns:**

- `expected<QJsonObject, PluginError>`: Command result or error

**Example:**

```cpp
expected<QJsonObject, PluginError> MyPlugin::execute_command(
    std::string_view command,
    const QJsonObject& params
) {
    if (command == "hello") {
        QJsonObject result;
        QString name = params.value("name").toString("World");
        result["message"] = QString("Hello, %1!").arg(name);
        result["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        return result;

    } else if (command == "status") {
        QJsonObject result;
        result["state"] = static_cast<int>(state());
        result["initialized"] = is_initialized();
        result["uptime"] = m_uptime_timer.elapsed();
        return result;

    } else {
        return make_unexpected(PluginError{
            PluginErrorCode::UnknownCommand,
            std::string("Unknown command: ") + std::string(command)
        });
    }
}
```

### `supported_commands()`

Returns list of supported commands.

```cpp
virtual std::vector<std::string> supported_commands() const = 0;
```

**Returns:**

- `std::vector<std::string>`: List of command names

### `supports_command()`

Checks if a specific command is supported.

```cpp
virtual bool supports_command(std::string_view command) const = 0;
```

**Parameters:**

- `command`: Command name to check

**Returns:**

- `bool`: `true` if command is supported

## Capabilities

### `capabilities()`

Returns the plugin's capabilities.

```cpp
virtual PluginCapabilities capabilities() const noexcept = 0;
```

**Returns:**

- `PluginCapabilities`: Plugin capability flags

**PluginCapabilities:**

```cpp
enum class PluginCapability : uint32_t {
    None = 0,
    Service = 1 << 0,        // Background service
    UI = 1 << 1,             // User interface
    Network = 1 << 2,        // Network operations
    FileSystem = 1 << 3,     // File system access
    Database = 1 << 4,       // Database operations
    Scripting = 1 << 5,      // Scripting support
    HotReload = 1 << 6,      // Hot reload support
    MultiInstance = 1 << 7,  // Multiple instances
    ThreadSafe = 1 << 8,     // Thread-safe operations

    // Combinations
    All = 0xFFFFFFFF
};

using PluginCapabilities = QFlags<PluginCapability>;
```

## Implementation Example

Here's a complete example of implementing the `IPlugin` interface:

```cpp
class MyExamplePlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "metadata.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    MyExamplePlugin(QObject* parent = nullptr);
    ~MyExamplePlugin() override;

    // Lifecycle
    expected<void, PluginError> initialize() override;
    void shutdown() noexcept override;
    PluginState state() const noexcept override { return m_state; }
    bool is_initialized() const noexcept override { return m_initialized; }

    // Metadata
    PluginMetadata metadata() const override;
    std::string id() const noexcept override { return "com.example.myplugin"; }
    std::string name() const noexcept override { return "My Example Plugin"; }
    std::string version() const noexcept override { return "1.0.0"; }
    std::string description() const noexcept override { return "Example plugin"; }
    std::string author() const noexcept override { return "Example Author"; }

    // Configuration
    expected<void, PluginError> configure(const QJsonObject& config) override;
    QJsonObject current_configuration() const override;

    // Commands
    expected<QJsonObject, PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}
    ) override;

    std::vector<std::string> supported_commands() const override;
    bool supports_command(std::string_view command) const override;
    PluginCapabilities capabilities() const noexcept override;

private:
    PluginState m_state = PluginState::Unloaded;
    bool m_initialized = false;
    QJsonObject m_config;
    QElapsedTimer m_uptime_timer;
};
```

## Best Practices

### Error Handling

- Always use `expected<T, E>` for operations that can fail
- Provide meaningful error messages
- Use appropriate error codes
- Handle exceptions and convert to errors

### Resource Management

- Use RAII for resource management
- Clean up in `shutdown()` even if initialization failed
- Don't throw exceptions from `shutdown()`

### Thread Safety

- Document thread safety guarantees
- Use appropriate synchronization
- Consider concurrent access patterns

### Performance

- Keep metadata methods fast (they're called frequently)
- Cache expensive computations
- Use move semantics where appropriate

## See Also

- [Plugin Manager](plugin-manager.md) - Managing plugins
- [Plugin Loader](plugin-loader.md) - Loading plugins
- [Error Handling](../utils/error-handling.md) - Error handling patterns
- [Examples](../../examples/index.md) - Working examples
