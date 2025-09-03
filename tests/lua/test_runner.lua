#!/usr/bin/env lua
--[[
Comprehensive test runner for QtForge Lua bindings.
Runs all test modules and provides detailed reporting.
--]]

-- Helper function to check if a file exists
local function file_exists(path)
    local file = io.open(path, "r")
    if file then
        file:close()
        return true
    end
    return false
end

-- Helper function to get the directory of the current script
local function get_script_dir()
    local str = debug.getinfo(2, "S").source:sub(2)
    return str:match("(.*/)")
end

-- Check if QtForge Lua bindings are available
local function check_bindings_availability()
    local success, err = pcall(function()
        return qtforge ~= nil
    end)
    
    if success and qtforge then
        return true, nil
    else
        return false, err or "qtforge module not found"
    end
end

-- Run a single test file
local function run_test_file(test_path, test_name)
    print(string.format("🧪 Running %s...", test_name))
    
    if not file_exists(test_path) then
        print(string.format("❌ Test file not found: %s", test_path))
        return false
    end
    
    local success, result = pcall(dofile, test_path)
    
    if success then
        if result == 0 then
            print(string.format("✅ %s passed", test_name))
            return true
        else
            print(string.format("❌ %s failed (exit code: %s)", test_name, tostring(result)))
            return false
        end
    else
        print(string.format("❌ %s crashed: %s", test_name, tostring(result)))
        return false
    end
end

