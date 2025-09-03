# Plugin Development Guide

This guide covers everything you need to know to develop plugins for the QtForge system.

## Getting Started

### Prerequisites

- Qt 6.0 or later
- C++20 compatible compiler
- CMake 3.21 or later
- QtForge library installed

### Plugin Structure

A typical plugin consists of:

- **Plugin class**: Implements `qtplugin::IPlugin` interface
- **Metadata file**: JSON file describing the plugin
- **CMakeLists.txt**: Build configuration
- **Resources**: Any additional files needed by the plugin

## Creating Your First Plugin

### Step 1: Plugin Class Implementation

Create a header file `my_plugin.hpp`:

```cpp
#pragma once

#include <qtplugin/core/plugin_interface.hpp>
#include <QObject>

class MyPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "metadata.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    MyPlugin(QObject* parent = nullptr);
    ~MyPlugin() override;

    // IPlugin interface implementation
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() override;
    qtplugin::PluginState state() const override;
    bool is_initialized() const override;

    qtplugin::PluginMetadata metadata() const override;
    std::string id() const override;
    std::string name() const override;
    std::string version() const override;
    std::string description() const override;

    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;
    QJsonObject current_configuration() const override;

    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}
    ) override;

    std::vector<std::string> supported_commands() const override;
    bool supports_command(std::string_view command) const override;
    qtplugin::PluginCapabilities capabilities() const override;

private:
    qtplugin::PluginState m_state = qtplugin::PluginState::Unloaded;
    QJsonObject m_configuration;
    bool m_initialized = false;
};
```

### Step 2: Plugin Implementation

Create the implementation file `my_plugin.cpp`:

```cpp
#include "my_plugin.hpp"
#include <QDebug>
#include <QJsonDocument>

MyPlugin::MyPlugin(QObject* parent)
    : QObject(parent)
{
    qDebug() << "MyPlugin constructor called";
}

MyPlugin::~MyPlugin()
{
    if (m_initialized) {
        shutdown();
    }
    qDebug() << "MyPlugin destructor called";
}

qtplugin::expected<void, qtplugin::PluginError> MyPlugin::initialize()
{
    if (m_initialized) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::InitializationFailed,
            "Plugin already initialized"
        });
    }

    try {
        // Perform initialization logic here
        qDebug() << "Initializing MyPlugin...";

        // Example: Setup internal state, connect to services, etc.
        m_state = qtplugin::PluginState::Running;
        m_initialized = true;

        qDebug() << "MyPlugin initialized successfully";
        return {};
    }
    catch (const std::exception& e) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::InitializationFailed,
            std::string("Initialization failed: ") + e.what()
        });
    }
}

void MyPlugin::shutdown()
{
    if (!m_initialized) {
        return;
    }

    qDebug() << "Shutting down MyPlugin...";

    // Perform cleanup logic here
    m_state = qtplugin::PluginState::Unloaded;
    m_initialized = false;

    qDebug() << "MyPlugin shutdown complete";
}

qtplugin::PluginState MyPlugin::state() const
{
    return m_state;
}

bool MyPlugin::is_initialized() const
{
    return m_initialized;
}

qtplugin::PluginMetadata MyPlugin::metadata() const
{
    return {
        .id = "com.example.myplugin",
        .name = "My Example Plugin",
        .version = "1.0.0",
        .description = "An example plugin demonstrating the QtPlugin system",
        .author = "Your Name",
        .license = "MIT",
        .dependencies = {},
        .tags = {"example", "demo"},
        .custom_data = {}
    };
}

std::string MyPlugin::id() const
{
    return metadata().id;
}

std::string MyPlugin::name() const
{
    return metadata().name;
}

std::string MyPlugin::version() const
{
    return metadata().version;
}

std::string MyPlugin::description() const
{
    return metadata().description;
}

qtplugin::expected<void, qtplugin::PluginError> MyPlugin::configure(const QJsonObject& config)
{
    qDebug() << "Configuring MyPlugin with:" << QJsonDocument(config).toJson();

    // Validate configuration
    if (config.contains("invalid_key")) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::ConfigurationError,
            "Invalid configuration key: invalid_key"
        });
    }

    m_configuration = config;
    return {};
}

QJsonObject MyPlugin::current_configuration() const
{
    return m_configuration;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError> MyPlugin::execute_command(
    std::string_view command,
    const QJsonObject& params)
{
    if (!m_initialized) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::ExecutionFailed,
            "Plugin not initialized"
        });
    }

    if (command == "hello") {
        QString name = params.value("name").toString("World");
        QJsonObject result;
        result["message"] = QString("Hello, %1!").arg(name);
        result["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        return result;
    }
    else if (command == "status") {
        QJsonObject result;
        result["state"] = static_cast<int>(m_state);
        result["initialized"] = m_initialized;
        result["uptime"] = "unknown"; // You could track this
        return result;
    }
    else {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::CommandNotSupported,
            std::string("Unsupported command: ") + std::string(command)
        });
    }
}

std::vector<std::string> MyPlugin::supported_commands() const
{
    return {"hello", "status"};
}

bool MyPlugin::supports_command(std::string_view command) const
{
    auto commands = supported_commands();
    return std::find(commands.begin(), commands.end(), command) != commands.end();
}

qtplugin::PluginCapabilities MyPlugin::capabilities() const
{
    qtplugin::PluginCapabilities caps;
    caps.supports_configuration = true;
    caps.supports_hot_reload = true;
    caps.supports_state_persistence = false;
    caps.thread_safe = true;
    caps.requires_ui = false;
    caps.requires_network = false;
    return caps;
}

#include "my_plugin.moc"
```

