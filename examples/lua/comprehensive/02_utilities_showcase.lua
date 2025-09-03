#!/usr/bin/env lua
--[[
QtForge Lua Bindings Example: Utilities Showcase

This example demonstrates the utility functions available in the QtForge Lua bindings.
It covers:

1. JSON parsing and manipulation
2. String utilities and operations
3. File system operations
4. Logging functionality
5. Time and date utilities
6. Error handling utilities

Prerequisites:
- QtForge built with Lua bindings enabled
- Utils module available
- Lua 5.1 or later
]]

-- Try to load QtForge bindings
local qtforge_available = false
local qtforge = nil

local success, result = pcall(require, "qtforge")
if success then
    qtforge = result
    qtforge_available = true
    print("✅ QtForge Lua bindings loaded successfully")
else
    print(string.format("❌ Failed to load QtForge: %s", result))
    print("Make sure QtForge is built with Lua bindings enabled")
    os.exit(1)
end

-- UtilitiesExample class
local UtilitiesExample = {}
UtilitiesExample.__index = UtilitiesExample

function UtilitiesExample:new()
    local obj = {}
    setmetatable(obj, UtilitiesExample)
    return obj
end

function UtilitiesExample:demonstrate_json_utilities()
    print("\n📄 Demonstrating JSON utilities...")
    
    if not qtforge.json then
        print("⚠️  JSON utilities not available")
        return
    end
    
    -- Test JSON parsing
    local test_json_strings = {
        '{"name": "John", "age": 30, "city": "New York"}',
        '{"numbers": [1, 2, 3, 4, 5], "nested": {"key": "value"}}',
        '{"boolean": true, "null_value": null, "float": 3.14}'
    }
    
    for i, json_str in ipairs(test_json_strings) do
        print(string.format("\nTesting JSON string %d: %s", i, json_str))
        
        -- Test parsing
        if qtforge.json.parse then
            local success, result = pcall(function()
                return qtforge.json.parse(json_str)
            end)
            
            if success then
                print(string.format("✅ Parsed successfully: %s", type(result)))
            else
                print(string.format("❌ Parse error: %s", result))
            end
        end
        
        -- Test validation
        if qtforge.json.validate then
            local is_valid = qtforge.json.validate(json_str)
            print(string.format("Validation result: %s", tostring(is_valid)))
        end
    end
    
    -- Test JSON stringification
    if qtforge.json.stringify then
        local test_tables = {
            {name = "Alice", age = 25},
            {numbers = {1, 2, 3}, text = "hello"},
            {mixed = {string = "test", number = 42, boolean = true}}
        }
        
        print("\nTesting JSON stringification:")
        for i, table_data in ipairs(test_tables) do
            local success, result = pcall(function()
                return qtforge.json.stringify(table_data)
            end)
            
            if success then
                print(string.format("Table %d stringified: %s", i, result))
            else
                print(string.format("Table %d stringify error: %s", i, result))
            end
        end
    end
    
    -- Test invalid JSON
    local invalid_json = '{"invalid": json, "missing": quotes}'
    print(string.format("\nTesting invalid JSON: %s", invalid_json))
    
    if qtforge.json.validate then
        local is_valid = qtforge.json.validate(invalid_json)
        print(string.format("Invalid JSON validation: %s", tostring(is_valid)))
    end
end

function UtilitiesExample:demonstrate_string_utilities()
    print("\n🔤 Demonstrating string utilities...")
    
    if not qtforge.string then
        print("⚠️  String utilities not available")
        return
    end
    
    local test_strings = {
        "  Hello World  ",
        "UPPERCASE TEXT",
        "lowercase text",
        "Mixed Case String"
    }
    
    for _, test_str in ipairs(test_strings) do
        print(string.format("\nTesting string: '%s'", test_str))
        
        -- Test trimming
        if qtforge.string.trim then
            local trimmed = qtforge.string.trim(test_str)
            print(string.format("Trimmed: '%s'", trimmed))
        end
        
        -- Test case conversion
        if qtforge.string.upper then
            local upper = qtforge.string.upper(test_str)
            print(string.format("Uppercase: '%s'", upper))
        end
        
        if qtforge.string.lower then
            local lower = qtforge.string.lower(test_str)
            print(string.format("Lowercase: '%s'", lower))
        end
    end
    
    -- Test string pattern matching
    local pattern_tests = {
        {text = "Hello World", prefix = "Hello", suffix = "World", contains = "lo Wo"},
        {text = "QtForge Framework", prefix = "Qt", suffix = "work", contains = "Forge"},
        {text = "example.txt", prefix = "ex", suffix = ".txt", contains = "ample"}
    }
    
    print("\nTesting string patterns:")
    for i, test in ipairs(pattern_tests) do
        print(string.format("\nPattern test %d: '%s'", i, test.text))
        
        if qtforge.string.starts_with then
            local starts = qtforge.string.starts_with(test.text, test.prefix)
            print(string.format("Starts with '%s': %s", test.prefix, tostring(starts)))
        end
        
        if qtforge.string.ends_with then
            local ends = qtforge.string.ends_with(test.text, test.suffix)
            print(string.format("Ends with '%s': %s", test.suffix, tostring(ends)))
        end
        
        if qtforge.string.contains then
            local contains = qtforge.string.contains(test.text, test.contains)
            print(string.format("Contains '%s': %s", test.contains, tostring(contains)))
        end
    end
