# Plugin Development Guide

This comprehensive guide covers everything you need to know about developing plugins for QtForge, from basic concepts to advanced patterns and best practices.

## Overview

QtForge provides a powerful and flexible plugin architecture that enables developers to create modular, extensible applications. This guide will walk you through:

- Plugin architecture fundamentals
- Plugin lifecycle management
- Development patterns and best practices
- Testing and debugging strategies
- Performance optimization
- Security considerations

## Plugin Architecture Fundamentals

### Core Concepts

QtForge plugins are built around several key concepts:

1. **Plugin Interface**: All plugins implement the `IPlugin` interface
2. **Plugin Manager**: Manages plugin loading, initialization, and lifecycle
3. **Message Bus**: Enables communication between plugins
4. **Service Registry**: Provides service discovery and dependency injection
5. **Security Manager**: Handles plugin validation and permissions

### Plugin Interface

Every plugin must implement the `IPlugin` interface:

```cpp
class MyPlugin : public qtforge::IPlugin {
public:
    // Plugin metadata
    std::string name() const override { return "MyPlugin"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override { return "Example plugin"; }
    std::string author() const override { return "Developer Name"; }
    std::vector<std::string> dependencies() const override { return {}; }
    
    // Plugin lifecycle
    qtforge::expected<void, qtforge::Error> initialize() override;
    qtforge::expected<void, qtforge::Error> activate() override;
    qtforge::expected<void, qtforge::Error> deactivate() override;
    void cleanup() override;
    
    // Plugin state
    qtforge::PluginState state() const override { return currentState_; }
    bool isCompatible(const std::string& version) const override;

private:
    qtforge::PluginState currentState_ = qtforge::PluginState::Unloaded;
};
```

## Plugin Lifecycle Management

### Lifecycle States

Plugins progress through several states during their lifetime:

1. **Unloaded**: Plugin is not loaded into memory
2. **Loaded**: Plugin binary is loaded but not initialized
3. **Initialized**: Plugin is initialized but not active
4. **Active**: Plugin is running and functional
5. **Error**: Plugin encountered an error

### State Transitions

```cpp
qtforge::expected<void, qtforge::Error> MyPlugin::initialize() {
    try {
        qtforge::Logger::info(name(), "Initializing plugin...");
        
        // Initialize plugin resources
        initializeResources();
        
        // Register with services
        registerServices();
        
        // Setup message handlers
        setupMessageHandlers();
        
        currentState_ = qtforge::PluginState::Initialized;
        qtforge::Logger::info(name(), "Plugin initialized successfully");
        
        return {};
        
    } catch (const std::exception& e) {
        currentState_ = qtforge::PluginState::Error;
        return qtforge::Error("Initialization failed: " + std::string(e.what()));
    }
}

qtforge::expected<void, qtforge::Error> MyPlugin::activate() {
    if (currentState_ != qtforge::PluginState::Initialized) {
        return qtforge::Error("Plugin must be initialized before activation");
    }
    
    try {
        qtforge::Logger::info(name(), "Activating plugin...");
        
        // Start plugin operations
        startOperations();
        
        // Begin processing
        beginProcessing();
        
        currentState_ = qtforge::PluginState::Active;
        qtforge::Logger::info(name(), "Plugin activated successfully");
        
        return {};
        
    } catch (const std::exception& e) {
        currentState_ = qtforge::PluginState::Error;
        return qtforge::Error("Activation failed: " + std::string(e.what()));
    }
}
```

## Development Patterns

### Dependency Injection

Use the service registry for dependency injection:

```cpp
class DatabasePlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        // Get required services
        auto& serviceRegistry = qtforge::ServiceRegistry::instance();
        
        auto configService = serviceRegistry.getService<IConfigurationService>("config.service");
        if (!configService) {
            return qtforge::Error("Configuration service not available");
        }
        
        auto logService = serviceRegistry.getService<ILoggingService>("logging.service");
        if (!logService) {
            return qtforge::Error("Logging service not available");
        }
        
        configService_ = configService.value();
        logService_ = logService.value();
        
        // Initialize database connection
        auto dbConfig = configService_->getSection("database");
        database_ = std::make_unique<DatabaseConnection>(dbConfig);
        
        return {};
    }

private:
    std::shared_ptr<IConfigurationService> configService_;
    std::shared_ptr<ILoggingService> logService_;
    std::unique_ptr<DatabaseConnection> database_;
};
```

