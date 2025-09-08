# Quick Start Guide

This guide will get you up and running with QtForge v3.2.0 in just a few minutes. We'll create a simple application that loads and uses plugins, including the new multilingual support features.

## Prerequisites

Before starting, make sure you have:

- ✅ QtForge v3.2.0 installed (see [Installation Guide](installation.md))
- ✅ Qt 6.0+ with Core module
- ✅ C++20 compatible compiler
- ✅ CMake 3.21+
- ✅ Python 3.8+ (for Python bindings, optional)
- ✅ Lua 5.4+ (for Lua plugin support, optional)

## Step 1: Create Your First Application

Let's create a simple application that demonstrates QtPlugin's core functionality.

### Project Structure

Create the following directory structure:

```
my_qtplugin_app/
├── CMakeLists.txt
├── main.cpp
└── plugins/
    └── (we'll add plugins here later)
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.21)
project(MyQtPluginApp VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 and QtForge
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(QtForge REQUIRED COMPONENTS Core Security)

# Create the executable
add_executable(my_app main.cpp)

# Link libraries
target_link_libraries(my_app
    Qt6::Core
    QtForge::Core
    QtPlugin::Security
)

# Set output directory
set_target_properties(my_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
```

### main.cpp

```cpp
#include <qtplugin/qtplugin.hpp>
#include <QCoreApplication>
#include <QDir>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    std::cout << "=== QtPlugin Quick Start Demo ===" << std::endl;

    // Step 1: Initialize QtPlugin library
    std::cout << "1. Initializing QtPlugin library..." << std::endl;
    qtplugin::LibraryInitializer init;
    if (!init.is_initialized()) {
        std::cerr << "❌ Failed to initialize QtPlugin library" << std::endl;
        return -1;
    }
    std::cout << "✅ QtPlugin library initialized (version "
              << qtplugin::version_string() << ")" << std::endl;

    // Step 2: Create plugin manager
    std::cout << "\n2. Creating plugin manager..." << std::endl;
    auto manager = qtplugin::PluginManager::create();
    if (!manager) {
        std::cerr << "❌ Failed to create plugin manager" << std::endl;
        return -1;
    }
    std::cout << "✅ Plugin manager created successfully" << std::endl;

    // Step 3: Set up plugin directory
    QString pluginDir = QDir::currentPath() + "/plugins";
    std::cout << "\n3. Looking for plugins in: " << pluginDir.toStdString() << std::endl;

    // Step 4: Discover plugins
    QDir dir(pluginDir);
    if (!dir.exists()) {
        std::cout << "⚠️  Plugin directory doesn't exist yet" << std::endl;
        std::cout << "   Create some plugins using the 'First Plugin' guide!" << std::endl;
    } else {
        auto pluginFiles = dir.entryList(QStringList() << "*.so" << "*.dll" << "*.dylib",
                                        QDir::Files);

        if (pluginFiles.isEmpty()) {
            std::cout << "⚠️  No plugin files found" << std::endl;
            std::cout << "   Create some plugins using the 'First Plugin' guide!" << std::endl;
        } else {
            std::cout << "📦 Found " << pluginFiles.size() << " plugin file(s):" << std::endl;

            // Step 5: Load each plugin
            for (const QString& fileName : pluginFiles) {
                QString fullPath = dir.absoluteFilePath(fileName);
                std::cout << "\n5. Loading plugin: " << fileName.toStdString() << std::endl;

                auto result = manager->load_plugin(fullPath.toStdString());
                if (!result) {
                    std::cerr << "❌ Failed to load plugin: "
                              << result.error().message << std::endl;
                    continue;
                }

                std::cout << "✅ Plugin loaded with ID: " << result.value() << std::endl;

                // Step 6: Get plugin information
                auto plugin = manager->get_plugin(result.value());
                if (plugin) {
                    std::cout << "   Name: " << plugin->name() << std::endl;
                    std::cout << "   Version: " << plugin->version() << std::endl;
                    std::cout << "   Description: " << plugin->description() << std::endl;

                    // Step 7: Initialize the plugin
                    auto init_result = plugin->initialize();
                    if (init_result) {
                        std::cout << "✅ Plugin initialized successfully" << std::endl;

                        // Step 8: Execute a command (if supported)
                        auto commands = plugin->supported_commands();
                        if (!commands.empty()) {
                            std::cout << "   Available commands: ";
                            for (size_t i = 0; i < commands.size(); ++i) {
                                std::cout << commands[i];
                                if (i < commands.size() - 1) std::cout << ", ";
                            }
                            std::cout << std::endl;

                            // Try to execute the first command
                            auto cmd_result = plugin->execute_command(commands[0]);
                            if (cmd_result) {
                                std::cout << "✅ Command '" << commands[0]
                                          << "' executed successfully" << std::endl;
                            }
                        }
                    } else {
                        std::cerr << "❌ Failed to initialize plugin: "
                                  << init_result.error().message << std::endl;
                    }
                }
            }
        }
    }

    std::cout << "\n=== Demo Complete ===" << std::endl;
    std::cout << "Next steps:" << std::endl;
    std::cout << "1. Create your first plugin: see 'First Plugin' guide" << std::endl;
    std::cout << "2. Explore examples: see 'Examples' section" << std::endl;
    std::cout << "3. Read the API documentation for advanced usage" << std::endl;

    return 0;
}
```

