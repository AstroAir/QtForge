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
add_subdirectory(build_system)
add_subdirectory(bridges)
add_subdirectory(composition)
add_subdirectory(marketplace)
add_subdirectory(transactions)
add_subdirectory(python)

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
