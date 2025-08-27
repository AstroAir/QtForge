# Test CMake file to verify Python binding setup
# This file can be used to test the Python binding configuration

cmake_minimum_required(VERSION 3.21)

# Test if pybind11 can be found
find_package(pybind11 QUIET)

if(pybind11_FOUND)
    message(STATUS "Test: pybind11 found - version ${pybind11_VERSION}")

    # Test if Python can be found
    find_package(Python COMPONENTS Interpreter Development QUIET)

    if(Python_FOUND)
        message(STATUS "Test: Python found - version ${Python_VERSION}")
        message(STATUS "Test: Python executable: ${Python_EXECUTABLE}")
        message(STATUS "Test: Python include dirs: ${Python_INCLUDE_DIRS}")
        message(STATUS "Test: Python libraries: ${Python_LIBRARIES}")

        # Test basic pybind11 module creation
        try_compile(PYBIND11_TEST_COMPILE
            ${CMAKE_CURRENT_BINARY_DIR}/test_pybind11
            ${CMAKE_CURRENT_SOURCE_DIR}/test_pybind11.cpp
            CMAKE_FLAGS
                "-DINCLUDE_DIRECTORIES=${pybind11_INCLUDE_DIRS};${Python_INCLUDE_DIRS}"
                "-DLINK_LIBRARIES=${Python_LIBRARIES}"
            OUTPUT_VARIABLE COMPILE_OUTPUT
        )

        if(PYBIND11_TEST_COMPILE)
            message(STATUS "Test: pybind11 compilation test PASSED")
        else()
            message(WARNING "Test: pybind11 compilation test FAILED")
            message(STATUS "Compile output: ${COMPILE_OUTPUT}")
        endif()
    else()
        message(WARNING "Test: Python not found")
    endif()
else()
    message(WARNING "Test: pybind11 not found")
endif()

# Test Qt6 availability
find_package(Qt6 QUIET COMPONENTS Core)
if(Qt6_FOUND)
    message(STATUS "Test: Qt6 Core found - version ${Qt6_VERSION}")
else()
    message(WARNING "Test: Qt6 Core not found")
endif()
