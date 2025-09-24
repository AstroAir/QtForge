-- QtForge Template Factory System
-- Provides template functions that work without requiring imports at global scope
-- Version: 3.2.0

-- Module table
local factory = {}

-- Template creation functions that can be called directly

-- Create a basic plugin using inline configuration
function factory.create_basic_plugin(name, config)
    config = config or {}

    target(name)
        set_kind("shared")
        set_basename(config.basename or name:lower())

        -- Add debug suffix if in debug mode
        if is_mode("debug") and (config.debug_postfix ~= false) then
            set_basename((config.basename or name:lower()) .. "_d")
        end

        -- Add Qt rules
        add_rules("qt.shared")

        -- Add source files
        if config.sources then
            if type(config.sources) == "string" then
                add_files(config.sources)
            else
                for _, source in ipairs(config.sources) do
                    add_files(source)
                end
            end
        end

        -- Add header files
        if config.headers then
            if type(config.headers) == "string" then
                add_headerfiles(config.headers)
            else
                for _, header in ipairs(config.headers) do
                    add_headerfiles(header)
                end
            end
        end

        -- Add Qt packages
        local qt_packages = config.qt_packages or {"qt6core"}
        for _, package in ipairs(qt_packages) do
            add_packages(package)
        end

        -- Add Qt frameworks
        local qt_frameworks = config.qt_frameworks or {"QtCore"}
        for _, framework in ipairs(qt_frameworks) do
            add_frameworks(framework)
        end

        -- Add QtForge dependencies
        local qtforge_deps = config.qtforge_deps or {"QtForgeCore"}
        for _, dep in ipairs(qtforge_deps) do
            add_deps(dep)
        end

        -- Add include directories
        add_includedirs("../../../include", {public = false})

        -- Set output directory
        set_targetdir(config.output_dir or "$(buildir)/plugins")

        -- Set version
        set_version(config.version or "1.0.0")

        -- Add defines
        local defines = config.defines or {"QT_PLUGIN"}
        for _, define in ipairs(defines) do
            add_defines(define)
        end

        -- Set visibility
        if config.visibility then
            set_symbols(config.visibility)
        end

        -- Install plugin
        if config.install ~= false then
            add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "plugins"})
        end

        -- Install metadata
        if config.metadata_file then
            add_installfiles(config.metadata_file, {prefixdir = "plugins"})

            -- Copy metadata to build directory
            after_build(function (target)
                os.cp(config.metadata_file, target:targetdir())
            end)
        end

        -- Custom configuration
        if config.custom_config then
            config.custom_config()
        end

    target_end()
end

-- Create a basic example application using inline configuration
function factory.create_basic_example(name, config)
    config = config or {}

    target(name)
        set_kind(config.kind or "binary")
        set_basename(config.basename or name:lower())

        -- Add debug suffix if in debug mode
        if is_mode("debug") and (config.debug_postfix ~= false) then
            set_basename((config.basename or name:lower()) .. "_d")
        end

        -- Add Qt rules based on app type
        local app_type = config.app_type or "console"
        if app_type == "console" then
            add_rules("qt.console")
        elseif app_type == "widgetapp" then
            add_rules("qt.widgetapp")
        else
            add_rules("qt.console")
        end

        -- Add source files
        if config.sources then
            if type(config.sources) == "string" then
                add_files(config.sources)
            else
                for _, source in ipairs(config.sources) do
                    add_files(source)
                end
            end
        end

        -- Add Qt packages
        local qt_packages = config.qt_packages or {"qt6core"}
        for _, package in ipairs(qt_packages) do
            add_packages(package)
        end

        -- Add Qt frameworks
        local qt_frameworks = config.qt_frameworks or {"QtCore"}
        for _, framework in ipairs(qt_frameworks) do
            add_frameworks(framework)
        end

        -- Add QtForge dependencies
        local qtforge_deps = config.qtforge_deps or {"QtForgeCore"}
        for _, dep in ipairs(qtforge_deps) do
            add_deps(dep)
        end

        -- Add include directories
        add_includedirs("../../../include", {public = false})

        -- Set output directory
        set_targetdir(config.output_dir or "$(buildir)/bin")

        -- Set version
        set_version(config.version or "1.0.0")

        -- Add defines
        if config.defines then
            for _, define in ipairs(config.defines) do
                add_defines(define)
            end
        end

        -- Install binary
        if config.install_binary then
            add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin"})
        end

        -- Custom configuration
        if config.custom_config then
            config.custom_config()
        end

    target_end()
end

