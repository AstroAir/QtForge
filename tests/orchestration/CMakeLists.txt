# QtForge Orchestration Module Tests
cmake_minimum_required(VERSION 3.21)

# Plugin Orchestration Tests
if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
    add_executable(test_plugin_orchestration
        test_plugin_orchestration.cpp
    )

    target_include_directories(test_plugin_orchestration PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_plugin_orchestration PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME OrchestrationPluginTests COMMAND test_plugin_orchestration)
    set_tests_properties(OrchestrationPluginTests PROPERTIES
        TIMEOUT 120
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Install test executables
    install(TARGETS
        test_plugin_orchestration
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/orchestration
    )
endif()
