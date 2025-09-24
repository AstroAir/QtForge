#!/usr/bin/env lua

-- QtForge Automated Toolchain Validation
-- Automated tests to ensure toolchain detection and configuration works correctly
-- Version: 3.2.0

-- Add xmake path for module imports
package.path = package.path .. ";cmake/xmake/?.lua"

-- Validation framework
local toolchain_validator = {
    results = {
        passed = 0,
        failed = 0,
        warnings = 0,
        errors = {},
        toolchains = {}
    }
}

-- Utility functions
local function log_info(message)
    print("[INFO] " .. message)
end

local function log_pass(message)
    print("[PASS] " .. message)
    toolchain_validator.results.passed = toolchain_validator.results.passed + 1
end

local function log_fail(message)
    print("[FAIL] " .. message)
    table.insert(toolchain_validator.results.errors, message)
    toolchain_validator.results.failed = toolchain_validator.results.failed + 1
end

local function log_warn(message)
    print("[WARN] " .. message)
    toolchain_validator.results.warnings = toolchain_validator.results.warnings + 1
end

-- Test toolchain selector module
local function test_toolchain_selector()
    log_info("Testing toolchain selector module...")

    local success, toolchain_selector = pcall(dofile, "cmake/xmake/toolchain_selector.lua")
    if not success then
        log_fail("Failed to load toolchain_selector module: " .. tostring(toolchain_selector))
        return false
    end

    -- Test detect_all function
    if type(toolchain_selector.detect_all) ~= "function" then
        log_fail("toolchain_selector.detect_all is not a function")
        return false
    end

    -- Test select_toolchain function
    if type(toolchain_selector.select_toolchain) ~= "function" then
        log_fail("toolchain_selector.select_toolchain is not a function")
        return false
    end

    -- Test configure_selected function
    if type(toolchain_selector.configure_selected) ~= "function" then
        log_fail("toolchain_selector.configure_selected is not a function")
        return false
    end

    log_pass("Toolchain selector module loaded successfully")
    return true
end

-- Test MSYS2 MinGW64 module
local function test_msys2_module()
    log_info("Testing MSYS2 MinGW64 module...")

    local success, msys2_mingw64 = pcall(dofile, "cmake/xmake/msys2_mingw64.lua")
    if not success then
        log_fail("Failed to load msys2_mingw64 module: " .. tostring(msys2_mingw64))
        return false
    end

    -- Test required functions
    local required_functions = {"setup", "validate_environment", "get_installed_qt6_packages"}
    for _, func_name in ipairs(required_functions) do
        if type(msys2_mingw64[func_name]) ~= "function" then
            log_fail("msys2_mingw64." .. func_name .. " is not a function")
            return false
        end
    end

    log_pass("MSYS2 MinGW64 module loaded successfully")
    return true
end

-- Test Qt6 detector module
local function test_qt6_detector()
    log_info("Testing Qt6 detector module...")

    local success, qt6_detector = pcall(dofile, "cmake/xmake/qt6_detector.lua")
    if not success then
        log_fail("Failed to load qt6_detector module: " .. tostring(qt6_detector))
        return false
    end

    -- Test required functions
    local required_functions = {"detect_all", "setup", "configure_qt6"}
    for _, func_name in ipairs(required_functions) do
        if type(qt6_detector[func_name]) ~= "function" then
            log_fail("qt6_detector." .. func_name .. " is not a function")
            return false
        end
    end

    log_pass("Qt6 detector module loaded successfully")
    return true
end

-- Test MSVC toolchain detection
local function test_msvc_detection()
    log_info("Testing MSVC toolchain detection...")

    -- Check for Visual Studio installations
    local vs_paths = {
        "C:/Program Files/Microsoft Visual Studio",
        "C:/Program Files (x86)/Microsoft Visual Studio"
    }

    local msvc_found = false
    for _, vs_path in ipairs(vs_paths) do
        local handle = io.popen('dir "' .. vs_path .. '" 2>nul')
        if handle then
            local result = handle:read("*a")
            handle:close()
            if result and result:len() > 0 then
                msvc_found = true
                break
            end
        end
    end

    if msvc_found then
        log_pass("MSVC installation detected")
        toolchain_validator.results.toolchains.msvc = {
            available = true,
            detected = true
        }
    else
        log_warn("MSVC installation not detected")
        toolchain_validator.results.toolchains.msvc = {
            available = false,
            detected = false
        }
    end

    return msvc_found
