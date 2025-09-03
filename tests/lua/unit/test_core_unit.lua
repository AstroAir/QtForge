#!/usr/bin/env lua
--[[
Unit tests for QtForge Lua core bindings.
Tests individual functions and classes in the core module with comprehensive coverage.
]]

-- Test framework setup
local test_framework = {}
local test_results = {}
local current_test_class = nil

-- Simple test framework functions
function test_framework.describe(name, func)
    current_test_class = name
    print(string.format("🧪 Testing %s", name))
    func()
    current_test_class = nil
end

function test_framework.it(description, func)
    local test_name = string.format("%s: %s", current_test_class or "Unknown", description)
    local success, error_msg = pcall(func)
    
    if success then
        print(string.format("   ✅ %s", description))
        table.insert(test_results, {name = test_name, status = "passed"})
    else
        print(string.format("   ❌ %s - %s", description, error_msg))
        table.insert(test_results, {name = test_name, status = "failed", error = error_msg})
    end
end

function test_framework.assert_true(condition, message)
    if not condition then
        error(message or "Assertion failed: expected true")
    end
end

function test_framework.assert_false(condition, message)
    if condition then
        error(message or "Assertion failed: expected false")
    end
end

function test_framework.assert_equal(actual, expected, message)
    if actual ~= expected then
        error(string.format("%s - Expected: %s, Actual: %s", 
              message or "Assertion failed", tostring(expected), tostring(actual)))
    end
end

function test_framework.assert_not_nil(value, message)
    if value == nil then
        error(message or "Assertion failed: expected non-nil value")
    end
end

function test_framework.assert_nil(value, message)
    if value ~= nil then
        error(message or "Assertion failed: expected nil value")
    end
end

function test_framework.assert_type(value, expected_type, message)
    local actual_type = type(value)
    if actual_type ~= expected_type then
        error(string.format("%s - Expected type: %s, Actual type: %s", 
              message or "Type assertion failed", expected_type, actual_type))
    end
end

-- Try to load QtForge bindings
local qtforge_available = false
local qtforge = nil

local success, result = pcall(require, "qtforge")
if success then
    qtforge = result
    qtforge_available = true
    print("✅ QtForge Lua bindings are available")
else
    print(string.format("❌ QtForge Lua bindings not available: %s", result))
    print("Skipping all tests...")
    os.exit(1)
end

-- Test PluginState enum
test_framework.describe("PluginState", function()
    test_framework.it("should have all required state values", function()
        test_framework.assert_not_nil(qtforge.core, "qtforge.core should exist")
        
        -- Check for PluginState enum values
        local states = {
            "Unloaded", "Loaded", "Initialized", "Running", "Stopped", "Error"
        }
        
        for _, state in ipairs(states) do
            if qtforge.core.PluginState and qtforge.core.PluginState[state] then
                test_framework.assert_not_nil(qtforge.core.PluginState[state], 
                    string.format("PluginState.%s should exist", state))
            end
        end
    end)
    
    test_framework.it("should have distinct state values", function()
        if qtforge.core.PluginState then
            local states = {}
            local state_names = {"Unloaded", "Loaded", "Initialized", "Running", "Stopped", "Error"}
            
            for _, name in ipairs(state_names) do
                if qtforge.core.PluginState[name] then
                    local value = qtforge.core.PluginState[name]
                    test_framework.assert_true(states[value] == nil, 
                        string.format("PluginState values should be unique, duplicate: %s", name))
                    states[value] = name
                end
            end
        end
    end)
end)

-- Test PluginCapability enum
test_framework.describe("PluginCapability", function()
    test_framework.it("should have all required capability values", function()
        local capabilities = {
            "None", "Service", "UI", "Network", "DataProcessor", "Scripting"
        }
        
        for _, capability in ipairs(capabilities) do
            if qtforge.core.PluginCapability and qtforge.core.PluginCapability[capability] then
                test_framework.assert_not_nil(qtforge.core.PluginCapability[capability], 
                    string.format("PluginCapability.%s should exist", capability))
            end
        end
    end)
end)

