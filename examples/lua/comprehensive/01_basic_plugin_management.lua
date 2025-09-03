#!/usr/bin/env lua
--[[
QtForge Lua Bindings Example: Basic Plugin Management

This example demonstrates the fundamental plugin management operations using
the QtForge Lua bindings. It covers:

1. Creating and configuring a PluginManager
2. Loading and unloading plugins
3. Querying plugin information
4. Managing plugin lifecycle
5. Error handling and best practices

Prerequisites:
- QtForge built with Lua bindings enabled
- Lua 5.1 or later (compatible with LuaJIT)
- sol2 library for C++ binding
]]

-- Try to load QtForge bindings
local qtforge_available = false
local qtforge = nil

local success, result = pcall(require, "qtforge")
if success then
    qtforge = result
    qtforge_available = true
    print("✅ QtForge Lua bindings loaded successfully")
    if qtforge.version then
        print(string.format("QtForge version: %s", qtforge.version))
    end
else
    print(string.format("❌ Failed to load QtForge: %s", result))
    print("Make sure QtForge is built with Lua bindings enabled")
    os.exit(1)
end

-- PluginManagerExample class
local PluginManagerExample = {}
PluginManagerExample.__index = PluginManagerExample

function PluginManagerExample:new()
    local obj = {
        manager = nil,
        registry = nil,
        loader = nil
    }
    setmetatable(obj, PluginManagerExample)
    obj:setup_components()
    return obj
end

function PluginManagerExample:setup_components()
    print("\n🔧 Setting up plugin management components...")
    
    -- Create PluginManager
    if qtforge.core and qtforge.core.create_plugin_manager then
        self.manager = qtforge.core.create_plugin_manager()
        print("✅ PluginManager created via factory function")
    elseif qtforge.core and qtforge.core.PluginManager then
        self.manager = qtforge.core.PluginManager()
        print("✅ PluginManager created")
    else
        error("PluginManager not available")
    end
    
    -- Create PluginRegistry
    if qtforge.core and qtforge.core.create_plugin_registry then
        self.registry = qtforge.core.create_plugin_registry()
        print("✅ PluginRegistry created via factory function")
    elseif qtforge.core and qtforge.core.PluginRegistry then
        self.registry = qtforge.core.PluginRegistry()
        print("✅ PluginRegistry created")
    end
    
    -- Create PluginLoader
    if qtforge.core and qtforge.core.create_plugin_loader then
        self.loader = qtforge.core.create_plugin_loader()
        print("✅ PluginLoader created via factory function")
    elseif qtforge.core and qtforge.core.PluginLoader then
        self.loader = qtforge.core.PluginLoader()
        print("✅ PluginLoader created")
    end
end

