# UI Plugin Example CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

project(UIPluginExample
    VERSION 1.0.0
    DESCRIPTION "Comprehensive UI plugin demonstrating Qt Widgets integration and theme support"
    LANGUAGES CXX
)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Plugin sources
set(PLUGIN_SOURCES
    ui_plugin.hpp
    ui_plugin.cpp
    ui_plugin_impl.cpp
    ui_plugin_helpers.cpp
    ui_plugin_commands.cpp
)

# Plugin metadata
set(PLUGIN_METADATA
    ui_plugin.json
)

# Create the plugin library
add_library(UIPlugin MODULE ${PLUGIN_SOURCES})

# Set plugin properties
set_target_properties(UIPlugin PROPERTIES
    OUTPUT_NAME "ui_plugin"
    SUFFIX ".qtplugin"
    PREFIX ""
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    AUTOMOC ON
)

# Link libraries
target_link_libraries(UIPlugin
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        QtPluginCore
)

# Include directories
target_include_directories(UIPlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# Compiler definitions
target_compile_definitions(UIPlugin
    PRIVATE
        QT_PLUGIN
        QTPLUGIN_BUILD_UI
)

# Copy metadata file to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ui_plugin.json
    ${CMAKE_CURRENT_BINARY_DIR}/ui_plugin.json
    COPYONLY
)

# Test application (GUI)
add_executable(UIPluginTest
    test_ui_plugin.cpp
)

# Command-line test application
add_executable(UIPluginTestCLI
    test_ui_plugin_cli.cpp
)

target_link_libraries(UIPluginTest
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        QtPluginCore
)

target_link_libraries(UIPluginTestCLI
    PRIVATE
        Qt6::Core
        QtPluginCore
)

target_include_directories(UIPluginTest
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(UIPluginTestCLI
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# Enable AUTOMOC for test application
set_target_properties(UIPluginTest PROPERTIES
    AUTOMOC ON
)

# Copy plugin to test directory
add_custom_command(TARGET UIPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:UIPlugin>
        $<TARGET_FILE_DIR:UIPluginTest>/
    COMMENT "Copying UIPlugin to test directory"
)

# Copy metadata to test directory
add_custom_command(TARGET UIPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/ui_plugin.json
        $<TARGET_FILE_DIR:UIPluginTest>/
    COMMENT "Copying plugin metadata to test directory"
)

# Install targets
install(TARGETS UIPlugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtplugin/examples
    COMPONENT Examples
)

install(FILES ui_plugin.json
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtplugin/examples
    COMPONENT Examples
)

install(TARGETS UIPluginTest
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
    COMPONENT Examples
)

# Documentation
install(FILES README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples/ui_plugin
    COMPONENT Documentation
)

# Enable testing
enable_testing()

# Add test
add_test(
    NAME UIPluginTest
    COMMAND UIPluginTest
    WORKING_DIRECTORY $<TARGET_FILE_DIR:UIPluginTest>
)

# Set test properties
set_tests_properties(UIPluginTest PROPERTIES
    TIMEOUT 60
    LABELS "ui;plugin;example;widgets"
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    set_target_properties(UIPlugin PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
elseif(APPLE)
    # macOS-specific settings
    set_target_properties(UIPlugin PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

# Debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(UIPlugin PRIVATE DEBUG_UI_PLUGIN)
    target_compile_definitions(UIPluginTest PRIVATE DEBUG_UI_PLUGIN)
endif()
