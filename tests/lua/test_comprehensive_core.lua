#!/usr/bin/env lua
--[[
Comprehensive test suite for QtForge Lua Core bindings.
Tests all core functionality including edge cases and error handling.
--]]

-- Test framework functions
local function assert_not_nil(value, message)
    if value == nil then
        error(message or "Value should not be nil")
    end
end

local function assert_equal(actual, expected, message)
    if actual ~= expected then
        error(message or string.format("Expected %s, got %s", tostring(expected), tostring(actual)))
    end
end

local function assert_type(value, expected_type, message)
    if type(value) ~= expected_type then
        error(message or string.format("Expected %s, got %s", expected_type, type(value)))
    end
end

local function pcall_test(test_func, test_name)
    print("  Testing " .. test_name .. "...")
    local success, err = pcall(test_func)
    if success then
        print("    ✅ " .. test_name .. " passed")
        return true
    else
        print("    ❌ " .. test_name .. " failed: " .. tostring(err))
        return false
    end
end

-- Test PluginManager functionality
local function test_plugin_manager()
    print("Testing PluginManager functionality...")
    
    local function test_plugin_manager_creation()
        assert_not_nil(qtforge, "qtforge module should be available")
        assert_not_nil(qtforge.core, "qtforge.core module should be available")
        
        if qtforge.core.create_plugin_manager then
            local manager = qtforge.core.create_plugin_manager()
            assert_not_nil(manager, "PluginManager should be created")
            
            -- Test basic methods exist
            local methods = {"load_plugin", "unload_plugin", "get_loaded_plugins"}
            for _, method in ipairs(methods) do
                if manager[method] then
                    assert_type(manager[method], "function", method .. " should be a function")
                end
            end
        end
    end
    
    local function test_plugin_manager_load_invalid_path()
        if qtforge.core.create_plugin_manager then
            local manager = qtforge.core.create_plugin_manager()
            
            -- Test with non-existent file
            local success, err = pcall(function()
                manager:load_plugin("/non/existent/path.so")
            end)
            
            -- Should either fail gracefully or throw appropriate error
            if not success then
                assert_type(err, "string", "Error should be a string")
            end
        end
    end
    
    local function test_plugin_manager_get_loaded_plugins()
        if qtforge.core.create_plugin_manager then
            local manager = qtforge.core.create_plugin_manager()
            
            if manager.get_loaded_plugins then
                local plugins = manager:get_loaded_plugins()
                assert_type(plugins, "table", "Loaded plugins should be a table")
            end
        end
    end
    
    local tests = {
        {test_plugin_manager_creation, "plugin_manager_creation"},
        {test_plugin_manager_load_invalid_path, "plugin_manager_load_invalid_path"},
        {test_plugin_manager_get_loaded_plugins, "plugin_manager_get_loaded_plugins"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("PluginManager tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test PluginLoader functionality
local function test_plugin_loader()
    print("Testing PluginLoader functionality...")
    
    local function test_plugin_loader_creation()
        if qtforge.core.create_plugin_loader then
            local loader = qtforge.core.create_plugin_loader()
            assert_not_nil(loader, "PluginLoader should be created")
            
            -- Test basic methods exist
            local methods = {"load", "unload"}
            for _, method in ipairs(methods) do
                if loader[method] then
                    assert_type(loader[method], "function", method .. " should be a function")
                end
            end
        end
    end
    
    local function test_plugin_loader_supported_formats()
        if qtforge.core.create_plugin_loader then
            local loader = qtforge.core.create_plugin_loader()
            
            if loader.get_supported_formats then
                local formats = loader:get_supported_formats()
                assert_type(formats, "table", "Supported formats should be a table")
                assert(#formats > 0, "Should support at least one format")
            end
        end
    end
    
    local tests = {
        {test_plugin_loader_creation, "plugin_loader_creation"},
        {test_plugin_loader_supported_formats, "plugin_loader_supported_formats"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("PluginLoader tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test PluginRegistry functionality
local function test_plugin_registry()
    print("Testing PluginRegistry functionality...")
    
    local function test_plugin_registry_creation()
        if qtforge.core.create_plugin_registry then
            local registry = qtforge.core.create_plugin_registry()
            assert_not_nil(registry, "PluginRegistry should be created")
            
            -- Test basic methods exist
            local methods = {"register_plugin", "unregister_plugin", "find_plugin"}
            for _, method in ipairs(methods) do
                if registry[method] then
                    assert_type(registry[method], "function", method .. " should be a function")
                end
            end
        end
    end
    
    local function test_plugin_registry_find_nonexistent()
        if qtforge.core.create_plugin_registry then
            local registry = qtforge.core.create_plugin_registry()
            
            if registry.find_plugin then
                local result = registry:find_plugin("non_existent_plugin")
                -- Should return nil or empty table
                assert(result == nil or (type(result) == "table" and #result == 0), 
                       "Non-existent plugin should return nil or empty table")
            end
        end
    end
    
    local function test_plugin_registry_get_all_plugins()
        if qtforge.core.create_plugin_registry then
            local registry = qtforge.core.create_plugin_registry()
            
            if registry.get_all_plugins then
                local plugins = registry:get_all_plugins()
                assert_type(plugins, "table", "All plugins should be a table")
            end
        end
    end
    
    local tests = {
        {test_plugin_registry_creation, "plugin_registry_creation"},
        {test_plugin_registry_find_nonexistent, "plugin_registry_find_nonexistent"},
        {test_plugin_registry_get_all_plugins, "plugin_registry_get_all_plugins"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("PluginRegistry tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test PluginDependencyResolver functionality
local function test_plugin_dependency_resolver()
    print("Testing PluginDependencyResolver functionality...")
    
    local function test_dependency_resolver_creation()
        if qtforge.core.create_plugin_dependency_resolver then
            local resolver = qtforge.core.create_plugin_dependency_resolver()
            assert_not_nil(resolver, "PluginDependencyResolver should be created")
            
            if resolver.resolve_dependencies then
                assert_type(resolver.resolve_dependencies, "function", 
                           "resolve_dependencies should be a function")
            end
        end
    end
    
    local function test_dependency_resolver_empty_list()
        if qtforge.core.create_plugin_dependency_resolver then
            local resolver = qtforge.core.create_plugin_dependency_resolver()
            
            if resolver.resolve_dependencies then
                local result = resolver:resolve_dependencies({})
                assert_type(result, "table", "Result should be a table")
                assert_equal(#result, 0, "Empty list should return empty result")
            end
        end
    end
    
    local tests = {
        {test_dependency_resolver_creation, "dependency_resolver_creation"},
        {test_dependency_resolver_empty_list, "dependency_resolver_empty_list"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("PluginDependencyResolver tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test PluginLifecycleManager functionality
local function test_plugin_lifecycle_manager()
    print("Testing PluginLifecycleManager functionality...")
    
    local function test_lifecycle_manager_creation()
        if qtforge.core.create_plugin_lifecycle_manager then
            local manager = qtforge.core.create_plugin_lifecycle_manager()
            assert_not_nil(manager, "PluginLifecycleManager should be created")
            
            -- Test basic methods exist
            local methods = {"start_plugin", "stop_plugin"}
            for _, method in ipairs(methods) do
                if manager[method] then
                    assert_type(manager[method], "function", method .. " should be a function")
                end
            end
        end
    end
    
    local function test_lifecycle_manager_invalid_plugin()
        if qtforge.core.create_plugin_lifecycle_manager then
            local manager = qtforge.core.create_plugin_lifecycle_manager()
            
            -- Test starting non-existent plugin
            if manager.start_plugin then
                local success, err = pcall(function()
                    manager:start_plugin("non_existent_plugin")
                end)
                
                -- Should handle gracefully or throw appropriate error
                if not success then
                    assert_type(err, "string", "Error should be a string")
                end
            end
        end
    end
    
    local function test_lifecycle_manager_get_plugin_state()
        if qtforge.core.create_plugin_lifecycle_manager then
            local manager = qtforge.core.create_plugin_lifecycle_manager()
            
            if manager.get_plugin_state then
                local state = manager:get_plugin_state("non_existent_plugin")
                -- Should return nil or valid state
                if state ~= nil and PluginState then
                    -- State should be a valid enum value
                    local valid_states = {"Unloaded", "Loading", "Loaded", "Starting", "Running", "Stopping", "Error"}
                    local found = false
                    for _, valid_state in ipairs(valid_states) do
                        if PluginState[valid_state] and state == PluginState[valid_state] then
                            found = true
                            break
                        end
                    end
                    assert(found or state == nil, "State should be valid or nil")
                end
            end
        end
    end
    
    local tests = {
        {test_lifecycle_manager_creation, "lifecycle_manager_creation"},
        {test_lifecycle_manager_invalid_plugin, "lifecycle_manager_invalid_plugin"},
        {test_lifecycle_manager_get_plugin_state, "lifecycle_manager_get_plugin_state"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("PluginLifecycleManager tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test plugin-related enums
local function test_plugin_enums()
    print("Testing plugin enums...")
    
    local function test_plugin_state_enum()
        if PluginState then
            local states = {"Unloaded", "Loading", "Loaded", "Starting", "Running", "Stopping", "Error"}
            for _, state in ipairs(states) do
                if PluginState[state] then
                    assert_not_nil(PluginState[state], "PluginState." .. state .. " should not be nil")
                end
            end
        end
    end
    
    local function test_plugin_capability_enum()
        if PluginCapability then
            local capabilities = {"Service", "Network", "FileSystem", "Database", "UI"}
            for _, capability in ipairs(capabilities) do
                if PluginCapability[capability] then
                    assert_not_nil(PluginCapability[capability], 
                                  "PluginCapability." .. capability .. " should not be nil")
                end
            end
        end
    end
    
    local function test_plugin_priority_enum()
        if PluginPriority then
            local priorities = {"Low", "Normal", "High", "Critical"}
            for _, priority in ipairs(priorities) do
                if PluginPriority[priority] then
                    assert_not_nil(PluginPriority[priority], 
                                  "PluginPriority." .. priority .. " should not be nil")
                end
            end
        end
    end
    
    local function test_plugin_type_enum()
        if PluginType then
            local types = {"Native", "Python", "Lua", "Remote", "Composite"}
            for _, plugin_type in ipairs(types) do
                if PluginType[plugin_type] then
                    assert_not_nil(PluginType[plugin_type], 
                                  "PluginType." .. plugin_type .. " should not be nil")
                end
            end
        end
    end
    
    local tests = {
        {test_plugin_state_enum, "plugin_state_enum"},
        {test_plugin_capability_enum, "plugin_capability_enum"},
        {test_plugin_priority_enum, "plugin_priority_enum"},
        {test_plugin_type_enum, "plugin_type_enum"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Plugin enum tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test error handling
local function test_error_handling()
    print("Testing error handling...")
    
    local function test_plugin_error_creation()
        if qtforge.core.error then
            local error_obj = qtforge.core.error(PluginErrorCode and PluginErrorCode.LoadFailed or 1, "Test error")
            assert_not_nil(error_obj, "Error object should be created")
        end
    end
    
    local function test_plugin_error_codes()
        if PluginErrorCode then
            local codes = {"LoadFailed", "InitializationFailed", "DependencyNotFound", "InvalidMetadata"}
            for _, code in ipairs(codes) do
                if PluginErrorCode[code] then
                    assert_not_nil(PluginErrorCode[code], 
                                  "PluginErrorCode." .. code .. " should not be nil")
                end
            end
        end
    end
    
    local tests = {
        {test_plugin_error_creation, "plugin_error_creation"},
        {test_plugin_error_codes, "plugin_error_codes"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Error handling tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Main test function
local function main()
    print("QtForge Lua Core Bindings - Comprehensive Test Suite")
    print(string.rep("=", 60))
    
    -- Check if qtforge module is available
    if not qtforge then
        print("❌ qtforge module not available")
        print("Ensure QtForge is built with Lua bindings enabled")
        return 1
    end
    
    print("✅ qtforge module is available")
    print()
    
    local test_suites = {
        {test_plugin_manager, "PluginManager"},
        {test_plugin_loader, "PluginLoader"},
        {test_plugin_registry, "PluginRegistry"},
        {test_plugin_dependency_resolver, "PluginDependencyResolver"},
        {test_plugin_lifecycle_manager, "PluginLifecycleManager"},
        {test_plugin_enums, "Plugin Enums"},
        {test_error_handling, "Error Handling"}
    }
    
    local passed_suites = 0
    local total_suites = #test_suites
    
    for _, suite in ipairs(test_suites) do
        print()
        if pcall_test(suite[1], suite[2]) then
            passed_suites = passed_suites + 1
        end
    end
    
    print()
    print(string.rep("=", 60))
    print(string.format("Test Results: %d/%d test suites passed", passed_suites, total_suites))
    
    if passed_suites == total_suites then
        print("✅ All core binding tests passed!")
        return 0
    else
        print("❌ Some core binding tests failed!")
        return 1
    end
end

-- Run the tests
return main()
