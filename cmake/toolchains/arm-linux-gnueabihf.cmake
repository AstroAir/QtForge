# ARM Linux GNU EABI Hard Float Toolchain
# For cross-compiling QtForge to ARM-based embedded Linux systems

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Cross-compilation toolchain prefix
set(CROSS_COMPILE_PREFIX arm-linux-gnueabihf)

# Toolchain paths - adjust these based on your toolchain installation
set(TOOLCHAIN_ROOT /usr/${CROSS_COMPILE_PREFIX})
set(TOOLCHAIN_BIN_DIR /usr/bin)

# Compiler settings
set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${CROSS_COMPILE_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${CROSS_COMPILE_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/${CROSS_COMPILE_PREFIX}-gcc)

# Linker and other tools
set(CMAKE_AR ${TOOLCHAIN_BIN_DIR}/${CROSS_COMPILE_PREFIX}-ar)
set(CMAKE_LINKER ${TOOLCHAIN_BIN_DIR}/${CROSS_COMPILE_PREFIX}-ld)
set(CMAKE_NM ${TOOLCHAIN_BIN_DIR}/${CROSS_COMPILE_PREFIX}-nm)
set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${CROSS_COMPILE_PREFIX}-objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/${CROSS_COMPILE_PREFIX}-objdump)
set(CMAKE_RANLIB ${TOOLCHAIN_BIN_DIR}/${CROSS_COMPILE_PREFIX}-ranlib)
set(CMAKE_SIZE ${TOOLCHAIN_BIN_DIR}/${CROSS_COMPILE_PREFIX}-size)
set(CMAKE_STRIP ${TOOLCHAIN_BIN_DIR}/${CROSS_COMPILE_PREFIX}-strip)

# Target environment
set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_ROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Compiler flags for ARM Cortex-A series
set(ARM_CPU_FLAGS "-mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard")
set(CMAKE_C_FLAGS_INIT "${ARM_CPU_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS_INIT "${ARM_CPU_FLAGS} -fPIC")

# Optimization flags
set(CMAKE_C_FLAGS_RELEASE_INIT "-O3 -DNDEBUG -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -DNDEBUG -ffunction-sections -fdata-sections")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-Wl,--gc-sections")

# Qt6 cross-compilation settings
set(QT_QMAKE_EXECUTABLE ${TOOLCHAIN_ROOT}/bin/qmake)
set(Qt6_DIR ${TOOLCHAIN_ROOT}/lib/cmake/Qt6)

# QtForge specific settings for embedded targets
set(QTFORGE_EMBEDDED_TARGET ON)
set(QTFORGE_TARGET_ARCH "arm")
set(QTFORGE_TARGET_PLATFORM "linux-embedded")

# Disable features not suitable for embedded systems
set(QTFORGE_BUILD_EXAMPLES OFF CACHE BOOL "Disable examples for embedded builds")
set(QTFORGE_BUILD_TESTS OFF CACHE BOOL "Disable tests for embedded builds")
set(QTFORGE_ENABLE_COMPONENT_LOGGING OFF CACHE BOOL "Disable verbose logging for embedded")

# Enable optimizations for embedded systems
set(QTFORGE_ENABLE_LTO ON CACHE BOOL "Enable LTO for smaller binaries")
set(QTFORGE_OPTIMIZE_SIZE ON CACHE BOOL "Optimize for size over speed")

# Package configuration for embedded deployment
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_ARCHITECTURE "armhf")
