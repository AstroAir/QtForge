# Multi-Language Binding Examples

!!! info "Cross-Language Examples"
**Languages**: Python, Lua  
 **Difficulty**: Beginner to Advanced  
 **QtForge Version**: v3.0.0+

## Overview

This collection demonstrates how to use QtForge from different programming languages, showcasing the consistency and power of the multi-language bindings. Each example shows the same functionality implemented in both Python and Lua.

## Basic Plugin Management

### Python Implementation

```python
import qtforge

def main():
    # Create plugin manager
    manager = qtforge.core.PluginManager()

    # Load plugin
    result = manager.load_plugin("./plugins/example_plugin.so")
    if not result.success:
        print(f"Failed to load plugin: {result.error}")
        return

    # Get plugin instance
    plugin = manager.get_plugin(result.plugin_id)
    if plugin:
        print(f"Loaded: {plugin.name()} v{plugin.version()}")

        # Initialize plugin
        init_result = plugin.initialize()
        if init_result.success:
            print("Plugin initialized successfully")
        else:
            print(f"Initialization failed: {init_result.error}")

if __name__ == "__main__":
    main()
```

### Lua Implementation

```lua
local qtforge = require('qtforge')

local function main()
    -- Create plugin manager
    local manager = qtforge.core.PluginManager()

    -- Load plugin
    local result = manager:load_plugin("./plugins/example_plugin.so")
    if not result.success then
        print("Failed to load plugin: " .. result.error)
        return
    end

    -- Get plugin instance
    local plugin = manager:get_plugin(result.plugin_id)
    if plugin then
        print("Loaded: " .. plugin:name() .. " v" .. plugin:version():to_string())

        -- Initialize plugin
        local init_result = plugin:initialize()
        if init_result.success then
            print("Plugin initialized successfully")
        else
            print("Initialization failed: " .. init_result.error)
        end
    end
end

main()
```

## Inter-Plugin Communication

### Python Message Publisher

```python
import qtforge
import json
import time

def publisher_example():
    # Create message bus
    bus = qtforge.communication.MessageBus()

    # Create and configure message
    message = qtforge.communication.Message()
    message.type = "data.update"
    message.source = "python_publisher"
    message.timestamp = time.time()

    # Send periodic updates
    for i in range(10):
        message.data = {
            "counter": i,
            "timestamp": time.time(),
            "message": f"Update #{i} from Python"
        }

        bus.publish(message)
        print(f"Published message #{i}")
        time.sleep(1)

if __name__ == "__main__":
    publisher_example()
```

### Lua Message Subscriber

```lua
local qtforge = require('qtforge')

local function subscriber_example()
    -- Create message bus
    local bus = qtforge.communication.MessageBus()

    -- Subscribe to messages
    bus:subscribe("data.update", function(message)
        print("Received from " .. message.source .. ":")
        print("  Counter: " .. message.data.counter)
        print("  Message: " .. message.data.message)
        print("  Timestamp: " .. message.data.timestamp)
    end)

    -- Keep listening
    print("Listening for messages... (Press Ctrl+C to exit)")
    while true do
        bus:process_messages()
        -- Small delay to prevent busy waiting
        os.execute("sleep 0.1")
    end
end

subscriber_example()
```

## Configuration Management

### Python Configuration

```python
import qtforge
import json

def config_example():
    # Create configuration manager
    config = qtforge.managers.ConfigurationManager()

    # Load configuration from file
    config.load_from_file("app_config.json")

    # Access configuration values with defaults
    plugin_dir = config.get_string("plugin_directory", "./plugins")
    auto_load = config.get_bool("auto_load_plugins", True)
    log_level = config.get_string("log_level", "info")
    max_plugins = config.get_int("max_plugins", 100)

    print(f"Configuration loaded:")
    print(f"  Plugin directory: {plugin_dir}")
    print(f"  Auto load: {auto_load}")
    print(f"  Log level: {log_level}")
    print(f"  Max plugins: {max_plugins}")

    # Update configuration
    config.set_string("last_run", "2024-09-03T10:30:00Z")
    config.set_int("run_count", config.get_int("run_count", 0) + 1)

    # Save updated configuration
    config.save_to_file("app_config.json")
    print("Configuration updated and saved")

if __name__ == "__main__":
    config_example()
```

### Lua Configuration

