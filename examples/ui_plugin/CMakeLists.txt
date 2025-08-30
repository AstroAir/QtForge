# UI Plugin Example CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

project(UIPluginExample
    VERSION 1.0.0
    DESCRIPTION "Comprehensive UI plugin demonstrating Qt Widgets integration and theme support"
    LANGUAGES CXX
)

# Qt6 is already found by the parent project

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Widget Components Library
add_library(ui_plugin_widgets STATIC
    widgets/demo_widget.cpp
    widgets/demo_widget.hpp
)

target_link_libraries(ui_plugin_widgets
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
)

target_include_directories(ui_plugin_widgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(ui_plugin_widgets PROPERTIES
    AUTOMOC ON
)

# Dialog Components Library
add_library(ui_plugin_dialogs STATIC
    dialogs/settings_dialog.cpp
    dialogs/settings_dialog.hpp
    dialogs/about_dialog.cpp
    dialogs/about_dialog.hpp
)

target_link_libraries(ui_plugin_dialogs
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
)

target_include_directories(ui_plugin_dialogs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(ui_plugin_dialogs PROPERTIES
    AUTOMOC ON
)

# Core Plugin Library
add_library(ui_plugin_core_lib STATIC
    core/ui_plugin_core.cpp
    core/ui_plugin_core.hpp
)

target_link_libraries(ui_plugin_core_lib
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        QtForgeCore
        ui_plugin_widgets
        ui_plugin_dialogs
)

target_include_directories(ui_plugin_core_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(ui_plugin_core_lib PROPERTIES
    AUTOMOC ON
)

# Plugin sources (reorganized)
set(PLUGIN_SOURCES
    core/ui_plugin_core.hpp
    core/ui_plugin_core.cpp
)

# Plugin metadata
set(PLUGIN_METADATA
    ui_plugin.json
)

# Create the plugin library (reorganized)
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

# Link libraries (with reorganized components)
target_link_libraries(UIPlugin
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        QtForgeCore
        ui_plugin_widgets
        ui_plugin_dialogs
        ui_plugin_core_lib
)

# Reorganized Example Application
add_executable(ui_plugin_example
    main.cpp
)

target_link_libraries(ui_plugin_example
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        QtForgeCore
        ui_plugin_widgets
        ui_plugin_dialogs
        ui_plugin_core_lib
)

target_include_directories(ui_plugin_example
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(ui_plugin_example PROPERTIES
    AUTOMOC ON
    OUTPUT_NAME "ui_plugin_reorganized"
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
        QtForgeCore
)

target_link_libraries(UIPluginTestCLI
    PRIVATE
        Qt6::Core
        QtForgeCore
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

# Install reorganized example application
install(TARGETS ui_plugin_example
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
    COMPONENT Examples
)

# Install reorganized source files for reference
install(FILES
    main.cpp
    README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples/ui_plugin
    COMPONENT Documentation
)

# Install modular component files
install(DIRECTORY
    widgets/
    dialogs/
    core/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples/ui_plugin
    FILES_MATCHING PATTERN "*.cpp" PATTERN "*.hpp"
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
