# RISC-V embedded toolchain file for QtForge
# Cross-compilation toolchain for RISC-V embedded systems

# Ensure we're cross-compiling
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# Set minimum required CMake version
cmake_minimum_required(VERSION 3.21)

# RISC-V toolchain configuration
set(RISCV_TOOLCHAIN_PREFIX "riscv64-unknown-elf-")

# Try to find the RISC-V toolchain
set(TOOLCHAIN_PATHS
    "/opt/riscv/bin"
    "/usr/local/riscv/bin"
    "/usr/bin"
    "$ENV{RISCV}/bin"
    "$ENV{RISCV_TOOLCHAIN_ROOT}/bin"
)

# Find the cross-compiler
find_program(CMAKE_C_COMPILER
    NAMES ${RISCV_TOOLCHAIN_PREFIX}gcc
    PATHS ${TOOLCHAIN_PATHS}
    NO_DEFAULT_PATH
)

find_program(CMAKE_CXX_COMPILER
    NAMES ${RISCV_TOOLCHAIN_PREFIX}g++
    PATHS ${TOOLCHAIN_PATHS}
    NO_DEFAULT_PATH
)

# Fallback to system PATH if not found in specific paths
if(NOT CMAKE_C_COMPILER)
    find_program(CMAKE_C_COMPILER ${RISCV_TOOLCHAIN_PREFIX}gcc)
endif()

if(NOT CMAKE_CXX_COMPILER)
    find_program(CMAKE_CXX_COMPILER ${RISCV_TOOLCHAIN_PREFIX}g++)
endif()

# Check if compilers were found
if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "RISC-V cross-compiler not found. Please install riscv64-unknown-elf-gcc or set RISCV/RISCV_TOOLCHAIN_ROOT environment variable")
endif()

if(NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "RISC-V cross-compiler not found. Please install riscv64-unknown-elf-g++ or set RISCV/RISCV_TOOLCHAIN_ROOT environment variable")
endif()

# Get toolchain directory
get_filename_component(TOOLCHAIN_DIR ${CMAKE_C_COMPILER} DIRECTORY)

