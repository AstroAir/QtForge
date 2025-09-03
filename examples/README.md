# QtForge Examples

**Comprehensive examples demonstrating QtForge plugin system capabilities with clear learning progression.**

## 🎯 Quick Start Guide

**New to QtForge?** Start here:

1. **[Hello World](01-fundamentals/hello-world/)** - Your first plugin (5 minutes)
2. **[Basic Plugin](01-fundamentals/basic-plugin/)** - Core concepts (15 minutes)
3. **[Message Bus](02-communication/message-bus/)** - Inter-plugin communication (20 minutes)

**Building Applications?** Jump to:

- **[Full Application](06-comprehensive/full-application/)** - Complete feature demonstration
- **[Background Tasks](03-services/background-tasks/)** - Service-oriented architecture

## 📚 Learning Path

### 🌱 01-fundamentals/ - Essential Concepts

Start your QtForge journey with these foundational examples.

### 🐍 python/ - Python Binding Examples

**Comprehensive Python API demonstrations** - Learn QtForge through Python

#### Legacy Examples
- ✅ **[01_basic_plugin_management.py](python/01_basic_plugin_management.py)** - Plugin managers, loading, lifecycle
- ✅ **[02_communication_and_messaging.py](python/02_communication_and_messaging.py)** - Message buses, pub/sub, services
- ✅ **[03_security_and_validation.py](python/03_security_and_validation.py)** - Security systems, permissions, validation
- ✅ **[04_orchestration_and_workflows.py](python/04_orchestration_and_workflows.py)** - Workflows, steps, execution modes
- ✅ **[README.md](python/README.md)** - Detailed Python binding documentation

#### New Comprehensive Examples
- 🆕 **[comprehensive/01_basic_plugin_management.py](python/comprehensive/01_basic_plugin_management.py)** - Complete plugin management walkthrough
  - PluginManager, PluginRegistry, PluginLoader creation and configuration
  - Plugin loading, unloading, and querying operations
  - Plugin states, capabilities, and priorities demonstration
  - Dependency management and lifecycle configuration
  - Comprehensive error handling and resource cleanup
- 🆕 **[comprehensive/02_communication_system.py](python/comprehensive/02_communication_system.py)** - Full communication system demo
  - MessageBus creation and configuration
  - Message publishing and subscription patterns
  - Request/Response communication protocols
  - Service contracts and communication utilities
  - Threading safety and performance testing

### 🌙 lua/ - Lua Binding Examples

**Comprehensive Lua API demonstrations** - Learn QtForge through Lua

#### Legacy Examples
- ✅ **[01_basic_plugin_management.lua](lua/01_basic_plugin_management.lua)** - Plugin managers, loading, lifecycle
- ✅ **[02_communication_and_messaging.lua](lua/02_communication_and_messaging.lua)** - Message buses, pub/sub, services
- ✅ **[03_comprehensive_features.lua](lua/03_comprehensive_features.lua)** - Security, orchestration, monitoring, utilities
- ✅ **[README.md](lua/README.md)** - Detailed Lua binding documentation

#### New Comprehensive Examples
- 🆕 **[comprehensive/01_basic_plugin_management.lua](lua/comprehensive/01_basic_plugin_management.lua)** - Complete plugin management in Lua
  - Core plugin system operations with Lua-specific patterns
  - Plugin manager creation, configuration, and lifecycle management
  - Plugin loading, querying, and state management
  - Dependency resolution and error handling with pcall
  - Resource management and cleanup patterns
- 🆕 **[comprehensive/02_utilities_showcase.lua](lua/comprehensive/02_utilities_showcase.lua)** - Comprehensive utilities demo
  - JSON parsing, validation, and manipulation
  - String utilities (trim, case conversion, pattern matching)
  - File system operations (existence checks, path manipulation)
  - Logging functionality with different levels
  - Time and date utilities with formatting
  - Error handling utilities and edge case testing

#### [hello-world/](01-fundamentals/hello-world/)

**The simplest possible plugin** - Perfect for absolute beginners

- ✅ Minimal IPlugin implementation (~70 lines)
- ✅ Single `hello` command
- ✅ Basic lifecycle and error handling

#### [basic-plugin/](01-fundamentals/basic-plugin/)

**Core IPlugin interface** - Foundation for all plugin development

- ✅ Complete lifecycle management
- ✅ Configuration with validation
- ✅ Multiple commands (status, echo, config, timer)
- ✅ Thread-safe operations
- ✅ Background processing with QTimer

#### [configuration/](01-fundamentals/configuration/)

**Configuration management patterns** - Essential for production plugins

- ✅ JSON schema validation
- ✅ Live configuration updates
- ✅ Default and custom configurations
- ✅ Configuration persistence

