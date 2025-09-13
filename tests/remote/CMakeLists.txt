# CMakeLists.txt for Remote Plugin Tests
cmake_minimum_required(VERSION 3.16)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Network)
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
)

# Test source files
set(REMOTE_PLUGIN_TEST_SOURCES
    test_remote_plugin_source.cpp
    test_plugin_download_manager.cpp
    test_http_plugin_loader.cpp
    test_remote_plugin_integration.cpp
    test_remote_plugin_security.cpp
    test_remote_plugin_registry.cpp
    test_remote_plugin_discovery.cpp
    test_remote_plugin_configuration.cpp
    test_remote_plugin_validator.cpp
    test_remote_plugin_marketplace_integration.cpp
    test_remote_plugin_performance.cpp
)

# Create test executable
add_executable(remote_plugin_tests
    ${REMOTE_PLUGIN_TEST_SOURCES}
    main_test_runner.cpp
)

# Link libraries
target_link_libraries(remote_plugin_tests
    Qt6::Core
    Qt6::Network
    qtplugin_remote
    qtplugin_core
    qtplugin_utils
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    pthread
)

# Compiler flags
target_compile_features(remote_plugin_tests PRIVATE cxx_std_17)
target_compile_definitions(remote_plugin_tests PRIVATE
    QT_NO_KEYWORDS
    GTEST_HAS_PTHREAD=1
)

# Test discovery
include(GoogleTest)
gtest_discover_tests(remote_plugin_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Individual test targets for specific components
add_executable(test_remote_plugin_source
    test_remote_plugin_source.cpp
    test_main.cpp
)
target_link_libraries(test_remote_plugin_source
    Qt6::Core
    qtplugin_remote
    ${GTEST_LIBRARIES}
    pthread
)

add_executable(test_plugin_download_manager
    test_plugin_download_manager.cpp
    test_main.cpp
)
target_link_libraries(test_plugin_download_manager
    Qt6::Core
    Qt6::Network
    qtplugin_remote
    ${GTEST_LIBRARIES}
    pthread
)

add_executable(test_http_plugin_loader
    test_http_plugin_loader.cpp
    test_main.cpp
)
target_link_libraries(test_http_plugin_loader
    Qt6::Core
    Qt6::Network
    qtplugin_remote
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    pthread
)

add_executable(test_remote_plugin_integration
    test_remote_plugin_integration.cpp
    test_main.cpp
)
target_link_libraries(test_remote_plugin_integration
    Qt6::Core
    Qt6::Network
    qtplugin_remote
    qtplugin_core
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    pthread
)

add_executable(test_remote_plugin_security
    test_remote_plugin_security.cpp
    test_main.cpp
)
target_link_libraries(test_remote_plugin_security
    Qt6::Core
    qtplugin_remote
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    pthread
)

# Test data directory
file(COPY test_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Custom test targets
add_custom_target(test_remote_plugins
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS remote_plugin_tests
    COMMENT "Running remote plugin tests"
)

add_custom_target(test_remote_plugins_verbose
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS remote_plugin_tests
    COMMENT "Running remote plugin tests with verbose output"
)

add_custom_target(test_remote_plugins_parallel
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel 4
    DEPENDS remote_plugin_tests
    COMMENT "Running remote plugin tests in parallel"
)

# Coverage target (if coverage tools are available)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
        target_compile_options(remote_plugin_tests PRIVATE --coverage)
        target_link_options(remote_plugin_tests PRIVATE --coverage)

        add_custom_target(coverage_remote_plugins
            COMMAND ${LCOV_PATH} --directory . --zerocounters
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/tests/*' '*/build/*' --output-file coverage.info.cleaned
            COMMAND ${GENHTML_PATH} -o coverage_html coverage.info.cleaned
            DEPENDS remote_plugin_tests
            COMMENT "Generating coverage report for remote plugin tests"
        )
    endif()
endif()

# Valgrind target (if valgrind is available)
find_program(VALGRIND_PATH valgrind)
if(VALGRIND_PATH)
    add_custom_target(test_remote_plugins_valgrind
        COMMAND ${VALGRIND_PATH} --tool=memcheck --leak-check=full --show-leak-kinds=all
                --track-origins=yes --verbose --error-exitcode=1
                $<TARGET_FILE:remote_plugin_tests>
        DEPENDS remote_plugin_tests
        COMMENT "Running remote plugin tests with Valgrind"
    )
endif()

# Benchmark target (if Google Benchmark is available)
find_package(benchmark QUIET)
if(benchmark_FOUND)
    add_executable(remote_plugin_benchmarks
        benchmarks/benchmark_plugin_loading.cpp
        benchmarks/benchmark_plugin_discovery.cpp
        benchmarks/benchmark_download_manager.cpp
    )

    target_link_libraries(remote_plugin_benchmarks
        Qt6::Core
        Qt6::Network
        qtplugin_remote
        benchmark::benchmark
        pthread
    )

    add_custom_target(benchmark_remote_plugins
        COMMAND $<TARGET_FILE:remote_plugin_benchmarks>
        DEPENDS remote_plugin_benchmarks
        COMMENT "Running remote plugin benchmarks"
    )
endif()

# Install test executables (optional, for CI/CD)
if(INSTALL_TESTS)
    install(TARGETS remote_plugin_tests
        RUNTIME DESTINATION bin/tests
    )

    install(DIRECTORY test_data
        DESTINATION share/qtforge/tests
    )
endif()

# Test configuration
set_tests_properties(remote_plugin_tests PROPERTIES
    TIMEOUT 300  # 5 minutes timeout
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=false"
)

# Add labels for test organization
set_tests_properties(remote_plugin_tests PROPERTIES
    LABELS "remote;plugins;integration"
)

# Test fixtures for setup/teardown
add_test(NAME setup_remote_plugin_tests
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/test_temp
)
set_tests_properties(setup_remote_plugin_tests PROPERTIES
    FIXTURES_SETUP remote_plugin_fixture
)

add_test(NAME cleanup_remote_plugin_tests
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/test_temp
)
set_tests_properties(cleanup_remote_plugin_tests PROPERTIES
    FIXTURES_CLEANUP remote_plugin_fixture
)

set_tests_properties(remote_plugin_tests PROPERTIES
    FIXTURES_REQUIRED remote_plugin_fixture
)
