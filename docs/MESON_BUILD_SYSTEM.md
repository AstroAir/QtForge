# QtForge Meson Build System

This document provides comprehensive information about building QtForge using the Meson build system.

## Overview

QtForge now supports Meson as a modern, fast, and user-friendly build system alongside the existing CMake and XMake systems. Meson provides excellent Qt6 integration, cross-platform support, and fast build times.

## Prerequisites

### Required Dependencies
- **Meson** >= 0.63.0
- **Ninja** (recommended backend)
- **Qt6** >= 6.0.0 with Core module
- **C++20 compatible compiler**

### Optional Dependencies
- **Qt6 Network** - for network plugin support
- **Qt6 Widgets** - for UI plugin support  
- **Qt6 Sql** - for database plugin support
- **Qt6 Concurrent** - for concurrent processing support
- **Qt6 StateMachine** - for state machine support
- **Python3** + **pybind11** - for Python bindings
- **Doxygen** - for documentation generation

## Quick Start

### Basic Build
```bash
# Configure the build
meson setup builddir

# Compile the project
meson compile -C builddir

# Run tests (if enabled)
meson test -C builddir

# Install the project
meson install -C builddir
```

### Build with Options
```bash
# Configure with custom options
meson setup builddir \
  -Dbuild_examples=true \
  -Dbuild_tests=true \
  -Dpython_bindings=true \
  -Dbuild_docs=true

# Build everything
meson compile -C builddir
```

## Build Options

### Core Options
- `default_library` - Library type: `shared`, `static`, or `both` (default: `shared`)
- `buildtype` - Build type: `debug`, `release`, `debugoptimized`, `minsize` (default: `release`)
- `warning_level` - Warning level: `0`, `1`, `2`, `3` (default: `3`)

### Component Options
- `build_examples` - Build example applications (default: `true`)
- `build_tests` - Build unit tests (default: `false`)
- `build_docs` - Build documentation (default: `false`)
- `python_bindings` - Build Python bindings (default: `false`)

### Qt Component Options
- `force_qt_network` - Force enable Qt Network (default: `false`)
- `force_qt_widgets` - Force enable Qt Widgets (default: `false`)
- `force_qt_sql` - Force enable Qt SQL (default: `false`)
- `force_qt_concurrent` - Force enable Qt Concurrent (default: `false`)
- `force_qt_statemachine` - Force enable Qt StateMachine (default: `false`)

### Advanced Options
- `qt_method` - Qt detection method: `auto`, `pkgconfig`, `qmake`, `cmake` (default: `auto`)
- `enable_warnings` - Enable compiler warnings (default: `true`)
- `enable_werror` - Treat warnings as errors (default: `false`)
- `enable_sanitizers` - Enable sanitizers (default: `false`)
- `enable_lto` - Enable Link Time Optimization (default: `false`)

### Feature Options
- `enable_hot_reload` - Enable plugin hot reload (default: `true`)
- `enable_metrics` - Enable plugin metrics (default: `true`)
- `enable_transactions` - Enable plugin transactions (default: `true`)
- `enable_orchestration` - Enable plugin orchestration (default: `true`)
- `enable_composition` - Enable plugin composition (default: `true`)
- `enable_marketplace` - Enable plugin marketplace (default: `true`)

## Configuration Examples

### Development Build
```bash
meson setup builddir \
  -Dbuildtype=debug \
  -Dbuild_examples=true \
  -Dbuild_tests=true \
  -Denable_warnings=true \
  -Denable_sanitizers=true
```

### Release Build
```bash
meson setup builddir \
  -Dbuildtype=release \
  -Denable_lto=true \
  -Dbuild_docs=true
```

### Python Development Build
```bash
meson setup builddir \
  -Dpython_bindings=true \
  -Dbuild_examples=true \
  -Dbuild_tests=true
```

### Minimal Build
```bash
meson setup builddir \
  -Dbuild_examples=false \
  -Dbuild_tests=false \
  -Denable_hot_reload=false \
  -Denable_metrics=false \
  -Denable_marketplace=false
```

## Cross-Compilation

### Windows (MinGW)
```bash
meson setup builddir --cross-file cross/mingw-w64.txt
```

### Linux ARM64
```bash
meson setup builddir --cross-file cross/linux-arm64.txt
```

## Installation

### System Installation
```bash
meson install -C builddir
```

### Custom Installation Prefix
```bash
meson setup builddir --prefix=/opt/qtforge
meson install -C builddir
```

### Installation Components
- **Runtime**: Core libraries and executables
- **Development**: Headers and CMake/pkg-config files
- **Documentation**: API documentation and guides
- **Examples**: Example source code and binaries
- **Python**: Python bindings and modules

## Packaging

### Create Distribution Package
```bash
# Configure with packaging
meson setup builddir -Dpackage_format=tar.xz

# Build package
meson compile -C builddir package
```

### Supported Package Formats
- `zip` - ZIP archive
- `tar.gz` - Gzipped tar archive
- `tar.xz` - XZ compressed tar archive

## Integration with IDEs

### Visual Studio Code
1. Install the Meson extension
2. Open the project folder
3. Use Ctrl+Shift+P → "Meson: Configure"

### Qt Creator
1. Open CMakeLists.txt (for compatibility)
2. Or use "Plain C++ Project" with custom build steps

### CLion
1. Use "Makefile Project" 
2. Configure custom build targets for Meson

## Troubleshooting

### Qt6 Not Found
```bash
# Specify Qt6 installation path
export PKG_CONFIG_PATH=/path/to/qt6/lib/pkgconfig:$PKG_CONFIG_PATH
meson setup builddir
```

### MOC Issues
- Ensure Qt6 tools (moc, uic, rcc) are in PATH
- Check that headers with Q_OBJECT are properly listed

### Python Bindings Issues
```bash
# Install pybind11
pip install pybind11

# Configure with explicit Python
meson setup builddir -Dpython_bindings=true
```

### Build Performance
```bash
# Use Ninja backend (default)
meson setup builddir --backend=ninja

# Parallel builds
meson compile -C builddir -j$(nproc)
```

## Comparison with Other Build Systems

| Feature | Meson | CMake | XMake |
|---------|-------|-------|-------|
| Qt6 Integration | Excellent | Good | Good |
| Build Speed | Fast | Medium | Fast |
| Syntax | Clean | Complex | Clean |
| Cross-compilation | Excellent | Good | Good |
| IDE Support | Good | Excellent | Medium |

## Migration Guide

See [MESON_MIGRATION.md](MESON_MIGRATION.md) for detailed migration instructions from CMake/XMake to Meson.
