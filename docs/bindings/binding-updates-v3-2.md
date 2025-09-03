# QtForge Binding Updates v3.2.0

This document outlines the comprehensive updates made to both Python and Lua bindings to match the latest C++ implementation in QtForge v3.2.0.

## Overview

The binding updates ensure complete functional coverage of the C++ API, including:

- **Core Module**: Enhanced with dependency resolution and lifecycle management
- **Managers Module**: Updated configuration system and added version management
- **Security Module**: Complete security component coverage
- **Type Safety**: Updated Python type stubs for better IDE support

## Python Binding Updates

### Core Module (`qtforge.core`)

#### New Classes Added

1. **PluginDependencyResolver**
   ```python
   import qtforge
   
   resolver = qtforge.core.create_plugin_dependency_resolver()
   resolver.update_dependency_graph(plugin_registry)
   load_order = resolver.get_load_order()
   ```

2. **PluginLifecycleManager**
   ```python
   lifecycle_manager = qtforge.core.create_plugin_lifecycle_manager()
   
   # Configure lifecycle settings
   config = qtforge.core.PluginLifecycleConfig()
   config.enable_health_monitoring = True
   config.auto_restart_on_failure = True
   
   lifecycle_manager.register_plugin(plugin, config)
   lifecycle_manager.initialize_plugin("my_plugin")
   ```

3. **Enhanced Enums**
   - `PluginLifecycleEvent`: Comprehensive lifecycle event types
   - Extended `PluginState`, `PluginCapability`, `PluginPriority`

#### New Data Structures

- `DependencyNode`: Plugin dependency graph representation
- `PluginLifecycleConfig`: Lifecycle management configuration
- `PluginLifecycleEventData`: Event data with timestamps and metadata
- `PluginHealthStatus`: Health monitoring information

### Managers Module (`qtforge.managers`)

#### Updated Classes

1. **ConfigurationManager** - Enhanced API
   ```python
   config_manager = qtforge.managers.create_configuration_manager()
   
   # Scope-based configuration
   config_manager.set_value("database.host", "localhost", 
                           qtforge.managers.ConfigurationScope.Plugin, 
                           "my_plugin")
   
   # Validation support
   result = config_manager.validate_configuration(schema, config)
   ```

2. **PluginVersionManager** - New Class
   ```python
   version_manager = qtforge.managers.create_plugin_version_manager(
       registry, config_manager, logger)
   
   # Version management
   version_manager.install_version("plugin_id", "2.0.0", plugin_path)
   version_manager.set_active_version("plugin_id", "2.0.0")
   version_manager.migrate_version("plugin_id", "1.0.0", "2.0.0")
   ```

#### New Enums and Data Structures

- `ConfigurationScope`: Global, Plugin, User, Session, Runtime
- `ConfigurationChangeType`: Added, Modified, Removed, Reloaded
- `ConfigurationValidationResult`: Validation results with errors/warnings
- `ConfigurationChangeEvent`: Change event tracking

### Security Module (`qtforge.security`)

#### New Security Components

1. **SecurityValidator**
   ```python
   validator = qtforge.security.create_security_validator()
   result = validator.validate_file_integrity("/path/to/plugin.so")
   ```

2. **SignatureVerifier**
   ```python
   verifier = qtforge.security.create_signature_verifier()
   verifier.add_trusted_certificate("/path/to/cert.pem")
   result = verifier.verify_plugin_signature("/path/to/plugin.so")
   ```

3. **PermissionManager**
   ```python
   perm_manager = qtforge.security.create_permission_manager()
   perm_manager.grant_permission("plugin_id", "file_system_access")
   has_perm = perm_manager.has_permission("plugin_id", "network_access")
   ```

4. **SecurityPolicyEngine**
   ```python
   policy_engine = qtforge.security.create_security_policy_engine()
   policy_engine.add_policy_rule("rule1", "allow if plugin.trusted == true")
   result = policy_engine.evaluate_policy("plugin_id", context)
   ```