### Event-Driven Architecture

Use the message bus for event-driven communication:

```cpp
class EventProcessorPlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        auto& messageBus = qtforge::MessageBus::instance();
        
        // Subscribe to events
        subscriptions_.emplace_back(
            messageBus.subscribe<DataEvent>("data.events", 
                [this](const DataEvent& event) {
                    handleDataEvent(event);
                })
        );
        
        subscriptions_.emplace_back(
            messageBus.subscribe<SystemEvent>("system.events",
                [this](const SystemEvent& event) {
                    handleSystemEvent(event);
                })
        );
        
        return {};
    }
    
    void cleanup() override {
        // Unsubscribe from all events
        subscriptions_.clear();
    }

private:
    void handleDataEvent(const DataEvent& event) {
        // Process data event
        processData(event.data);
        
        // Publish result
        auto& messageBus = qtforge::MessageBus::instance();
        ProcessingResult result;
        result.sourceEvent = event.id;
        result.processedData = processedData;
        result.timestamp = std::chrono::system_clock::now();
        
        messageBus.publish("processing.results", result);
    }
    
    std::vector<qtforge::SubscriptionHandle> subscriptions_;
};
```

### Resource Management

Implement proper resource management with RAII:

```cpp
class ResourceManagerPlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        try {
            // Initialize resources with RAII
            fileManager_ = std::make_unique<FileManager>();
            networkManager_ = std::make_unique<NetworkManager>();
            threadPool_ = std::make_unique<ThreadPool>(4);
            
            // Setup resource monitoring
            resourceMonitor_ = std::make_unique<ResourceMonitor>();
            resourceMonitor_->setMemoryLimit(100 * 1024 * 1024); // 100MB
            resourceMonitor_->setCpuLimit(25.0); // 25%
            
            return {};
            
        } catch (const std::exception& e) {
            cleanup(); // Ensure cleanup on failure
            return qtforge::Error("Resource initialization failed: " + std::string(e.what()));
        }
    }
    
    void cleanup() override {
        // Resources are automatically cleaned up by unique_ptr destructors
        // But we can do explicit cleanup if needed
        if (threadPool_) {
            threadPool_->shutdown();
        }
        
        if (networkManager_) {
            networkManager_->disconnect();
        }
        
        // Reset all resources
        resourceMonitor_.reset();
        threadPool_.reset();
        networkManager_.reset();
        fileManager_.reset();
    }

private:
    std::unique_ptr<FileManager> fileManager_;
    std::unique_ptr<NetworkManager> networkManager_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::unique_ptr<ResourceMonitor> resourceMonitor_;
};
```

## Configuration Management

### Plugin Configuration

Handle plugin configuration properly:

```cpp
class ConfigurablePlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        // Load plugin configuration
        auto config = loadConfiguration();
        if (!config) {
            return config.error();
        }
        
        config_ = config.value();
        
        // Validate configuration
        auto validation = validateConfiguration(config_);
        if (!validation) {
            return validation.error();
        }
        
        // Apply configuration
        applyConfiguration(config_);
        
        return {};
    }

private:
    struct PluginConfig {
        std::string serverUrl;
        int connectionTimeout;
        int maxRetries;
        bool enableLogging;
        std::vector<std::string> allowedHosts;
    };
    
    qtforge::expected<PluginConfig, qtforge::Error> loadConfiguration() {
        auto& configService = qtforge::ServiceRegistry::instance()
            .getService<IConfigurationService>("config.service");
        
        if (!configService) {
            return qtforge::Error("Configuration service not available");
        }
        
        try {
            PluginConfig config;
            config.serverUrl = configService.value()->getString("server.url", "localhost");
            config.connectionTimeout = configService.value()->getInt("connection.timeout", 30);
            config.maxRetries = configService.value()->getInt("connection.retries", 3);
            config.enableLogging = configService.value()->getBool("logging.enabled", true);
            config.allowedHosts = configService.value()->getStringList("security.allowed_hosts");
            
            return config;
            
        } catch (const std::exception& e) {
            return qtforge::Error("Configuration loading failed: " + std::string(e.what()));
        }
    }
    
    qtforge::expected<void, qtforge::Error> validateConfiguration(const PluginConfig& config) {
        if (config.serverUrl.empty()) {
            return qtforge::Error("Server URL cannot be empty");
        }
        
        if (config.connectionTimeout <= 0) {
            return qtforge::Error("Connection timeout must be positive");
        }
        
        if (config.maxRetries < 0) {
            return qtforge::Error("Max retries cannot be negative");
        }
        
        return {};
    }
    
    PluginConfig config_;
};
```

