# QtForgeDependencies.cmake
# Dependency management module for QtForge
# Provides centralized dependency detection and configuration

include_guard(GLOBAL)

# Include platform detection
include(${CMAKE_CURRENT_LIST_DIR}/QtForgePlatform.cmake)

# Dependency detection variables
set(QTFORGE_QT_FOUND FALSE)
set(QTFORGE_QT_VERSION "unknown")
set(QTFORGE_QT_COMPONENTS_FOUND "")

#[=======================================================================[.rst:
qtforge_find_qt
---------------

Finds Qt6 and its components with intelligent fallback to Qt5.

Options:
- REQUIRED: Make Qt a required dependency
- COMPONENTS: List of Qt components to find
- OPTIONAL_COMPONENTS: List of optional Qt components

Sets the following variables:
- QTFORGE_QT_FOUND: TRUE if Qt was found
- QTFORGE_QT_VERSION: Qt version string
- QTFORGE_QT_MAJOR_VERSION: Qt major version (5 or 6)
- QTFORGE_QT_COMPONENTS_FOUND: List of found components
#]=======================================================================]
function(qtforge_find_qt)
    cmake_parse_arguments(QT
        "REQUIRED"
        ""
        "COMPONENTS;OPTIONAL_COMPONENTS"
        ${ARGN}
    )

    # Default components
    if(NOT QT_COMPONENTS)
        set(QT_COMPONENTS Core)
    endif()

    # Try Qt6 first
    find_package(Qt6 QUIET COMPONENTS ${QT_COMPONENTS})

    if(Qt6_FOUND)
        set(QTFORGE_QT_FOUND TRUE PARENT_SCOPE)
        set(QTFORGE_QT_VERSION "${Qt6_VERSION}" PARENT_SCOPE)
        set(QTFORGE_QT_MAJOR_VERSION 6 PARENT_SCOPE)
        set(QT_VERSION_MAJOR 6 PARENT_SCOPE)

        # Check for optional components
        if(QT_OPTIONAL_COMPONENTS)
            find_package(Qt6 QUIET COMPONENTS ${QT_OPTIONAL_COMPONENTS})
        endif()

        # Build list of found components
        set(FOUND_COMPONENTS "")
        foreach(COMPONENT ${QT_COMPONENTS} ${QT_OPTIONAL_COMPONENTS})
            if(TARGET Qt6::${COMPONENT})
                list(APPEND FOUND_COMPONENTS ${COMPONENT})
            endif()
        endforeach()
        set(QTFORGE_QT_COMPONENTS_FOUND "${FOUND_COMPONENTS}" PARENT_SCOPE)

        message(STATUS "QtForge: Found Qt6 ${Qt6_VERSION}")
        message(STATUS "QtForge: Qt6 components: ${FOUND_COMPONENTS}")

        # Enable automatic moc generation for Qt classes with Q_OBJECT
        set(CMAKE_AUTOMOC ON PARENT_SCOPE)
        set(CMAKE_AUTORCC ON PARENT_SCOPE)
        set(CMAKE_AUTOUIC ON PARENT_SCOPE)
        message(STATUS "QtForge: Enabled Qt AUTOMOC, AUTORCC, and AUTOUIC")

    else()
        # Fallback to Qt5
        find_package(Qt5 QUIET COMPONENTS ${QT_COMPONENTS})

        if(Qt5_FOUND)
            set(QTFORGE_QT_FOUND TRUE PARENT_SCOPE)
            set(QTFORGE_QT_VERSION "${Qt5_VERSION}" PARENT_SCOPE)
            set(QTFORGE_QT_MAJOR_VERSION 5 PARENT_SCOPE)
            set(QT_VERSION_MAJOR 5 PARENT_SCOPE)

            # Check for optional components
            if(QT_OPTIONAL_COMPONENTS)
                find_package(Qt5 QUIET COMPONENTS ${QT_OPTIONAL_COMPONENTS})
            endif()

            # Build list of found components
            set(FOUND_COMPONENTS "")
            foreach(COMPONENT ${QT_COMPONENTS} ${QT_OPTIONAL_COMPONENTS})
                if(TARGET Qt5::${COMPONENT})
                    list(APPEND FOUND_COMPONENTS ${COMPONENT})
                endif()
            endforeach()
            set(QTFORGE_QT_COMPONENTS_FOUND "${FOUND_COMPONENTS}" PARENT_SCOPE)

            message(STATUS "QtForge: Found Qt5 ${Qt5_VERSION}")
            message(STATUS "QtForge: Qt5 components: ${FOUND_COMPONENTS}")

            # Enable automatic moc generation for Qt classes with Q_OBJECT
            set(CMAKE_AUTOMOC ON PARENT_SCOPE)
            set(CMAKE_AUTORCC ON PARENT_SCOPE)
            set(CMAKE_AUTOUIC ON PARENT_SCOPE)
            message(STATUS "QtForge: Enabled Qt AUTOMOC, AUTORCC, and AUTOUIC")

        else()
            set(QTFORGE_QT_FOUND FALSE PARENT_SCOPE)

            if(QT_REQUIRED)
                message(FATAL_ERROR "QtForge: Qt6 or Qt5 is required but not found")
            else()
                message(WARNING "QtForge: Qt6 or Qt5 not found")
            endif()
        endif()
    endif()
