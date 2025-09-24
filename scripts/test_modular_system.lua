#!/usr/bin/env lua

-- QtForge Modular Build System Automated Testing
-- Automated tests to ensure the modular build system works correctly
-- Version: 3.2.0

-- Add xmake path for module imports
package.path = package.path .. ";cmake/xmake/?.lua"

-- Test framework
local test_framework = {
    tests = {},
    results = {
        passed = 0,
        failed = 0,
        skipped = 0,
        errors = {}
    }
}

-- Test utilities
local function log_test(name, status, message)
    local prefix = status == "PASS" and "[PASS]" or
                   status == "FAIL" and "[FAIL]" or
                   status == "SKIP" and "[SKIP]" or "[INFO]"
    print(prefix .. " " .. name .. (message and ": " .. message or ""))
end

local function assert_true(condition, message)
    if not condition then
        error(message or "Assertion failed")
    end
end

local function assert_equals(actual, expected, message)
    if actual ~= expected then
        error((message or "Values not equal") .. " (expected: " .. tostring(expected) .. ", actual: " .. tostring(actual) .. ")")
    end
end

local function assert_type(value, expected_type, message)
    local actual_type = type(value)
    if actual_type ~= expected_type then
        error((message or "Type mismatch") .. " (expected: " .. expected_type .. ", actual: " .. actual_type .. ")")
    end
end

-- Test runner
local function run_test(name, test_func)
    local success, error_msg = pcall(test_func)

    if success then
        test_framework.results.passed = test_framework.results.passed + 1
        log_test(name, "PASS")
    else
        test_framework.results.failed = test_framework.results.failed + 1
        table.insert(test_framework.results.errors, {name = name, error = error_msg})
        log_test(name, "FAIL", error_msg)
    end
end