-- Run comprehensive tests
local function run_comprehensive_tests()
    print("QtForge Lua Bindings - Comprehensive Test Suite")
    print(string.rep("=", 60))
    
    -- Check if bindings are available
    local available, error = check_bindings_availability()
    if not available then
        print(string.format("❌ QtForge Lua bindings not available: %s", error))
        print("\nTo run tests, ensure QtForge is built with Lua bindings enabled:")
        print("  cmake -DQTFORGE_BUILD_LUA_BINDINGS=ON ..")
        print("  make")
        return 1
    end
    
    print("✅ QtForge Lua bindings are available")
    print()
    
    -- Define test modules
    local test_modules = {
        {"test_comprehensive_core.lua", "Core Bindings"},
        {"test_comprehensive_utils.lua", "Utils Bindings"},
        {"test_comprehensive_remaining_modules.lua", "Remaining Modules"},
        {"test_updated_bindings.lua", "Updated Bindings"}  -- Include existing test
    }
    
    -- Get the directory containing this script
    local script_dir = get_script_dir() or "./"
    
    -- Check which test files exist and run them
    local existing_tests = {}
    local missing_tests = {}
    
    for _, test_info in ipairs(test_modules) do
        local test_file, test_name = test_info[1], test_info[2]
        local test_path = script_dir .. test_file
        
        if file_exists(test_path) then
            table.insert(existing_tests, {test_path, test_name})
        else
            table.insert(missing_tests, test_file)
        end
    end
    
    if #missing_tests > 0 then
        print("⚠️  Some test modules are missing:")
        for _, missing in ipairs(missing_tests) do
            print(string.format("   - %s", missing))
        end
        print()
    end
    
    if #existing_tests == 0 then
        print("❌ No test modules found!")
        return 1
    end
    
    print(string.format("🧪 Running %d test modules...", #existing_tests))
    print()
    
    -- Run the tests
    local start_time = os.clock()
    local passed_tests = 0
    local failed_tests = 0
    
    for _, test_info in ipairs(existing_tests) do
        local test_path, test_name = test_info[1], test_info[2]
        
        if run_test_file(test_path, test_name) then
            passed_tests = passed_tests + 1
        else
            failed_tests = failed_tests + 1
        end
        print()
    end
    
    local end_time = os.clock()
    local duration = end_time - start_time
    
    -- Print summary
    print(string.rep("=", 60))
    print(string.format("Test execution completed in %.2f seconds", duration))
    print(string.format("Results: %d passed, %d failed", passed_tests, failed_tests))
    
    if failed_tests == 0 then
        print("✅ All tests passed!")
        return 0
    else
        print("❌ Some tests failed!")
        return 1
    end
end

-- Run a specific test module
local function run_specific_module(module_name)
    local script_dir = get_script_dir() or "./"
    local test_path = script_dir .. string.format("test_comprehensive_%s.lua", module_name)
    
    if not file_exists(test_path) then
        print(string.format("❌ Test module not found: %s", test_path))
        return 1
    end
    
    print(string.format("🧪 Running tests for %s module...", module_name))
    
    if run_test_file(test_path, module_name .. " module") then
        return 0
    else
        return 1
    end
end

-- List available test modules
local function list_available_modules()
    print("Available test modules:")
    print(string.rep("-", 30))
    
    local modules = {
        {"core", "Core plugin system functionality"},
        {"utils", "Utility functions and helpers"},
        {"remaining_modules", "Communication, security, managers, orchestration, monitoring"}
    }
    
    local script_dir = get_script_dir() or "./"
    
    for _, module_info in ipairs(modules) do
        local module, description = module_info[1], module_info[2]
        local test_path = script_dir .. string.format("test_comprehensive_%s.lua", module)
        local status = file_exists(test_path) and "✅" or "❌"
        print(string.format("%s %-20s - %s", status, module, description))
    end
end

-- Print help information
local function print_help()
    print("QtForge Lua Bindings Test Runner")
    print()
    print("Usage:")
    print("  lua test_runner.lua                    # Run all tests")
    print("  lua test_runner.lua <module>           # Run specific module tests")
    print("  lua test_runner.lua --list             # List available modules")
    print("  lua test_runner.lua --help             # Show this help")
    print()
    print("Examples:")
    print("  lua test_runner.lua core               # Test core module")
    print("  lua test_runner.lua utils              # Test utils module")
    print("  lua test_runner.lua remaining_modules  # Test remaining modules")
end

-- Test individual components for debugging
local function test_qtforge_availability()
    print("Testing QtForge availability...")
    
    -- Test basic module availability
    if qtforge then
        print("✅ qtforge module is available")
        
        -- Test submodules
        local submodules = {"core", "utils", "communication", "security", "managers", "orchestration", "monitoring"}
        for _, submodule in ipairs(submodules) do
            if qtforge[submodule] then
                print(string.format("✅ qtforge.%s is available", submodule))
            else
                print(string.format("❌ qtforge.%s is not available", submodule))
            end
        end
        
        -- Test some basic functions
        if qtforge.core and qtforge.core.create_plugin_manager then
            print("✅ qtforge.core.create_plugin_manager is available")
            
            local success, manager = pcall(qtforge.core.create_plugin_manager)
            if success and manager then
                print("✅ PluginManager can be created")
            else
                print("❌ PluginManager creation failed")
            end
        else
            print("❌ qtforge.core.create_plugin_manager is not available")
        end
        
    else
        print("❌ qtforge module is not available")
        print("Ensure QtForge is built with Lua bindings enabled:")
        print("  cmake -DQTFORGE_BUILD_LUA_BINDINGS=ON ..")
        print("  make")
    end
end

-- Main entry point
local function main(args)
    args = args or {}
    
    if #args == 0 then
        -- Run all tests
        return run_comprehensive_tests()
    elseif #args == 1 then
        local arg = args[1]
        
        if arg == "-h" or arg == "--help" then
            print_help()
            return 0
        elseif arg == "--list" then
            list_available_modules()
            return 0
        elseif arg == "--test-availability" then
            test_qtforge_availability()
            return 0
        else
            -- Run specific module
            return run_specific_module(arg)
        end
    else
        print("❌ Too many arguments. Use --help for usage information.")
        return 1
    end
end

-- Run the main function with command line arguments
return main(arg)
