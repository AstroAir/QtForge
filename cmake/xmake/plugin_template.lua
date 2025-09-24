-- QtForge Plugin Template Module
-- Standardized plugin target creation functions
-- Version: 3.2.0

-- Import dependencies
local common = import("common")
local dependencies = import("dependencies")

-- Module table
local plugin_template = {}

-- Default plugin configuration
local default_config = {
    kind = "shared",
    qt_features = "plugin",
    output_dir = "$(buildir)/plugins",
    version = "1.0.0",
    qtforge_deps = {"QtForgeCore"},
    install_metadata = true,
    copy_metadata = true,
    debug_postfix = true,
    visibility = "hidden",
    plugin_defines = {"QT_PLUGIN"}
}

-- Create a basic plugin target
function plugin_template.create_plugin(name, config)
    config = config or {}

    -- Merge with defaults
    for k, v in pairs(default_config) do
        if config[k] == nil then
            config[k] = v
        end
    end

    -- Validate required parameters
    if not name then
        error("Plugin name is required")
    end

    if not config.sources then
        error("Plugin sources are required")
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

        -- Setup Qt rules and dependencies
        dependencies.setup_qt_rules("plugin", config.qt_features)
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

        -- Apply toolchain-specific plugin configurations
        plugin_template.apply_toolchain_config(active_toolchain, config)

        -- Set output directory
        set_targetdir(config.output_dir)

        -- Set version
        set_version(config.version)

        -- Set visibility
        if config.visibility then
            set_symbols(config.visibility)
        end

        -- Add plugin-specific defines
        if config.plugin_defines then
            add_defines(config.plugin_defines)
        end

        -- Add custom defines if specified
        if config.defines then
            add_defines(config.defines)
        end

        -- Install metadata file if specified
        if config.install_metadata and config.metadata_file then
            common.install_plugin_metadata(config.metadata_file)
        end

        -- Copy metadata to build directory if specified
        if config.copy_metadata and config.metadata_file then
            common.copy_plugin_metadata(config.metadata_file)
        end

        -- Add custom configuration if provided
        if config.custom_config then
            config.custom_config()
        end

    target_end()
end

-- Create a plugin with test target
function plugin_template.create_plugin_with_test(plugin_name, test_name, config)
    config = config or {}

    -- Create the plugin
    plugin_template.create_plugin(plugin_name, config)

    -- Create test target if test configuration is provided
    if config.test_config then
        local test_config = config.test_config
        test_config.qt_features = test_config.qt_features or config.qt_features or "plugin"

        target(test_name or (plugin_name .. "Test"))
            set_kind("binary")
            set_basename(test_config.basename or (plugin_name:lower() .. "_test"))

            -- Setup Qt rules for console application
            dependencies.setup_qt_rules("console", test_config.qt_features)
            dependencies.apply_qt(test_config.qt_features)

            -- Add test source files
            if test_config.sources then
                if type(test_config.sources) == "string" then
                    add_files(test_config.sources)
                else
                    for _, source in ipairs(test_config.sources) do
                        add_files(source)
                    end
                end
            end

            -- Add QtForge dependencies and plugin dependency
            local test_deps = test_config.qtforge_deps or {"QtForgeCore"}
            table.insert(test_deps, plugin_name)
            common.add_qtforge_deps(test_deps)
            common.add_qtforge_includes()

            -- Set output directory
            set_targetdir(test_config.output_dir or "$(buildir)/bin")

            -- Set version
            set_version(config.version)

            -- Add test-specific defines
            if test_config.defines then
                add_defines(test_config.defines)
            end

            -- Install test binary if specified
            if test_config.install then
                add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin"})
            end

        target_end()
    end
end

