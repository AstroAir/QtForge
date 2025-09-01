-- QtForge Comprehensive Example XMake Configuration
-- Full application demonstrating all QtForge features

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Set project info
set_project("QtForgeComprehensiveExample")
set_version("3.0.0")

-- Set C++ standard
set_languages("c++20")

-- Add Qt dependencies
add_requires("qt6core", {optional = true, configs = {shared = true}})
add_requires("qt6widgets", {optional = true, configs = {shared = true}})
add_requires("qt6network", {optional = true, configs = {shared = true}})

-- Comprehensive Demo Application target
target("comprehensive_demo")
    set_kind("binary")
    set_basename("comprehensive_demo")

    -- Add Qt rules for proper Qt integration with MOC support
    add_rules("qt.widgetapp")

    -- Add source files
    add_files("main.cpp")

    -- Add Qt packages for proper MOC support
    add_packages("qt6core", "qt6widgets", "qt6network")

    -- Add Qt frameworks
    add_frameworks("QtCore", "QtWidgets", "QtNetwork")

    -- Link with QtForge (assuming it's available in parent directory)
    add_deps("QtForgeCore", "QtForgeSecurity")
    add_includedirs("../../../include", {public = false})

    -- Set output directory
    set_targetdir("$(buildir)/bin")

    -- Set version
    set_version("3.0.0")

    -- Compiler-specific options
    if is_plat("windows") then
        if is_config("toolchain", "msvc") then
            add_cxxflags("/W4", "/permissive-")
        else
            add_cxxflags("-Wall", "-Wextra", "-Wpedantic")
        end
    else
        add_cxxflags("-Wall", "-Wextra", "-Wpedantic")
    end

    -- Set debug postfix
    if is_mode("debug") then
        set_basename("comprehensive_demo_d")
    end

    -- Install application
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin"})

    -- Install configuration files
    add_installfiles("config/application.json", {prefixdir = "share/qtplugin/examples/config"})

    -- Install Python scripts
    add_installfiles("python/comprehensive_demo.py", {prefixdir = "share/qtplugin/examples/python"})

    -- Install build scripts
    add_installfiles("build.sh", {prefixdir = "share/qtplugin/examples"})
    add_installfiles("build.bat", {prefixdir = "share/qtplugin/examples"})

    -- Copy configuration and plugins to build directory after build
    after_build(function (target)
        -- Copy config files
        os.cp("config", target:targetdir())

        -- Copy Python scripts
        os.cp("python", target:targetdir())

        -- Create plugins directory
        os.mkdir(path.join(target:targetdir(), "plugins"))
    end)
target_end()

-- Include comprehensive plugin subdirectory
includes("plugins/comprehensive_plugin")

-- Install documentation
add_installfiles("README.md", {prefixdir = "share/doc/qtplugin/examples/comprehensive"})
add_installfiles("USAGE.md", {prefixdir = "share/doc/qtplugin/examples/comprehensive"})

-- Add tests
add_tests("ComprehensiveDemo", {
    kind = "binary",
    files = "main.cpp",
    timeout = 60,
    labels = {"comprehensive", "application", "example"}
})
