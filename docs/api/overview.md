# API Overview

QtForge v3.2.0 provides a comprehensive C++ API for building modular, plugin-based applications with advanced multilingual support. This overview introduces the key concepts and components of the framework.

## Core Architecture

QtForge is built around several key architectural principles:

- **Advanced Plugin Interfaces**: Support for `IPlugin`, `IAdvancedPlugin`, and `IDynamicPlugin`
- **Plugin Manager**: Central orchestration of plugin lifecycle with hot reload support
- **Service Contract System**: Type-safe inter-plugin communication with service discovery
- **Enhanced Security Model**: Multi-layer plugin validation, sandboxing, and trust management
- **Resource Management**: Automatic resource lifecycle tracking with monitoring
- **Multilingual Support**: Native support for C++, Python, and Lua plugins

## API Organization

The QtForge API is organized into several modules:

### Core Module (`qtforge::core`)

The foundation of the plugin system with enhanced interfaces:

- **[IPlugin](core/plugin-interface.md)**: Base interface for all plugins
- **[IAdvancedPlugin](core/advanced-plugin-interface.md)**: Advanced plugin interface with service contracts
- **[IDynamicPlugin](core/dynamic-plugin-interface.md)**: Dynamic interface adaptation and capability negotiation
- **[PluginManager](core/plugin-manager.md)**: Central plugin management with hot reload support
- **[PluginLoader](core/plugin-loader.md)**: Dynamic library loading with multilingual support
- **[PluginType](core/plugin-types.md)**: Support for Native, Python, Lua, JavaScript, Remote, and Composite plugins

### Communication Module (`qtforge::communication`)

Inter-plugin messaging and events:

- **[MessageBus](communication/message-bus.md)**: Central message routing
- **[MessageTypes](communication/message-types.md)**: Standard message formats

### Security Module (`qtforge::security`)

Enhanced plugin validation, sandboxing, and trust management:

- **[SecurityManager](security/security-manager.md)**: Security policy enforcement and management
- **[PluginValidator](security/plugin-validator.md)**: Plugin verification and validation
- **[PluginSandbox](security/plugin-sandbox.md)**: Advanced plugin sandboxing with policy validation
- **[SecurityPolicyValidator](security/security-policy-validator.md)**: Security policy integrity validation
- **[ResourceMonitor](security/resource-monitor.md)**: Resource usage monitoring and threshold management
- **[SecurityEnforcer](security/security-enforcer.md)**: Policy enforcement with signal handling

### Orchestration Module (`qtforge::orchestration`)

Workflow and process management:

- **[PluginOrchestrator](orchestration/plugin-orchestrator.md)**: Workflow coordination

### Monitoring Module (`qtforge::monitoring`)

Performance and health monitoring:

- **[HotReloadManager](monitoring/plugin-hot-reload-manager.md)**: Development-time reloading
- **[MetricsCollector](monitoring/plugin-metrics-collector.md)**: Performance metrics

### Transaction Module (`qtforge::transactions`)

ACID transaction support:

- **[TransactionManager](transactions/plugin-transaction-manager.md)**: Transaction coordination

### Composition Module (`qtforge::composition`)

Advanced plugin composition patterns:

- **[CompositePlugin](composition/composite-plugin.md)**: Plugin aggregation

### Marketplace Module (`qtforge::marketplace`)

Plugin distribution and discovery:

- **[PluginMarketplace](marketplace/plugin-marketplace.md)**: Plugin marketplace integration

## Language Bindings

QtForge supports multiple programming languages:

### Python Bindings (`qtforge.python`)

Full Python integration with:
- **[Python Plugin Manager](python/core/plugin-manager.md)**: Python-specific management
- **[Python Orchestrator](python/orchestration/plugin-orchestrator.md)**: Python workflows

### Lua Integration (`qtforge.lua`)

Lua scripting support:
- **[Lua Overview](lua/overview.md)**: Lua integration capabilities

## Key Concepts

### Plugin Lifecycle

1. **Discovery**: Plugins are discovered through filesystem scanning or registry
2. **Loading**: Dynamic library loading and symbol resolution
3. **Validation**: Security and compatibility checks
4. **Initialization**: Plugin-specific setup and resource allocation
5. **Activation**: Plugin becomes available for use
6. **Deactivation**: Graceful shutdown and resource cleanup
7. **Unloading**: Library unloading and memory cleanup

### Message-Driven Architecture

QtForge uses a message-driven architecture for loose coupling:

```cpp
// Send a message
auto message = std::make_shared<CustomMessage>("data");
messageBus.publish("topic", message);

// Receive messages
messageBus.subscribe("topic", [](const auto& message) {
    // Handle message
});
```

### Error Handling

QtForge uses modern C++ error handling with `expected<T, E>`:

```cpp
auto result = pluginManager.loadPlugin("plugin.dll");
if (result) {
    auto plugin = result.value();
    // Use plugin
} else {
    auto error = result.error();
    // Handle error
}
```

### Resource Management

Automatic resource management through RAII and smart pointers:

```cpp
class MyPlugin : public IPlugin {
    std::unique_ptr<Resource> resource_;
public:
    expected<void, Error> initialize() override {
        resource_ = std::make_unique<Resource>();
        return {};
    }
    // Resource automatically cleaned up in destructor
};
```

## Threading Model

QtForge is designed for multi-threaded environments:

- **Thread-Safe**: All public APIs are thread-safe
- **Async Operations**: Non-blocking operations with futures/promises
- **Worker Threads**: Background processing support
- **Qt Integration**: Seamless Qt event loop integration

## Performance Considerations

### Memory Management

- **Smart Pointers**: Automatic memory management
- **Object Pools**: Reusable object allocation
- **Lazy Loading**: On-demand resource allocation

### Optimization Features

- **Plugin Caching**: Compiled plugin metadata caching
- **Hot Reloading**: Development-time plugin reloading
- **Metrics Collection**: Performance monitoring and profiling

## Integration Patterns

### Qt Application Integration

```cpp
#include <QApplication>
#include <qtforge/core/plugin_manager.hpp>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    qtforge::PluginManager manager;
    manager.loadPluginsFromDirectory("plugins/");
    
    return app.exec();
}
```

### CMake Integration

```cmake
find_package(QtForge REQUIRED)
target_link_libraries(your_app QtForge::Core QtForge::Communication)
```

## Best Practices

1. **Interface Design**: Keep plugin interfaces minimal and stable
2. **Error Handling**: Always check return values and handle errors gracefully
3. **Resource Management**: Use RAII and smart pointers consistently
4. **Threading**: Be aware of thread safety requirements
5. **Testing**: Write comprehensive tests for plugin interactions

## Getting Started

1. **[Installation](../installation.md)**: Set up QtForge in your environment
2. **[Quick Start](../getting-started/quick-start.md)**: Create your first plugin
3. **[Examples](../examples/index.md)**: Study working examples
4. **[User Guide](../user-guide/plugin-management.md)**: Learn advanced features

## API Reference

For detailed API documentation, see:

- **[Core API](core/plugin-interface.md)**: Essential plugin interfaces
- **[Communication API](communication/message-bus.md)**: Messaging system
- **[Security API](security/security-manager.md)**: Security framework
- **[Orchestration API](orchestration/plugin-orchestrator.md)**: Workflow management
