# Examples

This section provides comprehensive examples demonstrating QtPlugin's capabilities. Each example includes complete source code, detailed explanations, and practical use cases.

## 📚 Example Categories

### 🎯 Getting Started Examples

Perfect for beginners learning QtPlugin fundamentals:

<div class="grid cards" markdown>

-   :material-hand-wave: **[Basic Plugin](basic-plugin.md)**

    ---

    Simple "Hello World" plugin demonstrating core concepts

    **Features:**
    - Plugin interface implementation
    - Command execution
    - Configuration management
    - Lifecycle handling

    **Difficulty:** Beginner

-   :material-cog: **[Service Plugin](service-plugin.md)**

    ---

    Background service with timer-based processing

    **Features:**
    - Background processing
    - Timer management
    - State persistence
    - Resource monitoring

    **Difficulty:** Intermediate

</div>

### 🌐 Advanced Examples

For developers building production applications:

<div class="grid cards" markdown>

-   :material-network: **[Network Plugin](network-plugin.md)**

    ---

    HTTP client/server plugin with REST API

    **Features:**
    - HTTP requests/responses
    - REST API integration
    - Async networking
    - Error handling

    **Difficulty:** Advanced

-   :material-monitor: **[UI Plugin](ui-plugin.md)**

    ---

    Plugin with custom user interface

    **Features:**
    - Qt Widgets integration
    - Custom dialogs
    - Event handling
    - Theme support

    **Difficulty:** Advanced

</div>

### 🏗️ Architecture Examples

Demonstrating advanced architectural patterns:

<div class="grid cards" markdown>

