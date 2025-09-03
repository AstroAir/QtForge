# QtForge Lua API Overview

!!! info "Lua Integration"
**Module**: `qtforge`  
 **Lua Version**: 5.4+  
 **Since**: QtForge v3.0.0  
 **Status**: Stable

## Overview

QtForge provides comprehensive Lua bindings that expose the full C++ API to Lua scripts and applications. The bindings are built using sol2 and provide a natural Lua interface while maintaining the performance and capabilities of the underlying C++ implementation.

### Key Features

- **Complete API Coverage**: All major QtForge components available in Lua
- **Type Safety**: Proper error handling and type checking
- **Performance**: Minimal overhead with direct C++ integration
- **Lua-native Interface**: Natural Lua syntax and conventions
- **Cross-Platform**: Works on Windows, macOS, and Linux
- **Embedded Support**: Perfect for embedded scripting scenarios

### Module Structure

The QtForge Lua package provides access to all major components:

```lua
local qtforge = require('qtforge')

-- Core plugin system
local core = qtforge.core                    -- Plugin management, loading, registry
local utils = qtforge.utils                  -- Utility classes and functions
local communication = qtforge.communication -- Message bus and inter-plugin communication
local security = qtforge.security           -- Security and validation components

-- Advanced features
local orchestration = qtforge.orchestration -- Workflow management and plugin orchestration
local monitoring = qtforge.monitoring       -- Hot reload, metrics collection, performance monitoring
local transactions = qtforge.transactions   -- ACID transactions for plugin operations
local composition = qtforge.composition     -- Plugin composition and architectural patterns
local marketplace = qtforge.marketplace     -- Plugin discovery, installation, and updates
local threading = qtforge.threading         -- Thread pool management and concurrency utilities

-- Management components
local managers = qtforge.managers           -- Configuration, logging, and resource management
```

## Installation

### Prerequisites

- **Lua 5.4+** with development headers
- **Qt6** (6.0 or later)
- **QtForge C++ library** (v3.0.0 or later)
- **sol2** (automatically handled by build system)

### Installation Methods

#### Using LuaRocks

```bash
# Install from LuaRocks (when available)
luarocks install qtforge

# Install with specific Lua version
luarocks --lua-version=5.4 install qtforge
```

#### Building from Source

```bash
# Clone the repository
git clone https://github.com/AstroAir/QtForge.git
cd QtForge

# Build Lua bindings
mkdir build && cd build
cmake .. -DQTFORGE_BUILD_LUA=ON -DLUA_EXECUTABLE=$(which lua)
cmake --build . --target qtforge_lua

# Install Lua module
sudo make install
```

## Quick Start

### Basic Plugin Management

```lua
local qtforge = require('qtforge')

-- Create plugin manager
local manager = qtforge.core.PluginManager()

-- Load a plugin
local result = manager:load_plugin("./plugins/example_plugin.so")
if result.success then
    print("Plugin loaded successfully: " .. result.plugin_id)
else
    print("Failed to load plugin: " .. result.error)
end

-- Get plugin information
local plugin = manager:get_plugin(result.plugin_id)
if plugin then
    print("Plugin name: " .. plugin:name())
    print("Plugin version: " .. plugin:version():to_string())
    print("Plugin state: " .. tostring(plugin:state()))
end
```

### Plugin Communication

```lua
local qtforge = require('qtforge')

-- Create message bus
local bus = qtforge.communication.MessageBus()

-- Subscribe to messages
bus:subscribe("plugin.status", function(message)
    print("Received status update: " .. message.data.status)
end)

-- Send a message
local message = qtforge.communication.Message()
message.type = "plugin.command"
message.data = {command = "start", target = "example_plugin"}
bus:publish(message)
```

### Configuration Management

```lua
local qtforge = require('qtforge')

-- Load configuration
local config = qtforge.managers.ConfigurationManager()
config:load_from_file("config.json")

-- Access configuration values
local plugin_dir = config:get_string("plugin_directory", "./plugins")
local auto_load = config:get_bool("auto_load_plugins", true)
local log_level = config:get_string("log_level", "info")

print("Plugin directory: " .. plugin_dir)
print("Auto load: " .. tostring(auto_load))
print("Log level: " .. log_level)
```

## API Reference by Module

### Core Module (`qtforge.core`)

Essential plugin management functionality:

