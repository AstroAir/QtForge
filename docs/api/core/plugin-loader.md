# PluginLoader

The `PluginLoader` class provides low-level functionality for loading and unloading plugin libraries dynamically at runtime.

## Overview

The PluginLoader is responsible for:
- Dynamic library loading and unloading
- Symbol resolution and function pointer retrieval
- Platform-specific library handling
- Error reporting and diagnostics
- Plugin validation and verification

## Class Declaration

```cpp
namespace qtforge::core {

class QTFORGE_EXPORT PluginLoader {
public:
    // Construction and destruction
    PluginLoader();
    explicit PluginLoader(const std::string& libraryPath);
    ~PluginLoader();
    
    // Move semantics (non-copyable)
    PluginLoader(PluginLoader&& other) noexcept;
    PluginLoader& operator=(PluginLoader&& other) noexcept;
    
    // Library loading
    expected<void, Error> load(const std::string& libraryPath);
    expected<void, Error> unload();
    bool isLoaded() const;
    
    // Symbol resolution
    template<typename T>
    expected<T, Error> getSymbol(const std::string& symbolName) const;
    expected<void*, Error> getSymbolAddress(const std::string& symbolName) const;
    
    // Plugin factory functions
    expected<CreatePluginFunc, Error> getCreateFunction() const;
    expected<DestroyPluginFunc, Error> getDestroyFunction() const;
    expected<GetPluginInfoFunc, Error> getInfoFunction() const;
    
    // Library information
    std::string getLibraryPath() const;
    std::string getLibraryName() const;
    std::vector<std::string> getExportedSymbols() const;
    
    // Validation
    expected<bool, Error> validatePlugin() const;
    expected<PluginInfo, Error> getPluginInfo() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

// Plugin factory function types
using CreatePluginFunc = IPlugin*(*)();
using DestroyPluginFunc = void(*)(IPlugin*);
using GetPluginInfoFunc = PluginInfo*(*)();

} // namespace qtforge::core
```

## Core Methods

### Library Loading

#### load()

Loads a dynamic library from the specified path.

```cpp
PluginLoader loader;
auto result = loader.load("plugins/my_plugin.qtplugin");
if (result) {
    std::cout << "Library loaded successfully" << std::endl;
} else {
    std::cerr << "Failed to load library: " << result.error().message() << std::endl;
}
```

**Parameters:**
- `libraryPath`: Path to the dynamic library file

**Returns:**
- `expected<void, Error>`: Success or error information

#### unload()

Unloads the currently loaded library.

```cpp
auto result = loader.unload();
if (result) {
    std::cout << "Library unloaded successfully" << std::endl;
} else {
    std::cerr << "Failed to unload library: " << result.error().message() << std::endl;
}
```

**Returns:**
- `expected<void, Error>`: Success or error information

### Symbol Resolution

#### getSymbol()

Retrieves a typed symbol from the loaded library.

```cpp
// Get a function pointer
auto createFunc = loader.getSymbol<CreatePluginFunc>("createPlugin");
if (createFunc) {
    auto plugin = createFunc.value()();
    // Use plugin...
} else {
    std::cerr << "Symbol not found: " << createFunc.error().message() << std::endl;
}

// Get a variable
auto version = loader.getSymbol<int*>("plugin_version");
if (version) {
    std::cout << "Plugin version: " << *version.value() << std::endl;
}
```

**Template Parameters:**
- `T`: Type of the symbol to retrieve

**Parameters:**
- `symbolName`: Name of the symbol to resolve

**Returns:**
- `expected<T, Error>`: Typed symbol or error

#### getSymbolAddress()

Retrieves the raw address of a symbol.

```cpp
auto address = loader.getSymbolAddress("createPlugin");
if (address) {
    void* funcPtr = address.value();
    // Cast to appropriate function pointer type
    auto createFunc = reinterpret_cast<CreatePluginFunc>(funcPtr);
}
```

### Plugin Factory Functions

#### getCreateFunction()

Retrieves the plugin creation function.

```cpp
auto createFunc = loader.getCreateFunction();
if (createFunc) {
    auto plugin = std::unique_ptr<IPlugin>(createFunc.value()());
    std::cout << "Created plugin: " << plugin->name() << std::endl;
}
```

#### getDestroyFunction()

Retrieves the plugin destruction function.

```cpp
auto destroyFunc = loader.getDestroyFunction();
if (destroyFunc && plugin) {
    destroyFunc.value()(plugin.release());
}
```

#### getInfoFunction()

Retrieves the plugin information function.

```cpp
auto infoFunc = loader.getInfoFunction();
if (infoFunc) {
    auto info = std::unique_ptr<PluginInfo>(infoFunc.value()());
    std::cout << "Plugin: " << info->name << " v" << info->version << std::endl;
}
```

## Plugin Validation

### validatePlugin()

Validates that the loaded library is a valid QtForge plugin.

