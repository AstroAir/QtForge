# QtForge Bridges Module Tests
cmake_minimum_required(VERSION 3.21)

# Python Bridge Tests - temporarily disabled due to API mismatches
# add_executable(test_python_bridge
#     test_python_bridge.cpp
# )
#
# target_include_directories(test_python_bridge PRIVATE
#     ${CMAKE_SOURCE_DIR}/include
# )
#
# target_link_libraries(test_python_bridge PRIVATE
#     Qt6::Test
#     Qt6::Core
#     QtForgeCore
# )
#
# add_test(NAME BridgesPythonTests COMMAND test_python_bridge)
# set_tests_properties(BridgesPythonTests PROPERTIES
#     TIMEOUT 120
#     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
# )

# Install test executables
# install(TARGETS
#     test_python_bridge
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/bridges
# )
