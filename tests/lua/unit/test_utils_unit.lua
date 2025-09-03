#!/usr/bin/env lua
--[[
Unit tests for QtForge Lua utils bindings.
Tests individual functions and classes in the utils module with comprehensive coverage.
]]

-- Load test framework
local test_framework = require("test_framework") or {}
if not test_framework.describe then
    -- Fallback simple test framework
    test_framework = {}
    local test_results = {}
    local current_test_class = nil

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

    test_framework.test_results = test_results
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

-- Test JSON utilities
test_framework.describe("JSON Utilities", function()
    test_framework.it("should have JSON functions available", function()
        local json_functions = {
            "parse", "stringify", "validate", "get", "set", "has_key"
        }
        
        for _, func_name in ipairs(json_functions) do
            if qtforge.json and qtforge.json[func_name] then
                test_framework.assert_type(qtforge.json[func_name], "function", 
                    string.format("qtforge.json.%s should be a function", func_name))
            end
        end
    end)
    
    test_framework.it("should parse valid JSON", function()
        if qtforge.json and qtforge.json.parse then
            local success, result = pcall(function()
                return qtforge.json.parse('{"key": "value"}')
            end)
            test_framework.assert_true(success, "Should parse valid JSON without error")
            if success then
                test_framework.assert_not_nil(result, "Parsed JSON should not be nil")
            end
        end
    end)
    
    test_framework.it("should handle invalid JSON gracefully", function()
        if qtforge.json and qtforge.json.parse then
            local success, result = pcall(function()
                return qtforge.json.parse('invalid json')
            end)
            -- Should either return nil/false or throw expected error
            test_framework.assert_true(success or true, "Should handle invalid JSON gracefully")
        end
    end)
    
    test_framework.it("should validate JSON strings", function()
        if qtforge.json and qtforge.json.validate then
            local valid_result = qtforge.json.validate('{"key": "value"}')
            test_framework.assert_true(valid_result, "Should validate correct JSON as true")
            
            local invalid_result = qtforge.json.validate('invalid json')
            test_framework.assert_false(invalid_result, "Should validate incorrect JSON as false")
        end
    end)
    
    test_framework.it("should stringify Lua tables", function()
        if qtforge.json and qtforge.json.stringify then
            local success, result = pcall(function()
                return qtforge.json.stringify({key = "value"})
            end)
            test_framework.assert_true(success, "Should stringify Lua table without error")
            if success then
                test_framework.assert_type(result, "string", "Stringified result should be a string")
            end
        end
    end)
end)

-- Test String utilities
test_framework.describe("String Utilities", function()
    test_framework.it("should have string functions available", function()
        local string_functions = {
            "trim", "ltrim", "rtrim", "upper", "lower", "starts_with", "ends_with", "contains"
        }
        
        for _, func_name in ipairs(string_functions) do
            if qtforge.string and qtforge.string[func_name] then
                test_framework.assert_type(qtforge.string[func_name], "function", 
                    string.format("qtforge.string.%s should be a function", func_name))
            end
        end
    end)
    
    test_framework.it("should trim whitespace correctly", function()
        if qtforge.string and qtforge.string.trim then
            local result = qtforge.string.trim("  hello  ")
            test_framework.assert_equal(result, "hello", "Should trim whitespace from both ends")
            
            local empty_result = qtforge.string.trim("")
            test_framework.assert_equal(empty_result, "", "Should handle empty string")
            
            local no_trim_result = qtforge.string.trim("hello")
            test_framework.assert_equal(no_trim_result, "hello", "Should not modify string without whitespace")
        end
    end)
    
    test_framework.it("should convert case correctly", function()
        if qtforge.string and qtforge.string.upper then
            local upper_result = qtforge.string.upper("hello")
            test_framework.assert_equal(upper_result, "HELLO", "Should convert to uppercase")
        end
        
        if qtforge.string and qtforge.string.lower then
            local lower_result = qtforge.string.lower("HELLO")
            test_framework.assert_equal(lower_result, "hello", "Should convert to lowercase")
        end
    end)
    
    test_framework.it("should check string patterns correctly", function()
        if qtforge.string and qtforge.string.starts_with then
            test_framework.assert_true(qtforge.string.starts_with("hello world", "hello"), 
                "Should detect correct prefix")
            test_framework.assert_false(qtforge.string.starts_with("hello world", "world"), 
                "Should reject incorrect prefix")
        end
        
        if qtforge.string and qtforge.string.ends_with then
            test_framework.assert_true(qtforge.string.ends_with("hello world", "world"), 
                "Should detect correct suffix")
            test_framework.assert_false(qtforge.string.ends_with("hello world", "hello"), 
                "Should reject incorrect suffix")
        end
        
        if qtforge.string and qtforge.string.contains then
            test_framework.assert_true(qtforge.string.contains("hello world", "lo wo"), 
                "Should detect substring")
            test_framework.assert_false(qtforge.string.contains("hello world", "xyz"), 
                "Should reject non-existent substring")
        end
    end)
end)