-- Create a plugin with test using inline configuration
function factory.create_plugin_with_test(plugin_name, test_name, config)
    config = config or {}

    -- Create the plugin
    factory.create_basic_plugin(plugin_name, config)

    -- Create test target if test configuration is provided
    if config.test_config then
        local test_config = config.test_config

        target(test_name or (plugin_name .. "Test"))
            set_kind("binary")
            set_basename(test_config.basename or (plugin_name:lower() .. "_test"))

            -- Add Qt rules for console application
            add_rules("qt.console")

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

            -- Add Qt packages
            local qt_packages = test_config.qt_packages or {"qt6core"}
            for _, package in ipairs(qt_packages) do
                add_packages(package)
            end

            -- Add Qt frameworks
            local qt_frameworks = test_config.qt_frameworks or {"QtCore"}
            for _, framework in ipairs(qt_frameworks) do
                add_frameworks(framework)
            end

            -- Add QtForge dependencies and plugin dependency
            local test_deps = test_config.qtforge_deps or {"QtForgeCore"}
            table.insert(test_deps, plugin_name)
            for _, dep in ipairs(test_deps) do
                add_deps(dep)
            end

            -- Add include directories
            add_includedirs("../../../include", {public = false})

            -- Set output directory
            set_targetdir(test_config.output_dir or "$(buildir)/bin")

            -- Set version
            set_version(config.version or "1.0.0")

            -- Add test-specific defines
            if test_config.defines then
                for _, define in ipairs(test_config.defines) do
                    add_defines(define)
                end
            end

            -- Install test binary if specified
            if test_config.install then
                add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin"})
            end

        target_end()
    end
end

-- Utility function to setup common project settings
function factory.setup_project(name, version, qt_features)
    set_xmakever("3.0.1")
    set_project(name or "QtForgeExample")
    set_version(version or "1.0.0")
    set_languages("c++20")

    -- Add Qt requirements based on features
    qt_features = qt_features or "basic"

    if qt_features == "basic" or qt_features == "plugin" then
        add_requires("qt6core", {optional = true, configs = {shared = true}})
    elseif qt_features == "ui" then
        add_requires("qt6core", {optional = true, configs = {shared = true}})
        add_requires("qt6widgets", {optional = true, configs = {shared = true}})
        add_requires("qt6gui", {optional = true, configs = {shared = true}})
    elseif qt_features == "network" then
        add_requires("qt6core", {optional = true, configs = {shared = true}})
        add_requires("qt6network", {optional = true, configs = {shared = true}})
    elseif qt_features == "comprehensive" then
        add_requires("qt6core", {optional = true, configs = {shared = true}})
        add_requires("qt6widgets", {optional = true, configs = {shared = true}})
        add_requires("qt6gui", {optional = true, configs = {shared = true}})
        add_requires("qt6network", {optional = true, configs = {shared = true}})
    end
end

-- Template inheritance system for different example categories

-- Base template configurations for different categories
factory.base_templates = {
    -- Fundamental examples base template
    fundamentals = {
        qt_features = "basic",
        app_type = "console",
        debug_postfix = true,
        install = false,
        defines = {"QTFORGE_EXAMPLE_FUNDAMENTALS"},
        common_sources = {"src/main.cpp"},
        common_headers = {"include/example_base.h"}
    },

    -- Communication examples base template
    communication = {
        qt_features = "network",
        app_type = "console",
        debug_postfix = true,
        install = false,
        defines = {"QTFORGE_EXAMPLE_COMMUNICATION", "QTFORGE_NETWORK_ENABLED"},
        common_sources = {"src/main.cpp", "src/communication_base.cpp"},
        common_headers = {"include/communication_base.h"},
        packages = {"qt6network"}
    },

    -- UI examples base template
    ui = {
        qt_features = "ui",
        app_type = "gui",
        debug_postfix = true,
        install = false,
        defines = {"QTFORGE_EXAMPLE_UI", "QTFORGE_UI_ENABLED"},
        common_sources = {"src/main.cpp", "src/ui_base.cpp"},
        common_headers = {"include/ui_base.h"},
        packages = {"qt6widgets", "qt6gui"}
    },

    -- Specialized examples base template
    specialized = {
        qt_features = "comprehensive",
        app_type = "console",
        debug_postfix = true,
        install = false,
        defines = {"QTFORGE_EXAMPLE_SPECIALIZED"},
        common_sources = {"src/main.cpp", "src/specialized_base.cpp"},
        common_headers = {"include/specialized_base.h"}
    },

    -- Plugin examples base template
    plugins = {
        qt_features = "plugin",
        kind = "shared",
        debug_postfix = true,
        install = true,
        defines = {"QTFORGE_PLUGIN_EXAMPLE"},
        common_sources = {"src/plugin.cpp"},
        common_headers = {"include/plugin_interface.h"},
        plugin_type = "service"
    }
}

