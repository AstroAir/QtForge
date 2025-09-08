# Installation Guide

This guide covers the installation and setup of QtForge v3.2.0, a modern C++ plugin framework built on Qt with comprehensive multilingual support.

## Prerequisites

### System Requirements

- **Operating System**: Windows 10/11, macOS 10.15+, or Linux (Ubuntu 20.04+)
- **Compiler**:
  - GCC 10+ (Linux)
  - Clang 12+ (macOS)
  - MSVC 2019+ (Windows)
- **CMake**: Version 3.20 or higher
- **Qt**: Version 6.2 or higher

### Development Tools

- **Git**: For version control and dependency management
- **Python**: 3.8+ (for build scripts and Python bindings)
- **Lua**: 5.4+ (for Lua plugin support)
- **pkg-config**: For dependency resolution (Linux/macOS)

### New in v3.2.0

- **Enhanced Python Bindings**: Complete API coverage with type stubs
- **Lua Plugin Bridge**: Full Lua plugin support with sol2 integration
- **Advanced Security**: Enhanced sandboxing and policy validation
- **Hot Reload**: Dynamic plugin reloading capabilities

## Installation Methods

### Method 1: Package Manager (Recommended)

#### Using vcpkg (Windows/Linux/macOS)

```bash
# Install vcpkg if not already installed
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # Linux/macOS
# or
./bootstrap-vcpkg.bat  # Windows

# Install QtForge
./vcpkg install qtforge
```

#### Using Conan (Cross-platform)

```bash
# Install Conan if not already installed
pip install conan

# Add QtForge repository
conan remote add qtforge https://conan.qtforge.io

# Install QtForge
conan install qtforge/1.0@qtforge/stable
```

### Method 2: Build from Source

#### Clone the Repository

```bash
git clone https://github.com/AstroAir/QtForge.git
cd QtForge
git submodule update --init --recursive
```

#### Configure Build

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DQTFORGE_BUILD_EXAMPLES=ON \
         -DQTFORGE_BUILD_TESTS=ON \
         -DQTFORGE_ENABLE_PYTHON=ON
```

#### Build Options

| Option                   | Default | Description            |
| ------------------------ | ------- | ---------------------- |
| `QTFORGE_BUILD_EXAMPLES` | `OFF`   | Build example plugins  |
| `QTFORGE_BUILD_TESTS`    | `OFF`   | Build unit tests       |
| `QTFORGE_ENABLE_PYTHON`  | `OFF`   | Enable Python bindings |
| `QTFORGE_ENABLE_LUA`     | `OFF`   | Enable Lua scripting   |
| `QTFORGE_BUILD_DOCS`     | `OFF`   | Build documentation    |

#### Compile

```bash
# Build the project
cmake --build . --config Release

# Install (optional)
cmake --install . --prefix /usr/local
```

### Method 3: Pre-built Binaries

Download pre-built binaries from the [releases page](https://github.com/AstroAir/QtForge/releases):

1. Download the appropriate package for your platform
2. Extract to your desired location
3. Add the `bin` directory to your system PATH
4. Set the `QTFORGE_ROOT` environment variable

## Verification

### Test Installation

Create a simple test file to verify the installation:

```cpp
#include <qtforge/core/plugin_interface.hpp>
#include <iostream>

int main() {
    std::cout << "QtForge version: " << qtforge::version() << std::endl;
    return 0;
}
```

Compile and run:

```bash
g++ -std=c++20 test.cpp -lqtforge -o test
./test
```

### Run Examples

If you built with examples enabled:

```bash
# Run basic plugin example
./examples/basic_plugin_example

# Run orchestration example
./examples/orchestration_example
```

## Environment Setup

### Environment Variables

Set these environment variables for optimal development experience:

```bash
# Linux/macOS
export QTFORGE_ROOT=/path/to/qtforge
export PATH=$QTFORGE_ROOT/bin:$PATH
export LD_LIBRARY_PATH=$QTFORGE_ROOT/lib:$LD_LIBRARY_PATH

# Windows (PowerShell)
$env:QTFORGE_ROOT = "C:\path\to\qtforge"
$env:PATH = "$env:QTFORGE_ROOT\bin;$env:PATH"
```

### IDE Configuration

#### Visual Studio Code

Install the recommended extensions:

- C/C++ Extension Pack
- CMake Tools
- Qt tools

#### CLion

Configure CMake profiles with QtForge paths in Settings → Build → CMake.

#### Qt Creator

Add QtForge as a custom kit in Tools → Options → Kits.

## Troubleshooting

### Common Issues

**Qt not found**

```bash
# Ensure Qt is in PATH or set Qt6_DIR
export Qt6_DIR=/path/to/qt6/lib/cmake/Qt6
```

**Missing dependencies**

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake qt6-base-dev

# macOS
brew install cmake qt6

# Windows
# Use Qt installer or vcpkg
```

**Python bindings not working**

```bash
# Ensure Python development headers are installed
sudo apt-get install python3-dev  # Ubuntu
brew install python3              # macOS
```

### Getting Help

- **Documentation**: [https://qtforge.readthedocs.io](https://qtforge.readthedocs.io)
- **Issues**: [GitHub Issues](https://github.com/AstroAir/QtForge/issues)
- **Discussions**: [GitHub Discussions](https://github.com/AstroAir/QtForge/discussions)
- **Discord**: [QtForge Community](https://discord.gg/qtforge)

## Next Steps

After successful installation:

1. **[Quick Start Guide](getting-started/quick-start.md)** - Create your first plugin
2. **[API Reference](api/index.md)** - Explore the API documentation
3. **[Examples](examples/index.md)** - Study example implementations
4. **[User Guide](user-guide/plugin-management.md)** - Learn advanced features