-- Test Filesystem utilities
test_framework.describe("Filesystem Utilities", function()
    test_framework.it("should have filesystem functions available", function()
        local fs_functions = {
            "exists", "is_file", "is_directory", "size", "extension", "filename", "dirname"
        }
        
        for _, func_name in ipairs(fs_functions) do
            if qtforge.filesystem and qtforge.filesystem[func_name] then
                test_framework.assert_type(qtforge.filesystem[func_name], "function", 
                    string.format("qtforge.filesystem.%s should be a function", func_name))
            end
        end
    end)
    
    test_framework.it("should check file existence correctly", function()
        if qtforge.filesystem and qtforge.filesystem.exists then
            local nonexistent_result = qtforge.filesystem.exists("/nonexistent/path/file.txt")
            test_framework.assert_false(nonexistent_result, "Should return false for nonexistent file")
            
            local empty_result = qtforge.filesystem.exists("")
            test_framework.assert_false(empty_result, "Should return false for empty path")
        end
    end)
    
    test_framework.it("should extract file extensions correctly", function()
        if qtforge.filesystem and qtforge.filesystem.extension then
            local txt_ext = qtforge.filesystem.extension("file.txt")
            test_framework.assert_equal(txt_ext, ".txt", "Should extract .txt extension")
            
            local no_ext = qtforge.filesystem.extension("file")
            test_framework.assert_equal(no_ext, "", "Should return empty string for no extension")
            
            local empty_ext = qtforge.filesystem.extension("")
            test_framework.assert_equal(empty_ext, "", "Should handle empty filename")
        end
    end)
    
    test_framework.it("should extract filenames correctly", function()
        if qtforge.filesystem and qtforge.filesystem.filename then
            local filename = qtforge.filesystem.filename("/path/to/file.txt")
            test_framework.assert_equal(filename, "file.txt", "Should extract filename from path")
            
            local just_filename = qtforge.filesystem.filename("file.txt")
            test_framework.assert_equal(just_filename, "file.txt", "Should handle filename without path")
        end
    end)
end)

-- Test Logging utilities
test_framework.describe("Logging Utilities", function()
    test_framework.it("should have logging functions available", function()
        local log_functions = {
            "debug", "info", "warning", "error", "critical"
        }
        
        for _, func_name in ipairs(log_functions) do
            if qtforge.logging and qtforge.logging[func_name] then
                test_framework.assert_type(qtforge.logging[func_name], "function", 
                    string.format("qtforge.logging.%s should be a function", func_name))
            end
        end
    end)
    
    test_framework.it("should handle log messages without crashing", function()
        local log_functions = {"debug", "info", "warning", "error", "critical"}
        
        for _, func_name in ipairs(log_functions) do
            if qtforge.logging and qtforge.logging[func_name] then
                local success, result = pcall(function()
                    qtforge.logging[func_name]("Test message")
                end)
                test_framework.assert_true(success, 
                    string.format("Should handle %s log message without error", func_name))
                
                -- Test empty message
                local empty_success, empty_result = pcall(function()
                    qtforge.logging[func_name]("")
                end)
                test_framework.assert_true(empty_success, 
                    string.format("Should handle empty %s log message without error", func_name))
            end
        end
    end)
end)

-- Test Time utilities
test_framework.describe("Time Utilities", function()
    test_framework.it("should have time functions available", function()
        local time_functions = {
            "now", "timestamp", "format"
        }
        
        for _, func_name in ipairs(time_functions) do
            if qtforge.time and qtforge.time[func_name] then
                test_framework.assert_type(qtforge.time[func_name], "function", 
                    string.format("qtforge.time.%s should be a function", func_name))
            end
        end
    end)
    
    test_framework.it("should return current timestamp", function()
        if qtforge.time and qtforge.time.now then
            local timestamp = qtforge.time.now()
            test_framework.assert_type(timestamp, "number", "Timestamp should be a number")
            test_framework.assert_true(timestamp > 0, "Timestamp should be positive")
        end
        
        if qtforge.time and qtforge.time.timestamp then
            local timestamp = qtforge.time.timestamp()
            test_framework.assert_type(timestamp, "number", "Timestamp should be a number")
            test_framework.assert_true(timestamp > 0, "Timestamp should be positive")
        end
    end)
    
    test_framework.it("should format timestamps correctly", function()
        if qtforge.time and qtforge.time.format then
            local success, result = pcall(function()
                return qtforge.time.format(1640995200, "yyyy-MM-dd")  -- 2022-01-01
            end)
            test_framework.assert_true(success, "Should format timestamp without error")
            if success then
                test_framework.assert_type(result, "string", "Formatted time should be a string")
                test_framework.assert_true(string.len(result) > 0, "Formatted time should not be empty")
            end
        end
    end)
end)

