-- Request-Response Communication Plugin XMake Configuration
-- Demonstrates QtForge request-response communication patterns
-- Uses direct configuration (modular templates moved to includes)

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Set project info
set_project("RequestResponsePlugin")
set_version("3.0.0")

-- Set C++ standard
set_languages("c++20")

-- Add Qt dependencies
add_requires("qt6core", {optional = true, configs = {shared = true}})

-- Request-Response Plugin target
target("RequestResponsePlugin")
    set_kind("shared")
    set_basename("request_response_plugin")

    -- Add Qt rules for proper Qt integration with MOC support
    add_rules("qt.shared")

    -- Add source files
    add_files("request_response_plugin.cpp")

    -- Add header files
    add_headerfiles("request_response_plugin.hpp")

    -- Add metadata files
    add_installfiles("request_response_plugin.json", {prefixdir = "plugins"})

    -- Add Qt packages for proper MOC support
    add_packages("qt6core")

    -- Add Qt frameworks
    add_frameworks("QtCore")

    -- Link with QtForge
    add_deps("QtForgeCore")
    add_includedirs("../../../include", {public = false})

    -- Set output directory
    set_targetdir("$(buildir)/plugins")

    -- Set version
    set_version("3.0.0")

    -- Compiler definitions
    add_defines("QT_PLUGIN")

    -- Install plugin
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "plugins"})

    -- Copy metadata to build directory
    after_build(function (target)
        os.cp("request_response_plugin.json", target:targetdir())
    end)
target_end()

-- Request-Response Plugin Test target
target("RequestResponsePluginTest")
    set_kind("binary")
    set_basename("request_response_plugin_test")

    -- Add Qt rules
    add_rules("qt.console")

    -- Add source files
    add_files("test_request_response_plugin.cpp")

    -- Add Qt packages
    add_packages("qt6core")

    -- Add Qt frameworks
    add_frameworks("QtCore")

    -- Link with QtForge and plugin
    add_deps("QtForgeCore", "RequestResponsePlugin")
    add_includedirs("../../../include", {public = false})

    -- Set output directory
    set_targetdir("$(buildir)/bin")

    -- Set version
    set_version("3.0.0")

    -- Install test
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin"})
target_end()