-   :material-puzzle-plus: **[Plugin Composition](advanced.md#plugin-composition)**

    ---

    Multiple plugins working together

    **Features:**
    - Inter-plugin communication
    - Dependency management
    - Service registration
    - Event coordination

-   :material-message-processing: **[Message Bus Usage](advanced.md#message-bus)**

    ---

    Advanced messaging patterns

    **Features:**
    - Publish-subscribe
    - Request-response
    - Event broadcasting
    - Type-safe messaging

</div>

## 🚀 Quick Start

### Running Examples

All examples are included in the QtPlugin repository:

```bash
# Clone repository
git clone https://github.com/QtForge/QtPlugin.git
cd QtPlugin

# Build with examples
mkdir build && cd build
cmake .. -DQTPLUGIN_BUILD_EXAMPLES=ON
cmake --build .

# Run examples
cd examples
./basic_plugin_demo
./service_plugin_demo
./network_plugin_demo
```

### Example Structure

Each example follows this structure:

```
example_name/
├── CMakeLists.txt          # Build configuration
├── README.md               # Example-specific documentation
├── plugin/                 # Plugin source code
│   ├── plugin.hpp
│   ├── plugin.cpp
│   └── metadata.json
├── app/                    # Test application
│   └── main.cpp
└── tests/                  # Unit tests
    └── test_plugin.cpp
```

## 📖 Learning Path

### 🎯 Beginner Path

1. **[Basic Plugin](basic-plugin.md)** - Start here to understand fundamentals
2. **[Service Plugin](service-plugin.md)** - Learn background processing
3. **[Configuration Examples](basic-plugin.md#configuration)** - Master plugin configuration

### 🚀 Intermediate Path

4. **[Network Plugin](network-plugin.md)** - Add network capabilities
5. **[UI Plugin](ui-plugin.md)** - Create user interfaces
6. **[Message Bus Examples](advanced.md#message-bus)** - Inter-plugin communication

### 🏆 Advanced Path

7. **[Plugin Composition](advanced.md#plugin-composition)** - Complex architectures
8. **[Security Examples](advanced.md#security)** - Plugin validation and trust
9. **[Performance Optimization](advanced.md#performance)** - High-performance plugins

## 🔧 Development Tools

### CMake Helpers

QtPlugin provides CMake functions to simplify plugin development:

```cmake
# Create a plugin
qtplugin_add_plugin(my_plugin
    TYPE service
    SOURCES src/plugin.cpp
    HEADERS include/plugin.hpp
    METADATA metadata.json
    DEPENDENCIES Qt6::Network
)

# Create plugin test
qtplugin_add_plugin_test(my_plugin_test
    PLUGIN my_plugin
    SOURCES tests/test_plugin.cpp
)

# Find plugins in directory
qtplugin_find_plugins(PLUGIN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/plugins")
```

### Testing Framework

Use QtPlugin's testing framework for plugin tests:

```cpp
#include <qtplugin/testing/plugin_test_framework.hpp>

class PluginTest : public QObject {
    Q_OBJECT

private slots:
    void testPluginLoading();
    void testPluginCommands();
};

void PluginTest::testPluginLoading() {
    qtplugin::testing::PluginTestFramework framework;
    
    auto result = framework.load_test_plugin("my_plugin.so");
    QVERIFY(result.has_value());
    
    auto plugin = framework.get_plugin(result.value());
    QVERIFY(plugin != nullptr);
    QVERIFY(plugin->is_initialized());
}
```

### Debug Tools

Enable debugging for plugin development:

```cpp
// Enable debug logging
qtplugin::set_log_level(qtplugin::LogLevel::Debug);
qtplugin::enable_component_logging(true);

// Enable plugin profiling
manager->enable_profiling(true);

// Get performance metrics
auto metrics = manager->performance_metrics();
qDebug() << "Plugin loading time:" << metrics.average_load_time;
```

## 📊 Example Comparison

| Example | Complexity | Features | Use Cases |
|---------|------------|----------|-----------|
| **Basic Plugin** | ⭐ | Commands, Config | Learning, Simple tools |
| **Service Plugin** | ⭐⭐ | Background tasks | Monitoring, Processing |
| **Network Plugin** | ⭐⭐⭐ | HTTP, REST API | Web services, APIs |
| **UI Plugin** | ⭐⭐⭐ | Widgets, Dialogs | Desktop apps, Tools |
| **Advanced** | ⭐⭐⭐⭐ | Multi-plugin | Enterprise apps |

## 🎯 Use Case Examples

### Desktop Applications

- **Text Editor Plugins**: Syntax highlighting, code completion
- **Media Player Plugins**: Codecs, visualizations, effects
- **IDE Extensions**: Language support, debugging tools

### Server Applications

- **Web Server Modules**: Authentication, logging, caching
- **Database Plugins**: Drivers, connection pooling, migrations
- **Monitoring Plugins**: Metrics collection, alerting

### Embedded Systems

- **Device Drivers**: Hardware abstraction, communication
- **Protocol Handlers**: Network protocols, data formats
- **Control Systems**: Sensors, actuators, automation

## 🔍 Code Patterns

### Common Plugin Patterns

1. **Command Pattern**:
   ```cpp
   auto result = plugin->execute_command("process_data", params);
   ```

2. **Observer Pattern**:
   ```cpp
   bus.subscribe<DataUpdateEvent>([](const auto& event) {
       // Handle data update
   });
   ```

3. **Factory Pattern**:
   ```cpp
   auto processor = plugin->create_processor(ProcessorType::Advanced);
   ```

4. **Strategy Pattern**:
   ```cpp
   plugin->set_algorithm(std::make_unique<FastAlgorithm>());
   ```

### Error Handling Patterns

```cpp
// Check and handle errors
auto result = plugin->execute_command("risky_operation");
if (!result) {
    switch (result.error().code) {
        case PluginErrorCode::NotInitialized:
            // Handle not initialized
            break;
        case PluginErrorCode::InvalidParameters:
            // Handle invalid parameters
            break;
        default:
            // Handle other errors
            break;
    }
}
```

## 🧪 Testing Examples

### Unit Testing

```cpp
void TestMyPlugin::testCommandExecution() {
    // Setup
    MyPlugin plugin;
    QVERIFY(plugin.initialize().has_value());
    
    // Test
    QJsonObject params{{"input", "test_data"}};
    auto result = plugin.execute_command("process", params);
    
    // Verify
    QVERIFY(result.has_value());
    QCOMPARE(result.value()["status"].toString(), "success");
}
```

### Integration Testing

```cpp
void TestPluginIntegration::testPluginCommunication() {
    // Load multiple plugins
    auto manager = qtplugin::PluginManager::create();
    auto producer_id = manager->load_plugin("producer_plugin.so");
    auto consumer_id = manager->load_plugin("consumer_plugin.so");
    
    // Test communication
    auto& bus = manager->message_bus();
    bool message_received = false;
    
    bus.subscribe<TestMessage>([&](const TestMessage& msg) {
        message_received = true;
    });
    
    bus.publish(TestMessage{"test"});
    
    // Wait and verify
    QTest::qWait(100);
    QVERIFY(message_received);
}
```

## 📚 Additional Resources

### Documentation

- **[API Reference](../api/index.md)**: Complete API documentation
- **[Developer Guide](../developer-guide/plugin-development.md)**: Advanced development
- **[Best Practices](../developer-guide/best-practices.md)**: Production guidelines

### Community Examples

- **[GitHub Repository](https://github.com/QtForge/QtPlugin/tree/main/examples)**: Official examples
- **[Community Plugins](https://github.com/topics/qtplugin)**: Community contributions
- **[Plugin Registry](https://qtplugin.org/registry)**: Discover existing plugins

### Getting Help

- **[FAQ](../appendix/faq.md)**: Common questions and answers
- **[Troubleshooting](../user-guide/troubleshooting.md)**: Problem solving guide
- **[GitHub Discussions](https://github.com/QtForge/QtPlugin/discussions)**: Community support

Ready to start? Begin with the [Basic Plugin](basic-plugin.md) example!
