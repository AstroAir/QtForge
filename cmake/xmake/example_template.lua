-- QtForge Example Template Module
-- Standardized example application target creation functions
-- Version: 3.2.0

-- Import dependencies
local common = import("common")
local dependencies = import("dependencies")

-- Module table
local example_template = {}

-- Default example configuration
local default_config = {
    kind = "binary",
    qt_features = "basic",
    output_dir = "$(buildir)/bin",
    version = "1.0.0",
    qtforge_deps = {"QtForgeCore"},
    debug_postfix = true,
    install_binary = false,
    install_config = false
}

-- Create a basic example application
function example_template.create_example(name, config)
    config = config or {}

    -- Merge with defaults
    for k, v in pairs(default_config) do
        if config[k] == nil then
            config[k] = v
        end
    end

    -- Validate required parameters
    if not name then
        error("Example name is required")
    end

    if not config.sources then
        error("Example sources are required")
    end

    -- Create target
    target(name)
        set_kind(config.kind)

        -- Set basename with debug postfix if enabled
        if config.debug_postfix then
            common.set_debug_postfix(config.basename or name:lower())
        else
            set_basename(config.basename or name:lower())
        end

        -- Setup Qt rules based on application type
        local app_type = config.app_type or "console"
        dependencies.setup_qt_rules(app_type, config.qt_features)
        dependencies.apply_qt(config.qt_features)

        -- Add source files
        if type(config.sources) == "string" then
            add_files(config.sources)
        else
            for _, source in ipairs(config.sources) do
                add_files(source)
            end
        end

        -- Add header files if specified
        if config.headers then
            if type(config.headers) == "string" then
                add_headerfiles(config.headers)
            else
                for _, header in ipairs(config.headers) do
                    add_headerfiles(header)
                end
            end
        end

        -- Add QtForge dependencies with toolchain support
        local active_toolchain = common.detect_active_toolchain()
        common.add_qtforge_deps_with_toolchain(active_toolchain)
        common.add_qtforge_includes()

        -- Apply toolchain-specific example configurations
        example_template.apply_toolchain_config(active_toolchain, config)

        -- Set output directory
        set_targetdir(config.output_dir)

        -- Set version
        set_version(config.version)

        -- Add custom defines if specified
        if config.defines then
            add_defines(config.defines)
        end

        -- Install binary if specified
        if config.install_binary then
            add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin"})
        end

        -- Add custom configuration if provided
        if config.custom_config then
            config.custom_config()
        end

        -- Add after_build actions if specified
        if config.after_build then
            after_build(config.after_build)
        end

    target_end()
end

-- Create a console example application
function example_template.create_console_example(name, config)
    config = config or {}
    config.app_type = "console"
    config.qt_features = config.qt_features or "basic"

    return example_template.create_example(name, config)
end

-- Create a widget-based example application
function example_template.create_widget_example(name, config)
    config = config or {}
    config.app_type = "widgetapp"
    config.qt_features = config.qt_features or "ui"

    return example_template.create_example(name, config)
end

