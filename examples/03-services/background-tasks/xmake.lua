-- Service Plugin Example XMake Configuration
-- Comprehensive service plugin demonstrating background processing and MessageBus integration

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Set project info
set_project("ServicePluginExample")
set_version("1.0.0")

-- Set C++ standard
set_languages("c++20")

-- Add Qt dependencies
add_requires("qt6core", {optional = true, configs = {shared = true}})

-- Service Plugin target
target("ServicePlugin")
    set_kind("shared")
    set_basename("service_plugin")
    set_extension(".qtplugin")
    set_prefixname("")
    
    -- Add Qt rules for proper Qt integration with MOC support
    add_rules("qt.shared")
    
    -- Add source files
    add_files("service_plugin.cpp")
    
    -- Add header files
    add_headerfiles("service_plugin.hpp")
    
    -- Add metadata files
    add_installfiles("service_plugin.json", {prefixdir = "lib/qtplugin/examples"})
    
    -- Add Qt packages for proper MOC support
    add_packages("qt6core")
    
    -- Add Qt frameworks
    add_frameworks("QtCore")
    
    -- Link with QtForge (assuming it's available in parent directory)
    add_deps("QtForgeCore")
    add_includedirs("../../../include", {public = false})
    
    -- Set output directory
    set_targetdir("$(buildir)/lib/qtplugin/examples")
    
    -- Set version
    set_version("1.0.0")
    
    -- Compiler definitions
    add_defines("QT_PLUGIN")
    
    -- Set visibility
    set_symbols("hidden")
    
    -- Install plugin
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "lib/qtplugin/examples"})
    
    -- Copy metadata to build directory
    after_build(function (target)
        os.cp("service_plugin.json", target:targetdir())
    end)
target_end()

-- Service Plugin Test target
target("ServicePluginTest")
    set_kind("binary")
    set_basename("service_plugin_test")
    
    -- Add Qt rules
    add_rules("qt.console")
    
    -- Add source files
    add_files("test_service_plugin.cpp")
    
    -- Add Qt packages
    add_packages("qt6core")
    
    -- Add Qt frameworks
    add_frameworks("QtCore")
    
    -- Link with QtForge
    add_deps("QtForgeCore")
    add_includedirs("../../../include", {public = false})
    
    -- Set output directory
    set_targetdir("$(buildir)/bin/examples")
    
    -- Set version
    set_version("1.0.0")
    
    -- Install test
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin/examples"})
    
    -- Copy plugin and metadata to test directory after build
    after_build(function (target)
        local plugin_target = target:dep("ServicePlugin")
        if plugin_target then
            os.cp(plugin_target:targetfile(), target:targetdir())
            os.cp("service_plugin.json", target:targetdir())
        end
    end)
target_end()

-- Service Plugin Task Test target
target("ServicePluginTaskTest")
    set_kind("binary")
    set_basename("service_plugin_task_test")
    
    -- Add Qt rules
    add_rules("qt.console")
    
    -- Add source files
    add_files("test_task_processing.cpp")
    
    -- Add Qt packages
    add_packages("qt6core")
    
    -- Add Qt frameworks
    add_frameworks("QtCore")
    
    -- Link with QtForge
    add_deps("QtForgeCore")
    add_includedirs("../../../include", {public = false})
    
    -- Set output directory
    set_targetdir("$(buildir)/bin/examples")
    
    -- Set version
    set_version("1.0.0")
    
    -- Install test
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "bin/examples"})
target_end()

-- Install documentation
add_installfiles("README.md", {prefixdir = "share/doc/qtplugin/examples/service_plugin"})

-- Add tests
add_tests("ServicePluginTest", {
    kind = "binary",
    files = "test_service_plugin.cpp",
    timeout = 30,
    labels = {"service", "plugin", "example"}
})