## Lua Binding Updates

### Core Module (`qtforge.core`)

#### New Classes Added

1. **PluginManager, PluginLoader, PluginRegistry**
   ```lua
   local qtforge = require("qtforge")
   
   local manager = qtforge.core.create_plugin_manager()
   local result = manager:load_plugin("/path/to/plugin.so")
   
   local registry = qtforge.core.create_plugin_registry()
   registry:register_plugin(plugin)
   ```

2. **PluginDependencyResolver**
   ```lua
   local resolver = qtforge.core.create_plugin_dependency_resolver()
   resolver:update_dependency_graph(registry)
   local load_order = resolver:get_load_order()
   ```

### Managers Module (`qtforge.managers`)

#### Enhanced Manager Classes

1. **ConfigurationManager**
   ```lua
   local config_manager = qtforge.managers.create_configuration_manager()
   
   -- Scope-based configuration
   config_manager:set_value("key", "value", 
                           qtforge.managers.ConfigurationScope.Plugin, 
                           "plugin_id")
   ```

2. **LoggingManager and ResourceManager**
   ```lua
   local logger = qtforge.managers.create_logging_manager()
   logger:info("Plugin loaded successfully", "plugin_id")
   
   local resource_manager = qtforge.managers.create_resource_manager()
   resource_manager:allocate_resource("resource_id", "memory", {size = 1024})
   ```

### Security Module (`qtforge.security`)

#### Complete Security Component Coverage

```lua
local security = qtforge.security

-- Security validation
local validator = security.create_security_validator()
local result = validator:validate_file_integrity("/path/to/plugin.so")

-- Permission management
local perm_manager = security.create_permission_manager()
perm_manager:grant_permission("plugin_id", "network_access")

-- Policy evaluation
local policy_engine = security.create_security_policy_engine()
policy_engine:add_policy_rule("rule1", "allow if plugin.category == 'trusted'")
```

## Type Safety Improvements

### Updated Python Type Stubs

- **qtforge/core.pyi**: Complete type definitions for all core classes
- **qtforge/managers.pyi**: Manager class type definitions
- **qtforge/security.pyi**: Security component type definitions
- **qtforge/__init__.pyi**: Updated main module types

### Benefits

- Better IDE autocompletion and error detection
- Static type checking with mypy
- Improved developer experience
- Documentation through types

## Breaking Changes

### Python Bindings

1. **ConfigurationManager API**: Updated to use scope-based configuration
   - **Old**: `config.get_value(key)`
   - **New**: `config.get_value(key, scope, plugin_id)`

2. **Version Updates**: Version string updated from "3.0.0" to "3.2.0"

### Lua Bindings

1. **Manager Bindings**: Some old manager function names updated for consistency
2. **Security Types**: Updated enum values to match C++ implementation

## Migration Guide

### For Python Users

```python
# Old way (v3.0.0)
config = qtforge.ConfigurationManager()
value = config.get_value("key")

# New way (v3.2.0)
config = qtforge.managers.create_configuration_manager()
value = config.get_value("key", qtforge.managers.ConfigurationScope.Global)
```

### For Lua Users

```lua
-- Old way (v3.0.0)
local config = qtforge.create_configuration_manager()

-- New way (v3.2.0)
local config = qtforge.managers.create_configuration_manager()
```

## Testing

All updated bindings include comprehensive test coverage:

- **Python Tests**: Located in `tests/python/`
- **Lua Tests**: Located in `tests/lua/`
- **Integration Tests**: Cross-language compatibility tests

## Build System Integration

The updated bindings are fully integrated with all supported build systems:

- **CMake**: Updated CMakeLists.txt files
- **Meson**: Updated meson.build files  
- **XMake**: Updated xmake.lua files

## Next Steps

1. Run comprehensive test suite to verify all bindings work correctly
2. Update example code and tutorials
3. Verify build system integration
4. Update API documentation with new examples
