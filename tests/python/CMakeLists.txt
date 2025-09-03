# QtForge Python Module Tests
cmake_minimum_required(VERSION 3.21)

# Python tests require Python development headers, pybind11, and Python bindings to be enabled
find_package(Python3 COMPONENTS Interpreter Development QUIET)
find_package(pybind11 QUIET)

if(QTFORGE_BUILD_PYTHON_BINDINGS AND Python3_FOUND AND pybind11_FOUND)
    # Python Binding Tests
    add_executable(test_python_bindings
        test_python_bindings.cpp
    )

    target_include_directories(test_python_bindings PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_python_bindings PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME PythonBindingTests
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
        -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_python_bindings.exe
        -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(PythonBindingTests PROPERTIES
        TIMEOUT 120
    )

    # Install test executables
    install(TARGETS
        test_python_bindings
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/python
    )
else()
    message(STATUS "Python binding tests disabled - requires QTFORGE_BUILD_PYTHON_BINDINGS=ON, Python3, and pybind11")
endif()