## Step 2: Build and Run

### Build the Application

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Create plugins directory
mkdir -p bin/plugins
```

### Run the Application

```bash
# Run the application
./bin/my_app
```

You should see output like:

```
=== QtPlugin Quick Start Demo ===
1. Initializing QtPlugin library...
✅ QtPlugin library initialized (version 3.0.0)

2. Creating plugin manager...
✅ Plugin manager created successfully

3. Looking for plugins in: /path/to/your/app/build/bin/plugins
⚠️  No plugin files found
   Create some plugins using the 'First Plugin' guide!

=== Demo Complete ===
Next steps:
1. Create your first plugin: see 'First Plugin' guide
2. Explore examples: see 'Examples' section
3. Read the API documentation for advanced usage
```

## Step 3: Understanding the Code

Let's break down what the code does:

### 1. Library Initialization

```cpp
qtplugin::LibraryInitializer init;
```

This initializes the QtPlugin library and sets up internal systems. Always do this first!

### 2. Plugin Manager Creation

```cpp
auto manager = qtplugin::PluginManager::create();
```

The plugin manager is the central component that handles all plugin operations.

### 3. Plugin Loading

```cpp
auto result = manager->load_plugin(fullPath.toStdString());
```

This loads a plugin from a file. The result is either a plugin ID or an error.

### 4. Plugin Access

```cpp
auto plugin = manager->get_plugin(result.value());
```

Get a reference to the loaded plugin using its ID.

### 5. Plugin Initialization

```cpp
auto init_result = plugin->initialize();
```

Initialize the plugin so it's ready to use.

### 6. Command Execution

```cpp
auto cmd_result = plugin->execute_command(commands[0]);
```

Execute plugin-specific commands.

## Step 4: Error Handling

QtPlugin uses the modern `expected<T, E>` pattern for error handling:

```cpp
// Check if operation succeeded
if (result) {
    // Success - use result.value()
    std::cout << "Success: " << result.value() << std::endl;
} else {
    // Error - use result.error()
    std::cerr << "Error: " << result.error().message << std::endl;
}
```

This provides:

- **Type Safety**: Compile-time error checking
- **No Exceptions**: Explicit error handling
- **Performance**: Zero-cost abstractions

## Step 5: Next Steps

Now that you have a working QtPlugin application:

### Create Your First Plugin

Follow the [First Plugin Guide](first-plugin.md) to create a simple plugin that your application can load.

### Explore Examples

Check out the [Examples](../examples/index.md) to see more sophisticated usage patterns:

- **Basic Plugin**: Simple plugin with commands
- **Service Plugin**: Background processing
- **Network Plugin**: Network-enabled plugins
- **UI Plugin**: Plugins with user interfaces

### Learn More

- **[User Guide](../user-guide/plugin-management.md)**: Comprehensive usage guide
- **[API Reference](../api/index.md)**: Complete API documentation
- **[Developer Guide](../developer-guide/plugin-development.md)**: Advanced plugin development

## Common Patterns

### Plugin Discovery

```cpp
// Automatically discover plugins in a directory
auto plugins = manager->discover_plugins("/path/to/plugins");
for (const auto& plugin_path : plugins) {
    auto result = manager->load_plugin(plugin_path);
    // Handle result...
}
```

### Configuration

```cpp
// Configure a plugin
QJsonObject config;
config["setting1"] = "value1";
config["setting2"] = 42;

auto result = plugin->configure(config);
if (result) {
    std::cout << "Plugin configured successfully" << std::endl;
}
```

### Cleanup

```cpp
// Proper cleanup (automatic with RAII)
{
    qtplugin::LibraryInitializer init;
    auto manager = qtplugin::PluginManager::create();
    // ... use plugins ...
} // Automatic cleanup when going out of scope
```

## Troubleshooting

### Plugin Not Loading

1. **Check file permissions**: Ensure the plugin file is readable
2. **Verify dependencies**: Make sure all required libraries are available
3. **Check architecture**: Plugin must match your application's architecture (32/64-bit)
4. **Debug symbols**: Build in Debug mode for more detailed error messages

### Initialization Failures

1. **Qt version**: Ensure Qt 6.0+ is available
2. **Missing components**: Check that required Qt modules are installed
3. **Permissions**: Verify write access to temporary directories

### Build Issues

1. **CMake version**: Ensure CMake 3.21+ is installed
2. **Compiler support**: Verify C++20 support in your compiler
3. **Qt detection**: Set `Qt6_DIR` if Qt is not found automatically

Ready to create your first plugin? Continue to the [First Plugin Guide](first-plugin.md)!
