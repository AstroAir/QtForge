# QtForge Lua Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Only build Lua tests if Lua bindings are enabled
if(NOT QTFORGE_LUA_BINDINGS)
    message(STATUS "Skipping Lua tests - QTFORGE_LUA_BINDINGS not enabled")
    return()
endif()

message(STATUS "Building QtForge Lua tests")

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Test Core)

# Find sol2 if available
find_package(sol2 QUIET)
if(sol2_FOUND)
    message(STATUS "Found sol2 for Lua tests")
    set(LUA_TEST_LIBRARIES Qt6::Test Qt6::Core qtforge sol2::sol2)
else()
    message(STATUS "sol2 not found - using system Lua")
    set(LUA_TEST_LIBRARIES Qt6::Test Qt6::Core qtforge)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Test sources
set(LUA_TEST_SOURCES
    test_lua_bindings.cpp
    test_lua_plugin_bridge.cpp
    test_lua_plugin_loader.cpp
    test_qt_conversions.cpp
    test_lua_integration.cpp
)

# Create individual test executables
foreach(test_source ${LUA_TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    set(full_test_name "QtForge_Lua_${test_name}")
    
    add_executable(${full_test_name} ${test_source})
    target_link_libraries(${full_test_name} ${LUA_TEST_LIBRARIES})
    
    # Set properties
    set_target_properties(${full_test_name} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        AUTOMOC ON
    )
    
    # Add compile definitions
    target_compile_definitions(${full_test_name} PRIVATE 
        QTFORGE_LUA_BINDINGS
        QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
    )
    
    # Add Lua include directories if available
    if(LUA_INCLUDE_DIR)
        target_include_directories(${full_test_name} PRIVATE ${LUA_INCLUDE_DIR})
    endif()
    
    # Register test with CTest
    add_test(NAME ${full_test_name} COMMAND ${full_test_name})
    set_tests_properties(${full_test_name} PROPERTIES
        TIMEOUT 300
        LABELS "Lua;Bindings"
    )
endforeach()

# Create comprehensive Lua test suite
add_executable(QtForge_Lua_TestSuite
    test_lua_bindings.cpp
    test_lua_plugin_bridge.cpp
    test_lua_plugin_loader.cpp
    test_qt_conversions.cpp
    test_lua_integration.cpp
)

target_link_libraries(QtForge_Lua_TestSuite ${LUA_TEST_LIBRARIES})

set_target_properties(QtForge_Lua_TestSuite PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_compile_definitions(QtForge_Lua_TestSuite PRIVATE 
    QTFORGE_LUA_BINDINGS
    QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
)

if(LUA_INCLUDE_DIR)
    target_include_directories(QtForge_Lua_TestSuite PRIVATE ${LUA_INCLUDE_DIR})
endif()

# Register comprehensive test
add_test(NAME QtForge_Lua_ComprehensiveSuite COMMAND QtForge_Lua_TestSuite)
set_tests_properties(QtForge_Lua_ComprehensiveSuite PROPERTIES
    TIMEOUT 600
    LABELS "Lua;Comprehensive"
)

# Custom test targets
add_custom_target(run_lua_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -L "Lua" --verbose
    COMMENT "Running QtForge Lua tests"
    DEPENDS ${LUA_TEST_SOURCES}
)

add_custom_target(run_lua_bindings_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "test_lua_bindings" --verbose
    COMMENT "Running Lua bindings tests"
    DEPENDS QtForge_Lua_test_lua_bindings
)

add_custom_target(run_lua_bridge_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "test_lua_plugin_bridge" --verbose
    COMMENT "Running Lua plugin bridge tests"
    DEPENDS QtForge_Lua_test_lua_plugin_bridge
)

add_custom_target(run_lua_loader_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "test_lua_plugin_loader" --verbose
    COMMENT "Running Lua plugin loader tests"
    DEPENDS QtForge_Lua_test_lua_plugin_loader
)

add_custom_target(run_lua_conversion_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "test_qt_conversions" --verbose
    COMMENT "Running Qt-Lua conversion tests"
    DEPENDS QtForge_Lua_test_qt_conversions
)

add_custom_target(run_lua_integration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "test_lua_integration" --verbose
    COMMENT "Running Lua integration tests"
    DEPENDS QtForge_Lua_test_lua_integration
)

# Test data directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_data)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_plugins)

# Create sample Lua plugins for testing
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test_plugins/sample_plugin.lua
"--[[
@plugin_name: Sample Test Plugin
@plugin_description: A sample plugin for testing
@plugin_version: 1.0.0
@plugin_author: Test Suite
]]

local plugin = {}

function plugin.initialize()
    return {success = true, message = 'Plugin initialized'}
end

function plugin.get_info()
    return {
        name = 'Sample Test Plugin',
        version = '1.0.0',
        description = 'A sample plugin for testing'
    }
end

function plugin.execute_command(command, params)
    if command == 'ping' then
        return {success = true, result = 'pong'}
    elseif command == 'echo' then
        return {success = true, result = params.message or 'empty'}
    else
        return {success = false, error = 'Unknown command: ' .. command}
    end
end

return plugin
")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test_plugins/invalid_plugin.lua
"-- This is an invalid plugin with syntax errors
invalid lua syntax !!!")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test_plugins/minimal_plugin.lua
"return {}")

