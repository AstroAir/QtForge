-- QtForge Validation Framework
-- Comprehensive validation system for example configurations
-- Version: 3.2.0

-- Module table
local validator = {}

-- Validation rules
validator.rules = {}

-- Register a validation rule
function validator.register_rule(name, rule_func, description)
    validator.rules[name] = {
        func = rule_func,
        description = description or "No description provided"
    }
end

-- Built-in validation rules

-- Check if directory exists
validator.register_rule("directory_exists", function(path, config)
    if not os.isdir(path) then
        return false, "Directory does not exist: " .. path
    end
    return true
end, "Validates that the example directory exists")

-- Check if xmake.lua exists
validator.register_rule("xmake_file_exists", function(path, config)
    local xmake_file = path.join(path, "xmake.lua")
    if not os.isfile(xmake_file) then
        return false, "xmake.lua file not found: " .. xmake_file
    end
    return true
end, "Validates that xmake.lua file exists")

-- Check if source files exist
validator.register_rule("source_files_exist", function(path, config)
    if not config.sources then
        return true  -- No sources specified, skip validation
    end

    local sources = type(config.sources) == "string" and {config.sources} or config.sources
    local missing = {}

    for _, source in ipairs(sources) do
        local source_path = path.join(path, source)
        if not os.isfile(source_path) then
            table.insert(missing, source)
        end
    end

    if #missing > 0 then
        return false, "Missing source files: " .. table.concat(missing, ", ")
    end

    return true
end, "Validates that specified source files exist")

-- Check if header files exist
validator.register_rule("header_files_exist", function(path, config)
    if not config.headers then
        return true  -- No headers specified, skip validation
    end

    local headers = type(config.headers) == "string" and {config.headers} or config.headers
    local missing = {}

    for _, header in ipairs(headers) do
        local header_path = path.join(path, header)
        if not os.isfile(header_path) then
            table.insert(missing, header)
        end
    end

    if #missing > 0 then
        return false, "Missing header files: " .. table.concat(missing, ", ")
    end

    return true
end, "Validates that specified header files exist")

-- Check if metadata files exist
validator.register_rule("metadata_files_exist", function(path, config)
    if not config.metadata_file then
        return true  -- No metadata specified, skip validation
    end

    local metadata_path = path.join(path, config.metadata_file)
    if not os.isfile(metadata_path) then
        return false, "Missing metadata file: " .. config.metadata_file
    end

    return true
end, "Validates that specified metadata files exist")

-- Check Qt package requirements
validator.register_rule("qt_packages_available", function(path, config)
    if config.requires_widgets and not has_package("qt6widgets") then
        return false, "Qt6Widgets required but not available"
    end

    if config.requires_network and not has_package("qt6network") then
        return false, "Qt6Network required but not available"
    end

    if config.requires_gui and not has_package("qt6gui") then
        return false, "Qt6Gui required but not available"
    end

    return true
end, "Validates that required Qt packages are available")

-- Check configuration completeness
validator.register_rule("config_complete", function(path, config)
    local required_fields = config.required_fields or {}
    local missing = {}

    for _, field in ipairs(required_fields) do
        if not config[field] then
            table.insert(missing, field)
        end
    end

    if #missing > 0 then
        return false, "Missing required configuration fields: " .. table.concat(missing, ", ")
    end

    return true
end, "Validates that required configuration fields are present")

-- Validate a single example
function validator.validate_example(path, config, rules_to_apply)
    rules_to_apply = rules_to_apply or {
        "directory_exists",
        "xmake_file_exists",
        "source_files_exist",
        "header_files_exist",
        "metadata_files_exist",
        "qt_packages_available",
        "config_complete"
    }

    local results = {
        path = path,
        config = config,
        passed = {},
        failed = {},
        warnings = {},
        valid = true
    }

    for _, rule_name in ipairs(rules_to_apply) do
        local rule = validator.rules[rule_name]
        if rule then
            local success, message = rule.func(path, config)
            if success then
                table.insert(results.passed, rule_name)
            else
                table.insert(results.failed, {rule = rule_name, message = message})
                results.valid = false
            end
        else
            table.insert(results.warnings, "Unknown validation rule: " .. rule_name)
        end
    end

    return results
end

-- Validate multiple examples
function validator.validate_examples(examples_config, rules_to_apply)
    local results = {
        total = 0,
        valid = 0,
        invalid = 0,
        examples = {}
    }

    for path, config in pairs(examples_config) do
        results.total = results.total + 1

        local example_result = validator.validate_example(path, config, rules_to_apply)
        results.examples[path] = example_result

        if example_result.valid then
            results.valid = results.valid + 1
        else
            results.invalid = results.invalid + 1
        end
    end

    return results
end

