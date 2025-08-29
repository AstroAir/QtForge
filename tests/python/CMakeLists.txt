# QtForge Python Module Tests
cmake_minimum_required(VERSION 3.21)

# Python tests require Python development headers and pybind11
find_package(Python3 COMPONENTS Interpreter Development QUIET)
find_package(pybind11 QUIET)

if(Python3_FOUND)
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

    add_test(NAME PythonBindingTests COMMAND test_python_bindings)
    set_tests_properties(PythonBindingTests PROPERTIES
        TIMEOUT 120
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Install test executables
    install(TARGETS
        test_python_bindings
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/python
    )
else()
    message(STATUS "Python3 not found - Python tests disabled")
endif()
