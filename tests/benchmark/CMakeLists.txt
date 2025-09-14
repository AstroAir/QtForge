# QtForge Benchmark Tests
cmake_minimum_required(VERSION 3.21)

# Plugin System Benchmark Tests
add_executable(test_plugin_system_benchmark
    plugin_system_benchmark.cpp
)

target_include_directories(test_plugin_system_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_plugin_system_benchmark PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
)

# Benchmark tests are optional and only run when explicitly requested
if(QTFORGE_BUILD_BENCHMARK_TESTS)
    add_test(NAME BenchmarkPluginSystemTests
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
        -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_plugin_system_benchmark.exe
        -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(BenchmarkPluginSystemTests PROPERTIES
        TIMEOUT 300
        LABELS "benchmark"
    )
endif()

# Install test executables
install(TARGETS
    test_plugin_system_benchmark
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/benchmark
)