### 🔄 02-communication/ - Inter-Plugin Communication

Master the art of plugin communication and coordination.

#### [message-bus/](02-communication/message-bus/)

**MessageBus communication patterns** - Core communication system

- ✅ Publishing and subscribing to messages
- ✅ Type-safe message handling
- ✅ Message priority and delivery modes
- ✅ Performance optimization

#### [request-response/](02-communication/request-response/)

**Synchronous communication** - Service-oriented patterns

- ✅ Request-response with timeouts
- ✅ Asynchronous request handling
- ✅ Error propagation and handling
- ✅ Request correlation and tracking

#### [event-driven/](02-communication/event-driven/)

**Event broadcasting and filtering** - Event-driven architecture

- ✅ Event publishing to multiple subscribers
- ✅ Message filtering and routing
- ✅ Event aggregation and batching
- ✅ Performance monitoring

### ⚙️ 03-services/ - Background Processing

Build robust service-oriented applications with QtForge.

#### [background-tasks/](03-services/background-tasks/)

**Threading and task management** - Service architecture patterns

- ✅ Worker threads and task queues
- ✅ Thread-safe operations
- ✅ Service registration and discovery
- ✅ Background processing patterns
- ✅ Resource management and monitoring

#### [service-discovery/](03-services/service-discovery/)

**Service registration and discovery** - Microservice patterns

- ✅ Automatic service registration
- ✅ Service metadata and endpoints
- ✅ Heartbeat and health monitoring
- ✅ Service lifecycle management

#### [workflow-orchestration/](03-services/workflow-orchestration/)

**Complex workflow management** - Enterprise integration

- ✅ Workflow step execution
- ✅ Plugin orchestration
- ✅ Transaction management
- ✅ Error handling and rollback

### 🔒 04-specialized/ - Domain-Specific Features

Advanced features for specialized use cases.

#### [security/](04-specialized/security/)

**Security validation and permissions** - Production security

- ✅ Plugin validation and verification
- ✅ Signature verification and trust chains
- ✅ Permission management and policies
- ✅ Security audit logging
- ✅ Real-time security monitoring

#### [monitoring/](04-specialized/monitoring/)

**Hot reload and metrics collection** - Production monitoring

- ✅ Hot reload with file system monitoring
- ✅ Performance metrics and analysis
- ✅ Resource usage tracking
- ✅ Alert system with configurable thresholds
- ✅ Real-time monitoring dashboards

#### [network/](04-specialized/network/)

**Network protocols and APIs** - Network programming

- ✅ HTTP client and server capabilities
- ✅ WebSocket communication
- ✅ REST API implementation
- ✅ SSL/TLS security
- ✅ Network diagnostics and monitoring

#### [ui-integration/](04-specialized/ui-integration/)

**Qt Widgets integration** - GUI development

- ✅ Qt Widgets and dialog management
- ✅ Theme support and customization
- ✅ UI component integration
- ✅ Event handling and user interaction

### 🔗 05-integration/ - Cross-Language & External

Integrate QtForge with external systems and languages.

#### [python-bindings/](05-integration/python-bindings/)

**Python integration patterns** - Cross-language development

- ✅ Python bindings and script execution
- ✅ Error handling across languages
- ✅ Performance optimization patterns
- ✅ Advanced usage patterns

#### [version-management/](05-integration/version-management/)

**Plugin versioning and compatibility** - Version control

- ✅ Version handling and comparison
- ✅ Compatibility checking
- ✅ Migration strategies
- ✅ Dependency management

#### [marketplace/](05-integration/marketplace/)

**Plugin discovery and installation** - Plugin ecosystem

- ✅ Plugin discovery and search
- ✅ Installation and updates
- ✅ Marketplace integration
- ✅ Plugin distribution

### 🚀 06-comprehensive/ - Complete Applications

Real-world applications demonstrating all features.

#### [full-application/](06-comprehensive/full-application/)

**Complete feature demonstration** - Production-ready example

- ✅ **ALL QtForge features** integrated
- ✅ Enterprise-grade implementation
- ✅ Comprehensive testing suite
- ✅ Performance benchmarks
- ✅ Cross-platform support

#### [performance-optimized/](06-comprehensive/performance-optimized/)

**High-performance patterns** - Performance engineering

- ✅ Performance optimization techniques
- ✅ Benchmarking and profiling
- ✅ Memory and CPU optimization
- ✅ Scalability patterns

## 🛠️ Building Examples

### Prerequisites

- **QtForge** library v3.0.0+ installed
- **Qt6** with Core, Network, Widgets, Test modules
- **CMake** 3.21 or later
- **C++20** compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)