endfunction()

#[=======================================================================[.rst:
qtforge_find_system_dependencies
--------------------------------

Finds system dependencies based on platform and enabled features.
#]=======================================================================]
function(qtforge_find_system_dependencies)
    # Threading support (required)
    find_package(Threads REQUIRED)

    # Platform-specific dependencies
    if(QTFORGE_IS_LINUX)
        # Find X11 for Linux desktop applications
        find_package(X11)
        if(X11_FOUND)
            message(STATUS "QtForge: Found X11 support")
        endif()

        # Find Wayland for modern Linux
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(WAYLAND QUIET wayland-client)
            if(WAYLAND_FOUND)
                message(STATUS "QtForge: Found Wayland support")
            endif()
        endif()

    elseif(QTFORGE_IS_WINDOWS)
        # Windows-specific libraries are typically found automatically
        message(STATUS "QtForge: Using Windows system libraries")

    elseif(QTFORGE_IS_MACOS)
        # macOS frameworks
        find_library(COCOA_FRAMEWORK Cocoa)
        find_library(FOUNDATION_FRAMEWORK Foundation)
        if(COCOA_FRAMEWORK AND FOUNDATION_FRAMEWORK)
            message(STATUS "QtForge: Found macOS frameworks")
        endif()

    elseif(QTFORGE_IS_ANDROID)
        # Android-specific setup
        if(NOT ANDROID_NDK)
            message(FATAL_ERROR "QtForge: ANDROID_NDK must be set for Android builds")
        endif()
        message(STATUS "QtForge: Using Android NDK: ${ANDROID_NDK}")

    elseif(QTFORGE_IS_IOS)
        # iOS-specific setup
        message(STATUS "QtForge: Configuring for iOS")
    endif()
endfunction()

#[=======================================================================[.rst:
qtforge_find_development_dependencies
------------------------------------

Finds development and testing dependencies.
#]=======================================================================]
function(qtforge_find_development_dependencies)
    # Documentation generation
    if(QTFORGE_BUILD_DOCS)
        find_package(Doxygen QUIET)
        if(DOXYGEN_FOUND)
            message(STATUS "QtForge: Found Doxygen for documentation generation")
        else()
            message(WARNING "QtForge: Doxygen not found, documentation will not be generated")
        endif()
    endif()

    # Testing framework
    if(QTFORGE_BUILD_TESTS)
        # Try to find Google Test
        find_package(GTest QUIET)
        if(GTest_FOUND)
            message(STATUS "QtForge: Found Google Test")
        else()
            # Fallback to Qt Test - only find if not already available
            if(QTFORGE_QT_FOUND AND QT_VERSION_MAJOR EQUAL 6)
                if(NOT TARGET Qt6::Test)
                    find_package(Qt6 QUIET COMPONENTS Test)
                endif()
                if(TARGET Qt6::Test)
                    message(STATUS "QtForge: Using Qt6 Test framework")
                endif()
            elseif(QTFORGE_QT_FOUND AND QT_VERSION_MAJOR EQUAL 5)
                if(NOT TARGET Qt5::Test)
                    find_package(Qt5 QUIET COMPONENTS Test)
                endif()
                if(TARGET Qt5::Test)
                    message(STATUS "QtForge: Using Qt5 Test framework")
                endif()
            endif()
        endif()
    endif()

    # Benchmarking
    if(QTFORGE_BUILD_BENCHMARKS)
        find_package(benchmark QUIET)
        if(benchmark_FOUND)
            message(STATUS "QtForge: Found Google Benchmark")
        else()
            message(WARNING "QtForge: Google Benchmark not found, benchmarks will not be built")
        endif()
    endif()
endfunction()

#[=======================================================================[.rst:
qtforge_find_python_dependencies
---------------------------------

