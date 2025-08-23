# QtPlugin Examples

This directory contains comprehensive example plugins and applications demonstrating the QtPlugin library's capabilities.

## 🚀 Examples Overview

### 📦 Basic Plugin (`basic_plugin/`)

A foundational example demonstrating core plugin interface implementation.

**Features:**

- ✅ Complete plugin lifecycle (initialize, shutdown)
- ✅ Command execution with parameter handling
- ✅ JSON-based configuration management
- ✅ Comprehensive metadata definition
- ✅ Error handling with expected<T,E> pattern
- ✅ Thread-safe operations

**Commands Supported:**

- `hello` - Greeting with customizable name parameter
- `status` - Plugin status and health information
- `echo` - Echo input parameters for testing
- `config` - Display current configuration

### 🔧 Service Plugin (`service_plugin/`)

Advanced service-oriented plugin with background processing capabilities.

**Features:**

- ✅ Service registration and discovery
- ✅ Background task execution with threading
- ✅ Inter-plugin communication via message bus
- ✅ Resource management and monitoring
- ✅ Hot-reload support with state preservation
- ✅ Performance metrics and monitoring

### 📊 Data Processor Plugin (`data_processor/`)

High-performance data processing with pipeline integration.

**Features:**

- ✅ Stream-based data transformation
- ✅ Pipeline processing with filters
- ✅ Batch and real-time processing modes
- ✅ Memory-efficient large data handling
- ✅ Custom data format support
- ✅ Performance optimization techniques

## 🛠️ Building Examples

### Prerequisites

- QtPlugin library v3.0.0+ installed
- Qt6 with Core, Network, Widgets, Test modules
- CMake 3.21 or later
- C++20 compatible compiler

### Build All Examples

```bash
# Clone and setup
git clone <repository-url>
cd qt-plugin-sample/lib/examples

# Configure and build
mkdir build && cd build
cmake .. -DQTPLUGIN_BUILD_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# Run tests
ctest --output-on-failure
```

### Build Individual Example

```bash
cd basic_plugin
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Test the plugin
./test_basic_plugin
```

## 🚀 Running Examples

### Basic Plugin Test

```bash
cd build/basic_plugin
./basic_plugin_test

# Expected output:
# QtPlugin library initialized, version 3.0.0
# Plugin loaded successfully: com.example.basic_plugin
# Command 'hello' executed: {"message": "Hello, World!"}
# Plugin shutdown completed
```

### Interactive Plugin Manager Demo

```bash
cd build
./plugin_manager_demo --plugin-dir examples/plugins --interactive

# Available commands:
# load <plugin_path>     - Load a plugin
# unload <plugin_id>     - Unload a plugin
# list                   - List loaded plugins
# execute <id> <cmd>     - Execute plugin command
# config <id> <json>     - Configure plugin
# quit                   - Exit application
```

## 📖 Learning Path

### 🎯 Beginner Level

1. **Basic Plugin** - Learn fundamental concepts and interface implementation
2. **Configuration** - Understand JSON-based configuration management
3. **Commands** - Implement command execution with parameters
4. **Error Handling** - Master expected<T,E> error handling pattern

### 🚀 Intermediate Level

5. **Service Plugin** - Explore background services and threading
6. **Communication** - Implement inter-plugin messaging
7. **Resource Management** - Handle resources and lifecycle properly
8. **Testing** - Write comprehensive unit and integration tests

### 🏆 Advanced Level

9. **Data Processor** - Optimize for high-performance data processing
10. **Network Plugin** - Implement network protocols and communication
11. **UI Integration** - Create rich user interfaces
12. **Custom Application** - Build complete plugin-based applications

## 🔧 Common Patterns

### Plugin Registration and Metadata

```cpp
// Plugin class declaration
class MyPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "metadata.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    // Implement required interface methods
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() override;
    qtplugin::PluginMetadata metadata() const override;
    // ... other methods
};
```

### Modern Error Handling

```cpp
// Command execution with error handling
auto result = plugin->execute_command("process_data", params);
if (result.has_value()) {
    // Success path
    auto output = result.value();
    qDebug() << "Processing completed:" << output;
} else {
    // Error path
    auto error = result.error();
    qWarning() << "Command failed:" << error.message.c_str()
               << "Code:" << static_cast<int>(error.code);
}

// Chaining operations with monadic interface
auto final_result = plugin->execute_command("step1", params)
    .and_then([&](const auto& result1) {
        return plugin->execute_command("step2", result1);
    })
    .and_then([&](const auto& result2) {
        return plugin->execute_command("step3", result2);
    });
```

