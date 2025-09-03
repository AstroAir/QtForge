# API Reference

Welcome to the comprehensive QtForge API reference. This documentation covers all public APIs, classes, and functions available in the QtForge library.

## Overview

QtForge provides a modern, type-safe C++ plugin system with the following key components:

- **[Core](#core-components)**: Essential plugin management functionality
- **[Orchestration](#orchestration-system)**: Plugin workflow and orchestration
- **[Monitoring](#monitoring-system)**: Plugin monitoring and metrics
- **[Python Integration](#python-integration)**: Python binding support
- **[Language Bindings](#language-bindings)**: Multi-language support

## Quick Reference

### Essential Classes

| Class                                             | Purpose                    | Header                                   |
| ------------------------------------------------- | -------------------------- | ---------------------------------------- |
| [`IPlugin`](core/plugin-interface.md)             | Base plugin interface      | `qtplugin/core/plugin_interface.hpp`     |
| [`CompositePlugin`](composition/composite-plugin.md) | Plugin composition system | `qtplugin/composition/composite_plugin.hpp` |
| [`PluginMarketplace`](marketplace/plugin-marketplace.md) | Plugin marketplace | `qtplugin/marketplace/plugin_marketplace.hpp` |
| [`PluginOrchestrator`](orchestration/plugin-orchestrator.md) | Plugin orchestration | `qtplugin/orchestration/plugin_orchestrator.hpp` |
| [`PluginTransactionManager`](transactions/plugin-transaction-manager.md) | Transaction management | `qtplugin/transactions/plugin_transaction_manager.hpp` |

### Key Types

| Type             | Description               | Header                               |
| ---------------- | ------------------------- | ------------------------------------ |
| `expected<T, E>` | Error handling type       | `qtplugin/utils/error_handling.hpp`  |
| `PluginError`    | Plugin error information  | `qtplugin/utils/error_handling.hpp`  |
| `PluginMetadata` | Plugin metadata structure | `qtplugin/core/plugin_interface.hpp` |
| `Version`        | Version information       | `qtplugin/utils/version.hpp`         |

## Core Components

### Plugin System

The core plugin system provides the fundamental functionality for plugin management:

<div class="grid cards" markdown>

- :material-puzzle: **[Plugin Interface](core/plugin-interface.md)**

  ***

  Base interface that all plugins must implement

  **Key Features:**

  - Lifecycle management (initialize, shutdown)
  - Metadata and identification
  - Command execution
  - Configuration management

- :material-cog: **[Composite Plugin](composition/composite-plugin.md)**

  ***

  Advanced plugin composition system

  **Key Features:**

  - Plugin composition patterns
  - Hierarchical plugin structures
  - Component orchestration
  - Advanced lifecycle management

- :material-download: **[Plugin Marketplace](marketplace/plugin-marketplace.md)**

  ***

  Plugin marketplace and distribution system

  **Key Features:**

  - Plugin discovery and installation
  - Version management
  - Security validation
  - Marketplace integration

- :material-database: **[Transaction Manager](transactions/plugin-transaction-manager.md)**

  ***

  Transaction management for plugin operations

  **Key Features:**

  - ACID transaction support
  - Rollback capabilities
  - State management
  - Error recovery

</div>

## Orchestration System

Plugin orchestration and workflow management:

<div class="grid cards" markdown>

- :material-message: **[Plugin Orchestrator](orchestration/plugin-orchestrator.md)**

  ***

  Plugin workflow orchestration system

  **Key Features:**

  - Workflow definition and execution
  - Plugin coordination
  - Event-driven orchestration
  - Error handling and recovery

- :material-format-list-bulleted-type: **[Advanced Orchestrator](orchestration/advanced-orchestrator.md)**

  ***

  Advanced orchestration patterns and features

  **Key Features:**

  - Complex workflow patterns
  - Conditional execution
  - Parallel processing
  - Advanced error handling

</div>

## Monitoring System

Plugin monitoring and performance tracking:

<div class="grid cards" markdown>

- :material-shield-check: **[Hot Reload Manager](monitoring/plugin-hot-reload-manager.md)**

  ***

  Dynamic plugin hot-reloading system

  **Key Features:**

  - Runtime plugin reloading
  - State preservation
  - Dependency management
  - Development workflow support

- :material-certificate: **[Metrics Collector](monitoring/plugin-metrics-collector.md)**

  ***

  Plugin performance monitoring and metrics

  **Key Features:**

  - Performance metrics collection
  - Resource usage tracking
  - Runtime statistics
  - Monitoring dashboards

</div>

## Python Integration

Python bindings and integration support:

<div class="grid cards" markdown>

- :material-alert-circle: **[Python Overview](python/overview.md)**

  ***

  Python integration overview and capabilities

  **Key Features:**

  - Python plugin support
  - Bidirectional communication
  - Type conversion
  - Error handling integration

- :material-tag: **[Python Plugin Manager](python/core/plugin-manager.md)**

  ***

  Python-specific plugin management

  **Key Features:**

  - Python plugin loading
  - Virtual environment support
  - Dependency management
  - Python-C++ interop

- :material-check-circle: **[Python Orchestrator](python/orchestration/plugin-orchestrator.md)**

  ***

  Python workflow orchestration

  **Key Features:**

  - Python workflow definition
  - Mixed C++/Python workflows
  - Async/await support
  - Error propagation

## Language Bindings

Multi-language support and bindings:

<div class="grid cards" markdown>

- :material-network: **[Lua Integration](lua/overview.md)**

  ***

  Lua scripting support for plugins

  **Key Features:**

  - Lua plugin development
  - C++ to Lua bindings
  - Script execution environment
  - Dynamic scripting support

</div>
  - Theme support

</div>

## Usage Patterns

### Basic Plugin Loading

```cpp
#include <qtplugin/qtplugin.hpp>

// Initialize library
qtplugin::LibraryInitializer init;

// Create manager
auto manager = qtplugin::PluginManager::create();

// Load plugin
auto result = manager->load_plugin("path/to/plugin.so");
if (result) {
    auto plugin = manager->get_plugin(result.value());
    auto init_result = plugin->initialize();
    // Use plugin...
}
```

### Error Handling

```cpp
auto result = manager->load_plugin("plugin.so");
if (!result) {
    std::cerr << "Failed to load plugin: "
              << result.error().message << std::endl;
    return;
}

// Success path
auto plugin_id = result.value();
```

### Message Communication

```cpp
// Get message bus
auto& bus = manager->message_bus();

// Subscribe to messages
bus.subscribe<MyMessageType>([](const MyMessageType& msg) {
    // Handle message
});

// Send message
MyMessageType message{"Hello, World!"};
bus.publish(message);
```

## API Conventions

### Naming

- **Classes**: `PascalCase` (e.g., `PluginManager`)
- **Functions**: `snake_case` (e.g., `load_plugin`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_PLUGINS`)
- **Namespaces**: `lowercase` (e.g., `qtplugin`)

### Error Handling

QtPlugin uses the `expected<T, E>` pattern for error handling:

```cpp
// Function that can fail
qtplugin::expected<std::string, qtplugin::PluginError>
load_plugin(const std::string& path);

// Usage
auto result = load_plugin("plugin.so");
if (result) {
    // Success - use result.value()
} else {
    // Error - use result.error()
}
```

### Memory Management

- **RAII**: All resources are automatically managed
- **Smart Pointers**: Use `std::shared_ptr` and `std::unique_ptr`
- **No Manual Memory Management**: No need for `new`/`delete`

### Thread Safety

- **Thread-Safe Classes**: Marked in documentation
- **Synchronization**: Internal synchronization where needed
- **Concurrent Access**: Safe for multi-threaded applications

## Version Compatibility

| QtPlugin Version | Qt Version | C++ Standard | Status     |
| ---------------- | ---------- | ------------ | ---------- |
| 3.0.x            | Qt 6.0+    | C++20        | Current    |
| 2.x.x            | Qt 5.15+   | C++17        | Legacy     |
| 1.x.x            | Qt 5.12+   | C++14        | Deprecated |

## Getting Started

New to QtPlugin? Start with these guides:

1. **[Getting Started](../getting-started/overview.md)** - Overview and concepts
2. **[Installation](../getting-started/installation.md)** - Install QtPlugin
3. **[Quick Start](../getting-started/quick-start.md)** - First application
4. **[First Plugin](../getting-started/first-plugin.md)** - Create a plugin

## Advanced Topics

For advanced usage:

1. **[Plugin Development](../developer-guide/plugin-development.md)** - Comprehensive plugin guide
2. **[Advanced Patterns](../developer-guide/advanced-patterns.md)** - Advanced techniques
3. **[Architecture](../architecture/system-design.md)** - System design
4. **[Best Practices](../developer-guide/best-practices.md)** - Production guidelines

## Support

- **Documentation**: Browse this API reference
- **Examples**: Check [examples](../examples/index.md)
- **Issues**: Report on [GitHub](https://github.com/QtForge/QtPlugin/issues)
- **Discussions**: Ask on [GitHub Discussions](https://github.com/QtForge/QtPlugin/discussions)
