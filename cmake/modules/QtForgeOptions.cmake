# QtForgeOptions.cmake
# Build options and feature flags module for QtForge
# Provides centralized configuration for all build options

include_guard(GLOBAL)

# Include required CMake modules
include(CMakeDependentOption)

# Include platform detection
include(${CMAKE_CURRENT_LIST_DIR}/QtForgePlatform.cmake)

#[=======================================================================[.rst:
qtforge_define_options
---------------------

Defines all build options for QtForge with intelligent defaults based on
platform and available dependencies.

This function should be called early in the main CMakeLists.txt to set up
all configuration options.
#]=======================================================================]
function(qtforge_define_options)
    # Core library options
    option(QTFORGE_BUILD_SHARED "Build shared libraries" ON)
    option(QTFORGE_BUILD_STATIC "Build static libraries" OFF)

    # Component options (auto-detect Qt components)
    cmake_dependent_option(QTFORGE_BUILD_NETWORK
        "Build network plugin support"
        ON
        "Qt6Network_FOUND"
        OFF
    )

    cmake_dependent_option(QTFORGE_BUILD_UI
        "Build UI plugin support"
        ON
        "Qt6Widgets_FOUND"
        OFF
    )

    cmake_dependent_option(QTFORGE_BUILD_SQL
        "Build SQL plugin support"
        ON
        "Qt6Sql_FOUND"
        OFF
    )

    cmake_dependent_option(QTFORGE_BUILD_CONCURRENT
        "Build concurrent plugin support"
        ON
        "Qt6Concurrent_FOUND"
        OFF
    )

    # Development options
    option(QTFORGE_BUILD_EXAMPLES "Build example plugins" ON)
    option(QTFORGE_BUILD_TESTS "Build unit tests" OFF)
    option(QTFORGE_BUILD_BENCHMARKS "Build performance benchmarks" OFF)
    option(QTFORGE_BUILD_DOCS "Build documentation" OFF)

    # Python binding options
    option(QTFORGE_BUILD_PYTHON_BINDINGS "Build Python bindings using pybind11" OFF)
    option(QTFORGE_PYTHON_BINDINGS_INSTALL "Install Python bindings" ON)
    option(QTFORGE_PYTHON_BINDINGS_TESTS "Build Python binding tests" OFF)

    # Advanced options
    option(QTFORGE_BUILD_COMPONENT_TESTS "Build component-specific tests" OFF)
    option(QTFORGE_ENABLE_COMPONENT_LOGGING "Enable detailed component logging" OFF)
    option(QTFORGE_ENABLE_PLUGIN_PROFILING "Enable plugin performance profiling" OFF)
    option(QTFORGE_ENABLE_SECURITY_HARDENING "Enable security hardening features" ON)

    # Compiler options
    option(QTFORGE_ENABLE_WARNINGS "Enable comprehensive compiler warnings" ON)
    option(QTFORGE_ENABLE_WERROR "Treat warnings as errors" OFF)
    option(QTFORGE_ENABLE_SANITIZERS "Enable sanitizers in debug builds" OFF)
    option(QTFORGE_ENABLE_LTO "Enable Link Time Optimization" OFF)
    option(QTFORGE_ENABLE_FAST_MATH "Enable fast math optimizations" OFF)

    # Platform-specific options
    if(QTFORGE_IS_WINDOWS)
        option(QTFORGE_ENABLE_WINDOWS_MANIFEST "Enable Windows application manifest" ON)
        option(QTFORGE_ENABLE_WINDOWS_RESOURCES "Enable Windows resources" ON)
    endif()

    if(QTFORGE_IS_MACOS)
        option(QTFORGE_ENABLE_MACOS_BUNDLE "Create macOS application bundles" ON)
        option(QTFORGE_ENABLE_MACOS_CODESIGN "Enable macOS code signing" OFF)
    endif()

    if(QTFORGE_IS_LINUX)
        option(QTFORGE_ENABLE_APPIMAGE "Enable AppImage packaging" OFF)
        option(QTFORGE_ENABLE_FLATPAK "Enable Flatpak packaging" OFF)
    endif()

    if(QTFORGE_IS_ANDROID)
        option(QTFORGE_ENABLE_ANDROID_GRADLE "Use Gradle for Android builds" ON)
        set(QTFORGE_ANDROID_MIN_SDK_VERSION "21" CACHE STRING "Minimum Android SDK version")
        set(QTFORGE_ANDROID_TARGET_SDK_VERSION "33" CACHE STRING "Target Android SDK version")
    endif()

    if(QTFORGE_IS_IOS)
        option(QTFORGE_ENABLE_IOS_SIMULATOR "Build for iOS Simulator" OFF)
        set(QTFORGE_IOS_DEPLOYMENT_TARGET "12.0" CACHE STRING "iOS deployment target")
    endif()

    # Cross-compilation options
    option(QTFORGE_ENABLE_CROSS_COMPILATION "Enable cross-compilation support" OFF)

    # Packaging options
    option(QTFORGE_CREATE_PACKAGES "Create installation packages" OFF)
    option(QTFORGE_PACKAGE_COMPONENTS "Create component-based packages" ON)

    # Installation options
    set(QTFORGE_INSTALL_RUNTIME_DIR "bin" CACHE STRING "Runtime installation directory")
    set(QTFORGE_INSTALL_LIBRARY_DIR "lib" CACHE STRING "Library installation directory")
    set(QTFORGE_INSTALL_ARCHIVE_DIR "lib" CACHE STRING "Archive installation directory")
    set(QTFORGE_INSTALL_INCLUDE_DIR "include" CACHE STRING "Header installation directory")
    set(QTFORGE_INSTALL_CMAKE_DIR "lib/cmake/QtForge" CACHE STRING "CMake files installation directory")
    set(QTFORGE_INSTALL_PKGCONFIG_DIR "lib/pkgconfig" CACHE STRING "pkg-config files installation directory")
    set(QTFORGE_INSTALL_DOC_DIR "share/doc/qtforge" CACHE STRING "Documentation installation directory")

    # Python binding installation options
    if(QTFORGE_BUILD_PYTHON_BINDINGS)
        set(QTFORGE_PYTHON_INSTALL_DIR "" CACHE STRING "Python module installation directory (auto-detected if empty)")
        set(QTFORGE_PYTHON_MIN_VERSION "3.8" CACHE STRING "Minimum required Python version")
        set(QTFORGE_PYTHON_MAX_VERSION "3.12" CACHE STRING "Maximum supported Python version")
    endif()

    # Print configuration summary
    qtforge_print_configuration_summary()
