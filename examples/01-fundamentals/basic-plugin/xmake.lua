-- Basic Plugin XMake Configuration
-- Comprehensive QtForge plugin demonstrating core functionality

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Set project info
set_project("BasicPlugin")
set_version("2.0.0")

-- Set C++ standard
set_languages("c++20")

-- Add Qt dependencies
add_requires("qt6core", {optional = true, configs = {shared = true}})

-- Basic Plugin target
target("BasicPlugin")
    set_kind("shared")
    set_basename("basic_plugin")
    
    -- Add Qt rules for proper Qt integration with MOC support
    add_rules("qt.shared")
    
    -- Add source files
    add_files("basic_plugin.cpp")
    
    -- Add header files
    add_headerfiles("basic_plugin.hpp")
    
    -- Add metadata files
    add_installfiles("basic_plugin.json", {prefixdir = "plugins"})
    
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
    set_version("2.0.0")
    
    -- Export symbols for shared library
    add_defines("BASIC_PLUGIN_EXPORTS")
    
    -- Install plugin to plugins directory
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "plugins"})
target_end()

-- Basic Plugin Test target
target("BasicPluginTest")
    set_kind("binary")
    set_basename("basic_plugin_test")
    
    -- Add Qt rules
    add_rules("qt.console")
    
    -- Add source files
    add_files("test_basic_plugin.cpp")
    
    -- Add Qt packages
    add_packages("qt6core")
    
    -- Add Qt frameworks
    add_frameworks("QtCore")
    
    -- Link with QtForge and BasicPlugin
    add_deps("QtForgeCore", "BasicPlugin")
    add_includedirs("../../../include", {public = false})
    
    -- Set output directory
    set_targetdir("$(buildir)/bin")
    
    -- Set version
    set_version("2.0.0")
    
    -- Install test to bin directory
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin"})
target_end()
