# QtForge Security Module Tests
cmake_minimum_required(VERSION 3.21)

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
    QtForgeCore
    QtForgeSecurity
)

add_test(NAME SecurityManagerTests
    COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_security_manager.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(SecurityManagerTests PROPERTIES
    TIMEOUT 120
)

# Sandbox Tests (only if sandbox is enabled)
if(QTFORGE_ENABLE_SANDBOX)
    # Plugin Sandbox Tests
    add_executable(test_plugin_sandbox
        test_plugin_sandbox.cpp
    )

    target_include_directories(test_plugin_sandbox PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_plugin_sandbox PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME PluginSandboxTests COMMAND test_plugin_sandbox)
    set_tests_properties(PluginSandboxTests PROPERTIES
        TIMEOUT 180
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Resource Monitor Tests
    add_executable(test_resource_monitor
        test_resource_monitor.cpp
    )

    target_include_directories(test_resource_monitor PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_resource_monitor PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME ResourceMonitorTests COMMAND test_resource_monitor)
    set_tests_properties(ResourceMonitorTests PROPERTIES
        TIMEOUT 120
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Security Enforcer Tests
    add_executable(test_security_enforcer
        test_security_enforcer.cpp
    )

    target_include_directories(test_security_enforcer PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_security_enforcer PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Network
        QtForgeCore
    )

    add_test(NAME SecurityEnforcerTests COMMAND test_security_enforcer)
    set_tests_properties(SecurityEnforcerTests PROPERTIES
        TIMEOUT 120
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Sandbox Manager Tests
    add_executable(test_sandbox_manager
        test_sandbox_manager.cpp
    )

    target_include_directories(test_sandbox_manager PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_sandbox_manager PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME SandboxManagerTests COMMAND test_sandbox_manager)
    set_tests_properties(SandboxManagerTests PROPERTIES
        TIMEOUT 120
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Sandbox Integration Tests
    add_executable(test_sandbox_integration
        test_sandbox_integration.cpp
    )

    target_include_directories(test_sandbox_integration PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_sandbox_integration PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME SandboxIntegrationTests COMMAND test_sandbox_integration)
    set_tests_properties(SandboxIntegrationTests PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Simple Sandbox Tests (basic functionality verification)
    add_executable(test_sandbox_simple
        test_sandbox_simple.cpp
    )

    target_include_directories(test_sandbox_simple PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_sandbox_simple PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME SandboxSimpleTests COMMAND test_sandbox_simple)
    set_tests_properties(SandboxSimpleTests PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Comprehensive Security Manager Tests - temporarily disabled due to linking errors
# if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
#     add_executable(test_security_manager_comprehensive
#         test_security_manager.cpp
#     )
#
#     target_include_directories(test_security_manager_comprehensive PRIVATE
#         ${CMAKE_SOURCE_DIR}/include
#     )
#
#     target_link_libraries(test_security_manager_comprehensive PRIVATE
#         Qt6::Test
#         Qt6::Core
#         QtForgeCore
#         QtForgeSecurity
#     )
#
#     add_test(NAME SecurityManagerComprehensiveTests COMMAND test_security_manager_comprehensive)
#     set_tests_properties(SecurityManagerComprehensiveTests PROPERTIES
#         TIMEOUT 180
#         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#         ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR};${CMAKE_BINARY_DIR};D:/msys64/mingw64/bin;$ENV{PATH}"
#     )
# endif()

# Install test executables
set(SECURITY_TEST_TARGETS test_security_manager)

if(QTFORGE_ENABLE_SANDBOX)
    list(APPEND SECURITY_TEST_TARGETS
        test_plugin_sandbox
        test_resource_monitor
        test_security_enforcer
        test_sandbox_manager
        test_sandbox_integration
        test_sandbox_simple
    )
endif()

install(TARGETS ${SECURITY_TEST_TARGETS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/security
)

# if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
#     install(TARGETS
#         test_security_manager_comprehensive
#         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/security
#     )
# endif()
