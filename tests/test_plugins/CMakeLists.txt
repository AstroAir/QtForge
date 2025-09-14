# QtForge Test Plugins
cmake_minimum_required(VERSION 3.21)

# This directory contains test plugins used by other tests
# We build them as shared libraries for testing purposes

# Simple Plugin for testing
add_library(simple_plugin SHARED
    simple_plugin/simple_plugin.cpp
    simple_plugin/simple_plugin.hpp
)

target_include_directories(simple_plugin PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_plugin
)

target_link_libraries(simple_plugin PRIVATE
    Qt6::Core
    QtForgeCore
)

# Set plugin properties
set_target_properties(simple_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/plugins
    PREFIX ""  # Remove lib prefix for plugins
)

# Copy plugin metadata
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_plugin/simple_plugin.json
    ${CMAKE_CURRENT_BINARY_DIR}/plugins/simple_plugin.json
    COPYONLY
)

# Install test plugins
install(TARGETS simple_plugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtforge/test_plugins
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/qtforge/test_plugins
)

install(FILES
    simple_plugin/simple_plugin.json
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtforge/test_plugins
)

# Create a custom target for building all test plugins
add_custom_target(build_test_plugins
    DEPENDS simple_plugin
    COMMENT "Building all test plugins"
)
