#!/usr/bin/env lua
--[[
Advanced Lua Test Suite for QtForge
Tests all available Lua bindings and functionality
]]

-- Test configuration
local test_config = {
    verbose = true,
    stop_on_error = false,
    timeout_seconds = 30
}

-- Test results tracking
local test_results = {
    total = 0,
    passed = 0,
    failed = 0,
    skipped = 0,
    errors = {}
}

-- Utility functions
local function log(message)
    if test_config.verbose then
        print("[LOG] " .. message)
    end
end

local function test_assert(condition, message)
    test_results.total = test_results.total + 1
    if condition then
        test_results.passed = test_results.passed + 1
        log("PASS: " .. message)
        return true
    else
        test_results.failed = test_results.failed + 1
        table.insert(test_results.errors, message)
        print("[FAIL] " .. message)
        if test_config.stop_on_error then
            error("Test failed: " .. message)
        end
        return false
    end
end

local function test_skip(message)
    test_results.total = test_results.total + 1
    test_results.skipped = test_results.skipped + 1
    log("SKIP: " .. message)
end

-- Test basic Lua functionality
local function test_lua_basics()
    print("\n=== Testing Basic Lua Functionality ===")

    -- Test basic types
    test_assert(type(42) == "number", "Number type recognition")
    test_assert(type("hello") == "string", "String type recognition")
    test_assert(type(true) == "boolean", "Boolean type recognition")
    test_assert(type({}) == "table", "Table type recognition")
    test_assert(type(function() end) == "function", "Function type recognition")

    -- Test basic operations
    test_assert(2 + 3 == 5, "Basic arithmetic")
    test_assert("hello" .. " world" == "hello world", "String concatenation")

    -- Test table operations
    local t = {1, 2, 3}
    test_assert(#t == 3, "Table length")
    test_assert(t[1] == 1, "Table indexing")

    log("Basic Lua functionality tests completed")
end

-- Test QtForge module availability
local function test_qtforge_availability()
    print("\n=== Testing QtForge Module Availability ===")

    if qtforge then
        test_assert(qtforge ~= nil, "QtForge module is available")

        -- Test version information
        if qtforge.version then
            test_assert(type(qtforge.version) == "string", "QtForge version is string")
            log("QtForge version: " .. tostring(qtforge.version))
        else
            test_skip("QtForge version not available")
        end

        -- Test version components
        if qtforge.version_major then
            test_assert(type(qtforge.version_major) == "number", "Version major is number")
        end
        if qtforge.version_minor then
            test_assert(type(qtforge.version_minor) == "number", "Version minor is number")
        end
        if qtforge.version_patch then
            test_assert(type(qtforge.version_patch) == "number", "Version patch is number")
        end

    else
        test_skip("QtForge module not available")
    end
end

-- Test QtForge core module
local function test_qtforge_core()
    print("\n=== Testing QtForge Core Module ===")

    if not qtforge then
        test_skip("QtForge not available - skipping core tests")
        return
    end

    if qtforge.core then
        test_assert(qtforge.core ~= nil, "QtForge core module is available")

        -- Test core functions
        if qtforge.core.test_function then
            local result = qtforge.core.test_function()
            test_assert(result ~= nil, "Core test function returns result")
        else
            test_skip("Core test function not available")
        end

        if qtforge.core.add then
            local result = qtforge.core.add(2, 3)
            test_assert(result == 5, "Core add function works correctly")
        else
            test_skip("Core add function not available")
        end

    else
        test_skip("QtForge core module not available")
    end
end

-- Test QtForge utils module
local function test_qtforge_utils()
    print("\n=== Testing QtForge Utils Module ===")

    if not qtforge then
        test_skip("QtForge not available - skipping utils tests")
        return
    end

    if qtforge.utils then
        test_assert(qtforge.utils ~= nil, "QtForge utils module is available")

        -- Test utils functions
        if qtforge.utils.utils_test then
            local result = qtforge.utils.utils_test()
            test_assert(result ~= nil, "Utils test function returns result")
        else
            test_skip("Utils test function not available")
        end

        if qtforge.utils.create_version then
            local version = qtforge.utils.create_version(1, 2, 3)
            test_assert(version ~= nil, "Utils create_version function works")
        else
            test_skip("Utils create_version function not available")
        end

    else
        test_skip("QtForge utils module not available")
    end
end

-- Test QtForge logging
local function test_qtforge_logging()
    print("\n=== Testing QtForge Logging ===")

    if not qtforge then
        test_skip("QtForge not available - skipping logging tests")
        return
    end

    if qtforge.log then
        -- Test logging function
        local success = pcall(qtforge.log, "Test log message from Lua")
        test_assert(success, "QtForge logging function works")
    else
        test_skip("QtForge logging function not available")
    end
end

-- Test error handling
local function test_error_handling()
    print("\n=== Testing Error Handling ===")

    -- Test pcall for error handling
    local success, result = pcall(function()
        error("Test error")
    end)
    test_assert(not success, "Error handling with pcall works")
    test_assert(type(result) == "string", "Error message is string")

    -- Test xpcall for error handling
    local function error_handler(err)
        return "Handled: " .. tostring(err)
    end

    local success, result = xpcall(function()
        error("Test error for xpcall")
    end, error_handler)
    test_assert(not success, "Error handling with xpcall works")
    test_assert(string.find(result, "Handled:") == 1, "Custom error handler works")
end

-- Test performance baseline
local function test_performance_baseline()
    print("\n=== Testing Performance Baseline ===")

    local start_time = os.clock()

    -- Simple performance test
    local sum = 0
    for i = 1, 10000 do
        sum = sum + i
    end

    local end_time = os.clock()
    local duration = end_time - start_time

    test_assert(sum == 50005000, "Performance test calculation correct")
    test_assert(duration < 1.0, "Performance test completes in reasonable time")

    log(string.format("Performance test duration: %.3f seconds", duration))
end

-- Main test runner
local function run_all_tests()
    print("QtForge Lua Comprehensive Test Suite")
    print(string.rep("=", 50))

    local start_time = os.clock()

    -- Run all test suites
    test_lua_basics()
    test_qtforge_availability()
    test_qtforge_core()
    test_qtforge_utils()
    test_qtforge_logging()
    test_error_handling()
    test_performance_baseline()

    local end_time = os.clock()
    local total_duration = end_time - start_time

    -- Print results
    print("\n" .. string.rep("=", 50))
    print("Test Results Summary:")
    print(string.format("Total tests: %d", test_results.total))
    print(string.format("Passed: %d", test_results.passed))
    print(string.format("Failed: %d", test_results.failed))
    print(string.format("Skipped: %d", test_results.skipped))
    print(string.format("Duration: %.3f seconds", total_duration))

    if test_results.failed > 0 then
        print("\nFailed tests:")
        for _, error in ipairs(test_results.errors) do
            print("  - " .. error)
        end
    end

    local success_rate = test_results.total > 0 and (test_results.passed / test_results.total * 100) or 0
    print(string.format("Success rate: %.1f%%", success_rate))

    if test_results.failed == 0 then
        print("\nAll tests passed! ✓")
        return 0
    else
        print("\nSome tests failed! ✗")
        return 1
    end
end

-- Run the test suite
return run_all_tests()
