# Service Plugin Example CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

project(ServicePluginExample
    VERSION 1.0.0
    DESCRIPTION "Comprehensive service plugin demonstrating background processing and MessageBus integration"
    LANGUAGES CXX
)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Plugin sources
set(PLUGIN_SOURCES
    service_plugin.hpp
    service_plugin.cpp
)

# Plugin metadata
set(PLUGIN_METADATA
    service_plugin.json
)

# Create the plugin library
add_library(ServicePlugin MODULE ${PLUGIN_SOURCES})

# Set plugin properties
set_target_properties(ServicePlugin PROPERTIES
    OUTPUT_NAME "service_plugin"
    SUFFIX ".qtplugin"
    PREFIX ""
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    AUTOMOC ON
)

# Link libraries
target_link_libraries(ServicePlugin
    PRIVATE
        Qt6::Core
        QtPluginCore
)

# Include directories
target_include_directories(ServicePlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# Compiler definitions
target_compile_definitions(ServicePlugin
    PRIVATE
        QT_PLUGIN
)

# Copy metadata file to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/service_plugin.json
    ${CMAKE_CURRENT_BINARY_DIR}/service_plugin.json
    COPYONLY
)

# Test application
add_executable(ServicePluginTest
    test_service_plugin.cpp
)

# Task processing test
add_executable(ServicePluginTaskTest
    test_task_processing.cpp
)

target_link_libraries(ServicePluginTest
    PRIVATE
        Qt6::Core
        QtPluginCore
)

target_link_libraries(ServicePluginTaskTest
    PRIVATE
        Qt6::Core
        QtPluginCore
)

target_include_directories(ServicePluginTest
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(ServicePluginTaskTest
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# Copy plugin to test directory
add_custom_command(TARGET ServicePlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:ServicePlugin>
        $<TARGET_FILE_DIR:ServicePluginTest>/
    COMMENT "Copying ServicePlugin to test directory"
)

# Copy metadata to test directory
add_custom_command(TARGET ServicePlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/service_plugin.json
        $<TARGET_FILE_DIR:ServicePluginTest>/
    COMMENT "Copying plugin metadata to test directory"
)

# Install targets
install(TARGETS ServicePlugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtplugin/examples
    COMPONENT Examples
)

install(FILES service_plugin.json
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtplugin/examples
    COMPONENT Examples
)

install(TARGETS ServicePluginTest
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
    COMPONENT Examples
)

# Documentation
install(FILES README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples/service_plugin
    COMPONENT Documentation
)

# Enable testing
enable_testing()

# Add test
add_test(
    NAME ServicePluginTest
    COMMAND ServicePluginTest
    WORKING_DIRECTORY $<TARGET_FILE_DIR:ServicePluginTest>
)

# Set test properties
set_tests_properties(ServicePluginTest PROPERTIES
    TIMEOUT 30
    LABELS "service;plugin;example"
)
