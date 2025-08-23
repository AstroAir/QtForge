# Test script for QtForge modular build system
# This script tests various build configurations and options

cmake_minimum_required(VERSION 3.21)

# Test configuration
set(TEST_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(TEST_BUILD_DIR "${CMAKE_CURRENT_LIST_DIR}/../build-test")

# Function to run a build test
function(run_build_test TEST_NAME)
    message(STATUS "Running build test: ${TEST_NAME}")
    
    # Parse arguments
    cmake_parse_arguments(TEST
        "SHOULD_FAIL"
        "PRESET"
        "OPTIONS"
        ${ARGN}
    )
    
    # Create unique build directory
    set(BUILD_DIR "${TEST_BUILD_DIR}/${TEST_NAME}")
    file(REMOVE_RECURSE "${BUILD_DIR}")
    file(MAKE_DIRECTORY "${BUILD_DIR}")
    
    # Prepare CMake command
    if(TEST_PRESET)
        set(CMAKE_CMD cmake --preset ${TEST_PRESET})
    else()
        set(CMAKE_CMD cmake -S "${TEST_SOURCE_DIR}" -B "${BUILD_DIR}")
        if(TEST_OPTIONS)
            list(APPEND CMAKE_CMD ${TEST_OPTIONS})
        endif()
    endif()
    
    # Run configuration
    execute_process(
        COMMAND ${CMAKE_CMD}
        WORKING_DIRECTORY "${TEST_SOURCE_DIR}"
        RESULT_VARIABLE CONFIG_RESULT
        OUTPUT_VARIABLE CONFIG_OUTPUT
        ERROR_VARIABLE CONFIG_ERROR
    )
    
    # Check configuration result
    if(TEST_SHOULD_FAIL)
        if(CONFIG_RESULT EQUAL 0)
            message(FATAL_ERROR "Test ${TEST_NAME} should have failed but succeeded")
        else()
            message(STATUS "Test ${TEST_NAME} failed as expected")
            return()
        endif()
    else()
        if(NOT CONFIG_RESULT EQUAL 0)
            message(FATAL_ERROR "Test ${TEST_NAME} configuration failed: ${CONFIG_ERROR}")
        endif()
    endif()
    
    # Run build
    execute_process(
        COMMAND cmake --build "${BUILD_DIR}" --config Release
        RESULT_VARIABLE BUILD_RESULT
        OUTPUT_VARIABLE BUILD_OUTPUT
        ERROR_VARIABLE BUILD_ERROR
    )
    
    if(NOT BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Test ${TEST_NAME} build failed: ${BUILD_ERROR}")
    endif()
    
    message(STATUS "Test ${TEST_NAME} passed")
endfunction()

# Test basic configuration
run_build_test("basic"
    OPTIONS
        -DCMAKE_BUILD_TYPE=Release
        -DQTFORGE_BUILD_TESTS=OFF
        -DQTFORGE_BUILD_EXAMPLES=OFF
)

# Test debug configuration
run_build_test("debug"
    OPTIONS
        -DCMAKE_BUILD_TYPE=Debug
        -DQTFORGE_BUILD_TESTS=ON
        -DQTFORGE_BUILD_EXAMPLES=ON
        -DQTFORGE_ENABLE_COMPONENT_LOGGING=ON
)

# Test shared library build
run_build_test("shared"
    OPTIONS
        -DCMAKE_BUILD_TYPE=Release
        -DQTFORGE_BUILD_SHARED=ON
        -DQTFORGE_BUILD_STATIC=OFF
)

# Test static library build
run_build_test("static"
    OPTIONS
        -DCMAKE_BUILD_TYPE=Release
        -DQTFORGE_BUILD_SHARED=OFF
        -DQTFORGE_BUILD_STATIC=ON
)

# Test with all components enabled
run_build_test("all_components"
    OPTIONS
        -DCMAKE_BUILD_TYPE=Release
        -DQTFORGE_BUILD_NETWORK=ON
        -DQTFORGE_BUILD_UI=ON
        -DQTFORGE_BUILD_EXAMPLES=ON
        -DQTFORGE_BUILD_TESTS=ON
)

# Test compiler options
run_build_test("compiler_options"
    OPTIONS
        -DCMAKE_BUILD_TYPE=Release
        -DQTFORGE_ENABLE_WARNINGS=ON
        -DQTFORGE_ENABLE_LTO=ON
)

# Test packaging configuration
run_build_test("packaging"
    OPTIONS
        -DCMAKE_BUILD_TYPE=Release
        -DQTFORGE_CREATE_PACKAGES=ON
        -DQTFORGE_PACKAGE_COMPONENTS=ON
)

# Test preset configurations (if available)
if(EXISTS "${TEST_SOURCE_DIR}/CMakePresets.json")
    # Test debug preset
    run_build_test("preset_debug"
        PRESET debug
    )
    
    # Test release preset
    run_build_test("preset_release"
        PRESET release
    )
endif()

# Test invalid configuration (should fail)
run_build_test("invalid_config"
    SHOULD_FAIL
    OPTIONS
        -DCMAKE_BUILD_TYPE=Release
        -DQTFORGE_BUILD_SHARED=OFF
        -DQTFORGE_BUILD_STATIC=OFF
)

message(STATUS "All build system tests passed!")

# Function to test platform detection
function(test_platform_detection)
    message(STATUS "Testing platform detection...")
    
    # Create a minimal test project
    set(TEST_CMAKE_FILE "${TEST_BUILD_DIR}/platform_test/CMakeLists.txt")
    file(MAKE_DIRECTORY "${TEST_BUILD_DIR}/platform_test")
    
    file(WRITE "${TEST_CMAKE_FILE}" "
cmake_minimum_required(VERSION 3.21)
project(PlatformTest)

list(PREPEND CMAKE_MODULE_PATH \"${TEST_SOURCE_DIR}/cmake/modules\")
include(QtForgePlatform)

message(STATUS \"Platform: \${QTFORGE_PLATFORM_NAME}\")
message(STATUS \"Architecture: \${QTFORGE_ARCH_NAME}\")
message(STATUS \"Bits: \${QTFORGE_ARCH_BITS}\")
")
    
    execute_process(
        COMMAND cmake -S "${TEST_BUILD_DIR}/platform_test" -B "${TEST_BUILD_DIR}/platform_test/build"
        RESULT_VARIABLE RESULT
        OUTPUT_VARIABLE OUTPUT
        ERROR_VARIABLE ERROR
    )
    
    if(NOT RESULT EQUAL 0)
        message(FATAL_ERROR "Platform detection test failed: ${ERROR}")
    endif()
    
    message(STATUS "Platform detection test passed")
endfunction()

# Test platform detection
test_platform_detection()

# Function to test compiler detection
function(test_compiler_detection)
    message(STATUS "Testing compiler detection...")
    
    # Create a minimal test project
    set(TEST_CMAKE_FILE "${TEST_BUILD_DIR}/compiler_test/CMakeLists.txt")
    file(MAKE_DIRECTORY "${TEST_BUILD_DIR}/compiler_test")
    
    file(WRITE "${TEST_CMAKE_FILE}" "
cmake_minimum_required(VERSION 3.21)
project(CompilerTest)

list(PREPEND CMAKE_MODULE_PATH \"${TEST_SOURCE_DIR}/cmake/modules\")
include(QtForgeCompiler)

message(STATUS \"Compiler: \${QTFORGE_COMPILER_NAME}\")
message(STATUS \"Version: \${QTFORGE_COMPILER_VERSION}\")
")
    
    execute_process(
        COMMAND cmake -S "${TEST_BUILD_DIR}/compiler_test" -B "${TEST_BUILD_DIR}/compiler_test/build"
        RESULT_VARIABLE RESULT
        OUTPUT_VARIABLE OUTPUT
        ERROR_VARIABLE ERROR
    )
    
    if(NOT RESULT EQUAL 0)
        message(FATAL_ERROR "Compiler detection test failed: ${ERROR}")
    endif()
    
    message(STATUS "Compiler detection test passed")
endfunction()

# Test compiler detection
test_compiler_detection()

message(STATUS "All modular build system tests completed successfully!")
