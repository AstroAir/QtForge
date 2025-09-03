#!/usr/bin/env lua
--[[
QtForge Lua Bindings Example 1: Basic Plugin Management

This example demonstrates the fundamental plugin management operations
using the QtForge Lua bindings, including:
- Creating a plugin manager
- Loading and unloading plugins
- Querying plugin information
- Handling plugin states and lifecycle
--]]

-- Helper functions for output formatting
local function print_header(title)
    print("\n" .. string.rep("=", 50))
    print("🔧 " .. title)
    print(string.rep("=", 50))
end

local function print_success(message)
    print("✅ " .. message)
end

local function print_warning(message)
    print("⚠️  " .. message)
end

local function print_error(message)
    print("❌ " .. message)
end

local function print_info(message)
    print("📋 " .. message)
end

local function print_status(message)
    print("📊 " .. message)
end

-- Check if QtForge bindings are available
local function check_qtforge_availability()
    if not qtforge then
        print_error("QtForge Lua bindings not available")
        print("Make sure QtForge is built with Lua bindings enabled:")
        print("  cmake -DQTFORGE_BUILD_LUA_BINDINGS=ON ..")
        print("  make")
        return false
    end
    
    print_success("QtForge Lua bindings loaded successfully")
    
    -- Check for version information if available
    if qtforge.version then
        print("📦 QtForge version: " .. tostring(qtforge.version))
    end
    
    return true
end

-- Demonstrate plugin manager creation and configuration
local function demonstrate_plugin_manager_creation()
    print_header("Creating Plugin Manager")
    
    if not qtforge.core or not qtforge.core.create_plugin_manager then
        print_error("Plugin manager creation not available")
        return nil
    end
    
    local success, manager = pcall(qtforge.core.create_plugin_manager)
    
    if not success then
        print_error("Failed to create plugin manager: " .. tostring(manager))
        return nil
    end
    
    print_success("Plugin manager created successfully")
    
    -- Check available methods
    local methods = {"load_plugin", "unload_plugin", "get_loaded_plugins", "is_plugin_loaded"}
    local available_methods = {}
    
    for _, method in ipairs(methods) do
        if manager[method] then
            table.insert(available_methods, method)
        end
    end
    
    print_info("Available methods: " .. table.concat(available_methods, ", "))
    
    return manager
end

-- Demonstrate plugin loading operations
local function demonstrate_plugin_loading(manager)
    print_header("Plugin Loading Operations")
    
    if not manager then
        print_error("No plugin manager available")
        return
    end
    
    -- Example plugin paths (these may not exist in a real scenario)
    local example_plugins = {
        "libexample_plugin.so",
        "/path/to/plugin.dll",
        "example_plugin.dylib"
    }
    
    for _, plugin_path in ipairs(example_plugins) do
        print("\n🔍 Attempting to load: " .. plugin_path)
        
        local success, result = pcall(function()
            return manager:load_plugin(plugin_path)
        end)
        
        if success then
            if result then
                print_success("Successfully loaded: " .. plugin_path)
                
                -- Check if plugin is loaded
                if manager.is_plugin_loaded then
                    local is_loaded_success, is_loaded = pcall(function()
                        return manager:is_plugin_loaded(plugin_path)
                    end)
                    
                    if is_loaded_success then
                        print_status("Plugin loaded status: " .. tostring(is_loaded))
                    end
                end
            else
                print_warning("Plugin loading returned false: " .. plugin_path)
            end
        else
            if string.find(tostring(result), "not found") or string.find(tostring(result), "No such file") then
                print("📁 Plugin file not found: " .. plugin_path)
            else
                print_error("Failed to load plugin: " .. tostring(result))
            end
        end
    end
end