```lua
local qtforge = require('qtforge')

local function config_example()
    -- Create configuration manager
    local config = qtforge.managers.ConfigurationManager()

    -- Load configuration from file
    config:load_from_file("app_config.json")

    -- Access configuration values with defaults
    local plugin_dir = config:get_string("plugin_directory", "./plugins")
    local auto_load = config:get_bool("auto_load_plugins", true)
    local log_level = config:get_string("log_level", "info")
    local max_plugins = config:get_int("max_plugins", 100)

    print("Configuration loaded:")
    print("  Plugin directory: " .. plugin_dir)
    print("  Auto load: " .. tostring(auto_load))
    print("  Log level: " .. log_level)
    print("  Max plugins: " .. max_plugins)

    -- Update configuration
    config:set_string("last_run", "2024-09-03T10:30:00Z")
    config:set_int("run_count", config:get_int("run_count", 0) + 1)

    -- Save updated configuration
    config:save_to_file("app_config.json")
    print("Configuration updated and saved")
end

config_example()
```

## Plugin Orchestration

### Python Workflow

```python
import qtforge
import asyncio

async def orchestration_example():
    # Create orchestrator
    orchestrator = qtforge.orchestration.PluginOrchestrator()

    # Create workflow
    workflow = qtforge.orchestration.Workflow("data_processing")

    # Add workflow steps
    workflow.add_step("load_data", "data_loader_plugin", {
        "source": "input.csv",
        "format": "csv"
    })

    workflow.add_step("validate_data", "data_validator_plugin", {
        "rules": ["not_null", "positive_numbers"]
    })

    workflow.add_step("process_data", "data_processor_plugin", {
        "algorithm": "statistical_analysis"
    })

    workflow.add_step("save_results", "data_saver_plugin", {
        "destination": "output.json",
        "format": "json"
    })

    # Set up dependencies
    workflow.add_dependency("validate_data", "load_data")
    workflow.add_dependency("process_data", "validate_data")
    workflow.add_dependency("save_results", "process_data")

    # Execute workflow
    print("Starting workflow execution...")
    result = await orchestrator.execute_workflow_async(workflow)

    if result.success:
        print("Workflow completed successfully")
        print(f"Execution time: {result.execution_time}ms")
    else:
        print(f"Workflow failed: {result.error}")

if __name__ == "__main__":
    asyncio.run(orchestration_example())
```

### Lua Workflow

```lua
local qtforge = require('qtforge')

local function orchestration_example()
    -- Create orchestrator
    local orchestrator = qtforge.orchestration.PluginOrchestrator()

    -- Create workflow
    local workflow = qtforge.orchestration.Workflow("data_processing")

    -- Add workflow steps
    workflow:add_step("load_data", "data_loader_plugin", {
        source = "input.csv",
        format = "csv"
    })

    workflow:add_step("validate_data", "data_validator_plugin", {
        rules = {"not_null", "positive_numbers"}
    })

    workflow:add_step("process_data", "data_processor_plugin", {
        algorithm = "statistical_analysis"
    })

    workflow:add_step("save_results", "data_saver_plugin", {
        destination = "output.json",
        format = "json"
    })

    -- Set up dependencies
    workflow:add_dependency("validate_data", "load_data")
    workflow:add_dependency("process_data", "validate_data")
    workflow:add_dependency("save_results", "process_data")

    -- Execute workflow
    print("Starting workflow execution...")
    local result = orchestrator:execute_workflow(workflow)

    if result.success then
        print("Workflow completed successfully")
        print("Execution time: " .. result.execution_time .. "ms")
    else
        print("Workflow failed: " .. result.error)
    end
end

orchestration_example()
```

## Security and Validation

### Python Security Example

```python
import qtforge

def sha256_verification_example():
    # SHA256 verification example
    manager = qtforge.create_plugin_manager()

    # Calculate SHA256 hash for verification
    plugin_path = "./plugins/my_plugin.so"
    calculated_hash = manager.calculate_file_sha256(plugin_path)
    print(f"Plugin SHA256: {calculated_hash}")

    # Load plugin with SHA256 verification
    options = qtforge.PluginLoadOptions()
    options.validate_sha256 = True
    options.expected_sha256 = calculated_hash

    result = manager.load_plugin(plugin_path, options)

    if validation_result.is_valid:
        print(f"Plugin validation passed:")
        print(f"  Trust level: {validation_result.trust_level}")
        print(f"  Permissions: {validation_result.permissions}")
        print(f"  Signature valid: {validation_result.signature_valid}")

        # Safe to load plugin
        manager = qtforge.core.PluginManager()
        manager.set_security_manager(security)
        result = manager.load_plugin(plugin_path)

        if result.success:
            print("Plugin loaded securely")
        else:
            print(f"Failed to load plugin: {result.error}")
    else:
        print(f"Plugin validation failed: {validation_result.error}")

if __name__ == "__main__":
    security_example()
```