-- Template inheritance resolver
function factory.resolve_template_inheritance(category, config)
    config = config or {}

    -- Get base template
    local base_template = factory.base_templates[category]
    if not base_template then
        print("Warning: Unknown template category '" .. category .. "', using default")
        base_template = factory.base_templates.fundamentals
    end

    -- Create resolved configuration by merging base with overrides
    local resolved = {}

    -- Copy base template
    for k, v in pairs(base_template) do
        if type(v) == "table" then
            resolved[k] = {}
            for i, item in ipairs(v) do
                table.insert(resolved[k], item)
            end
        else
            resolved[k] = v
        end
    end

    -- Apply user overrides
    for k, v in pairs(config) do
        if type(v) == "table" and type(resolved[k]) == "table" then
            -- Merge arrays
            for _, item in ipairs(v) do
                table.insert(resolved[k], item)
            end
        else
            -- Override scalar values
            resolved[k] = v
        end
    end

    return resolved
end

-- Create example with template inheritance
function factory.create_inherited_example(name, category, config)
    local resolved_config = factory.resolve_template_inheritance(category, config)

    print("Creating example '" .. name .. "' with category '" .. category .. "'")

    -- Import required modules
    local example_template = import("example_template")

    -- Create example with resolved configuration
    return example_template.create_validated_example(name, resolved_config)
end

-- Create plugin with template inheritance
function factory.create_inherited_plugin(name, category, config)
    local resolved_config = factory.resolve_template_inheritance(category, config)

    print("Creating plugin '" .. name .. "' with category '" .. category .. "'")

    -- Import required modules
    local plugin_template = import("plugin_template")

    -- Create plugin with resolved configuration
    return plugin_template.create_validated_plugin(name, resolved_config)
end

-- Get available template categories
function factory.get_template_categories()
    local categories = {}
    for category, _ in pairs(factory.base_templates) do
        table.insert(categories, category)
    end
    return categories
end

-- Get template category information
function factory.get_category_info(category)
    local base_template = factory.base_templates[category]
    if not base_template then
        return nil
    end

    return {
        category = category,
        qt_features = base_template.qt_features,
        app_type = base_template.app_type,
        defines = base_template.defines,
        packages = base_template.packages or {},
        description = factory.get_category_description(category)
    }
end

-- Get category descriptions
function factory.get_category_description(category)
    local descriptions = {
        fundamentals = "Basic examples demonstrating core QtForge concepts",
        communication = "Examples showing inter-plugin communication patterns",
        ui = "User interface examples with Qt widgets and graphics",
        specialized = "Advanced examples for specific use cases",
        plugins = "Plugin development examples and templates"
    }

    return descriptions[category] or "Custom category"
end

-- Validate template inheritance configuration
function factory.validate_inheritance_config(category, config)
    local validation = {
        valid = true,
        warnings = {},
        errors = {}
    }

    -- Check if category exists
    if not factory.base_templates[category] then
        table.insert(validation.errors, "Unknown template category: " .. category)
        validation.valid = false
    end

    -- Check for conflicting configurations
    local base_template = factory.base_templates[category]
    if base_template and config then
        -- Check app_type conflicts
        if base_template.app_type == "gui" and config.app_type == "console" then
            table.insert(validation.warnings, "Overriding GUI app_type with console may cause issues")
        end

        -- Check Qt features conflicts
        if base_template.qt_features == "ui" and config.qt_features == "basic" then
            table.insert(validation.warnings, "Downgrading from UI to basic Qt features")
        end
    end

    return validation
end

-- Generate inheritance report
function factory.generate_inheritance_report()
    local report = {
        timestamp = os.date("%Y-%m-%d %H:%M:%S"),
        categories = {},
        total_categories = 0
    }

    for category, template in pairs(factory.base_templates) do
        report.categories[category] = {
            qt_features = template.qt_features,
            app_type = template.app_type,
            defines_count = template.defines and #template.defines or 0,
            packages_count = template.packages and #template.packages or 0,
            description = factory.get_category_description(category)
        }
        report.total_categories = report.total_categories + 1
    end

    print("Template Inheritance Report:")
    print("  Total Categories: " .. report.total_categories)
    for category, info in pairs(report.categories) do
        print("  " .. category .. ": " .. info.description)
        print("    Qt Features: " .. info.qt_features)
        print("    App Type: " .. info.app_type)
        print("    Defines: " .. info.defines_count)
        print("    Packages: " .. info.packages_count)
    end

    return report
end

-- Export the module
return factory