# Performance tests (if enabled)
if(QTFORGE_BUILD_PERFORMANCE_TESTS)
    add_executable(QtForge_Lua_PerformanceTests test_lua_performance.cpp)
    target_link_libraries(QtForge_Lua_PerformanceTests ${LUA_TEST_LIBRARIES})
    
    set_target_properties(QtForge_Lua_PerformanceTests PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        AUTOMOC ON
    )
    
    target_compile_definitions(QtForge_Lua_PerformanceTests PRIVATE QTFORGE_LUA_BINDINGS)
    
    add_test(NAME QtForge_Lua_Performance COMMAND QtForge_Lua_PerformanceTests)
    set_tests_properties(QtForge_Lua_Performance PROPERTIES
        TIMEOUT 900
        LABELS "Lua;Performance"
    )
endif()

# Memory tests (if enabled and Valgrind available)
if(QTFORGE_BUILD_MEMORY_TESTS)
    find_program(VALGRIND_PROGRAM valgrind)
    if(VALGRIND_PROGRAM AND NOT WIN32)
        add_test(NAME QtForge_Lua_MemoryLeakTests
                 COMMAND ${VALGRIND_PROGRAM}
                         --tool=memcheck
                         --leak-check=full
                         --error-exitcode=1
                         --suppressions=${CMAKE_CURRENT_SOURCE_DIR}/valgrind.supp
                         $<TARGET_FILE:QtForge_Lua_TestSuite>)
        set_tests_properties(QtForge_Lua_MemoryLeakTests PROPERTIES
            TIMEOUT 1200
            LABELS "Lua;Memory"
        )
    endif()
endif()

# Test coverage (if enabled)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    option(QTFORGE_LUA_ENABLE_COVERAGE "Enable Lua test coverage reporting" OFF)
    
    if(QTFORGE_LUA_ENABLE_COVERAGE)
        find_program(GCOV_PATH gcov)
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)
        
        if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
            message(STATUS "Lua test coverage reporting enabled")
            
            # Add coverage flags to Lua tests
            foreach(test_source ${LUA_TEST_SOURCES})
                get_filename_component(test_name ${test_source} NAME_WE)
                set(full_test_name "QtForge_Lua_${test_name}")
                target_compile_options(${full_test_name} PRIVATE --coverage)
                target_link_options(${full_test_name} PRIVATE --coverage)
            endforeach()
            
            # Coverage target for Lua tests
            add_custom_target(lua_coverage
                COMMAND ${LCOV_PATH} --directory . --capture --output-file lua_coverage.info
                COMMAND ${LCOV_PATH} --remove lua_coverage.info '/usr/*' --output-file lua_coverage.info
                COMMAND ${LCOV_PATH} --list lua_coverage.info
                COMMAND ${GENHTML_PATH} -o lua_coverage lua_coverage.info
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating Lua test coverage report"
            )
        endif()
    endif()
endif()

message(STATUS "Lua tests configuration completed")
message(STATUS "  Test executables: ${LUA_TEST_SOURCES}")
message(STATUS "  Test timeout: 300 seconds")
message(STATUS "  Test data directory: ${CMAKE_CURRENT_BINARY_DIR}/test_data")
message(STATUS "  Test plugins directory: ${CMAKE_CURRENT_BINARY_DIR}/test_plugins")