-- Demonstrate enumerating loaded plugins
local function demonstrate_plugin_enumeration(manager)
    print_header("Plugin Enumeration")
    
    if not manager then
        print_error("No plugin manager available")
        return
    end
    
    if not manager.get_loaded_plugins then
        print_warning("Plugin enumeration not available")
        return
    end
    
    local success, loaded_plugins = pcall(function()
        return manager:get_loaded_plugins()
    end)
    
    if not success then
        print_error("Failed to enumerate plugins: " .. tostring(loaded_plugins))
        return {}
    end
    
    local plugin_count = 0
    if loaded_plugins then
        if type(loaded_plugins) == "table" then
            plugin_count = #loaded_plugins
        elseif type(loaded_plugins) == "number" then
            plugin_count = loaded_plugins
        end
    end
    
    print_status("Number of loaded plugins: " .. plugin_count)
    
    if loaded_plugins and type(loaded_plugins) == "table" and #loaded_plugins > 0 then
        print_info("Loaded plugins:")
        for i, plugin in ipairs(loaded_plugins) do
            print("  " .. i .. ". " .. tostring(plugin))
        end
    else
        print("📭 No plugins currently loaded")
    end
    
    return loaded_plugins or {}
end

-- Demonstrate plugin registry operations
local function demonstrate_plugin_registry()
    print_header("Plugin Registry Operations")
    
    if not qtforge.core or not qtforge.core.create_plugin_registry then
        print_warning("Plugin registry not available")
        return nil
    end
    
    local success, registry = pcall(qtforge.core.create_plugin_registry)
    
    if not success then
        print_error("Failed to create plugin registry: " .. tostring(registry))
        return nil
    end
    
    print_success("Plugin registry created successfully")
    
    -- Demonstrate registry operations
    if registry.get_all_plugins then
        local all_success, all_plugins = pcall(function()
            return registry:get_all_plugins()
        end)
        
        if all_success then
            local count = 0
            if all_plugins then
                if type(all_plugins) == "table" then
                    count = #all_plugins
                elseif type(all_plugins) == "number" then
                    count = all_plugins
                end
            end
            print_status("Total registered plugins: " .. count)
        end
    end
    
    -- Demonstrate plugin search
    if registry.find_plugin then
        print("\n🔍 Searching for plugins...")
        
        local search_terms = {"example", "test", "demo"}
        for _, term in ipairs(search_terms) do
            local search_success, results = pcall(function()
                return registry:find_plugin(term)
            end)
            
            if search_success then
                local result_count = 0
                if results then
                    if type(results) == "table" then
                        result_count = #results
                    elseif type(results) == "number" then
                        result_count = results
                    end
                end
                
                if result_count > 0 then
                    print("  Found " .. result_count .. " plugins matching '" .. term .. "'")
                else
                    print("  No plugins found matching '" .. term .. "'")
                end
            else
                print("  Search for '" .. term .. "' failed: " .. tostring(results))
            end
        end
    end
    
    return registry
end

-- Demonstrate plugin lifecycle management
local function demonstrate_plugin_lifecycle()
    print_header("Plugin Lifecycle Management")
    
    if not qtforge.core or not qtforge.core.create_plugin_lifecycle_manager then
        print_warning("Plugin lifecycle manager not available")
        return nil
    end
    
    local success, lifecycle_manager = pcall(qtforge.core.create_plugin_lifecycle_manager)
    
    if not success then
        print_error("Failed to create plugin lifecycle manager: " .. tostring(lifecycle_manager))
        return nil
    end
    
    print_success("Plugin lifecycle manager created successfully")
    
    -- Demonstrate lifecycle operations
    local example_plugin = "example_plugin"
    
    print("\n🔄 Managing lifecycle for: " .. example_plugin)
    
    -- Check initial state
    if lifecycle_manager.get_plugin_state then
        local state_success, state = pcall(function()
            return lifecycle_manager:get_plugin_state(example_plugin)
        end)
        
        if state_success then
            print_status("Initial state: " .. tostring(state))
        else
            print_warning("Could not get initial state: " .. tostring(state))
        end
    end
    
    -- Attempt to start plugin
    if lifecycle_manager.start_plugin then
        local start_success, result = pcall(function()
            return lifecycle_manager:start_plugin(example_plugin)
        end)
        
        if start_success then
            print("▶️  Start plugin result: " .. tostring(result))
        else
            print_warning("Could not start plugin: " .. tostring(result))
        end
    end
    
    -- Attempt to stop plugin
    if lifecycle_manager.stop_plugin then
        local stop_success, result = pcall(function()
            return lifecycle_manager:stop_plugin(example_plugin)
        end)
        
        if stop_success then
            print("⏹️  Stop plugin result: " .. tostring(result))
        else
            print_warning("Could not stop plugin: " .. tostring(result))
        end
    end
    
    return lifecycle_manager
