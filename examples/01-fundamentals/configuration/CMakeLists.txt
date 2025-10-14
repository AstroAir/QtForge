# Configuration Plugin Example
cmake_minimum_required(VERSION 3.16)

project(ConfigurationPluginExample VERSION 1.0.0)

# Find required packages (Qt6 is already found by parent CMakeLists.txt)

# Create the plugin library
add_library(configuration_plugin SHARED configuration_plugin.cpp
                                        configuration_plugin.hpp)

# Set target properties
set_target_properties(
  configuration_plugin
  PROPERTIES CXX_STANDARD 20
             CXX_STANDARD_REQUIRED ON
             AUTOMOC ON
             PREFIX ""
             SUFFIX ".qtplugin")

# Link libraries
target_link_libraries(configuration_plugin Qt6::Core QtForgeCore)

# Include directories
target_include_directories(
  configuration_plugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                               ${CMAKE_SOURCE_DIR}/src)

# Copy plugin metadata
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configuration_plugin.json
               ${CMAKE_CURRENT_BINARY_DIR}/configuration_plugin.json COPYONLY)

# Install plugin
install(
  TARGETS configuration_plugin
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtforge/plugins
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/qtforge/plugins)

install(FILES configuration_plugin.json
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtforge/plugins)

# Create test executable
add_executable(ConfigurationPluginTest test_configuration_plugin.cpp)

target_link_libraries(ConfigurationPluginTest Qt6::Core QtForgeCore
                      configuration_plugin)

target_include_directories(
  ConfigurationPluginTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                  ${CMAKE_SOURCE_DIR}/src)

set_target_properties(
  ConfigurationPluginTest
  PROPERTIES CXX_STANDARD 20
             CXX_STANDARD_REQUIRED ON
             AUTOMOC ON)

# Add test
enable_testing()
add_test(NAME ConfigurationPluginTest COMMAND ConfigurationPluginTest)
