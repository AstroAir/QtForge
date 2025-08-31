# MSYS2 UCRT64 Toolchain
# For building QtForge with MSYS2 UCRT64 environment (Universal C Runtime)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# MSYS2 environment detection
if(NOT DEFINED ENV{MSYSTEM})
    message(FATAL_ERROR "This toolchain requires MSYS2 environment. Please run from MSYS2 terminal.")
endif()

if(NOT $ENV{MSYSTEM} STREQUAL "UCRT64")
    message(WARNING "This toolchain is optimized for UCRT64. Current MSYSTEM: $ENV{MSYSTEM}")
endif()

# Toolchain configuration
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

# Compiler settings
find_program(CMAKE_C_COMPILER NAMES ${TOOLCHAIN_PREFIX}-gcc gcc)
find_program(CMAKE_CXX_COMPILER NAMES ${TOOLCHAIN_PREFIX}-g++ g++)
find_program(CMAKE_RC_COMPILER NAMES ${TOOLCHAIN_PREFIX}-windres windres)

if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "UCRT64 GCC compiler not found. Please install mingw-w64-ucrt-x86_64-gcc")
endif()

if(NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "UCRT64 G++ compiler not found. Please install mingw-w64-ucrt-x86_64-gcc")
endif()

# Linker and other tools
find_program(CMAKE_AR NAMES ${TOOLCHAIN_PREFIX}-ar ar)
find_program(CMAKE_LINKER NAMES ${TOOLCHAIN_PREFIX}-ld ld)
find_program(CMAKE_NM NAMES ${TOOLCHAIN_PREFIX}-nm nm)
find_program(CMAKE_OBJCOPY NAMES ${TOOLCHAIN_PREFIX}-objcopy objcopy)
find_program(CMAKE_OBJDUMP NAMES ${TOOLCHAIN_PREFIX}-objdump objdump)
find_program(CMAKE_RANLIB NAMES ${TOOLCHAIN_PREFIX}-ranlib ranlib)
find_program(CMAKE_SIZE NAMES ${TOOLCHAIN_PREFIX}-size size)
find_program(CMAKE_STRIP NAMES ${TOOLCHAIN_PREFIX}-strip strip)

# MSYS2 paths
set(MSYS2_ROOT $ENV{MSYSTEM_PREFIX})
set(CMAKE_FIND_ROOT_PATH ${MSYS2_ROOT})

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Compiler flags for UCRT64 (Universal C Runtime)
set(UCRT64_FLAGS "-m64 -march=x86-64 -mtune=generic -D_UCRT")
set(CMAKE_C_FLAGS_INIT "${UCRT64_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${UCRT64_FLAGS}")

# Optimization flags
set(CMAKE_C_FLAGS_RELEASE_INIT "-O3 -DNDEBUG -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -DNDEBUG -ffunction-sections -fdata-sections")

# Debug flags
set(CMAKE_C_FLAGS_DEBUG_INIT "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -O0 -DDEBUG")

# Linker flags for UCRT
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections -lucrt")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-Wl,--gc-sections -lucrt")

# Windows-specific settings
set(CMAKE_SYSTEM_VERSION 10.0)
set(WIN32 TRUE)
set(MINGW TRUE)

# Qt6 settings for MSYS2 UCRT64
set(Qt6_DIR "${MSYS2_ROOT}/lib/cmake/Qt6")
set(QT_QMAKE_EXECUTABLE "${MSYS2_ROOT}/bin/qmake.exe")

# QtForge specific settings for MSYS2 UCRT64
set(QTFORGE_IS_MSYS2 ON)
set(QTFORGE_IS_UCRT64 ON)
set(QTFORGE_TARGET_ARCH "x64")
set(QTFORGE_TARGET_PLATFORM "msys2-ucrt64")

# Enable Windows-specific features
set(QTFORGE_BUILD_SHARED ON CACHE BOOL "Build shared libraries on MSYS2 UCRT64")
set(QTFORGE_BUILD_EXAMPLES ON CACHE BOOL "Build examples on MSYS2 UCRT64")

# Package configuration
set(CPACK_GENERATOR "ZIP;NSIS")
set(CPACK_PACKAGE_ARCHITECTURE "x64")
set(CPACK_SYSTEM_NAME "msys2-ucrt64")

# MSYS2-specific optimizations
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

# Path conversion for MSYS2
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "MSYS")
    # Convert MSYS paths to Windows paths for better compatibility
    string(REGEX REPLACE "^/([a-zA-Z])/" "\\1:/" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
endif()

# Additional MSYS2 package paths
list(APPEND CMAKE_PREFIX_PATH
    "${MSYS2_ROOT}"
    "${MSYS2_ROOT}/lib/cmake"
    "${MSYS2_ROOT}/share/cmake"
)

# PKG_CONFIG settings for MSYS2
set(ENV{PKG_CONFIG_PATH} "${MSYS2_ROOT}/lib/pkgconfig:${MSYS2_ROOT}/share/pkgconfig")

# Set up proper library naming
set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_EXECUTABLE_SUFFIX ".exe")

# MSYS2 UCRT64 runtime dependencies
set(QTFORGE_MSYS2_RUNTIME_DEPS
    "libgcc_s_seh-1.dll"
    "libstdc++-6.dll"
    "libwinpthread-1.dll"
)

# Function to copy MSYS2 runtime dependencies
function(qtforge_copy_msys2_runtime target)
    if(QTFORGE_IS_MSYS2)
        foreach(dep ${QTFORGE_MSYS2_RUNTIME_DEPS})
            find_file(${dep}_PATH ${dep} PATHS ${MSYS2_ROOT}/bin NO_DEFAULT_PATH)
            if(${dep}_PATH)
                add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${${dep}_PATH}
                    $<TARGET_FILE_DIR:${target}>
                    COMMENT "Copying MSYS2 runtime dependency: ${dep}"
                )
            endif()
        endforeach()
    endif()
endfunction()

message(STATUS "QtForge: Using MSYS2 UCRT64 toolchain")
message(STATUS "QtForge: MSYSTEM = $ENV{MSYSTEM}")
message(STATUS "QtForge: MSYS2_ROOT = ${MSYS2_ROOT}")
message(STATUS "QtForge: C Compiler = ${CMAKE_C_COMPILER}")
message(STATUS "QtForge: CXX Compiler = ${CMAKE_CXX_COMPILER}")