end

-- Test MinGW64 toolchain detection
local function test_mingw64_detection()
    log_info("Testing MinGW64 toolchain detection...")

    -- Check for MSYS2 installation
    local msys2_paths = {
        "D:/msys64",
        "C:/msys64",
        "C:/tools/msys64"
    }

    local mingw64_found = false
    local msys2_root = nil

    for _, msys2_path in ipairs(msys2_paths) do
        local mingw64_path = msys2_path .. "/mingw64"
        local handle = io.popen('dir "' .. mingw64_path .. '" 2>nul')
        if handle then
            local result = handle:read("*a")
            handle:close()
            if result and result:len() > 0 then
                mingw64_found = true
                msys2_root = msys2_path
                break
            end
        end
    end

    if mingw64_found then
        log_pass("MinGW64 installation detected at: " .. msys2_root)
        toolchain_validator.results.toolchains.mingw64 = {
            available = true,
            detected = true,
            msys2_root = msys2_root
        }
    else
        log_warn("MinGW64 installation not detected")
        toolchain_validator.results.toolchains.mingw64 = {
            available = false,
            detected = false
        }
    end

    return mingw64_found
end

-- Test Qt6 detection for MSVC
local function test_qt6_msvc_detection()
    if not toolchain_validator.results.toolchains.msvc or
       not toolchain_validator.results.toolchains.msvc.available then
        log_warn("Skipping Qt6 MSVC detection - MSVC not available")
        return false
    end

    log_info("Testing Qt6 detection for MSVC...")

    -- Common Qt6 installation paths
    local qt6_paths = {
        "C:/Qt/6.5.0/msvc2022_64",
        "C:/Qt/6.6.0/msvc2022_64",
        "C:/Qt/6.7.0/msvc2022_64"
    }

    local qt6_msvc_found = false
    for _, qt6_path in ipairs(qt6_paths) do
        local qt6_core_lib = qt6_path .. "/lib/Qt6Core.lib"
        local handle = io.popen('dir "' .. qt6_core_lib .. '" 2>nul')
        if handle then
            local result = handle:read("*a")
            handle:close()
            if result and result:len() > 0 then
                qt6_msvc_found = true
                toolchain_validator.results.toolchains.msvc.qt6_path = qt6_path
                break
            end
        end
    end

    if qt6_msvc_found then
        log_pass("Qt6 for MSVC detected")
        return true
    else
        log_warn("Qt6 for MSVC not detected")
        return false
    end
end

-- Test Qt6 detection for MinGW64
local function test_qt6_mingw64_detection()
    if not toolchain_validator.results.toolchains.mingw64 or
       not toolchain_validator.results.toolchains.mingw64.available then
        log_warn("Skipping Qt6 MinGW64 detection - MinGW64 not available")
        return false
    end

    log_info("Testing Qt6 detection for MinGW64...")

    local msys2_root = toolchain_validator.results.toolchains.mingw64.msys2_root
    local qt6_mingw64_path = msys2_root .. "/mingw64/lib"

    -- Check for Qt6 libraries
    local handle = io.popen('dir "' .. qt6_mingw64_path .. '/libQt6Core*" 2>nul')
    local qt6_mingw64_found = false

    if handle then
        local result = handle:read("*a")
        handle:close()
        if result and result:len() > 0 then
            qt6_mingw64_found = true
            toolchain_validator.results.toolchains.mingw64.qt6_path = msys2_root .. "/mingw64"
        end
    end

    if qt6_mingw64_found then
        log_pass("Qt6 for MinGW64 detected")
        return true
    else
        log_warn("Qt6 for MinGW64 not detected")
        return false
    end
end

