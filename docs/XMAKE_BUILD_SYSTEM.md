# QtForge XMake Build System

## Overview

QtForge now supports XMake as an alternative build system alongside the existing CMake system. XMake provides a simpler, more maintainable configuration with built-in Qt support and cross-platform compatibility.

## Features

- **Cross-platform support**: Windows, Linux, macOS
- **Qt6 integration**: Automatic Qt detection and framework linking
- **Modular architecture**: Conditional compilation based on available Qt components
- **Multiple build modes**: Debug, Release, RelWithDebInfo, MinSizeRel
- **Optional components**: Network, Widgets, SQL, Concurrent modules
- **Python bindings**: Optional pybind11 integration
- **Examples and tests**: Comprehensive example projects and test suite

## Prerequisites

### Required
- **XMake 3.0.1+**: Cross-platform build utility
- **Qt6**: Qt6Core (minimum requirement)
- **C++20 compiler**: GCC 15.2+, Clang 15+, or MSVC 2022+

### Optional
- **Qt6Network**: For network-dependent features
- **Qt6Widgets**: For UI components
- **Qt6SQL**: For database functionality
- **Python 3.13+**: For Python bindings
- **pybind11**: For Python integration

## Quick Start

### 1. Install XMake
```bash
# Windows (PowerShell)
Invoke-Expression (Invoke-WebRequest 'https://xmake.io/psget.text' -UseBasicParsing).Content

# Linux/macOS
curl -fsSL https://xmake.io/shget.text | bash
```

### 2. Configure Project
```bash
# Basic configuration
xmake f

# With specific toolchain (recommended for Windows with Qt from MinGW)
xmake f --toolchain=mingw

# Enable optional features
xmake f --tests=y --examples=y --python_bindings=y
```

### 3. Build
```bash
# Build all targets
xmake build

# Build specific target
xmake build QtForgeCore

# Build with verbose output
xmake build -v
```

### 4. Run Tests
```bash
# Run configuration test
xmake run xmake_config_test

# Run simple example
xmake run simple_example
```

## Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `shared` | `true` | Build shared libraries |
| `static` | `false` | Build static libraries |
| `examples` | `true` | Build example projects |
| `tests` | `false` | Build test suite |
| `python_bindings` | `false` | Build Python bindings |

### Example Configurations
```bash
# Development build with tests
xmake f --mode=debug --tests=y --examples=y

# Release build for distribution
xmake f --mode=release --shared=y --static=n

# Minimal build (core only)
xmake f --examples=n --tests=n --python_bindings=n
```

## Build Targets

### Core Libraries
- **QtForgeCore**: Main plugin system library
- **QtForgeSecurity**: Security and validation components

### Optional Libraries
- **QtForgeNetwork**: Network-dependent features (requires Qt6Network)
- **QtForgeUI**: UI components (requires Qt6Widgets)

### Examples
- **simple_example**: Basic QtForge usage demonstration
- **xmake_config_test**: Build system validation

### Python Bindings
- **qtforge_python**: Python module (requires Python3 + pybind11)

## Platform-Specific Notes

### Windows
- **Recommended**: Use MinGW toolchain with Qt from MSYS2
- **Qt Detection**: Automatic detection from common installation paths
- **Toolchain**: `xmake f --toolchain=mingw` for compatibility

### Linux
- **Qt Installation**: Install via package manager or Qt installer
- **Dependencies**: Ensure development packages are installed
- **Example**: `sudo apt install qt6-base-dev qt6-tools-dev`

### macOS
- **Qt Installation**: Use Homebrew or Qt installer
- **Framework Support**: Automatic Qt framework detection
- **Example**: `brew install qt6`

## Troubleshooting

### Common Issues

#### Qt Not Found
```bash
# Specify Qt path manually
xmake f --qt=/path/to/qt

# Check Qt installation
xmake show -l packages
```

#### Compilation Errors
```bash
# Clean and rebuild
xmake clean
xmake build

# Use different toolchain
xmake f --toolchain=gcc
```

#### MOC Errors (Expected)
The current implementation excludes MOC-dependent files. This is expected behavior and will be addressed in future updates.

### Build System Comparison

| Feature | XMake | CMake |
|---------|-------|-------|
| Configuration | Simple Lua syntax | Complex CMake syntax |
| Qt Integration | Built-in support | Manual configuration |
| Cross-platform | Native support | Requires generators |
| Package Management | Integrated xrepo | External tools |
| Build Speed | Fast incremental builds | Standard |
| Learning Curve | Low | High |

## Advanced Usage

### Custom Qt Installation
```lua
-- In xmake.lua
add_linkdirs("/custom/qt/path/lib")
add_includedirs("/custom/qt/path/include")
```

### Adding New Targets
```lua
target("my_plugin")
    set_kind("shared")
    add_rules("qt.shared")
    add_frameworks("QtCore")
    add_deps("QtForgeCore")
    add_files("src/my_plugin.cpp")
target_end()
```

## Integration with IDEs

### Visual Studio Code
1. Install XMake extension
2. Open project folder
3. Use Command Palette: "XMake: Configure"

### Qt Creator
1. Open as generic project
2. Configure build commands to use xmake
3. Set up custom build steps

## Contributing

When contributing to QtForge with XMake support:

1. **Test both build systems**: Ensure changes work with both XMake and CMake
2. **Update configurations**: Modify both `xmake.lua` and `CMakeLists.txt`
3. **Document changes**: Update this documentation for new features
4. **Cross-platform testing**: Verify builds on multiple platforms

## Future Enhancements

- **MOC Integration**: Full Qt MOC file generation support
- **Package Distribution**: XMake package creation and distribution
- **CI/CD Integration**: GitHub Actions with XMake builds
- **IDE Integration**: Enhanced IDE support and project templates

## Support

For XMake-specific issues:
- Check [XMake Documentation](https://xmake.io/guide/)
- Report issues in QtForge repository with `[XMake]` tag
- Include `xmake show` output for debugging

For QtForge-specific issues:
- Follow standard QtForge contribution guidelines
- Test with both build systems when possible
