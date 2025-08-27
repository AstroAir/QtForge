# Migration Guide: Legacy to Modular Build System

This guide helps you migrate from the legacy QtPlugin build system to the new modular QtForge build system.

## Overview of Changes

### Project Rename
- **Old**: QtPlugin
- **New**: QtForge

### Build System Changes
- **Modular Architecture**: Build logic split into reusable modules
- **Enhanced Platform Support**: Better cross-compilation and platform detection
- **Modern CMake**: Uses CMake 3.21+ features and presets
- **Improved Options**: More granular build configuration options

## Quick Migration Steps

### 1. Update Build Options

| Legacy Option | New Option | Notes |
|---------------|------------|-------|
| `QTPLUGIN_BUILD_NETWORK` | `QTFORGE_BUILD_NETWORK` | Auto-detected by default |
| `QTPLUGIN_BUILD_UI` | `QTFORGE_BUILD_UI` | Auto-detected by default |
| `QTPLUGIN_BUILD_EXAMPLES` | `QTFORGE_BUILD_EXAMPLES` | Same default (ON) |
| `QTPLUGIN_BUILD_TESTS` | `QTFORGE_BUILD_TESTS` | Same default (OFF) |

### 2. Update Build Commands

#### Legacy Build
```bash
mkdir build && cd build
cmake .. -DQTPLUGIN_BUILD_TESTS=ON
cmake --build .
```

#### New Build (Presets - Recommended)
```bash
cmake --preset debug
cmake --build --preset debug
```

## API Compatibility

The plugin API remains backward compatible. Only build system changes are required:

```cpp
#include <qtplugin/core/plugin_interface.hpp>  // Still works

class MyPlugin : public QtPlugin::PluginInterface {
    // Implementation unchanged
};
```

## New Features Available

### 1. CMake Presets
```bash
cmake --list-presets
cmake --preset release
```

### 2. Enhanced Cross-Compilation
```bash
cmake --preset android-arm64
cmake --preset ios
cmake --preset cross-linux-arm64
```

### 3. Advanced Compiler Options
```bash
cmake --preset debug -DQTFORGE_ENABLE_SANITIZERS=ON
```

For detailed migration instructions, see [BUILD_SYSTEM.md](BUILD_SYSTEM.md).

