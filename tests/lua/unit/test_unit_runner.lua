#!/usr/bin/env lua
--[[
Unit test runner for QtForge Lua bindings.
Runs all unit tests with comprehensive reporting and coverage analysis.
]]

-- Test runner class
local TestRunner = {}
TestRunner.__index = TestRunner

function TestRunner:new()
    local obj = {
        test_dir = ".",
        test_modules = {
            "test_core_unit.lua",
            "test_utils_unit.lua",
            "test_communication_unit.lua",
            "test_security_unit.lua",
            "test_managers_unit.lua",
            "test_orchestration_unit.lua",
            "test_monitoring_unit.lua",
            "test_transactions_unit.lua",
            "test_composition_unit.lua",
            "test_marketplace_unit.lua",
            "test_threading_unit.lua"
        },
        results = {}
    }
    setmetatable(obj, TestRunner)
    return obj
end

function TestRunner:check_qtforge_availability()
    local success, qtforge = pcall(require, "qtforge")
    if success then
        print("✅ QtForge Lua bindings are available")
        return true, qtforge
    else
        print(string.format("❌ QtForge Lua bindings not available: %s", qtforge))
        return false, nil
    end
end

function TestRunner:check_test_files()
    local existing_tests = {}
    local missing_tests = {}
    
    for _, test_module in ipairs(self.test_modules) do
        local file = io.open(test_module, "r")
        if file then
            file:close()
            table.insert(existing_tests, test_module)
        else
            table.insert(missing_tests, test_module)
        end
    end
    
    if #missing_tests > 0 then
        print("⚠️  Some unit test modules are missing:")
        for _, missing in ipairs(missing_tests) do
            print(string.format("   - %s", missing))
        end
        print()
    end
    
    return existing_tests, missing_tests
end

function TestRunner:run_individual_test(test_file)
    print(string.format("🧪 Running %s...", test_file))
    
    local start_time = os.clock()
    
    -- Execute the test file
    local success, result = pcall(function()
        return dofile(test_file)
    end)
    
    local end_time = os.clock()
    local duration = end_time - start_time
    
    if success then
        local exit_code = result or 0
        if exit_code == 0 then
            return {
                status = 'passed',
                duration = duration,
                exit_code = exit_code
            }
        else
            return {
                status = 'failed',
                duration = duration,
                exit_code = exit_code
            }
        end
    else
        return {
            status = 'error',
            duration = duration,
            error = tostring(result)
        }
    end
end

