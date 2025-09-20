# Core Plugin Interfaces

This directory contains the fundamental plugin interfaces that form the foundation of the QtForge plugin system.

## Available Interfaces

### IPlugin (`plugin_interface.hpp`)
**Version**: 3.2.0  
**Interface ID**: `qtplugin.IPlugin/3.2`

The base interface that all plugins must implement. Provides core functionality including:
- Plugin lifecycle management (initialize, shutdown)
- Metadata and identification
- Command execution system
- Configuration management
- Error handling with `qtplugin::expected<T, PluginError>`

### IAdvancedPlugin (`advanced_plugin_interface.hpp`)
**Version**: 3.2.0
**Interface ID**: `qtplugin.IAdvancedPlugin/3.2`

Advanced plugin interface extending IPlugin with:
- Service contract support for formal inter-plugin communication
- Advanced communication capabilities with service discovery
- Dynamic capability negotiation and dependency management
- Enhanced metadata and versioning with configuration schemas
- Hot reload support for runtime plugin updates
- Dependency change handling for plugin lifecycle management
- Transaction support for atomic operations
- Configuration validation with JSON schemas

### IDynamicPlugin (`dynamic_plugin_interface.hpp`)
**Version**: 3.2.0  
**Interface ID**: `qtplugin.IDynamicPlugin/3.2`

Dynamic plugin interface providing runtime adaptation:
- Runtime interface discovery and adaptation
- Interface versioning and compatibility
- Optional interface extensions
- Capability negotiation
- Multi-language plugin support

### IServicePlugin (`service_plugin_interface.hpp`)
**Version**: 3.2.0
**Interface ID**: `qtplugin.IServicePlugin/3.2`

Service plugin interface for background services and long-running operations:
- Service lifecycle management (start, stop, pause, resume)
- Service health monitoring with status reporting
- Background processing capabilities with thread management
- Service priority management and execution modes
- Thread-safe service operations with proper synchronization
- Service statistics and performance metrics
- Service factory pattern for service creation

## Usage Guidelines

1. **Base Implementation**: All plugins should inherit from `IPlugin` as the minimum requirement
2. **Advanced Features**: Use `IAdvancedPlugin` for service contracts and advanced communication
3. **Runtime Adaptation**: Use `IDynamicPlugin` for plugins that need runtime interface adaptation
4. **Background Services**: Use `IServicePlugin` for long-running background services

## Interface Hierarchy

```text
IPlugin (base interface) - v3.2.0
├── IAdvancedPlugin (extends IPlugin) - v3.2.0
│   └── IDynamicPlugin (extends IAdvancedPlugin) - v3.2.0
└── IServicePlugin (extends IPlugin) - v3.2.0
```

## Best Practices

- Always implement the base `IPlugin` interface
- Use `qtplugin::expected<T, PluginError>` for error handling
- Follow semantic versioning for interface IDs
- Implement proper Qt integration with Q_OBJECT, Q_INTERFACES, Q_PLUGIN_METADATA
- Document all interface methods with Doxygen comments

## Version History

### Version 3.2.0 (Current)
- **IPlugin**: Updated to v3.2.0 with enhanced error handling and improved metadata support
- **IAdvancedPlugin**: **NEW** - Advanced plugin interface with service contracts and enhanced capabilities
- **IDynamicPlugin**: Updated to v3.2.0 with improved runtime adaptation and capability negotiation
- **IServicePlugin**: Updated to v3.2.0 with enhanced service management and factory pattern support
- **Interface Organization**: All core interfaces moved to `interfaces/core/` directory for better organization
- **Backward Compatibility**: Forwarding headers maintained in original locations

### Version 3.1.0
- **IPlugin**: Enhanced with service contract support and transaction capabilities
- **IDynamicPlugin**: Introduced dynamic interface adaptation
- **IServicePlugin**: Enhanced service lifecycle management

### Version 3.0.0
- **IPlugin**: Base plugin interface with core functionality
- **IServicePlugin**: Basic service plugin support

## See Also

- [Plugin Development Guide](../../../../docs/user-guide/plugin-development.md)
- [Interface Design Guidelines](../../../../docs/contributing/interface-guidelines.md)
- [Error Handling Utilities](../../utils/error_handling.hpp)
- [Interface Migration Guide](../MIGRATION_PLAN.md)
- [Backward Compatibility Strategy](../BACKWARD_COMPATIBILITY.md)
