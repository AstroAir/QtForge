-- QtForge Example Registry and Discovery System
-- Centralized system for registering, discovering, and validating examples
-- Version: 3.2.0

-- Module table
local registry = {}

-- Global registry for examples
registry.examples = {}
registry.plugins = {}
registry.categories = {}

-- Example categories and their default configurations
registry.default_categories = {
    fundamentals = {
        qt_features = "basic",
        description = "Basic QtForge functionality examples"
    },
    communication = {
        qt_features = "plugin",
        description = "Inter-plugin communication examples"
    },
    services = {
        qt_features = "service",
        description = "Background service and processing examples"
    },
    specialized = {
        qt_features = "comprehensive",
        description = "Specialized plugin examples (UI, network, monitoring)"
    },
    integration = {
        qt_features = "basic",
        description = "Integration examples with other systems"
    },
    comprehensive = {
        qt_features = "comprehensive",
        description = "Full-featured application examples"
    }
}

-- Register an example
function registry.register_example(path, config)
    config = config or {}

    -- Extract category from path
    local category = path:match("^(%d+%-[^/]+)")
    if category then
        category = category:gsub("^%d+%-", "")  -- Remove number prefix
    else
        category = "misc"
    end

    -- Apply category defaults
    local category_config = registry.default_categories[category] or {}
    for k, v in pairs(category_config) do
        if config[k] == nil then
            config[k] = v
        end
    end

    -- Store example registration
    registry.examples[path] = {
        path = path,
        category = category,
        config = config,
        registered = true
    }

    return registry.examples[path]
end

-- Register a plugin
function registry.register_plugin(name, config)
    config = config or {}

    registry.plugins[name] = {
        name = name,
        config = config,
        registered = true
    }

    return registry.plugins[name]
end

-- Discover examples in directory structure
function registry.discover_examples(base_dir)
    base_dir = base_dir or "."
    local discovered = {}

    local example_dirs = {
        "01-fundamentals",
        "02-communication",
        "03-services",
        "04-specialized",
        "05-integration",
        "06-comprehensive"
    }

    for _, dir in ipairs(example_dirs) do
        local full_dir = path.join(base_dir, dir)
        if os.isdir(full_dir) then
            for _, subdir in ipairs(os.dirs(path.join(full_dir, "*"))) do
                local example_path = path.join(dir, path.basename(subdir))
                local xmake_file = path.join(base_dir, example_path, "xmake.lua")

                if os.isfile(xmake_file) then
                    local example_info = {
                        path = example_path,
                        full_path = path.join(base_dir, example_path),
                        xmake_file = xmake_file,
                        category = dir:gsub("^%d+%-", ""),
                        discovered = true
                    }

                    table.insert(discovered, example_info)
                end
            end
        end
    end

    return discovered
end

-- Validate example configuration
function registry.validate_example(example_path, config)
    local issues = {}

    -- Check if directory exists
    if not os.isdir(example_path) then
        table.insert(issues, "Directory not found: " .. example_path)
    end

    -- Check if xmake.lua exists
    local xmake_file = path.join(example_path, "xmake.lua")
    if not os.isfile(xmake_file) then
        table.insert(issues, "No xmake.lua found: " .. xmake_file)
    end

    -- Check Qt requirements
    if config.requires_widgets and not has_package("qt6widgets") then
        table.insert(issues, "Qt6Widgets not available")
    end

    if config.requires_network and not has_package("qt6network") then
        table.insert(issues, "Qt6Network not available")
    end

    -- Check source files if specified
    if config.sources then
        local sources = type(config.sources) == "string" and {config.sources} or config.sources
        for _, source in ipairs(sources) do
            local source_path = path.join(example_path, source)
            if not os.isfile(source_path) then
                table.insert(issues, "Source file not found: " .. source_path)
            end
        end
    end

    return issues
end

-- Get example status
function registry.get_example_status(example_path, config)
    local issues = registry.validate_example(example_path, config)

    return {
        path = example_path,
        valid = #issues == 0,
        issues = issues,
        config = config
    }
end

-- Filter examples based on criteria
function registry.filter_examples(examples, filter_func)
    local filtered = {}

    for _, example in ipairs(examples) do
        if filter_func(example) then
            table.insert(filtered, example)
        end
    end

    return filtered
end

-- Get examples by category
function registry.get_examples_by_category(category)
    local category_examples = {}

    for path, example in pairs(registry.examples) do
        if example.category == category then
            table.insert(category_examples, example)
        end
    end

    return category_examples
end

