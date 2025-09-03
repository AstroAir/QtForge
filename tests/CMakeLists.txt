# QtForge Library Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.21)
project(QtForgeTests VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use Qt6 components already found by the main CMakeLists.txt
# Only find Test component if not already found
if(NOT TARGET Qt6::Test)
    find_package(Qt6 COMPONENTS Test QUIET)
    if(NOT Qt6Test_FOUND)
        message(WARNING "Qt6Test not found - some tests may be disabled")
    endif()
endif()

# Enable automatic MOC
set(CMAKE_AUTOMOC ON)

# Test build options
option(QTFORGE_BUILD_COMPREHENSIVE_TESTS "Build comprehensive test suite" ON)
option(QTFORGE_BUILD_PERFORMANCE_TESTS "Build performance tests" ON)
option(QTFORGE_BUILD_MEMORY_TESTS "Build memory tests" ON)
option(QTFORGE_BUILD_CROSS_PLATFORM_TESTS "Build cross-platform tests" ON)

# Enable testing
enable_testing()

# Function to configure test environment
function(configure_test_environment test_name)
    # Set PATH to include build directory and Qt bin directory for DLL loading on Windows
    if(WIN32)
        # Get Qt bin directory
        get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(QT_BIN_DIR ${QT_QMAKE_EXECUTABLE} DIRECTORY)

        set_tests_properties(${test_name} PROPERTIES
            ENVIRONMENT "PATH=${CMAKE_BINARY_DIR};${QT_BIN_DIR};$ENV{PATH}"
        )
    endif()
endfunction()

# Add custom target to copy DLLs to test directories for Windows
if(WIN32)
    # Find Qt installation directory
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_WINDEPLOYQT_EXECUTABLE ${QT_QMAKE_EXECUTABLE} PATH)
    set(QT_WINDEPLOYQT_EXECUTABLE "${QT_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

    # Define test directories
    set(TEST_DIRS core communication security managers monitoring orchestration utils platform python)

    add_custom_target(copy_dlls_to_tests ALL
        COMMENT "Copying DLLs to test directories"
    )

    # Copy QtForge DLLs to each test directory
    foreach(TEST_DIR ${TEST_DIRS})
        add_custom_command(TARGET copy_dlls_to_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:QtForgeCore> ${CMAKE_CURRENT_BINARY_DIR}/${TEST_DIR}/
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:QtForgeSecurity> ${CMAKE_CURRENT_BINARY_DIR}/${TEST_DIR}/
            COMMENT "Copying QtForge DLLs to ${TEST_DIR}"
        )
    endforeach()

    add_dependencies(copy_dlls_to_tests QtForgeCore QtForgeSecurity)
endif()

# Include modular test directories
add_subdirectory(core)
add_subdirectory(communication)
add_subdirectory(security)
add_subdirectory(managers)
add_subdirectory(monitoring)
add_subdirectory(orchestration)
add_subdirectory(utils)
add_subdirectory(platform)
add_subdirectory(integration)
# add_subdirectory(build_system) # Temporarily disabled - causes configuration hang
add_subdirectory(bridges)
add_subdirectory(composition)
add_subdirectory(marketplace)
add_subdirectory(transactions)
add_subdirectory(python)

# Lua tests (conditional)
if(QTFORGE_LUA_BINDINGS)
    add_subdirectory(lua)
endif()

# Memory leak detection tests (if Valgrind is available)
if(QTFORGE_BUILD_MEMORY_TESTS)
    find_program(VALGRIND_PROGRAM valgrind)
    if(VALGRIND_PROGRAM AND NOT WIN32)
        add_test(NAME MemoryLeakTests
                 COMMAND ${VALGRIND_PROGRAM}
                         --tool=memcheck
                         --leak-check=full
                         --error-exitcode=1
                         --suppressions=${CMAKE_CURRENT_SOURCE_DIR}/valgrind.supp
                         $<TARGET_FILE:test_configuration_manager>)
        set_tests_properties(MemoryLeakTests PROPERTIES
            TIMEOUT 600
        )
    endif()
endif()

# Custom test targets
add_custom_target(run_core_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "Core" --verbose
    COMMENT "Running QtForge core tests"
)

add_custom_target(run_communication_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "Communication" --verbose
    COMMENT "Running QtForge communication tests"
)

add_custom_target(run_security_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "Security" --verbose
    COMMENT "Running QtForge security tests"
)

add_custom_target(run_managers_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "Managers" --verbose
    COMMENT "Running QtForge managers tests"
)

add_custom_target(run_monitoring_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "Monitoring" --verbose
    COMMENT "Running QtForge monitoring tests"
)

add_custom_target(run_orchestration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "Orchestration" --verbose
    COMMENT "Running QtForge orchestration tests"
)

add_custom_target(run_utils_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "Utils" --verbose
    COMMENT "Running QtForge utils tests"
)

add_custom_target(run_platform_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "Platform" --verbose
    COMMENT "Running QtForge platform tests"
)

add_custom_target(run_integration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "Integration" --verbose
    COMMENT "Running QtForge integration tests"
)

# Lua tests (if enabled)
if(QTFORGE_LUA_BINDINGS)
    add_custom_target(run_lua_tests
        COMMAND ${CMAKE_CTEST_COMMAND} -L "Lua" --verbose
        COMMENT "Running QtForge Lua tests"
    )
endif()

# Run all tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    COMMENT "Running all QtForge tests"
)

if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
    add_custom_target(run_comprehensive_tests
        COMMAND ${CMAKE_CTEST_COMMAND} -R "Comprehensive" --verbose
        COMMENT "Running comprehensive QtForge tests"
    )
endif()

if(QTFORGE_BUILD_PERFORMANCE_TESTS)
    add_custom_target(run_performance_tests
        COMMAND ${CMAKE_CTEST_COMMAND} -R "Performance" --verbose
        COMMENT "Running performance QtForge tests"
    )
endif()