```cpp
auto isValid = loader.validatePlugin();
if (isValid && isValid.value()) {
    std::cout << "Valid QtForge plugin" << std::endl;
} else if (isValid) {
    std::cout << "Not a valid QtForge plugin" << std::endl;
} else {
    std::cerr << "Validation error: " << isValid.error().message() << std::endl;
}
```

**Returns:**
- `expected<bool, Error>`: True if valid plugin, false if not, or error

### getPluginInfo()

Retrieves plugin metadata without creating a plugin instance.

```cpp
auto info = loader.getPluginInfo();
if (info) {
    const auto& pluginInfo = info.value();
    std::cout << "Name: " << pluginInfo.name << std::endl;
    std::cout << "Version: " << pluginInfo.version << std::endl;
    std::cout << "Description: " << pluginInfo.description << std::endl;
}
```

## Platform-Specific Behavior

### Windows

```cpp
// Windows-specific library extensions
std::vector<std::string> extensions = {".dll", ".qtplugin"};

// Windows-specific error handling
auto result = loader.load("plugin.dll");
if (!result) {
    auto error = result.error();
    if (error.code() == LoaderError::LibraryNotFound) {
        // Check if Visual C++ Redistributable is installed
        // Check PATH environment variable
    }
}
```

### Linux

```cpp
// Linux-specific library extensions
std::vector<std::string> extensions = {".so", ".qtplugin"};

// Linux-specific library paths
std::vector<std::string> searchPaths = {
    "/usr/lib/qtforge/plugins/",
    "/usr/local/lib/qtforge/plugins/",
    "./plugins/"
};
```

### macOS

```cpp
// macOS-specific library extensions
std::vector<std::string> extensions = {".dylib", ".qtplugin"};

// macOS-specific bundle handling
auto result = loader.load("MyPlugin.qtplugin/Contents/MacOS/MyPlugin");
```

## Error Handling

### Common Error Types

```cpp
enum class LoaderError {
    LibraryNotFound,
    LoadingFailed,
    SymbolNotFound,
    InvalidLibrary,
    PlatformError,
    PermissionDenied,
    DependencyMissing
};
```

### Error Handling Example

```cpp
auto result = loader.load("nonexistent_plugin.qtplugin");
if (!result) {
    auto error = result.error();
    switch (error.code()) {
        case LoaderError::LibraryNotFound:
            std::cerr << "Plugin file not found" << std::endl;
            break;
        case LoaderError::LoadingFailed:
            std::cerr << "Failed to load library: " << error.message() << std::endl;
            break;
        case LoaderError::DependencyMissing:
            std::cerr << "Missing dependencies: " << error.details() << std::endl;
            break;
        default:
            std::cerr << "Unknown error: " << error.message() << std::endl;
    }
}
```

## Advanced Usage

### Custom Symbol Loading

```cpp
class CustomPluginLoader : public PluginLoader {
public:
    expected<CustomInterface*, Error> getCustomInterface() {
        return getSymbol<CustomInterface*>("getCustomInterface");
    }
    
    expected<void, Error> validateCustomPlugin() {
        // Custom validation logic
        auto interface = getCustomInterface();
        if (!interface) {
            return Error("Custom interface not found");
        }
        
        // Additional validation...
        return {};
    }
};
```

### Plugin Metadata Extraction

```cpp
struct ExtendedPluginInfo {
    std::string name;
    std::string version;
    std::string description;
    std::vector<std::string> dependencies;
    std::vector<std::string> capabilities;
    std::map<std::string, std::string> metadata;
};

expected<ExtendedPluginInfo, Error> getExtendedInfo(const PluginLoader& loader) {
    auto getExtendedInfoFunc = loader.getSymbol<ExtendedPluginInfo*(*)()>("getExtendedPluginInfo");
    if (!getExtendedInfoFunc) {
        return Error("Extended info function not found");
    }
    
    auto info = std::unique_ptr<ExtendedPluginInfo>(getExtendedInfoFunc.value()());
    return *info;
}
```

## Thread Safety

The PluginLoader is not thread-safe. If you need to use it from multiple threads, provide external synchronization.

```cpp
class ThreadSafePluginLoader {
private:
    PluginLoader loader_;
    mutable std::mutex mutex_;
    
public:
    expected<void, Error> load(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        return loader_.load(path);
    }
    
    template<typename T>
    expected<T, Error> getSymbol(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return loader_.getSymbol<T>(name);
    }
};
```

## Best Practices

1. **Error Handling**: Always check return values and handle errors appropriately
2. **Resource Management**: Ensure proper cleanup by unloading libraries
3. **Symbol Validation**: Validate that required symbols exist before using them
4. **Platform Awareness**: Handle platform-specific differences appropriately
5. **Thread Safety**: Provide synchronization if using from multiple threads

## See Also

- **[Plugin Manager](plugin-manager.md)**: High-level plugin management
- **[IPlugin Interface](plugin-interface.md)**: Base plugin interface
- **[Error Handling](../utils/error-handling.md)**: Error handling utilities
- **[Plugin Development Guide](../../user-guide/plugin-development.md)**: Plugin development guide