-- Generate validation report
function validator.generate_report(validation_results)
    local report = {
        summary = {
            total = validation_results.total,
            valid = validation_results.valid,
            invalid = validation_results.invalid,
            success_rate = validation_results.total > 0 and
                          (validation_results.valid / validation_results.total * 100) or 0
        },
        details = {}
    }

    for path, result in pairs(validation_results.examples) do
        report.details[path] = {
            valid = result.valid,
            passed_rules = #result.passed,
            failed_rules = #result.failed,
            warnings = #result.warnings,
            issues = result.failed
        }
    end

    return report
end

-- Print validation report
function validator.print_report(validation_results)
    local report = validator.generate_report(validation_results)

    print("QtForge Example Validation Report")
    print("=================================")
    print("Total examples: " .. report.summary.total)
    print("Valid examples: " .. report.summary.valid)
    print("Invalid examples: " .. report.summary.invalid)
    print("Success rate: " .. string.format("%.1f%%", report.summary.success_rate))
    print("")

    if report.summary.invalid > 0 then
        print("Issues found:")
        for path, details in pairs(report.details) do
            if not details.valid then
                print("  " .. path .. ":")
                for _, issue in ipairs(details.issues) do
                    print("    - " .. issue.message)
                end
            end
        end
        print("")
    end

    print("Valid examples:")
    for path, details in pairs(report.details) do
        if details.valid then
            print("  ✓ " .. path)
        end
    end
end

-- Quick validation function for use in build scripts
function validator.quick_validate(examples_config)
    local results = validator.validate_examples(examples_config)
    validator.print_report(results)
    return results
end

-- Enhanced validation for dual toolchain support

-- Validate toolchain availability and configuration
validator.validation_rules.toolchain_available = function(target_info, context)
    local toolchain = context.toolchain or "auto"

    if toolchain == "auto" then
        -- Check if any toolchain is available
        local toolchain_selector = import("toolchain_selector")
        if toolchain_selector.detect_all() then
            return {
                valid = true,
                message = "At least one toolchain is available"
            }
        else
            return {
                valid = false,
                message = "No supported toolchains found"
            }
        end
    elseif toolchain == "msvc" then
        local toolchain_selector = import("toolchain_selector")
        toolchain_selector.detect_msvc()
        if toolchain_selector.detection_results.msvc.available then
            return {
                valid = true,
                message = "MSVC toolchain is available"
            }
        else
            return {
                valid = false,
                message = "MSVC toolchain not found"
            }
        end
    elseif toolchain == "mingw64" then
        local toolchain_selector = import("toolchain_selector")
        toolchain_selector.detect_mingw64()
        if toolchain_selector.detection_results.mingw64.available then
            return {
                valid = true,
                message = "MinGW64 toolchain is available"
            }
        else
            return {
                valid = false,
                message = "MinGW64 toolchain not found"
            }
        end
    end

    return {
        valid = false,
        message = "Unknown toolchain: " .. toolchain
    }
end

-- Validate Qt6 availability for specific toolchain
validator.validation_rules.qt6_toolchain_available = function(target_info, context)
    local toolchain = context.toolchain or "auto"

    if toolchain == "auto" then
        -- Detect active toolchain
        local common = import("common")
        toolchain = common.detect_active_toolchain()
    end

    local qt6_detector = import("qt6_detector")
    local qt_info = qt6_detector.get_info(toolchain)

    if qt_info.available then
        return {
            valid = true,
            message = "Qt6 is available for " .. toolchain .. " toolchain (version: " .. (qt_info.version or "unknown") .. ")"
        }
    else
        return {
            valid = false,
            message = "Qt6 not available for " .. toolchain .. " toolchain"
        }
    end
end

-- Validate MSYS2 environment for MinGW64
validator.validation_rules.msys2_environment = function(target_info, context)
    local msys2_mingw64 = import("msys2_mingw64")

    if not msys2_mingw64.detected.available then
        -- Try to detect first
        msys2_mingw64.setup()
    end

    if msys2_mingw64.detected.available then
        local validation = msys2_mingw64.validate_environment(context.required_features or {})
        return {
            valid = validation.valid,
            message = validation.valid and "MSYS2 environment is valid" or "MSYS2 environment has issues",
            details = validation.issues
        }
    else
        return {
            valid = false,
            message = "MSYS2 not found"
        }
    end
end

-- Validate toolchain-specific build configuration
validator.validation_rules.toolchain_build_config = function(target_info, context)
    local toolchain = context.toolchain or "auto"

    if toolchain == "auto" then
        local common = import("common")
        toolchain = common.detect_active_toolchain()
    end

    local issues = {}

    -- Check toolchain-specific configurations
    if toolchain == "mingw64" then
        -- MinGW64-specific checks
        if not has_config("shared") and target_info.kind == "shared" then
            table.insert(issues, "Shared library target with static configuration")
        end

        -- Check for Qt6 GUI requirements
        if context.qt_features and table.contains(context.qt_features, "ui") then
            local msys2_mingw64 = import("msys2_mingw64")
            local qt_packages = msys2_mingw64.get_installed_qt6_packages()
            local has_gui_packages = false

            for _, package in ipairs(qt_packages) do
                if package:find("qt6%-base") then
                    has_gui_packages = true
                    break
                end
            end

            if not has_gui_packages then
                table.insert(issues, "GUI features requested but Qt6 GUI packages not installed")
            end
        end

    elseif toolchain == "msvc" then
        -- MSVC-specific checks
        if is_mode("debug") and not has_config("runtimes") then
            table.insert(issues, "Debug mode should specify runtime library")
        end
    end

    return {
        valid = #issues == 0,
        message = #issues == 0 and "Build configuration is valid for " .. toolchain or "Build configuration issues found",
        details = issues
    }
