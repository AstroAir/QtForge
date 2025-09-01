-- QtForge Examples XMake Configuration

-- Enhanced Basic Plugin Example
if os.isdir("01-fundamentals/basic-plugin") then
    includes("01-fundamentals/basic-plugin")
end

-- Service Plugin Example (background processing and MessageBus integration)
if os.isdir("03-services/background-tasks") then
    includes("03-services/background-tasks")
end

-- UI Plugin Example (Qt Widgets integration and theme support)
if has_config("examples") and has_package("qt6widgets") then
    if os.isdir("04-specialized/ui-integration") then
        includes("04-specialized/ui-integration")
    end
else
    print("UI Plugin example skipped (Qt6Widgets not available)")
end

-- Additional examples based on available directories
local example_dirs = {
    "01-fundamentals",
    "02-communication",
    "03-services",
    "04-specialized",
    "05-integration",
    "06-comprehensive"
}

for _, dir in ipairs(example_dirs) do
    if os.isdir(dir) then
        -- Check for xmake.lua in subdirectories
        for _, subdir in ipairs(os.dirs(path.join(dir, "*"))) do
            local example_path = path.join(dir, path.basename(subdir))
            if os.isfile(path.join(example_path, "xmake.lua")) then
                includes(example_path)
            end
        end
    end
end

-- Simple example target for testing
target("simple_example")
    set_kind("binary")
    set_basename("simple_example")

    -- Use modern Qt console application rule
    add_rules("qt.console")

    -- Add Qt packages using modern package system
    add_packages("qt6core")

    -- Add dependencies
    add_deps("QtForgeCore")

    -- Simple test source
    add_files("simple_example.cpp")

    -- Set C++ standard
    set_languages("c++20")

    -- Set as default example
    set_default(false)
target_end()
