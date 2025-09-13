# CMakeLists.txt for Python Bridge Tests

cmake_minimum_required(VERSION 3.16)

# Find required packages - use parent's Qt6 configuration
if(NOT TARGET Qt6::Core)
    find_package(Qt6 REQUIRED COMPONENTS Core Test Concurrent)
endif()

# Only build if Python bindings are enabled
if(NOT QTFORGE_BUILD_PYTHON_BINDINGS)
    message(STATUS "Python bridge tests skipped (QTFORGE_BUILD_PYTHON_BINDINGS=OFF)")
    return()
endif()

if(NOT QTFORGE_PYTHON_FOUND)
    message(STATUS "Python bridge tests skipped (Python not found)")
    return()
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# ============================================================================
# C++ Tests
# ============================================================================

# Simple Bridge Test (for debugging)
add_executable(test_simple_bridge
    test_simple_bridge.cpp
)

target_link_libraries(test_simple_bridge
    Qt6::Core
    Qt6::Test
    QtForgeCore
)

add_test(NAME SimpleBridgeTest
    COMMAND test_simple_bridge
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Complete Python Plugin Bridge Test
add_executable(test_python_plugin_bridge_complete
    test_python_plugin_bridge_complete.cpp
)

target_link_libraries(test_python_plugin_bridge_complete
    Qt6::Core
    Qt6::Test
    Qt6::Concurrent
    QtForgeCore
)

target_compile_definitions(test_python_plugin_bridge_complete PRIVATE
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
)

add_test(NAME PythonPluginBridgeComplete
    COMMAND test_python_plugin_bridge_complete
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Event System Test
add_executable(test_event_system
    test_event_system.cpp
)

target_link_libraries(test_event_system
    Qt6::Core
    Qt6::Test
    QtForgeCore
)

target_compile_definitions(test_event_system PRIVATE
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
)

add_test(NAME PythonBridgeEventSystem
    COMMAND test_event_system
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# ============================================================================
# Python Tests
# ============================================================================

# Python Bridge Script Test
add_test(NAME PythonBridgeScript
    COMMAND ${QTFORGE_PYTHON_EXECUTABLE} test_python_bridge_script.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(PythonBridgeScript PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}:${CMAKE_BINARY_DIR}:$ENV{PYTHONPATH}"
)

# Enhanced Python Bindings Test
add_test(NAME PythonBindingsEnhanced
    COMMAND ${QTFORGE_PYTHON_EXECUTABLE} test_python_bindings_enhanced.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties(PythonBindingsEnhanced PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}:${CMAKE_BINARY_DIR}:$ENV{PYTHONPATH}"
)

# ============================================================================
# Integration Tests
# ============================================================================

# Full Integration Test
add_executable(test_python_bridge_integration
    test_python_bridge_integration.cpp
)

target_link_libraries(test_python_bridge_integration
    Qt6::Core
    Qt6::Test
    QtForgeCore
)

add_test(NAME PythonBridgeIntegration
    COMMAND test_python_bridge_integration
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# ============================================================================
# Performance Tests
# ============================================================================

# Performance Test
add_executable(test_python_bridge_performance
    test_python_bridge_performance.cpp
)

target_link_libraries(test_python_bridge_performance
    Qt6::Core
    Qt6::Test
    QtForgeCore
)

add_test(NAME PythonBridgePerformance
    COMMAND test_python_bridge_performance
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# ============================================================================
# Test Data and Resources
# ============================================================================

# Copy Python bridge script to test directory
configure_file(
    ${CMAKE_SOURCE_DIR}/python_bridge.py
    ${CMAKE_CURRENT_BINARY_DIR}/python_bridge.py
    COPYONLY
)

# Copy sample plugin to test directory
configure_file(
    ${CMAKE_SOURCE_DIR}/examples/python_plugins/sample_plugin.py
    ${CMAKE_CURRENT_BINARY_DIR}/sample_plugin.py
    COPYONLY
)

# Create test plugin directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_plugins)

# ============================================================================
# Test Labels and Properties
# ============================================================================

# Set test labels for categorization with reduced timeouts
set_tests_properties(SimpleBridgeTest PROPERTIES
    LABELS "Python;Bridge;Unit;Basic"
    TIMEOUT 15
)

set_tests_properties(PythonPluginBridgeComplete PROPERTIES
    LABELS "Python;Bridge;Unit"
    TIMEOUT 30
)

set_tests_properties(PythonBridgeEventSystem PROPERTIES
    LABELS "Python;Bridge;Events;Unit"
    TIMEOUT 30
)

set_tests_properties(PythonBridgeScript PROPERTIES
    LABELS "Python;Bridge;Script;Unit"
    TIMEOUT 15
)

set_tests_properties(PythonBindingsEnhanced PROPERTIES
    LABELS "Python;Bindings;Unit"
    TIMEOUT 15
)

set_tests_properties(PythonBridgeIntegration PROPERTIES
    LABELS "Python;Bridge;Integration"
    TIMEOUT 60
)

set_tests_properties(PythonBridgePerformance PROPERTIES
    LABELS "Python;Bridge;Performance"
    TIMEOUT 120
)

# ============================================================================
# Custom Test Targets
# ============================================================================

# Run all Python bridge tests
add_custom_target(test_python_bridge
    COMMAND ${CMAKE_CTEST_COMMAND} -L "Python" --verbose
    DEPENDS
        test_python_plugin_bridge_complete
        test_event_system
        test_python_bridge_integration
        test_python_bridge_performance
    COMMENT "Running all Python bridge tests"
)

# Run only unit tests
add_custom_target(test_python_bridge_unit
    COMMAND ${CMAKE_CTEST_COMMAND} -L "Unit" --verbose
    DEPENDS
        test_python_plugin_bridge_complete
        test_event_system
    COMMENT "Running Python bridge unit tests"
)

# Run only integration tests
add_custom_target(test_python_bridge_integration_only
    COMMAND ${CMAKE_CTEST_COMMAND} -L "Integration" --verbose
    DEPENDS test_python_bridge_integration
    COMMENT "Running Python bridge integration tests"
)

# Run only performance tests
add_custom_target(test_python_bridge_performance_only
    COMMAND ${CMAKE_CTEST_COMMAND} -L "Performance" --verbose
    DEPENDS test_python_bridge_performance
    COMMENT "Running Python bridge performance tests"
)

# ============================================================================
# Test Coverage (if available)
# ============================================================================

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Add coverage flags for debug builds
    target_compile_options(test_python_plugin_bridge_complete PRIVATE --coverage)
    target_link_options(test_python_plugin_bridge_complete PRIVATE --coverage)

    target_compile_options(test_event_system PRIVATE --coverage)
    target_link_options(test_event_system PRIVATE --coverage)

    target_compile_options(test_python_bridge_integration PRIVATE --coverage)
    target_link_options(test_python_bridge_integration PRIVATE --coverage)
endif()

# ============================================================================
# Test Summary
# ============================================================================

message(STATUS "Python Bridge Tests Configuration:")
message(STATUS "  - Complete Bridge Test: ENABLED")
message(STATUS "  - Event System Test: ENABLED")
message(STATUS "  - Python Script Test: ENABLED")
message(STATUS "  - Enhanced Bindings Test: ENABLED")
message(STATUS "  - Integration Test: ENABLED")
message(STATUS "  - Performance Test: ENABLED")
message(STATUS "  - Python Executable: ${QTFORGE_PYTHON_EXECUTABLE}")
message(STATUS "  - Test Timeout: 60-300 seconds")
message(STATUS "  - Coverage: ${CMAKE_BUILD_TYPE} build")
