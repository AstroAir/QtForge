# QtForge Unit Tests
cmake_minimum_required(VERSION 3.21)

# Plugin Loader Unit Tests
add_executable(test_plugin_loader_unit
    test_plugin_loader.cpp
)

target_include_directories(test_plugin_loader_unit PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_plugin_loader_unit PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
)

add_test(NAME UnitPluginLoaderTests
    COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_plugin_loader_unit.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(UnitPluginLoaderTests PROPERTIES
    TIMEOUT 60
    LABELS "unit"
)

# Plugin Manager Unit Tests
add_executable(test_plugin_manager_unit
    test_plugin_manager.cpp
)

target_include_directories(test_plugin_manager_unit PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_plugin_manager_unit PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
)

add_test(NAME UnitPluginManagerTests
    COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_plugin_manager_unit.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(UnitPluginManagerTests PROPERTIES
    TIMEOUT 60
    LABELS "unit"
)

# Install test executables
install(TARGETS
    test_plugin_loader_unit
    test_plugin_manager_unit
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/unit
)