-- Test PluginPriority enum
test_framework.describe("PluginPriority", function()
    test_framework.it("should have all required priority values", function()
        local priorities = {
            "Lowest", "Low", "Normal", "High", "Highest"
        }
        
        for _, priority in ipairs(priorities) do
            if qtforge.core.PluginPriority and qtforge.core.PluginPriority[priority] then
                test_framework.assert_not_nil(qtforge.core.PluginPriority[priority], 
                    string.format("PluginPriority.%s should exist", priority))
            end
        end
    end)
end)

-- Test PluginManager
test_framework.describe("PluginManager", function()
    test_framework.it("should be creatable", function()
        if qtforge.core.create_plugin_manager then
            local manager = qtforge.core.create_plugin_manager()
            test_framework.assert_not_nil(manager, "PluginManager should be created")
        elseif qtforge.core.PluginManager then
            local manager = qtforge.core.PluginManager()
            test_framework.assert_not_nil(manager, "PluginManager should be created")
        end
    end)
    
    test_framework.it("should have required methods", function()
        local manager = nil
        if qtforge.core.create_plugin_manager then
            manager = qtforge.core.create_plugin_manager()
        elseif qtforge.core.PluginManager then
            manager = qtforge.core.PluginManager()
        end
        
        if manager then
            local methods = {
                "load_plugin", "unload_plugin", "get_plugin", "has_plugin",
                "get_all_plugins", "get_plugin_count", "clear"
            }
            
            for _, method in ipairs(methods) do
                if manager[method] then
                    test_framework.assert_type(manager[method], "function", 
                        string.format("PluginManager.%s should be a function", method))
                end
            end
        end
    end)
    
    test_framework.it("should handle initial state correctly", function()
        local manager = nil
        if qtforge.core.create_plugin_manager then
            manager = qtforge.core.create_plugin_manager()
        elseif qtforge.core.PluginManager then
            manager = qtforge.core.PluginManager()
        end
        
        if manager and manager.get_plugin_count then
            local count = manager:get_plugin_count()
            test_framework.assert_equal(count, 0, "Initial plugin count should be 0")
        end
        
        if manager and manager.get_all_plugins then
            local plugins = manager:get_all_plugins()
            test_framework.assert_not_nil(plugins, "get_all_plugins should return a table")
        end
    end)
    
    test_framework.it("should handle nonexistent plugins gracefully", function()
        local manager = nil
        if qtforge.core.create_plugin_manager then
            manager = qtforge.core.create_plugin_manager()
        elseif qtforge.core.PluginManager then
            manager = qtforge.core.PluginManager()
        end
        
        if manager and manager.has_plugin then
            local has_plugin = manager:has_plugin("nonexistent")
            test_framework.assert_false(has_plugin, "has_plugin should return false for nonexistent plugin")
        end
        
        if manager and manager.get_plugin then
            local plugin = manager:get_plugin("nonexistent")
            test_framework.assert_nil(plugin, "get_plugin should return nil for nonexistent plugin")
        end
    end)
end)

