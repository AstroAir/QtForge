# Basic Plugin

A comprehensive example demonstrating the core IPlugin interface - the foundation of QtForge plugin development.

## What This Example Teaches

- **Complete IPlugin Interface**: All essential methods every plugin needs
- **Lifecycle Management**: Proper initialization and shutdown patterns
- **Configuration System**: JSON-based configuration with validation
- **Command Execution**: Multiple commands with parameters and error handling
- **Thread Safety**: Safe concurrent access to plugin state
- **Error Handling**: Modern expected<T,E> pattern throughout

## Features

- ✅ **Core Interface**: Complete IPlugin implementation (~300 lines)
- ✅ **4 Commands**: status, echo, config, timer operations
- ✅ **Configuration**: JSON config with validation and live updates
- ✅ **Background Processing**: Timer-based background operations
- ✅ **Thread Safety**: Mutex-protected state and configuration
- ✅ **Comprehensive Testing**: Full test suite demonstrating all features

## Quick Start

### Build and Run

```bash
cd examples/01-fundamentals/basic-plugin
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Run the test application
./BasicPluginTest
```

### Expected Output

```
QtForge BasicPlugin Test Application
====================================

=== Testing Plugin Metadata ===
Plugin metadata:
  Name: BasicPlugin
  Description: Core QtForge plugin demonstrating essential IPlugin interface
  Version: 2.0.0
  Author: QtForge Examples
  ID: com.qtforge.examples.basic

=== Testing Plugin Lifecycle ===
BasicPlugin: Initializing...
BasicPlugin: Logging enabled, timer interval: 5000 ms
BasicPlugin: Initialized successfully!
✅ Plugin initialized successfully

=== Testing Plugin Commands ===
Available commands:
  - status
  - echo
  - config
  - timer
✅ Status command result: {"plugin":"BasicPlugin","state":1,"timer_count":0,"timer_active":true,"timestamp":"2024-01-15T10:30:00Z"}

=== Observing Background Processing ===
BasicPlugin: Timer event #1 - Hello from BasicPlugin!
BasicPlugin: Timer event #2 - Hello from BasicPlugin!
```

## Available Commands

### `status`

Get comprehensive plugin status information.

**Response:**

```json
{
  "plugin": "BasicPlugin",
  "state": 1,
  "timer_count": 5,
  "timer_active": true,
  "timestamp": "2024-01-15T10:30:00Z"
}
```

### `echo`

Echo back provided parameters with timestamp.

**Parameters:** Any JSON object
**Response:**

```json
{
  "echo": { "message": "Hello", "number": 42 },
  "timestamp": "2024-01-15T10:30:00Z",
  "plugin": "BasicPlugin"
}
```

### `config`

Manage plugin configuration.

**Get Configuration:**

```json
{ "action": "get" }
```

**Set Configuration:**

```json
{
  "action": "set",
  "config": {
    "timer_interval": 3000,
    "custom_message": "New message!"
  }
}
```

### `timer`

Control background timer operations.

**Actions:**

- `{"action": "start"}` - Start timer
- `{"action": "stop"}` - Stop timer
- `{"action": "reset"}` - Reset counter

## Configuration Options

```json
{
  "timer_interval": 5000, // Timer interval (1000-60000ms)
  "timer_enabled": true, // Enable/disable timer
  "logging_enabled": true, // Enable/disable logging
  "custom_message": "Hello!" // Custom timer message (max 200 chars)
}
```

## Key Concepts Demonstrated

### 1. Complete Plugin Interface

```cpp
class BasicPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "basic_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

    // All essential IPlugin methods implemented
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject&) override;
    // ... more methods
};
```

### 2. Configuration Validation

```cpp
bool BasicPlugin::validate_configuration(const QJsonObject& config) const {
    if (config.contains("timer_interval")) {
        int interval = config.value("timer_interval").toInt(-1);
        if (interval < 1000 || interval > 60000) {
            return false;
        }
    }
    return true;
}
```

### 3. Thread-Safe State Management

```cpp
// Atomic state for thread-safe access
std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};

// Mutex for configuration protection
mutable QMutex m_config_mutex;
QJsonObject m_configuration;
```

### 4. Command Pattern Implementation

```cpp
qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
    std::string_view command, const QJsonObject& params) {

    if (command == "status") {
        return execute_status_command(params);
    } else if (command == "echo") {
        return execute_echo_command(params);
    }
    // ... handle other commands
}
```

### 5. Background Processing

```cpp
// Timer-based background processing
std::unique_ptr<QTimer> m_timer;
std::atomic<int> m_timer_count{0};

void BasicPlugin::on_timer_timeout() {
    int count = ++m_timer_count;
    // Safe background processing
}
```

## Architecture Patterns

### Error Handling Strategy

- Uses `expected<T,E>` for all operations that can fail
- Provides detailed error messages with context
- Graceful degradation when operations fail

### State Management

- Atomic state variables for thread safety
- Mutex protection for complex data structures
- Clear state transitions with validation

### Configuration Management

- JSON-based configuration with schema validation
- Live configuration updates without restart
- Default configuration with sensible values

## Learning Path

After mastering this example, continue to:

1. **[Configuration Example](../configuration/)** - Advanced configuration patterns
2. **[Message Bus](../../02-communication/message-bus/)** - Inter-plugin communication
3. **[Background Tasks](../../03-services/background-tasks/)** - Advanced service patterns

## Common Questions

**Q: Why use atomic variables for state?**
A: Ensures thread-safe access without locking for simple state checks.

**Q: When should I use mutex vs atomic?**
A: Use atomic for simple values, mutex for complex data structures like JSON objects.

**Q: How do I add new commands?**
A: Add to `execute_command()` method and `available_commands()` list, implement handler method.

**Q: Can configuration be changed while plugin is running?**
A: Yes, this example demonstrates live configuration updates.

## Dependencies

- **Required**: QtForge::Core, Qt6::Core
- **Optional**: None

## License

MIT License - Same as QtForge library
