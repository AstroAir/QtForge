# Plugin Development Guide

This comprehensive guide covers everything you need to know about developing plugins for QtForge v3.2.0, including the new advanced plugin interfaces and multilingual support.

## Getting Started

### Plugin Basics

A QtForge plugin is a dynamic library that implements one of the plugin interfaces. QtForge v3.2.0 supports multiple plugin types and interfaces:

- **Native C++ Plugins**: Implement `IPlugin`, `IAdvancedPlugin`, or `IDynamicPlugin`
- **Python Plugins**: Written in Python using QtForge Python bindings
- **Lua Plugins**: Written in Lua using the enhanced Lua Plugin Bridge
- **JavaScript Plugins**: Planned for future releases
- **Remote Plugins**: Plugins running in separate processes
- **Composite Plugins**: Combinations of multiple plugin types

Plugins are loaded at runtime and can extend application functionality without requiring recompilation.

### Minimal Plugin Example

```cpp
#include <qtforge/core/plugin_interface.hpp>

class MyPlugin : public qtforge::IPlugin {
public:
    // Plugin metadata
    std::string name() const override {
        return "MyPlugin";
    }
    
    std::string version() const override {
        return "1.0.0";
    }
    
    std::string description() const override {
        return "A simple example plugin";
    }
    
    // Lifecycle methods
    qtforge::expected<void, qtforge::Error> initialize() override {
        // Plugin initialization logic
        return {};
    }
    
    qtforge::expected<void, qtforge::Error> activate() override {
        // Plugin activation logic
        return {};
    }
    
    qtforge::expected<void, qtforge::Error> deactivate() override {
        // Plugin deactivation logic
        return {};
    }
    
    void cleanup() override {
        // Plugin cleanup logic
    }
};

// Plugin factory function
extern "C" QTFORGE_EXPORT qtforge::IPlugin* createPlugin() {
    return new MyPlugin();
}

extern "C" QTFORGE_EXPORT void destroyPlugin(qtforge::IPlugin* plugin) {
    delete plugin;
}
```

## Plugin Architecture

### Plugin Interface

The `IPlugin` interface defines the contract that all plugins must implement:

```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    // Metadata
    virtual std::string name() const = 0;
    virtual std::string version() const = 0;
    virtual std::string description() const = 0;
    virtual std::vector<std::string> dependencies() const { return {}; }
    
    // Lifecycle
    virtual expected<void, Error> initialize() = 0;
    virtual expected<void, Error> activate() = 0;
    virtual expected<void, Error> deactivate() = 0;
    virtual void cleanup() = 0;
    
    // State
    virtual PluginState state() const = 0;
    virtual bool isCompatible(const std::string& version) const = 0;
};
```

### Plugin States

Plugins progress through several states during their lifecycle:

1. **Unloaded**: Plugin library not loaded
2. **Loaded**: Library loaded, plugin object created
3. **Initialized**: Plugin initialized, dependencies resolved
4. **Active**: Plugin is running and functional
5. **Inactive**: Plugin is loaded but not active
6. **Error**: Plugin encountered an error

### Plugin Metadata

Plugins should provide comprehensive metadata:

```cpp
class MyPlugin : public IPlugin {
public:
    std::string name() const override { return "MyPlugin"; }
    std::string version() const override { return "1.2.3"; }
    std::string description() const override { return "Plugin description"; }
    std::string author() const override { return "Your Name"; }
    std::string license() const override { return "MIT"; }
    
    std::vector<std::string> dependencies() const override {
        return {"CorePlugin >= 1.0.0", "UtilsPlugin >= 2.1.0"};
    }
    
    std::vector<std::string> tags() const override {
        return {"utility", "data-processing", "experimental"};
    }
};
```

## Advanced Plugin Features

### Service Plugins

Service plugins provide functionality to other plugins:

```cpp
class DatabaseService : public IPlugin, public IService {
public:
    // IPlugin implementation...
    
    // Service interface
    std::string serviceId() const override {
        return "database.service";
    }
    
    expected<void, Error> query(const std::string& sql) {
        // Database query implementation
        return {};
    }
};
```

### Event-Driven Plugins

Plugins can respond to system events:

```cpp
class EventPlugin : public IPlugin, public IEventHandler {
public:
    expected<void, Error> initialize() override {
        // Subscribe to events
        eventBus().subscribe("system.startup", this);
        eventBus().subscribe("user.action", this);
        return {};
    }
    
    void handleEvent(const Event& event) override {
        if (event.type() == "system.startup") {
            // Handle startup event
        } else if (event.type() == "user.action") {
            // Handle user action
        }
    }
};
```

### Configuration Support

Plugins can have configurable settings:

```cpp
class ConfigurablePlugin : public IPlugin {
private:
    PluginConfig config_;
    
public:
    expected<void, Error> initialize() override {
        // Load configuration
        config_ = configManager().loadConfig(name());
        
        auto timeout = config_.get<int>("timeout", 5000);
        auto enabled = config_.get<bool>("enabled", true);
        
        return {};
    }
    
    void updateConfig(const PluginConfig& newConfig) {
        config_ = newConfig;
        // Apply configuration changes
    }
};
```

## Plugin Communication

### Message Bus

Plugins communicate through the message bus:

