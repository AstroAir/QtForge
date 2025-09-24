-- Version Management Example XMake Configuration
-- Demonstrates QtForge version management features
-- Uses modular xmake template system

-- Add module directory and import templates
add_moduledirs("../../../cmake/xmake")
local example_template = import("example_template")

-- Setup project
example_template.setup_example_project("VersionManagementExample", "3.0.0", "basic")

-- Create the version management example
example_template.create_console_example("VersionManagementExample", {
    basename = "version_management_example",
    sources = "version_management_example.cpp",
    qt_features = {"core"},
    qtforge_deps = {"QtForgeCore"},
    version = "3.0.0",
    install_binary = true
})
