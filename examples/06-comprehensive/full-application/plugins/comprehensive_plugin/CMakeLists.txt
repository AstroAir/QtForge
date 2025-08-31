cmake_minimum_required(VERSION 3.21)
project(ComprehensivePlugin VERSION 3.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
find_package(QtForge REQUIRED COMPONENTS 
    Core 
    Security 
    Communication 
    Monitoring
    Orchestration
    Transactions
    Marketplace
    Threading
)

# Optional Python support
if(QTFORGE_PYTHON_SUPPORT)
    find_package(QtForge REQUIRED COMPONENTS Python)
endif()

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Plugin sources
set(PLUGIN_SOURCES
    comprehensive_plugin.cpp
)

set(PLUGIN_HEADERS
    comprehensive_plugin.hpp
)

set(PLUGIN_METADATA
    comprehensive_plugin.json
)

# Create the plugin library
add_library(comprehensive_plugin SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    ${PLUGIN_METADATA}
)

# Set plugin properties
set_target_properties(comprehensive_plugin PROPERTIES
    OUTPUT_NAME "comprehensive_plugin"
    SUFFIX ".qtplugin"
    PREFIX ""
    DEBUG_POSTFIX "_d"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Link libraries
target_link_libraries(comprehensive_plugin
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    QtForge::Core
    QtForge::Security
    QtForge::Communication
    QtForge::Monitoring
    QtForge::Orchestration
    QtForge::Transactions
    QtForge::Marketplace
    QtForge::Threading
)

# Optional Python support
if(QTFORGE_PYTHON_SUPPORT)
    target_link_libraries(comprehensive_plugin QtForge::Python)
    target_compile_definitions(comprehensive_plugin PRIVATE QTFORGE_PYTHON_SUPPORT)
endif()

# Include directories
target_include_directories(comprehensive_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Compiler-specific options
if(MSVC)
    target_compile_options(comprehensive_plugin PRIVATE /W4 /permissive-)
else()
    target_compile_options(comprehensive_plugin PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy plugin to plugins directory
add_custom_command(TARGET comprehensive_plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:comprehensive_plugin>
    ${CMAKE_BINARY_DIR}/plugins/$<TARGET_FILE_NAME:comprehensive_plugin>
    COMMENT "Copying plugin to plugins directory"
)

# Copy metadata file
add_custom_command(TARGET comprehensive_plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/comprehensive_plugin.json
    ${CMAKE_BINARY_DIR}/plugins/comprehensive_plugin.json
    COMMENT "Copying plugin metadata"
)

# Test executable for the plugin
add_executable(test_comprehensive_plugin
    test_comprehensive_plugin.cpp
    ${PLUGIN_HEADERS}
)

target_link_libraries(test_comprehensive_plugin
    Qt6::Core
    Qt6::Test
    QtForge::Core
    comprehensive_plugin
)

# Add test
if(BUILD_TESTING)
    add_test(NAME ComprehensivePluginTest COMMAND test_comprehensive_plugin)
    set_tests_properties(ComprehensivePluginTest PROPERTIES
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endif()

# Install plugin
install(TARGETS comprehensive_plugin
    LIBRARY DESTINATION lib/qtforge/plugins
    RUNTIME DESTINATION lib/qtforge/plugins
)

# Install metadata
install(FILES comprehensive_plugin.json
    DESTINATION lib/qtforge/plugins
)

# Install headers (for development)
install(FILES ${PLUGIN_HEADERS}
    DESTINATION include/qtforge/examples/comprehensive_plugin
)

# Package information
set_target_properties(comprehensive_plugin PROPERTIES
    DESCRIPTION "Comprehensive plugin demonstrating all QtForge features"
    AUTHOR "QtForge Development Team"
    LICENSE "MIT"
    HOMEPAGE "https://github.com/qtforge/qtforge"
)
