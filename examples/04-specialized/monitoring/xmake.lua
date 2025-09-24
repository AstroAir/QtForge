-- Monitoring Plugin XMake Configuration
-- Monitoring plugin demonstrating QtForge monitoring features
-- Uses modular xmake template system

-- Add module directory and import templates
add_moduledirs("../../../cmake/xmake")
local plugin_template = import("plugin_template")

-- Setup project
plugin_template.setup_plugin_project("MonitoringPluginExample", "3.0.0", "monitoring")

-- Create the monitoring plugin with test
plugin_template.create_monitoring_plugin("MonitoringPlugin", {
    basename = "monitoring_plugin",
    sources = "monitoring_plugin.cpp",
    headers = "monitoring_plugin.hpp",
    metadata_file = "monitoring_plugin.json",
    version = "3.0.0",
    qt_features = {"core"},
    qtforge_deps = {"QtForgeCore"},
    defines = {"QT_PLUGIN", "QTPLUGIN_BUILD_MONITORING"},
    visibility = "hidden",

    custom_config = function()
        -- Add strict Qt defines for monitoring plugin
        add_defines({
            "QT_NO_CAST_FROM_ASCII",
            "QT_NO_CAST_TO_ASCII",
            "QT_NO_URL_CAST_FROM_STRING",
            "QT_NO_CAST_FROM_BYTEARRAY",
            "QT_USE_QSTRINGBUILDER",
            "QT_STRICT_ITERATORS"
        })

        -- Add compiler-specific warnings
        if is_plat("windows") and is_config("toolchain", "msvc") then
            add_cxxflags("/W4", "/WX")
        else
            add_cxxflags("-Wall", "-Wextra", "-Werror", "-pedantic")
        end

        -- Install plugin and metadata
        add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "lib/qtforge/plugins"})
        add_installfiles("monitoring_plugin.json", {prefixdir = "share/qtforge/plugins"})
    end
})

-- Test executable
target("TestMonitoringPlugin")
    set_kind("binary")
    set_basename("test_monitoring_plugin")

    -- Add Qt rules
    add_rules("qt.console")

    -- Add source files
    add_files("test_monitoring_plugin.cpp")

    -- Add Qt packages
    add_packages("qt6core")
    add_frameworks("QtCore")

    -- Link with QtForge and MonitoringPlugin
    add_deps("QtForgeCore", "MonitoringPlugin")
    add_includedirs("../../../include", {public = false})

    -- Set output directory
    set_targetdir("$(buildir)/bin")

    -- Set version
    set_version("3.0.0")

    -- Install test binary
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin"})
target_end()

-- Install documentation
if os.isfile("README.md") then
    add_installfiles("README.md", {prefixdir = "share/doc/qtforge/examples/monitoring"})
end