end

-- Demonstrate plugin dependency resolution
local function demonstrate_plugin_dependencies()
    print_header("Plugin Dependency Resolution")
    
    if not qtforge.core or not qtforge.core.create_plugin_dependency_resolver then
        print_warning("Plugin dependency resolver not available")
        return nil
    end
    
    local success, resolver = pcall(qtforge.core.create_plugin_dependency_resolver)
    
    if not success then
        print_error("Failed to create plugin dependency resolver: " .. tostring(resolver))
        return nil
    end
    
    print_success("Plugin dependency resolver created successfully")
    
    -- Demonstrate dependency resolution with empty list
    print("\n🔍 Resolving dependencies for empty plugin list...")
    
    if resolver.resolve_dependencies then
        local resolve_success, resolved = pcall(function()
            return resolver:resolve_dependencies({})
        end)
        
        if resolve_success then
            local count = 0
            if resolved and type(resolved) == "table" then
                count = #resolved
            end
            print_success("Resolved dependencies: " .. count .. " plugins")
        else
            print_error("Dependency resolution failed: " .. tostring(resolved))
        end
    end
    
    -- Create mock plugin metadata if available
    if qtforge.core.PluginMetadata then
        print("\n📋 Creating mock plugin metadata...")
        
        local metadata_success, metadata1 = pcall(qtforge.core.PluginMetadata)
        local metadata2_success, metadata2 = pcall(qtforge.core.PluginMetadata)
        
        if metadata_success and metadata2_success then
            -- Set basic properties if available
            if metadata1.name then
                metadata1.name = "plugin_a"
                metadata2.name = "plugin_b"
            end
            
            print_success("Mock plugin metadata created")
            
            -- Attempt dependency resolution
            if resolver.resolve_dependencies then
                local resolve_success, resolved = pcall(function()
                    return resolver:resolve_dependencies({metadata1, metadata2})
                end)
                
                if resolve_success then
                    local count = 0
                    if resolved and type(resolved) == "table" then
                        count = #resolved
                    end
                    print_success("Resolved " .. count .. " plugins with dependencies")
                else
                    print_warning("Dependency resolution with metadata failed: " .. tostring(resolved))
                end
            end
        else
            print_error("Failed to create plugin metadata")
        end
    end
    
    return resolver
end

