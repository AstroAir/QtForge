# Your First Plugin

This guide walks you through creating your first QtPlugin plugin from scratch. By the end, you'll have a working plugin that demonstrates the core concepts and can be loaded by any QtPlugin application.

## What We'll Build

We'll create a simple "Hello World" plugin that:

- ✅ Implements the `IPlugin` interface
- ✅ Provides metadata and identification
- ✅ Supports basic commands
- ✅ Handles configuration
- ✅ Demonstrates proper lifecycle management

## Prerequisites

Before starting, ensure you have:

- ✅ QtPlugin installed (see [Installation Guide](installation.md))
- ✅ Basic understanding of C++ and Qt
- ✅ CMake 3.21+ and a C++20 compiler
- ✅ A working QtPlugin application (from [Quick Start](quick-start.md))

## Step 1: Project Setup

### Create Project Structure

```bash
mkdir my_first_plugin
cd my_first_plugin

# Create directory structure
mkdir -p src include
touch CMakeLists.txt
touch src/hello_plugin.cpp
touch include/hello_plugin.hpp
touch metadata.json
```

Your project structure should look like:

```
my_first_plugin/
├── CMakeLists.txt
├── metadata.json
├── include/
│   └── hello_plugin.hpp
└── src/
    └── hello_plugin.cpp
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.21)
project(HelloPlugin VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(QtPlugin REQUIRED COMPONENTS Core)

# Create the plugin
qtplugin_add_plugin(hello_plugin
    TYPE service
    SOURCES 
        src/hello_plugin.cpp
    HEADERS 
        include/hello_plugin.hpp
    METADATA 
        metadata.json
    INCLUDE_DIRECTORIES
        include
    DEPENDENCIES
        Qt6::Core
        QtPlugin::Core
)

# Set output directory
set_target_properties(hello_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
```

## Step 2: Plugin Metadata

### metadata.json

```json
{
    "id": "com.example.hello-plugin",
    "name": "Hello World Plugin",
    "version": "1.0.0",
    "description": "A simple Hello World plugin demonstrating QtPlugin basics",
    "author": "Your Name",
    "license": "MIT",
    "qtplugin_version": "3.0.0",
    "qt_version": "6.0.0",
    "tags": ["example", "tutorial", "hello-world"],
    "capabilities": ["service"],
    "dependencies": [],
    "custom_data": {
        "category": "Examples",
        "documentation_url": "https://example.com/docs"
    }
}
```

## Step 3: Plugin Header

### include/hello_plugin.hpp

```cpp
#pragma once

#include <qtplugin/core/plugin_interface.hpp>
#include <QObject>
#include <QJsonObject>
#include <QElapsedTimer>
#include <memory>

class HelloPlugin : public QObject, public qtplugin::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "metadata.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit HelloPlugin(QObject* parent = nullptr);
    ~HelloPlugin() override;

    // Lifecycle management
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override;
    bool is_initialized() const noexcept override;

    // Metadata and identification
    qtplugin::PluginMetadata metadata() const override;
    std::string id() const noexcept override;
    std::string name() const noexcept override;
    std::string version() const noexcept override;
    std::string description() const noexcept override;
    std::string author() const noexcept override;

    // Configuration management
    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;
    QJsonObject current_configuration() const override;

    // Command execution
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, 
        const QJsonObject& params = {}
    ) override;

    // Capability queries
    std::vector<std::string> supported_commands() const override;
    bool supports_command(std::string_view command) const override;
    qtplugin::PluginCapabilities capabilities() const noexcept override;

private:
    // Helper methods
    void setup_default_configuration();
    QJsonObject create_hello_response(const QString& name) const;
    QJsonObject create_status_response() const;

    // Member variables
    qtplugin::PluginState m_state;
    bool m_initialized;
    QJsonObject m_configuration;
    QElapsedTimer m_uptime_timer;
    int m_command_count;
    QString m_greeting_prefix;
};
```

## Step 4: Plugin Implementation

### src/hello_plugin.cpp

