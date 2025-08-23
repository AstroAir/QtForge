# Linux ARM64 cross-compilation toolchain for QtForge
# Cross-compilation toolchain for Linux ARM64 (aarch64) targets

# Ensure we're cross-compiling
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Set minimum required CMake version
cmake_minimum_required(VERSION 3.21)

# Cross-compilation toolchain prefix
set(CROSS_COMPILE_PREFIX "aarch64-linux-gnu-")

# Try to find the cross-compilation toolchain
set(TOOLCHAIN_PATHS
    "/usr/bin"
    "/usr/local/bin"
    "/opt/cross/bin"
    "/opt/gcc-linaro-aarch64-linux-gnu/bin"
    "$ENV{CROSS_COMPILE_ROOT}/bin"
)

# Find the cross-compiler
find_program(CMAKE_C_COMPILER
    NAMES ${CROSS_COMPILE_PREFIX}gcc
    PATHS ${TOOLCHAIN_PATHS}
    NO_DEFAULT_PATH
)

find_program(CMAKE_CXX_COMPILER
    NAMES ${CROSS_COMPILE_PREFIX}g++
    PATHS ${TOOLCHAIN_PATHS}
    NO_DEFAULT_PATH
)

# Fallback to system PATH if not found in specific paths
if(NOT CMAKE_C_COMPILER)
    find_program(CMAKE_C_COMPILER ${CROSS_COMPILE_PREFIX}gcc)
endif()

if(NOT CMAKE_CXX_COMPILER)
    find_program(CMAKE_CXX_COMPILER ${CROSS_COMPILE_PREFIX}g++)
endif()

# Check if compilers were found
if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "ARM64 cross-compiler not found. Please install gcc-aarch64-linux-gnu or set CROSS_COMPILE_ROOT environment variable")
endif()

if(NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "ARM64 cross-compiler not found. Please install g++-aarch64-linux-gnu or set CROSS_COMPILE_ROOT environment variable")
endif()

# Get toolchain directory
get_filename_component(TOOLCHAIN_DIR ${CMAKE_C_COMPILER} DIRECTORY)

