# CMakeLists.txt for Python Plugin Bridge Test

cmake_minimum_required(VERSION 3.16)

# Find Qt6 - use parent's Qt6 configuration
if(NOT TARGET Qt6::Core)
    find_package(Qt6 REQUIRED COMPONENTS Core)
endif()

# Create test executable
add_executable(python_bridge_example test_bridge.cpp)

# Link with Qt and QtForge
target_link_libraries(python_bridge_example
    Qt6::Core
    QtForgeCore
)

# Include directories
target_include_directories(python_bridge_example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Copy Python plugin to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/sample_plugin.py
    ${CMAKE_CURRENT_BINARY_DIR}/sample_plugin.py
    COPYONLY
)

# Copy Python bridge script to build directory
configure_file(
    ${CMAKE_SOURCE_DIR}/python_bridge.py
    ${CMAKE_CURRENT_BINARY_DIR}/python_bridge.py
    COPYONLY
)