-- Create a comprehensive example with multiple components
function example_template.create_comprehensive_example(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "comprehensive"
    config.qtforge_deps = config.qtforge_deps or {"QtForgeCore", "QtForgeSecurity"}
    config.app_type = config.app_type or "widgetapp"

    -- Add default after_build action for comprehensive examples
    if not config.after_build then
        config.after_build = function(target)
            -- Copy config files if they exist
            if os.isdir("config") then
                os.cp("config", target:targetdir())
            end

            -- Copy Python scripts if they exist
            if os.isdir("python") then
                os.cp("python", target:targetdir())
            end

            -- Create plugins directory
            os.mkdir(path.join(target:targetdir(), "plugins"))
        end
    end

    return example_template.create_example(name, config)
end

-- Create a network example application
function example_template.create_network_example(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "network"
    config.app_type = config.app_type or "console"

    return example_template.create_example(name, config)
end

-- Create a monitoring example application
function example_template.create_monitoring_example(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "monitoring"
    config.app_type = config.app_type or "console"

    return example_template.create_example(name, config)
end

-- Helper function to setup common example project
function example_template.setup_example_project(name, version, qt_features)
    common.setup_project(name, version)
    common.setup_compiler()
    common.setup_warnings()
    common.add_version_defines()

    -- Setup Qt dependencies
    local features = dependencies.setup_qt(qt_features or "basic")
    return features
end

-- Create example with subdirectory includes
function example_template.create_example_with_subdirs(name, subdirs, config)
    config = config or {}

    -- Create main example
    example_template.create_example(name, config)

    -- Include subdirectories
    if subdirs then
        for _, subdir in ipairs(subdirs) do
            if os.isdir(subdir) and os.isfile(path.join(subdir, "xmake.lua")) then
                includes(subdir)
            end
        end
    end
end

-- Add test configuration to example
function example_template.add_test_config(test_name, config)
    config = config or {}

    add_tests(test_name, {
        kind = config.kind or "binary",
        files = config.files or "main.cpp",
        timeout = config.timeout or 60,
        labels = config.labels or {"example", "test"}
    })
end

-- Install documentation files
function example_template.install_docs(docs, prefix)
    docs = docs or {"README.md"}
    prefix = prefix or "share/doc/qtforge/examples"

    for _, doc in ipairs(docs) do
        if os.isfile(doc) then
            add_installfiles(doc, {prefixdir = prefix})
        end
    end
end

-- Setup example with common patterns
function example_template.setup_standard_example(name, version, sources, qt_features, config)
    config = config or {}

    -- Setup project
    example_template.setup_example_project(name, version, qt_features)

    -- Create example with sources
    config.sources = sources
    config.qt_features = qt_features
    example_template.create_example(name, config)

    -- Install documentation if it exists
    if os.isfile("README.md") then
        example_template.install_docs({"README.md"}, "share/doc/qtforge/examples/" .. name:lower())
    end
end

-- Enhanced application templates with better configuration

-- Create a console application example
function example_template.create_console_example(name, config)
    config = config or {}
    config.kind = "binary"
    config.app_type = "console"
    config.qt_features = config.qt_features or "basic"

    -- Add console-specific configuration
    if not config.custom_config then
        config.custom_config = function()
            -- Console applications don't need GUI
            add_defines("QT_NO_GUI")

            -- Add console-specific settings
            if is_plat("windows") then
                add_ldflags("/SUBSYSTEM:CONSOLE")
            end
        end
    end

    return example_template.create_example(name, config)
end

-- Create a GUI application example
function example_template.create_gui_example(name, config)
    config = config or {}
    config.kind = "binary"
    config.app_type = "widgetapp"
    config.qt_features = config.qt_features or "ui"

    -- Add GUI-specific configuration
    if not config.custom_config then
        config.custom_config = function()
            -- GUI applications need widgets
            if not has_package("qt6widgets") then
                print("Warning: Qt6Widgets not available for GUI application")
            end

            -- Add GUI-specific settings
            if is_plat("windows") then
                add_ldflags("/SUBSYSTEM:WINDOWS")
            end
        end
    end

    return example_template.create_example(name, config)
end

-- Create a service/daemon example
function example_template.create_service_example(name, config)
    config = config or {}
    config.kind = "binary"
    config.app_type = "console"
    config.qt_features = config.qt_features or "service"

    -- Add service-specific configuration
    if not config.custom_config then
        config.custom_config = function()
            -- Services don't need GUI
            add_defines("QT_NO_GUI", "QTFORGE_SERVICE_APPLICATION")

            -- Add service-specific libraries
            if is_plat("windows") then
                add_syslinks("advapi32")  -- For Windows services
            elseif is_plat("linux") then
                add_syslinks("systemd")   -- For systemd integration
            end
        end
    end

    return example_template.create_example(name, config)
end

-- Create a benchmark/performance example
function example_template.create_benchmark_example(name, config)
    config = config or {}
    config.kind = "binary"
    config.app_type = "console"
    config.qt_features = config.qt_features or "basic"
    config.output_dir = config.output_dir or "$(buildir)/benchmark"

    -- Add benchmark-specific configuration
    if not config.custom_config then
        config.custom_config = function()
            add_defines("QTFORGE_BENCHMARK_APPLICATION")

            -- Add benchmark framework if available
            if has_package("benchmark") then
                add_packages("benchmark")
                add_defines("QTFORGE_HAS_BENCHMARK")
            end

            -- Optimization flags for benchmarks
            if is_mode("release") then
                add_cxflags("-O3", "-DNDEBUG")
                if is_plat("linux") or is_plat("macosx") then
                    add_cxflags("-march=native")
                end
            end
        end
    end

    return example_template.create_example(name, config)
end

-- Create a test application example
function example_template.create_test_example(name, config)
    config = config or {}
    config.kind = "binary"
    config.app_type = "console"
    config.qt_features = config.qt_features or "basic"
    config.output_dir = config.output_dir or "$(buildir)/test"
    config.install_binary = config.install_binary or false

    -- Add test-specific configuration
    if not config.custom_config then
        config.custom_config = function()
            add_defines("QTFORGE_TEST_APPLICATION")

            -- Add test framework
            if has_package("gtest") then
                add_packages("gtest")
                add_defines("QTFORGE_HAS_GTEST")
            end

            -- Test applications are not installed by default
            set_default(false)
        end
    end

    return example_template.create_example(name, config)
end

-- Create a multi-target example (plugin + application)
function example_template.create_multi_target_example(name, targets_config)
    targets_config = targets_config or {}

    for target_name, target_config in pairs(targets_config) do
        local target_type = target_config.type or "example"

        if target_type == "plugin" then
            -- Import plugin_template in the target context
            target(target_name)
                on_config(function(target)
                    local plugin_template = import("plugin_template")
                    plugin_template.create_plugin(target_name, target_config)
                end)
            target_end()
        else
            example_template.create_example(target_name, target_config)
        end
    end
end

-- Enhanced example creation with validation
function example_template.create_validated_example(name, config)
    config = config or {}

    -- Validate configuration
    local required_fields = {"sources"}
    for _, field in ipairs(required_fields) do
        if not config[field] then
            error("Missing required field '" .. field .. "' for example '" .. name .. "'")
        end
    end

    -- Validate source files exist
    local sources = type(config.sources) == "string" and {config.sources} or config.sources
    local missing_sources = {}

    for _, source in ipairs(sources) do
        if not os.isfile(source) then
            table.insert(missing_sources, source)
        end
    end

    if #missing_sources > 0 then
        print("Warning: Missing source files for example '" .. name .. "':")
        for _, missing in ipairs(missing_sources) do
            print("  - " .. missing)
        end

        if config.strict_validation then
            error("Missing source files in strict validation mode")
        end
    end

    return example_template.create_example(name, config)
end

-- Create example with automatic type detection
function example_template.create_auto_example(name, config)
    config = config or {}

    -- Auto-detect example type
    local example_type = config.example_type

    if not example_type then
        local name_lower = name:lower()
        if name_lower:find("gui") or name_lower:find("widget") or name_lower:find("ui") then
            example_type = "gui"
        elseif name_lower:find("console") or name_lower:find("cli") then
            example_type = "console"
        elseif name_lower:find("service") or name_lower:find("daemon") then
            example_type = "service"
        elseif name_lower:find("benchmark") or name_lower:find("perf") then
            example_type = "benchmark"
        elseif name_lower:find("test") then
            example_type = "test"
        else
            example_type = "console"  -- Default to console
        end
    end

    -- Create example based on detected type
    if example_type == "gui" then
        return example_template.create_gui_example(name, config)
    elseif example_type == "console" then
        return example_template.create_console_example(name, config)
    elseif example_type == "service" then
        return example_template.create_service_example(name, config)
    elseif example_type == "benchmark" then
        return example_template.create_benchmark_example(name, config)
    elseif example_type == "test" then
        return example_template.create_test_example(name, config)
    else
        return example_template.create_example(name, config)
    end
end

-- Apply toolchain-specific configurations to example
function example_template.apply_toolchain_config(toolchain, config)
    if toolchain == "mingw64" then
        -- MinGW64-specific example configurations
        add_defines("QTFORGE_EXAMPLE_MINGW64")

        -- Add MinGW64-specific compiler flags for examples
        add_cxxflags("-fPIC")

        -- MinGW64-specific linking for examples
        if config.app_type == "gui" then
            -- GUI applications need additional Windows libraries
            add_syslinks("user32", "gdi32", "shell32", "ole32", "oleaut32", "uuid")
        end

        -- Console applications
        if config.app_type == "console" then
            add_syslinks("kernel32")
        end

    elseif toolchain == "msvc" then
        -- MSVC-specific example configurations
        add_defines("QTFORGE_EXAMPLE_MSVC")

        -- MSVC-specific compiler flags for examples
        add_cxxflags("/wd4996") -- Disable deprecated function warnings

        -- MSVC-specific linking
        if config.app_type == "gui" then
            add_syslinks("user32", "gdi32", "shell32", "ole32", "oleaut32", "uuid")
        end
    end

    -- Common example configurations
    add_defines("QTFORGE_EXAMPLE_BUILD")
    add_defines("QTFORGE_ACTIVE_TOOLCHAIN=\"" .. toolchain .. "\"")

    -- Application type specific configurations
    if config.app_type == "gui" then
        add_defines("QTFORGE_EXAMPLE_GUI")
    elseif config.app_type == "console" then
        add_defines("QTFORGE_EXAMPLE_CONSOLE")
    elseif config.app_type == "service" then
        add_defines("QTFORGE_EXAMPLE_SERVICE")
    end
end

-- Validate example configuration for toolchain
function example_template.validate_example_config(config, toolchain)
    local validation = {
        valid = true,
        warnings = {},
        errors = {}
    }

    -- Check for toolchain-specific issues
    if toolchain == "mingw64" then
        -- MinGW64-specific validations
        local common = import("common")
        if not common.is_mingw64_available() then
            table.insert(validation.errors, "MinGW64 toolchain not available")
            validation.valid = false
        end

        -- Check Qt6 availability for GUI examples
        if config.app_type == "gui" then
            local msys2_mingw64 = import("msys2_mingw64")
            if not msys2_mingw64.detected.qt_installation then
                table.insert(validation.errors, "Qt6 not available for GUI examples with MinGW64")
                validation.valid = false
            end
        end

    elseif toolchain == "msvc" then
        -- MSVC-specific validations
        if config.app_type == "gui" and not has_package("qt6widgets") then
            table.insert(validation.warnings, "Qt6 Widgets may not be available for GUI examples with MSVC")
        end
    end

    -- Common validations
    if not config.sources then
        table.insert(validation.errors, "Example sources are required")
        validation.valid = false
    end

    if config.app_type == "gui" and not config.qt_features then
        table.insert(validation.warnings, "GUI examples should specify qt_features")
    end

    return validation
end

-- Create example with enhanced toolchain validation
function example_template.create_validated_example(name, config)
    config = config or {}

    -- Detect active toolchain
    local common = import("common")
    local active_toolchain = common.detect_active_toolchain()

    -- Validate configuration
    local validation = example_template.validate_example_config(config, active_toolchain)

    if not validation.valid then
        print("Example validation failed for " .. name .. ":")
        for _, error in ipairs(validation.errors) do
            print("  Error: " .. error)
        end
        return false
    end

    if #validation.warnings > 0 then
        print("Example validation warnings for " .. name .. ":")
        for _, warning in ipairs(validation.warnings) do
            print("  Warning: " .. warning)
        end
    end

    -- Create example with validated configuration
    example_template.create_example(name, config)
    return true
end

-- Get toolchain-specific example recommendations
function example_template.get_toolchain_recommendations(toolchain)
    local recommendations = {
        suggested_configs = {},
        best_practices = {},
        common_issues = {}
    }

    if toolchain == "mingw64" then
        recommendations.suggested_configs = {
            app_type = "console", -- Console apps are more reliable with MinGW64
            qt_features = "basic", -- Start with basic Qt features
            debug_postfix = true -- Easier debugging
        }

        recommendations.best_practices = {
            "Use console applications for initial testing",
            "Ensure MSYS2 Qt6 packages are installed",
            "Test with both debug and release builds",
            "Use static linking when possible"
        }

        recommendations.common_issues = {
            "Missing Qt6 packages in MSYS2",
            "Path issues with Qt6 tools",
            "DLL loading issues with shared builds"
        }

    elseif toolchain == "msvc" then
        recommendations.suggested_configs = {
            app_type = "gui", -- GUI apps work well with MSVC
            qt_features = "ui", -- Full UI features available
            debug_postfix = true
        }

        recommendations.best_practices = {
            "Use GUI applications for rich interfaces",
            "Leverage Visual Studio debugging tools",
            "Use shared linking for smaller executables",
            "Enable all Qt features as needed"
        }

        recommendations.common_issues = {
            "Qt6 installation path detection",
            "Runtime library mismatches",
            "Missing Visual Studio components"
        }
    end

    return recommendations
end

-- Export the module
return example_template
