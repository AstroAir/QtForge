# QtForge Bridges Module Tests
cmake_minimum_required(VERSION 3.21)

# Lua Plugin Bridge Tests
if(QTFORGE_LUA_BINDINGS)
    add_executable(test_lua_plugin_bridge
        test_lua_plugin_bridge.cpp
    )

    target_include_directories(test_lua_plugin_bridge PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_link_libraries(test_lua_plugin_bridge PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME BridgesLuaPluginTests
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
        -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_lua_plugin_bridge.exe
        -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(BridgesLuaPluginTests PROPERTIES
        TIMEOUT 120
        LABELS "bridges;lua;plugin"
    )
endif()

# Python Bridge Tests
if(QTFORGE_PYTHON_BINDINGS)
    add_executable(test_python_bridge
        test_python_bridge.cpp
    )

    target_include_directories(test_python_bridge PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_link_libraries(test_python_bridge PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME BridgesPythonTests
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
        -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_python_bridge.exe
        -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(BridgesPythonTests PROPERTIES
        TIMEOUT 120
        LABELS "bridges;python;plugin"
        ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR};${CMAKE_BINARY_DIR};$ENV{PATH}"
    )
endif()

# Install test executables
set(BRIDGE_TEST_TARGETS "")

if(QTFORGE_LUA_BINDINGS)
    list(APPEND BRIDGE_TEST_TARGETS test_lua_plugin_bridge)
endif()

if(QTFORGE_PYTHON_BINDINGS)
    list(APPEND BRIDGE_TEST_TARGETS test_python_bridge)
endif()

if(BRIDGE_TEST_TARGETS)
    install(TARGETS ${BRIDGE_TEST_TARGETS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/bridges
    )
endif()