function PluginManagerExample:demonstrate_basic_operations()
    print("\n📋 Demonstrating basic plugin operations...")
    
    -- Check initial state
    if self.manager and self.manager.get_plugin_count then
        local initial_count = self.manager:get_plugin_count()
        print(string.format("Initial plugin count: %d", initial_count))
    end
    
    -- Get all plugins (should be empty initially)
    if self.manager and self.manager.get_all_plugins then
        local plugins = self.manager:get_all_plugins()
        print(string.format("Initial plugins list length: %d", #plugins))
    end
    
    -- Test plugin existence check
    if self.manager and self.manager.has_plugin then
        local has_test_plugin = self.manager:has_plugin("test_plugin")
        print(string.format("Has 'test_plugin': %s", tostring(has_test_plugin)))
    end
    
    -- Test getting non-existent plugin
    if self.manager and self.manager.get_plugin then
        local plugin = self.manager:get_plugin("non_existent_plugin")
        print(string.format("Get non-existent plugin result: %s", tostring(plugin)))
    end
end

function PluginManagerExample:demonstrate_plugin_loading()
    print("\n🔌 Demonstrating plugin loading...")
    
    -- Create some example plugin paths (these won't exist, but demonstrate the API)
    local example_plugins = {
        "/path/to/example_plugin.so",
        "/path/to/another_plugin.dll",
        "relative/path/plugin.dylib"
    }
    
    for _, plugin_path in ipairs(example_plugins) do
        print(string.format("\nAttempting to load plugin: %s", plugin_path))
        
        local success, result = pcall(function()
            if self.manager and self.manager.load_plugin then
                return self.manager:load_plugin(plugin_path)
            elseif self.loader and self.loader.load_plugin then
                return self.loader:load_plugin(plugin_path)
            else
                return "Plugin loading not available"
            end
        end)
        
        if success then
            print(string.format("Load result: %s", tostring(result)))
        else
            print(string.format("Expected error (file doesn't exist): %s", result))
        end
    end
end

function PluginManagerExample:demonstrate_plugin_queries()
    print("\n🔍 Demonstrating plugin queries...")
    
    -- Query plugins by capability
    if self.manager and self.manager.get_plugins_by_capability then
        local capabilities = {}
        
        -- Collect available capabilities
        if qtforge.core and qtforge.core.PluginCapability then
            local cap_names = {"Service", "UI", "Network", "DataProcessor"}
            for _, name in ipairs(cap_names) do
                if qtforge.core.PluginCapability[name] then
                    table.insert(capabilities, qtforge.core.PluginCapability[name])
                end
            end
        end
        
        for _, capability in ipairs(capabilities) do
            local success, plugins = pcall(function()
                return self.manager:get_plugins_by_capability(capability)
            end)
            
            if success then
                print(string.format("Plugins with capability %s: %d", tostring(capability), #plugins))
            else
                print(string.format("Plugin capability query error: %s", plugins))
            end
        end
    end
    
    -- Test registry operations if available
    if self.registry then
        print("\n📚 Testing registry operations...")
        
        if self.registry.size then
            local size = self.registry:size()
            print(string.format("Registry size: %d", size))
        end
        
        if self.registry.get_all_plugins then
            local all_plugins = self.registry:get_all_plugins()
            print(string.format("Registry plugins: %d", #all_plugins))
        end
    end
end

function PluginManagerExample:demonstrate_plugin_states()
    print("\n🔄 Demonstrating plugin states...")
    
    -- Show available plugin states
    if qtforge.core and qtforge.core.PluginState then
        print("Available plugin states:")
        local state_names = {"Unloaded", "Loaded", "Initialized", "Running", "Stopped", "Error"}
        for _, name in ipairs(state_names) do
            if qtforge.core.PluginState[name] then
                local state = qtforge.core.PluginState[name]
                print(string.format("  - %s: %s", name, tostring(state)))
            end
        end
    end
    
    -- Show available plugin capabilities
    if qtforge.core and qtforge.core.PluginCapability then
        print("\nAvailable plugin capabilities:")
        local cap_names = {"None", "Service", "UI", "Network", "DataProcessor", "Scripting"}
        for _, name in ipairs(cap_names) do
            if qtforge.core.PluginCapability[name] then
                local capability = qtforge.core.PluginCapability[name]
                print(string.format("  - %s: %s", name, tostring(capability)))
            end
        end
    end
    
    -- Show available plugin priorities
    if qtforge.core and qtforge.core.PluginPriority then
        print("\nAvailable plugin priorities:")
        local priority_names = {"Lowest", "Low", "Normal", "High", "Highest"}
        for _, name in ipairs(priority_names) do
            if qtforge.core.PluginPriority[name] then
                local priority = qtforge.core.PluginPriority[name]
                print(string.format("  - %s: %s", name, tostring(priority)))
            end
        end
    end
end

function PluginManagerExample:demonstrate_dependency_management()
    print("\n🔗 Demonstrating dependency management...")
    
    -- Create dependency resolver if available
    local resolver = nil
    local success, result = pcall(function()
        if qtforge.core and qtforge.core.create_plugin_dependency_resolver then
            return qtforge.core.create_plugin_dependency_resolver()
        elseif qtforge.core and qtforge.core.PluginDependencyResolver then
            return qtforge.core.PluginDependencyResolver()
        else
            error("PluginDependencyResolver not available")
        end
    end)
    
    if success then
        resolver = result
        print("✅ PluginDependencyResolver created")
    else
        print(string.format("⚠️  Dependency resolver not available: %s", result))
        return
    end
    
    if resolver then
        -- Test dependency operations
        local operations = {
            {"get_load_order", "Current load order"},
            {"has_circular_dependencies", "Has circular dependencies"},
            {"get_dependency_graph", "Dependency graph"}
        }
        
        for _, op in ipairs(operations) do
            local method_name, description = op[1], op[2]
            if resolver[method_name] then
                local success, result = pcall(function()
                    return resolver[method_name](resolver)
                end)
                
                if success then
                    print(string.format("%s: %s", description, tostring(result)))
                else
                    print(string.format("%s error: %s", description, result))
                end
            end
        end
    end
end

function PluginManagerExample:demonstrate_error_handling()
    print("\n⚠️  Demonstrating error handling...")
    
    -- Test various error conditions
    local error_tests = {
        {
            name = "Loading non-existent plugin",
            func = function()
                if self.manager and self.manager.load_plugin then
                    return self.manager:load_plugin("/non/existent/path.so")
                end
                return nil
            end
        },
        {
            name = "Getting non-existent plugin",
            func = function()
                if self.manager and self.manager.get_plugin then
                    return self.manager:get_plugin("non_existent")
                end
                return nil
            end
        },
        {
            name = "Unloading non-existent plugin",
            func = function()
                if self.manager and self.manager.unload_plugin then
                    return self.manager:unload_plugin("non_existent")
                end
                return nil
            end
        }
    }
    
    for _, test in ipairs(error_tests) do
        print(string.format("\nTesting: %s", test.name))
        local success, result = pcall(test.func)
        
        if success then
            print(string.format("Result: %s (type: %s)", tostring(result), type(result)))
        else
            print(string.format("Exception caught: %s", result))
        end
    end
end

function PluginManagerExample:cleanup()
    print("\n🧹 Cleaning up resources...")
    
    -- Clear manager if available
    if self.manager and self.manager.clear then
        local success, result = pcall(function()
            self.manager:clear()
        end)
        
        if success then
            print("✅ PluginManager cleared")
        else
            print(string.format("Manager cleanup error: %s", result))
        end
    end
    
    -- Clear registry if available
    if self.registry and self.registry.clear then
        local success, result = pcall(function()
            self.registry:clear()
        end)
        
        if success then
            print("✅ PluginRegistry cleared")
        else
            print(string.format("Registry cleanup error: %s", result))
        end
    end
end

function PluginManagerExample:run_complete_example()
    print("🚀 QtForge Lua Bindings - Basic Plugin Management Example")
    print(string.rep("=", 70))
    
    local success, error_msg = pcall(function()
        self:demonstrate_basic_operations()
        self:demonstrate_plugin_loading()
        self:demonstrate_plugin_queries()
        self:demonstrate_plugin_states()
        self:demonstrate_dependency_management()
        self:demonstrate_error_handling()
    end)
    
    -- Always cleanup
    self:cleanup()
    
    if success then
        print("\n✅ Basic plugin management example completed successfully!")
        return 0
    else
        print(string.format("\n❌ Example failed with error: %s", error_msg))
        return 1
    end
end

-- Main function
local function main()
    local success, result = pcall(function()
        local example = PluginManagerExample:new()
        return example:run_complete_example()
    end)
    
    if success then
        return result
    else
        print(string.format("❌ Failed to run example: %s", result))
        return 1
    end
end

-- Run the example
local exit_code = main()
os.exit(exit_code)
