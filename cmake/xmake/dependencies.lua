-- QtForge Dependency Management Module
-- Centralized Qt and other dependency management for examples
-- Version: 3.2.0

-- Module table
local dependencies = {}

-- Common Qt package configuration
local qt_config = {
    optional = true,
    configs = {
        shared = true,
        runtimes = is_plat("windows") and "MD" or nil
    }
}

-- Qt package definitions
dependencies.qt_packages = {
    core = "qt6core",
    widgets = "qt6widgets",
    gui = "qt6gui",
    network = "qt6network",
    sql = "qt6sql",
    concurrent = "qt6concurrent",
    statemachine = "qt6statemachine",
    websockets = "qt6websockets",
    httpserver = "qt6httpserver"
}

-- Qt framework mappings
dependencies.qt_frameworks = {
    core = "QtCore",
    widgets = "QtWidgets",
    gui = "QtGui",
    network = "QtNetwork",
    sql = "QtSql",
    concurrent = "QtConcurrent",
    statemachine = "QtStateMachine",
    websockets = "QtWebSockets",
    httpserver = "QtHttpServer"
}

-- Add Qt requirements based on feature list
function dependencies.add_qt_requirements(features)
    features = features or {"core"}

    for _, feature in ipairs(features) do
        local package = dependencies.qt_packages[feature]
        if package then
            add_requires(package, qt_config)
        else
            print("Warning: Unknown Qt feature '" .. feature .. "'")
        end
    end
end

-- Add Qt packages to target based on feature list
function dependencies.add_qt_packages(features)
    features = features or {"core"}

    local packages = {}
    for _, feature in ipairs(features) do
        local package = dependencies.qt_packages[feature]
        if package then
            table.insert(packages, package)
        end
    end

    if #packages > 0 then
        add_packages(packages)
    end
end

-- Add Qt frameworks to target based on feature list
function dependencies.add_qt_frameworks(features)
    features = features or {"core"}

    local frameworks = {}
    for _, feature in ipairs(features) do
        local framework = dependencies.qt_frameworks[feature]
        if framework then
            table.insert(frameworks, framework)
        end
    end

    if #frameworks > 0 then
        add_frameworks(frameworks)
    end
end

-- Predefined Qt feature sets for common use cases
dependencies.feature_sets = {
    basic = {"core"},
    plugin = {"core"},
    ui = {"core", "widgets", "gui"},
    network = {"core", "network"},
    full_ui = {"core", "widgets", "gui", "network"},
    comprehensive = {"core", "widgets", "gui", "network", "sql", "concurrent"},
    monitoring = {"core"},
    service = {"core", "network"},
    web = {"core", "network", "websockets", "httpserver"}
}

-- Setup Qt dependencies for a specific use case
function dependencies.setup_qt(feature_set_name)
    local features = dependencies.feature_sets[feature_set_name]
    if not features then
        print("Warning: Unknown feature set '" .. feature_set_name .. "', using 'basic'")
        features = dependencies.feature_sets.basic
    end

    dependencies.add_qt_requirements(features)
    return features
end

-- Apply Qt dependencies to current target
function dependencies.apply_qt(features)
    dependencies.add_qt_packages(features)
    dependencies.add_qt_frameworks(features)
end

-- Setup Qt rules based on target type and features
function dependencies.setup_qt_rules(target_type, features)
    features = features or {"core"}

    if target_type == "plugin" then
        add_rules("qt.shared")
    elseif target_type == "console" then
        add_rules("qt.console")
    elseif target_type == "widgetapp" then
        add_rules("qt.widgetapp")
    elseif target_type == "shared" then
        add_rules("qt.shared")
    else
        -- Default to shared for plugins
        add_rules("qt.shared")
    end
end

-- Other common dependencies
dependencies.other_packages = {
    python3 = {
        name = "python3",
        config = {
            optional = true,
            configs = {version = ">=3.8"}
        }
    },
    pybind11 = {
        name = "pybind11",
        config = {
            optional = true,
            configs = {version = ">=2.6.0"}
        }
    },
    lua = {
        name = "lua",
        config = {
            optional = true,
            configs = {version = "5.4"}
        }
    },
    sol2 = {
        name = "sol2",
        config = {
            optional = true,
            configs = {version = ">=3.3.0"}
        }
    },
    gtest = {
        name = "gtest",
        config = {optional = true}
    },
    benchmark = {
        name = "benchmark",
        config = {optional = true}
    }
}