### Configuration Management

```cpp
// Comprehensive configuration with validation
qtplugin::expected<void, qtplugin::PluginError> MyPlugin::configure(const QJsonObject& config) {
    // Validate required fields
    if (!config.contains("server_url")) {
        return qtplugin::make_unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::ConfigurationError,
            "Missing required field: server_url"
        });
    }
    
    // Type validation
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

### Inter-Plugin Communication

```cpp
// Publisher plugin
void MyPlugin::publishEvent(const QString& event, const QJsonObject& data) {
    auto* bus = qtplugin::MessageBus::instance();
    QJsonObject message;
    message["event"] = event;
    message["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    message["data"] = data;
    bus->publish("myplugin.events", message);
}

// Subscriber plugin
void MyPlugin::initialize() {
    auto* bus = qtplugin::MessageBus::instance();
    bus->subscribe("myplugin.events", this, SLOT(onEventReceived(QJsonObject)));
    bus->subscribe("system.shutdown", this, SLOT(onSystemShutdown(QJsonObject)));
}
```

## ✅ Best Practices

### 🏗️ Architecture

1. **Follow SOLID Principles** - Single responsibility, open/closed, etc.
2. **Use Dependency Injection** - Make components testable and flexible
3. **Implement Proper Interfaces** - Clear contracts between components
4. **Design for Concurrency** - Thread-safe operations from the start

### 🔧 Implementation

5. **Always Use expected<T,E>** - Modern error handling without exceptions
6. **Validate All Inputs** - Configuration, commands, and parameters
7. **Follow RAII** - Automatic resource management and cleanup
8. **Use Smart Pointers** - Automatic memory management

### 🧪 Testing

9. **Write Comprehensive Tests** - Unit, integration, and performance tests
10. **Mock Dependencies** - Isolate components for testing
11. **Test Error Paths** - Ensure proper error handling
12. **Performance Testing** - Validate performance requirements

### 📖 Documentation

13. **Document APIs Clearly** - Comprehensive API documentation
14. **Provide Usage Examples** - Real-world usage scenarios
15. **Version Compatibility** - Clear version requirements and compatibility
16. **Migration Guides** - Help users upgrade between versions

## 🔧 Troubleshooting

### Common Issues and Solutions

#### 🚫 Plugin Loading Failures

**Symptoms:**

- Plugin fails to load
- "Invalid plugin file format" errors
- Missing symbol errors

**Solutions:**

- Ensure plugin file has correct permissions
- Validate JSON metadata syntax with `jq`
- Verify all required Qt modules are available
- Check plugin interface implementation completeness

#### ⚙️ Configuration Errors

**Symptoms:**

- Configuration validation failures
- Type mismatch errors
- Missing required fields

**Solutions:**

- Validate JSON configuration format and syntax
- Check required vs optional fields in metadata
- Verify data types match plugin expectations
- Use configuration validation schemas

#### 🏃 Runtime Errors

**Symptoms:**

- Plugin crashes during execution
- Memory access violations
- Thread safety issues

**Solutions:**

- Verify plugin initialization completed successfully
- Check resource availability and limits
- Monitor memory usage for leaks
- Ensure thread safety in concurrent operations

### Debug Configuration

```cpp
// Enable debug logging in main application
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Enable QtPlugin debug logging
    QLoggingCategory::setFilterRules("qtplugin.*=true");
    
    // Set debug environment
    qputenv("QTPLUGIN_DEBUG", "1");
    qputenv("QTPLUGIN_LOG_LEVEL", "debug");
    
    // Your application code
    return app.exec();
}
```

## 🤝 Contributing

### Adding New Examples

1. **Create Directory Structure**

   ```bash
   mkdir examples/my_example
   cd examples/my_example
   ```

2. **Implement Plugin**
   - Follow existing patterns and conventions
   - Include comprehensive error handling
   - Add thorough documentation
   - Implement unit tests

3. **Add Build Configuration**

   ```cmake
   # CMakeLists.txt
   cmake_minimum_required(VERSION 3.21)
   project(MyExample)
   
   find_package(QtPlugin REQUIRED COMPONENTS Core)
   # ... rest of configuration
   ```

4. **Update Documentation**
   - Add example to this README
   - Include usage instructions
   - Document any special requirements

5. **Submit Pull Request**
   - Ensure all tests pass
   - Include comprehensive commit message

## 📄 License

All examples are provided under the same MIT license as the QtPlugin library.