```cpp
#include "hello_plugin.hpp"
#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>

HelloPlugin::HelloPlugin(QObject* parent)
    : QObject(parent)
    , m_state(qtplugin::PluginState::Unloaded)
    , m_initialized(false)
    , m_command_count(0)
    , m_greeting_prefix("Hello")
{
    qDebug() << "HelloPlugin: Constructor called";
    setup_default_configuration();
}

HelloPlugin::~HelloPlugin()
{
    if (m_initialized) {
        shutdown();
    }
    qDebug() << "HelloPlugin: Destructor called";
}

// Lifecycle Management
qtplugin::expected<void, qtplugin::PluginError> HelloPlugin::initialize()
{
    if (m_initialized) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::AlreadyInitialized,
            "Plugin is already initialized"
        });
    }

    try {
        qDebug() << "HelloPlugin: Initializing...";
        
        // Start uptime timer
        m_uptime_timer.start();
        
        // Reset counters
        m_command_count = 0;
        
        // Set state
        m_initialized = true;
        m_state = qtplugin::PluginState::Running;
        
        qDebug() << "HelloPlugin: Initialization completed successfully";
        return {};
        
    } catch (const std::exception& e) {
        m_state = qtplugin::PluginState::Error;
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::InitializationFailed,
            std::string("Initialization failed: ") + e.what()
        });
    }
}

void HelloPlugin::shutdown() noexcept
{
    try {
        qDebug() << "HelloPlugin: Shutting down...";
        
        if (m_initialized) {
            // Cleanup resources
            m_command_count = 0;
            m_initialized = false;
        }
        
        m_state = qtplugin::PluginState::Unloaded;
        qDebug() << "HelloPlugin: Shutdown completed";
        
    } catch (...) {
        qWarning() << "HelloPlugin: Error during shutdown (ignored)";
    }
}

qtplugin::PluginState HelloPlugin::state() const noexcept
{
    return m_state;
}

bool HelloPlugin::is_initialized() const noexcept
{
    return m_initialized;
}

// Metadata
qtplugin::PluginMetadata HelloPlugin::metadata() const
{
    qtplugin::PluginMetadata meta;
    meta.id = id();
    meta.name = name();
    meta.version = version();
    meta.description = description();
    meta.author = author();
    meta.license = "MIT";
    meta.tags = {"example", "tutorial", "hello-world"};
    meta.capabilities = capabilities();
    
    // Custom data
    meta.custom_data = QJsonObject{
        {"category", "Examples"},
        {"uptime_ms", m_uptime_timer.isValid() ? m_uptime_timer.elapsed() : 0},
        {"command_count", m_command_count}
    };
    
    return meta;
}

std::string HelloPlugin::id() const noexcept
{
    return "com.example.hello-plugin";
}

std::string HelloPlugin::name() const noexcept
{
    return "Hello World Plugin";
}

std::string HelloPlugin::version() const noexcept
{
    return "1.0.0";
}

std::string HelloPlugin::description() const noexcept
{
    return "A simple Hello World plugin demonstrating QtPlugin basics";
}

std::string HelloPlugin::author() const noexcept
{
    return "Your Name";
}

// Configuration
qtplugin::expected<void, qtplugin::PluginError> HelloPlugin::configure(const QJsonObject& config)
{
    try {
        qDebug() << "HelloPlugin: Applying configuration...";
        
        // Validate configuration
        if (config.contains("greeting_prefix")) {
            if (!config["greeting_prefix"].isString()) {
                return qtplugin::make_unexpected(qtplugin::PluginError{
                    qtplugin::PluginErrorCode::InvalidConfiguration,
                    "greeting_prefix must be a string"
                });
            }
            m_greeting_prefix = config["greeting_prefix"].toString();
        }
        
        // Store configuration
        m_configuration = config;
        
        qDebug() << "HelloPlugin: Configuration applied successfully";
        return {};
        
    } catch (const std::exception& e) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::ConfigurationFailed,
            std::string("Configuration failed: ") + e.what()
        });
    }
}

QJsonObject HelloPlugin::current_configuration() const
{
    return m_configuration;
}

// Command Execution
qtplugin::expected<QJsonObject, qtplugin::PluginError> HelloPlugin::execute_command(
    std::string_view command, 
    const QJsonObject& params)
{
    if (!m_initialized) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::NotInitialized,
            "Plugin is not initialized"
        });
    }

    ++m_command_count;
    
    try {
        if (command == "hello") {
            QString name = params.value("name").toString("World");
            return create_hello_response(name);
            
        } else if (command == "status") {
            return create_status_response();
            
        } else if (command == "echo") {
            QJsonObject result;
            result["echo"] = params;
            result["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
            return result;
            
        } else if (command == "config") {
            return current_configuration();
            
        } else {
            return qtplugin::make_unexpected(qtplugin::PluginError{
                qtplugin::PluginErrorCode::UnknownCommand,
                std::string("Unknown command: ") + std::string(command)
            });
        }
        
    } catch (const std::exception& e) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::CommandExecutionFailed,
            std::string("Command execution failed: ") + e.what()
        });
    }
}

// Capabilities
std::vector<std::string> HelloPlugin::supported_commands() const
{
    return {"hello", "status", "echo", "config"};
}

bool HelloPlugin::supports_command(std::string_view command) const
{
    auto commands = supported_commands();
    return std::find(commands.begin(), commands.end(), command) != commands.end();
}

qtplugin::PluginCapabilities HelloPlugin::capabilities() const noexcept
{
    return qtplugin::PluginCapability::Service | qtplugin::PluginCapability::ThreadSafe;
}

// Helper Methods
void HelloPlugin::setup_default_configuration()
{
    m_configuration = QJsonObject{
        {"greeting_prefix", "Hello"},
        {"enable_timestamps", true},
        {"max_name_length", 50}
    };
    
    m_greeting_prefix = m_configuration["greeting_prefix"].toString();
}

QJsonObject HelloPlugin::create_hello_response(const QString& name) const
{
    QJsonObject result;
    result["message"] = QString("%1, %2!").arg(m_greeting_prefix, name);
    result["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    result["plugin_id"] = QString::fromStdString(id());
    result["command_count"] = m_command_count;
    
    return result;
}

QJsonObject HelloPlugin::create_status_response() const
{
    QJsonObject result;
    result["state"] = static_cast<int>(m_state);
    result["initialized"] = m_initialized;
    result["uptime_ms"] = m_uptime_timer.elapsed();
    result["command_count"] = m_command_count;
    result["plugin_id"] = QString::fromStdString(id());
    result["version"] = QString::fromStdString(version());
    
    return result;
}

// Include the MOC file
#include "hello_plugin.moc"
```