-- Test toolchain configuration
local function test_toolchain_configuration()
    log_info("Testing toolchain configuration...")

    -- Test MSVC configuration
    if toolchain_validator.results.toolchains.msvc and
       toolchain_validator.results.toolchains.msvc.available then

        log_info("Testing MSVC configuration...")

        -- Simulate MSVC configuration
        local msvc_config = {
            toolchain = "msvc",
            qt6_available = toolchain_validator.results.toolchains.msvc.qt6_path ~= nil,
            compiler = "cl.exe",
            linker = "link.exe"
        }

        if msvc_config.qt6_available then
            log_pass("MSVC configuration with Qt6 support")
        else
            log_warn("MSVC configuration without Qt6 support")
        end
    end

    -- Test MinGW64 configuration
    if toolchain_validator.results.toolchains.mingw64 and
       toolchain_validator.results.toolchains.mingw64.available then

        log_info("Testing MinGW64 configuration...")

        -- Simulate MinGW64 configuration
        local mingw64_config = {
            toolchain = "mingw64",
            qt6_available = toolchain_validator.results.toolchains.mingw64.qt6_path ~= nil,
            compiler = "gcc.exe",
            linker = "g++.exe"
        }

        if mingw64_config.qt6_available then
            log_pass("MinGW64 configuration with Qt6 support")
        else
            log_warn("MinGW64 configuration without Qt6 support")
        end
    end

    return true
end

-- Test dual toolchain support
local function test_dual_toolchain_support()
    log_info("Testing dual toolchain support...")

    local msvc_available = toolchain_validator.results.toolchains.msvc and
                          toolchain_validator.results.toolchains.msvc.available
    local mingw64_available = toolchain_validator.results.toolchains.mingw64 and
                             toolchain_validator.results.toolchains.mingw64.available

    if msvc_available and mingw64_available then
        log_pass("Dual toolchain support available (MSVC + MinGW64)")
        return true
    elseif msvc_available then
        log_warn("Only MSVC toolchain available")
        return false
    elseif mingw64_available then
        log_warn("Only MinGW64 toolchain available")
        return false
    else
        log_fail("No toolchains available")
        return false
    end
end

-- Test toolchain validation integration
local function test_validation_integration()
    log_info("Testing validation integration...")

    local success, validator = pcall(dofile, "cmake/xmake/validator.lua")
    if not success then
        log_fail("Failed to load validator module")
        return false
    end

    -- Test dual toolchain validation function
    if type(validator.validate_dual_toolchain_support) == "function" then
        log_pass("Dual toolchain validation function available")
    else
        log_fail("Dual toolchain validation function not available")
        return false
    end

    return true
end

-- Main validation function
local function run_toolchain_validation()
    log_info("Starting QtForge Toolchain Validation")
    log_info("=" .. string.rep("=", 50))

    -- Test module loading
    test_toolchain_selector()
    test_msys2_module()
    test_qt6_detector()

    -- Test toolchain detection
    test_msvc_detection()
    test_mingw64_detection()

    -- Test Qt6 detection
    test_qt6_msvc_detection()
    test_qt6_mingw64_detection()

    -- Test configuration
    test_toolchain_configuration()
    test_dual_toolchain_support()
    test_validation_integration()

    -- Print summary
    log_info("=" .. string.rep("=", 50))
    log_info("Toolchain Validation Summary:")
    log_info("  Passed: " .. toolchain_validator.results.passed)
    log_info("  Failed: " .. toolchain_validator.results.failed)
    log_info("  Warnings: " .. toolchain_validator.results.warnings)

    -- Print toolchain status
    log_info("Detected Toolchains:")
    for toolchain, info in pairs(toolchain_validator.results.toolchains) do
        local status = info.available and "Available" or "Not Available"
        local qt6_status = info.qt6_path and " (Qt6: Yes)" or " (Qt6: No)"
        log_info("  " .. toolchain:upper() .. ": " .. status .. qt6_status)
        if info.msys2_root then
            log_info("    MSYS2 Root: " .. info.msys2_root)
        end
        if info.qt6_path then
            log_info("    Qt6 Path: " .. info.qt6_path)
        end
    end

    if toolchain_validator.results.failed > 0 then
        log_info("Errors encountered:")
        for _, error in ipairs(toolchain_validator.results.errors) do
            log_info("  - " .. error)
        end
        return false
    else
        log_info("Toolchain validation completed successfully!")
        return true
    end
end

-- Run validation if script is executed directly
if arg and arg[0] and arg[0]:match("validate_toolchains%.lua$") then
    local success = run_toolchain_validation()
    os.exit(success and 0 or 1)
end

-- Export for use as module
return {
    run_toolchain_validation = run_toolchain_validation,
    toolchain_validator = toolchain_validator
}
