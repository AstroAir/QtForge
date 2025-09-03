# QtForge Managers Module Tests
cmake_minimum_required(VERSION 3.21)

# Configuration Manager Tests
add_executable(test_configuration_manager
    test_configuration_manager.cpp
)

target_include_directories(test_configuration_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_configuration_manager PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
)

add_test(NAME ManagersConfigurationTests
    COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_configuration_manager.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(ManagersConfigurationTests PROPERTIES
    TIMEOUT 60
)

# Plugin Version Manager Tests
if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
    add_executable(test_plugin_version_manager
        test_plugin_version_manager.cpp
    )

    target_include_directories(test_plugin_version_manager PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_plugin_version_manager PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME ManagersPluginVersionTests
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
        -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_plugin_version_manager.exe
        -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(ManagersPluginVersionTests PROPERTIES
        TIMEOUT 120
    )
endif()

# Resource Management Tests - temporarily disabled due to MinGW compiler issue
# add_executable(test_resource_management
#     test_resource_management.cpp
# )
#
# target_include_directories(test_resource_management PRIVATE
#     ${CMAKE_SOURCE_DIR}/include
# )
#
# target_link_libraries(test_resource_management PRIVATE
#     Qt6::Test
#     Qt6::Core
#     Qt6::Network
#     Qt6::Sql
#     QtForgeCore
# )
#
# add_test(NAME ManagersResourceTests COMMAND test_resource_management)
# set_tests_properties(ManagersResourceTests PROPERTIES
#     TIMEOUT 120
#     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#     ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR};${CMAKE_BINARY_DIR};D:/msys64/mingw64/bin;$ENV{PATH}"
# )

# Manager-Component Integration Tests - temporarily disabled due to compiler warnings
# add_executable(test_manager_component_integration
#     test_manager_component_integration.cpp
# )
#
# target_include_directories(test_manager_component_integration PRIVATE
#     ${CMAKE_SOURCE_DIR}/include
# )
#
# target_link_libraries(test_manager_component_integration PRIVATE
#     Qt6::Test
#     Qt6::Core
#     Qt6::Concurrent
#     QtForgeCore
# )
#
# add_test(NAME ManagersComponentIntegrationTests COMMAND test_manager_component_integration)
# set_tests_properties(ManagersComponentIntegrationTests PROPERTIES
#     TIMEOUT 240
#     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#     ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR};${CMAKE_BINARY_DIR};D:/msys64/mingw64/bin;$ENV{PATH}"
# )

# Install test executables
install(TARGETS
    test_configuration_manager
    # test_resource_management  # disabled
    # test_manager_component_integration  # disabled
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/managers
)

if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
    install(TARGETS
        test_plugin_version_manager
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/managers
    )
endif()