## Step 5: Build the Plugin

### Build Commands

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Check output
ls -la plugins/
```

You should see output like:
```
plugins/
├── hello_plugin.so          # Linux
├── hello_plugin.dll         # Windows
└── hello_plugin.dylib       # macOS
```

### Troubleshooting Build Issues

If you encounter build errors:

1. **MOC Issues**:
   ```bash
   # Clean and rebuild
   rm -rf build
   mkdir build && cd build
   cmake .. && cmake --build .
   ```

2. **Missing QtPlugin**:
   ```cmake
   # Add to CMakeLists.txt if needed
   find_package(QtPlugin REQUIRED COMPONENTS Core)
   ```

3. **C++20 Issues**:
   ```cmake
   # Ensure C++20 is properly set
   set(CMAKE_CXX_STANDARD 20)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   ```

## Step 6: Test Your Plugin

### Create Test Application

Create `test_hello_plugin.cpp`:

```cpp
#include <qtplugin/qtplugin.hpp>
#include <QCoreApplication>
#include <QDir>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    std::cout << "=== Testing Hello Plugin ===" << std::endl;

    // Initialize QtPlugin
    qtplugin::LibraryInitializer init;
    if (!init.is_initialized()) {
        std::cerr << "Failed to initialize QtPlugin" << std::endl;
        return -1;
    }

    // Create plugin manager
    auto manager = qtplugin::PluginManager::create();
    if (!manager) {
        std::cerr << "Failed to create plugin manager" << std::endl;
        return -1;
    }

    // Load the plugin
    QString plugin_path = QDir::currentPath() + "/plugins/hello_plugin";
#ifdef Q_OS_WIN
    plugin_path += ".dll";
#elif defined(Q_OS_MAC)
    plugin_path += ".dylib";
#else
    plugin_path += ".so";