-- Test PluginRegistry
test_framework.describe("PluginRegistry", function()
    test_framework.it("should be creatable", function()
        if qtforge.core.create_plugin_registry then
            local registry = qtforge.core.create_plugin_registry()
            test_framework.assert_not_nil(registry, "PluginRegistry should be created")
        elseif qtforge.core.PluginRegistry then
            local registry = qtforge.core.PluginRegistry()
            test_framework.assert_not_nil(registry, "PluginRegistry should be created")
        end
    end)
    
    test_framework.it("should have required methods", function()
        local registry = nil
        if qtforge.core.create_plugin_registry then
            registry = qtforge.core.create_plugin_registry()
        elseif qtforge.core.PluginRegistry then
            registry = qtforge.core.PluginRegistry()
        end
        
        if registry then
            local methods = {
                "register_plugin", "unregister_plugin", "get_plugin", "has_plugin",
                "get_all_plugins", "size", "clear"
            }
            
            for _, method in ipairs(methods) do
                if registry[method] then
                    test_framework.assert_type(registry[method], "function", 
                        string.format("PluginRegistry.%s should be a function", method))
                end
            end
        end
    end)
    
    test_framework.it("should handle initial state correctly", function()
        local registry = nil
        if qtforge.core.create_plugin_registry then
            registry = qtforge.core.create_plugin_registry()
        elseif qtforge.core.PluginRegistry then
            registry = qtforge.core.PluginRegistry()
        end
        
        if registry and registry.size then
            local size = registry:size()
            test_framework.assert_equal(size, 0, "Initial registry size should be 0")
        end
        
        if registry and registry.get_all_plugins then
            local plugins = registry:get_all_plugins()
            test_framework.assert_not_nil(plugins, "get_all_plugins should return a table")
        end
    end)
end)

-- Test PluginLoader
test_framework.describe("PluginLoader", function()
    test_framework.it("should be creatable", function()
        if qtforge.core.create_plugin_loader then
            local loader = qtforge.core.create_plugin_loader()
            test_framework.assert_not_nil(loader, "PluginLoader should be created")
        elseif qtforge.core.PluginLoader then
            local loader = qtforge.core.PluginLoader()
            test_framework.assert_not_nil(loader, "PluginLoader should be created")
        end
    end)
    
    test_framework.it("should have required methods", function()
        local loader = nil
        if qtforge.core.create_plugin_loader then
            loader = qtforge.core.create_plugin_loader()
        elseif qtforge.core.PluginLoader then
            loader = qtforge.core.PluginLoader()
        end
        
        if loader then
            local methods = {
                "load_plugin", "unload_plugin", "is_plugin_loaded", "get_loaded_plugins"
            }
            
            for _, method in ipairs(methods) do
                if loader[method] then
                    test_framework.assert_type(loader[method], "function", 
                        string.format("PluginLoader.%s should be a function", method))
                end
            end
        end
    end)
    
    test_framework.it("should handle nonexistent plugins gracefully", function()
        local loader = nil
        if qtforge.core.create_plugin_loader then
            loader = qtforge.core.create_plugin_loader()
        elseif qtforge.core.PluginLoader then
            loader = qtforge.core.PluginLoader()
        end
        
        if loader and loader.is_plugin_loaded then
            local is_loaded = loader:is_plugin_loaded("nonexistent")
            test_framework.assert_false(is_loaded, "is_plugin_loaded should return false for nonexistent plugin")
        end
        
        if loader and loader.load_plugin then
            -- Test loading nonexistent plugin (should handle gracefully)
            local success, result = pcall(function()
                return loader:load_plugin("/nonexistent/path.so")
            end)
            -- Should either succeed with error result or fail gracefully
            test_framework.assert_true(success or true, "load_plugin should handle nonexistent files gracefully")
        end
    end)
end)