-- Generate example report
function registry.generate_report()
    local report = {
        total_examples = 0,
        valid_examples = 0,
        invalid_examples = 0,
        categories = {},
        issues = {}
    }

    for path, example in pairs(registry.examples) do
        report.total_examples = report.total_examples + 1

        local status = registry.get_example_status(example.path, example.config)

        if status.valid then
            report.valid_examples = report.valid_examples + 1
        else
            report.invalid_examples = report.invalid_examples + 1
            report.issues[path] = status.issues
        end

        -- Count by category
        local category = example.category
        if not report.categories[category] then
            report.categories[category] = {total = 0, valid = 0, invalid = 0}
        end

        report.categories[category].total = report.categories[category].total + 1
        if status.valid then
            report.categories[category].valid = report.categories[category].valid + 1
        else
            report.categories[category].invalid = report.categories[category].invalid + 1
        end
    end

    return report
end

-- Print registry status
function registry.print_status()
    local report = registry.generate_report()

    print("QtForge Example Registry Status:")
    print("================================")
    print("Total examples: " .. report.total_examples)
    print("Valid examples: " .. report.valid_examples)
    print("Invalid examples: " .. report.invalid_examples)
    print("")

    print("By category:")
    for category, stats in pairs(report.categories) do
        print("  " .. category .. ": " .. stats.valid .. "/" .. stats.total .. " valid")
    end

    if report.invalid_examples > 0 then
        print("")
        print("Issues found:")
        for path, issues in pairs(report.issues) do
            print("  " .. path .. ":")
            for _, issue in ipairs(issues) do
                print("    - " .. issue)
            end
        end
    end
end

-- Enhanced discovery and inclusion system

-- Include examples with validation and error handling
function registry.include_examples_safe(examples_config)
    local included = {}
    local skipped = {}

    for example_path, config in pairs(examples_config) do
        local status = registry.get_example_status(example_path, config)

        if status.valid then
            print("Including example: " .. example_path)
            includes(example_path)
            table.insert(included, example_path)
        else
            print("Skipping example " .. example_path .. ":")
            for _, issue in ipairs(status.issues) do
                print("  - " .. issue)
            end
            table.insert(skipped, {path = example_path, issues = status.issues})
        end
    end

    return included, skipped
end

-- Auto-discover and include all valid examples
function registry.auto_include_examples(base_dir)
    base_dir = base_dir or "."
    local discovered = registry.discover_examples(base_dir)
    local included = {}
    local skipped = {}

    for _, example in ipairs(discovered) do
        -- Try to determine configuration from path and category
        local config = {
            qt_features = registry.default_categories[example.category] and
                         registry.default_categories[example.category].qt_features or "basic"
        }

        local status = registry.get_example_status(example.path, config)

        if status.valid then
            print("Auto-including example: " .. example.path)
            includes(example.path)
            table.insert(included, example.path)
        else
            print("Auto-skipping example " .. example.path .. ":")
            for _, issue in ipairs(status.issues) do
                print("  - " .. issue)
            end
            table.insert(skipped, {path = example.path, issues = status.issues})
        end
    end

    return included, skipped
end

-- Create a comprehensive example configuration
function registry.create_example_config()
    return {
        -- Fundamentals (working examples)
        ["01-fundamentals/hello-world"] = {qt_features = "basic"},
        ["01-fundamentals/basic-plugin"] = {qt_features = "plugin"},
        ["01-fundamentals/configuration"] = {qt_features = "plugin"},

        -- Communication (working examples)
        ["02-communication/message-bus"] = {qt_features = "plugin"},
        ["02-communication/event-driven"] = {qt_features = "plugin"},
        ["02-communication/request-response"] = {qt_features = "plugin"},

        -- Services
        ["03-services/background-tasks"] = {qt_features = "service"},

        -- Specialized
        ["04-specialized/network"] = {qt_features = "network"},
        ["04-specialized/ui-integration"] = {qt_features = "ui", requires_widgets = true},
        ["04-specialized/monitoring"] = {qt_features = "monitoring"},

        -- Integration
        ["05-integration/version-management"] = {qt_features = "basic"},

        -- Comprehensive
        ["06-comprehensive/full-application"] = {qt_features = "comprehensive"},
        ["06-comprehensive/performance-optimized"] = {qt_features = "comprehensive"}
    }
end

-- Utility function to setup modular example system
function registry.setup_modular_examples()
    -- Add module directory
    add_moduledirs("../cmake/xmake")

    -- Get example configuration
    local example_configs = registry.create_example_config()

    -- Include examples with validation
    local included, skipped = registry.include_examples_safe(example_configs)

    -- Print summary
    print("")
    print("Example inclusion summary:")
    print("  Included: " .. #included .. " examples")
    print("  Skipped: " .. #skipped .. " examples")

    if #skipped > 0 then
        print("  Skipped examples can be fixed by addressing the issues listed above.")
    end

    return included, skipped
end

-- Export the module
return registry