-- Add other dependencies
function dependencies.add_other_requirements(deps)
    for _, dep in ipairs(deps) do
        local package_info = dependencies.other_packages[dep]
        if package_info then
            add_requires(package_info.name, package_info.config)
        else
            print("Warning: Unknown dependency '" .. dep .. "'")
        end
    end
end

-- Apply other dependencies to target
function dependencies.apply_other(deps)
    for _, dep in ipairs(deps) do
        local package_info = dependencies.other_packages[dep]
        if package_info then
            add_packages(package_info.name)
        end
    end
end

-- Complete dependency setup for common example types
function dependencies.setup_plugin_example(qt_features, other_deps)
    qt_features = qt_features or "plugin"
    other_deps = other_deps or {}

    local features = dependencies.setup_qt(qt_features)
    dependencies.add_other_requirements(other_deps)

    return features, other_deps
end

function dependencies.setup_application_example(qt_features, other_deps)
    qt_features = qt_features or "ui"
    other_deps = other_deps or {}

    local features = dependencies.setup_qt(qt_features)
    dependencies.add_other_requirements(other_deps)

    return features, other_deps
end

-- Enhanced Qt feature detection and validation

-- Check if a Qt package is available
function dependencies.is_qt_package_available(package_name)
    return has_package(package_name)
end

-- Validate Qt feature requirements
function dependencies.validate_qt_features(features)
    local missing = {}
    local available = {}

    for _, feature in ipairs(features) do
        local package = dependencies.qt_packages[feature]
        if package then
            if dependencies.is_qt_package_available(package) then
                table.insert(available, feature)
            else
                table.insert(missing, feature)
            end
        else
            table.insert(missing, feature .. " (unknown feature)")
        end
    end

    return available, missing
end

-- Get Qt feature status report
function dependencies.get_qt_status_report(features)
    local available, missing = dependencies.validate_qt_features(features)

    local report = {
        requested = features,
        available = available,
        missing = missing,
        all_available = #missing == 0
    }

    return report
end

-- Enhanced dependency setup with validation
function dependencies.setup_qt_with_validation(feature_set_name)
    local features = dependencies.feature_sets[feature_set_name]
    if not features then
        print("Warning: Unknown feature set '" .. feature_set_name .. "', using 'basic'")
        features = dependencies.feature_sets.basic
    end

    -- Add requirements
    dependencies.add_qt_requirements(features)

    -- Validate and report
    local report = dependencies.get_qt_status_report(features)

    if #report.missing > 0 then
        print("Warning: Some Qt features are not available:")
        for _, missing in ipairs(report.missing) do
            print("  - " .. missing)
        end
    end

    if #report.available > 0 then
        print("Available Qt features:")
        for _, available in ipairs(report.available) do
            print("  + " .. available)
        end
    end

    return features, report
end

-- Conditional dependency application
function dependencies.apply_qt_conditional(features, condition_func)
    condition_func = condition_func or function() return true end

    if condition_func() then
        dependencies.apply_qt(features)
        return true
    else
        print("Skipping Qt dependencies due to condition")
        return false
    end
end

-- Enhanced error handling for missing dependencies
function dependencies.handle_missing_dependencies(missing_deps, fallback_action)
    if #missing_deps == 0 then
        return true
    end

    print("Missing dependencies detected:")
    for _, dep in ipairs(missing_deps) do
        print("  - " .. dep)
    end

    if fallback_action then
        print("Applying fallback action...")
        fallback_action(missing_deps)
    end

    return false
end

-- Dependency conflict detection
function dependencies.detect_conflicts(features)
    local conflicts = {}

    -- Example conflict detection (can be extended)
    if table.contains(features, "widgets") and not table.contains(features, "gui") then
        table.insert(conflicts, "widgets requires gui")
    end

    return conflicts
end

-- Smart dependency resolution
function dependencies.resolve_dependencies(features)
    local resolved = {}

    -- Add features and their dependencies
    for _, feature in ipairs(features) do
        table.insert(resolved, feature)

        -- Add implicit dependencies
        if feature == "widgets" and not table.contains(resolved, "gui") then
            table.insert(resolved, "gui")
        end
        if feature == "gui" and not table.contains(resolved, "core") then
            table.insert(resolved, "core")
        end
        if feature == "network" and not table.contains(resolved, "core") then
            table.insert(resolved, "core")
        end
    end

    -- Remove duplicates
    local unique = {}
    local seen = {}
    for _, feature in ipairs(resolved) do
        if not seen[feature] then
            table.insert(unique, feature)
            seen[feature] = true
        end
    end

    return unique
