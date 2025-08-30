# Getting Started with QtForge - Complete Tutorial

!!! info "Tutorial Information"
    **Difficulty**: Beginner  
    **Duration**: 45-60 minutes  
    **Prerequisites**: Basic C++ knowledge, Qt6 installed  
    **QtForge Version**: v3.0+

## Overview

This comprehensive tutorial will guide you through creating your first QtForge plugin system from scratch. You'll learn how to set up the environment, create plugins, manage them, and build a simple application that uses the plugin system.

### What You'll Build

By the end of this tutorial, you'll have:

- [x] A working QtForge development environment
- [x] A simple calculator plugin
- [x] A plugin manager application
- [x] Understanding of plugin lifecycle management
- [x] Knowledge of inter-plugin communication

### Learning Objectives

- [ ] Set up QtForge development environment
- [ ] Create and build your first plugin
- [ ] Implement plugin loading and management
- [ ] Handle plugin communication and data exchange
- [ ] Debug and troubleshoot common issues

## Step 1: Environment Setup

### 1.1 Install Prerequisites

First, ensure you have the required tools installed:

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake qt6-base-dev qt6-tools-dev

# macOS (using Homebrew)
brew install cmake qt6

# Windows (using vcpkg)
vcpkg install qt6-base qt6-tools
```

### 1.2 Clone and Build QtForge

```bash
# Clone the repository
git clone https://github.com/QtForge/QtForge.git
cd QtForge

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DQTFORGE_BUILD_EXAMPLES=ON

# Build QtForge
cmake --build . --parallel

# Install (optional)
sudo cmake --install .
```

### 1.3 Verify Installation

Create a simple test to verify QtForge is working:

**test_qtforge.cpp**
```cpp
#include <qtplugin/core/plugin_manager.hpp>
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // Test QtForge initialization
    auto manager = qtplugin::PluginManager::create();
    if (manager) {
        qDebug() << "QtForge initialized successfully!";
        qDebug() << "Available plugin directories:" << manager->get_plugin_directories();
        return 0;
    } else {
        qDebug() << "Failed to initialize QtForge";
        return 1;
    }
}
```

**CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.16)
project(QtForgeTest)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(QtForge REQUIRED)

add_executable(test_qtforge test_qtforge.cpp)
target_link_libraries(test_qtforge Qt6::Core QtForge::Core)
```

Build and run:
```bash
mkdir test_project && cd test_project
# Copy the files above
cmake . && make
./test_qtforge
```

## Step 2: Create Your First Plugin

### 2.1 Plugin Project Structure

Create a new directory for your calculator plugin:

```
calculator_plugin/
├── CMakeLists.txt
├── calculator_plugin.hpp
├── calculator_plugin.cpp
├── calculator_plugin.json
└── README.md
```

### 2.2 Plugin Header File

**calculator_plugin.hpp**
```cpp
#pragma once

#include <qtplugin/core/plugin_interface.hpp>
#include <QObject>

class CalculatorPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qtforge.tutorial.CalculatorPlugin" FILE "calculator_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit CalculatorPlugin(QObject* parent = nullptr);
    ~CalculatorPlugin() override = default;

    // IPlugin interface implementation
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, 
        const QJsonObject& params = {}) override;
    
    std::vector<std::string> available_commands() const override;
    qtplugin::PluginMetadata metadata() const override;
    qtplugin::PluginState state() const override;

private:
    qtplugin::PluginState m_state{qtplugin::PluginState::Unloaded};
    
    // Calculator operations
    double add(double a, double b) const { return a + b; }
    double subtract(double a, double b) const { return a - b; }
    double multiply(double a, double b) const { return a * b; }
    double divide(double a, double b) const;
    
    // Helper methods
    bool validate_numbers(const QJsonObject& params, double& a, double& b) const;
};
```

### 2.3 Plugin Implementation