# Set other tools
find_program(CMAKE_AR
    NAMES ${RISCV_TOOLCHAIN_PREFIX}ar
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

find_program(CMAKE_RANLIB
    NAMES ${RISCV_TOOLCHAIN_PREFIX}ranlib
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

find_program(CMAKE_STRIP
    NAMES ${RISCV_TOOLCHAIN_PREFIX}strip
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

find_program(CMAKE_OBJCOPY
    NAMES ${RISCV_TOOLCHAIN_PREFIX}objcopy
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

find_program(CMAKE_OBJDUMP
    NAMES ${RISCV_TOOLCHAIN_PREFIX}objdump
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

find_program(CMAKE_SIZE
    NAMES ${RISCV_TOOLCHAIN_PREFIX}size
    PATHS ${TOOLCHAIN_DIR}
    NO_DEFAULT_PATH
)

# RISC-V architecture configuration
if(NOT RISCV_ARCH)
    set(RISCV_ARCH "rv64gc")  # Default to RV64GC (64-bit with standard extensions)
endif()

if(NOT RISCV_ABI)
    set(RISCV_ABI "lp64d")    # Default to LP64D ABI
endif()

# Set RISC-V specific compiler flags
set(RISCV_COMPILER_FLAGS "-march=${RISCV_ARCH}")
set(RISCV_COMPILER_FLAGS "${RISCV_COMPILER_FLAGS} -mabi=${RISCV_ABI}")
set(RISCV_COMPILER_FLAGS "${RISCV_COMPILER_FLAGS} -mcmodel=medany")
set(RISCV_COMPILER_FLAGS "${RISCV_COMPILER_FLAGS} -ffunction-sections")
set(RISCV_COMPILER_FLAGS "${RISCV_COMPILER_FLAGS} -fdata-sections")

# Embedded system specific flags
set(RISCV_COMPILER_FLAGS "${RISCV_COMPILER_FLAGS} -nostartfiles")
set(RISCV_COMPILER_FLAGS "${RISCV_COMPILER_FLAGS} -nostdlib")
set(RISCV_COMPILER_FLAGS "${RISCV_COMPILER_FLAGS} -ffreestanding")

# Apply flags
set(CMAKE_C_FLAGS_INIT "${RISCV_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${RISCV_COMPILER_FLAGS}")

# Set optimization flags
set(CMAKE_C_FLAGS_RELEASE_INIT "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG_INIT "-Og -g")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Og -g")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-Wl,--gc-sections")

# Set library search paths
set(RISCV_SYSROOT_PATHS
    "${TOOLCHAIN_DIR}/../riscv64-unknown-elf"
    "${TOOLCHAIN_DIR}/../lib/gcc/riscv64-unknown-elf"
    "/opt/riscv/riscv64-unknown-elf"
)

foreach(SYSROOT_PATH ${RISCV_SYSROOT_PATHS})
    if(EXISTS "${SYSROOT_PATH}")
        set(CMAKE_SYSROOT "${SYSROOT_PATH}")
        break()
    endif()
endforeach()

if(CMAKE_SYSROOT)
    set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
    message(STATUS "Using RISC-V sysroot: ${CMAKE_SYSROOT}")
else()
    message(WARNING "No sysroot found for RISC-V cross-compilation")
endif()

# Set find modes
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# RISC-V embedded specific definitions
add_definitions(-D__riscv)
add_definitions(-D__riscv_xlen=64)
add_definitions(-DQTFORGE_RISCV)
add_definitions(-DQTFORGE_EMBEDDED)

# Disable shared libraries for embedded systems
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# Set install prefix for embedded
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install prefix" FORCE)
endif()

# Embedded system configuration
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# Helper function for RISC-V embedded target configuration
function(configure_riscv_embedded_target TARGET_NAME)
    # Set RISC-V embedded specific properties
    set_target_properties(${TARGET_NAME} PROPERTIES
        SUFFIX ".elf"
    )
    
    # Add RISC-V embedded specific compile definitions
    target_compile_definitions(${TARGET_NAME} PRIVATE
        __riscv
        __riscv_xlen=64
        QTFORGE_RISCV
        QTFORGE_EMBEDDED
    )
    
    # Add custom commands for binary generation
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.hex
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TARGET_NAME}>
        COMMENT "Generating binary and hex files for ${TARGET_NAME}"
    )
endfunction()

# Function to help find RISC-V libraries
function(find_riscv_library VAR_NAME LIB_NAME)
    find_library(${VAR_NAME}
        NAMES ${LIB_NAME}
        PATHS
            ${CMAKE_FIND_ROOT_PATH}
            ${CMAKE_SYSROOT}/lib
        NO_DEFAULT_PATH
    )
    
    if(NOT ${VAR_NAME})
        find_library(${VAR_NAME} ${LIB_NAME})
    endif()
endfunction()

# Disable Qt for embedded systems (typically not available)
set(QTFORGE_BUILD_NETWORK OFF CACHE BOOL "Network support not available on embedded RISC-V" FORCE)
set(QTFORGE_BUILD_UI OFF CACHE BOOL "UI support not available on embedded RISC-V" FORCE)
set(QTFORGE_BUILD_EXAMPLES OFF CACHE BOOL "Examples disabled for embedded RISC-V" FORCE)
set(QTFORGE_BUILD_TESTS OFF CACHE BOOL "Tests disabled for embedded RISC-V" FORCE)

# Print configuration summary
message(STATUS "RISC-V embedded toolchain configuration:")
message(STATUS "  Target: ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Architecture: ${RISCV_ARCH}")
message(STATUS "  ABI: ${RISCV_ABI}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  CXX Compiler: ${CMAKE_CXX_COMPILER}")
if(CMAKE_SYSROOT)
    message(STATUS "  Sysroot: ${CMAKE_SYSROOT}")
endif()
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")

# Mark as configured
set(QTFORGE_RISCV_EMBEDDED_TOOLCHAIN_CONFIGURED TRUE)