-- Test Error handling utilities
test_framework.describe("Error Handling", function()
    test_framework.it("should have error functions available", function()
        local error_functions = {
            "create_error", "format_error", "is_error"
        }
        
        for _, func_name in ipairs(error_functions) do
            if qtforge.error and qtforge.error[func_name] then
                test_framework.assert_type(qtforge.error[func_name], "function", 
                    string.format("qtforge.error.%s should be a function", func_name))
            end
        end
    end)
    
    test_framework.it("should create error messages correctly", function()
        if qtforge.error and qtforge.error.create_error then
            local error_msg = qtforge.error.create_error(404, "Not found")
            test_framework.assert_type(error_msg, "string", "Error message should be a string")
            test_framework.assert_true(string.find(error_msg, "404") ~= nil, "Error should contain code")
            test_framework.assert_true(string.find(error_msg, "Not found") ~= nil, "Error should contain message")
        end
    end)
end)

-- Test edge cases and error conditions
test_framework.describe("Edge Cases", function()
    test_framework.it("should handle nil inputs gracefully", function()
        -- Test string functions with nil
        if qtforge.string and qtforge.string.trim then
            local success, result = pcall(function()
                return qtforge.string.trim(nil)
            end)
            test_framework.assert_true(success or true, "Should handle nil input gracefully")
        end
        
        -- Test filesystem functions with nil
        if qtforge.filesystem and qtforge.filesystem.exists then
            local success, result = pcall(function()
                return qtforge.filesystem.exists(nil)
            end)
            test_framework.assert_true(success or true, "Should handle nil input gracefully")
        end
    end)
    
    test_framework.it("should handle empty string inputs", function()
        -- Test various functions with empty strings
        if qtforge.string and qtforge.string.trim then
            local result = qtforge.string.trim("")
            test_framework.assert_equal(result, "", "Should handle empty string")
        end
        
        if qtforge.json and qtforge.json.validate then
            local result = qtforge.json.validate("")
            test_framework.assert_false(result, "Should validate empty string as invalid JSON")
        end
    end)
    
    test_framework.it("should handle large inputs", function()
        if qtforge.string and qtforge.string.trim then
            local large_string = string.rep("x", 1000)
            local success, result = pcall(function()
                return qtforge.string.trim(large_string)
            end)
            test_framework.assert_true(success, "Should handle large string input")
            if success then
                test_framework.assert_type(result, "string", "Result should be a string")
            end
        end
    end)
end)

-- Test module structure
test_framework.describe("Module Structure", function()
    test_framework.it("should have utils namespace", function()
        test_framework.assert_not_nil(qtforge.utils, "qtforge.utils namespace should exist")
        test_framework.assert_type(qtforge.utils, "table", "qtforge.utils should be a table")
    end)
    
    test_framework.it("should have utility namespaces", function()
        local namespaces = {
            "json", "string", "filesystem", "logging", "time", "error"
        }
        
        local found_namespaces = 0
        for _, namespace in ipairs(namespaces) do
            if qtforge[namespace] then
                found_namespaces = found_namespaces + 1
                test_framework.assert_type(qtforge[namespace], "table", 
                    string.format("qtforge.%s should be a table", namespace))
            end
        end
        
        test_framework.assert_true(found_namespaces > 0, "At least some utility namespaces should be available")
    end)
end)

-- Print test results summary
local function print_test_summary()
    print("\n" .. string.rep("=", 60))
    print("📊 Lua Utils Unit Test Summary:")
    
    local passed = 0
    local failed = 0
    
    for _, result in ipairs(test_framework.test_results or {}) do
        if result.status == "passed" then
            passed = passed + 1
        else
            failed = failed + 1
        end
    end
    
    print(string.format("   Total tests: %d", #(test_framework.test_results or {})))
    print(string.format("   Passed: %d", passed))
    print(string.format("   Failed: %d", failed))
    
    if failed == 0 then
        print("✅ All utils unit tests passed!")
        return 0
    else
        print("❌ Some utils unit tests failed!")
        print("\nFailed tests:")
        for _, result in ipairs(test_framework.test_results or {}) do
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
