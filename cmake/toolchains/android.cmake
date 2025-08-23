# Android toolchain file for QtForge
# Cross-compilation toolchain for Android platforms

# Ensure we're cross-compiling
set(CMAKE_SYSTEM_NAME Android)

# Set minimum required CMake version
cmake_minimum_required(VERSION 3.21)

# Android NDK path detection
if(NOT ANDROID_NDK)
    if(DEFINED ENV{ANDROID_NDK_ROOT})
        set(ANDROID_NDK $ENV{ANDROID_NDK_ROOT})
    elseif(DEFINED ENV{ANDROID_NDK})
        set(ANDROID_NDK $ENV{ANDROID_NDK})
    elseif(DEFINED ENV{ANDROID_NDK_HOME})
        set(ANDROID_NDK $ENV{ANDROID_NDK_HOME})
    else()
        message(FATAL_ERROR "Android NDK not found. Please set ANDROID_NDK, ANDROID_NDK_ROOT, or ANDROID_NDK_HOME environment variable")
    endif()
endif()

# Validate Android NDK
if(NOT EXISTS "${ANDROID_NDK}")
    message(FATAL_ERROR "Android NDK path does not exist: ${ANDROID_NDK}")
endif()

# Set Android API level (default to 21 for 64-bit support)
if(NOT ANDROID_PLATFORM)
    set(ANDROID_PLATFORM android-21)
endif()

# Extract API level from platform string
string(REGEX MATCH "[0-9]+" ANDROID_API_LEVEL "${ANDROID_PLATFORM}")
if(ANDROID_API_LEVEL LESS 21)
    message(WARNING "Android API level ${ANDROID_API_LEVEL} is below recommended minimum of 21")
endif()

# Set Android ABI (default to arm64-v8a)
if(NOT ANDROID_ABI)
    set(ANDROID_ABI arm64-v8a)
endif()

# Validate Android ABI
set(SUPPORTED_ANDROID_ABIS
    armeabi-v7a
    arm64-v8a
    x86
    x86_64
)

if(NOT ANDROID_ABI IN_LIST SUPPORTED_ANDROID_ABIS)
    message(FATAL_ERROR "Unsupported Android ABI: ${ANDROID_ABI}. Supported ABIs: ${SUPPORTED_ANDROID_ABIS}")
endif()

# Set architecture-specific variables
if(ANDROID_ABI STREQUAL "armeabi-v7a")
    set(CMAKE_SYSTEM_PROCESSOR armv7-a)
    set(ANDROID_ARCH_NAME arm)
    set(ANDROID_LLVM_TRIPLE armv7-none-linux-androideabi)
elseif(ANDROID_ABI STREQUAL "arm64-v8a")
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    set(ANDROID_ARCH_NAME arm64)
    set(ANDROID_LLVM_TRIPLE aarch64-none-linux-android)
elseif(ANDROID_ABI STREQUAL "x86")
    set(CMAKE_SYSTEM_PROCESSOR i686)
    set(ANDROID_ARCH_NAME x86)
    set(ANDROID_LLVM_TRIPLE i686-none-linux-android)
elseif(ANDROID_ABI STREQUAL "x86_64")
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
    set(ANDROID_ARCH_NAME x86_64)
    set(ANDROID_LLVM_TRIPLE x86_64-none-linux-android)
endif()

# Set Android STL (default to c++_shared)
if(NOT ANDROID_STL)
    set(ANDROID_STL c++_shared)
endif()

# Validate Android STL
set(SUPPORTED_ANDROID_STLS
    c++_shared
    c++_static
    system
)

if(NOT ANDROID_STL IN_LIST SUPPORTED_ANDROID_STLS)
    message(FATAL_ERROR "Unsupported Android STL: ${ANDROID_STL}. Supported STLs: ${SUPPORTED_ANDROID_STLS}")
endif()

# Set toolchain paths
set(ANDROID_TOOLCHAIN_ROOT "${ANDROID_NDK}/toolchains/llvm/prebuilt")

