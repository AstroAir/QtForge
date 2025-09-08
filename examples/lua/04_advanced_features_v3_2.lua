#!/usr/bin/env lua
--[[
QtForge v3.2.0 - Advanced Features Lua Example

This example demonstrates the new advanced features introduced in QtForge v3.2.0:
- Enhanced Lua Plugin Bridge with full QtForge integration
- Advanced plugin interfaces and dynamic capabilities
- Service contract system and communication
- Enhanced security features and sandboxing
- Configuration management with scoped access
--]]

-- Load QtForge Lua bindings
local qtforge = require('qtforge')

-- Helper function to print section headers
local function print_section(title)
    print("\n" .. string.rep("=", 60))
    print(title)
    print(string.rep("=", 60))
end

-- Helper function to print subsection headers
local function print_subsection(title)
    print("\n" .. string.rep("-", 40))
    print(title)
    print(string.rep("-", 40))
end

-- Demonstrate basic QtForge Lua integration
local function demonstrate_basic_integration()
    print_section("QtForge Lua Integration - Basic Features")
    
    -- Test connection and version
    local connection_test = qtforge.test_connection()
    print("Connection test: " .. tostring(connection_test))
    
    local version = qtforge.get_version()
    print("QtForge version: " .. version)
    
    -- List available modules
    print("\nAvailable QtForge modules:")
    local modules = qtforge.list_modules()
    for i, module in ipairs(modules) do
        print("  " .. i .. ". " .. module)
    end
end

-- Demonstrate core module functionality
local function demonstrate_core_module()
    print_section("Core Module - Enhanced Features")
    
    -- Create plugin manager
    local manager = qtforge.core.create_plugin_manager()
    print("Created plugin manager: " .. tostring(manager))
    
    -- Demonstrate plugin types
    print("\nSupported plugin types:")
    local plugin_types = {
        qtforge.core.PluginType.Native,
        qtforge.core.PluginType.Python,
        qtforge.core.PluginType.Lua,
        qtforge.core.PluginType.JavaScript,
        qtforge.core.PluginType.Remote,
        qtforge.core.PluginType.Composite
    }
    
    for i, ptype in ipairs(plugin_types) do
        print("  - " .. tostring(ptype))
    end
    
    -- Demonstrate plugin states
    print("\nPlugin states:")
    local states = {
        qtforge.core.PluginState.Unloaded,
        qtforge.core.PluginState.Loading,
        qtforge.core.PluginState.Loaded,
        qtforge.core.PluginState.Initializing,
        qtforge.core.PluginState.Running,
        qtforge.core.PluginState.Paused,
        qtforge.core.PluginState.Stopping,
        qtforge.core.PluginState.Stopped,
        qtforge.core.PluginState.Error,
        qtforge.core.PluginState.Reloading
    }
    
    for i, state in ipairs(states) do
        print("  - " .. tostring(state))
    end
end

-- Demonstrate security features
local function demonstrate_security_features()
    print_section("Enhanced Security Features")
    
    -- Demonstrate plugin permissions
    print("Available plugin permissions:")
    local permissions = {
        qtforge.security.PluginPermission.FileSystemRead,
        qtforge.security.PluginPermission.FileSystemWrite,
        qtforge.security.PluginPermission.NetworkAccess,
        qtforge.security.PluginPermission.SystemInfo,
        qtforge.security.PluginPermission.ProcessControl,
        qtforge.security.PluginPermission.RegistryAccess
    }
    
    for i, perm in ipairs(permissions) do
        print("  - " .. tostring(perm))
    end
    
    -- Demonstrate trust levels
    print("\nAvailable trust levels:")
    local trust_levels = {
        qtforge.security.TrustLevel.Untrusted,
        qtforge.security.TrustLevel.Low,
        qtforge.security.TrustLevel.Medium,
        qtforge.security.TrustLevel.High,
        qtforge.security.TrustLevel.Trusted
    }
    
    for i, level in ipairs(trust_levels) do
        print("  - " .. tostring(level))
    end
    
    -- Create a security policy
    local policy = qtforge.security.create_security_policy()
    policy.name = "LuaPluginPolicy"
    policy.description = "Security policy for Lua plugins"
    policy.minimum_trust_level = qtforge.security.TrustLevel.Medium
    
    print("\nCreated security policy: " .. policy.name)
    print("Minimum trust level: " .. tostring(policy.minimum_trust_level))
end

