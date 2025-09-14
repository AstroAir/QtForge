# Plugin Types

This document describes the different types of plugins supported by the QtForge plugin system.

## Overview

QtForge supports multiple plugin types to accommodate different use cases and development patterns:

- **Core Plugins** - Essential system plugins
- **Service Plugins** - Background service implementations
- **UI Plugins** - User interface extensions
- **Network Plugins** - Network-related functionality
- **Custom Plugins** - Application-specific implementations

## Plugin Type Definitions

### Core Plugin Type

Core plugins provide essential functionality that is loaded at system startup.

```cpp
class CorePlugin : public PluginInterface {
public:
    PluginType type() const override {
        return PluginType::Core;
    }
    
    // Implementation details...
};
```

### Service Plugin Type

Service plugins run in the background and provide services to other plugins.

```cpp
class ServicePlugin : public PluginInterface {
public:
    PluginType type() const override {
        return PluginType::Service;
    }
    
    // Implementation details...
};
```

### UI Plugin Type

UI plugins extend the user interface with new widgets, dialogs, or windows.

```cpp
class UIPlugin : public PluginInterface {
public:
    PluginType type() const override {
        return PluginType::UI;
    }
    
    // Implementation details...
};
```

### Network Plugin Type

Network plugins handle network communications and protocols.

```cpp
class NetworkPlugin : public PluginInterface {
public:
    PluginType type() const override {
        return PluginType::Network;
    }
    
    // Implementation details...
};
```

## Type Registry

The plugin system maintains a registry of all supported plugin types:

```cpp
enum class PluginType {
    Unknown = 0,
    Core,
    Service,
    UI,
    Network,
    Custom
};

class PluginTypeRegistry {
public:
    static QString typeToString(PluginType type);
    static PluginType stringToType(const QString& typeStr);
    static QList<PluginType> supportedTypes();
};
```

## Usage Examples

### Registering a Plugin Type

```cpp
// Register a custom plugin type
PluginManager manager;
manager.registerPluginType<MyCustomPlugin>("MyCustom");
```

### Loading by Type

```cpp
// Load all plugins of a specific type
auto corePlugins = manager.getPluginsByType(PluginType::Core);
auto servicePlugins = manager.getPluginsByType(PluginType::Service);
```

## Best Practices

1. **Choose Appropriate Types**: Select the plugin type that best matches your functionality
2. **Consistent Naming**: Use consistent naming conventions for plugin types
3. **Type-Specific Interfaces**: Consider creating specialized interfaces for each plugin type
4. **Loading Order**: Be aware that plugin loading order may depend on type

## See Also

- [Plugin Interface](plugin-interface.md)
- [Plugin Manager](plugin-manager.md)
- [Plugin Registry](plugin-registry.md)