-- Demonstrate plugin-related enumerations
local function demonstrate_plugin_enums()
    print_header("Plugin Enumerations")
    
    -- Plugin State enum
    if PluginState then
        print("🔄 Plugin States:")
        local states = {"Unloaded", "Loading", "Loaded", "Starting", "Running", "Stopping", "Error"}
        
        for _, state in ipairs(states) do
            if PluginState[state] then
                local value = PluginState[state]
                print("  • " .. state .. ": " .. tostring(value))
            end
        end
    end
    
    -- Plugin Capability enum
    if PluginCapability then
        print("\n🛠️  Plugin Capabilities:")
        local capabilities = {"Service", "Network", "FileSystem", "Database", "UI"}
        
        for _, capability in ipairs(capabilities) do
            if PluginCapability[capability] then
                local value = PluginCapability[capability]
                print("  • " .. capability .. ": " .. tostring(value))
            end
        end
    end
    
    -- Plugin Priority enum
    if PluginPriority then
        print("\n⚡ Plugin Priorities:")
        local priorities = {"Low", "Normal", "High", "Critical"}
        
        for _, priority in ipairs(priorities) do
            if PluginPriority[priority] then
                local value = PluginPriority[priority]
                print("  • " .. priority .. ": " .. tostring(value))
            end
        end
    end
    
    -- Plugin Type enum
    if PluginType then
        print("\n🏷️  Plugin Types:")
        local types = {"Native", "Python", "Lua", "Remote", "Composite"}
        
        for _, plugin_type in ipairs(types) do
            if PluginType[plugin_type] then
                local value = PluginType[plugin_type]
                print("  • " .. plugin_type .. ": " .. tostring(value))
            end
        end
    end
end

-- Demonstrate error handling in plugin operations
local function demonstrate_error_handling()
    print_header("Error Handling Demonstration")
    
    -- Create manager for error testing
    if not qtforge.core or not qtforge.core.create_plugin_manager then
        print_error("Cannot demonstrate error handling without plugin manager")
        return
    end
    
    local success, manager = pcall(qtforge.core.create_plugin_manager)
    
    if not success then
        print_error("Failed to create manager for error testing: " .. tostring(manager))
        return
    end
    
    -- Test loading non-existent plugin
    print("🧪 Testing error handling with non-existent plugin...")
    
    local load_success, result = pcall(function()
        return manager:load_plugin("/definitely/does/not/exist.so")
    end)
    
    if load_success then
        print_warning("Unexpected success: " .. tostring(result))
    else
        print_success("Correctly caught exception: " .. tostring(result))
    end
    
    -- Test with invalid parameters
    print("\n🧪 Testing error handling with invalid parameters...")
    
    -- Test with empty path
    local empty_success, empty_result = pcall(function()
        return manager:load_plugin("")
    end)
    
    if empty_success then
        print_warning("Unexpected success with empty path: " .. tostring(empty_result))
    else
        print_success("Correctly caught exception for empty path: " .. tostring(empty_result))
    end
    
    -- Test with nil path
    local nil_success, nil_result = pcall(function()
        return manager:load_plugin(nil)
    end)
    
    if nil_success then
        print_warning("Unexpected success with nil path: " .. tostring(nil_result))
    else
        print_success("Correctly caught exception for nil path: " .. tostring(nil_result))
    end
end

-- Main demonstration function
local function main()
    print("QtForge Lua Bindings - Basic Plugin Management Example")
    print(string.rep("=", 60))
    
    -- Check if QtForge is available
    if not check_qtforge_availability() then
        return 1
    end
    
    -- Demonstrate each aspect of plugin management
    local manager = demonstrate_plugin_manager_creation()
    demonstrate_plugin_loading(manager)
    demonstrate_plugin_enumeration(manager)
    demonstrate_plugin_registry()
    demonstrate_plugin_lifecycle()
    demonstrate_plugin_dependencies()
    demonstrate_plugin_enums()
    demonstrate_error_handling()
    
    print("\n" .. string.rep("=", 60))
    print("🎉 Basic Plugin Management Example Complete!")
    print(string.rep("=", 60))
    
    print("\n📚 Key Takeaways:")
    print("• Plugin managers coordinate all plugin operations")
    print("• Plugin registries help discover and organize plugins")
    print("• Lifecycle managers handle plugin state transitions")
    print("• Dependency resolvers ensure proper loading order")
    print("• Proper error handling is essential for robust applications")
    
    print("\n🔗 Next Steps:")
    print("• Explore communication between plugins")
    print("• Learn about security and validation")
    print("• Implement custom plugin interfaces")
    print("• Set up plugin orchestration workflows")
    
    return 0
end

-- Run the main function
return main()