end

-- Comprehensive validation for dual toolchain support
function validator.validate_dual_toolchain_support(target_info, context)
    local results = {
        msvc = {
            available = false,
            qt6_available = false,
            validation_results = {}
        },
        mingw64 = {
            available = false,
            qt6_available = false,
            validation_results = {}
        },
        overall_valid = false
    }

    -- Test MSVC toolchain
    local msvc_context = table.copy(context)
    msvc_context.toolchain = "msvc"

    local msvc_toolchain_result = validator.validation_rules.toolchain_available(target_info, msvc_context)
    results.msvc.available = msvc_toolchain_result.valid

    if results.msvc.available then
        local msvc_qt6_result = validator.validation_rules.qt6_toolchain_available(target_info, msvc_context)
        results.msvc.qt6_available = msvc_qt6_result.valid

        local msvc_config_result = validator.validation_rules.toolchain_build_config(target_info, msvc_context)
        results.msvc.validation_results = {
            toolchain = msvc_toolchain_result,
            qt6 = msvc_qt6_result,
            config = msvc_config_result
        }
    end

    -- Test MinGW64 toolchain
    local mingw64_context = table.copy(context)
    mingw64_context.toolchain = "mingw64"

    local mingw64_toolchain_result = validator.validation_rules.toolchain_available(target_info, mingw64_context)
    results.mingw64.available = mingw64_toolchain_result.valid

    if results.mingw64.available then
        local mingw64_qt6_result = validator.validation_rules.qt6_toolchain_available(target_info, mingw64_context)
        results.mingw64.qt6_available = mingw64_qt6_result.valid

        local mingw64_config_result = validator.validation_rules.toolchain_build_config(target_info, mingw64_context)
        local msys2_result = validator.validation_rules.msys2_environment(target_info, mingw64_context)

        results.mingw64.validation_results = {
            toolchain = mingw64_toolchain_result,
            qt6 = mingw64_qt6_result,
            config = mingw64_config_result,
            msys2 = msys2_result
        }
    end

    -- Overall validation
    results.overall_valid = (results.msvc.available and results.msvc.qt6_available) or
                           (results.mingw64.available and results.mingw64.qt6_available)

    return results
end

-- Generate comprehensive validation report
function validator.generate_toolchain_report(target_info, context)
    local report = {
        target = target_info.name,
        timestamp = os.date("%Y-%m-%d %H:%M:%S"),
        toolchain_support = {},
        recommendations = {},
        summary = {}
    }

    -- Run dual toolchain validation
    local validation_results = validator.validate_dual_toolchain_support(target_info, context)
    report.toolchain_support = validation_results

    -- Generate recommendations
    if validation_results.msvc.available and validation_results.mingw64.available then
        table.insert(report.recommendations, "Both MSVC and MinGW64 toolchains are available")
        table.insert(report.recommendations, "Consider testing with both toolchains for maximum compatibility")
    elseif validation_results.msvc.available then
        table.insert(report.recommendations, "Only MSVC toolchain is available")
        table.insert(report.recommendations, "Consider installing MSYS2 MinGW64 for additional toolchain support")
    elseif validation_results.mingw64.available then
        table.insert(report.recommendations, "Only MinGW64 toolchain is available")
        table.insert(report.recommendations, "Consider installing Visual Studio for MSVC toolchain support")
    else
        table.insert(report.recommendations, "No supported toolchains found")
        table.insert(report.recommendations, "Install either Visual Studio (MSVC) or MSYS2 MinGW64")
    end

    -- Generate summary
    report.summary.total_toolchains = (validation_results.msvc.available and 1 or 0) +
                                     (validation_results.mingw64.available and 1 or 0)
    report.summary.qt6_support = (validation_results.msvc.qt6_available and 1 or 0) +
                                (validation_results.mingw64.qt6_available and 1 or 0)
    report.summary.ready_for_build = validation_results.overall_valid

    return report
end

-- Utility function for table copying
function table.copy(orig)
    local copy = {}
    for k, v in pairs(orig) do
        copy[k] = v
    end
    return copy
end

-- Utility function for table.contains
function table.contains(tbl, value)
    for _, v in ipairs(tbl) do
        if v == value then
            return true
        end
    end
    return false
end

-- Export the module
return validator
