# PluginManager

The `PluginManager` class is the central component responsible for managing the lifecycle of plugins in QtForge applications.

## Overview

The PluginManager provides a comprehensive interface for:
- Loading and unloading plugins
- Managing plugin dependencies
- Coordinating plugin lifecycle
- Providing plugin discovery and registration
- Handling plugin errors and recovery

## Class Declaration

```cpp
namespace qtforge::core {

class QTFORGE_EXPORT PluginManager {
public:
    // Construction and destruction
    PluginManager();
    explicit PluginManager(const PluginManagerConfig& config);
    ~PluginManager();
    
    // Plugin loading
    expected<std::shared_ptr<IPlugin>, Error> loadPlugin(const std::string& path);
    expected<std::vector<std::shared_ptr<IPlugin>>, Error> loadPluginsFromDirectory(const std::string& directory);
    expected<void, Error> unloadPlugin(const std::string& name);
    expected<void, Error> unloadAllPlugins();
    
    // Plugin management
    std::vector<std::shared_ptr<IPlugin>> getLoadedPlugins() const;
    std::shared_ptr<IPlugin> findPlugin(const std::string& name) const;
    template<typename T> std::shared_ptr<T> findPlugin(const std::string& name) const;
    
    // Plugin lifecycle
    expected<void, Error> initializePlugin(const std::string& name);
    expected<void, Error> activatePlugin(const std::string& name);
    expected<void, Error> deactivatePlugin(const std::string& name);
    
    // Plugin information
    std::vector<PluginInfo> getPluginInfo() const;
    PluginState getPluginState(const std::string& name) const;
    std::vector<std::string> getPluginDependencies(const std::string& name) const;
    
    // Configuration
    void setConfig(const PluginManagerConfig& config);
    const PluginManagerConfig& getConfig() const;
    
    // Events
    void setEventHandler(std::shared_ptr<IPluginEventHandler> handler);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace qtforge::core
```

## Core Methods

### Plugin Loading

#### loadPlugin()

Loads a single plugin from the specified path.

```cpp
auto result = pluginManager.loadPlugin("plugins/my_plugin.qtplugin");
if (result) {
    auto plugin = result.value();
    std::cout << "Loaded plugin: " << plugin->name() << std::endl;
} else {
    std::cerr << "Failed to load plugin: " << result.error().message() << std::endl;
}
```

**Parameters:**
- `path`: File path to the plugin library

**Returns:**
- `expected<std::shared_ptr<IPlugin>, Error>`: Plugin instance or error

#### loadPluginsFromDirectory()

Loads all plugins from a directory.

```cpp
auto result = pluginManager.loadPluginsFromDirectory("plugins/");
if (result) {
    auto plugins = result.value();
    std::cout << "Loaded " << plugins.size() << " plugins" << std::endl;
} else {
    std::cerr << "Failed to load plugins: " << result.error().message() << std::endl;
}
```

**Parameters:**
- `directory`: Directory path containing plugin files

**Returns:**
- `expected<std::vector<std::shared_ptr<IPlugin>>, Error>`: Vector of plugin instances or error

### Plugin Discovery

#### findPlugin()

Finds a loaded plugin by name.

```cpp
// Find by name
auto plugin = pluginManager.findPlugin("MyPlugin");
if (plugin) {
    std::cout << "Found plugin: " << plugin->name() << std::endl;
}

// Find by type
auto typedPlugin = pluginManager.findPlugin<IDataProcessor>("DataProcessor");
if (typedPlugin) {
    auto result = typedPlugin->processData(inputData);
}
```

**Parameters:**
- `name`: Plugin name to search for

**Returns:**
- `std::shared_ptr<IPlugin>`: Plugin instance or nullptr if not found

### Plugin Lifecycle Management

#### initializePlugin()

Initializes a loaded plugin.

```cpp
auto result = pluginManager.initializePlugin("MyPlugin");
if (!result) {
    std::cerr << "Failed to initialize plugin: " << result.error().message() << std::endl;
}
```

#### activatePlugin()

Activates an initialized plugin.

```cpp
auto result = pluginManager.activatePlugin("MyPlugin");
if (result) {
    std::cout << "Plugin activated successfully" << std::endl;
}
```

#### deactivatePlugin()

Deactivates an active plugin.

```cpp
auto result = pluginManager.deactivatePlugin("MyPlugin");
if (result) {
    std::cout << "Plugin deactivated successfully" << std::endl;
}
```

## Configuration

### PluginManagerConfig

Configuration options for the PluginManager:

