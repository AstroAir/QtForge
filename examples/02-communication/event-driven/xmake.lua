-- Event-Driven Communication Example XMake Configuration
-- Demonstrates event-driven communication patterns with message filters

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Set project info
set_project("EventDrivenExample")
set_version("3.0.0")

-- Set C++ standard
set_languages("c++17")

-- Add Qt dependencies
add_requires("qt6core", {optional = true, configs = {shared = true}})

-- Event-Driven Example target
target("EventDrivenExample")
    set_kind("shared")
    set_basename("event_driven_example")
    
    -- Add Qt rules for proper Qt integration
    add_rules("qt.shared")
    
    -- Add source files
    add_files("message_filters.cpp")
    add_files("../messages/system_event_message.cpp")
    
    -- Add header files
    add_headerfiles("message_filters.hpp")
    add_headerfiles("../messages/system_event_message.hpp")
    
    -- Add Qt packages for proper MOC support
    add_packages("qt6core")
    
    -- Add Qt frameworks
    add_frameworks("QtCore")
    
    -- Link with QtForge (assuming it's available in parent directory)
    add_deps("QtForgeCore")
    add_includedirs("../../../include", {public = false})
    add_includedirs("..", {public = true})
    
    -- Set output directory
    set_targetdir("$(buildir)/lib")
    
    -- Set version
    set_version("3.0.0")
    
    -- Compile definitions
    add_defines("QT_NO_KEYWORDS")
    
    -- Export symbols for shared library
    add_defines("EVENT_DRIVEN_EXAMPLE_EXPORTS")
    
    -- Install library
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "lib"})
    
    -- Install headers
    add_installfiles("message_filters.hpp", {prefixdir = "include/qtplugin/examples/communication"})
    add_installfiles("../messages/system_event_message.hpp", {prefixdir = "include/qtplugin/examples/communication"})
target_end()