**calculator_plugin.cpp**
```cpp
#include "calculator_plugin.hpp"
#include <QDebug>
#include <cmath>

CalculatorPlugin::CalculatorPlugin(QObject* parent)
    : QObject(parent), m_state(qtplugin::PluginState::Unloaded) {
}

qtplugin::expected<void, qtplugin::PluginError> CalculatorPlugin::initialize() {
    qDebug() << "CalculatorPlugin: Initializing...";
    
    // Perform any initialization logic here
    m_state = qtplugin::PluginState::Running;
    
    qDebug() << "CalculatorPlugin: Successfully initialized";
    return qtplugin::make_success();
}

void CalculatorPlugin::shutdown() noexcept {
    qDebug() << "CalculatorPlugin: Shutting down...";
    m_state = qtplugin::PluginState::Unloaded;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError> CalculatorPlugin::execute_command(
    std::string_view command, 
    const QJsonObject& params) {
    
    if (m_state != qtplugin::PluginState::Running) {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::InvalidState, 
            "Plugin not initialized");
    }
    
    double a, b;
    if (!validate_numbers(params, a, b)) {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::InvalidParameter, 
            "Invalid or missing parameters 'a' and 'b'");
    }
    
    double result = 0.0;
    QString operation;
    
    if (command == "add") {
        result = add(a, b);
        operation = "addition";
    }
    else if (command == "subtract") {
        result = subtract(a, b);
        operation = "subtraction";
    }
    else if (command == "multiply") {
        result = multiply(a, b);
        operation = "multiplication";
    }
    else if (command == "divide") {
        if (std::abs(b) < 1e-10) {
            return qtplugin::make_error<QJsonObject>(
                qtplugin::PluginErrorCode::InvalidParameter, 
                "Division by zero");
        }
        result = divide(a, b);
        operation = "division";
    }
    else {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound, 
            QString("Unknown command: %1").arg(QString::fromStdString(std::string(command))));
    }
    
    return QJsonObject{
        {"result", result},
        {"operation", operation},
        {"operands", QJsonArray{a, b}}
    };
}

std::vector<std::string> CalculatorPlugin::available_commands() const {
    return {"add", "subtract", "multiply", "divide"};
}

qtplugin::PluginMetadata CalculatorPlugin::metadata() const {
    qtplugin::PluginMetadata meta;
    meta.id = "calculator_plugin";
    meta.name = "Calculator Plugin";
    meta.version = qtplugin::Version(1, 0, 0);
    meta.description = "A simple calculator plugin for basic arithmetic operations";
    meta.author = "QtForge Tutorial";
    meta.license = "MIT";
    meta.website = "https://github.com/QtForge/QtForge";
    return meta;
}

qtplugin::PluginState CalculatorPlugin::state() const {
    return m_state;
}

double CalculatorPlugin::divide(double a, double b) const {
    return a / b;
}

bool CalculatorPlugin::validate_numbers(const QJsonObject& params, double& a, double& b) const {
    if (!params.contains("a") || !params.contains("b")) {
        return false;
    }
    
    QJsonValue a_val = params["a"];
    QJsonValue b_val = params["b"];
    
    if (!a_val.isDouble() || !b_val.isDouble()) {
        return false;
    }
    
    a = a_val.toDouble();
    b = b_val.toDouble();
    return true;
}
```

### 2.4 Plugin Metadata

**calculator_plugin.json**
```json
{
    "id": "calculator_plugin",
    "name": "Calculator Plugin",
    "version": "1.0.0",
    "description": "A simple calculator plugin for basic arithmetic operations",
    "author": "QtForge Tutorial",
    "license": "MIT",
    "website": "https://github.com/QtForge/QtForge",
    "interfaces": ["IPlugin"],
    "dependencies": [],
    "capabilities": ["arithmetic", "basic_math"],
    "commands": [
        {
            "name": "add",
            "description": "Add two numbers",
            "parameters": {
                "a": {"type": "number", "description": "First number"},
                "b": {"type": "number", "description": "Second number"}
            }
        },
        {
            "name": "subtract",
            "description": "Subtract two numbers",
            "parameters": {
                "a": {"type": "number", "description": "First number"},
                "b": {"type": "number", "description": "Second number"}
            }
        },
        {
            "name": "multiply",
            "description": "Multiply two numbers",
            "parameters": {
                "a": {"type": "number", "description": "First number"},
                "b": {"type": "number", "description": "Second number"}
            }
        },
        {
            "name": "divide",
            "description": "Divide two numbers",
            "parameters": {
                "a": {"type": "number", "description": "Dividend"},
                "b": {"type": "number", "description": "Divisor (cannot be zero)"}
            }
        }
    ]
}
```

### 2.5 Build Configuration

**CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.16)
project(CalculatorPlugin)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(QtForge REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the plugin library
add_library(calculator_plugin SHARED
    calculator_plugin.hpp
    calculator_plugin.cpp
)

# Link libraries
target_link_libraries(calculator_plugin
    Qt6::Core
    QtForge::Core
)

# Set plugin properties
set_target_properties(calculator_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# Copy metadata file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/calculator_plugin.json"
    "${CMAKE_BINARY_DIR}/plugins/calculator_plugin.json"
    COPYONLY
)
```

### 2.6 Build the Plugin

```bash
cd calculator_plugin
mkdir build && cd build
cmake ..
make

# Verify the plugin was built
ls -la plugins/
# Should show: calculator_plugin.so (or .dll on Windows) and calculator_plugin.json
```

## Step 3: Create a Plugin Manager Application

### 3.1 Application Structure

Create a simple console application to test your plugin:

```
calculator_app/
├── CMakeLists.txt
├── main.cpp
└── plugins/  (copy your built plugin here)
```

### 3.2 Main Application

**main.cpp**
```cpp
#include <qtplugin/core/plugin_manager.hpp>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <iostream>
#include <string>

class CalculatorApp {
private:
    std::shared_ptr<qtplugin::PluginManager> m_plugin_manager;
    QString m_calculator_plugin_id;

public:
    bool initialize() {
        // Create plugin manager
        m_plugin_manager = qtplugin::PluginManager::create();
        if (!m_plugin_manager) {
            qCritical() << "Failed to create plugin manager";
            return false;
        }

        // Set plugin directory
        m_plugin_manager->add_plugin_directory("./plugins");

        // Load calculator plugin
        auto load_result = m_plugin_manager->load_plugin("./plugins/calculator_plugin.so");
        if (!load_result) {
            qCritical() << "Failed to load calculator plugin:" << load_result.error().message();
            return false;
        }

        m_calculator_plugin_id = load_result.value();
        qDebug() << "Successfully loaded calculator plugin with ID:" << m_calculator_plugin_id;

        return true;
    }

    void run() {
        std::cout << "=== QtForge Calculator Tutorial ===" << std::endl;
        std::cout << "Available commands: add, subtract, multiply, divide, quit" << std::endl;
        std::cout << "Usage: <command> <number1> <number2>" << std::endl;
        std::cout << "Example: add 5.5 3.2" << std::endl << std::endl;

        std::string line;
        while (std::getline(std::cin, line)) {
            if (line == "quit" || line == "exit") {
                break;
            }

            if (line.empty()) {
                continue;
            }

            processCommand(QString::fromStdString(line));
        }

        std::cout << "Goodbye!" << std::endl;
    }

private:
    void processCommand(const QString& input) {
        QStringList parts = input.split(' ', Qt::SkipEmptyParts);

        if (parts.size() != 3) {
            std::cout << "Error: Please provide command and two numbers" << std::endl;
            return;
        }

        QString command = parts[0];
        bool ok1, ok2;
        double a = parts[1].toDouble(&ok1);
        double b = parts[2].toDouble(&ok2);

        if (!ok1 || !ok2) {
            std::cout << "Error: Invalid numbers provided" << std::endl;
            return;
        }

        // Get plugin and execute command
        auto plugin = m_plugin_manager->get_plugin(m_calculator_plugin_id);
        if (!plugin) {
            std::cout << "Error: Calculator plugin not available" << std::endl;
            return;
        }

        QJsonObject params{
            {"a", a},
            {"b", b}
        };

        auto result = plugin->execute_command(command.toStdString(), params);
        if (result) {
            QJsonObject response = result.value();
            double calc_result = response["result"].toDouble();
            QString operation = response["operation"].toString();

            std::cout << "Result: " << a << " " << command.toStdString() << " " << b
                      << " = " << calc_result << " (" << operation.toStdString() << ")" << std::endl;
        } else {
            std::cout << "Error: " << result.error().message().toStdString() << std::endl;
        }
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    CalculatorApp calculator_app;

    if (!calculator_app.initialize()) {
        return 1;
    }

    calculator_app.run();
    return 0;
}
```

### 3.3 Application Build Configuration

**CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.16)
project(CalculatorApp)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(QtForge REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the application
add_executable(calculator_app main.cpp)

# Link libraries
target_link_libraries(calculator_app
    Qt6::Core
    QtForge::Core
)
```

### 3.4 Build and Test the Application

```bash
cd calculator_app
mkdir build && cd build

# Copy the plugin to the plugins directory
mkdir -p plugins
cp ../../calculator_plugin/build/plugins/* plugins/

# Build the application
cmake ..
make

# Run the application
./calculator_app
```

### 3.5 Test Your Plugin

Try these commands in your application:

```
add 10 5
subtract 10 5
multiply 10 5
divide 10 5
divide 10 0  # Should show error
invalid_command 1 2  # Should show error
quit
```

## Step 4: Advanced Features

### 4.1 Plugin Discovery

Enhance your application to discover plugins automatically:

```cpp
void discoverPlugins() {
    auto discovered = m_plugin_manager->discover_plugins("./plugins");

    std::cout << "Discovered " << discovered.size() << " plugins:" << std::endl;
    for (const QString& plugin_path : discovered) {
        std::cout << "  - " << plugin_path.toStdString() << std::endl;
    }
}
```

### 4.2 Plugin Information Display

Add a command to show plugin information:

```cpp
void showPluginInfo() {
    auto plugin = m_plugin_manager->get_plugin(m_calculator_plugin_id);
    if (plugin) {
        auto metadata = plugin->metadata();
        std::cout << "=== Plugin Information ===" << std::endl;
        std::cout << "ID: " << metadata.id.toStdString() << std::endl;
        std::cout << "Name: " << metadata.name.toStdString() << std::endl;
        std::cout << "Version: " << metadata.version.toString().toStdString() << std::endl;
        std::cout << "Author: " << metadata.author.toStdString() << std::endl;
        std::cout << "Description: " << metadata.description.toStdString() << std::endl;

        auto commands = plugin->available_commands();
        std::cout << "Available commands: ";
        for (size_t i = 0; i < commands.size(); ++i) {
            std::cout << commands[i];
            if (i < commands.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
}
```

### 4.3 Error Handling and Logging

Add comprehensive error handling:

```cpp
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(calculatorApp, "calculator.app")

// In your error handling:
qCWarning(calculatorApp) << "Plugin operation failed:" << result.error().message();
```

## Step 5: Testing and Debugging

### 5.1 Unit Testing Your Plugin

Create a simple test for your plugin:

**test_calculator_plugin.cpp**
```cpp
#include <QtTest/QtTest>
#include "../calculator_plugin.hpp"

class TestCalculatorPlugin : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        m_plugin = std::make_unique<CalculatorPlugin>();
        auto init_result = m_plugin->initialize();
        QVERIFY(init_result.has_value());
    }

    void testAddition() {
        QJsonObject params{{"a", 5.0}, {"b", 3.0}};
        auto result = m_plugin->execute_command("add", params);

        QVERIFY(result.has_value());
        QCOMPARE(result.value()["result"].toDouble(), 8.0);
    }

    void testDivisionByZero() {
        QJsonObject params{{"a", 5.0}, {"b", 0.0}};
        auto result = m_plugin->execute_command("divide", params);

        QVERIFY(!result.has_value());
        QVERIFY(result.error().code() == qtplugin::PluginErrorCode::InvalidParameter);
    }

    void testInvalidCommand() {
        QJsonObject params{{"a", 5.0}, {"b", 3.0}};
        auto result = m_plugin->execute_command("invalid", params);

        QVERIFY(!result.has_value());
        QVERIFY(result.error().code() == qtplugin::PluginErrorCode::CommandNotFound);
    }

private:
    std::unique_ptr<CalculatorPlugin> m_plugin;
};

QTEST_MAIN(TestCalculatorPlugin)
#include "test_calculator_plugin.moc"
```

### 5.2 Debugging Tips

1. **Enable Debug Output**: Set `QT_LOGGING_RULES="*.debug=true"`
2. **Check Plugin Loading**: Verify plugin files exist and have correct permissions
3. **Validate JSON**: Ensure plugin metadata JSON is valid
4. **Memory Issues**: Use tools like Valgrind or AddressSanitizer

## Troubleshooting

### Common Issues

#### Plugin Not Loading

**Problem**: Plugin fails to load with "file not found" error

**Solutions**:
1. Check file path and permissions
2. Verify all dependencies are available
3. Check Qt plugin path environment variables

#### Symbol Not Found Errors

**Problem**: Plugin loads but symbols are missing

**Solutions**:
1. Ensure proper linking with QtForge libraries
2. Check that all required Qt modules are linked
3. Verify plugin interface implementation

#### Runtime Crashes

**Problem**: Application crashes when using plugin

**Solutions**:
1. Check for null pointer dereferences
2. Validate input parameters
3. Ensure proper plugin lifecycle management

## Next Steps

Congratulations! You've successfully created your first QtForge plugin system. Here are some ideas for further exploration:

- [ ] Create additional plugins (string manipulation, file operations)
- [ ] Implement plugin-to-plugin communication
- [ ] Add configuration management
- [ ] Explore advanced features like transactions and orchestration
- [ ] Build a GUI application using your plugins

### Recommended Reading

- [Advanced Plugin Development Guide](../user-guide/advanced-plugin-development.md)
- [Plugin Architecture Best Practices](../user-guide/plugin-architecture.md)
- [QtForge API Reference](../api/overview.md)

---

*Tutorial completed! You now have a solid foundation for building plugin-based applications with QtForge.*