#endif

    std::cout << "Loading plugin: " << plugin_path.toStdString() << std::endl;

    auto result = manager->load_plugin(plugin_path.toStdString());
    if (!result) {
        std::cerr << "Failed to load plugin: " << result.error().message << std::endl;
        return -1;
    }

    std::cout << "✅ Plugin loaded with ID: " << result.value() << std::endl;

    // Get plugin instance
    auto plugin = manager->get_plugin(result.value());
    if (!plugin) {
        std::cerr << "Failed to get plugin instance" << std::endl;
        return -1;
    }

    // Display plugin information
    std::cout << "\n=== Plugin Information ===" << std::endl;
    std::cout << "Name: " << plugin->name() << std::endl;
    std::cout << "Version: " << plugin->version() << std::endl;
    std::cout << "Description: " << plugin->description() << std::endl;
    std::cout << "Author: " << plugin->author() << std::endl;

    // Initialize plugin
    auto init_result = plugin->initialize();
    if (!init_result) {
        std::cerr << "Failed to initialize plugin: "
                  << init_result.error().message << std::endl;
        return -1;
    }

    std::cout << "✅ Plugin initialized successfully" << std::endl;

    // Test commands
    std::cout << "\n=== Testing Commands ===" << std::endl;

    auto commands = plugin->supported_commands();
    std::cout << "Supported commands: ";
    for (size_t i = 0; i < commands.size(); ++i) {
        std::cout << commands[i];
        if (i < commands.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    // Test hello command
    std::cout << "\n1. Testing 'hello' command:" << std::endl;
    QJsonObject hello_params;
    hello_params["name"] = "QtPlugin User";

    auto hello_result = plugin->execute_command("hello", hello_params);
    if (hello_result) {
        QJsonDocument doc(hello_result.value());
        std::cout << "   Result: " << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
    } else {
        std::cout << "   Error: " << hello_result.error().message << std::endl;
    }

    // Test status command
    std::cout << "\n2. Testing 'status' command:" << std::endl;
    auto status_result = plugin->execute_command("status");
    if (status_result) {
        QJsonDocument doc(status_result.value());
        std::cout << "   Result: " << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
    } else {
        std::cout << "   Error: " << status_result.error().message << std::endl;
    }

    // Test configuration
    std::cout << "\n3. Testing configuration:" << std::endl;
    QJsonObject config;
    config["greeting_prefix"] = "Hi";

    auto config_result = plugin->configure(config);
    if (config_result) {
        std::cout << "   ✅ Configuration applied" << std::endl;

        // Test hello again with new configuration
        auto hello_result2 = plugin->execute_command("hello", hello_params);
        if (hello_result2) {
            QJsonDocument doc(hello_result2.value());
            std::cout << "   New greeting: " << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
        }
    } else {
        std::cout << "   Error: " << config_result.error().message << std::endl;
    }

    std::cout << "\n=== Test Complete ===" << std::endl;
    return 0;
}
```

### Add Test to CMakeLists.txt

```cmake
# Add test executable
add_executable(test_hello_plugin test_hello_plugin.cpp)
target_link_libraries(test_hello_plugin
    Qt6::Core
    QtPlugin::Core
)

# Set output directory
set_target_properties(test_hello_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
```

### Run the Test

```bash
# Build test
cmake --build .

# Run test
./test_hello_plugin
```

Expected output:
```
=== Testing Hello Plugin ===
Loading plugin: /path/to/build/plugins/hello_plugin.so
✅ Plugin loaded with ID: com.example.hello-plugin

=== Plugin Information ===
Name: Hello World Plugin
Version: 1.0.0
Description: A simple Hello World plugin demonstrating QtPlugin basics
Author: Your Name
✅ Plugin initialized successfully

=== Testing Commands ===
Supported commands: hello, status, echo, config

1. Testing 'hello' command:
   Result: {"message":"Hello, QtPlugin User!","timestamp":"2024-01-15T10:30:45","plugin_id":"com.example.hello-plugin","command_count":1}

2. Testing 'status' command:
   Result: {"state":2,"initialized":true,"uptime_ms":1234,"command_count":2,"plugin_id":"com.example.hello-plugin","version":"1.0.0"}

3. Testing configuration:
   ✅ Configuration applied
   New greeting: {"message":"Hi, QtPlugin User!","timestamp":"2024-01-15T10:30:45","plugin_id":"com.example.hello-plugin","command_count":3}

=== Test Complete ===
```

## Step 7: Understanding the Code

### Key Concepts Demonstrated

1. **Plugin Interface Implementation**:
   - All plugins must inherit from `qtplugin::IPlugin`
   - Use Qt's plugin system with `Q_OBJECT`, `Q_PLUGIN_METADATA`, and `Q_INTERFACES`

2. **Error Handling**:
   - Use `expected<T, E>` for operations that can fail
   - Provide meaningful error messages and codes

3. **Lifecycle Management**:
   - Proper initialization and shutdown procedures
   - State tracking and validation

4. **Command System**:
   - Flexible command execution with JSON parameters
   - Type-safe parameter handling

5. **Configuration**:
   - Dynamic configuration with validation
   - Persistent configuration storage

### Best Practices Shown

- **RAII**: Automatic resource management
- **const correctness**: Proper use of const methods
- **Exception safety**: Proper exception handling
- **Thread safety**: Safe for concurrent access
- **Documentation**: Clear code documentation

## Next Steps

Congratulations! You've created your first QtPlugin plugin. Here's what to explore next:

### 🚀 Enhance Your Plugin

1. **Add More Commands**: Implement additional functionality
2. **Persistent Storage**: Save data between sessions
3. **Background Processing**: Add timer-based operations
4. **Inter-Plugin Communication**: Use the message bus

### 📚 Learn More

1. **[Developer Guide](../developer-guide/plugin-development.md)**: Advanced plugin development
2. **[API Reference](../api/index.md)**: Complete API documentation
3. **[Examples](../examples/index.md)**: More complex examples
4. **[Best Practices](../developer-guide/best-practices.md)**: Production guidelines

### 🔧 Advanced Features

1. **Security**: Add plugin validation and permissions
2. **Hot Reloading**: Support runtime plugin updates
3. **Dependencies**: Handle plugin dependencies
4. **UI Integration**: Create plugins with user interfaces

### 🧪 Testing

1. **Unit Tests**: Write comprehensive tests for your plugin
2. **Integration Tests**: Test plugin interactions
3. **Performance Tests**: Optimize plugin performance

Ready to dive deeper? Continue with the [Developer Guide](../developer-guide/plugin-development.md) for advanced plugin development techniques!
