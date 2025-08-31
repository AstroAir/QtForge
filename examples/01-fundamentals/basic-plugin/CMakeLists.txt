cmake_minimum_required(VERSION 3.21)
project(BasicPlugin VERSION 2.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 and QtForge are already found by parent CMakeLists.txt
# find_package(Qt6 REQUIRED COMPONENTS Core)
# find_package(QtForge REQUIRED COMPONENTS Core)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Plugin sources
set(PLUGIN_SOURCES
    basic_plugin.cpp
)

set(PLUGIN_HEADERS
    basic_plugin.hpp
)

set(PLUGIN_METADATA
    basic_plugin.json
)

# Create the plugin as a shared library for testing
add_library(BasicPlugin SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

# Set plugin properties
set_target_properties(BasicPlugin PROPERTIES
    OUTPUT_NAME "basic_plugin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    AUTOMOC ON
)

# Link QtForge
target_link_libraries(BasicPlugin
    PRIVATE
        QtForge::Core
        Qt6::Core
)

# Set plugin properties
set_target_properties(BasicPlugin PROPERTIES
    OUTPUT_NAME "basic_plugin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# Create test executable
add_executable(BasicPluginTest
    test_basic_plugin.cpp
)

target_link_libraries(BasicPluginTest
    PRIVATE
        QtForge::Core
        Qt6::Core
        BasicPlugin
)

# Install plugin
install(TARGETS BasicPlugin
    LIBRARY DESTINATION plugins
    RUNTIME DESTINATION plugins
)

# Install metadata
install(FILES ${PLUGIN_METADATA}
    DESTINATION plugins
)

# Install test
install(TARGETS BasicPluginTest
    RUNTIME DESTINATION bin
)