end

-- Utility function to check if table contains value
function table.contains(tbl, value)
    for _, v in ipairs(tbl) do
        if v == value then
            return true
        end
    end
    return false
end

-- Enhanced setup functions with better error handling
function dependencies.setup_plugin_example_enhanced(qt_features, other_deps, validation_options)
    qt_features = qt_features or "plugin"
    other_deps = other_deps or {}
    validation_options = validation_options or {}

    -- Resolve dependencies
    local feature_list = dependencies.feature_sets[qt_features] or {qt_features}
    local resolved_features = dependencies.resolve_dependencies(feature_list)

    -- Detect conflicts
    local conflicts = dependencies.detect_conflicts(resolved_features)
    if #conflicts > 0 then
        print("Dependency conflicts detected:")
        for _, conflict in ipairs(conflicts) do
            print("  ! " .. conflict)
        end
        if validation_options.strict then
            error("Dependency conflicts found in strict mode")
        end
    end

    -- Setup with validation
    local features, report = dependencies.setup_qt_with_validation(qt_features)
    dependencies.add_other_requirements(other_deps)

    return features, other_deps, report
end

-- Enhanced Qt6 tools configuration for cross-toolchain support

-- Configure Qt6 build tools (MOC, UIC, RCC) for active toolchain
function dependencies.configure_qt6_tools(toolchain)
    local qt6_detector = import("qt6_detector")

    if not qt6_detector.detection_results[toolchain] or not qt6_detector.detection_results[toolchain].available then
        print("Warning: Qt6 not available for " .. toolchain .. " toolchain")
        return false
    end

    local tools = qt6_detector.detection_results[toolchain].tools

    print("Configuring Qt6 build tools for " .. toolchain .. ":")

    -- Configure MOC (Meta-Object Compiler)
    if tools.moc then
        set_config("qt.moc", tools.moc)
        print("  MOC: " .. tools.moc)

        -- Set MOC flags for QtForge
        set_values("qt.moc.flags", {
            "-DQTFORGE_VERSION_MAJOR=3",
            "-DQTFORGE_VERSION_MINOR=2",
            "-DQTFORGE_VERSION_PATCH=0"
        })
    end

    -- Configure UIC (User Interface Compiler)
    if tools.uic then
        set_config("qt.uic", tools.uic)
        print("  UIC: " .. tools.uic)
    end

    -- Configure RCC (Resource Compiler)
    if tools.rcc then
        set_config("qt.rcc", tools.rcc)
        print("  RCC: " .. tools.rcc)
    end

    -- Configure QMake (for compatibility)
    if tools.qmake6 then
        set_config("qt.qmake", tools.qmake6)
        print("  QMake: " .. tools.qmake6)
    end

    return true
end

-- Get toolchain-specific Qt6 package configuration
function dependencies.get_toolchain_qt_config(toolchain)
    local config = {
        optional = true,
        configs = {
            shared = true
        }
    }

    -- Add toolchain-specific configurations
    if toolchain == "msvc" then
        config.configs.runtimes = "MD"
    elseif toolchain == "mingw64" then
        -- MinGW64 specific configurations
        config.configs.pic = true
    end

    return config
end

-- Enhanced Qt6 setup with toolchain awareness
function dependencies.setup_qt6_with_toolchain(toolchain, features)
    local qt6_detector = import("qt6_detector")

    print("Setting up Qt6 for " .. toolchain .. " toolchain...")

    -- Special handling for MinGW64
    if toolchain == "mingw64" then
        local msys2_mingw64 = import("msys2_mingw64")

        -- Validate MSYS2 environment first
        local validation = msys2_mingw64.validate_environment(features)
        if not validation.valid then
            print("MSYS2 environment validation failed:")
            for _, issue in ipairs(validation.issues) do
                print("  - " .. issue)
            end
            print("Suggestions:")
            for _, suggestion in ipairs(validation.suggestions) do
                print("  - " .. suggestion)
            end
            -- Continue anyway, but warn user
            print("Continuing with Qt6 setup despite validation issues...")
        end
    end

    -- Detect and configure Qt6
    if not qt6_detector.setup(toolchain) then
        print("Warning: Failed to setup Qt6 for " .. toolchain)
        return false
    end

    -- Configure Qt6 tools
    if not dependencies.configure_qt6_tools(toolchain) then
        print("Warning: Failed to configure Qt6 tools for " .. toolchain)
    end

    -- Apply Qt6 features
    local qt_info = qt6_detector.get_info(toolchain)
    if qt_info.available then
        print("Qt6 setup completed:")
        print("  Version: " .. (qt_info.version or "unknown"))
        print("  Available components: " .. table.concat(qt_info.components, ", "))

        -- Configure Qt6 rules for xmake
        add_rules("qt.shared")

        -- Apply toolchain-specific Qt6 configurations
        dependencies.apply_toolchain_qt6_config(toolchain)

        return true
    end

    return false
