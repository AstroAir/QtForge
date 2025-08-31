cmake_minimum_required(VERSION 3.21)
project(HelloWorldPlugin VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(QtForge REQUIRED COMPONENTS Core)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Plugin sources
set(PLUGIN_SOURCES
    hello_world_plugin.cpp
)

set(PLUGIN_HEADERS
    hello_world_plugin.hpp
)

set(PLUGIN_METADATA
    hello_world_plugin.json
)

# Create the plugin
qtforge_add_plugin(
    NAME HelloWorldPlugin
    SOURCES ${PLUGIN_SOURCES}
    HEADERS ${PLUGIN_HEADERS}
    QT_COMPONENTS Core
    METADATA ${PLUGIN_METADATA}
)

# Link QtForge
target_link_libraries(HelloWorldPlugin
    PRIVATE
        QtForge::Core
        Qt6::Core
)

# Set plugin properties
set_target_properties(HelloWorldPlugin PROPERTIES
    OUTPUT_NAME "hello_world_plugin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# Install plugin
install(TARGETS HelloWorldPlugin
    LIBRARY DESTINATION plugins
    RUNTIME DESTINATION plugins
)

# Install metadata
install(FILES ${PLUGIN_METADATA}
    DESTINATION plugins
)