Finds Python interpreter and pybind11 for Python bindings.
#]=======================================================================]
function(qtforge_find_python_dependencies)
    if(NOT QTFORGE_BUILD_PYTHON_BINDINGS)
        return()
    endif()

    message(STATUS "QtForge: Finding Python dependencies...")

    # Find Python interpreter - prefer system Python
    set(Python_FIND_STRATEGY VERSION)
    set(Python_FIND_REGISTRY LAST)
    find_package(Python 3.8 COMPONENTS Interpreter Development QUIET)

    if(Python_FOUND)
        message(STATUS "QtForge: Found Python ${Python_VERSION} at ${Python_EXECUTABLE}")

        # Check Python version requirements
        if(Python_VERSION VERSION_LESS ${QTFORGE_PYTHON_MIN_VERSION})
            message(WARNING "QtForge: Python ${Python_VERSION} is below minimum required version ${QTFORGE_PYTHON_MIN_VERSION}")
            set(QTFORGE_PYTHON_FOUND FALSE PARENT_SCOPE)
            return()
        endif()

        if(Python_VERSION VERSION_GREATER ${QTFORGE_PYTHON_MAX_VERSION})
            message(WARNING "QtForge: Python ${Python_VERSION} is above maximum supported version ${QTFORGE_PYTHON_MAX_VERSION}")
        endif()

        # Set Python as found since interpreter was located
        set(QTFORGE_PYTHON_FOUND TRUE PARENT_SCOPE)
        set(QTFORGE_PYTHON_VERSION ${Python_VERSION} PARENT_SCOPE)
        set(QTFORGE_PYTHON_EXECUTABLE ${Python_EXECUTABLE} PARENT_SCOPE)

        # Find pybind11
        find_package(pybind11 QUIET)
        if(pybind11_FOUND)
            message(STATUS "QtForge: Found pybind11 ${pybind11_VERSION}")
            set(QTFORGE_PYBIND11_FOUND TRUE PARENT_SCOPE)
        else()
            message(WARNING "QtForge: pybind11 not found, Python bindings will not be built")
            set(QTFORGE_PYBIND11_FOUND FALSE PARENT_SCOPE)
        endif()
    else()
        message(WARNING "QtForge: Python interpreter not found, Python bindings will not be built")
        set(QTFORGE_PYTHON_FOUND FALSE PARENT_SCOPE)
        set(QTFORGE_PYBIND11_FOUND FALSE PARENT_SCOPE)
    endif()
endfunction()

#[=======================================================================[.rst:
qtforge_find_lua_dependencies
------------------------------

Finds Lua interpreter and sol2 for Lua bindings.
#]=======================================================================]
function(qtforge_find_lua_dependencies)
    if(NOT QTFORGE_BUILD_LUA_BINDINGS)
        return()
    endif()

    message(STATUS "QtForge: Finding Lua dependencies...")

    # Find Lua interpreter
    find_package(Lua QUIET)

    if(Lua_FOUND)
        message(STATUS "QtForge: Found Lua ${LUA_VERSION_STRING}")

        # Check Lua version requirements
        if(LUA_VERSION_STRING VERSION_LESS ${QTFORGE_LUA_MIN_VERSION})
            message(WARNING "QtForge: Lua ${LUA_VERSION_STRING} is below minimum required version ${QTFORGE_LUA_MIN_VERSION}")
            set(QTFORGE_LUA_FOUND FALSE PARENT_SCOPE)
            return()
        endif()

        if(LUA_VERSION_STRING VERSION_GREATER ${QTFORGE_LUA_MAX_VERSION})
            message(WARNING "QtForge: Lua ${LUA_VERSION_STRING} is above maximum supported version ${QTFORGE_LUA_MAX_VERSION}")
        endif()

        # Find sol2 (header-only library)
        find_package(sol2 QUIET)
        if(sol2_FOUND)
            message(STATUS "QtForge: Found sol2 ${sol2_VERSION}")
            set(QTFORGE_SOL2_FOUND TRUE PARENT_SCOPE)
            set(QTFORGE_LUA_FOUND TRUE PARENT_SCOPE)
            set(QTFORGE_LUA_VERSION ${LUA_VERSION_STRING} PARENT_SCOPE)
            set(QTFORGE_LUA_INCLUDE_DIR ${LUA_INCLUDE_DIR} PARENT_SCOPE)
            set(QTFORGE_LUA_LIBRARIES ${LUA_LIBRARIES} PARENT_SCOPE)
        else()
            # Try to find sol2 manually as it's header-only
            find_path(SOL2_INCLUDE_DIR
                NAMES sol/sol.hpp
                PATHS
                    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sol2/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/external/sol2/include
                    /usr/include
                    /usr/local/include
                    ${CMAKE_INSTALL_PREFIX}/include
                PATH_SUFFIXES sol2
            )

            if(SOL2_INCLUDE_DIR)
                message(STATUS "QtForge: Found sol2 headers at ${SOL2_INCLUDE_DIR}")
                set(QTFORGE_SOL2_FOUND TRUE PARENT_SCOPE)
                set(QTFORGE_LUA_FOUND TRUE PARENT_SCOPE)
                set(QTFORGE_LUA_VERSION ${LUA_VERSION_STRING} PARENT_SCOPE)
                set(QTFORGE_LUA_INCLUDE_DIR ${LUA_INCLUDE_DIR} PARENT_SCOPE)
                set(QTFORGE_LUA_LIBRARIES ${LUA_LIBRARIES} PARENT_SCOPE)
                set(QTFORGE_SOL2_INCLUDE_DIR ${SOL2_INCLUDE_DIR} PARENT_SCOPE)
            else()
                message(WARNING "QtForge: sol2 not found, Lua bindings will not be built")
                set(QTFORGE_LUA_FOUND FALSE PARENT_SCOPE)
                set(QTFORGE_SOL2_FOUND FALSE PARENT_SCOPE)
            endif()
        endif()
    else()
        message(WARNING "QtForge: Lua interpreter not found, Lua bindings will not be built")
        set(QTFORGE_LUA_FOUND FALSE PARENT_SCOPE)
        set(QTFORGE_SOL2_FOUND FALSE PARENT_SCOPE)
    endif()