## Error Handling

### Comprehensive Error Handling

Implement robust error handling throughout your plugin:

```cpp
class RobustPlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> processData(const std::vector<uint8_t>& data) {
        try {
            // Validate input
            if (data.empty()) {
                return qtforge::Error("Input data is empty");
            }
            
            if (data.size() > maxDataSize_) {
                return qtforge::Error("Input data exceeds maximum size limit");
            }
            
            // Process data with error checking
            auto parseResult = parseData(data);
            if (!parseResult) {
                return qtforge::Error("Data parsing failed: " + parseResult.error().message());
            }
            
            auto processResult = processInternal(parseResult.value());
            if (!processResult) {
                return qtforge::Error("Data processing failed: " + processResult.error().message());
            }
            
            auto saveResult = saveResults(processResult.value());
            if (!saveResult) {
                return qtforge::Error("Result saving failed: " + saveResult.error().message());
            }
            
            return {};
            
        } catch (const std::bad_alloc& e) {
            return qtforge::Error("Memory allocation failed: " + std::string(e.what()));
        } catch (const std::runtime_error& e) {
            return qtforge::Error("Runtime error: " + std::string(e.what()));
        } catch (const std::exception& e) {
            return qtforge::Error("Unexpected error: " + std::string(e.what()));
        } catch (...) {
            return qtforge::Error("Unknown error occurred");
        }
    }

private:
    qtforge::expected<ParsedData, qtforge::Error> parseData(const std::vector<uint8_t>& data) {
        // Implementation with error handling
        return ParsedData{};
    }
    
    qtforge::expected<ProcessedData, qtforge::Error> processInternal(const ParsedData& data) {
        // Implementation with error handling
        return ProcessedData{};
    }
    
    qtforge::expected<void, qtforge::Error> saveResults(const ProcessedData& data) {
        // Implementation with error handling
        return {};
    }
    
    static constexpr size_t maxDataSize_ = 10 * 1024 * 1024; // 10MB
};
```

## Testing Strategies

### Unit Testing

Create comprehensive unit tests for your plugins:

```cpp
#include <gtest/gtest.h>
#include "my_plugin.hpp"

class MyPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        plugin_ = std::make_unique<MyPlugin>();
    }
    
    void TearDown() override {
        if (plugin_) {
            plugin_->cleanup();
        }
    }
    
    std::unique_ptr<MyPlugin> plugin_;
};

TEST_F(MyPluginTest, InitializationSuccess) {
    auto result = plugin_->initialize();
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(plugin_->state(), qtforge::PluginState::Initialized);
}

TEST_F(MyPluginTest, ActivationAfterInitialization) {
    ASSERT_TRUE(plugin_->initialize().has_value());
    
    auto result = plugin_->activate();
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(plugin_->state(), qtforge::PluginState::Active);
}

TEST_F(MyPluginTest, ErrorHandling) {
    // Test error conditions
    auto result = plugin_->activate(); // Should fail without initialization
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(plugin_->state(), qtforge::PluginState::Unloaded);
}
```

### Integration Testing

Test plugin interactions:

```cpp
class PluginIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        pluginManager_ = std::make_unique<qtforge::PluginManager>();
        
        // Load and initialize plugins
        auto pluginA = pluginManager_->loadPlugin("PluginA.qtplugin");
        auto pluginB = pluginManager_->loadPlugin("PluginB.qtplugin");
        
        ASSERT_TRUE(pluginA.has_value());
        ASSERT_TRUE(pluginB.has_value());
        
        ASSERT_TRUE(pluginA.value()->initialize().has_value());
        ASSERT_TRUE(pluginB.value()->initialize().has_value());
        
        ASSERT_TRUE(pluginA.value()->activate().has_value());
        ASSERT_TRUE(pluginB.value()->activate().has_value());
    }
    
    std::unique_ptr<qtforge::PluginManager> pluginManager_;
};

TEST_F(PluginIntegrationTest, PluginCommunication) {
    auto& messageBus = qtforge::MessageBus::instance();
    
    // Test message passing between plugins
    bool messageReceived = false;
    auto handle = messageBus.subscribe<TestMessage>("test.messages",
        [&messageReceived](const TestMessage& msg) {
            messageReceived = true;
        });
    
    TestMessage testMsg;
    testMsg.content = "Hello from test";
    messageBus.publish("test.messages", testMsg);
    
    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(messageReceived);
}
```

