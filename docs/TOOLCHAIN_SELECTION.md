# QtForge Toolchain Selection Guide

This guide explains how to select and configure different toolchains (MSVC, MinGW64) in the QtForge build system.

## Overview

QtForge supports multiple toolchains on Windows:

- **MSVC** (Microsoft Visual C++) - Native Windows toolchain
- **MinGW64** (Minimalist GNU for Windows) - GCC-based toolchain via MSYS2

The build system automatically detects available toolchains and provides intelligent selection with user preferences.

## Quick Start

### Automatic Detection

The simplest approach - let the system choose:

```bash
# Automatic toolchain detection and selection
xmake f -c
```

The system will:

1. Detect available toolchains
2. Check Qt6 installations for each
3. Select the best available option
4. Configure the build accordingly

### Explicit Selection

Choose a specific toolchain:

```bash
# Use MSVC toolchain
xmake f -c --toolchain=msvc

# Use MinGW64 toolchain
xmake f -c --toolchain=mingw64

# Prefer MinGW64 when both are available
xmake f -c --prefer_mingw64=true
```

## Toolchain Detection

### MSVC Detection

The system detects MSVC by checking:

- Visual Studio installations
- Build Tools for Visual Studio
- Windows SDK availability
- Compiler versions (cl.exe)

```bash
# Check MSVC detection
xmake show -l toolchains | grep msvc
```

### MinGW64 Detection

The system detects MinGW64 by checking:

- MSYS2 installation directory
- MinGW64 toolchain availability
- GCC compiler versions
- Qt6 packages via pacman

```bash
# Check MinGW64 detection
xmake show -l toolchains | grep mingw
```

## Configuration Options

### Basic Options

```bash
# Toolchain selection
--toolchain=auto|msvc|mingw64

# Toolchain preference
--prefer_mingw64=true|false

# MSYS2 root directory (if non-standard)
--msys2_root=D:/msys64
```

### Advanced Options

```bash
# Qt6 configuration
--qt6_root=C:/Qt/6.5.0/msvc2022_64
--qt6_tools_root=C:/Qt/Tools

# Build optimizations
--lto=true              # Link-time optimization
--ccache=true           # Compiler cache
--pgo=true              # Profile-guided optimization

# Parallel compilation
--jobs=8                # Number of parallel jobs
```

## Toolchain-Specific Features

### MSVC Toolchain

**Advantages:**

- Native Windows debugging support
- Excellent Visual Studio integration
- Microsoft-specific optimizations
- Windows SDK integration

**Configuration:**

```bash
xmake f -c --toolchain=msvc --mode=release --lto=true
```

**Optimizations:**

- Profile-Guided Optimization (PGO)
- Whole Program Optimization (/GL)
- Link-Time Code Generation (/LTCG)
- Advanced vectorization (/arch:AVX2)

### MinGW64 Toolchain

**Advantages:**

- Open-source GCC-based toolchain
- POSIX compatibility
- Advanced GCC optimizations
- MSYS2 package management

**Configuration:**

```bash
xmake f -c --toolchain=mingw64 --mode=release --msys2_root=D:/msys64
```

**Optimizations:**

- CPU-specific optimization (-march=native)
- Advanced vectorization
- Inter-procedural optimization
- Loop optimizations

## Qt6 Integration

### MSVC + Qt6

```bash
# Configure with MSVC and Qt6
xmake f -c --toolchain=msvc --qt6_root=C:/Qt/6.5.0/msvc2022_64

# The system will automatically:
# - Detect Qt6 tools (moc, uic, rcc)
# - Configure library paths
# - Set up proper linking
```

### MinGW64 + Qt6

```bash
# Configure with MinGW64 and MSYS2 Qt6
xmake f -c --toolchain=mingw64 --msys2_root=D:/msys64

# Install Qt6 via MSYS2 if needed:
# pacman -S mingw-w64-x86_64-qt6-base
# pacman -S mingw-w64-x86_64-qt6-tools
```

