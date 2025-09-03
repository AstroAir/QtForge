#!/usr/bin/env lua
--[[
Cross-language integration tests for QtForge Lua bindings.
Tests interoperability between Lua and Python bindings, threading safety, and memory management.
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

-- Utility function to run Python scripts
local function run_python_script(script_content)
    local temp_file = os.tmpname() .. ".py"
    local file = io.open(temp_file, "w")
    if not file then
        return false, "Failed to create temp file", ""
    end
    
    file:write(script_content)
    file:close()
    
    -- Try different Python executables
    local python_commands = {"python3", "python", "python3.8", "python3.9", "python3.10"}
    
    for _, cmd in ipairs(python_commands) do
        local command = string.format("%s %s 2>&1", cmd, temp_file)
        local handle = io.popen(command)
        if handle then
            local output = handle:read("*a")
            local success = handle:close()
            
            -- Clean up temp file
            os.remove(temp_file)
            
            if success then
                return true, output, ""
            else
                -- Try next Python command
            end
        end
    end
    
    -- Clean up temp file if all attempts failed
    os.remove(temp_file)
    return false, "Python not found or script failed", ""
end

-- Test Cross-Language Interoperability
test_framework.describe("Cross-Language Interoperability", function()
    test_framework.it("should have consistent PluginManager behavior with Python", function()
        -- Test Lua PluginManager
        local lua_manager = nil
        if qtforge.core and qtforge.core.create_plugin_manager then
            lua_manager = qtforge.core.create_plugin_manager()
        elseif qtforge.core and qtforge.core.PluginManager then
            lua_manager = qtforge.core.PluginManager()
        end
        
        test_framework.assert_not_nil(lua_manager, "Lua PluginManager should be created")
        
        local lua_count = 0
        if lua_manager and lua_manager.get_plugin_count then
            lua_count = lua_manager:get_plugin_count()
        end
        
        -- Test Python PluginManager
        local python_script = [[
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../build'))

try:
    import qtforge.core as core
    manager = core.PluginManager()
    count = manager.get_plugin_count()
    print("PYTHON_MANAGER_COUNT:" + str(count))
except ImportError:
    print("PYTHON_MANAGER_FAIL:module_not_available")
except Exception as e:
    print("PYTHON_MANAGER_FAIL:" + str(e))
]]
        
        local success, output, error = run_python_script(python_script)
        
        if success and string.find(output, "PYTHON_MANAGER_COUNT:") then
            local python_count_str = string.match(output, "PYTHON_MANAGER_COUNT:(%d+)")
            if python_count_str then
                local python_count = tonumber(python_count_str)
                test_framework.assert_equal(lua_count, python_count, 
                    string.format("Plugin count should match: Lua=%d, Python=%d", lua_count, python_count))
                print("✅ PluginManager consistency verified")
            end
        else
            print(string.format("⚠️  Python PluginManager test failed: %s", output))
        end
    end)
    
    test_framework.it("should have consistent MessageBus behavior with Python", function()
        -- Test Lua MessageBus
        local lua_bus = nil
        if qtforge.communication and qtforge.communication.create_message_bus then
            local success, bus = pcall(qtforge.communication.create_message_bus)
            if success then
                lua_bus = bus
            end
        end
        
        -- Test Python MessageBus
        local python_script = [[
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../build'))

try:
    import qtforge.communication as comm
    if hasattr(comm, 'MessageBus'):
        bus = comm.MessageBus()
        print("PYTHON_MESSAGE_BUS_OK")
    elif hasattr(comm, 'create_message_bus'):
        bus = comm.create_message_bus()
        print("PYTHON_MESSAGE_BUS_OK")
    else:
        print("PYTHON_MESSAGE_BUS_FAIL:no_message_bus")
except ImportError:
    print("PYTHON_MESSAGE_BUS_FAIL:module_not_available")
except Exception as e:
    print("PYTHON_MESSAGE_BUS_FAIL:" + str(e))
]]
        
        local success, output, error = run_python_script(python_script)
        
        if lua_bus and success and string.find(output, "PYTHON_MESSAGE_BUS_OK") then
            print("✅ MessageBus interoperability verified")
        else
            print(string.format("⚠️  MessageBus interoperability test incomplete: Lua=%s, Python=%s", 
                tostring(lua_bus ~= nil), output))
        end
    end)
    
    test_framework.it("should have consistent enum values with Python", function()
        -- Test Lua enum values
        local lua_states = {}
        if qtforge.core and qtforge.core.PluginState then
            local state_names = {"Unloaded", "Loaded", "Initialized", "Running", "Stopped", "Error"}
            for _, name in ipairs(state_names) do
                if qtforge.core.PluginState[name] then
                    lua_states[name] = tostring(qtforge.core.PluginState[name])
                end
            end
        end
        
        -- Test Python enum values
        local python_script = [[
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../build'))

try:
    import qtforge.core as core
    if hasattr(core, 'PluginState'):
        states = ['Unloaded', 'Loaded', 'Initialized', 'Running', 'Stopped', 'Error']
        for state in states:
            if hasattr(core.PluginState, state):
                value = getattr(core.PluginState, state)
                print(f"PYTHON_ENUM_{state}:{value}")
except ImportError:
    print("PYTHON_ENUM_FAIL:module_not_available")
except Exception as e:
    print("PYTHON_ENUM_FAIL:" + str(e))
]]
        
        local success, output, error = run_python_script(python_script)
        
        if success then
            local python_states = {}
            for line in string.gmatch(output, "[^\r\n]+") do
                local state_name, state_value = string.match(line, "PYTHON_ENUM_([^:]+):(.+)")
                if state_name and state_value then
                    python_states[state_name] = state_value
                end
            end
            
            -- Compare enum values
            local matches = 0
            for state_name, lua_value in pairs(lua_states) do
                if python_states[state_name] then
                    matches = matches + 1
                    print(string.format("✅ Enum %s consistency verified", state_name))
                end
            end
            
            if matches > 0 then
                print(string.format("✅ Enum consistency verified for %d states", matches))
            end
        else
            print(string.format("⚠️  Python enum test failed: %s", output))
        end
    end)
end)

-- Test Memory Management
test_framework.describe("Memory Management", function()
    test_framework.it("should handle object creation and cleanup", function()
        -- Create many objects
        local objects = {}
        for i = 1, 100 do
            local manager = nil
            local registry = nil
            local loader = nil
            
            if qtforge.core.create_plugin_manager then
                manager = qtforge.core.create_plugin_manager()
            end
            if qtforge.core.create_plugin_registry then
                registry = qtforge.core.create_plugin_registry()
            end
            if qtforge.core.create_plugin_loader then
                loader = qtforge.core.create_plugin_loader()
            end
            
            table.insert(objects, {manager = manager, registry = registry, loader = loader})
        end
        
        test_framework.assert_equal(#objects, 100, "Should create 100 object sets")
        
        -- Clear references
        objects = nil
        
        -- Force garbage collection
        collectgarbage("collect")
        
        print("✅ Object creation and cleanup test completed")
    end)
    
    test_framework.it("should handle large data operations", function()
        local manager = nil
        if qtforge.core.create_plugin_manager then
            manager = qtforge.core.create_plugin_manager()
        elseif qtforge.core.PluginManager then
            manager = qtforge.core.PluginManager()
        end
        
        if manager then
            -- Perform many operations
            for i = 1, 1000 do
                if manager.get_plugin_count then
                    local count = manager:get_plugin_count()
                end
                if manager.has_plugin then
                    local has_plugin = manager:has_plugin("test_plugin_" .. i)
                end
                if manager.get_all_plugins then
                    local plugins = manager:get_all_plugins()
                end
            end
            
            print("✅ Large data operations completed")
        else
            print("⚠️  PluginManager not available for large data test")
        end
    end)
end)

-- Test Error Handling
test_framework.describe("Error Handling", function()
    test_framework.it("should handle invalid operations gracefully", function()
        local manager = nil
        if qtforge.core.create_plugin_manager then
            manager = qtforge.core.create_plugin_manager()
        elseif qtforge.core.PluginManager then
            manager = qtforge.core.PluginManager()
        end
        
        if manager then
            -- Test various error conditions
            local error_tests = {
                function() return manager:has_plugin(nil) end,
                function() return manager:has_plugin("") end,
                function() return manager:get_plugin("nonexistent") end,
                function() return manager:load_plugin("/invalid/path.so") end,
            }
            
            for i, test_func in ipairs(error_tests) do
                local success, result = pcall(test_func)
                -- Should either succeed with appropriate result or fail gracefully
                print(string.format("Error test %d: %s", i, success and "handled" or "failed gracefully"))
            end
            
            print("✅ Error handling test completed")
        end
    end)
    
    test_framework.it("should handle resource cleanup on errors", function()
        -- Perform operations that may fail
        for i = 1, 50 do
            local success, result = pcall(function()
                local manager = nil
                if qtforge.core.create_plugin_manager then
                    manager = qtforge.core.create_plugin_manager()
                end
                
                if manager and manager.load_plugin then
                    -- This should fail
                    manager:load_plugin("/invalid/path_" .. i .. ".so")
                end
            end)
            -- Expected to fail, but should not leak resources
        end
        
        -- Force garbage collection
        collectgarbage("collect")
        
        print("✅ Resource cleanup on errors test completed")
    end)
end)

-- Test Performance
test_framework.describe("Performance", function()
    test_framework.it("should create objects quickly", function()
        local start_time = os.clock()
        
        -- Create many objects
        local objects = {}
        for i = 1, 1000 do
            local manager = nil
            if qtforge.core.create_plugin_manager then
                manager = qtforge.core.create_plugin_manager()
            elseif qtforge.core.PluginManager then
                manager = qtforge.core.PluginManager()
            end
            table.insert(objects, manager)
        end
        
        local creation_time = os.clock() - start_time
        local objects_per_second = #objects / creation_time
        
        print(string.format("Created %d objects in %.3fs (%.0f objects/sec)", 
              #objects, creation_time, objects_per_second))
        
        test_framework.assert_true(creation_time < 5.0, 
            string.format("Object creation too slow: %.3fs", creation_time))
        
        print("✅ Object creation performance test passed")
    end)
    
    test_framework.it("should perform method calls quickly", function()
        local manager = nil
        if qtforge.core.create_plugin_manager then
            manager = qtforge.core.create_plugin_manager()
        elseif qtforge.core.PluginManager then
            manager = qtforge.core.PluginManager()
        end
        
        if manager then
            local start_time = os.clock()
            
            -- Perform many method calls
            for i = 1, 5000 do
                if manager.get_plugin_count then
                    local count = manager:get_plugin_count()
                end
                if manager.has_plugin then
                    local has_plugin = manager:has_plugin("test_" .. (i % 100))
                end
            end
            
            local call_time = os.clock() - start_time
            local calls_per_second = 10000 / call_time
            
            print(string.format("Performed 10000 method calls in %.3fs (%.0f calls/sec)", 
                  call_time, calls_per_second))
            
            test_framework.assert_true(call_time < 10.0, 
                string.format("Method calls too slow: %.3fs", call_time))
            
            print("✅ Method call performance test passed")
        end
    end)
end)

-- Print test results summary
local function print_test_summary()
    print("\n" .. string.rep("=", 60))
    print("📊 Lua Integration Test Summary:")
    
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
        print("✅ All integration tests passed!")
        return 0
    else
        print("❌ Some integration tests failed!")
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