-- Test: Common module functionality
local function test_common_module()
    local success, common = pcall(dofile, "cmake/xmake/common.lua")
    assert_true(success, "Failed to load common module")
    assert_type(common, "table", "Common module should return table")

    -- Test required functions exist
    assert_type(common.detect_active_toolchain, "function", "detect_active_toolchain should be function")
    assert_type(common.get_qt_features, "function", "get_qt_features should be function")

    -- Test Qt features
    local features = common.get_qt_features("basic")
    assert_type(features, "table", "Qt features should return table")
    assert_true(#features > 0, "Qt features should not be empty")
end

-- Test: Dependencies module functionality
local function test_dependencies_module()
    local success, dependencies = pcall(dofile, "cmake/xmake/dependencies.lua")
    assert_true(success, "Failed to load dependencies module")
    assert_type(dependencies, "table", "Dependencies module should return table")

    -- Test required functions exist
    assert_type(dependencies.setup_qt6_with_toolchain, "function", "setup_qt6_with_toolchain should be function")
    assert_type(dependencies.validate_all_dependencies, "function", "validate_all_dependencies should be function")
end

-- Test: Example template functionality
local function test_example_template_module()
    local success, example_template = pcall(dofile, "cmake/xmake/example_template.lua")
    assert_true(success, "Failed to load example_template module")
    assert_type(example_template, "table", "Example template module should return table")

    -- Test required functions exist
    assert_type(example_template.create_validated_example, "function", "create_validated_example should be function")
    assert_type(example_template.apply_toolchain_config, "function", "apply_toolchain_config should be function")
end

-- Test: Plugin template functionality
local function test_plugin_template_module()
    local success, plugin_template = pcall(dofile, "cmake/xmake/plugin_template.lua")
    assert_true(success, "Failed to load plugin_template module")
    assert_type(plugin_template, "table", "Plugin template module should return table")

    -- Test required functions exist
    assert_type(plugin_template.create_validated_plugin, "function", "create_validated_plugin should be function")
    assert_type(plugin_template.apply_toolchain_config, "function", "apply_toolchain_config should be function")
end

-- Test: Registry functionality
local function test_registry_module()
    local success, registry = pcall(dofile, "cmake/xmake/registry.lua")
    assert_true(success, "Failed to load registry module")
    assert_type(registry, "table", "Registry module should return table")

    -- Test required functions exist
    assert_type(registry.register_example, "function", "register_example should be function")
    assert_type(registry.discover_examples, "function", "discover_examples should be function")
    assert_type(registry.validate_example_metadata, "function", "validate_example_metadata should be function")
end

-- Test: Validator functionality
local function test_validator_module()
    local success, validator = pcall(dofile, "cmake/xmake/validator.lua")
    assert_true(success, "Failed to load validator module")
    assert_type(validator, "table", "Validator module should return table")

    -- Test required functions exist
    assert_type(validator.validate_dual_toolchain_support, "function", "validate_dual_toolchain_support should be function")
    assert_type(validator.generate_toolchain_report, "function", "generate_toolchain_report should be function")
end

-- Test: Factory functionality
local function test_factory_module()
    local success, factory = pcall(dofile, "cmake/xmake/factory.lua")
    assert_true(success, "Failed to load factory module")
    assert_type(factory, "table", "Factory module should return table")

    -- Test required functions exist
    assert_type(factory.create_inherited_example, "function", "create_inherited_example should be function")
    assert_type(factory.create_inherited_plugin, "function", "create_inherited_plugin should be function")
    assert_type(factory.get_template_categories, "function", "get_template_categories should be function")

    -- Test template categories
    local categories = factory.get_template_categories()
    assert_type(categories, "table", "Template categories should return table")
    assert_true(#categories > 0, "Should have template categories")

    -- Test base templates exist
    assert_type(factory.base_templates, "table", "Base templates should exist")
    assert_type(factory.base_templates.fundamentals, "table", "Fundamentals template should exist")
    assert_type(factory.base_templates.communication, "table", "Communication template should exist")
end

-- Test: Toolchain selector functionality
local function test_toolchain_selector_module()
    local success, toolchain_selector = pcall(dofile, "cmake/xmake/toolchain_selector.lua")
    assert_true(success, "Failed to load toolchain_selector module")
    assert_type(toolchain_selector, "table", "Toolchain selector module should return table")

    -- Test required functions exist
    assert_type(toolchain_selector.detect_all, "function", "detect_all should be function")
    assert_type(toolchain_selector.select_toolchain, "function", "select_toolchain should be function")
    assert_type(toolchain_selector.configure_selected, "function", "configure_selected should be function")
end

-- Test: MSYS2 MinGW64 functionality
local function test_msys2_mingw64_module()
    local success, msys2_mingw64 = pcall(dofile, "cmake/xmake/msys2_mingw64.lua")
    assert_true(success, "Failed to load msys2_mingw64 module")
    assert_type(msys2_mingw64, "table", "MSYS2 MinGW64 module should return table")

    -- Test required functions exist
    assert_type(msys2_mingw64.setup, "function", "setup should be function")
    assert_type(msys2_mingw64.validate_environment, "function", "validate_environment should be function")
    assert_type(msys2_mingw64.get_installed_qt6_packages, "function", "get_installed_qt6_packages should be function")
end

-- Test: Qt6 detector functionality
local function test_qt6_detector_module()
    local success, qt6_detector = pcall(dofile, "cmake/xmake/qt6_detector.lua")
    assert_true(success, "Failed to load qt6_detector module")
    assert_type(qt6_detector, "table", "Qt6 detector module should return table")

    -- Test required functions exist
    assert_type(qt6_detector.detect_all, "function", "detect_all should be function")
    assert_type(qt6_detector.setup, "function", "setup should be function")
    assert_type(qt6_detector.configure_qt6, "function", "configure_qt6 should be function")
end

-- Test: Optimization functionality
local function test_optimization_module()
    local success, optimization = pcall(dofile, "cmake/xmake/optimization.lua")
    assert_true(success, "Failed to load optimization module")
    assert_type(optimization, "table", "Optimization module should return table")

    -- Test required functions exist
    assert_type(optimization.apply_optimizations, "function", "apply_optimizations should be function")
    assert_type(optimization.setup_compiler_cache, "function", "setup_compiler_cache should be function")
    assert_type(optimization.generate_report, "function", "generate_report should be function")

    -- Test configuration
    assert_type(optimization.config, "table", "Optimization config should exist")
    assert_type(optimization.config.ccache, "table", "CCCache config should exist")
    assert_type(optimization.config.lto, "table", "LTO config should exist")
end

-- Test: Logger functionality
local function test_logger_module()
    local success, logger = pcall(dofile, "cmake/xmake/logger.lua")
    assert_true(success, "Failed to load logger module")
    assert_type(logger, "table", "Logger module should return table")

    -- Test required functions exist
    assert_type(logger.init, "function", "init should be function")
    assert_type(logger.info, "function", "info should be function")
    assert_type(logger.error, "function", "error should be function")
    assert_type(logger.handle_error, "function", "handle_error should be function")

    -- Test log levels
    assert_type(logger.levels, "table", "Log levels should exist")
    assert_type(logger.levels.INFO, "number", "INFO level should be number")
    assert_type(logger.levels.ERROR, "number", "ERROR level should be number")
end

-- Test: Template inheritance system
local function test_template_inheritance()
    local success, factory = pcall(dofile, "cmake/xmake/factory.lua")
    assert_true(success, "Failed to load factory module")

    -- Test inheritance resolution
    local config = factory.resolve_template_inheritance("fundamentals", {
        sources = {"custom.cpp"}
    })

    assert_type(config, "table", "Resolved config should be table")
    assert_type(config.sources, "table", "Sources should be table")
    assert_true(#config.sources > 1, "Should merge sources from base and override")

    -- Test category validation
    local validation = factory.validate_inheritance_config("fundamentals", {})
    assert_type(validation, "table", "Validation should return table")
    assert_type(validation.valid, "boolean", "Validation should have valid field")
end

-- Test: Error handling system
local function test_error_handling()
    local success, logger = pcall(dofile, "cmake/xmake/logger.lua")
    assert_true(success, "Failed to load logger module")

    -- Test error handling
    local error_obj = logger.handle_error("Test error", {module = "test"})
    assert_type(error_obj, "table", "Error object should be table")
    assert_equals(error_obj.message, "Test error", "Error message should match")
    assert_equals(error_obj.handled, true, "Error should be marked as handled")

    -- Test parameter validation
    local valid, error_msg = logger.validate_params(
        {name = "test", value = 42},
        {
            name = {required = true, type = "string"},
            value = {required = true, type = "number"}
        },
        "test_function"
    )
    assert_equals(valid, true, "Valid parameters should pass validation")

    -- Test invalid parameters
    local invalid, invalid_msg = logger.validate_params(
        {name = 123}, -- Wrong type
        {name = {required = true, type = "string"}},
        "test_function"
    )
    assert_equals(invalid, false, "Invalid parameters should fail validation")
    assert_type(invalid_msg, "string", "Should return error message")
end

-- Integration test: Full workflow
local function test_integration_workflow()
    -- Load required modules
    local factory_success, factory = pcall(dofile, "cmake/xmake/factory.lua")
    local logger_success, logger = pcall(dofile, "cmake/xmake/logger.lua")

    assert_true(factory_success, "Failed to load factory module")
    assert_true(logger_success, "Failed to load logger module")

    -- Test complete workflow
    local categories = factory.get_template_categories()
    assert_true(#categories > 0, "Should have categories")

    local category_info = factory.get_category_info("fundamentals")
    assert_type(category_info, "table", "Category info should be table")
    assert_equals(category_info.category, "fundamentals", "Category should match")

    -- Test inheritance report
    local report = factory.generate_inheritance_report()
    assert_type(report, "table", "Report should be table")
    assert_type(report.categories, "table", "Report should have categories")
    assert_true(report.total_categories > 0, "Should have categories in report")
end

-- Main test runner
local function run_all_tests()
    print("Starting QtForge Modular Build System Tests")
    print("=" .. string.rep("=", 50))

    -- Register and run tests
    local tests = {
        {"Common Module", test_common_module},
        {"Dependencies Module", test_dependencies_module},
        {"Example Template Module", test_example_template_module},
        {"Plugin Template Module", test_plugin_template_module},
        {"Registry Module", test_registry_module},
        {"Validator Module", test_validator_module},
        {"Factory Module", test_factory_module},
        {"Toolchain Selector Module", test_toolchain_selector_module},
        {"MSYS2 MinGW64 Module", test_msys2_mingw64_module},
        {"Qt6 Detector Module", test_qt6_detector_module},
        {"Optimization Module", test_optimization_module},
        {"Logger Module", test_logger_module},
        {"Template Inheritance", test_template_inheritance},
        {"Error Handling", test_error_handling},
        {"Integration Workflow", test_integration_workflow}
    }

    for _, test in ipairs(tests) do
        run_test(test[1], test[2])
    end

    -- Print summary
    print("=" .. string.rep("=", 50))
    print("Test Summary:")
    print("  Passed: " .. test_framework.results.passed)
    print("  Failed: " .. test_framework.results.failed)
    print("  Skipped: " .. test_framework.results.skipped)

    if test_framework.results.failed > 0 then
        print("Failed tests:")
        for _, error in ipairs(test_framework.results.errors) do
            print("  - " .. error.name .. ": " .. error.error)
        end
        return false
    else
        print("All tests passed!")
        return true
    end
end

-- Run tests if script is executed directly
if arg and arg[0] and arg[0]:match("test_modular_system%.lua$") then
    local success = run_all_tests()
    os.exit(success and 0 or 1)
end

-- Export for use as module
return {
    run_all_tests = run_all_tests,
    test_framework = test_framework
}