```cpp
class PublisherPlugin : public IPlugin {
public:
    void publishData(const std::string& data) {
        auto message = std::make_shared<DataMessage>(data);
        messageBus().publish("data.channel", message);
    }
};

class SubscriberPlugin : public IPlugin {
public:
    expected<void, Error> initialize() override {
        messageBus().subscribe("data.channel", 
            [this](const auto& message) {
                handleDataMessage(message);
            });
        return {};
    }
    
private:
    void handleDataMessage(const std::shared_ptr<Message>& message) {
        auto dataMsg = std::dynamic_pointer_cast<DataMessage>(message);
        if (dataMsg) {
            // Process data
        }
    }
};
```

### Direct Plugin Communication

Plugins can communicate directly through interfaces:

```cpp
class ConsumerPlugin : public IPlugin {
public:
    expected<void, Error> initialize() override {
        // Find service plugin
        auto service = pluginManager().findPlugin<DatabaseService>("database.service");
        if (service) {
            service_ = service.value();
        }
        return {};
    }
    
    void performQuery() {
        if (service_) {
            auto result = service_->query("SELECT * FROM users");
            // Handle result
        }
    }
    
private:
    std::shared_ptr<DatabaseService> service_;
};
```

## Build System Integration

### CMake Configuration

```cmake
# Plugin CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(MyPlugin)

find_package(QtForge REQUIRED)

add_library(MyPlugin SHARED
    src/my_plugin.cpp
    src/my_plugin.hpp
)

target_link_libraries(MyPlugin
    QtForge::Core
    QtForge::Communication
)

# Set plugin properties
set_target_properties(MyPlugin PROPERTIES
    PREFIX ""  # Remove 'lib' prefix on Unix
    SUFFIX ".qtplugin"  # Custom extension
)

# Install plugin
install(TARGETS MyPlugin
    DESTINATION plugins/
)
```

### Plugin Manifest

Create a plugin manifest file (`plugin.json`):

```json
{
    "name": "MyPlugin",
    "version": "1.0.0",
    "description": "A sample plugin",
    "author": "Your Name",
    "license": "MIT",
    "dependencies": [
        {
            "name": "CorePlugin",
            "version": ">=1.0.0"
        }
    ],
    "capabilities": [
        "data-processing",
        "file-io"
    ],
    "configuration": {
        "timeout": {
            "type": "integer",
            "default": 5000,
            "description": "Operation timeout in milliseconds"
        },
        "enabled": {
            "type": "boolean",
            "default": true,
            "description": "Enable plugin functionality"
        }
    }
}
```

## Testing Plugins

### Unit Testing

```cpp
#include <gtest/gtest.h>
#include "my_plugin.hpp"

class MyPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        plugin_ = std::make_unique<MyPlugin>();
    }
    
    std::unique_ptr<MyPlugin> plugin_;
};

TEST_F(MyPluginTest, InitializationSucceeds) {
    auto result = plugin_->initialize();
    EXPECT_TRUE(result.has_value());
}

TEST_F(MyPluginTest, ActivationSucceeds) {
    plugin_->initialize();
    auto result = plugin_->activate();
    EXPECT_TRUE(result.has_value());
}
```

### Integration Testing

```cpp
TEST(PluginIntegrationTest, PluginLoadsCorrectly) {
    qtforge::PluginManager manager;
    auto result = manager.loadPlugin("MyPlugin.qtplugin");
    
    ASSERT_TRUE(result.has_value());
    auto plugin = result.value();
    
    EXPECT_EQ(plugin->name(), "MyPlugin");
    EXPECT_EQ(plugin->version(), "1.0.0");
}
```

## Best Practices

### Design Principles

1. **Single Responsibility**: Each plugin should have one clear purpose
2. **Loose Coupling**: Minimize dependencies between plugins
3. **Interface Stability**: Keep plugin interfaces stable across versions
4. **Error Handling**: Always handle errors gracefully
5. **Resource Management**: Use RAII and smart pointers

### Performance Tips

1. **Lazy Initialization**: Initialize resources only when needed
2. **Efficient Communication**: Use appropriate message types
3. **Memory Management**: Avoid memory leaks and excessive allocations
4. **Threading**: Be aware of thread safety requirements

### Security Considerations

1. **Input Validation**: Validate all external inputs
2. **Resource Limits**: Implement appropriate resource limits
3. **Privilege Separation**: Run with minimal required privileges
4. **Secure Communication**: Use encrypted communication when needed

## Debugging Plugins

### Logging

```cpp
class MyPlugin : public IPlugin {
public:
    expected<void, Error> initialize() override {
        logger().info("MyPlugin initializing...");
        
        try {
            // Initialization code
            logger().info("MyPlugin initialized successfully");
            return {};
        } catch (const std::exception& e) {
            logger().error("MyPlugin initialization failed: {}", e.what());
            return qtforge::Error("Initialization failed");
        }
    }
};
```

### Debug Builds

Enable debug features in CMake:

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(MyPlugin PRIVATE
        QTFORGE_DEBUG=1
        QTFORGE_ENABLE_LOGGING=1
    )
endif()
```

## Next Steps

- **[Advanced Plugin Development](advanced-plugin-development.md)**: Learn advanced techniques
- **[Plugin Architecture](plugin-architecture.md)**: Understand architectural patterns
- **[API Reference](../api/index.md)**: Detailed API documentation
- **[Examples](../examples/index.md)**: Study working examples
