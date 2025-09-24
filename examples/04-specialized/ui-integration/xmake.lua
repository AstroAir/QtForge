-- UI Integration Plugin XMake Configuration
-- Comprehensive UI plugin demonstrating Qt Widgets integration and theme support
-- Uses modular xmake template system

-- Add module directory and import templates
add_moduledirs("../../../cmake/xmake")
local plugin_template = import("plugin_template")
local example_template = import("example_template")

-- Setup project
plugin_template.setup_plugin_project("UIPluginExample", "1.0.0", "ui")

-- Widget Components Library
target("ui_plugin_widgets")
    set_kind("static")
    set_basename("ui_plugin_widgets")

    -- Add Qt rules for widgets
    add_rules("qt.static")

    -- Add source files
    add_files("widgets/demo_widget.cpp")
    add_headerfiles("widgets/demo_widget.hpp")

    -- Add Qt packages
    add_packages("qt6core", "qt6widgets", "qt6gui")
    add_frameworks("QtCore", "QtWidgets", "QtGui")

    -- Include directories
    add_includedirs(".", {public = true})
    add_includedirs("../../../include", {public = false})

    -- Set version
    set_version("1.0.0")
target_end()

-- Dialog Components Library
target("ui_plugin_dialogs")
    set_kind("static")
    set_basename("ui_plugin_dialogs")

    -- Add Qt rules for widgets
    add_rules("qt.static")

    -- Add source files
    add_files("dialogs/settings_dialog.cpp")
    add_files("dialogs/about_dialog.cpp")
    add_headerfiles("dialogs/settings_dialog.hpp")
    add_headerfiles("dialogs/about_dialog.hpp")

    -- Add Qt packages
    add_packages("qt6core", "qt6widgets", "qt6gui")
    add_frameworks("QtCore", "QtWidgets", "QtGui")

    -- Include directories
    add_includedirs(".", {public = true})
    add_includedirs("../../../include", {public = false})

    -- Set version
    set_version("1.0.0")
target_end()

-- Core UI Plugin Library
target("ui_plugin_core")
    set_kind("static")
    set_basename("ui_plugin_core")

    -- Add Qt rules for widgets
    add_rules("qt.static")

    -- Add source files
    add_files("core/ui_plugin_core.cpp")
    add_headerfiles("core/ui_plugin_core.hpp")

    -- Add Qt packages
    add_packages("qt6core", "qt6widgets", "qt6gui")
    add_frameworks("QtCore", "QtWidgets", "QtGui")

    -- Dependencies
    add_deps("ui_plugin_widgets", "ui_plugin_dialogs")

    -- Include directories
    add_includedirs(".", {public = true})
    add_includedirs("../../../include", {public = false})

    -- Set version
    set_version("1.0.0")
target_end()

-- Main UI Plugin
plugin_template.create_ui_plugin("UIPlugin", {
    basename = "ui_plugin",
    sources = "main.cpp",
    metadata_file = "ui_plugin.json",
    version = "1.0.0",
    qt_features = {"core", "widgets", "gui"},
    qtforge_deps = {"QtForgeCore"},
    defines = {"QT_PLUGIN", "QTPLUGIN_BUILD_UI"},

    custom_config = function()
        -- Add dependencies on component libraries
        add_deps("ui_plugin_core", "ui_plugin_widgets", "ui_plugin_dialogs")

        -- Copy metadata to build directory
        after_build(function (target)
            os.cp("ui_plugin.json", target:targetdir())
        end)
    end
})

-- GUI Test Application
example_template.create_widget_example("UIPluginTest", {
    basename = "ui_plugin_test",
    sources = "test_ui_plugin.cpp",
    qt_features = {"core", "widgets"},
    qtforge_deps = {"QtForgeCore"},
    install_binary = true
})

-- Command-line Test Application
example_template.create_console_example("UIPluginTestCLI", {
    basename = "ui_plugin_test_cli",
    sources = "test_ui_plugin_cli.cpp",
    qt_features = {"core"},
    qtforge_deps = {"QtForgeCore"},
    install_binary = true
})

-- Install documentation
example_template.install_docs({"README.md"}, "share/doc/qtforge/examples/ui-integration")
