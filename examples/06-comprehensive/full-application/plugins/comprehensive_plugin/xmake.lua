-- Comprehensive Plugin XMake Configuration
-- Demonstrates all QtForge features in a single plugin

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Set project info
set_project("ComprehensivePlugin")
set_version("3.0.0")

-- Set C++ standard
set_languages("c++20")

-- Add Qt dependencies
add_requires("qt6core", {optional = true, configs = {shared = true}})
add_requires("qt6widgets", {optional = true, configs = {shared = true}})
add_requires("qt6network", {optional = true, configs = {shared = true}})

-- Optional Python support
option("python_support")
    set_default(false)
    set_showmenu(true)
    set_description("Enable Python support for the comprehensive plugin")
option_end()

if has_config("python_support") then
    add_requires("python3", {optional = true})
    add_requires("pybind11", {optional = true})
end

-- Comprehensive Plugin target
target("comprehensive_plugin")
    set_kind("shared")
    set_basename("comprehensive_plugin")

    -- Add Qt rules for proper Qt integration with MOC support
    add_rules("qt.shared")

    -- Add source files
    add_files("comprehensive_plugin.cpp")

    -- Add header files
    add_headerfiles("comprehensive_plugin.hpp")

    -- Add metadata files
    add_installfiles("comprehensive_plugin.json", {prefixdir = "plugins"})

    -- Add Qt packages for proper MOC support
    add_packages("qt6core", "qt6widgets", "qt6network")

    -- Add Qt frameworks
    add_frameworks("QtCore", "QtWidgets", "QtNetwork")

    -- Link with QtForge (assuming it's available in parent directory)
    add_deps("QtForgeCore", "QtForgeSecurity")
    add_includedirs("../../../../../include", {public = false})

    -- Set output directory
    set_targetdir("$(buildir)/plugins")

    -- Set version
    set_version("3.0.0")

    -- Set visibility
    set_symbols("hidden")

    -- Export symbols for shared library
    add_defines("COMPREHENSIVE_PLUGIN_EXPORTS")

    -- Optional Python support
    if has_config("python_support") then
        add_packages("python3", "pybind11")
        add_defines("QTFORGE_PYTHON_SUPPORT")
    end

    -- Install plugin
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "plugins"})

    -- Copy metadata to build directory
    after_build(function (target)
        os.cp("comprehensive_plugin.json", target:targetdir())
    end)
target_end()

-- Comprehensive Plugin Test target
target("ComprehensivePluginTest")
    set_kind("binary")
    set_basename("comprehensive_plugin_test")

    -- Add Qt rules
    add_rules("qt.widgetapp")

    -- Add source files
    add_files("test_comprehensive_plugin.cpp")

    -- Add Qt packages
    add_packages("qt6core", "qt6widgets", "qt6network")

    -- Add Qt frameworks
    add_frameworks("QtCore", "QtWidgets", "QtNetwork")

    -- Link with QtForge and comprehensive plugin
    add_deps("QtForgeCore", "QtForgeSecurity", "comprehensive_plugin")
    add_includedirs("../../../../../include", {public = false})

    -- Set output directory
    set_targetdir("$(buildir)/bin/examples")

    -- Set version
    set_version("3.0.0")

    -- Optional Python support
    if has_config("python_support") then
        add_packages("python3", "pybind11")
        add_defines("QTFORGE_PYTHON_SUPPORT")
    end

    -- Install test
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin/examples"})

    -- Copy plugin and metadata to test directory after build
    after_build(function (target)
        local plugin_target = target:dep("comprehensive_plugin")
        if plugin_target then
            os.cp(plugin_target:targetfile(), target:targetdir())
            os.cp("comprehensive_plugin.json", target:targetdir())
        end
    end)
target_end()

-- Add tests
add_tests("ComprehensivePluginTest", {
    kind = "binary",
    files = "test_comprehensive_plugin.cpp",
    timeout = 60,
    labels = {"comprehensive", "plugin", "example"}
})