```cpp
struct PluginManagerConfig {
    // Plugin discovery
    std::vector<std::string> pluginDirectories;
    std::vector<std::string> pluginExtensions = {".qtplugin", ".dll", ".so", ".dylib"};
    
    // Loading behavior
    bool autoLoadPlugins = true;
    bool autoInitializePlugins = true;
    bool autoActivatePlugins = false;
    
    // Dependency resolution
    bool resolveDependencies = true;
    bool allowCircularDependencies = false;
    
    // Error handling
    bool continueOnError = true;
    int maxRetryAttempts = 3;
    
    // Security
    bool validatePluginSignatures = false;
    std::vector<std::string> trustedPublishers;
    
    // Performance
    bool enablePluginCaching = true;
    size_t maxConcurrentLoads = 4;
};
```

### Usage Example

```cpp
PluginManagerConfig config;
config.pluginDirectories = {"plugins/", "extensions/"};
config.autoLoadPlugins = true;
config.resolveDependencies = true;
config.validatePluginSignatures = true;

PluginManager manager(config);
```

## Event Handling

### IPluginEventHandler

Interface for handling plugin events:

```cpp
class IPluginEventHandler {
public:
    virtual ~IPluginEventHandler() = default;
    
    virtual void onPluginLoaded(const std::string& name, std::shared_ptr<IPlugin> plugin) {}
    virtual void onPluginUnloaded(const std::string& name) {}
    virtual void onPluginInitialized(const std::string& name) {}
    virtual void onPluginActivated(const std::string& name) {}
    virtual void onPluginDeactivated(const std::string& name) {}
    virtual void onPluginError(const std::string& name, const Error& error) {}
};
```

### Event Handler Example

```cpp
class MyEventHandler : public IPluginEventHandler {
public:
    void onPluginLoaded(const std::string& name, std::shared_ptr<IPlugin> plugin) override {
        std::cout << "Plugin loaded: " << name << std::endl;
    }
    
    void onPluginError(const std::string& name, const Error& error) override {
        std::cerr << "Plugin error in " << name << ": " << error.message() << std::endl;
    }
};

// Set event handler
auto handler = std::make_shared<MyEventHandler>();
pluginManager.setEventHandler(handler);
```

## Dependency Management

The PluginManager automatically resolves plugin dependencies based on the dependency information provided by plugins.

### Dependency Resolution

```cpp
// Plugin declares dependencies
class MyPlugin : public IPlugin {
public:
    std::vector<std::string> dependencies() const override {
        return {"CorePlugin >= 1.0.0", "UtilsPlugin >= 2.1.0"};
    }
};

// PluginManager resolves dependencies automatically
auto result = pluginManager.loadPluginsFromDirectory("plugins/");
// Dependencies are loaded and initialized in correct order
```

### Circular Dependency Detection

```cpp
// Configure to detect circular dependencies
PluginManagerConfig config;
config.allowCircularDependencies = false;  // Default

PluginManager manager(config);
// Will fail if circular dependencies are detected
```

## Error Handling

### Common Error Types

```cpp
enum class PluginManagerError {
    PluginNotFound,
    LoadingFailed,
    InitializationFailed,
    DependencyNotFound,
    CircularDependency,
    InvalidPlugin,
    SecurityViolation
};
```

### Error Handling Example

```cpp
auto result = pluginManager.loadPlugin("invalid_plugin.qtplugin");
if (!result) {
    auto error = result.error();
    switch (error.code()) {
        case PluginManagerError::PluginNotFound:
            std::cerr << "Plugin file not found" << std::endl;
            break;
        case PluginManagerError::LoadingFailed:
            std::cerr << "Failed to load plugin library" << std::endl;
            break;
        case PluginManagerError::InvalidPlugin:
            std::cerr << "Plugin does not implement required interface" << std::endl;
            break;
        default:
            std::cerr << "Unknown error: " << error.message() << std::endl;
    }
}
```

## Thread Safety

The PluginManager is thread-safe and can be used from multiple threads concurrently. However, individual plugin instances may not be thread-safe.

```cpp
// Safe to call from multiple threads
std::thread t1([&]() {
    pluginManager.loadPlugin("plugin1.qtplugin");
});

std::thread t2([&]() {
    pluginManager.loadPlugin("plugin2.qtplugin");
});

t1.join();
t2.join();
```

## Best Practices

1. **Error Handling**: Always check return values from PluginManager methods
2. **Configuration**: Configure the PluginManager appropriately for your use case
3. **Event Handling**: Use event handlers to monitor plugin lifecycle
4. **Dependencies**: Declare plugin dependencies explicitly
5. **Thread Safety**: Be aware of thread safety requirements for individual plugins

## See Also

- **[IPlugin Interface](plugin-interface.md)**: Base plugin interface
- **[Plugin Loader](plugin-loader.md)**: Low-level plugin loading
- **[Error Handling](../utils/error-handling.md)**: Error handling utilities
- **[Plugin Development Guide](../../user-guide/plugin-development.md)**: Plugin development guide
