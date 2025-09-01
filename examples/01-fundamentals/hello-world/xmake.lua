-- Hello World Plugin XMake Configuration
-- Simple QtForge plugin demonstrating basic functionality

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Set project info
set_project("HelloWorldPlugin")
set_version("1.0.0")

-- Set C++ standard
set_languages("c++20")

-- Add Qt dependencies
add_requires("qt6core", {optional = true, configs = {shared = true}})

-- Plugin target
target("HelloWorldPlugin")
    set_kind("shared")
    set_basename("hello_world_plugin")

    -- Add Qt rules for proper Qt integration with MOC support
    add_rules("qt.shared")

    -- Add source files
    add_files("hello_world_plugin.cpp")

    -- Add header files
    add_headerfiles("hello_world_plugin.hpp")

    -- Add metadata files
    add_installfiles("hello_world_plugin.json", {prefixdir = "plugins"})

    -- Add Qt packages for proper MOC support
    add_packages("qt6core")

    -- Add Qt frameworks
    add_frameworks("QtCore")

    -- Link with QtForge (assuming it's available in parent directory)
    add_deps("QtForgeCore")
    add_includedirs("../../../include", {public = false})

    -- Set output directory
    set_targetdir("$(buildir)/plugins")

    -- Set version
    set_version("1.0.0")

    -- Export symbols for shared library
    add_defines("HELLO_WORLD_PLUGIN_EXPORTS")

    -- Install plugin to plugins directory
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "plugins"})
target_end()