# Set other tools
find_program(CMAKE_AR
    NAMES ${CROSS_COMPILE_PREFIX}ar
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

find_program(CMAKE_RANLIB
    NAMES ${CROSS_COMPILE_PREFIX}ranlib
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

find_program(CMAKE_STRIP
    NAMES ${CROSS_COMPILE_PREFIX}strip
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

find_program(CMAKE_OBJCOPY
    NAMES ${CROSS_COMPILE_PREFIX}objcopy
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

find_program(CMAKE_OBJDUMP
    NAMES ${CROSS_COMPILE_PREFIX}objdump
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

# Set PKG_CONFIG for cross-compilation
find_program(PKG_CONFIG_EXECUTABLE
    NAMES ${CROSS_COMPILE_PREFIX}pkg-config
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

if(NOT PKG_CONFIG_EXECUTABLE)
    find_program(PKG_CONFIG_EXECUTABLE pkg-config)
    if(PKG_CONFIG_EXECUTABLE)
        # Set PKG_CONFIG environment variables for cross-compilation
        set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig")
        set(ENV{PKG_CONFIG_SYSROOT_DIR} "/")
    endif()
endif()

# Set sysroot if available
set(SYSROOT_PATHS
    "/usr/aarch64-linux-gnu"
    "/opt/cross/aarch64-linux-gnu/sysroot"
    "$ENV{CROSS_COMPILE_ROOT}/aarch64-linux-gnu/sysroot"
)

foreach(SYSROOT_PATH ${SYSROOT_PATHS})
    if(EXISTS "${SYSROOT_PATH}")
        set(CMAKE_SYSROOT "${SYSROOT_PATH}")
        break()
    endif()
endforeach()

if(CMAKE_SYSROOT)
    message(STATUS "Using sysroot: ${CMAKE_SYSROOT}")
else()
    message(WARNING "No sysroot found for ARM64 cross-compilation. Libraries may not be found correctly.")
endif()

# Set compiler flags for ARM64
set(ARM64_COMPILER_FLAGS "-march=armv8-a")
set(ARM64_COMPILER_FLAGS "${ARM64_COMPILER_FLAGS} -mtune=cortex-a72")
set(ARM64_COMPILER_FLAGS "${ARM64_COMPILER_FLAGS} -fPIC")

# Apply flags
set(CMAKE_C_FLAGS_INIT "${ARM64_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${ARM64_COMPILER_FLAGS}")

# Set optimization flags
set(CMAKE_C_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG_INIT "-O0 -g")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g")

# Set library search paths
if(CMAKE_SYSROOT)
    set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
    list(APPEND CMAKE_FIND_ROOT_PATH "/usr/lib/aarch64-linux-gnu")
    list(APPEND CMAKE_FIND_ROOT_PATH "/lib/aarch64-linux-gnu")
else()
    set(CMAKE_FIND_ROOT_PATH "/usr/lib/aarch64-linux-gnu" "/lib/aarch64-linux-gnu")
endif()

# Set find modes
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set install prefix for cross-compilation
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install prefix" FORCE)
endif()

# Qt cross-compilation settings
if(DEFINED ENV{QT_HOST_PATH})
    set(QT_HOST_PATH "$ENV{QT_HOST_PATH}")
    message(STATUS "Using Qt host tools from: ${QT_HOST_PATH}")
endif()

# Set Qt for cross-compilation
if(Qt6_DIR OR DEFINED ENV{Qt6_DIR})
    # Qt6 cross-compilation
    if(QT_HOST_PATH)
        list(PREPEND CMAKE_PREFIX_PATH "${QT_HOST_PATH}")
    endif()
elseif(Qt5_DIR OR DEFINED ENV{Qt5_DIR})
    # Qt5 cross-compilation
    if(QT_HOST_PATH)
        list(PREPEND CMAKE_PREFIX_PATH "${QT_HOST_PATH}")
    endif()
endif()

# ARM64-specific definitions
add_definitions(-D__aarch64__)
add_definitions(-DQTFORGE_ARM64)

# Set thread library preference
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Enable position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set RPATH for cross-compiled binaries
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/../lib/aarch64-linux-gnu")

# Function to help find ARM64 libraries
function(find_arm64_library VAR_NAME LIB_NAME)
    find_library(${VAR_NAME}
        NAMES ${LIB_NAME}
        PATHS
            ${CMAKE_FIND_ROOT_PATH}
            /usr/lib/aarch64-linux-gnu
            /lib/aarch64-linux-gnu
        NO_DEFAULT_PATH
    )
    
    if(NOT ${VAR_NAME})
        find_library(${VAR_NAME} ${LIB_NAME})
    endif()
endfunction()

# Helper function to configure ARM64-specific target properties
function(configure_arm64_target TARGET_NAME)
    # Set ARM64-specific properties
    set_target_properties(${TARGET_NAME} PROPERTIES
        COMPILE_FLAGS "${ARM64_COMPILER_FLAGS}"
    )
    
    # Add ARM64-specific compile definitions
    target_compile_definitions(${TARGET_NAME} PRIVATE
        __aarch64__
        QTFORGE_ARM64
    )
endfunction()

# Print configuration summary
message(STATUS "Linux ARM64 cross-compilation toolchain configuration:")
message(STATUS "  Target: ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  CXX Compiler: ${CMAKE_CXX_COMPILER}")
if(CMAKE_SYSROOT)
    message(STATUS "  Sysroot: ${CMAKE_SYSROOT}")
endif()
if(QT_HOST_PATH)
    message(STATUS "  Qt Host Path: ${QT_HOST_PATH}")
endif()
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")

# Mark as configured
set(QTFORGE_LINUX_ARM64_TOOLCHAIN_CONFIGURED TRUE)