# Detect host system for toolchain
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(ANDROID_HOST_TAG linux-x86_64)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(ANDROID_HOST_TAG darwin-x86_64)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(ANDROID_HOST_TAG windows-x86_64)
else()
    message(FATAL_ERROR "Unsupported host system: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

set(ANDROID_TOOLCHAIN_PREFIX "${ANDROID_TOOLCHAIN_ROOT}/${ANDROID_HOST_TAG}")

if(NOT EXISTS "${ANDROID_TOOLCHAIN_PREFIX}")
    message(FATAL_ERROR "Android toolchain not found: ${ANDROID_TOOLCHAIN_PREFIX}")
endif()

# Set compiler paths
set(CMAKE_C_COMPILER "${ANDROID_TOOLCHAIN_PREFIX}/bin/clang")
set(CMAKE_CXX_COMPILER "${ANDROID_TOOLCHAIN_PREFIX}/bin/clang++")
set(CMAKE_ASM_COMPILER "${ANDROID_TOOLCHAIN_PREFIX}/bin/clang")

# Set archiver and other tools
set(CMAKE_AR "${ANDROID_TOOLCHAIN_PREFIX}/bin/llvm-ar" CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB "${ANDROID_TOOLCHAIN_PREFIX}/bin/llvm-ranlib" CACHE FILEPATH "Ranlib")
set(CMAKE_STRIP "${ANDROID_TOOLCHAIN_PREFIX}/bin/llvm-strip" CACHE FILEPATH "Strip")

# Set sysroot
set(CMAKE_SYSROOT "${ANDROID_TOOLCHAIN_PREFIX}/sysroot")

# Set compiler and linker flags
set(ANDROID_COMPILER_FLAGS "-target ${ANDROID_LLVM_TRIPLE}${ANDROID_API_LEVEL}")
set(ANDROID_COMPILER_FLAGS "${ANDROID_COMPILER_FLAGS} -fdata-sections -ffunction-sections")
set(ANDROID_COMPILER_FLAGS "${ANDROID_COMPILER_FLAGS} -funwind-tables -fstack-protector-strong")
set(ANDROID_COMPILER_FLAGS "${ANDROID_COMPILER_FLAGS} -no-canonical-prefixes")

# ABI-specific flags
if(ANDROID_ABI STREQUAL "armeabi-v7a")
    set(ANDROID_COMPILER_FLAGS "${ANDROID_COMPILER_FLAGS} -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16")
    set(ANDROID_LINKER_FLAGS "-march=armv7-a -Wl,--fix-cortex-a8")
elseif(ANDROID_ABI STREQUAL "arm64-v8a")
    set(ANDROID_COMPILER_FLAGS "${ANDROID_COMPILER_FLAGS}")
    set(ANDROID_LINKER_FLAGS "")
elseif(ANDROID_ABI STREQUAL "x86")
    set(ANDROID_COMPILER_FLAGS "${ANDROID_COMPILER_FLAGS}")
    set(ANDROID_LINKER_FLAGS "")
elseif(ANDROID_ABI STREQUAL "x86_64")
    set(ANDROID_COMPILER_FLAGS "${ANDROID_COMPILER_FLAGS}")
    set(ANDROID_LINKER_FLAGS "")
endif()

# Set STL flags
if(ANDROID_STL STREQUAL "c++_shared")
    set(ANDROID_STL_FLAGS "-stdlib=libc++")
elseif(ANDROID_STL STREQUAL "c++_static")
    set(ANDROID_STL_FLAGS "-stdlib=libc++ -static-libstdc++")
elseif(ANDROID_STL STREQUAL "system")
    set(ANDROID_STL_FLAGS "-stdlib=libstdc++")
endif()

# Apply flags
set(CMAKE_C_FLAGS_INIT "${ANDROID_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${ANDROID_COMPILER_FLAGS} ${ANDROID_STL_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${ANDROID_COMPILER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${ANDROID_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${ANDROID_LINKER_FLAGS}")

# Set library search paths
set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Android-specific definitions
add_definitions(-DANDROID)
add_definitions(-D__ANDROID_API__=${ANDROID_API_LEVEL})

# Set install prefix for Android
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install prefix" FORCE)
endif()

# Qt for Android specific settings
if(Qt6_DIR OR Qt5_DIR)
    # Qt Android deployment settings
    set(QT_ANDROID_APPLICATION_BINARY "qtforge-android")
    set(QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android")
    
    # Android manifest template
    if(EXISTS "${CMAKE_SOURCE_DIR}/android/AndroidManifest.xml.in")
        set(QT_ANDROID_MANIFEST_TEMPLATE "${CMAKE_SOURCE_DIR}/android/AndroidManifest.xml.in")
    endif()
endif()

# Print configuration summary
message(STATUS "Android toolchain configuration:")
message(STATUS "  NDK: ${ANDROID_NDK}")
message(STATUS "  Platform: ${ANDROID_PLATFORM} (API ${ANDROID_API_LEVEL})")
message(STATUS "  ABI: ${ANDROID_ABI}")
message(STATUS "  Architecture: ${ANDROID_ARCH_NAME}")
message(STATUS "  STL: ${ANDROID_STL}")
message(STATUS "  Toolchain: ${ANDROID_TOOLCHAIN_PREFIX}")

# Mark as configured
set(QTFORGE_ANDROID_TOOLCHAIN_CONFIGURED TRUE)
