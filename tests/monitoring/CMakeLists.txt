# QtForge Monitoring Module Tests
cmake_minimum_required(VERSION 3.21)

# Performance Tests
if(QTFORGE_BUILD_PERFORMANCE_TESTS)
    add_executable(test_performance
        test_performance.cpp
    )

    target_include_directories(test_performance PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_performance PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME MonitoringPerformanceTests
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
        -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_performance.exe
        -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(MonitoringPerformanceTests PROPERTIES
        TIMEOUT 600
    )
endif()

# Component Performance Tests
if(QTFORGE_BUILD_PERFORMANCE_TESTS)
    add_executable(test_component_performance
        test_component_performance.cpp
    )

    target_include_directories(test_component_performance PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_component_performance PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Concurrent
        QtForgeCore
    )

    add_test(NAME MonitoringComponentPerformanceTests
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
        -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_component_performance.exe
        -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(MonitoringComponentPerformanceTests PROPERTIES
        TIMEOUT 600
    )
endif()

# Install test executables
if(QTFORGE_BUILD_PERFORMANCE_TESTS)
    install(TARGETS
        test_performance
        test_component_performance
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/monitoring
    )
endif()
