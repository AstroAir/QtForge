# Windows ARM64 cross-compilation toolchain for QtForge
# Cross-compilation toolchain for Windows ARM64 targets

# Ensure we're cross-compiling
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR ARM64)

# Set minimum required CMake version
cmake_minimum_required(VERSION 3.21)

# Visual Studio ARM64 cross-compilation setup
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    # Native Windows cross-compilation
    
    # Try to find Visual Studio with ARM64 support
    set(VS_VERSIONS "2022" "2019")
    set(VS_EDITIONS "Enterprise" "Professional" "Community" "BuildTools")
    
    foreach(VS_VERSION ${VS_VERSIONS})
        foreach(VS_EDITION ${VS_EDITIONS})
            set(VS_PATH "C:/Program Files/Microsoft Visual Studio/${VS_VERSION}/${VS_EDITION}")
            if(EXISTS "${VS_PATH}")
                set(VISUAL_STUDIO_PATH "${VS_PATH}")
                set(VISUAL_STUDIO_VERSION "${VS_VERSION}")
                break()
            endif()
        endforeach()
        if(VISUAL_STUDIO_PATH)
            break()
        endif()
    endforeach()
    
    if(NOT VISUAL_STUDIO_PATH)
        message(FATAL_ERROR "Visual Studio with ARM64 support not found")
    endif()
    
    # Set up MSVC ARM64 cross-compilation
    set(CMAKE_GENERATOR_PLATFORM "ARM64")
    set(CMAKE_VS_PLATFORM_NAME "ARM64")
    
    # Set MSVC toolset
    if(VISUAL_STUDIO_VERSION STREQUAL "2022")
        set(CMAKE_GENERATOR_TOOLSET "v143")
    elseif(VISUAL_STUDIO_VERSION STREQUAL "2019")
        set(CMAKE_GENERATOR_TOOLSET "v142")
    endif()
    
    message(STATUS "Using Visual Studio ${VISUAL_STUDIO_VERSION} for ARM64 cross-compilation")
    
else()
    # Cross-compilation from non-Windows host (e.g., Linux)
    message(FATAL_ERROR "Cross-compilation to Windows ARM64 from ${CMAKE_HOST_SYSTEM_NAME} is not currently supported")
endif()

# Windows ARM64 specific compiler flags
set(WIN_ARM64_COMPILER_FLAGS "/arch:ARMv8.0")

# Apply flags
set(CMAKE_C_FLAGS_INIT "${WIN_ARM64_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${WIN_ARM64_COMPILER_FLAGS}")

# Set optimization flags
set(CMAKE_C_FLAGS_RELEASE_INIT "/O2 /DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "/O2 /DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG_INIT "/Od /Zi")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "/Od /Zi")

# Windows ARM64 specific definitions
add_definitions(-D_WIN32)
add_definitions(-D_WIN64)
add_definitions(-DWIN32)
add_definitions(-DWIN64)
add_definitions(-D_ARM64_)
add_definitions(-DQTFORGE_WINDOWS_ARM64)

# Set runtime library
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Set install prefix for Windows ARM64
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install prefix" FORCE)
endif()

# Qt for Windows ARM64 specific settings
if(Qt6_DIR OR Qt5_DIR)
    # Qt ARM64 deployment settings
    message(STATUS "Configuring Qt for Windows ARM64")
    
    # Look for Qt ARM64 binaries
    if(DEFINED ENV{QT_DIR})
        set(QT_ARM64_PATH "$ENV{QT_DIR}")
    endif()
endif()

# Helper function for Windows ARM64-specific target configuration
function(configure_windows_arm64_target TARGET_NAME)
    # Set Windows ARM64-specific properties
    set_target_properties(${TARGET_NAME} PROPERTIES
        VS_PLATFORM_NAME "ARM64"
        VS_WINRT_COMPONENT TRUE
    )
    
    # Add Windows ARM64-specific compile definitions
    target_compile_definitions(${TARGET_NAME} PRIVATE
        _WIN32
        _WIN64
        WIN32
        WIN64
        _ARM64_
        QTFORGE_WINDOWS_ARM64
    )
    
    # Link Windows ARM64 libraries
    target_link_libraries(${TARGET_NAME} PRIVATE
        kernel32
        user32
        gdi32
        winspool
        shell32
        ole32
        oleaut32
        uuid
        comdlg32
        advapi32
    )
endfunction()

# Windows ARM64 library search configuration
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Enable position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set thread library preference
set(THREADS_PREFER_PTHREAD_FLAG OFF)  # Use Windows threading

# Windows ARM64 packaging configuration
set(CPACK_SYSTEM_NAME "Windows-ARM64")
set(CPACK_GENERATOR "ZIP;NSIS")

# Function to help find Windows ARM64 libraries
function(find_windows_arm64_library VAR_NAME LIB_NAME)
    find_library(${VAR_NAME}
        NAMES ${LIB_NAME}
        PATHS
            "C:/Program Files (x86)/Windows Kits/10/Lib/*/um/arm64"
            "C:/Program Files (x86)/Microsoft Visual Studio/*/*/VC/Tools/MSVC/*/lib/arm64"
        NO_DEFAULT_PATH
    )
    
    if(NOT ${VAR_NAME})
        find_library(${VAR_NAME} ${LIB_NAME})
    endif()
endfunction()

# Print configuration summary
message(STATUS "Windows ARM64 cross-compilation toolchain configuration:")
message(STATUS "  Target: ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
if(VISUAL_STUDIO_PATH)
    message(STATUS "  Visual Studio: ${VISUAL_STUDIO_PATH}")
    message(STATUS "  Toolset: ${CMAKE_GENERATOR_TOOLSET}")
endif()
message(STATUS "  Platform: ${CMAKE_GENERATOR_PLATFORM}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")

# Mark as configured
set(QTFORGE_WINDOWS_ARM64_TOOLCHAIN_CONFIGURED TRUE)