function TestRunner:run_all_tests()
    local available, qtforge = self:check_qtforge_availability()
    if not available then
        print("❌ Cannot run unit tests - QtForge bindings not available")
        return {status = 'skipped', reason = 'Bindings not available'}
    end
    
    print("🚀 Running QtForge Lua unit tests...")
    print(string.rep("=", 60))
    
    local existing_tests, missing_tests = self:check_test_files()
    
    if #existing_tests == 0 then
        print("❌ No unit test files found!")
        return {status = 'no_tests'}
    end
    
    print(string.format("📋 Found %d unit test modules", #existing_tests))
    print()
    
    local overall_start_time = os.clock()
    local results = {}
    
    -- Run each test module
    for _, test_file in ipairs(existing_tests) do
        local result = self:run_individual_test(test_file)
        results[test_file] = result
        
        -- Print immediate result
        local status = result.status
        local duration = result.duration
        
        if status == 'passed' then
            print(string.format("   ✅ %s - PASSED (%.2fs)", test_file, duration))
        elseif status == 'failed' then
            print(string.format("   ❌ %s - FAILED (%.2fs)", test_file, duration))
        elseif status == 'error' then
            print(string.format("   ⚠️  %s - ERROR (%.2fs)", test_file, duration))
            if result.error then
                print(string.format("      Error: %s", result.error))
            end
        else
            print(string.format("   ❓ %s - %s (%.2fs)", test_file, status:upper(), duration))
        end
    end
    
    local overall_end_time = os.clock()
    local total_duration = overall_end_time - overall_start_time
    
    -- Calculate summary statistics
    local passed = 0
    local failed = 0
    local errors = 0
    
    for _, result in pairs(results) do
        if result.status == 'passed' then
            passed = passed + 1
        elseif result.status == 'failed' then
            failed = failed + 1
        elseif result.status == 'error' then
            errors = errors + 1
        end
    end
    
    print()
    print(string.rep("=", 60))
    print("📊 Unit Test Summary:")
    print(string.format("   Total modules: %d", #existing_tests))
    print(string.format("   Passed: %d", passed))
    print(string.format("   Failed: %d", failed))
    print(string.format("   Errors: %d", errors))
    print(string.format("   Total time: %.2fs", total_duration))
    
    local overall_status
    if failed == 0 and errors == 0 then
        print("✅ All unit tests passed!")
        overall_status = 'passed'
    else
        print("❌ Some unit tests failed!")
        overall_status = 'failed'
    end
    
    return {
        status = overall_status,
        total_duration = total_duration,
        results = results,
        summary = {
            total = #existing_tests,
            passed = passed,
            failed = failed,
            errors = errors
        }
    }
end

function TestRunner:generate_test_report(results)
    local report_lines = {
        "QtForge Lua Bindings Unit Test Report",
        string.rep("=", 50),
        string.format("Generated: %s", os.date("%Y-%m-%d %H:%M:%S")),
        ""
    }
    
    if results.status == 'skipped' then
        table.insert(report_lines, "❌ Tests were skipped:")
        table.insert(report_lines, string.format("   Reason: %s", results.reason or "Unknown"))
        table.insert(report_lines, "")
        return table.concat(report_lines, "\n")
    end
    
    local summary = results.summary or {}
    table.insert(report_lines, "📊 Summary:")
    table.insert(report_lines, string.format("   Total test modules: %d", summary.total or 0))
    table.insert(report_lines, string.format("   Passed: %d", summary.passed or 0))
    table.insert(report_lines, string.format("   Failed: %d", summary.failed or 0))
    table.insert(report_lines, string.format("   Errors: %d", summary.errors or 0))
    table.insert(report_lines, string.format("   Total duration: %.2fs", results.total_duration or 0))
    table.insert(report_lines, "")
    
    -- Detailed results
    local test_results = results.results or {}
    if next(test_results) then
        table.insert(report_lines, "📋 Detailed Results:")
        table.insert(report_lines, "")
        
        for test_file, result in pairs(test_results) do
            local status = result.status
            local duration = result.duration or 0
            
            local status_icon = {
                passed = '✅',
                failed = '❌',
                error = '⚠️'
            }
            
            local icon = status_icon[status] or '❓'
            table.insert(report_lines, string.format("   %s %s: %s (%.2fs)", 
                icon, test_file, status:upper(), duration))
            
            if result.error then
                table.insert(report_lines, string.format("      Error: %s", result.error))
            end
        end
    end
    
    return table.concat(report_lines, "\n")
end

function TestRunner:run_specific_module(module_name)
    local test_file = string.format("test_%s_unit.lua", module_name)
    
    local file = io.open(test_file, "r")
    if not file then
        print(string.format("❌ Test module not found: %s", test_file))
        return 1
    end
    file:close()
    
    print(string.format("🧪 Running tests for %s module...", module_name))
    
    local result = self:run_individual_test(test_file)
    
    if result.status == 'passed' then
        print("✅ Module tests passed!")
        return 0
    else
        print("❌ Module tests failed!")
        if result.error then
            print(string.format("Error: %s", result.error))
        end
        return 1
    end
end

function TestRunner:list_available_modules()
    print("Available test modules:")
    print()
    
    local existing_tests, missing_tests = self:check_test_files()
    
    if #existing_tests > 0 then
        print("✅ Available:")
        for _, test_file in ipairs(existing_tests) do
            local module_name = test_file:match("test_(.+)_unit%.lua")
            if module_name then
                print(string.format("   - %s", module_name))
            end
        end
    end
    
    if #missing_tests > 0 then
        print()
        print("❌ Missing:")
        for _, test_file in ipairs(missing_tests) do
            local module_name = test_file:match("test_(.+)_unit%.lua")
            if module_name then
                print(string.format("   - %s", module_name))
            end
        end
    end
end

-- Main function
local function main(args)
    local runner = TestRunner:new()
    
    if #args == 0 then
        -- Run all tests
        local results = runner:run_all_tests()
        return results.status == 'passed' and 0 or 1
        
    elseif args[1] == "--help" or args[1] == "-h" then
        print("QtForge Lua Unit Test Runner")
        print()
        print("Usage:")
        print("  lua test_unit_runner.lua                    # Run all tests")
        print("  lua test_unit_runner.lua <module>           # Run specific module tests")
        print("  lua test_unit_runner.lua --list             # List available modules")
        print("  lua test_unit_runner.lua --report           # Generate detailed report")
        print("  lua test_unit_runner.lua --help             # Show this help")
        print()
        print("Examples:")
        print("  lua test_unit_runner.lua core               # Test core module")
        print("  lua test_unit_runner.lua utils              # Test utils module")
        return 0
        
    elseif args[1] == "--list" then
        runner:list_available_modules()
        return 0
        
    elseif args[1] == "--report" then
        -- Run tests and generate detailed report
        local results = runner:run_all_tests()
        local report = runner:generate_test_report(results)
        
        -- Save report to file
        local report_file = "lua_unit_test_report.txt"
        local file = io.open(report_file, "w")
        if file then
            file:write(report)
            file:close()
            print(string.format("\n📄 Detailed report saved to: %s", report_file))
        else
            print("\n❌ Failed to save report file")
        end
        
        return results.status == 'passed' and 0 or 1
        
    else
        -- Run specific module
        return runner:run_specific_module(args[1])
    end
end

-- Parse command line arguments
local args = {}
for i = 1, #arg do
    table.insert(args, arg[i])
end

-- Run main function and exit with appropriate code
local exit_code = main(args)
os.exit(exit_code)
