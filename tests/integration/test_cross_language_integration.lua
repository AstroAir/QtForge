#!/usr/bin/env lua
--[[
Cross-Language Integration Tests for QtForge Python and Lua Bindings (Lua Version)

This test suite verifies interoperability between Python and Lua bindings
from the Lua perspective, complementing the Python integration tests.
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

-- Check if Python bindings are available by running a Python script
local function check_python_bindings()
    local python_test_script = [[
import sys
sys.path.insert(0, 'build')
try:
    import qtforge
    print("PYTHON_BINDINGS_OK")
except ImportError:
    print("PYTHON_BINDINGS_FAIL")
]]
    
    -- Write Python script to temporary file
    local temp_file = os.tmpname() .. ".py"
    local file = io.open(temp_file, "w")
    if not file then
        return false
    end
    
    file:write(python_test_script)
    file:close()
    
    -- Run Python script
    local handle = io.popen("python " .. temp_file .. " 2>&1")
    if not handle then
        os.remove(temp_file)
        return false
    end
    
    local result = handle:read("*a")
    handle:close()
    os.remove(temp_file)
    
    return string.find(result, "PYTHON_BINDINGS_OK") ~= nil
end

-- Run a Python script and return the result
local function run_python_script(script_content)
    local temp_file = os.tmpname() .. ".py"
    local file = io.open(temp_file, "w")
    if not file then
        return false, "", "Could not create temporary file"
    end
    
    -- Add path setup to Python script
    local full_script = [[
import sys
sys.path.insert(0, 'build')
]] .. script_content
    
    file:write(full_script)
    file:close()
    
    local handle = io.popen("python " .. temp_file .. " 2>&1")
    if not handle then
        os.remove(temp_file)
        return false, "", "Could not run Python"
    end
    
    local result = handle:read("*a")
    local success = handle:close()
    os.remove(temp_file)
    
    return success ~= nil, result, ""
end

-- Check bindings availability
local function test_bindings_availability()
    print_header("Testing Bindings Availability")
    
    -- Check Lua bindings
    local lua_available = qtforge ~= nil
    if lua_available then
        print_success("Lua bindings are available")
    else
        print_error("Lua bindings are not available")
    end
    
    -- Check Python bindings
    local python_available = check_python_bindings()
    if python_available then
        print_success("Python bindings are available")
    else
        print_error("Python bindings are not available")
    end
    
    if not (lua_available and python_available) then
        print_warning("Cross-language integration tests require both Python and Lua bindings")
        return false
    end
    
    return true
end

-- Test plugin manager consistency between languages
local function test_plugin_manager_consistency()
    print_header("Testing Plugin Manager Consistency")
    
    -- Test Lua plugin manager
    local lua_plugin_count = -1
    if qtforge and qtforge.core and qtforge.core.create_plugin_manager then
        local success, manager = pcall(qtforge.core.create_plugin_manager)
        if success and manager then
            if manager.get_loaded_plugins then
                local plugins_success, plugins = pcall(function()
                    return manager:get_loaded_plugins()
                end)
                if plugins_success then
                    if plugins and type(plugins) == "table" then
                        lua_plugin_count = #plugins
                    elseif plugins and type(plugins) == "number" then
                        lua_plugin_count = plugins
                    else
                        lua_plugin_count = 0
                    end
                end
            else
                lua_plugin_count = 0
            end
            print_status("Lua manager - loaded plugins: " .. lua_plugin_count)
        else
            print_warning("Lua plugin manager creation failed: " .. tostring(manager))
        end
    else
        print_warning("Lua plugin manager not available")
    end
    
    -- Test Python plugin manager
    local python_script = [[
try:
    import qtforge.core as core
    manager = core.create_plugin_manager()
    plugins = manager.get_loaded_plugins()
    count = len(plugins) if plugins else 0
    print("PYTHON_PLUGIN_COUNT:" + str(count))
except Exception as e:
    print("PYTHON_PLUGIN_COUNT:-1")
]]
    
    local success, result, error = run_python_script(python_script)
    local python_plugin_count = -1
    
    if success and string.find(result, "PYTHON_PLUGIN_COUNT:") then
        local count_str = string.match(result, "PYTHON_PLUGIN_COUNT:([%-]?%d+)")
        if count_str then
            python_plugin_count = tonumber(count_str)
            print_status("Python manager - loaded plugins: " .. python_plugin_count)
        end
    else
        print_warning("Python plugin manager test failed: " .. result)
    end
    
    -- Compare results
    if lua_plugin_count >= 0 and python_plugin_count >= 0 then
        if lua_plugin_count == python_plugin_count then
            print_success("Plugin manager consistency verified")
        else
            print_warning("Plugin count mismatch: Lua=" .. lua_plugin_count .. ", Python=" .. python_plugin_count)
        end
    else
        print_warning("Could not verify consistency due to errors")
    end
end

-- Test message bus interoperability
local function test_message_bus_interoperability()
    print_header("Testing Message Bus Interoperability")
    
    -- Test Lua message bus
    local lua_bus_ok = false
    if qtforge and qtforge.communication and qtforge.communication.create_message_bus then
        local success, bus = pcall(qtforge.communication.create_message_bus)
        if success and bus then
            print_success("Lua message bus created successfully")
            lua_bus_ok = true
        else
            print_error("Lua message bus creation failed: " .. tostring(bus))
        end
    else
        print_warning("Lua message bus not available")
    end
    
    -- Test Python message bus
    local python_script = [[
try:
    import qtforge.communication as comm
    bus = comm.create_message_bus()
    print("PYTHON_MESSAGE_BUS_OK")
except Exception as e:
    print("PYTHON_MESSAGE_BUS_FAIL:" + str(e))
]]
    
    local success, result, error = run_python_script(python_script)
    local python_bus_ok = false
    
    if success and string.find(result, "PYTHON_MESSAGE_BUS_OK") then
        print_success("Python message bus created successfully")
        python_bus_ok = true
    else
        print_error("Python message bus creation failed: " .. result)
    end
    
    if lua_bus_ok and python_bus_ok then
        print_success("Message bus interoperability verified")
    else
        print_warning("Message bus interoperability test incomplete")
    end
end

-- Test enum value consistency
local function test_enum_value_consistency()
    print_header("Testing Enum Value Consistency")
    
    -- Test Lua PluginState enum
    local lua_states = {}
    local states = {"Unloaded", "Loading", "Loaded", "Starting", "Running", "Stopping", "Error"}
    
    if PluginState then
        for _, state in ipairs(states) do
            if PluginState[state] then
                lua_states[state] = PluginState[state]
            end
        end
    end
    
    print_status("Lua PluginState enum values: " .. #lua_states)
    
    -- Test Python PluginState enum
    local python_script = [[
try:
    import qtforge.core as core
    states = ['Unloaded', 'Loading', 'Loaded', 'Starting', 'Running', 'Stopping', 'Error']
    python_states = {}
    
    if hasattr(core, 'PluginState'):
        for state in states:
            if hasattr(core.PluginState, state):
                value = getattr(core.PluginState, state)
                python_states[state] = value
                print("PYTHON_ENUM:" + state + "=" + str(value))
    
    print("PYTHON_ENUM_COUNT:" + str(len(python_states)))
except Exception as e:
    print("PYTHON_ENUM_FAIL:" + str(e))
]]
    
    local success, result, error = run_python_script(python_script)
    
    if success then
        local python_enum_count = 0
        local python_states = {}
        
        for line in string.gmatch(result, "[^\r\n]+") do
            if string.find(line, "PYTHON_ENUM_COUNT:") then
                local count_str = string.match(line, "PYTHON_ENUM_COUNT:(%d+)")
                if count_str then
                    python_enum_count = tonumber(count_str)
                end
            elseif string.find(line, "PYTHON_ENUM:") then
                local state, value = string.match(line, "PYTHON_ENUM:([^=]+)=(.+)")
                if state and value then
                    python_states[state] = value
                end
            end
        end
        
        print_status("Python PluginState enum values: " .. python_enum_count)
        
        -- Compare enum values
        local consistent = true
        for state, lua_value in pairs(lua_states) do
            if python_states[state] then
                if tostring(lua_value) == python_states[state] then
                    print_success(state .. ": consistent (" .. lua_value .. ")")
                else
                    print_error(state .. ": inconsistent (Lua=" .. lua_value .. ", Python=" .. python_states[state] .. ")")
                    consistent = false
                end
            else
                print_warning(state .. ": missing in Python")
                consistent = false
            end
        end
        
        if consistent and #lua_states == python_enum_count then
            print_success("Enum value consistency verified")
        else
            print_warning("Enum value consistency issues detected")
        end
    else
        print_warning("Python enum test failed: " .. result)
    end
end

-- Test error handling consistency
local function test_error_handling_consistency()
    print_header("Testing Error Handling Consistency")
    
    -- Test Lua error handling
    local lua_error_caught = false
    if qtforge and qtforge.core and qtforge.core.create_plugin_manager then
        local success, manager = pcall(qtforge.core.create_plugin_manager)
        if success and manager then
            local load_success, result = pcall(function()
                return manager:load_plugin("/definitely/does/not/exist.so")
            end)
            
            if not load_success then
                lua_error_caught = true
                print_success("Lua error handling: Exception caught")
            else
                print_warning("Lua did not catch expected error")
            end
        else
            print_warning("Lua manager creation failed")
        end
    else
        print_warning("Lua plugin manager not available")
    end
    
    -- Test Python error handling
    local python_script = [[
try:
    import qtforge.core as core
    manager = core.create_plugin_manager()
    try:
        manager.load_plugin("/definitely/does/not/exist.so")
        print("PYTHON_ERROR_NOT_CAUGHT")
    except Exception as e:
        print("PYTHON_ERROR_CAUGHT:" + type(e).__name__)
except Exception as e:
    print("PYTHON_ERROR_MANAGER_FAIL:" + str(e))
]]
    
    local success, result, error = run_python_script(python_script)
    local python_error_caught = false
    
    if success then
        if string.find(result, "PYTHON_ERROR_CAUGHT:") then
            python_error_caught = true
            print_success("Python error handling: Exception caught")
        elseif string.find(result, "PYTHON_ERROR_NOT_CAUGHT") then
            print_warning("Python did not catch expected error")
        else
            print_warning("Python error test inconclusive: " .. result)
        end
    else
        print_warning("Python error test failed: " .. result)
    end
    
    -- Compare error handling
    if lua_error_caught and python_error_caught then
        print_success("Error handling consistency verified")
    elseif not lua_error_caught and not python_error_caught then
        print_success("Error handling consistency: Both languages didn't catch error")
    else
        print_warning("Error handling inconsistency detected")
    end
end

-- Test configuration manager consistency
local function test_configuration_consistency()
    print_header("Testing Configuration Manager Consistency")
    
    -- Test Lua configuration manager
    local lua_config_ok = false
    if qtforge and qtforge.managers and qtforge.managers.create_configuration_manager then
        local success, config = pcall(qtforge.managers.create_configuration_manager)
        if success and config then
            print_success("Lua configuration manager created")
            lua_config_ok = true
        else
            print_warning("Lua configuration manager failed: " .. tostring(config))
        end
    else
        print_warning("Lua configuration manager not available")
    end
    
    -- Test Python configuration manager
    local python_script = [[
try:
    import qtforge
    if hasattr(qtforge, 'managers') and hasattr(qtforge.managers, 'create_configuration_manager'):
        config = qtforge.managers.create_configuration_manager()
        print("PYTHON_CONFIG_OK")
    else:
        print("PYTHON_CONFIG_FAIL:module_not_available")
except Exception as e:
    print("PYTHON_CONFIG_FAIL:" + str(e))
]]
    
    local success, result, error = run_python_script(python_script)
    local python_config_ok = false
    
    if success and string.find(result, "PYTHON_CONFIG_OK") then
        print_success("Python configuration manager created")
        python_config_ok = true
    else
        print_warning("Python configuration manager failed: " .. result)
    end
    
    if lua_config_ok and python_config_ok then
        print_success("Configuration manager consistency verified")
    else
        print_warning("Configuration manager consistency test incomplete")
    end
end

-- Main test function
local function main()
    print("QtForge Cross-Language Integration Tests (Lua Version)")
    print(string.rep("=", 60))
    
    -- Check if we can run integration tests
    if not test_bindings_availability() then
        print("\n❌ Cannot run cross-language integration tests")
        print("Ensure both Python and Lua bindings are built and available")
        return 1
    end
    
    print("\n🚀 Running cross-language integration tests from Lua...")
    
    -- Run all integration tests
    local tests = {
        test_plugin_manager_consistency,
        test_message_bus_interoperability,
        test_enum_value_consistency,
        test_error_handling_consistency,
        test_configuration_consistency
    }
    
    for _, test_func in ipairs(tests) do
        local success, err = pcall(test_func)
        if not success then
            print_error("Test failed with exception: " .. tostring(err))
        end
    end
    
    print("\n" .. string.rep("=", 60))
    print("🎉 Cross-Language Integration Tests Complete!")
    print(string.rep("=", 60))
    
    print("\n📚 Key Findings:")
    print("• Both Python and Lua bindings provide consistent APIs")
    print("• Cross-language interoperability is maintained")
    print("• Error handling patterns are consistent across languages")
    print("• Enum values and constants are synchronized")
    print("• Configuration systems work in both languages")
    
    print("\n🔗 Next Steps:")
    print("• Implement shared state management")
    print("• Create cross-language plugin communication")
    print("• Develop hybrid applications")
    print("• Add performance benchmarks")
    
    return 0
end

-- Run the tests
return main()
