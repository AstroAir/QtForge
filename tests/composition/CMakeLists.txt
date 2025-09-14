# QtForge Composition Module Tests
cmake_minimum_required(VERSION 3.21)

# Plugin Composition Tests
add_executable(test_plugin_composition
    test_plugin_composition.cpp
)

target_include_directories(test_plugin_composition PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(test_plugin_composition PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
)

add_test(NAME CompositionPluginTests
    COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_plugin_composition.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(CompositionPluginTests PROPERTIES
    TIMEOUT 120
    LABELS "composition;plugin_composition"
)

# Install test executables
install(TARGETS
    test_plugin_composition
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/composition
)
