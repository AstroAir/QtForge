# Basic Plugin Example CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

project(BasicPluginExample)

# Qt6 is already found by the parent project

# Create the plugin library
add_library(BasicPlugin MODULE
    basic_plugin.hpp
    basic_plugin.cpp
    basic_plugin.json
)

# Set plugin properties
set_target_properties(BasicPlugin PROPERTIES
    OUTPUT_NAME basic_plugin
    PREFIX ""
    SUFFIX ".qtplugin"
    AUTOMOC ON
)

# Include directories
target_include_directories(BasicPlugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(BasicPlugin PRIVATE
    Qt6::Core
    QtForgeCore
)

# Install plugin
install(TARGETS BasicPlugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtplugin/examples
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/qtplugin/examples
)

# Install metadata
install(FILES basic_plugin.json
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtplugin/examples
)

# Create a simple test application that uses the plugin
add_executable(BasicPluginTest
    test_basic_plugin.cpp
)

target_include_directories(BasicPluginTest PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(BasicPluginTest PRIVATE
    Qt6::Core
    QtForgeCore
)

# Install test application
install(TARGETS BasicPluginTest
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/qtplugin/examples
)