endfunction()

#[=======================================================================[.rst:
qtforge_configure_qt_features
-----------------------------

Configures Qt-specific features and components based on availability.
#]=======================================================================]
function(qtforge_configure_qt_features)
    if(NOT QTFORGE_QT_FOUND)
        return()
    endif()

    # Configure Qt components
    set(QT_PREFIX "Qt${QT_VERSION_MAJOR}")

    # Core is always required
    if(NOT TARGET ${QT_PREFIX}::Core)
        message(FATAL_ERROR "QtForge: Qt Core component is required")
    endif()

    # Network component
    if("Network" IN_LIST QTFORGE_QT_COMPONENTS_FOUND)
        set(QTFORGE_HAS_NETWORK TRUE PARENT_SCOPE)
        message(STATUS "QtForge: Network support enabled")
    else()
        set(QTFORGE_HAS_NETWORK FALSE PARENT_SCOPE)
        if(QTFORGE_BUILD_NETWORK)
            message(WARNING "QtForge: Network component requested but not found")
        endif()
    endif()

    # UI components
    if("Widgets" IN_LIST QTFORGE_QT_COMPONENTS_FOUND)
        set(QTFORGE_HAS_WIDGETS TRUE PARENT_SCOPE)
        message(STATUS "QtForge: Widgets support enabled")
    else()
        set(QTFORGE_HAS_WIDGETS FALSE PARENT_SCOPE)
        if(QTFORGE_BUILD_UI)
            message(WARNING "QtForge: Widgets component requested but not found")
        endif()
    endif()

    # SQL component
    if("Sql" IN_LIST QTFORGE_QT_COMPONENTS_FOUND)
        set(QTFORGE_HAS_SQL TRUE PARENT_SCOPE)
        message(STATUS "QtForge: SQL support enabled")
    else()
        set(QTFORGE_HAS_SQL FALSE PARENT_SCOPE)
    endif()

    # Concurrent component
    if("Concurrent" IN_LIST QTFORGE_QT_COMPONENTS_FOUND)
        set(QTFORGE_HAS_CONCURRENT TRUE PARENT_SCOPE)
        message(STATUS "QtForge: Concurrent support enabled")
    else()
        set(QTFORGE_HAS_CONCURRENT FALSE PARENT_SCOPE)
    endif()

    # State Machine component
    if("StateMachine" IN_LIST QTFORGE_QT_COMPONENTS_FOUND)
        set(QTFORGE_HAS_STATEMACHINE TRUE PARENT_SCOPE)
        message(STATUS "QtForge: StateMachine support enabled")
    else()
        set(QTFORGE_HAS_STATEMACHINE FALSE PARENT_SCOPE)
    endif()
endfunction()

#[=======================================================================[.rst:
qtforge_setup_dependencies
--------------------------

