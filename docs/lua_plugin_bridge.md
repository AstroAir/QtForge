# QtForge Lua Plugin Bridge

The QtForge Lua Plugin Bridge provides comprehensive support for creating and executing Lua-based plugins within the QtForge plugin system. This document describes the enhanced functionality and usage.

## Overview

The Lua Plugin Bridge consists of two main components:

1. **LuaExecutionEnvironment** - Manages Lua state and provides execution context
2. **LuaPluginBridge** - Implements the IPlugin interface for Lua-based plugins

## Features

### Core Functionality

- ✅ Full Lua state management with sol2 integration
- ✅ Comprehensive QtForge API bindings
- ✅ Method and property invocation from C++
- ✅ Sandboxing support for security
- ✅ Error handling and debugging support
- ✅ Hot reload capabilities

### Available Bindings

#### Core Module (`qtforge.core`)

- Version management
- Plugin states and capabilities
- Error handling types
- Utility functions

#### Utils Module (`qtforge.utils`)

- String manipulation (trim, split, join, case conversion)
- File path utilities
- Time and timestamp functions
- UUID generation
- Hash utilities
- Version parsing and comparison
- Input validation (email, URL)
- Math utilities (clamp, lerp)

#### Threading Module (`qtforge.threading`)

- Thread information and utilities
- Sleep and yield functions
- Thread pool management
- Mutex support
- Timer creation and management

#### Security Module (`qtforge.security`)

- Security levels and trust management
- Plugin validation
- Signature verification
- Permission management
- Security policy engine

#### Communication Module (`qtforge.communication`)

- Message bus for inter-plugin communication
- Request-response system
- Message and request/response types

#### Managers Module (`qtforge.managers`)

- Plugin manager integration
- Configuration management
- Logging system
- Resource management

#### Monitoring Module (`qtforge.monitoring`)

- Hot reload management
- Metrics collection
- Plugin health monitoring
- Performance tracking

#### Orchestration Module (`qtforge.orchestration`)

- Workflow management
- Step execution
- Orchestration patterns

#### Other Modules

- Transactions (`qtforge.transactions`)
- Composition (`qtforge.composition`)
- Marketplace (`qtforge.marketplace`)

## Usage Examples

### Basic Plugin Structure

```lua
-- Plugin metadata
plugin = {
    name = "MyLuaPlugin",
    version = "1.0.0",
    description = "A sample Lua plugin",
    author = "Developer Name"
}

-- Initialization
function plugin.initialize()
    qtforge.log("Plugin initializing...")
    return true
end

-- Shutdown
function plugin.shutdown()
    qtforge.log("Plugin shutting down...")
end

-- Method example
function plugin.process_data(input)
    local trimmed = qtforge.utils.trim(input)
    local upper = qtforge.utils.to_upper(trimmed)
    return {
        success = true,
        result = upper
    }
end
```

### C++ Integration

```cpp
#include "qtplugin/bridges/lua_plugin_bridge.hpp"

// Create and initialize bridge
qtplugin::LuaPluginBridge bridge;
auto init_result = bridge.initialize();

// Load Lua plugin
QJsonObject params;
params["path"] = "path/to/plugin.lua";
auto load_result = bridge.execute_command("load_script", params);

// Invoke method
QList<QVariant> args;
args << "test input";
auto method_result = bridge.invoke_method("process_data", args);

// Access properties
auto prop_result = bridge.get_property("name");
bridge.set_property("enabled", true);
```

### Using QtForge Utilities in Lua

```lua
-- String utilities
local trimmed = qtforge.utils.trim("  hello world  ")
local parts = qtforge.utils.split("a,b,c", ",")
local joined = qtforge.utils.join(parts, "|")

-- File path utilities
local filename = qtforge.utils.get_filename("/path/to/file.txt")
local dir = qtforge.utils.get_directory("/path/to/file.txt")

-- Time utilities
local timestamp = qtforge.utils.current_timestamp()
local formatted = qtforge.utils.format_timestamp(timestamp)

-- UUID generation
local uuid = qtforge.utils.generate_uuid()

-- Version handling
local parsed = qtforge.utils.parse_version("2.1.3")
local comparison = qtforge.utils.compare_versions("2.1.0", "2.1.3")

-- Threading
local thread_count = qtforge.threading.get_thread_count()
local is_main = qtforge.threading.is_main_thread()
qtforge.threading.sleep(1000) -- Sleep for 1 second
```

## Security Features

The Lua Plugin Bridge includes several security features:

### Sandboxing

When enabled, sandboxing restricts access to potentially dangerous functions:

- File system operations
- Process execution
- External module loading
- Network access (configurable)

### Permission System

Integration with QtForge's permission system allows fine-grained control over plugin capabilities.

### Signature Verification

Plugins can be digitally signed and verified before execution.

## Error Handling

The bridge provides comprehensive error handling:

```lua
-- Lua error handling
function plugin.safe_operation()
    local success, result = pcall(function()
        -- Potentially failing operation
        return risky_function()
    end)

    if success then
        return { success = true, result = result }
    else
        qtforge.log("Error: " .. result)
        return { success = false, error = result }
    end
end
```

## Performance Considerations

- Lua state is reused across plugin calls for efficiency
- Type conversions between C++ and Lua are optimized
- Memory usage is monitored and can be limited
- Hot reload minimizes downtime during development

## Debugging Support

- Comprehensive logging through QtForge's logging system
- Error messages include stack traces when available
- Debug mode provides additional runtime information
- Integration with Qt's debugging tools

## Best Practices

1. **Error Handling**: Always handle potential errors in Lua code
2. **Resource Management**: Clean up resources in shutdown functions
3. **Performance**: Avoid heavy computations in frequently called methods
4. **Security**: Use sandboxing in production environments
5. **Documentation**: Document plugin APIs and expected behavior

## Limitations

- Maximum 5 parameters for method invocation from C++
- Some Qt types require conversion through JSON
- Sandboxing may limit certain operations
- Performance overhead compared to native plugins

## Future Enhancements

- Async/await support for long-running operations
- Enhanced debugging tools
- More comprehensive Qt API bindings
- Performance optimizations
- Extended sandboxing options
