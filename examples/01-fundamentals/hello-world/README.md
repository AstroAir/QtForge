# Hello World Plugin

The simplest possible QtForge plugin - perfect for absolute beginners!

## What This Example Teaches

- **Plugin Basics**: How to create a minimal working plugin
- **Essential Interface**: Required methods from IPlugin interface
- **Simple Commands**: Basic command execution pattern
- **Plugin Lifecycle**: Initialize and shutdown operations
- **Metadata**: Plugin identification and information

## Features

- ✅ **Minimal Implementation**: Only ~70 lines of code
- ✅ **Single Command**: `hello` command with optional name parameter
- ✅ **Basic Lifecycle**: Initialize and shutdown
- ✅ **Error Handling**: Simple error responses
- ✅ **Metadata**: Complete plugin information

## Quick Start

### Build and Run

```bash
cd examples/01-fundamentals/hello-world
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Test the Plugin

```cpp
#include "hello_world_plugin.hpp"

// Create and initialize
auto plugin = std::make_unique<HelloWorldPlugin>();
auto result = plugin->initialize();

if (result) {
    // Execute hello command
    auto response = plugin->execute_command("hello");
    // Output: {"message": "Hello, World!", "timestamp": "...", "plugin": "HelloWorldPlugin"}

    // Execute with custom name
    QJsonObject params{{"name", "QtForge"}};
    auto custom_response = plugin->execute_command("hello", params);
    // Output: {"message": "Hello, QtForge!", "timestamp": "...", "plugin": "HelloWorldPlugin"}

    plugin->shutdown();
}
```

## Code Structure

### Header File (`hello_world_plugin.hpp`)

- Plugin class declaration
- Essential IPlugin interface methods
- Minimal member variables

### Implementation (`hello_world_plugin.cpp`)

- Simple initialization and shutdown
- Single command implementation
- Basic error handling

### Metadata (`hello_world_plugin.json`)

- Plugin identification
- Command listing
- Basic configuration schema

## Available Commands

### `hello`

Greets the user with an optional custom name.

**Parameters:**

- `name` (optional): Name to greet (default: "World")

**Response:**

```json
{
  "message": "Hello, [name]!",
  "timestamp": "2024-01-15T10:30:00Z",
  "plugin": "HelloWorldPlugin"
}
```

## Learning Path

After mastering this example, continue to:

1. **[Basic Plugin](../basic-plugin/)** - Complete IPlugin interface
2. **[Configuration](../configuration/)** - Configuration management
3. **[Message Bus](../../02-communication/message-bus/)** - Inter-plugin communication

## Key Concepts Demonstrated

### 1. Plugin Interface Implementation

```cpp
class HelloWorldPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "hello_world_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)
    // ...
};
```

### 2. Command Execution Pattern

```cpp
qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
    std::string_view command, const QJsonObject& params) {

    if (command == "hello") {
        QString name = params.value("name").toString("World");
        QJsonObject result;
        result["message"] = QString("Hello, %1!").arg(name);
        return result;
    }

    return qtplugin::make_unexpected(qtplugin::PluginError{
        qtplugin::PluginErrorCode::CommandNotFound,
        "Unknown command"
    });
}
```

### 3. Error Handling

```cpp
// Using expected<T,E> pattern for error handling
if (m_state != qtplugin::PluginState::Loaded) {
    return qtplugin::make_unexpected(qtplugin::PluginError{
        qtplugin::PluginErrorCode::InvalidState,
        "Plugin not initialized"
    });
}
```

## Common Beginner Questions

**Q: Why do I need Q_OBJECT macro?**
A: Required for Qt's meta-object system and plugin loading mechanism.

**Q: What's the purpose of the JSON file?**
A: Contains plugin metadata for the Qt plugin system and QtForge framework.

**Q: Why use expected<T,E> instead of exceptions?**
A: QtForge uses modern C++ error handling patterns that are more explicit and performant.

**Q: How do I add more commands?**
A: Add them to the `execute_command` method and `available_commands` list.

## Next Steps

Once you understand this example:

- Try modifying the hello command
- Add a new command (e.g., "goodbye")
- Experiment with different parameter types
- Move on to the [Basic Plugin](../basic-plugin/) example

## Dependencies

- **Required**: QtForge::Core, Qt6::Core
- **Optional**: None

## License

MIT License - Same as QtForge library