## Best Practices

### 1. Plugin Design Principles

- **Single Responsibility**: Each plugin should have one clear purpose
- **Loose Coupling**: Minimize dependencies between plugins
- **High Cohesion**: Keep related functionality together
- **Interface Segregation**: Use focused interfaces

### 2. Performance Considerations

- **Lazy Loading**: Load resources only when needed
- **Memory Management**: Use RAII and smart pointers
- **Thread Safety**: Protect shared resources with appropriate synchronization
- **Resource Pooling**: Reuse expensive resources

### 3. Security Guidelines

- **Input Validation**: Always validate external input
- **Permission Checking**: Verify permissions before operations
- **Secure Communication**: Use encrypted channels for sensitive data
- **Resource Limits**: Implement appropriate resource limits

### 4. Maintainability

- **Clear Documentation**: Document all public interfaces
- **Consistent Naming**: Use consistent naming conventions
- **Error Messages**: Provide clear, actionable error messages
- **Version Compatibility**: Handle version changes gracefully

## Common Pitfalls

### 1. Memory Leaks

```cpp
// BAD: Raw pointer without cleanup
class BadPlugin : public qtforge::IPlugin {
    SomeResource* resource_;
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        resource_ = new SomeResource(); // Memory leak if cleanup() not called
        return {};
    }
};

// GOOD: RAII with smart pointers
class GoodPlugin : public qtforge::IPlugin {
    std::unique_ptr<SomeResource> resource_;
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        resource_ = std::make_unique<SomeResource>(); // Automatic cleanup
        return {};
    }
};
```

### 2. Circular Dependencies

```cpp
// BAD: Circular dependency
// PluginA depends on PluginB, PluginB depends on PluginA

// GOOD: Use mediator pattern or event-driven communication
class MediatorPlugin : public qtforge::IPlugin {
    // Coordinates between PluginA and PluginB without direct dependencies
};
```

### 3. Blocking Operations

```cpp
// BAD: Blocking the main thread
void processData() {
    auto result = longRunningOperation(); // Blocks
    handleResult(result);
}

// GOOD: Asynchronous processing
void processDataAsync() {
    std::async(std::launch::async, [this]() {
        auto result = longRunningOperation();
        
        // Post result back to main thread
        auto& messageBus = qtforge::MessageBus::instance();
        messageBus.publish("processing.completed", result);
    });
}
```

## Debugging Techniques

### 1. Logging

Use structured logging throughout your plugin:

```cpp
class DebuggablePlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> processRequest(const Request& request) {
        qtforge::Logger::debug(name(), "Processing request: " + request.id);
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        try {
            auto result = doProcessing(request);
            
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime);
            
            qtforge::Logger::info(name(), 
                "Request processed successfully in " + std::to_string(duration.count()) + "ms");
            
            return result;
            
        } catch (const std::exception& e) {
            qtforge::Logger::error(name(), 
                "Request processing failed: " + std::string(e.what()));
            return qtforge::Error("Processing failed: " + std::string(e.what()));
        }
    }
};
```

### 2. Plugin State Monitoring

Monitor plugin state and health:

```cpp
class MonitoredPlugin : public qtforge::IPlugin {
public:
    void reportHealth() {
        HealthReport report;
        report.pluginName = name();
        report.state = state();
        report.memoryUsage = getCurrentMemoryUsage();
        report.cpuUsage = getCurrentCpuUsage();
        report.lastActivity = lastActivity_;
        report.errorCount = errorCount_;
        
        auto& messageBus = qtforge::MessageBus::instance();
        messageBus.publish("plugin.health", report);
    }

private:
    std::chrono::system_clock::time_point lastActivity_;
    size_t errorCount_ = 0;
};
```

## See Also

- **[Plugin Architecture](../user-guide/plugin-architecture.md)**: Architectural overview
- **[Best Practices](best-practices.md)**: Development best practices
- **[Advanced Patterns](advanced-patterns.md)**: Advanced development patterns
- **[Testing Guide](../contributing/testing.md)**: Testing strategies and tools
- **[Performance Optimization](../user-guide/performance-optimization.md)**: Performance tuning