### Lua Security Example

```lua
local qtforge = require('qtforge')

local function security_example()
    -- Create security manager
    local security = qtforge.security.SecurityManager()

    -- Configure security policy
    local policy = qtforge.security.SecurityPolicy()
    policy:set_trust_level(qtforge.security.TrustLevel.MEDIUM)
    policy:enable_signature_validation(true)
    policy:enable_permission_checking(true)

    security:set_policy(policy)

    -- Create plugin validator
    local validator = qtforge.security.PluginValidator(security)

    -- Validate plugin before loading
    local plugin_path = "./plugins/untrusted_plugin.so"
    local validation_result = validator:validate_plugin(plugin_path)

    if validation_result.is_valid then
        print("Plugin validation passed:")
        print("  Trust level: " .. tostring(validation_result.trust_level))
        print("  Permissions: " .. table.concat(validation_result.permissions, ", "))
        print("  Signature valid: " .. tostring(validation_result.signature_valid))

        -- Safe to load plugin
        local manager = qtforge.core.PluginManager()
        manager:set_security_manager(security)
        local result = manager:load_plugin(plugin_path)

        if result.success then
            print("Plugin loaded securely")
        else
            print("Failed to load plugin: " .. result.error)
        end
    else
        print("Plugin validation failed: " .. validation_result.error)
    end
end

security_example()
```

## Performance Monitoring

### Python Metrics Collection

```python
import qtforge
import time

def monitoring_example():
    # Create metrics collector
    metrics = qtforge.monitoring.PluginMetricsCollector()

    # Create plugin manager with metrics
    manager = qtforge.core.PluginManager()
    manager.set_metrics_collector(metrics)

    # Load some plugins
    plugins = [
        "./plugins/plugin1.so",
        "./plugins/plugin2.so",
        "./plugins/plugin3.so"
    ]

    for plugin_path in plugins:
        result = manager.load_plugin(plugin_path)
        if result.success:
            print(f"Loaded: {plugin_path}")

    # Collect metrics after some operations
    time.sleep(2)  # Let plugins run for a bit

    # Get system metrics
    system_metrics = metrics.get_system_metrics()
    print(f"\nSystem Metrics:")
    print(f"  CPU Usage: {system_metrics.cpu_usage}%")
    print(f"  Memory Usage: {system_metrics.memory_usage} MB")
    print(f"  Plugin Count: {system_metrics.plugin_count}")

    # Get plugin-specific metrics
    for plugin_id in manager.get_loaded_plugins():
        plugin_metrics = metrics.get_plugin_metrics(plugin_id)
        print(f"\nPlugin {plugin_id}:")
        print(f"  CPU Time: {plugin_metrics.cpu_time}ms")
        print(f"  Memory: {plugin_metrics.memory_usage} MB")
        print(f"  Messages: {plugin_metrics.message_count}")

if __name__ == "__main__":
    monitoring_example()
```

### Lua Metrics Collection

```lua
local qtforge = require('qtforge')

local function monitoring_example()
    -- Create metrics collector
    local metrics = qtforge.monitoring.PluginMetricsCollector()

    -- Create plugin manager with metrics
    local manager = qtforge.core.PluginManager()
    manager:set_metrics_collector(metrics)

    -- Load some plugins
    local plugins = {
        "./plugins/plugin1.so",
        "./plugins/plugin2.so",
        "./plugins/plugin3.so"
    }

    for _, plugin_path in ipairs(plugins) do
        local result = manager:load_plugin(plugin_path)
        if result.success then
            print("Loaded: " .. plugin_path)
        end
    end

    -- Collect metrics after some operations
    os.execute("sleep 2")  -- Let plugins run for a bit

    -- Get system metrics
    local system_metrics = metrics:get_system_metrics()
    print("\nSystem Metrics:")
    print("  CPU Usage: " .. system_metrics.cpu_usage .. "%")
    print("  Memory Usage: " .. system_metrics.memory_usage .. " MB")
    print("  Plugin Count: " .. system_metrics.plugin_count)

    -- Get plugin-specific metrics
    local loaded_plugins = manager:get_loaded_plugins()
    for _, plugin_id in ipairs(loaded_plugins) do
        local plugin_metrics = metrics:get_plugin_metrics(plugin_id)
        print("\nPlugin " .. plugin_id .. ":")
        print("  CPU Time: " .. plugin_metrics.cpu_time .. "ms")
        print("  Memory: " .. plugin_metrics.memory_usage .. " MB")
        print("  Messages: " .. plugin_metrics.message_count)
    end
end

monitoring_example()
```

