# Request-Response Plugin Example
cmake_minimum_required(VERSION 3.16)

project(RequestResponsePluginExample VERSION 1.0.0)

# Find required packages (Qt6 is already found by parent CMakeLists.txt)

# Create the plugin library
add_library(request_response_plugin SHARED
    request_response_plugin.cpp
    request_response_plugin.hpp
)

# Set target properties
set_target_properties(request_response_plugin PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
    PREFIX ""
    SUFFIX ".qtplugin"
)

# Link libraries
target_link_libraries(request_response_plugin
    Qt6::Core
    QtForgeCore
)

# Include directories
target_include_directories(request_response_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Copy plugin metadata
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/request_response_plugin.json
    ${CMAKE_CURRENT_BINARY_DIR}/request_response_plugin.json
    COPYONLY
)

# Install plugin
install(TARGETS request_response_plugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtforge/plugins
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/qtforge/plugins
)

install(FILES request_response_plugin.json
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtforge/plugins
)

# Create test executable
add_executable(RequestResponsePluginTest
    test_request_response_plugin.cpp
)

target_link_libraries(RequestResponsePluginTest
    Qt6::Core
    QtForgeCore
    request_response_plugin
)

target_include_directories(RequestResponsePluginTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(RequestResponsePluginTest PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

# Add test
enable_testing()
add_test(NAME RequestResponsePluginTest COMMAND RequestResponsePluginTest)
