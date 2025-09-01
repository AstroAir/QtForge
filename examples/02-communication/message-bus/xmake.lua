-- Message Bus Communication Example XMake Configuration
-- Demonstrates inter-plugin communication using QtForge message bus

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Set project info
set_project("MessageBusExample")
set_version("3.0.0")

-- Set C++ standard
set_languages("c++17")

-- Add Qt dependencies
add_requires("qt6core", {optional = true, configs = {shared = true}})

-- Message Bus Example target
target("MessageBusExample")
    set_kind("shared")
    set_basename("message_bus_example")

    -- Add Qt rules for proper Qt integration
    add_rules("qt.shared")

    -- Add source files
    add_files("message_bus_example.cpp")

    -- Add header files
    add_headerfiles("message_bus_example.hpp")

    -- Add Qt packages for proper MOC support
    add_packages("qt6core")

    -- Add Qt frameworks
    add_frameworks("QtCore")

    -- Link with QtForge (assuming it's available in parent directory)
    add_deps("QtForgeCore")
    add_includedirs("../../../include", {public = false})

    -- Set output directory
    set_targetdir("$(buildir)/lib")

    -- Set version
    set_version("3.0.0")

    -- Compile definitions
    add_defines("QT_NO_KEYWORDS")

    -- Export symbols for shared library
    add_defines("MESSAGE_BUS_EXAMPLE_EXPORTS")

    -- Install library
    add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "lib"})

    -- Install headers
    add_installfiles("message_bus_example.hpp", {prefixdir = "include/qtplugin/examples/communication"})
target_end()
