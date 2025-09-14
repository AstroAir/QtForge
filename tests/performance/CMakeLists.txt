# QtForge Performance Tests
cmake_minimum_required(VERSION 3.21)

# Performance tests are primarily Python and Lua scripts
# We create a custom target to run them

# Custom target to run Python performance tests
add_custom_target(run_python_performance_tests
    COMMAND ${CMAKE_COMMAND} -E echo "Running Python performance tests..."
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/run_performance_tests.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Python performance tests"
)

# Custom target to run Lua performance tests (if Lua bindings are enabled)
if(QTFORGE_LUA_BINDINGS)
    add_custom_target(run_lua_performance_tests
        COMMAND ${CMAKE_COMMAND} -E echo "Running Lua performance tests..."
        COMMAND lua ${CMAKE_CURRENT_SOURCE_DIR}/test_memory_management.lua
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running Lua performance tests"
    )
endif()

# Performance tests are optional and only run when explicitly requested
if(QTFORGE_BUILD_PERFORMANCE_TESTS)
    # Add Python performance tests
    add_test(NAME PerformancePythonMemoryTests
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/test_memory_management.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(PerformancePythonMemoryTests PROPERTIES
        TIMEOUT 600
        LABELS "performance;python"
    )

    # Add Lua performance tests if available
    if(QTFORGE_LUA_BINDINGS)
        add_test(NAME PerformanceLuaMemoryTests
            COMMAND lua ${CMAKE_CURRENT_SOURCE_DIR}/test_memory_management.lua
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        set_tests_properties(PerformanceLuaMemoryTests PROPERTIES
            TIMEOUT 600
            LABELS "performance;lua"
        )
    endif()

    # Add performance test runner
    add_test(NAME PerformanceTestRunner
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/run_performance_tests.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(PerformanceTestRunner PROPERTIES
        TIMEOUT 900
        LABELS "performance;comprehensive"
    )
endif()
