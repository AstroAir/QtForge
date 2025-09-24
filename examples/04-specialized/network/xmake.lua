-- Network Plugin XMake Configuration
-- Network plugin demonstrating QtForge network features
-- Uses modular xmake template system

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Set project info
set_project("NetworkPluginExample")
set_version("3.0.0")

-- Set C++ standard
set_languages("c++20")

-- Add Qt dependencies
add_requires("qt6core", {optional = true, configs = {shared = true}})
add_requires("qt6network", {optional = true, configs = {shared = true}})
add_requires("qt6websockets", {optional = true, configs = {shared = true}})
add_requires("qt6httpserver", {optional = true, configs = {shared = true}})

-- Network Plugin target
target("NetworkPlugin")
    set_kind("shared")
    set_basename("network_plugin")

    -- Add Qt rules for proper Qt integration with MOC support
    add_rules("qt.shared")

    -- Add source files
    add_files("network_plugin.cpp")

    -- Add header files
    add_headerfiles("network_plugin.hpp")

    -- Add metadata files
    add_installfiles("network_plugin.json", {prefixdir = "plugins"})

    -- Add Qt packages for proper MOC support
    add_packages("qt6core", "qt6network", "qt6websockets", "qt6httpserver")

    -- Add Qt frameworks
    add_frameworks("QtCore", "QtNetwork", "QtWebSockets", "QtHttpServer")

    -- Link with QtForge
    add_deps("QtForgeCore")
    add_includedirs("../../../include", {public = false})

    -- Set output directory
    set_targetdir("$(buildir)/plugins")

    -- Set version
    set_version("3.0.0")

    -- Compiler definitions
    add_defines("QT_PLUGIN", "QTPLUGIN_BUILD_NETWORK")

    -- Set visibility
    set_symbols("hidden")

    -- Install plugin
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "plugins"})

    -- Copy metadata to build directory
    after_build(function (target)
        os.cp("network_plugin.json", target:targetdir())
    end)
target_end()
