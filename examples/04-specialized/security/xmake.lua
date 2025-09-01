-- Security Plugin Example XMake Configuration
-- Security plugin demonstrating QtForge security features

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Set project info
set_project("SecurityPluginExample")
set_version("3.0.0")

-- Set C++ standard
set_languages("c++20")

-- Add Qt dependencies
add_requires("qt6core", {optional = true, configs = {shared = true}})

-- Security Plugin target
target("security_plugin")
    set_kind("shared")
    set_basename("security_plugin")
    
    -- Add Qt rules for proper Qt integration with MOC support
    add_rules("qt.shared")
    
    -- Add source files
    add_files("security_plugin.cpp")
    
    -- Add header files
    add_headerfiles("security_plugin.hpp")
    
    -- Add metadata files
    add_installfiles("security_plugin.json", {prefixdir = "lib/qtplugin/examples"})
    
    -- Add Qt packages for proper MOC support
    add_packages("qt6core")
    
    -- Add Qt frameworks
    add_frameworks("QtCore")
    
    -- Link with QtForge (assuming it's available in parent directory)
    add_deps("QtForgeCore", "QtForgeSecurity")
    add_includedirs("../../../include", {public = false})
    
    -- Set output directory
    set_targetdir("$(buildir)/lib")
    
    -- Set version
    set_version("3.0.0")
    
    -- Set visibility
    set_symbols("hidden")
    
    -- Export symbols for shared library
    add_defines("SECURITY_PLUGIN_EXPORTS")
    
    -- Install plugin
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "lib"})
target_end()

-- Security Plugin Test target
target("SecurityPluginTest")
    set_kind("binary")
    set_basename("security_plugin_test")
    
    -- Add Qt rules
    add_rules("qt.console")
    
    -- Add source files
    add_files("test_security_plugin.cpp")
    
    -- Add Qt packages
    add_packages("qt6core")
    
    -- Add Qt frameworks
    add_frameworks("QtCore")
    
    -- Link with QtForge and security plugin
    add_deps("QtForgeCore", "QtForgeSecurity", "security_plugin")
    add_includedirs("../../../include", {public = false})
    
    -- Set output directory
    set_targetdir("$(buildir)/bin/examples")
    
    -- Set version
    set_version("3.0.0")
    
    -- Install test
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin/examples"})
    
    -- Copy plugin and metadata to test directory after build
    after_build(function (target)
        local plugin_target = target:dep("security_plugin")
        if plugin_target then
            os.cp(plugin_target:targetfile(), target:targetdir())
            os.cp("security_plugin.json", target:targetdir())
        end
    end)
target_end()

-- Install documentation
add_installfiles("README.md", {prefixdir = "share/doc/qtplugin/examples/security"})

-- Add tests
add_tests("SecurityPluginTest", {
    kind = "binary",
    files = "test_security_plugin.cpp",
    timeout = 30,
    labels = {"security", "plugin", "example"}
})