end

function UtilitiesExample:demonstrate_filesystem_utilities()
    print("\n📁 Demonstrating filesystem utilities...")
    
    if not qtforge.filesystem then
        print("⚠️  Filesystem utilities not available")
        return
    end
    
    local test_paths = {
        "/path/to/file.txt",
        "relative/path/document.pdf",
        "example.json",
        "/usr/bin/program",
        "C:\\Windows\\System32\\file.dll"
    }
    
    for _, path in ipairs(test_paths) do
        print(string.format("\nTesting path: '%s'", path))
        
        -- Test file existence (will be false for these examples)
        if qtforge.filesystem.exists then
            local exists = qtforge.filesystem.exists(path)
            print(string.format("Exists: %s", tostring(exists)))
        end
        
        -- Test file extension extraction
        if qtforge.filesystem.extension then
            local ext = qtforge.filesystem.extension(path)
            print(string.format("Extension: '%s'", ext))
        end
        
        -- Test filename extraction
        if qtforge.filesystem.filename then
            local filename = qtforge.filesystem.filename(path)
            print(string.format("Filename: '%s'", filename))
        end
        
        -- Test directory extraction
        if qtforge.filesystem.dirname then
            local dirname = qtforge.filesystem.dirname(path)
            print(string.format("Directory: '%s'", dirname))
        end
    end
    
    -- Test file type checking
    print("\nTesting file type checking:")
    local type_test_paths = {".", "..", "/tmp", "/etc/passwd", "nonexistent.txt"}
    
    for _, path in ipairs(type_test_paths) do
        if qtforge.filesystem.is_file then
            local is_file = qtforge.filesystem.is_file(path)
            print(string.format("'%s' is file: %s", path, tostring(is_file)))
        end
        
        if qtforge.filesystem.is_directory then
            local is_dir = qtforge.filesystem.is_directory(path)
            print(string.format("'%s' is directory: %s", path, tostring(is_dir)))
        end
    end
end

function UtilitiesExample:demonstrate_logging_utilities()
    print("\n📝 Demonstrating logging utilities...")
    
    if not qtforge.logging then
        print("⚠️  Logging utilities not available")
        return
    end
    
    local log_levels = {
        {"debug", "This is a debug message"},
        {"info", "This is an info message"},
        {"warning", "This is a warning message"},
        {"error", "This is an error message"},
        {"critical", "This is a critical message"}
    }
    
    print("Testing different log levels:")
    for _, log_data in ipairs(log_levels) do
        local level, message = log_data[1], log_data[2]
        
        if qtforge.logging[level] then
            local success, result = pcall(function()
                qtforge.logging[level](message)
            end)
            
            if success then
                print(string.format("✅ %s log sent", level))
            else
                print(string.format("❌ %s log error: %s", level, result))
            end
        else
            print(string.format("⚠️  %s logging not available", level))
        end
    end
    
    -- Test logging with different message types
    print("\nTesting logging with different message types:")
    local message_types = {
        "Simple string message",
        "",  -- Empty string
        "Message with numbers: 123, 456.789",
        "Message with special chars: !@#$%^&*()"
    }
    
    for i, message in ipairs(message_types) do
        if qtforge.logging.info then
            local success, result = pcall(function()
                qtforge.logging.info(string.format("Test message %d: %s", i, message))
            end)
            
            if success then
                print(string.format("✅ Message %d logged", i))
            else
                print(string.format("❌ Message %d error: %s", i, result))
            end
        end
    end
end