-- Demonstrate communication features
local function demonstrate_communication()
    print_section("Communication and Service Contracts")
    
    -- Create message bus
    local message_bus = qtforge.communication.create_message_bus()
    print("Created message bus: " .. tostring(message_bus))
    
    -- Demonstrate service capabilities
    print("\nService capabilities:")
    local capabilities = {
        qtforge.communication.ServiceCapability.DataTransformation,
        qtforge.communication.ServiceCapability.AsyncProcessing,
        qtforge.communication.ServiceCapability.BatchProcessing,
        qtforge.communication.ServiceCapability.RealTimeProcessing,
        qtforge.communication.ServiceCapability.EventHandling
    }
    
    for i, cap in ipairs(capabilities) do
        print("  - " .. tostring(cap))
    end
    
    -- Create a service contract
    local contract = qtforge.communication.create_service_contract()
    contract.name = "LuaDataService"
    contract.version = qtforge.communication.create_service_version(1, 2, 0)
    contract.description = "Lua-based data processing service"
    
    print("\nCreated service contract: " .. contract.name)
    print("Service version: " .. tostring(contract.version))
end

-- Demonstrate configuration management
local function demonstrate_configuration()
    print_section("Enhanced Configuration Management")
    
    -- Create configuration manager
    local config = qtforge.managers.create_configuration_manager()
    print("Created configuration manager: " .. tostring(config))
    
    -- Demonstrate configuration scopes
    print("\nConfiguration scopes:")
    local scopes = {
        qtforge.managers.ConfigurationScope.Global,
        qtforge.managers.ConfigurationScope.User,
        qtforge.managers.ConfigurationScope.Plugin,
        qtforge.managers.ConfigurationScope.Session
    }
    
    for i, scope in ipairs(scopes) do
        print("  - " .. tostring(scope))
    end
    
    -- Set and get configuration values
    config:set_value("lua.plugin.name", "ExampleLuaPlugin", qtforge.managers.ConfigurationScope.Plugin)
    config:set_value("lua.plugin.version", "1.0.0", qtforge.managers.ConfigurationScope.Plugin)
    config:set_value("user.preference", "enabled", qtforge.managers.ConfigurationScope.User)
    
    print("\nConfiguration values:")
    print("  Plugin name: " .. config:get_value("lua.plugin.name", qtforge.managers.ConfigurationScope.Plugin))
    print("  Plugin version: " .. config:get_value("lua.plugin.version", qtforge.managers.ConfigurationScope.Plugin))
    print("  User preference: " .. config:get_value("user.preference", qtforge.managers.ConfigurationScope.User))
end

-- Demonstrate utility functions
local function demonstrate_utilities()
    print_section("Utility Functions")
    
    -- String utilities
    local test_string = "  Hello, QtForge!  "
    print("Original string: '" .. test_string .. "'")
    print("Trimmed: '" .. qtforge.utils.trim(test_string) .. "'")
    print("Uppercase: '" .. qtforge.utils.to_upper(test_string) .. "'")
    print("Lowercase: '" .. qtforge.utils.to_lower(test_string) .. "'")
    
    -- Logging utilities
    qtforge.utils.log_info("This is an info message from Lua")
    qtforge.utils.log_warning("This is a warning message from Lua")
    qtforge.utils.log_debug("This is a debug message from Lua")
    
    -- Path utilities
    local test_path = "/path/to/plugin.lua"
    print("\nPath utilities:")
    print("Test path: " .. test_path)
    print("Directory: " .. qtforge.utils.get_directory(test_path))
    print("Filename: " .. qtforge.utils.get_filename(test_path))
    print("Extension: " .. qtforge.utils.get_extension(test_path))
end

-- Demonstrate monitoring features
local function demonstrate_monitoring()
    print_section("Monitoring and Hot Reload")
    
    -- Create hot reload manager
    local hot_reload = qtforge.monitoring.create_hot_reload_manager()
    print("Created hot reload manager: " .. tostring(hot_reload))
    
    -- Demonstrate metrics collection
    local metrics = qtforge.monitoring.create_metrics_collector()
    print("Created metrics collector: " .. tostring(metrics))
    
    -- Add some sample metrics
    metrics:record_counter("lua.plugin.calls", 1)
    metrics:record_gauge("lua.plugin.memory_usage", 1024 * 1024)  -- 1MB
    metrics:record_timer("lua.plugin.execution_time", 0.05)  -- 50ms
    
    print("\nRecorded sample metrics:")
    print("  Plugin calls: 1")
    print("  Memory usage: 1MB")
    print("  Execution time: 50ms")
end

-- Main function
local function main()
    print("QtForge v3.2.0 - Advanced Features Lua Example")
    print("This example demonstrates the enhanced Lua bindings in QtForge v3.2.0")
    
    -- Run all demonstrations
    demonstrate_basic_integration()
    demonstrate_core_module()
    demonstrate_security_features()
    demonstrate_communication()
    demonstrate_configuration()
    demonstrate_utilities()
    demonstrate_monitoring()
    
    print_section("Example Completed Successfully!")
    print("All QtForge v3.2.0 Lua binding features demonstrated.")
end

-- Run the example
main()
