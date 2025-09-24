#!/usr/bin/env lua

-- QtForge Build System Integrity Validation Script
-- Validates the integrity and correctness of the modular build system
-- Version: 3.2.0

-- Add xmake path for module imports
package.path = package.path .. ";cmake/xmake/?.lua"

-- Validation results
local validation_results = {
    passed = 0,
    failed = 0,
    warnings = 0,
    errors = {}
}

-- Utility functions
local function log_info(message)
    print("[INFO] " .. message)
end

local function log_warn(message)
    print("[WARN] " .. message)
    validation_results.warnings = validation_results.warnings + 1
end

local function log_error(message)
    print("[ERROR] " .. message)
    table.insert(validation_results.errors, message)
    validation_results.failed = validation_results.failed + 1
end

local function log_pass(message)
    print("[PASS] " .. message)
    validation_results.passed = validation_results.passed + 1
end

-- Check if file exists
local function file_exists(path)
    local file = io.open(path, "r")
    if file then
        file:close()
        return true
    end
    return false
end

-- Check if directory exists
local function dir_exists(path)
    local ok, err, code = os.rename(path, path)
    if not ok then
        if code == 13 then -- Permission denied, but it exists
            return true
        end
    end
    return ok, err
end

-- Validate module structure
local function validate_module_structure()
    log_info("Validating module structure...")

    local required_modules = {
        "cmake/xmake/common.lua",
        "cmake/xmake/dependencies.lua",
        "cmake/xmake/example_template.lua",
        "cmake/xmake/plugin_template.lua",
        "cmake/xmake/registry.lua",
        "cmake/xmake/validator.lua",
        "cmake/xmake/factory.lua",
        "cmake/xmake/toolchain_selector.lua",
        "cmake/xmake/msys2_mingw64.lua",
        "cmake/xmake/qt6_detector.lua",
        "cmake/xmake/optimization.lua",
        "cmake/xmake/logger.lua"
    }

    for _, module in ipairs(required_modules) do
        if file_exists(module) then
            log_pass("Module exists: " .. module)
        else
            log_error("Missing required module: " .. module)
        end
    end
end

-- Validate module syntax
local function validate_module_syntax()
    log_info("Validating module syntax...")

    local modules = {
        "cmake/xmake/common.lua",
        "cmake/xmake/dependencies.lua",
        "cmake/xmake/example_template.lua",
        "cmake/xmake/plugin_template.lua",
        "cmake/xmake/registry.lua",
        "cmake/xmake/validator.lua",
        "cmake/xmake/factory.lua",
        "cmake/xmake/toolchain_selector.lua",
        "cmake/xmake/msys2_mingw64.lua",
        "cmake/xmake/qt6_detector.lua",
        "cmake/xmake/optimization.lua",
        "cmake/xmake/logger.lua"
    }

    for _, module in ipairs(modules) do
        if file_exists(module) then
            local chunk, err = loadfile(module)
            if chunk then
                log_pass("Syntax valid: " .. module)
            else
                log_error("Syntax error in " .. module .. ": " .. (err or "unknown error"))
            end
        end
    end
end

-- Validate module exports
local function validate_module_exports()
    log_info("Validating module exports...")

    local expected_exports = {
        ["cmake/xmake/common.lua"] = {
            "detect_active_toolchain",
            "setup_mingw64_development_environment",
            "get_qt_features"
        },
        ["cmake/xmake/dependencies.lua"] = {
            "setup_qt6_with_toolchain",
            "validate_all_dependencies",
            "auto_setup_dependencies"
        },
        ["cmake/xmake/example_template.lua"] = {
            "create_validated_example",
            "apply_toolchain_config",
            "get_toolchain_recommendations"
        },
        ["cmake/xmake/plugin_template.lua"] = {
            "create_validated_plugin",
            "apply_toolchain_config"
        },
        ["cmake/xmake/registry.lua"] = {
            "register_example",
            "discover_examples",
            "validate_example_metadata"
        },
        ["cmake/xmake/validator.lua"] = {
            "validate_dual_toolchain_support",
            "generate_toolchain_report"
        },
        ["cmake/xmake/factory.lua"] = {
            "create_inherited_example",
            "create_inherited_plugin",
            "get_template_categories"
        },
        ["cmake/xmake/toolchain_selector.lua"] = {
            "detect_all",
            "select_toolchain",
            "configure_selected"
        },
        ["cmake/xmake/msys2_mingw64.lua"] = {
            "setup",
            "validate_environment",
            "get_installed_qt6_packages"
        },
        ["cmake/xmake/qt6_detector.lua"] = {
            "detect_all",
            "setup",
            "configure_qt6"
        },
        ["cmake/xmake/optimization.lua"] = {
            "apply_optimizations",
            "setup_compiler_cache",
            "generate_report"
        },
        ["cmake/xmake/logger.lua"] = {
            "init",
            "info",
            "error",
            "handle_error"
        }
    }

    for module_path, expected_funcs in pairs(expected_exports) do
        if file_exists(module_path) then
            local success, module = pcall(dofile, module_path)
            if success and type(module) == "table" then
                for _, func_name in ipairs(expected_funcs) do
                    if type(module[func_name]) == "function" then
                        log_pass("Function exists: " .. module_path .. "." .. func_name)
                    else
                        log_error("Missing function: " .. module_path .. "." .. func_name)
                    end
                end
            else
                log_error("Failed to load module: " .. module_path)
            end
        end
    end