-- Create a specialized UI plugin
function plugin_template.create_ui_plugin(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "ui"
    config.plugin_defines = config.plugin_defines or {"QT_PLUGIN", "QTPLUGIN_BUILD_UI"}

    return plugin_template.create_plugin(name, config)
end

-- Create a specialized network plugin
function plugin_template.create_network_plugin(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "network"
    config.plugin_defines = config.plugin_defines or {"QT_PLUGIN", "QTPLUGIN_BUILD_NETWORK"}

    return plugin_template.create_plugin(name, config)
end

-- Create a comprehensive plugin with all features
function plugin_template.create_comprehensive_plugin(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "comprehensive"
    config.plugin_defines = config.plugin_defines or {
        "QT_PLUGIN",
        "QTPLUGIN_BUILD_UI",
        "QTPLUGIN_BUILD_NETWORK",
        "COMPREHENSIVE_PLUGIN_EXPORTS"
    }
    config.qtforge_deps = config.qtforge_deps or {"QtForgeCore", "QtForgeSecurity"}

    return plugin_template.create_plugin(name, config)
end

-- Create a monitoring plugin
function plugin_template.create_monitoring_plugin(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "monitoring"
    config.plugin_defines = config.plugin_defines or {"QT_PLUGIN", "QTPLUGIN_BUILD_MONITORING"}

    return plugin_template.create_plugin(name, config)
end

-- Helper function to setup common plugin project
function plugin_template.setup_plugin_project(name, version, qt_features)
    common.setup_project(name, version)
    common.setup_compiler()
    common.setup_warnings()
    common.add_version_defines()

    -- Setup Qt dependencies
    local features = dependencies.setup_qt(qt_features or "plugin")
    return features
end

-- Enhanced specialized plugin types

-- Create a service plugin (background processing, async operations)
function plugin_template.create_service_plugin(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "service"
    config.plugin_defines = config.plugin_defines or {"QT_PLUGIN", "QTPLUGIN_BUILD_SERVICE"}
    config.output_dir = config.output_dir or "$(buildir)/lib/qtplugin/services"

    -- Add service-specific configuration
    if not config.custom_config then
        config.custom_config = function()
            -- Service plugins often need custom extensions
            set_extension(".qtservice")
            set_prefixname("")

            -- Add service-specific defines
            add_defines("QTFORGE_SERVICE_PLUGIN")
        end
    end

    return plugin_template.create_plugin(name, config)
end

-- Create a data processing plugin
function plugin_template.create_data_plugin(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "plugin"
    config.plugin_defines = config.plugin_defines or {"QT_PLUGIN", "QTPLUGIN_BUILD_DATA"}

    -- Add data processing specific configuration
    if not config.custom_config then
        config.custom_config = function()
            add_defines("QTFORGE_DATA_PLUGIN")

            -- Data plugins might need concurrent processing
            if has_package("qt6concurrent") then
                add_packages("qt6concurrent")
                add_frameworks("QtConcurrent")
                add_defines("QTFORGE_HAS_CONCURRENT")
            end
        end
    end

    return plugin_template.create_plugin(name, config)
end

-- Create a communication plugin (messaging, networking)
function plugin_template.create_communication_plugin(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "network"
    config.plugin_defines = config.plugin_defines or {"QT_PLUGIN", "QTPLUGIN_BUILD_COMMUNICATION"}

    -- Add communication specific configuration
    if not config.custom_config then
        config.custom_config = function()
            add_defines("QTFORGE_COMMUNICATION_PLUGIN")

            -- Add network-related defines if available
            if has_package("qt6network") then
                add_defines("QTFORGE_HAS_NETWORK")
            end

            if has_package("qt6websockets") then
                add_packages("qt6websockets")
                add_frameworks("QtWebSockets")
                add_defines("QTFORGE_HAS_WEBSOCKETS")
            end
        end
    end

    return plugin_template.create_plugin(name, config)
end

-- Create a security plugin
function plugin_template.create_security_plugin(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "plugin"
    config.plugin_defines = config.plugin_defines or {"QT_PLUGIN", "QTPLUGIN_BUILD_SECURITY"}
    config.visibility = config.visibility or "hidden"  -- Security plugins should have hidden symbols

    -- Add security specific configuration
    if not config.custom_config then
        config.custom_config = function()
            add_defines("QTFORGE_SECURITY_PLUGIN")

            -- Add security-specific compiler flags
            if is_plat("windows") then
                add_cxflags("/GS", "/sdl")  -- Buffer security check, SDL checks
            else
                add_cxflags("-fstack-protector-strong", "-D_FORTIFY_SOURCE=2")
            end

            -- Strict warnings for security plugins
            if is_plat("windows") and is_config("toolchain", "msvc") then
                add_cxflags("/W4", "/WX")
            else
                add_cxflags("-Wall", "-Wextra", "-Werror")
            end
        end
    end

    return plugin_template.create_plugin(name, config)
end

-- Create a testing plugin (for unit tests, integration tests)
function plugin_template.create_test_plugin(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "plugin"
    config.plugin_defines = config.plugin_defines or {"QT_PLUGIN", "QTPLUGIN_BUILD_TEST"}
    config.output_dir = config.output_dir or "$(buildir)/test/plugins"

    -- Add test specific configuration
    if not config.custom_config then
        config.custom_config = function()
            add_defines("QTFORGE_TEST_PLUGIN")

            -- Add test framework if available
            if has_package("gtest") then
                add_packages("gtest")
                add_defines("QTFORGE_HAS_GTEST")
            end

            -- Test plugins are not installed by default
            set_default(false)
        end
    end

    return plugin_template.create_plugin(name, config)
end

-- Create a development/debug plugin
function plugin_template.create_debug_plugin(name, config)
    config = config or {}
    config.qt_features = config.qt_features or "plugin"
    config.plugin_defines = config.plugin_defines or {"QT_PLUGIN", "QTPLUGIN_BUILD_DEBUG"}
    config.debug_postfix = config.debug_postfix or true

    -- Add debug specific configuration
    if not config.custom_config then
        config.custom_config = function()
            add_defines("QTFORGE_DEBUG_PLUGIN")

            -- Debug plugins only in debug mode
            if not is_mode("debug") then
                set_enabled(false)
            end

            -- Add debug-specific flags
            if is_mode("debug") then
                add_defines("QTFORGE_ENABLE_DEBUG_OUTPUT")
                set_symbols("debug")
            end
        end
    end

    return plugin_template.create_plugin(name, config)
end

-- Enhanced plugin creation with automatic type detection
function plugin_template.create_auto_plugin(name, config)
    config = config or {}

    -- Auto-detect plugin type based on name or configuration
    local plugin_type = config.plugin_type

    if not plugin_type then
        local name_lower = name:lower()
        if name_lower:find("ui") or name_lower:find("widget") then
            plugin_type = "ui"
        elseif name_lower:find("network") or name_lower:find("communication") then
            plugin_type = "network"
        elseif name_lower:find("service") or name_lower:find("background") then
            plugin_type = "service"
        elseif name_lower:find("monitor") then
            plugin_type = "monitoring"
        elseif name_lower:find("security") then
            plugin_type = "security"
        elseif name_lower:find("test") then
            plugin_type = "test"
        elseif name_lower:find("debug") then
            plugin_type = "debug"
        else
            plugin_type = "basic"
        end
    end

    -- Create plugin based on detected type
    if plugin_type == "ui" then
        return plugin_template.create_ui_plugin(name, config)
    elseif plugin_type == "network" then
        return plugin_template.create_network_plugin(name, config)
    elseif plugin_type == "service" then
        return plugin_template.create_service_plugin(name, config)
    elseif plugin_type == "monitoring" then
        return plugin_template.create_monitoring_plugin(name, config)
    elseif plugin_type == "security" then
        return plugin_template.create_security_plugin(name, config)
    elseif plugin_type == "test" then
        return plugin_template.create_test_plugin(name, config)
    elseif plugin_type == "debug" then
        return plugin_template.create_debug_plugin(name, config)
    else
        return plugin_template.create_plugin(name, config)
    end
end

-- Apply toolchain-specific configurations to plugin
function plugin_template.apply_toolchain_config(toolchain, config)
    if toolchain == "mingw64" then
        -- MinGW64-specific plugin configurations
        add_defines("QTFORGE_PLUGIN_MINGW64")

        -- Add MinGW64-specific compiler flags for plugins
        add_cxxflags("-fvisibility=hidden") -- Hide symbols by default
        add_cxxflags("-fvisibility-inlines-hidden")

        -- Plugin export/import macros for MinGW64
        add_defines("QTFORGE_PLUGIN_EXPORT=__declspec(dllexport)")
        add_defines("QTFORGE_PLUGIN_IMPORT=__declspec(dllimport)")

        -- MinGW64-specific linking for plugins
        if config.kind == "shared" then
            add_ldflags("-Wl,--enable-auto-import")
            add_ldflags("-Wl,--export-all-symbols")
        end

    elseif toolchain == "msvc" then
        -- MSVC-specific plugin configurations
        add_defines("QTFORGE_PLUGIN_MSVC")

        -- Plugin export/import macros for MSVC
        add_defines("QTFORGE_PLUGIN_EXPORT=__declspec(dllexport)")
        add_defines("QTFORGE_PLUGIN_IMPORT=__declspec(dllimport)")

        -- MSVC-specific compiler flags for plugins
        add_cxxflags("/wd4251") -- Disable warning about DLL interface
        add_cxxflags("/wd4275") -- Disable warning about DLL interface for base classes
    end

    -- Common plugin configurations
    if config.kind == "shared" then
        add_defines("QTFORGE_PLUGIN_SHARED")
    else
        add_defines("QTFORGE_PLUGIN_STATIC")
    end

    -- Add toolchain identifier
    add_defines("QTFORGE_ACTIVE_TOOLCHAIN=\"" .. toolchain .. "\"")
end

-- Validate plugin configuration for toolchain
function plugin_template.validate_plugin_config(config, toolchain)
    local validation = {
        valid = true,
        warnings = {},
        errors = {}
    }

    -- Check for toolchain-specific issues
    if toolchain == "mingw64" then
        -- MinGW64-specific validations
        if config.kind == "shared" and not config.sources then
            table.insert(validation.errors, "Shared plugins require source files for MinGW64")
            validation.valid = false
        end

        -- Check for Qt6 availability
        local common = import("common")
        if not common.is_mingw64_available() then
            table.insert(validation.errors, "MinGW64 toolchain not available")
            validation.valid = false
        end

    elseif toolchain == "msvc" then
        -- MSVC-specific validations
        if config.kind == "shared" and not config.defines then
            table.insert(validation.warnings, "Consider adding export defines for MSVC shared plugins")
        end
    end

    -- Common validations
    if not config.sources then
        table.insert(validation.errors, "Plugin sources are required")
        validation.valid = false
    end

    return validation
end

-- Create plugin with enhanced toolchain validation
function plugin_template.create_validated_plugin(name, config)
    config = config or {}

    -- Detect active toolchain
    local common = import("common")
    local active_toolchain = common.detect_active_toolchain()

    -- Validate configuration
    local validation = plugin_template.validate_plugin_config(config, active_toolchain)

    if not validation.valid then
        print("Plugin validation failed for " .. name .. ":")
        for _, error in ipairs(validation.errors) do
            print("  Error: " .. error)
        end
        return false
    end

    if #validation.warnings > 0 then
        print("Plugin validation warnings for " .. name .. ":")
        for _, warning in ipairs(validation.warnings) do
            print("  Warning: " .. warning)
        end
    end

    -- Create plugin with validated configuration
    plugin_template.create_plugin(name, config)
    return true
end

-- Export the module
return plugin_template
