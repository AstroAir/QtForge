# QtForge Quick Reference

This page provides a quick reference for common QtForge operations and patterns.

## Essential Classes

### Plugin Interface

```cpp
class MyPlugin : public qtforge::IPlugin {
public:
    std::string name() const override;
    std::string version() const override;
    qtforge::expected<void, std::string> initialize() override;
    qtforge::expected<void, std::string> execute(const std::string& command) override;
    void cleanup() override;
};
```

### Plugin Manager

```cpp
// Create plugin manager
auto manager = qtforge::PluginManager::create();

// Load plugin
auto result = manager->load_plugin("plugin.dll");
if (result) {
    auto plugin = result.value();
    plugin->initialize();
}

// Unload plugin
manager->unload_plugin("plugin_name");
```

## Common Patterns

### Error Handling

```cpp
auto result = plugin->execute("command");
if (!result) {
    std::cerr << "Error: " << result.error() << std::endl;
    return;
}
```

### Plugin Discovery

```cpp
auto plugins = manager->discover_plugins("./plugins/");
for (const auto& path : plugins) {
    std::cout << "Found plugin: " << path << std::endl;
}
```

### Security Configuration

```cpp
auto& security = manager->security_manager();
security.set_trust_level(qtforge::TrustLevel::Trusted);
security.add_trusted_path("./trusted_plugins/");
```

## CMake Integration

### Basic Setup

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(QtForge REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app Qt6::Core QtForge::Core)
```

### Plugin Creation

```cmake
qtforge_add_plugin(my_plugin
    SOURCES my_plugin.cpp
    METADATA metadata.json
    LINK_LIBRARIES Qt6::Core
)
```

## Plugin Metadata

### metadata.json

```json
{
  "name": "MyPlugin",
  "version": "1.0.0",
  "description": "Example plugin",
  "author": "Your Name",
  "dependencies": [],
  "capabilities": ["command_execution"],
  "security_level": "trusted"
}
```

## Python Integration

### Python Plugin

```python
import qtforge

class MyPythonPlugin(qtforge.IPlugin):
    def name(self) -> str:
        return "MyPythonPlugin"

    def version(self) -> str:
        return "1.0.0"

    def initialize(self) -> qtforge.Result[None, str]:
        print("Python plugin initialized!")
        return qtforge.Ok(None)

    def execute(self, command: str) -> qtforge.Result[None, str]:
        if command == "hello":
            print("Hello from Python!")
            return qtforge.Ok(None)
        return qtforge.Err(f"Unknown command: {command}")

# Export plugin
qtforge.export_plugin(MyPythonPlugin)
```

## Lua Integration

### Lua Plugin

```lua
local plugin = {}

function plugin.name()
    return "MyLuaPlugin"
end

function plugin.version()
    return "1.0.0"
end

function plugin.initialize()
    print("Lua plugin initialized!")
    return true
end

function plugin.execute(command)
    if command == "hello" then
        print("Hello from Lua!")
        return true
    end
    return false, "Unknown command: " .. command
end

return plugin
```

## Debugging Tips

### Enable Logging

```cpp
qtforge::Logger::set_level(qtforge::LogLevel::Debug);
qtforge::Logger::enable_component_logging(true);
```

### Plugin Validation

```cpp
auto validation = manager->validate_plugin("plugin.dll");
if (!validation.is_valid) {
    for (const auto& error : validation.errors) {
        std::cerr << "Validation error: " << error << std::endl;
    }
}
```

## Performance Tips

### Lazy Loading

```cpp
// Load plugin metadata only
auto metadata = manager->load_plugin_metadata("plugin.dll");

// Load actual plugin when needed
auto plugin = manager->load_plugin_deferred("plugin.dll");
```

### Plugin Caching

```cpp
manager->enable_plugin_cache(true);
manager->set_cache_directory("./plugin_cache/");
```

## Common Issues

### Plugin Not Found

- Check plugin path
- Verify file permissions
- Ensure plugin is properly exported

### Loading Failures

- Check Qt version compatibility
- Verify all dependencies are available
- Review security settings

### Runtime Errors

- Enable debug logging
- Check plugin metadata
- Validate plugin interface implementation

## See Also

- [Installation Guide](installation.md)
- [Plugin Development Guide](../developer-guide/plugin-development.md)
- [API Reference](../api/index.md)
- [Examples](../examples/index.md)
- [Troubleshooting](../user-guide/troubleshooting.md)
