# QtPlugin Library Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.21)
project(QtPluginTests VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 Test component - make Test optional due to MinGW issues
find_package(Qt6 REQUIRED COMPONENTS Core Network Sql Concurrent)
find_package(Qt6 COMPONENTS Test QUIET)

if(NOT Qt6Test_FOUND)
    message(WARNING "Qt6Test not found - some tests may be disabled")
endif()

# Enable automatic MOC
set(CMAKE_AUTOMOC ON)

# Test build options
option(QTPLUGIN_BUILD_COMPREHENSIVE_TESTS "Build comprehensive test suite" ON)
option(QTPLUGIN_BUILD_PERFORMANCE_TESTS "Build performance tests" ON)
option(QTPLUGIN_BUILD_MEMORY_TESTS "Build memory tests" ON)
option(QTPLUGIN_BUILD_CROSS_PLATFORM_TESTS "Build cross-platform tests" ON)

# Enable testing
enable_testing()

# Configuration Manager Tests (existing)
add_executable(test_configuration_manager
    test_configuration_manager.cpp
)

target_include_directories(test_configuration_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_configuration_manager PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

add_test(NAME ConfigurationManagerTests COMMAND test_configuration_manager)
set_tests_properties(ConfigurationManagerTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Comprehensive Plugin Interface Tests
if(QTPLUGIN_BUILD_COMPREHENSIVE_TESTS)
    add_executable(test_plugin_interface_comprehensive
        test_plugin_interface_comprehensive.cpp
    )

    target_include_directories(test_plugin_interface_comprehensive PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_plugin_interface_comprehensive PRIVATE
        Qt6::Test
        Qt6::Core
        QtPluginCore
    )

    add_test(NAME PluginInterfaceComprehensiveTests COMMAND test_plugin_interface_comprehensive)
    set_tests_properties(PluginInterfaceComprehensiveTests PROPERTIES
        TIMEOUT 120
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Comprehensive Plugin Manager Tests
if(QTPLUGIN_BUILD_COMPREHENSIVE_TESTS)
    add_executable(test_plugin_manager_comprehensive
        test_plugin_manager_comprehensive.cpp
    )

    target_include_directories(test_plugin_manager_comprehensive PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_plugin_manager_comprehensive PRIVATE
        Qt6::Test
        Qt6::Core
        QtPluginCore
    )

    add_test(NAME PluginManagerComprehensiveTests COMMAND test_plugin_manager_comprehensive)
    set_tests_properties(PluginManagerComprehensiveTests PROPERTIES
        TIMEOUT 180
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Expected<T,E> Comprehensive Tests
if(QTPLUGIN_BUILD_COMPREHENSIVE_TESTS)
    add_executable(test_expected_comprehensive
        test_expected_comprehensive.cpp
    )

    target_include_directories(test_expected_comprehensive PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_expected_comprehensive PRIVATE
        Qt6::Test
        Qt6::Core
        QtPluginCore
    )

    add_test(NAME ExpectedComprehensiveTests COMMAND test_expected_comprehensive)
    set_tests_properties(ExpectedComprehensiveTests PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Cross-Platform Tests
if(QTPLUGIN_BUILD_CROSS_PLATFORM_TESTS)
    add_executable(test_cross_platform
        test_cross_platform.cpp
    )

    target_include_directories(test_cross_platform PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_cross_platform PRIVATE
        Qt6::Test
        Qt6::Core
        QtPluginCore
    )

    add_test(NAME CrossPlatformTests COMMAND test_cross_platform)
    set_tests_properties(CrossPlatformTests PROPERTIES
        TIMEOUT 300
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Performance Tests
if(QTPLUGIN_BUILD_PERFORMANCE_TESTS)
    add_executable(test_performance
        test_performance.cpp
    )

    target_include_directories(test_performance PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/include
    )

    target_link_libraries(test_performance PRIVATE
        Qt6::Test
        Qt6::Core
        QtPluginCore
    )

    add_test(NAME PerformanceTests COMMAND test_performance)
    set_tests_properties(PerformanceTests PROPERTIES
        TIMEOUT 600
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Memory leak detection tests (if Valgrind is available)
if(QTPLUGIN_BUILD_MEMORY_TESTS)
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
add_custom_target(run_basic_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "ConfigurationManagerTests" --verbose
    DEPENDS test_configuration_manager
    COMMENT "Running basic QtPlugin tests"
)

if(QTPLUGIN_BUILD_COMPREHENSIVE_TESTS)
    add_custom_target(run_comprehensive_tests
        COMMAND ${CMAKE_CTEST_COMMAND} -R "Comprehensive" --verbose
        DEPENDS
            test_plugin_interface_comprehensive
            test_plugin_manager_comprehensive
            test_expected_comprehensive
        COMMENT "Running comprehensive QtPlugin tests"
    )
endif()

if(QTPLUGIN_BUILD_CROSS_PLATFORM_TESTS)
    add_custom_target(run_cross_platform_tests
        COMMAND ${CMAKE_CTEST_COMMAND} -R "CrossPlatform" --verbose
        DEPENDS test_cross_platform
        COMMENT "Running cross-platform QtPlugin tests"
    )
endif()

if(QTPLUGIN_BUILD_PERFORMANCE_TESTS)
    add_custom_target(run_performance_tests
        COMMAND ${CMAKE_CTEST_COMMAND} -R "Performance" --verbose
        DEPENDS
            test_performance
            test_component_performance
        COMMENT "Running performance QtPlugin tests"
    )
endif()

# Component-specific test targets
add_custom_target(run_component_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "Component" --verbose
    DEPENDS
        test_component_architecture
        test_manager_component_integration
        test_component_performance
    COMMENT "Running QtPlugin component tests"
)

# Run all tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS
        test_configuration_manager
        test_component_architecture
        test_manager_component_integration
        $<$<BOOL:${QTPLUGIN_BUILD_COMPREHENSIVE_TESTS}>:test_plugin_interface_comprehensive>
        $<$<BOOL:${QTPLUGIN_BUILD_COMPREHENSIVE_TESTS}>:test_plugin_manager_comprehensive>
        $<$<BOOL:${QTPLUGIN_BUILD_COMPREHENSIVE_TESTS}>:test_expected_comprehensive>
        $<$<BOOL:${QTPLUGIN_BUILD_CROSS_PLATFORM_TESTS}>:test_cross_platform>
        $<$<BOOL:${QTPLUGIN_BUILD_PERFORMANCE_TESTS}>:test_performance>
    COMMENT "Running all QtPlugin tests"
)

# Install test executables (optional)
install(TARGETS test_configuration_manager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)



# Resource Management Tests
add_executable(test_resource_management
    test_resource_management.cpp
)

target_include_directories(test_resource_management PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_resource_management PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    QtPluginCore
)

# Add test to CTest
add_test(NAME ResourceManagementTests COMMAND test_resource_management)

# Set test properties
set_tests_properties(ResourceManagementTests PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Install test executable (optional)
install(TARGETS test_resource_management
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Component Architecture Tests
add_executable(test_component_architecture
    test_component_architecture.cpp
)

target_include_directories(test_component_architecture PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/include
)

target_link_libraries(test_component_architecture PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Concurrent
    QtPluginCore
)

# Add test to CTest
add_test(NAME ComponentArchitectureTests COMMAND test_component_architecture)

# Set test properties
set_tests_properties(ComponentArchitectureTests PROPERTIES
    TIMEOUT 180
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Plugin Lifecycle Manager Tests
add_executable(test_lifecycle_manager
    test_lifecycle_manager.cpp
)

target_include_directories(test_lifecycle_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_lifecycle_manager PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

# Add test to CTest
add_test(NAME LifecycleManagerTests COMMAND test_lifecycle_manager)

# Set test properties
set_tests_properties(LifecycleManagerTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Simple Lifecycle Manager Tests
add_executable(test_lifecycle_simple
    test_lifecycle_simple.cpp
)

target_include_directories(test_lifecycle_simple PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_lifecycle_simple PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

# Add test to CTest
add_test(NAME LifecycleSimpleTests COMMAND test_lifecycle_simple)

# Set test properties
set_tests_properties(LifecycleSimpleTests PROPERTIES
    TIMEOUT 30
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Lifecycle Manager Compilation Tests
add_executable(test_lifecycle_compilation
    test_lifecycle_compilation.cpp
)

target_include_directories(test_lifecycle_compilation PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_lifecycle_compilation PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

# Add test to CTest
add_test(NAME LifecycleCompilationTests COMMAND test_lifecycle_compilation)

# Set test properties
set_tests_properties(LifecycleCompilationTests PROPERTIES
    TIMEOUT 30
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Manager-Component Integration Tests
add_executable(test_manager_component_integration
    test_manager_component_integration.cpp
)

target_include_directories(test_manager_component_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/include
)

target_link_libraries(test_manager_component_integration PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Concurrent
    QtPluginCore
)

# Add test to CTest
add_test(NAME ManagerComponentIntegrationTests COMMAND test_manager_component_integration)

# Set test properties
set_tests_properties(ManagerComponentIntegrationTests PROPERTIES
    TIMEOUT 240
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Component Performance Tests
add_executable(test_component_performance
    test_component_performance.cpp
)

target_include_directories(test_component_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/include
)

target_link_libraries(test_component_performance PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Concurrent
    QtPluginCore
)

# Add test to CTest
add_test(NAME ComponentPerformanceTests COMMAND test_component_performance)

# Set test properties
set_tests_properties(ComponentPerformanceTests PROPERTIES
    TIMEOUT 600
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Plugin Manager Tests
add_executable(test_plugin_manager
    test_plugin_manager.cpp
)

target_include_directories(test_plugin_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_plugin_manager PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

add_test(NAME PluginManagerTests COMMAND test_plugin_manager)
set_tests_properties(PluginManagerTests PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_plugin_manager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Security Manager Tests
add_executable(test_security_manager
    test_security_manager_simple.cpp
)

target_include_directories(test_security_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_security_manager PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
    QtPluginSecurity
)

add_test(NAME SecurityManagerTests COMMAND test_security_manager)
set_tests_properties(SecurityManagerTests PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_security_manager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Message Bus Tests
add_executable(test_message_bus
    test_message_bus_simple.cpp
)

target_include_directories(test_message_bus PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_message_bus PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

add_test(NAME MessageBusTests COMMAND test_message_bus)
set_tests_properties(MessageBusTests PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_message_bus
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Version Tests
add_executable(test_version
    test_version_simple.cpp
)

target_include_directories(test_version PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_version PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

add_test(NAME VersionTests COMMAND test_version)
set_tests_properties(VersionTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_version
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Error Handling Tests
add_executable(test_error_handling
    test_error_handling_simple.cpp
)

target_include_directories(test_error_handling PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_error_handling PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

add_test(NAME ErrorHandlingTests COMMAND test_error_handling)
set_tests_properties(ErrorHandlingTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_error_handling
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Plugin Interface Tests
add_executable(test_plugin_interface
    test_plugin_interface.cpp
)

target_include_directories(test_plugin_interface PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_plugin_interface PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

add_test(NAME PluginInterfaceTests COMMAND test_plugin_interface)
set_tests_properties(PluginInterfaceTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_plugin_interface
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Enhanced Plugin System Tests (v3.1.0)
if(QTPLUGIN_BUILD_COMPREHENSIVE_TESTS)
    # Service Contracts Tests
    add_executable(test_service_contracts
        test_service_contracts.cpp
    )

    target_include_directories(test_service_contracts PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_service_contracts PRIVATE
        Qt6::Test
        Qt6::Core
        QtPluginCore
    )

    add_test(NAME ServiceContractsTests COMMAND test_service_contracts)
    set_tests_properties(ServiceContractsTests PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Plugin Orchestration Tests
    add_executable(test_plugin_orchestration
        test_plugin_orchestration.cpp
    )

    target_include_directories(test_plugin_orchestration PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_plugin_orchestration PRIVATE
        Qt6::Test
        Qt6::Core
        QtPluginCore
    )

    add_test(NAME PluginOrchestrationTests COMMAND test_plugin_orchestration)
    set_tests_properties(PluginOrchestrationTests PROPERTIES
        TIMEOUT 120
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Plugin Version Manager Tests (v3.1.0)
    add_executable(test_plugin_version_manager
        test_plugin_version_manager.cpp
    )

    target_include_directories(test_plugin_version_manager PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_plugin_version_manager PRIVATE
        Qt6::Test
        Qt6::Core
        QtPluginCore
    )

    add_test(NAME PluginVersionManagerTests COMMAND test_plugin_version_manager)
    set_tests_properties(PluginVersionManagerTests PROPERTIES
        TIMEOUT 120
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    install(TARGETS test_service_contracts test_plugin_orchestration test_plugin_version_manager
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
    )
endif()