function UtilitiesExample:demonstrate_time_utilities()
    print("\n⏰ Demonstrating time utilities...")
    
    if not qtforge.time then
        print("⚠️  Time utilities not available")
        return
    end
    
    -- Test current timestamp
    if qtforge.time.now then
        local current_time = qtforge.time.now()
        print(string.format("Current timestamp: %s", tostring(current_time)))
    end
    
    if qtforge.time.timestamp then
        local timestamp = qtforge.time.timestamp()
        print(string.format("Timestamp: %s", tostring(timestamp)))
    end
    
    -- Test time formatting
    if qtforge.time.format then
        local test_timestamps = {
            1640995200,  -- 2022-01-01 00:00:00 UTC
            1672531200,  -- 2023-01-01 00:00:00 UTC
            os.time()    -- Current time
        }
        
        local format_strings = {
            "yyyy-MM-dd",
            "yyyy-MM-dd HH:mm:ss",
            "MM/dd/yyyy"
        }
        
        print("\nTesting time formatting:")
        for _, timestamp in ipairs(test_timestamps) do
            for _, format_str in ipairs(format_strings) do
                local success, result = pcall(function()
                    return qtforge.time.format(timestamp, format_str)
                end)
                
                if success then
                    print(string.format("Timestamp %d with format '%s': %s", 
                          timestamp, format_str, result))
                else
                    print(string.format("Format error for %d: %s", timestamp, result))
                end
            end
        end
    end
end

function UtilitiesExample:demonstrate_error_utilities()
    print("\n⚠️  Demonstrating error utilities...")
    
    if not qtforge.error then
        print("⚠️  Error utilities not available")
        return
    end
    
    -- Test error creation
    if qtforge.error.create_error then
        local error_tests = {
            {code = 404, message = "Not Found"},
            {code = 500, message = "Internal Server Error"},
            {code = 0, message = "Success"},
            {code = -1, message = "Unknown Error"}
        }
        
        print("Testing error creation:")
        for _, test in ipairs(error_tests) do
            local success, result = pcall(function()
                return qtforge.error.create_error(test.code, test.message)
            end)
            
            if success then
                print(string.format("Error %d: %s", test.code, result))
            else
                print(string.format("Error creation failed for %d: %s", test.code, result))
            end
        end
    end
    
    -- Test error formatting
    if qtforge.error.format_error then
        local success, result = pcall(function()
            return qtforge.error.format_error("Test error message", "additional context")
        end)
        
        if success then
            print(string.format("Formatted error: %s", result))
        else
            print(string.format("Error formatting failed: %s", result))
        end
    end
end

function UtilitiesExample:demonstrate_edge_cases()
    print("\n🔍 Demonstrating edge cases and error handling...")
    
    -- Test with nil inputs
    print("Testing with nil inputs:")
    local nil_tests = {
        {name = "JSON parse", func = function() return qtforge.json and qtforge.json.parse and qtforge.json.parse(nil) end},
        {name = "String trim", func = function() return qtforge.string and qtforge.string.trim and qtforge.string.trim(nil) end},
        {name = "File exists", func = function() return qtforge.filesystem and qtforge.filesystem.exists and qtforge.filesystem.exists(nil) end}
    }
    
    for _, test in ipairs(nil_tests) do
        local success, result = pcall(test.func)
        print(string.format("%s with nil: %s", test.name, success and "handled" or "error"))
    end
    
    -- Test with empty strings
    print("\nTesting with empty strings:")
    local empty_tests = {
        {name = "JSON validate", func = function() return qtforge.json and qtforge.json.validate and qtforge.json.validate("") end},
        {name = "String trim", func = function() return qtforge.string and qtforge.string.trim and qtforge.string.trim("") end},
        {name = "File exists", func = function() return qtforge.filesystem and qtforge.filesystem.exists and qtforge.filesystem.exists("") end}
    }
    
    for _, test in ipairs(empty_tests) do
        local success, result = pcall(test.func)
        if success then
            print(string.format("%s with empty string: %s", test.name, tostring(result)))
        else
            print(string.format("%s with empty string: error", test.name))
        end
    end
end

function UtilitiesExample:run_complete_example()
    print("🚀 QtForge Lua Bindings - Utilities Showcase Example")
    print(string.rep("=", 70))
    
    local success, error_msg = pcall(function()
        self:demonstrate_json_utilities()
        self:demonstrate_string_utilities()
        self:demonstrate_filesystem_utilities()
        self:demonstrate_logging_utilities()
        self:demonstrate_time_utilities()
        self:demonstrate_error_utilities()
        self:demonstrate_edge_cases()
    end)
    
    if success then
        print("\n✅ Utilities showcase example completed successfully!")
        return 0
    else
        print(string.format("\n❌ Example failed with error: %s", error_msg))
        return 1
    end
end

-- Main function
local function main()
    local success, result = pcall(function()
        local example = UtilitiesExample:new()
        return example:run_complete_example()
    end)
    
    if success then
        return result
    else
        print(string.format("❌ Failed to run utilities example: %s", result))
        return 1
    end
end

-- Run the example
local exit_code = main()
os.exit(exit_code)
