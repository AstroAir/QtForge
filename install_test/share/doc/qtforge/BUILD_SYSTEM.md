# QtForge Modular Build System

QtForge features a modern, modular CMake build system designed for cross-platform development, easy maintenance, and extensibility. This document provides comprehensive information about the build system architecture, configuration options, and usage.

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [Quick Start](#quick-start)
- [Build Options](#build-options)
- [Platform Support](#platform-support)
- [Cross-Compilation](#cross-compilation)
- [CMake Presets](#cmake-presets)
- [Advanced Configuration](#advanced-configuration)
- [Troubleshooting](#troubleshooting)

## Architecture Overview

The QtForge build system is organized into modular components:

```
cmake/
├── modules/                    # Core build modules
│   ├── QtForgePlatform.cmake   # Platform and architecture detection
│   ├── QtForgeCompiler.cmake   # Compiler detection and configuration
│   ├── QtForgeOptions.cmake    # Build options and feature flags
│   ├── QtForgeDependencies.cmake # Dependency management
│   ├── QtForgeTargets.cmake    # Target creation utilities
│   └── QtForgePackaging.cmake  # Packaging configuration
├── toolchains/                 # Cross-compilation toolchains
│   ├── android.cmake           # Android cross-compilation
│   ├── ios.cmake               # iOS cross-compilation
│   ├── linux-arm64.cmake       # Linux ARM64 cross-compilation
│   ├── windows-arm64.cmake     # Windows ARM64 cross-compilation
│   └── embedded/               # Embedded system toolchains
└── presets/                    # CMake presets for common configurations
```

### Key Features

- **Modular Design**: Separated concerns for better maintainability
- **Platform Detection**: Automatic detection of platform, architecture, and capabilities
- **Compiler Configuration**: Intelligent compiler setup with optimization flags
- **Dependency Management**: Centralized Qt and system dependency handling
- **Cross-Compilation**: Built-in support for multiple target platforms
- **Modern CMake**: Uses CMake 3.21+ features and best practices

## Quick Start

### Basic Build

```bash
# Configure and build with default settings
cmake -S . -B build
cmake --build build

# Install
cmake --install build --prefix /usr/local
```

### Using CMake Presets (Recommended)

```bash
# List available presets
cmake --list-presets

# Configure with a preset
cmake --preset debug

# Build with a preset
cmake --build --preset debug

# Test with a preset
ctest --preset debug
```

### Common Build Configurations

```bash
# Debug build with all features
cmake --preset debug

# Release build with optimizations
cmake --preset release

# Platform-specific builds
cmake --preset windows-msvc    # Windows with MSVC
cmake --preset linux-gcc       # Linux with GCC
cmake --preset macos           # macOS with Xcode
```

## Build Options

The build system provides numerous configuration options:

### Core Options

| Option | Default | Description |
|--------|---------|-------------|
| `QTFORGE_BUILD_SHARED` | ON | Build shared libraries |
| `QTFORGE_BUILD_STATIC` | OFF | Build static libraries |
| `QTFORGE_BUILD_EXAMPLES` | ON | Build example plugins |
| `QTFORGE_BUILD_TESTS` | OFF | Build unit tests |
| `QTFORGE_BUILD_DOCS` | OFF | Build documentation |

### Component Options

| Option | Default | Description |
|--------|---------|-------------|
| `QTFORGE_BUILD_NETWORK` | Auto | Build network plugin support |
| `QTFORGE_BUILD_UI` | Auto | Build UI plugin support |
| `QTFORGE_BUILD_SQL` | Auto | Build SQL plugin support |
| `QTFORGE_BUILD_CONCURRENT` | Auto | Build concurrent plugin support |

### Compiler Options

| Option | Default | Description |
|--------|---------|-------------|
| `QTFORGE_ENABLE_WARNINGS` | ON | Enable comprehensive warnings |
| `QTFORGE_ENABLE_WERROR` | OFF | Treat warnings as errors |
| `QTFORGE_ENABLE_SANITIZERS` | OFF | Enable sanitizers (Debug only) |
| `QTFORGE_ENABLE_LTO` | OFF | Enable Link Time Optimization |
| `QTFORGE_ENABLE_FAST_MATH` | OFF | Enable fast math optimizations |

### Platform-Specific Options

#### Windows
- `QTFORGE_ENABLE_WINDOWS_MANIFEST`: Enable Windows application manifest
- `QTFORGE_ENABLE_WINDOWS_RESOURCES`: Enable Windows resources

#### macOS
- `QTFORGE_ENABLE_MACOS_BUNDLE`: Create macOS application bundles
- `QTFORGE_ENABLE_MACOS_CODESIGN`: Enable macOS code signing

#### Linux
- `QTFORGE_ENABLE_APPIMAGE`: Enable AppImage packaging
- `QTFORGE_ENABLE_FLATPAK`: Enable Flatpak packaging

#### Android
- `QTFORGE_ANDROID_MIN_SDK_VERSION`: Minimum Android SDK version (default: 21)
- `QTFORGE_ANDROID_TARGET_SDK_VERSION`: Target Android SDK version (default: 33)

#### iOS
- `QTFORGE_IOS_DEPLOYMENT_TARGET`: iOS deployment target (default: 12.0)
- `QTFORGE_ENABLE_IOS_SIMULATOR`: Build for iOS Simulator

## Platform Support

### Supported Platforms

| Platform | Architectures | Status | Notes |
|----------|---------------|--------|-------|
| Windows | x86, x64, ARM64 | ✅ Full | MSVC, MinGW, Clang |
| macOS | x64, ARM64 | ✅ Full | Xcode, Clang |
| Linux | x86, x64, ARM, ARM64 | ✅ Full | GCC, Clang |
| Android | ARM, ARM64, x86, x64 | ✅ Full | NDK r21+ |
| iOS | ARM64, x64 (Simulator) | ✅ Full | Xcode required |
| FreeBSD | x64, ARM64 | 🔶 Basic | Community supported |
| Embedded | RISC-V, ARM | 🔶 Basic | Limited Qt support |

### Platform Detection

The build system automatically detects:
- Operating system and version
- CPU architecture and capabilities
- Available compilers and toolchains
- Qt installation and components
- System libraries and dependencies

## Cross-Compilation

### Android

```bash
# Set up Android NDK
export ANDROID_NDK=/path/to/android-ndk

# Configure for Android ARM64
cmake --preset android-arm64

# Build
cmake --build --preset android-arm64
```

### iOS

```bash
# Configure for iOS device
cmake --preset ios

# Configure for iOS simulator
cmake --preset ios -DQTFORGE_ENABLE_IOS_SIMULATOR=ON

# Build
cmake --build --preset ios
```

### Linux ARM64

```bash
# Install cross-compilation tools
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# Configure
cmake --preset cross-linux-arm64

# Build
cmake --build --preset cross-linux-arm64
```

### Embedded RISC-V

```bash
# Set up RISC-V toolchain
export RISCV=/path/to/riscv-toolchain

# Configure
cmake --preset embedded-riscv

# Build
cmake --build --preset embedded-riscv
```

## CMake Presets

The build system includes comprehensive CMake presets for common configurations:

### Configure Presets

- `default`: Default configuration with auto-detection
- `debug`: Debug build with all debugging features
- `release`: Optimized release build
- `windows-msvc`: Windows with MSVC compiler
- `windows-clang`: Windows with Clang compiler
- `linux-gcc`: Linux with GCC compiler
- `linux-clang`: Linux with Clang compiler
- `macos`: macOS with Xcode
- `android-arm64`: Android ARM64 cross-compilation
- `ios`: iOS cross-compilation
- `cross-linux-arm64`: Linux ARM64 cross-compilation
- `embedded-riscv`: Embedded RISC-V cross-compilation

### Build Presets

- `debug`: Build debug configuration
- `release`: Build release configuration
- Platform-specific build presets for each configure preset

### Test Presets

- `debug`: Run tests in debug mode
- `release`: Run tests in release mode

### Package Presets

- `release`: Create release packages
- `windows-release`: Windows-specific packages (NSIS, ZIP, WIX)
- `linux-release`: Linux-specific packages (DEB, RPM, TGZ)
- `macos-release`: macOS-specific packages (DMG, TGZ)

## Advanced Configuration

### Custom Toolchains

Create custom toolchain files in `cmake/toolchains/`:

```cmake
# custom-target.cmake
set(CMAKE_SYSTEM_NAME YourSystem)
set(CMAKE_SYSTEM_PROCESSOR your_arch)

# Set compilers
set(CMAKE_C_COMPILER your-gcc)
set(CMAKE_CXX_COMPILER your-g++)

# Configure flags and paths
# ... your configuration
```

### Custom Build Modules

Extend the build system by creating modules in `cmake/modules/`:

```cmake
# YourModule.cmake
include_guard(GLOBAL)

function(your_custom_function)
    # Your custom build logic
endfunction()
```

### Environment Variables

The build system respects these environment variables:

- `QT_DIR`: Qt installation directory
- `QT_HOST_PATH`: Qt host tools path (for cross-compilation)
- `ANDROID_NDK`: Android NDK path
- `RISCV`: RISC-V toolchain path
- `CROSS_COMPILE_ROOT`: Cross-compilation toolchain root

## Troubleshooting

### Common Issues

#### Qt Not Found
```bash
# Set Qt path explicitly
cmake -DQt6_DIR=/path/to/qt/lib/cmake/Qt6 ...
```

#### Cross-Compilation Failures
```bash
# Verify toolchain installation
which aarch64-linux-gnu-gcc

# Check toolchain file
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-arm64.cmake --debug-output ...
```

#### Build Failures
```bash
# Clean build directory
rm -rf build
cmake --preset debug

# Verbose build output
cmake --build build --verbose
```

### Debug Information

Enable debug output for troubleshooting:

```bash
# CMake debug output
cmake --debug-output --preset debug

# Verbose makefile output
cmake --build build --verbose

# Show configuration summary
cmake -S . -B build -DQTFORGE_ENABLE_WARNINGS=ON
```

### Getting Help

1. Check the [GitHub Issues](https://github.com/qtforge/qtforge/issues)
2. Review the [Documentation](https://qtforge.readthedocs.io/)
3. Join our [Discord Community](https://discord.gg/qtforge)

## Migration from Legacy Build System

See [MIGRATION.md](MIGRATION.md) for detailed migration instructions from the previous build system.