### Build All Examples

```bash
# Navigate to examples directory
cd examples

# Configure and build
mkdir build && cd build
cmake .. -DQTFORGE_BUILD_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# Run tests
ctest --output-on-failure
```

### Build Individual Example

```bash
# Example: Build hello world plugin
cd 01-fundamentals/hello-world
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Test the plugin
./HelloWorldPluginTest
```

## 🚀 Running Examples

### Prerequisites

```bash
# Build QtForge with C++ examples
cmake -DQTFORGE_BUILD_EXAMPLES=ON ..
make -j$(nproc)

# For Python binding examples
cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON ..
make -j$(nproc)
pip install pytest  # Optional, for enhanced testing

# For Lua binding examples
cmake -DQTFORGE_BUILD_LUA_BINDINGS=ON ..
make -j$(nproc)

# For all examples (C++, Python, and Lua)
cmake -DQTFORGE_BUILD_EXAMPLES=ON -DQTFORGE_BUILD_PYTHON_BINDINGS=ON -DQTFORGE_BUILD_LUA_BINDINGS=ON ..
make -j$(nproc)
```

### 🐍 Python Binding Examples

```bash
# Run all Python examples
python examples/python/01_basic_plugin_management.py
python examples/python/02_communication_and_messaging.py
python examples/python/03_security_and_validation.py
python examples/python/04_orchestration_and_workflows.py

# Or run specific examples
cd examples/python
python 01_basic_plugin_management.py
```

### 🌙 Lua Binding Examples

```bash
# Run all Lua examples
lua examples/lua/01_basic_plugin_management.lua
lua examples/lua/02_communication_and_messaging.lua
lua examples/lua/03_comprehensive_features.lua

# Or run specific examples
cd examples/lua
lua 01_basic_plugin_management.lua
```

### 🌱 Start with C++ Fundamentals

```bash
# 1. Hello World - Your first plugin
cd 01-fundamentals/hello-world/build
./HelloWorldPluginTest

# 2. Basic Plugin - Core concepts
cd ../../basic-plugin/build
./BasicPluginTest

# 3. Configuration - Advanced config
cd ../../configuration/build
./ConfigurationTest
```

### 🔄 Explore Communication

```bash
# Message Bus patterns
cd 02-communication/message-bus/build
./MessageBusExample

# Request-Response patterns
cd ../../request-response/build
./RequestResponseExample

# Event-driven architecture
cd ../../event-driven/build
./EventDrivenExample
```

### 🚀 Complete Application

```bash
# Full feature demonstration
cd 06-comprehensive/full-application
./build.sh --run

# Expected output:
# 🚀 QtForge Comprehensive Demo v3.0.0
# =====================================
# [INIT] Initializing QtForge library...
# [CORE] Plugin manager initialized
# [SECURITY] Security level set to HIGH
# [COMMUNICATION] Message bus started
# [MONITORING] Metrics collection enabled
# [ORCHESTRATION] Workflow engine ready
# [TRANSACTIONS] Transaction manager active
# [MARKETPLACE] Plugin discovery enabled
# [THREADING] Thread pool (8 threads) ready
# [PYTHON] Python bridge initialized
#
# [LOADING] Loading plugins...
# ✅ DataProcessor v1.2.0 (data_processor.qtplugin)
# ✅ NetworkService v2.1.0 (network_service.qtplugin)
# ✅ UIComponent v1.0.0 (ui_component.qtplugin)
# ✅ SecurityScanner v1.5.0 (security_scanner.qtplugin)
# ✅ PythonScript v1.0.0 (python_script.py)
#
# [WORKFLOW] Executing comprehensive workflow...
# Step 1: Data validation ✅
# Step 2: Security scan ✅
# Step 3: Data processing ✅
# Step 4: Network transmission ✅
# Step 5: UI update ✅
#
# [SUCCESS] All features demonstrated successfully!
```

## 📖 Learning Path

### 🎯 Recommended Learning Sequence

**New to QtForge?** Follow this progression:

1. **[Hello World](01-fundamentals/hello-world/)** - Understand basic plugin structure (15 min)
2. **[Basic Plugin](01-fundamentals/basic-plugin/)** - Master core IPlugin interface (30 min)
3. **[Configuration](01-fundamentals/configuration/)** - Learn configuration management (20 min)
4. **[Message Bus](02-communication/message-bus/)** - Inter-plugin communication (30 min)
5. **[Background Tasks](03-services/background-tasks/)** - Service architecture (45 min)
6. **[Full Application](06-comprehensive/full-application/)** - See everything together (60 min)

### 🎯 By Use Case