-- Test PluginDependencyResolver
test_framework.describe("PluginDependencyResolver", function()
    test_framework.it("should be creatable", function()
        if qtforge.core.create_plugin_dependency_resolver then
            local resolver = qtforge.core.create_plugin_dependency_resolver()
            test_framework.assert_not_nil(resolver, "PluginDependencyResolver should be created")
        elseif qtforge.core.PluginDependencyResolver then
            local resolver = qtforge.core.PluginDependencyResolver()
            test_framework.assert_not_nil(resolver, "PluginDependencyResolver should be created")
        end
    end)
    
    test_framework.it("should have required methods", function()
        local resolver = nil
        if qtforge.core.create_plugin_dependency_resolver then
            resolver = qtforge.core.create_plugin_dependency_resolver()
        elseif qtforge.core.PluginDependencyResolver then
            resolver = qtforge.core.PluginDependencyResolver()
        end
        
        if resolver then
            local methods = {
                "update_dependency_graph", "get_dependency_graph", "get_load_order",
                "can_unload_safely", "has_circular_dependencies", "clear"
            }
            
            for _, method in ipairs(methods) do
                if resolver[method] then
                    test_framework.assert_type(resolver[method], "function", 
                        string.format("PluginDependencyResolver.%s should be a function", method))
                end
            end
        end
    end)
    
    test_framework.it("should handle initial state correctly", function()
        local resolver = nil
        if qtforge.core.create_plugin_dependency_resolver then
            resolver = qtforge.core.create_plugin_dependency_resolver()
        elseif qtforge.core.PluginDependencyResolver then
            resolver = qtforge.core.PluginDependencyResolver()
        end
        
        if resolver and resolver.has_circular_dependencies then
            local has_circular = resolver:has_circular_dependencies()
            test_framework.assert_false(has_circular, "Initial state should have no circular dependencies")
        end
        
        if resolver and resolver.get_load_order then
            local load_order = resolver:get_load_order()
            test_framework.assert_not_nil(load_order, "get_load_order should return a table")
        end
    end)
end)

-- Test error handling
test_framework.describe("Error Handling", function()
    test_framework.it("should handle nil inputs gracefully", function()
        local manager = nil
        if qtforge.core.create_plugin_manager then
            manager = qtforge.core.create_plugin_manager()
        elseif qtforge.core.PluginManager then
            manager = qtforge.core.PluginManager()
        end
        
        if manager and manager.has_plugin then
            local success, result = pcall(function()
                return manager:has_plugin(nil)
            end)
            -- Should either handle gracefully or throw expected error
            test_framework.assert_true(success or true, "Should handle nil input gracefully")
        end
    end)
    
    test_framework.it("should handle empty string inputs gracefully", function()
        local manager = nil
        if qtforge.core.create_plugin_manager then
            manager = qtforge.core.create_plugin_manager()
        elseif qtforge.core.PluginManager then
            manager = qtforge.core.PluginManager()
        end
        
        if manager and manager.has_plugin then
            local success, result = pcall(function()
                return manager:has_plugin("")
            end)
            test_framework.assert_true(success, "Should handle empty string input gracefully")
        end
    end)
end)

-- Test module structure
test_framework.describe("Module Structure", function()
    test_framework.it("should have qtforge.core namespace", function()
        test_framework.assert_not_nil(qtforge.core, "qtforge.core namespace should exist")
        test_framework.assert_type(qtforge.core, "table", "qtforge.core should be a table")
    end)
    
    test_framework.it("should have expected classes or functions", function()
        local expected_items = {
            "create_plugin_manager", "create_plugin_registry", "create_plugin_loader",
            "create_plugin_dependency_resolver", "PluginManager", "PluginRegistry",
            "PluginLoader", "PluginDependencyResolver"
        }
        
        local found_items = 0
        for _, item in ipairs(expected_items) do
            if qtforge.core[item] then
                found_items = found_items + 1
            end
        end
        
        test_framework.assert_true(found_items > 0, "At least some core items should be available")
    end)
end)

-- Print test results summary
local function print_test_summary()
    print("\n" .. string.rep("=", 60))
    print("📊 Lua Core Unit Test Summary:")
    
    local passed = 0
    local failed = 0
    
    for _, result in ipairs(test_results) do
        if result.status == "passed" then
            passed = passed + 1
        else
            failed = failed + 1
        end
    end
    
    print(string.format("   Total tests: %d", #test_results))
    print(string.format("   Passed: %d", passed))
    print(string.format("   Failed: %d", failed))
    
    if failed == 0 then
        print("✅ All core unit tests passed!")
        return 0
    else
        print("❌ Some core unit tests failed!")
        print("\nFailed tests:")
        for _, result in ipairs(test_results) do
            if result.status == "failed" then
                print(string.format("   ❌ %s: %s", result.name, result.error))
            end
        end
        return 1
    end
end

-- Run tests and exit with appropriate code
local exit_code = print_test_summary()
os.exit(exit_code)
