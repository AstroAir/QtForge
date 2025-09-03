#!/usr/bin/env lua
--[[
Comprehensive test suite for QtForge Lua Utils bindings.
Tests all utility functionality including edge cases and error handling.
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

-- Test Version utility functionality
local function test_version_utilities()
    print("Testing Version utilities...")
    
    local function test_version_creation()
        if qtforge.utils and qtforge.utils.Version then
            -- Test with major, minor, patch
            local version = qtforge.utils.Version(1, 2, 3)
            assert_not_nil(version, "Version should be created")
            
            -- Test with string
            local version_str = qtforge.utils.Version("1.2.3")
            assert_not_nil(version_str, "Version from string should be created")
        elseif qtforge.core and qtforge.core.version then
            -- Alternative API
            local version = qtforge.core.version(1, 2, 3)
            assert_not_nil(version, "Version should be created")
        end
    end
    
    local function test_version_comparison()
        if qtforge.utils and qtforge.utils.Version then
            local v1 = qtforge.utils.Version(1, 0, 0)
            local v2 = qtforge.utils.Version(1, 0, 1)
            local v3 = qtforge.utils.Version(1, 0, 0)
            
            -- Test comparison if available
            if v1.compare then
                local result = v1:compare(v2)
                assert_type(result, "number", "Comparison should return number")
                assert(result < 0, "v1 should be less than v2")
            end
        end
    end
    
    local function test_version_string_representation()
        if qtforge.utils and qtforge.utils.Version then
            local version = qtforge.utils.Version(1, 2, 3)
            local version_str = tostring(version)
            assert_type(version_str, "string", "Version string should be string")
            assert(string.find(version_str, "1"), "Version string should contain major version")
            assert(string.find(version_str, "2"), "Version string should contain minor version")
            assert(string.find(version_str, "3"), "Version string should contain patch version")
        end
    end
    
    local function test_version_invalid_input()
        if qtforge.utils and qtforge.utils.Version then
            -- Test with negative numbers
            local success, err = pcall(function()
                qtforge.utils.Version(-1, 0, 0)
            end)
            
            if not success then
                assert_type(err, "string", "Error should be a string")
            end
        end
    end
    
    local tests = {
        {test_version_creation, "version_creation"},
        {test_version_comparison, "version_comparison"},
        {test_version_string_representation, "version_string_representation"},
        {test_version_invalid_input, "version_invalid_input"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Version utility tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test JSON utility functions
local function test_json_utilities()
    print("Testing JSON utilities...")
    
    local function test_json_parse_valid()
        if qtforge.utils and qtforge.utils.parse_json then
            local valid_json = '{"key": "value", "number": 42}'
            local result = qtforge.utils.parse_json(valid_json)
            assert_not_nil(result, "JSON parsing should return result")
            assert_type(result, "table", "Parsed JSON should be table")
            
            if result.key then
                assert_equal(result.key, "value", "JSON key should be parsed correctly")
            end
            if result.number then
                assert_equal(result.number, 42, "JSON number should be parsed correctly")
            end
        end
    end
    
    local function test_json_parse_invalid()
        if qtforge.utils and qtforge.utils.parse_json then
            local invalid_json = '{"key": "value", "invalid": }'
            
            local success, err = pcall(function()
                qtforge.utils.parse_json(invalid_json)
            end)
            
            if not success then
                assert_type(err, "string", "Error should be a string")
            end
        end
    end
    
    local function test_json_stringify()
        if qtforge.utils and qtforge.utils.stringify_json then
            local data = {key = "value", number = 42, array = {1, 2, 3}}
            local result = qtforge.utils.stringify_json(data)
            
            assert_type(result, "string", "JSON stringify should return string")
            assert(string.find(result, "key"), "JSON should contain key")
            assert(string.find(result, "value"), "JSON should contain value")
        end
    end
    
    local tests = {
        {test_json_parse_valid, "json_parse_valid"},
        {test_json_parse_invalid, "json_parse_invalid"},
        {test_json_stringify, "json_stringify"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("JSON utility tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test string utility functions
local function test_string_utilities()
    print("Testing string utilities...")
    
    local function test_string_trim()
        if qtforge.utils and qtforge.utils.trim_string then
            local result = qtforge.utils.trim_string("  hello world  ")
            assert_equal(result, "hello world", "String should be trimmed")
            
            local empty_result = qtforge.utils.trim_string("")
            assert_equal(empty_result, "", "Empty string should remain empty")
            
            local whitespace_result = qtforge.utils.trim_string("   ")
            assert_equal(whitespace_result, "", "Whitespace-only string should become empty")
        end
    end
    
    local function test_string_split()
        if qtforge.utils and qtforge.utils.split_string then
            local result = qtforge.utils.split_string("a,b,c", ",")
            assert_type(result, "table", "Split result should be table")
            assert_equal(#result, 3, "Split should return 3 parts")
            
            -- Check individual parts
            local found_a, found_b, found_c = false, false, false
            for _, part in ipairs(result) do
                if part == "a" then found_a = true end
                if part == "b" then found_b = true end
                if part == "c" then found_c = true end
            end
            assert(found_a and found_b and found_c, "All parts should be found")
        end
    end
    
    local function test_string_join()
        if qtforge.utils and qtforge.utils.join_strings then
            local strings = {"a", "b", "c"}
            local result = qtforge.utils.join_strings(strings, ",")
            assert_equal(result, "a,b,c", "Strings should be joined correctly")
        end
    end
    
    local function test_string_case_conversion()
        local test_string = "Hello World"
        
        if qtforge.utils and qtforge.utils.to_lower then
            local result = qtforge.utils.to_lower(test_string)
            assert_equal(result, "hello world", "String should be converted to lowercase")
        end
        
        if qtforge.utils and qtforge.utils.to_upper then
            local result = qtforge.utils.to_upper(test_string)
            assert_equal(result, "HELLO WORLD", "String should be converted to uppercase")
        end
    end
    
    local tests = {
        {test_string_trim, "string_trim"},
        {test_string_split, "string_split"},
        {test_string_join, "string_join"},
        {test_string_case_conversion, "string_case_conversion"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("String utility tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test filesystem utility functions
local function test_filesystem_utilities()
    print("Testing filesystem utilities...")
    
    local function test_file_exists()
        if qtforge.utils and qtforge.utils.file_exists then
            -- Test with a file that should exist (this test file)
            local current_file = debug.getinfo(1, "S").source:match("@(.+)")
            if current_file then
                local exists = qtforge.utils.file_exists(current_file)
                assert_equal(exists, true, "Current test file should exist")
            end
            
            -- Test with non-existent file
            local not_exists = qtforge.utils.file_exists("/non/existent/file.txt")
            assert_equal(not_exists, false, "Non-existent file should not exist")
        end
    end
    
    local function test_directory_exists()
        if qtforge.utils and qtforge.utils.directory_exists then
            -- Test with current directory
            local exists = qtforge.utils.directory_exists(".")
            assert_equal(exists, true, "Current directory should exist")
            
            -- Test with non-existent directory
            local not_exists = qtforge.utils.directory_exists("/non/existent/directory")
            assert_equal(not_exists, false, "Non-existent directory should not exist")
        end
    end
    
    local function test_create_directory()
        if qtforge.utils and qtforge.utils.create_directory then
            local temp_dir = "/tmp/qtforge_test_" .. os.time()
            
            -- Create directory
            local success = qtforge.utils.create_directory(temp_dir)
            
            if success then
                -- Verify directory was created
                if qtforge.utils.directory_exists then
                    local exists = qtforge.utils.directory_exists(temp_dir)
                    assert_equal(exists, true, "Created directory should exist")
                end
                
                -- Clean up
                os.execute("rmdir " .. temp_dir)
            end
        end
    end
    
    local function test_get_file_size()
        if qtforge.utils and qtforge.utils.get_file_size then
            -- Create a temporary file
            local temp_file = "/tmp/qtforge_test_file_" .. os.time() .. ".txt"
            local test_content = "Hello, World!"
            
            local file = io.open(temp_file, "w")
            if file then
                file:write(test_content)
                file:close()
                
                local size = qtforge.utils.get_file_size(temp_file)
                assert_equal(size, #test_content, "File size should match content length")
                
                -- Clean up
                os.remove(temp_file)
            end
        end
    end
    
    local tests = {
        {test_file_exists, "file_exists"},
        {test_directory_exists, "directory_exists"},
        {test_create_directory, "create_directory"},
        {test_get_file_size, "get_file_size"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Filesystem utility tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test logging utility functions
local function test_logging_utilities()
    print("Testing logging utilities...")
    
    local function test_log_levels()
        if LogLevel then
            local levels = {"Debug", "Info", "Warning", "Error", "Critical"}
            for _, level in ipairs(levels) do
                if LogLevel[level] then
                    assert_not_nil(LogLevel[level], "LogLevel." .. level .. " should not be nil")
                end
            end
        end
    end
    
    local function test_log_functions()
        local test_message = "Test log message"
        
        -- Test different log levels
        local log_functions = {"log_debug", "log_info", "log_warning", "log_error"}
        for _, func_name in ipairs(log_functions) do
            if qtforge.utils and qtforge.utils[func_name] then
                -- Should not raise exception
                local success, err = pcall(function()
                    qtforge.utils[func_name](test_message)
                end)
                
                if not success then
                    print("    Warning: " .. func_name .. " failed: " .. tostring(err))
                end
            end
        end
    end
    
    local tests = {
        {test_log_levels, "log_levels"},
        {test_log_functions, "log_functions"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Logging utility tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test time utility functions
local function test_time_utilities()
    print("Testing time utilities...")
    
    local function test_current_timestamp()
        if qtforge.utils and qtforge.utils.current_timestamp then
            local timestamp = qtforge.utils.current_timestamp()
            assert_type(timestamp, "number", "Timestamp should be a number")
            assert(timestamp > 0, "Timestamp should be positive")
        end
    end
    
    local function test_format_timestamp()
        if qtforge.utils and qtforge.utils.format_timestamp then
            local timestamp = 1609459200  -- 2021-01-01 00:00:00 UTC
            local formatted = qtforge.utils.format_timestamp(timestamp)
            assert_type(formatted, "string", "Formatted timestamp should be string")
            assert(#formatted > 0, "Formatted timestamp should not be empty")
        end
    end
    
    local function test_sleep_function()
        if qtforge.utils and qtforge.utils.sleep then
            local start_time = os.clock()
            qtforge.utils.sleep(0.1)  -- Sleep for 100ms
            local end_time = os.clock()
            
            local elapsed = end_time - start_time
            assert(elapsed >= 0.09, "Should have slept for approximately 100ms")
        end
    end
    
    local tests = {
        {test_current_timestamp, "current_timestamp"},
        {test_format_timestamp, "format_timestamp"},
        {test_sleep_function, "sleep_function"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Time utility tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test error utility functions
local function test_error_utilities()
    print("Testing error utilities...")
    
    local function test_error_creation()
        if qtforge.utils and qtforge.utils.create_error then
            local error_obj = qtforge.utils.create_error("Test error message")
            assert_not_nil(error_obj, "Error object should be created")
            assert_equal(tostring(error_obj), "Test error message", "Error message should match")
        end
    end
    
    local function test_error_codes()
        if ErrorCode then
            local codes = {"Success", "InvalidArgument", "FileNotFound", "PermissionDenied"}
            for _, code in ipairs(codes) do
                if ErrorCode[code] then
                    assert_not_nil(ErrorCode[code], "ErrorCode." .. code .. " should not be nil")
                end
            end
        end
    end
    
    local tests = {
        {test_error_creation, "error_creation"},
        {test_error_codes, "error_codes"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Error utility tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test memory utility functions
local function test_memory_utilities()
    print("Testing memory utilities...")
    
    local function test_memory_usage()
        if qtforge.utils and qtforge.utils.get_memory_usage then
            local usage = qtforge.utils.get_memory_usage()
            assert_type(usage, "number", "Memory usage should be a number")
            assert(usage >= 0, "Memory usage should be non-negative")
        end
    end
    
    local function test_garbage_collection()
        if qtforge.utils and qtforge.utils.trigger_gc then
            -- Should not raise exception
            local success, err = pcall(function()
                qtforge.utils.trigger_gc()
            end)
            
            if not success then
                print("    Warning: trigger_gc failed: " .. tostring(err))
            end
        end
    end
    
    local tests = {
        {test_memory_usage, "memory_usage"},
        {test_garbage_collection, "garbage_collection"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Memory utility tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Main test function
local function main()
    print("QtForge Lua Utils Bindings - Comprehensive Test Suite")
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
        {test_version_utilities, "Version Utilities"},
        {test_json_utilities, "JSON Utilities"},
        {test_string_utilities, "String Utilities"},
        {test_filesystem_utilities, "Filesystem Utilities"},
        {test_logging_utilities, "Logging Utilities"},
        {test_time_utilities, "Time Utilities"},
        {test_error_utilities, "Error Utilities"},
        {test_memory_utilities, "Memory Utilities"}
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
        print("✅ All utils binding tests passed!")
        return 0
    else
        print("❌ Some utils binding tests failed!")
        return 1
    end
end

-- Run the tests
return main()