end

-- Validate example structure
local function validate_example_structure()
    log_info("Validating example structure...")

    local example_dirs = {
        "examples/01-fundamentals",
        "examples/02-communication",
        "examples/04-specialized"
    }

    for _, dir in ipairs(example_dirs) do
        if dir_exists(dir) then
            log_pass("Example directory exists: " .. dir)

            -- Check for xmake.lua in subdirectories
            local handle = io.popen("find " .. dir .. " -name 'xmake.lua' 2>/dev/null || dir /s /b " .. dir .. "\\xmake.lua 2>nul")
            if handle then
                local result = handle:read("*a")
                handle:close()

                if result and result:len() > 0 then
                    log_pass("Found xmake.lua files in: " .. dir)
                else
                    log_warn("No xmake.lua files found in: " .. dir)
                end
            end
        else
            log_error("Missing example directory: " .. dir)
        end
    end
end

-- Validate configuration files
local function validate_configuration_files()
    log_info("Validating configuration files...")

    local config_files = {
        "xmake.lua",
        "examples/xmake.lua"
    }

    for _, config in ipairs(config_files) do
        if file_exists(config) then
            log_pass("Configuration file exists: " .. config)

            -- Check syntax
            local chunk, err = loadfile(config)
            if chunk then
                log_pass("Configuration syntax valid: " .. config)
            else
                log_error("Configuration syntax error in " .. config .. ": " .. (err or "unknown error"))
            end
        else
            log_error("Missing configuration file: " .. config)
        end
    end
end

-- Validate documentation
local function validate_documentation()
    log_info("Validating documentation...")

    local doc_files = {
        "docs/MODULAR_BUILD_SYSTEM.md",
        "README.md"
    }

    for _, doc in ipairs(doc_files) do
        if file_exists(doc) then
            log_pass("Documentation exists: " .. doc)
        else
            log_warn("Missing documentation: " .. doc)
        end
    end
end

-- Validate toolchain integration
local function validate_toolchain_integration()
    log_info("Validating toolchain integration...")

    -- Check if toolchain selector can be loaded
    if file_exists("cmake/xmake/toolchain_selector.lua") then
        local success, toolchain_selector = pcall(dofile, "cmake/xmake/toolchain_selector.lua")
        if success and type(toolchain_selector) == "table" then
            if type(toolchain_selector.detect_all) == "function" then
                log_pass("Toolchain selector module functional")
            else
                log_error("Toolchain selector missing detect_all function")
            end
        else
            log_error("Failed to load toolchain selector module")
        end
    end

    -- Check MSYS2 integration
    if file_exists("cmake/xmake/msys2_mingw64.lua") then
        local success, msys2_module = pcall(dofile, "cmake/xmake/msys2_mingw64.lua")
        if success and type(msys2_module) == "table" then
            if type(msys2_module.setup) == "function" then
                log_pass("MSYS2 MinGW64 module functional")
            else
                log_error("MSYS2 MinGW64 module missing setup function")
            end
        else
            log_error("Failed to load MSYS2 MinGW64 module")
        end
    end
end

-- Validate build system consistency
local function validate_consistency()
    log_info("Validating build system consistency...")

    -- Check for consistent naming conventions
    local modules = {
        "common", "dependencies", "example_template", "plugin_template",
        "registry", "validator", "factory", "toolchain_selector",
        "msys2_mingw64", "qt6_detector", "optimization", "logger"
    }

    for _, module in ipairs(modules) do
        local path = "cmake/xmake/" .. module .. ".lua"
        if file_exists(path) then
            log_pass("Consistent naming: " .. module)
        else
            log_error("Inconsistent naming: " .. module)
        end
    end
end

-- Main validation function
local function run_validation()
    log_info("Starting QtForge Build System Validation")
    log_info("=" .. string.rep("=", 50))

    validate_module_structure()
    validate_module_syntax()
    validate_module_exports()
    validate_example_structure()
    validate_configuration_files()
    validate_documentation()
    validate_toolchain_integration()
    validate_consistency()

    -- Print summary
    log_info("=" .. string.rep("=", 50))
    log_info("Validation Summary:")
    log_info("  Passed: " .. validation_results.passed)
    log_info("  Failed: " .. validation_results.failed)
    log_info("  Warnings: " .. validation_results.warnings)

    if validation_results.failed > 0 then
        log_info("Errors encountered:")
        for _, error in ipairs(validation_results.errors) do
            log_info("  - " .. error)
        end
        return false
    else
        log_info("All validations passed!")
        return true
    end
end

-- Run validation if script is executed directly
if arg and arg[0] and arg[0]:match("validate_build_system%.lua$") then
    local success = run_validation()
    os.exit(success and 0 or 1)
end

-- Export for use as module
return {
    run_validation = run_validation,
    validation_results = validation_results
}