end

-- Apply toolchain-specific Qt6 configurations
function dependencies.apply_toolchain_qt6_config(toolchain)
    if toolchain == "mingw64" then
        -- MinGW64-specific Qt6 configurations
        add_defines("QT_NO_DEBUG_OUTPUT") -- Reduce debug output in release builds

        -- Add MinGW64-specific Qt6 link libraries
        add_syslinks("user32", "gdi32", "shell32", "ole32", "oleaut32", "uuid", "winmm", "ws2_32")

        -- Set Qt6 plugin path for MinGW64
        local msys2_mingw64 = import("msys2_mingw64")
        if msys2_mingw64.detected.qt_installation then
            local plugin_path = path.join(msys2_mingw64.detected.qt_installation, "plugins")
            if os.isdir(plugin_path) then
                add_defines("QT_PLUGIN_PATH=\"" .. plugin_path:gsub("\\", "/") .. "\"")
            end
        end

    elseif toolchain == "msvc" then
        -- MSVC-specific Qt6 configurations
        add_defines("QT_NO_DEBUG_OUTPUT") -- Reduce debug output in release builds

        -- MSVC-specific runtime configurations
        if is_mode("debug") then
            set_runtimes("MDd")
        else
            set_runtimes("MD")
        end
    end
end

-- Validate Qt6 tools for cross-compilation
function dependencies.validate_qt6_tools(toolchain)
    local qt6_detector = import("qt6_detector")
    local tools = qt6_detector.detection_results[toolchain] and qt6_detector.detection_results[toolchain].tools or {}

    local validation_results = {
        valid = true,
        missing_tools = {},
        available_tools = {}
    }

    -- Check essential tools
    local essential_tools = {"moc", "uic", "rcc"}

    for _, tool in ipairs(essential_tools) do
        if tools[tool] and os.isfile(tools[tool]) then
            table.insert(validation_results.available_tools, tool)
        else
            table.insert(validation_results.missing_tools, tool)
            validation_results.valid = false
        end
    end

    return validation_results
end

-- Get Qt6 library linking configuration for toolchain
function dependencies.get_qt6_linking_config(toolchain, components)
    local config = {
        libraries = {},
        link_dirs = {},
        frameworks = {}
    }

    local qt6_detector = import("qt6_detector")
    local qt_info = qt6_detector.get_info(toolchain)

    if not qt_info.available then
        return config
    end

    -- Add Qt6 library directory
    table.insert(config.link_dirs, path.join(qt_info.path, "lib"))

    -- Add component libraries
    for _, component in ipairs(components) do
        if toolchain == "msvc" then
            table.insert(config.libraries, "Qt6" .. component)
        elseif toolchain == "mingw64" then
            table.insert(config.libraries, "Qt6" .. component)
        end
    end

    return config
end

-- Comprehensive toolchain and dependency validation