## Cross-Language Integration

### Hybrid Application Example

This example shows how to use both Python and Lua in the same application:

```python
# main.py - Python main application
import qtforge
import subprocess
import sys

def main():
    # Initialize QtForge in Python
    manager = qtforge.core.PluginManager()
    bus = qtforge.communication.MessageBus()

    # Load Python plugins
    python_plugins = [
        "./plugins/data_processor.so",
        "./plugins/web_interface.so"
    ]

    for plugin in python_plugins:
        result = manager.load_plugin(plugin)
        if result.success:
            print(f"Loaded Python plugin: {plugin}")

    # Start Lua script for additional processing
    lua_script = "lua_worker.lua"
    lua_process = subprocess.Popen([
        "lua", lua_script
    ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # Set up message handling
    bus.subscribe("lua.result", handle_lua_result)
    bus.subscribe("python.request", handle_python_request)

    # Send initial message to Lua
    message = qtforge.communication.Message()
    message.type = "python.init"
    message.data = {"status": "ready", "plugins": len(python_plugins)}
    bus.publish(message)

    print("Hybrid application running...")

    # Main event loop
    try:
        while True:
            bus.process_messages()
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("Shutting down...")
        lua_process.terminate()

def handle_lua_result(message):
    print(f"Received from Lua: {message.data}")

def handle_python_request(message):
    # Process request from Lua
    result = {"processed": True, "timestamp": time.time()}

    response = qtforge.communication.Message()
    response.type = "python.response"
    response.data = result

    bus = qtforge.communication.MessageBus()
    bus.publish(response)

if __name__ == "__main__":
    main()
```

```lua
-- lua_worker.lua - Lua worker script
local qtforge = require('qtforge')

local function main()
    -- Initialize QtForge in Lua
    local manager = qtforge.core.PluginManager()
    local bus = qtforge.communication.MessageBus()

    -- Load Lua-specific plugins
    local lua_plugins = {
        "./plugins/script_engine.so",
        "./plugins/config_processor.so"
    }

    for _, plugin in ipairs(lua_plugins) do
        local result = manager:load_plugin(plugin)
        if result.success then
            print("Loaded Lua plugin: " .. plugin)
        end
    end

    -- Set up message handling
    bus:subscribe("python.init", function(message)
        print("Received init from Python: " .. message.data.status)

        -- Send response back to Python
        local response = qtforge.communication.Message()
        response.type = "lua.result"
        response.data = {
            status = "initialized",
            plugins = #lua_plugins,
            timestamp = os.time()
        }
        bus:publish(response)
    end)

    -- Main processing loop
    while true do
        bus:process_messages()

        -- Perform Lua-specific processing
        -- ...

        -- Small delay
        os.execute("sleep 0.1")
    end
end

main()
```

## Best Practices

### Error Handling

```python
# Python - Use try/except
try:
    result = manager.load_plugin("plugin.so")
    if not result.success:
        raise qtforge.PluginError(result.error)
except qtforge.PluginError as e:
    print(f"Plugin error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")
```

```lua
-- Lua - Use pcall and return values
local success, result = pcall(function()
    return manager:load_plugin("plugin.so")
end)

if success and result.success then
    print("Plugin loaded successfully")
elseif success then
    print("Plugin load failed: " .. result.error)
else
    print("Unexpected error: " .. result)
end
```

### Resource Management

```python
# Python - Use context managers
with qtforge.core.PluginManager() as manager:
    # Plugin operations
    pass  # Automatic cleanup
```

```lua
-- Lua - Manual cleanup
local manager = qtforge.core.PluginManager()
-- Use manager...
manager:cleanup()  -- Explicit cleanup
```

---

_Last updated: September 2024 | QtForge v3.0.0_