endfunction()

#[=======================================================================[.rst:
qtforge_validate_options
------------------------

Validates build options and resolves conflicts.
#]=======================================================================]
function(qtforge_validate_options)
    # Validate library type options
    if(NOT QTFORGE_BUILD_SHARED AND NOT QTFORGE_BUILD_STATIC)
        message(FATAL_ERROR "QtForge: At least one of QTFORGE_BUILD_SHARED or QTFORGE_BUILD_STATIC must be enabled")
    endif()

    # Validate component dependencies
    if(QTFORGE_BUILD_UI AND NOT QTFORGE_BUILD_NETWORK)
        message(WARNING "QtForge: UI components may require network support for some features")
    endif()

    # Validate test options
    if(QTFORGE_BUILD_COMPONENT_TESTS AND NOT QTFORGE_BUILD_TESTS)
        message(STATUS "QtForge: Enabling QTFORGE_BUILD_TESTS because QTFORGE_BUILD_COMPONENT_TESTS is enabled")
        set(QTFORGE_BUILD_TESTS ON PARENT_SCOPE)
    endif()

    # Validate platform-specific options
    if(QTFORGE_IS_ANDROID AND QTFORGE_BUILD_SHARED)
        message(STATUS "QtForge: Android builds typically use static libraries")
    endif()

    if(QTFORGE_IS_IOS AND QTFORGE_BUILD_SHARED)
        message(WARNING "QtForge: iOS does not support shared libraries, forcing static build")
        set(QTFORGE_BUILD_SHARED OFF PARENT_SCOPE)
        set(QTFORGE_BUILD_STATIC ON PARENT_SCOPE)
    endif()

    # Validate compiler options
    if(QTFORGE_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Release")
        message(WARNING "QtForge: Sanitizers are typically used in Debug builds")
    endif()

    if(QTFORGE_ENABLE_LTO AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "QtForge: LTO is typically used in Release builds")
    endif()
endfunction()

#[=======================================================================[.rst:
qtforge_print_configuration_summary
-----------------------------------

Prints a summary of the current build configuration.
#]=======================================================================]
function(qtforge_print_configuration_summary)
    message(STATUS "")
    message(STATUS "QtForge Build Configuration Summary:")
    message(STATUS "====================================")
    message(STATUS "Platform: ${QTFORGE_PLATFORM_NAME} (${QTFORGE_ARCH_NAME})")
    message(STATUS "Compiler: ${QTFORGE_COMPILER_NAME} ${QTFORGE_COMPILER_VERSION}")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "")

    # Library types
    message(STATUS "Library Types:")
    message(STATUS "  Shared Libraries: ${QTFORGE_BUILD_SHARED}")
    message(STATUS "  Static Libraries: ${QTFORGE_BUILD_STATIC}")
    message(STATUS "")

    # Components
    message(STATUS "Components:")
    message(STATUS "  Network Support: ${QTFORGE_BUILD_NETWORK}")
    message(STATUS "  UI Support: ${QTFORGE_BUILD_UI}")
    if(DEFINED QTFORGE_BUILD_SQL)
        message(STATUS "  SQL Support: ${QTFORGE_BUILD_SQL}")
    endif()
    if(DEFINED QTFORGE_BUILD_CONCURRENT)
        message(STATUS "  Concurrent Support: ${QTFORGE_BUILD_CONCURRENT}")
    endif()
    message(STATUS "")

    # Development options
    message(STATUS "Development:")
    message(STATUS "  Examples: ${QTFORGE_BUILD_EXAMPLES}")
    message(STATUS "  Tests: ${QTFORGE_BUILD_TESTS}")
    message(STATUS "  Benchmarks: ${QTFORGE_BUILD_BENCHMARKS}")
    message(STATUS "  Documentation: ${QTFORGE_BUILD_DOCS}")
    message(STATUS "")

    # Compiler options
    message(STATUS "Compiler Options:")
    message(STATUS "  Warnings: ${QTFORGE_ENABLE_WARNINGS}")
    message(STATUS "  Warnings as Errors: ${QTFORGE_ENABLE_WERROR}")
    message(STATUS "  Sanitizers: ${QTFORGE_ENABLE_SANITIZERS}")
    message(STATUS "  Link Time Optimization: ${QTFORGE_ENABLE_LTO}")
    message(STATUS "  Fast Math: ${QTFORGE_ENABLE_FAST_MATH}")
    message(STATUS "")

    # Platform-specific options
    if(QTFORGE_IS_ANDROID)
        message(STATUS "Android Options:")
        message(STATUS "  Min SDK Version: ${QTFORGE_ANDROID_MIN_SDK_VERSION}")
        message(STATUS "  Target SDK Version: ${QTFORGE_ANDROID_TARGET_SDK_VERSION}")
        message(STATUS "")
    endif()

    if(QTFORGE_IS_IOS)
        message(STATUS "iOS Options:")
        message(STATUS "  Deployment Target: ${QTFORGE_IOS_DEPLOYMENT_TARGET}")
        message(STATUS "  Simulator Build: ${QTFORGE_ENABLE_IOS_SIMULATOR}")
        message(STATUS "")
    endif()

    # Installation directories
    message(STATUS "Installation Directories:")
    message(STATUS "  Runtime: ${CMAKE_INSTALL_PREFIX}/${QTFORGE_INSTALL_RUNTIME_DIR}")
    message(STATUS "  Libraries: ${CMAKE_INSTALL_PREFIX}/${QTFORGE_INSTALL_LIBRARY_DIR}")
    message(STATUS "  Headers: ${CMAKE_INSTALL_PREFIX}/${QTFORGE_INSTALL_INCLUDE_DIR}")
    message(STATUS "  CMake: ${CMAKE_INSTALL_PREFIX}/${QTFORGE_INSTALL_CMAKE_DIR}")
    message(STATUS "")
endfunction()

#[=======================================================================[.rst:
qtforge_configure_build_type
----------------------------

Configures build type-specific settings.
#]=======================================================================]
function(qtforge_configure_build_type)
    # Set default build type if not specified
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
    endif()

    # Configure build type specific settings
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_definitions(QTFORGE_DEBUG=1)
        if(QTFORGE_ENABLE_COMPONENT_LOGGING)
            add_compile_definitions(QTFORGE_ENABLE_LOGGING=1)
        endif()
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_definitions(QTFORGE_RELEASE=1 NDEBUG=1)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_compile_definitions(QTFORGE_RELEASE=1 NDEBUG=1)
    elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        add_compile_definitions(QTFORGE_RELEASE=1 NDEBUG=1)
    endif()
endfunction()

# Auto-configure options when this module is included
qtforge_define_options()
qtforge_validate_options()
qtforge_configure_build_type()