- **PluginManager** - Central plugin management
- **IPlugin** - Plugin interface and base classes
- **PluginLoader** - Dynamic plugin loading
- **PluginRegistry** - Plugin registration and discovery
- **PluginDependencyResolver** - Dependency resolution
- **PluginLifecycleManager** - Lifecycle management

### Communication Module (`qtforge.communication`)

Inter-plugin messaging and events:

- **MessageBus** - Publish/subscribe messaging system
- **Message** - Message structure and types
- **RequestResponseSystem** - Request/response communication
- **ServiceContract** - Service interface definitions

### Security Module (`qtforge.security`)

Plugin validation and security:

- **SecurityManager** - Security policy enforcement
- **PluginValidator** - Plugin validation and verification
- **TrustManager** - Trust level management
- **PermissionSystem** - Permission-based access control

### Utils Module (`qtforge.utils`)

Utility classes and functions:

- **Version** - Version information and comparison
- **ErrorHandling** - Error types and handling
- **Logger** - Logging functionality
- **FileSystem** - File system utilities

## Advanced Features

### Plugin Orchestration

```lua
local qtforge = require('qtforge')

-- Create orchestrator
local orchestrator = qtforge.orchestration.PluginOrchestrator()

-- Define workflow
local workflow = qtforge.orchestration.Workflow()
workflow:add_step("load_data", "data_loader_plugin")
workflow:add_step("process_data", "data_processor_plugin")
workflow:add_step("save_results", "data_saver_plugin")

-- Execute workflow
orchestrator:execute_workflow(workflow)
```

### Hot Reload Support

```lua
local qtforge = require('qtforge')

-- Enable hot reload
local hot_reload = qtforge.monitoring.PluginHotReloadManager()
hot_reload:enable_auto_reload(true)
hot_reload:watch_directory("./plugins")

-- Set up reload callback
hot_reload:on_plugin_reloaded(function(plugin_id)
    print("Plugin reloaded: " .. plugin_id)
end)
```

### Transaction Support

```lua
local qtforge = require('qtforge')

-- Create transaction manager
local tx_manager = qtforge.transactions.PluginTransactionManager()

-- Execute transactional operations
tx_manager:begin_transaction()
try {
    -- Perform plugin operations
    manager:load_plugin("plugin1.so")
    manager:load_plugin("plugin2.so")
    manager:configure_plugin("plugin1", config)
    
    -- Commit if all successful
    tx_manager:commit()
} catch (error) {
    -- Rollback on error
    tx_manager:rollback()
    print("Transaction failed: " .. error)
}
```

## Error Handling

QtForge Lua bindings provide comprehensive error handling:

```lua
local qtforge = require('qtforge')

-- Handle plugin loading errors
local result = manager:load_plugin("nonexistent.so")
if not result.success then
    local error = result.error
    print("Error code: " .. error.code)
    print("Error message: " .. error.message)
    print("Error details: " .. error.details)
end

-- Use pcall for exception handling
local success, result = pcall(function()
    return manager:load_plugin("problematic.so")
end)

if not success then
    print("Exception caught: " .. result)
end
```

## Performance Considerations

### Best Practices

1. **Reuse Objects**: Create manager instances once and reuse them
2. **Batch Operations**: Group related operations together
3. **Async Operations**: Use async APIs for long-running operations
4. **Memory Management**: Lua's GC handles C++ object cleanup automatically

### Memory Usage

```lua
-- Monitor memory usage
local metrics = qtforge.monitoring.PluginMetricsCollector()
local memory_usage = metrics:get_memory_usage()
print("Total memory: " .. memory_usage.total .. " MB")
print("Plugin memory: " .. memory_usage.plugins .. " MB")
```

## Integration Examples

### With Love2D Game Engine

```lua
-- In your Love2D game
local qtforge = require('qtforge')
local manager = qtforge.core.PluginManager()

function love.load()
    -- Load game plugins
    manager:load_plugin("audio_plugin.so")
    manager:load_plugin("graphics_plugin.so")
end

function love.update(dt)
    -- Update plugins
    manager:update_all_plugins(dt)
end
```

### With OpenResty/Nginx

```lua
-- In your OpenResty application
local qtforge = require('qtforge')
local manager = qtforge.core.PluginManager()

-- Load web service plugins
manager:load_plugin("auth_plugin.so")
manager:load_plugin("cache_plugin.so")

-- Use in request handler
local auth = manager:get_plugin("auth_plugin")
if auth:validate_token(ngx.var.token) then
    -- Process request
end
```

---

_Last updated: September 2024 | QtForge v3.0.0_