Main function to set up all dependencies for QtForge.
This should be called from the main CMakeLists.txt.
#]=======================================================================]
function(qtforge_setup_dependencies)
    message(STATUS "QtForge: Setting up dependencies...")

    # Find Qt with required and optional components
    set(QT_REQUIRED_COMPONENTS Core)
    set(QT_OPTIONAL_COMPONENTS Network Widgets Sql Concurrent StateMachine Test WebSockets HttpServer)

    qtforge_find_qt(
        REQUIRED
        COMPONENTS ${QT_REQUIRED_COMPONENTS}
        OPTIONAL_COMPONENTS ${QT_OPTIONAL_COMPONENTS}
    )

    # Find system dependencies
    qtforge_find_system_dependencies()

    # Find development dependencies
    qtforge_find_development_dependencies()

    # Find Python dependencies
    qtforge_find_python_dependencies()

    # Find Lua dependencies
    qtforge_find_lua_dependencies()

    # Configure Qt features
    qtforge_configure_qt_features()

    # Propagate Qt variables to parent scope
    set(QTFORGE_QT_FOUND ${QTFORGE_QT_FOUND} PARENT_SCOPE)
    set(QTFORGE_QT_VERSION ${QTFORGE_QT_VERSION} PARENT_SCOPE)
    set(QTFORGE_QT_MAJOR_VERSION ${QTFORGE_QT_MAJOR_VERSION} PARENT_SCOPE)
    set(QT_VERSION_MAJOR ${QT_VERSION_MAJOR} PARENT_SCOPE)
    set(QTFORGE_QT_COMPONENTS_FOUND ${QTFORGE_QT_COMPONENTS_FOUND} PARENT_SCOPE)

    # Propagate feature flags to parent scope
    set(QTFORGE_HAS_NETWORK ${QTFORGE_HAS_NETWORK} PARENT_SCOPE)
    set(QTFORGE_HAS_WIDGETS ${QTFORGE_HAS_WIDGETS} PARENT_SCOPE)
    set(QTFORGE_HAS_SQL ${QTFORGE_HAS_SQL} PARENT_SCOPE)
    set(QTFORGE_HAS_CONCURRENT ${QTFORGE_HAS_CONCURRENT} PARENT_SCOPE)
    set(QTFORGE_HAS_STATEMACHINE ${QTFORGE_HAS_STATEMACHINE} PARENT_SCOPE)

    # Propagate Python variables to parent scope
    set(QTFORGE_PYTHON_FOUND ${QTFORGE_PYTHON_FOUND} PARENT_SCOPE)
    set(QTFORGE_PYBIND11_FOUND ${QTFORGE_PYBIND11_FOUND} PARENT_SCOPE)
    set(QTFORGE_PYTHON_VERSION ${QTFORGE_PYTHON_VERSION} PARENT_SCOPE)
    set(QTFORGE_PYTHON_EXECUTABLE ${QTFORGE_PYTHON_EXECUTABLE} PARENT_SCOPE)

    # Propagate Lua variables to parent scope
    set(QTFORGE_LUA_FOUND ${QTFORGE_LUA_FOUND} PARENT_SCOPE)
    set(QTFORGE_SOL2_FOUND ${QTFORGE_SOL2_FOUND} PARENT_SCOPE)
    set(QTFORGE_LUA_VERSION ${QTFORGE_LUA_VERSION} PARENT_SCOPE)
    set(QTFORGE_LUA_INCLUDE_DIR ${QTFORGE_LUA_INCLUDE_DIR} PARENT_SCOPE)
    set(QTFORGE_LUA_LIBRARIES ${QTFORGE_LUA_LIBRARIES} PARENT_SCOPE)
    set(QTFORGE_SOL2_INCLUDE_DIR ${QTFORGE_SOL2_INCLUDE_DIR} PARENT_SCOPE)

    message(STATUS "QtForge: Dependency setup complete")
endfunction()

#[=======================================================================[.rst:
qtforge_link_qt_libraries
-------------------------

Helper function to link Qt libraries to a target.

Usage:
  qtforge_link_qt_libraries(target_name COMPONENTS Core Network Widgets)
#]=======================================================================]
function(qtforge_link_qt_libraries TARGET_NAME)
    cmake_parse_arguments(LINK
        ""
        ""
        "COMPONENTS"
        ${ARGN}
    )

    if(NOT QTFORGE_QT_FOUND)
        message(FATAL_ERROR "QtForge: Qt not found, cannot link Qt libraries")
    endif()

    set(QT_PREFIX "Qt${QT_VERSION_MAJOR}")

    foreach(COMPONENT ${LINK_COMPONENTS})
        if(TARGET ${QT_PREFIX}::${COMPONENT})
            target_link_libraries(${TARGET_NAME} PRIVATE ${QT_PREFIX}::${COMPONENT})
        else()
            message(WARNING "QtForge: Qt component ${COMPONENT} not available for linking")
        endif()
    endforeach()
endfunction()

# Note: qtforge_setup_dependencies() should be called explicitly from main CMakeLists.txt
# after all options are configured, not automatically when this module is included
