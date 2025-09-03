# QtForge Integration Tests
cmake_minimum_required(VERSION 3.21)

# Note: Integration tests will be added here as needed
# This directory is reserved for cross-module integration tests

# Example placeholder for future integration tests
# add_executable(test_full_system_integration
#     test_full_system_integration.cpp
# )
#
# target_include_directories(test_full_system_integration PRIVATE
#     ${CMAKE_SOURCE_DIR}/include
# )
#
# target_link_libraries(test_full_system_integration PRIVATE
#     Qt6::Test
#     Qt6::Core
#     QtForgeCore
#     QtForgeSecurity
# )
#
# add_test(NAME IntegrationFullSystemTests COMMAND test_full_system_integration)
# set_tests_properties(IntegrationFullSystemTests PROPERTIES
#     TIMEOUT 300
#     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
##     ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR};${CMAKE_BINARY_DIR};D:/msys64/mingw64/bin;$ENV{PATH}"
# )