# API Reference

Welcome to the comprehensive QtPlugin API reference. This documentation covers all public APIs, classes, and functions available in the QtPlugin library.

## Overview

QtPlugin provides a modern, type-safe C++ plugin system with the following key components:

- **[Core](#core-components)**: Essential plugin management functionality
- **[Communication](#communication-system)**: Inter-plugin messaging and events
- **[Security](#security-system)**: Plugin validation and trust management
- **[Utils](#utility-classes)**: Helper classes and error handling
- **[Optional Components](#optional-components)**: Network and UI extensions

## Quick Reference

### Essential Classes

| Class | Purpose | Header |
|-------|---------|--------|
| [`IPlugin`](core/plugin-interface.md) | Base plugin interface | `qtplugin/core/plugin_interface.hpp` |
| [`PluginManager`](core/plugin-manager.md) | Central plugin management | `qtplugin/core/plugin_manager.hpp` |
| [`PluginLoader`](core/plugin-loader.md) | Dynamic plugin loading | `qtplugin/core/plugin_loader.hpp` |
| [`MessageBus`](communication/message-bus.md) | Inter-plugin communication | `qtplugin/communication/message_bus.hpp` |
| [`SecurityManager`](security/security-manager.md) | Plugin security | `qtplugin/security/security_manager.hpp` |

### Key Types

| Type | Description | Header |
|------|-------------|--------|
| `expected<T, E>` | Error handling type | `qtplugin/utils/error_handling.hpp` |
| `PluginError` | Plugin error information | `qtplugin/utils/error_handling.hpp` |
| `PluginMetadata` | Plugin metadata structure | `qtplugin/core/plugin_interface.hpp` |
| `Version` | Version information | `qtplugin/utils/version.hpp` |

## Core Components

### Plugin System

The core plugin system provides the fundamental functionality for plugin management:

<div class="grid cards" markdown>

-   :material-puzzle: **[Plugin Interface](core/plugin-interface.md)**

    ---

    Base interface that all plugins must implement

    **Key Features:**
    - Lifecycle management (initialize, shutdown)
    - Metadata and identification
    - Command execution
    - Configuration management

-   :material-cog: **[Plugin Manager](core/plugin-manager.md)**

    ---

    Central component for managing plugins

    **Key Features:**
    - Plugin loading and unloading
    - Plugin discovery and registration
    - Lifecycle management
    - Error handling and recovery

-   :material-download: **[Plugin Loader](core/plugin-loader.md)**

    ---

    Handles dynamic loading of plugin libraries

    **Key Features:**
    - Cross-platform library loading
    - Symbol resolution
    - Dependency management
    - Error reporting

-   :material-database: **[Plugin Registry](core/plugin-registry.md)**

    ---

    Maintains registry of available and loaded plugins

    **Key Features:**
    - Plugin metadata storage
    - Dependency tracking
    - Version management
    - Query interface

</div>

## Communication System

Inter-plugin communication and messaging:

<div class="grid cards" markdown>

-   :material-message: **[Message Bus](communication/message-bus.md)**

    ---

    Central message routing and delivery system

    **Key Features:**
    - Type-safe messaging
    - Publish-subscribe pattern
    - Request-response communication
    - Event broadcasting

-   :material-format-list-bulleted-type: **[Message Types](communication/message-types.md)**

    ---

    Predefined message types and structures

    **Key Features:**
    - Standard message formats
    - Custom message support
    - Serialization helpers
    - Type validation

</div>

## Security System

Plugin validation and security management:

<div class="grid cards" markdown>

-   :material-shield-check: **[Security Manager](security/security-manager.md)**

    ---

    Comprehensive plugin security management

    **Key Features:**
    - Multi-layer validation
    - Trust management
    - Permission system
    - Security policies

-   :material-certificate: **[Plugin Validator](security/plugin-validator.md)**

    ---

    Plugin validation and verification

    **Key Features:**
    - File integrity checking
    - Digital signature verification
    - Runtime validation
    - Security scanning

</div>

## Utility Classes

Helper classes and common functionality:

<div class="grid cards" markdown>

-   :material-alert-circle: **[Error Handling](utils/error-handling.md)**

    ---

    Modern error handling with `expected<T, E>`

    **Key Features:**
    - Type-safe error handling
    - No exceptions
    - Composable error types
    - Rich error information

-   :material-tag: **[Version](utils/version.md)**

    ---

    Version information and comparison

    **Key Features:**
    - Semantic versioning
    - Version comparison
    - Compatibility checking
    - Version ranges

-   :material-check-circle: **[Concepts](utils/concepts.md)**

    ---

    C++20 concepts for type validation

    **Key Features:**
    - Compile-time validation
    - Plugin type concepts
    - Interface concepts
    - Constraint checking

-   :material-tools: **[Helpers](utils/helpers.md)**

    ---

    Utility functions and helpers

    **Key Features:**
    - Plugin creation helpers
    - CMake integration
    - Development utilities
    - Testing support

</div>

## Optional Components

Extended functionality for specific use cases:

<div class="grid cards" markdown>

-   :material-network: **[Network](optional/network.md)**

    ---

    Network-enabled plugin interfaces

    **Requirements:** Qt6::Network

    **Key Features:**
    - HTTP client/server plugins
    - WebSocket support
    - REST API integration
    - Network discovery

-   :material-monitor: **[UI](optional/ui.md)**

    ---

    User interface plugin support

    **Requirements:** Qt6::Widgets

    **Key Features:**
    - Widget-based plugins
    - Dialog integration
    - Custom UI components
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

| QtPlugin Version | Qt Version | C++ Standard | Status |
|------------------|------------|--------------|--------|
| 3.0.x | Qt 6.0+ | C++20 | Current |
| 2.x.x | Qt 5.15+ | C++17 | Legacy |
| 1.x.x | Qt 5.12+ | C++14 | Deprecated |

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