### Step 3: Metadata File

Create `metadata.json`:

```json
{
  "id": "com.example.myplugin",
  "name": "My Example Plugin",
  "version": "1.0.0",
  "description": "An example plugin demonstrating the QtPlugin system",
  "author": "Your Name",
  "license": "MIT",
  "dependencies": [],
  "tags": ["example", "demo"],
  "capabilities": {
    "supports_configuration": true,
    "supports_hot_reload": true,
    "supports_state_persistence": false,
    "thread_safe": true,
    "requires_ui": false,
    "requires_network": false
  },
  "commands": [
    {
      "name": "hello",
      "description": "Returns a greeting message",
      "parameters": {
        "name": {
          "type": "string",
          "description": "Name to greet",
          "default": "World"
        }
      }
    },
    {
      "name": "status",
      "description": "Returns plugin status information",
      "parameters": {}
    }
  ]
}
```

### Step 4: CMake Configuration

Create `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyPlugin)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(QtPlugin REQUIRED COMPONENTS Core)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Create plugin library
add_library(my_plugin SHARED
    my_plugin.cpp
    my_plugin.hpp
)

# Link libraries
target_link_libraries(my_plugin
    Qt6::Core
    QtPlugin::Core
)

# Set plugin properties
set_target_properties(my_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    SUFFIX ".qtplugin"
)

# Copy metadata file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json"
    "${CMAKE_BINARY_DIR}/plugins/metadata.json"
    COPYONLY
)

# Install plugin
install(TARGETS my_plugin
    LIBRARY DESTINATION plugins
    RUNTIME DESTINATION plugins
)

install(FILES metadata.json
    DESTINATION plugins
)
```

## Advanced Topics

### Plugin Dependencies

To create a plugin that depends on other plugins:

```json
{
  "dependencies": [
    {
      "id": "com.example.base_plugin",
      "version": ">=1.0.0",
      "required": true
    },
    {
      "id": "com.example.optional_plugin",
      "version": "^2.0.0",
      "required": false
    }
  ]
}
```

### Inter-Plugin Communication

Use the message bus for communication:

```cpp
// In your plugin
void MyPlugin::sendMessage()
{
    auto* bus = qtplugin::MessageBus::instance();
    QJsonObject data;
    data["message"] = "Hello from MyPlugin";
    bus->publish("myplugin.events", data);
}

void MyPlugin::subscribeToMessages()
{
    auto* bus = qtplugin::MessageBus::instance();
    bus->subscribe("other_plugin.events", this, SLOT(onMessageReceived(QJsonObject)));
}

void MyPlugin::onMessageReceived(const QJsonObject& data)
{
    qDebug() << "Received message:" << data;
}
```

### Configuration Management

Handle complex configuration:

```cpp
qtplugin::expected<void, qtplugin::PluginError> MyPlugin::configure(const QJsonObject& config)
{
    // Validate required fields
    if (!config.contains("server_url")) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::ConfigurationError,
            "Missing required field: server_url"
        });
    }

    // Validate types
    if (!config["server_url"].isString()) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::ConfigurationError,
            "server_url must be a string"
        });
    }

    // Apply configuration
    m_serverUrl = config["server_url"].toString();
    m_timeout = config.value("timeout").toInt(5000);
    m_retries = config.value("retries").toInt(3);

    m_configuration = config;
    return {};
}
```

### Error Handling Best Practices

Always use the expected<T,E> pattern:

```cpp
qtplugin::expected<QJsonObject, qtplugin::PluginError> MyPlugin::execute_command(
    std::string_view command,
    const QJsonObject& params)
{
    try {
        if (command == "risky_operation") {
            // Perform operation that might fail
            auto result = performRiskyOperation(params);
            if (!result.isValid()) {
                return qtplugin::make_unexpected(qtplugin::PluginError{
                    qtplugin::PluginErrorCode::ExecutionFailed,
                    "Risky operation failed: " + result.errorString().toStdString()
                });
            }

            return result.toJsonObject();
        }

        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::CommandNotSupported,
            std::string("Unknown command: ") + std::string(command)
        });
    }
    catch (const std::exception& e) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::ExecutionFailed,
            std::string("Exception in command execution: ") + e.what()
        });
    }
}
```

## Testing Your Plugin

### Unit Testing

Create tests for your plugin:

```cpp
#include <QtTest/QtTest>
#include "my_plugin.hpp"

class TestMyPlugin : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testInitialization();
    void testCommands();
    void testConfiguration();
    void cleanupTestCase();

private:
    std::unique_ptr<MyPlugin> m_plugin;
};

void TestMyPlugin::initTestCase()
{
    m_plugin = std::make_unique<MyPlugin>();
}

void TestMyPlugin::testInitialization()
{
    auto result = m_plugin->initialize();
    QVERIFY(result.has_value());
    QVERIFY(m_plugin->is_initialized());
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Running);
}

void TestMyPlugin::testCommands()
{
    QJsonObject params;
    params["name"] = "Test";

    auto result = m_plugin->execute_command("hello", params);
    QVERIFY(result.has_value());
    QVERIFY(result.value().contains("message"));
    QCOMPARE(result.value()["message"].toString(), "Hello, Test!");
}

QTEST_MAIN(TestMyPlugin)
#include "test_my_plugin.moc"
```

## Deployment

### Building for Distribution

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target my_plugin
```

### Installation

```bash
cmake --install . --prefix /usr/local
```

The plugin will be installed to `/usr/local/plugins/` and can be loaded by applications using the QtPlugin system.

## Best Practices

1. **Always implement proper error handling** using expected<T,E>
2. **Validate all inputs** in configure() and execute_command()
3. **Use RAII** for resource management
4. **Make plugins thread-safe** when possible
5. **Provide comprehensive metadata** for better discoverability
6. **Test thoroughly** with unit tests and integration tests
7. **Document your plugin's API** clearly
8. **Handle shutdown gracefully** to prevent resource leaks