## Environment Setup

### MSYS2 MinGW64 Setup

1. **Install MSYS2:**

```bash
# Download from https://www.msys2.org/
# Install to D:/msys64 (recommended)
```

2. **Install MinGW64 toolchain:**

```bash
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-ninja
```

3. **Install Qt6:**

```bash
pacman -S mingw-w64-x86_64-qt6-base
pacman -S mingw-w64-x86_64-qt6-tools
pacman -S mingw-w64-x86_64-qt6-declarative
```

4. **Configure environment:**

```bash
# Add to PATH (or let xmake detect automatically)
export PATH="/d/msys64/mingw64/bin:$PATH"
```

### MSVC Setup

1. **Install Visual Studio or Build Tools:**
   - Visual Studio 2019/2022 Community/Professional
   - Build Tools for Visual Studio 2019/2022

2. **Install Qt6:**
   - Download from Qt website
   - Install MSVC-compatible version
   - Note installation path for configuration

## Build Modes and Optimization

### Debug Mode

```bash
# Debug with MSVC
xmake f -c --toolchain=msvc --mode=debug
xmake

# Debug with MinGW64
xmake f -c --toolchain=mingw64 --mode=debug
xmake
```

### Release Mode

```bash
# Optimized release with MSVC
xmake f -c --toolchain=msvc --mode=release --lto=true
xmake

# Optimized release with MinGW64
xmake f -c --toolchain=mingw64 --mode=release --lto=true
xmake
```

### Profile-Guided Optimization

```bash
# Step 1: Build instrumented version
xmake f -c --toolchain=msvc --mode=release --pgo_instrument=true
xmake

# Step 2: Run application to generate profile data
xmake run your_target

# Step 3: Build optimized version
xmake f -c --toolchain=msvc --mode=release --pgo_optimize=true
xmake
```

## Troubleshooting

### Common Issues

1. **Toolchain Not Detected**

```bash
# Check available toolchains
xmake show -l toolchains

# Verify installation paths
xmake f -c --verbose
```

2. **Qt6 Not Found**

```bash
# Specify Qt6 root explicitly
xmake f -c --qt6_root=C:/Qt/6.5.0/msvc2022_64

# For MSYS2, ensure packages are installed
pacman -Qs qt6
```

3. **Build Failures**

```bash
# Clean and reconfigure
xmake f -c
xmake clean
xmake -v
```

### Validation

```bash
# Validate toolchain configuration
lua scripts/validate_build_system.lua

# Test dual toolchain support
xmake f -c --toolchain=msvc && xmake
xmake f -c --toolchain=mingw64 && xmake
```

## Best Practices

### Development Workflow

1. **Use MSVC for Windows-specific development**
2. **Use MinGW64 for cross-platform compatibility**
3. **Test with both toolchains before release**
4. **Enable optimizations for release builds**

### Performance Optimization

```bash
# Maximum performance build
xmake f -c --toolchain=msvc --mode=release --lto=true --pgo=true --ccache=true
```

### CI/CD Integration

```yaml
# GitHub Actions example
- name: Configure MSVC
  run: xmake f -c --toolchain=msvc --mode=release

- name: Configure MinGW64
  run: xmake f -c --toolchain=mingw64 --mode=release --msys2_root=D:/msys64
```

## Advanced Configuration

### Custom Toolchain Paths

```lua
-- In xmake.lua
if has_config("custom_msvc_path") then
    set_toolchains("msvc", {
        vcvars = get_config("custom_msvc_path") .. "/vcvars64.bat"
    })
end
```

### Toolchain-Specific Settings

```lua
-- In your target configuration
if is_toolchain("msvc") then
    add_cxxflags("/W4", "/WX")
    add_defines("MSVC_BUILD")
elseif is_toolchain("mingw64") then
    add_cxxflags("-Wall", "-Wextra", "-Werror")
    add_defines("MINGW_BUILD")
end
```

This guide should help you effectively select and configure toolchains for QtForge development!