**Building a Simple Plugin?**
→ Start with [Hello World](01-fundamentals/hello-world/) → [Basic Plugin](01-fundamentals/basic-plugin/)

**Need Inter-Plugin Communication?**
→ [Message Bus](02-communication/message-bus/) → [Request-Response](02-communication/request-response/)

**Building a Service?**
→ [Background Tasks](03-services/background-tasks/) → [Service Discovery](03-services/service-discovery/)

**Security Requirements?**
→ [Security Plugin](04-specialized/security/) → [Permission Management](04-specialized/security/)

**Network Integration?**
→ [Network Plugin](04-specialized/network/) → [REST API](04-specialized/network/)

**GUI Application?**
→ [UI Integration](04-specialized/ui-integration/) → [Qt Widgets](04-specialized/ui-integration/)

**Python Integration?**
→ [Python Bindings](05-integration/python-bindings/) → [Cross-Language](05-integration/python-bindings/)

**Production Deployment?**
→ [Monitoring](04-specialized/monitoring/) → [Performance Optimized](06-comprehensive/performance-optimized/)

## 🧪 Comprehensive Testing and Examples

The QtForge project includes extensive testing and example coverage:

### 📊 Test Coverage

- **C++ Core Tests** - Complete API coverage with unit, integration, and performance tests
- **Python Binding Tests** - 100% Python API coverage with comprehensive error handling
- **Lua Binding Tests** - 100% Lua API coverage with comprehensive error handling
- **Cross-Language Consistency** - Verification that all bindings behave consistently

### 📚 Example Coverage

- **C++ Examples** - Traditional plugin development patterns and advanced techniques
- **Python Examples** - Pythonic patterns and integration with Python ecosystem
- **Lua Examples** - Lightweight scripting patterns and embedded system integration

### 🔗 Related Resources

- **[tests/](../tests/)** - Comprehensive test suites for all languages
- **[tests/python/](../tests/python/)** - Python binding test documentation
- **[tests/lua/](../tests/lua/)** - Lua binding test documentation

### 🎯 By Experience Level

**🌱 Beginner (New to QtForge)**

- [Hello World](01-fundamentals/hello-world/) - First plugin
- [Basic Plugin](01-fundamentals/basic-plugin/) - Core concepts
- [Configuration](01-fundamentals/configuration/) - Config management

**🔄 Intermediate (Know the basics)**

- [Message Bus](02-communication/message-bus/) - Communication
- [Background Tasks](03-services/background-tasks/) - Services
- [Security](04-specialized/security/) - Security patterns

**🚀 Advanced (Building applications)**

- [Workflow Orchestration](03-services/workflow-orchestration/) - Complex workflows
- [Performance Optimized](06-comprehensive/performance-optimized/) - Optimization
- [Full Application](06-comprehensive/full-application/) - Complete system

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

## 🔧 Troubleshooting

### Common Issues and Solutions

#### 🚫 Plugin Loading Failures

- **Symptoms**: Plugin fails to load, "Invalid plugin file format" errors
- **Solutions**: Check file permissions, validate JSON metadata, verify Qt modules

#### ⚙️ Configuration Errors

- **Symptoms**: Configuration validation failures, type mismatches
- **Solutions**: Validate JSON syntax, check required fields, verify data types

#### 🏃 Runtime Errors

- **Symptoms**: Plugin crashes, memory violations, thread safety issues
- **Solutions**: Verify initialization, check resources, monitor memory, ensure thread safety

### Debug Configuration

```cpp
// Enable debug logging in main application
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Enable QtForge debug logging
    QLoggingCategory::setFilterRules("qtforge.*=true");

    // Set debug environment
    qputenv("QTFORGE_DEBUG", "1");
    qputenv("QTFORGE_LOG_LEVEL", "debug");

    // Your application code
    return app.exec();
}
```

## 🤝 Contributing

### Adding New Examples

1. **Create Directory Structure** in appropriate category (01-fundamentals/, 02-communication/, etc.)
2. **Implement Plugin** following existing patterns and conventions
3. **Add Build Configuration** with CMakeLists.txt
4. **Update Documentation** including this README
5. **Submit Pull Request** with comprehensive testing

### Guidelines

- Follow the established directory structure
- Include comprehensive error handling
- Add thorough documentation and examples
- Implement unit tests for all functionality
- Ensure cross-platform compatibility

## 📄 License

All examples are provided under the same MIT license as the QtForge library.

---

## 🚀 Migration from Old Structure

**Upgrading from previous examples?** See [MIGRATION_GUIDE.md](MIGRATION_GUIDE.md) for:

- Path mapping from old to new structure
- Automated migration tools
- Breaking changes and solutions
- Backward compatibility information