-- Validate all dependencies for a specific toolchain
function dependencies.validate_all_dependencies(toolchain, required_features)
    local validation_results = {
        toolchain_valid = false,
        qt6_available = false,
        qt6_tools_valid = false,
        missing_packages = {},
        suggestions = {},
        overall_valid = false
    }

    print("Validating dependencies for " .. toolchain .. " toolchain...")

    -- Validate toolchain
    if toolchain == "mingw64" then
        local common = import("common")
        local toolchain_validation = common.validate_mingw64_toolchain()
        validation_results.toolchain_valid = toolchain_validation.valid

        if not toolchain_validation.valid then
            for _, issue in ipairs(toolchain_validation.issues) do
                table.insert(validation_results.suggestions, "Toolchain: " .. issue)
            end
        end

        -- Validate MSYS2 packages
        local msys2_mingw64 = import("msys2_mingw64")
        local package_validation = msys2_mingw64.validate_environment(required_features)

        if not package_validation.valid then
            for _, suggestion in ipairs(package_validation.suggestions) do
                table.insert(validation_results.suggestions, suggestion)
            end
        end

    elseif toolchain == "msvc" then
        -- MSVC validation (basic check)
        local toolchain_selector = import("toolchain_selector")
        validation_results.toolchain_valid = toolchain_selector.detection_results.msvc.available

        if not validation_results.toolchain_valid then
            table.insert(validation_results.suggestions, "Install Visual Studio with C++ development tools")
        end
    end

    -- Validate Qt6
    local qt6_detector = import("qt6_detector")
    local qt_info = qt6_detector.get_info(toolchain)
    validation_results.qt6_available = qt_info.available

    if qt_info.available then
        -- Validate Qt6 tools
        local tools_validation = dependencies.validate_qt6_tools(toolchain)
        validation_results.qt6_tools_valid = tools_validation.valid
        validation_results.missing_packages = tools_validation.missing_tools
    else
        table.insert(validation_results.suggestions, "Install Qt6 for " .. toolchain .. " toolchain")
    end

    -- Overall validation
    validation_results.overall_valid = validation_results.toolchain_valid and
                                      validation_results.qt6_available and
                                      validation_results.qt6_tools_valid

    -- Report results
    print("Dependency validation results:")
    print("  Toolchain: " .. (validation_results.toolchain_valid and "✓" or "✗"))
    print("  Qt6: " .. (validation_results.qt6_available and "✓" or "✗"))
    print("  Qt6 Tools: " .. (validation_results.qt6_tools_valid and "✓" or "✗"))

    if #validation_results.suggestions > 0 then
        print("  Suggestions:")
        for _, suggestion in ipairs(validation_results.suggestions) do
            print("    - " .. suggestion)
        end
    end

    return validation_results
end

-- Auto-detect best toolchain and setup dependencies
function dependencies.auto_setup_dependencies(required_features)
    local toolchain_selector = import("toolchain_selector")

    print("Auto-detecting and setting up dependencies...")

    -- Detect all available toolchains
    if not toolchain_selector.detect_all() then
        print("Error: No supported toolchains found")
        return false
    end

    -- Select best toolchain
    if not toolchain_selector.select_toolchain() then
        print("Error: Failed to select toolchain")
        return false
    end

    local selected_toolchain = toolchain_selector.selected.name
    print("Selected toolchain: " .. selected_toolchain)

    -- Validate dependencies for selected toolchain
    local validation = dependencies.validate_all_dependencies(selected_toolchain, required_features)

    if validation.overall_valid then
        -- Setup dependencies
        return dependencies.setup_qt6_with_toolchain(selected_toolchain, required_features)
    else
        print("Dependency validation failed for " .. selected_toolchain .. " toolchain")

        -- Try fallback toolchain if available
        local fallback_toolchain = nil
        if selected_toolchain == "msvc" and toolchain_selector.detection_results.mingw64.available then
            fallback_toolchain = "mingw64"
        elseif selected_toolchain == "mingw64" and toolchain_selector.detection_results.msvc.available then
            fallback_toolchain = "msvc"
        end

        if fallback_toolchain then
            print("Trying fallback toolchain: " .. fallback_toolchain)
            local fallback_validation = dependencies.validate_all_dependencies(fallback_toolchain, required_features)

            if fallback_validation.overall_valid then
                toolchain_selector.selected.name = fallback_toolchain
                return dependencies.setup_qt6_with_toolchain(fallback_toolchain, required_features)
            end
        end

        print("No valid toolchain configuration found")
        return false
    end
end

-- Get dependency summary for current configuration
function dependencies.get_dependency_summary()
    local toolchain_selector = import("toolchain_selector")
    local qt6_detector = import("qt6_detector")

    local summary = {
        active_toolchain = toolchain_selector.selected.name or "none",
        toolchain_configured = toolchain_selector.selected.configured,
        qt6_support = toolchain_selector.selected.qt_support,
        qt6_info = {},
        build_features = {}
    }

    -- Get Qt6 information
    if summary.active_toolchain and summary.active_toolchain ~= "none" then
        summary.qt6_info = qt6_detector.get_info(summary.active_toolchain)
    end

    -- Get build features
    local features = {"ui", "network", "sql", "concurrent"}
    for _, feature in ipairs(features) do
        summary.build_features[feature] = has_config(feature)
    end

    return summary
end

-- Export the module
return dependencies
