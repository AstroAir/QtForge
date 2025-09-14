# Installation Guide

This comprehensive guide covers the installation and setup of QtForge v3.2.0, a modern C++ plugin framework built on Qt with comprehensive multilingual support.

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

## Quick Installation

### Using Package Managers

=== "vcpkg"

    ```bash
    # Install vcpkg if you haven't already
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh  # Linux/macOS
    # or
    .\bootstrap-vcpkg.bat  # Windows

    # Install QtForge
    ./vcpkg install qtforge

    # With optional components
    ./vcpkg install qtforge[network,ui,examples]
    ```

=== "Conan"

    ```bash
    # Add the remote (if not already added)
    conan remote add qtforge https://api.bintray.com/conan/qtforge/conan

    # Add QtForge repository
    conan remote add qtforge https://conan.qtforge.io

    # Install QtForge
    conan install qtforge/1.0@qtforge/stable
    ```

=== "Homebrew (macOS)"

    ```bash
    # Add the tap
    brew tap qtforge/qtforge

    # Install QtForge
    brew install qtforge

    # With all components
    brew install qtforge --with-network --with-ui
    ```

### Using CMake FetchContent

Add this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    QtForge
    GIT_REPOSITORY https://github.com/AstroAir/QtForge.git
    GIT_TAG        v3.2.0  # or main for latest
)

FetchContent_MakeAvailable(QtForge)

# Link to your target
target_link_libraries(your_app
    QtForge::Core
    QtForge::Security  # optional
    QtForge::Network   # optional
    QtForge::UI        # optional
)
```

## Build from Source

### Clone the Repository

```bash
git clone https://github.com/AstroAir/QtForge.git
cd QtForge
git submodule update --init --recursive
```

### Configure Build

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DQTFORGE_BUILD_EXAMPLES=ON \
         -DQTFORGE_BUILD_TESTS=ON \
         -DQTFORGE_ENABLE_PYTHON=ON
```

### Build Options

| Option                   | Default | Description            |
| ------------------------ | ------- | ---------------------- |
| `QTFORGE_BUILD_EXAMPLES` | `OFF`   | Build example plugins  |
| `QTFORGE_BUILD_TESTS`    | `OFF`   | Build unit tests       |
| `QTFORGE_ENABLE_PYTHON`  | `OFF`   | Enable Python bindings |
| `QTFORGE_ENABLE_LUA`     | `OFF`   | Enable Lua scripting   |
| `QTFORGE_BUILD_DOCS`     | `OFF`   | Build documentation    |

### Compile

```bash
# Build the project
cmake --build . --config Release

# Install (optional)
cmake --install . --prefix /usr/local
```

## Platform-Specific Installation

### Windows

#### Prerequisites

1. **Visual Studio 2019 or later** (with C++20 support)
2. **Qt 6.0+** installed via:
   - Qt Online Installer (recommended)
   - vcpkg: `vcpkg install qt6-base qt6-network qt6-widgets`
3. **CMake 3.21+**
4. **Git** for version control

#### Installation Steps

=== "Visual Studio"

    ```powershell
    # Clone the repository
    git clone https://github.com/QtForge/QtPlugin.git
    cd QtPlugin

    # Create build directory
    mkdir build
    cd build

    # Configure with CMake
    cmake .. -G "Visual Studio 17 2022" -A x64 ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DQTPLUGIN_BUILD_EXAMPLES=ON ^
        -DQTPLUGIN_BUILD_TESTS=ON

    # Build
    cmake --build . --config Release

    # Install
    cmake --install . --prefix "C:\QtPlugin"
    ```

=== "MinGW"

    ```bash
    # Using MSYS2/MinGW64
    pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-qt6-base

    # Clone and build
    git clone https://github.com/QtForge/QtPlugin.git
    cd QtPlugin
    mkdir build && cd build

    cmake .. -G "MinGW Makefiles" \
        -DCMAKE_BUILD_TYPE=Release \
        -DQTPLUGIN_BUILD_EXAMPLES=ON

    cmake --build .
    cmake --install . --prefix /mingw64
    ```

#### Windows-Specific Notes

- **Qt Path**: Ensure Qt is in your PATH or set `Qt6_DIR`
- **MSVC Runtime**: Use `/MD` for release builds (default)
- **DLL Dependencies**: QtPlugin builds as static libraries by default

### Linux

#### Prerequisites

=== "Ubuntu/Debian"

    ```bash
    # Install dependencies
    sudo apt update
    sudo apt install -y \
        build-essential \
        cmake \
        git \
        qt6-base-dev \
        qt6-tools-dev \
        libqt6core6 \
        libqt6network6 \
        libqt6widgets6

    # Optional: for network/UI components
    sudo apt install -y \
        qt6-networkauth-dev \
        qt6-svg-dev
    ```

=== "CentOS/RHEL/Fedora"

    ```bash
    # Fedora
    sudo dnf install -y \
        gcc-c++ \
        cmake \
        git \
        qt6-qtbase-devel \
        qt6-qttools-devel \
        qt6-qtnetworkauth-devel

    # CentOS/RHEL (enable EPEL first)
    sudo yum install -y epel-release
    sudo yum install -y \
        gcc-c++ \
        cmake3 \
        git \
        qt6-qtbase-devel
    ```

=== "Arch Linux"

    ```bash
    # Install dependencies
    sudo pacman -S \
        base-devel \
        cmake \
        git \
        qt6-base \
        qt6-tools \
        qt6-networkauth \
        qt6-svg
    ```

#### Installation Steps

```bash
# Clone the repository
git clone https://github.com/QtForge/QtPlugin.git
cd QtPlugin

# Create build directory
mkdir build && cd build

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DQTPLUGIN_BUILD_EXAMPLES=ON \
    -DQTPLUGIN_BUILD_TESTS=ON \
    -DQTPLUGIN_BUILD_NETWORK=ON \
    -DQTPLUGIN_BUILD_UI=ON

# Build (use all available cores)
cmake --build . --parallel $(nproc)

# Run tests (optional)
ctest --output-on-failure

# Install
sudo cmake --install .
```

#### Linux-Specific Notes

- **pkg-config**: QtPlugin installs `.pc` files for pkg-config integration
- **RPATH**: Properly configured for system installation
- **Permissions**: Use `sudo` for system-wide installation

### macOS

#### Prerequisites

1. **Xcode Command Line Tools**:

   ```bash
   xcode-select --install
   ```

2. **Homebrew** (recommended):

   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

3. **Dependencies**:
   ```bash
   brew install cmake qt6 git
   ```

#### Installation Steps

```bash
# Clone the repository
git clone https://github.com/QtForge/QtPlugin.git
cd QtPlugin

# Create build directory
mkdir build && cd build

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
    -DQTPLUGIN_BUILD_EXAMPLES=ON \
    -DQTPLUGIN_BUILD_TESTS=ON

# Build
cmake --build . --parallel $(sysctl -n hw.ncpu)

# Run tests
ctest --output-on-failure

# Install
sudo cmake --install .
```

#### macOS-Specific Notes

- **Deployment Target**: Set to macOS 10.15+ for C++20 support
- **Code Signing**: May require code signing for distribution
- **Framework**: Qt can be installed as frameworks or libraries

## Build Configuration Options

QtPlugin provides several CMake options to customize your build:

### Core Options

| Option                           | Default | Description                    |
| -------------------------------- | ------- | ------------------------------ |
| `QTPLUGIN_BUILD_NETWORK`         | `AUTO`  | Build network plugin support   |
| `QTPLUGIN_BUILD_UI`              | `AUTO`  | Build UI plugin support        |
| `QTPLUGIN_BUILD_EXAMPLES`        | `ON`    | Build example plugins          |
| `QTPLUGIN_BUILD_TESTS`           | `OFF`   | Build unit tests               |
| `QTPLUGIN_BUILD_COMPONENT_TESTS` | `OFF`   | Build component-specific tests |

### Advanced Options

| Option                              | Default   | Description                               |
| ----------------------------------- | --------- | ----------------------------------------- |
| `QTPLUGIN_ENABLE_COMPONENT_LOGGING` | `OFF`     | Enable detailed component logging         |
| `CMAKE_BUILD_TYPE`                  | `Release` | Build type (Debug/Release/RelWithDebInfo) |
| `BUILD_SHARED_LIBS`                 | `OFF`     | Build shared libraries instead of static  |

### Example Configurations

=== "Minimal Build"

    ```bash
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DQTPLUGIN_BUILD_EXAMPLES=OFF \
        -DQTPLUGIN_BUILD_TESTS=OFF \
        -DQTPLUGIN_BUILD_NETWORK=OFF \
        -DQTPLUGIN_BUILD_UI=OFF
    ```

=== "Full Development Build"

    ```bash
    cmake .. \
        -DCMAKE_BUILD_TYPE=Debug \
        -DQTPLUGIN_BUILD_EXAMPLES=ON \
        -DQTPLUGIN_BUILD_TESTS=ON \
        -DQTPLUGIN_BUILD_COMPONENT_TESTS=ON \
        -DQTPLUGIN_BUILD_NETWORK=ON \
        -DQTPLUGIN_BUILD_UI=ON \
        -DQTPLUGIN_ENABLE_COMPONENT_LOGGING=ON
    ```

=== "Production Build"

    ```bash
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/qtplugin \
        -DQTPLUGIN_BUILD_EXAMPLES=ON \
        -DQTPLUGIN_BUILD_TESTS=ON \
        -DQTPLUGIN_BUILD_NETWORK=ON \
        -DQTPLUGIN_BUILD_UI=ON \
        -DBUILD_SHARED_LIBS=ON
    ```

## Verification

After installation, verify that QtPlugin is working correctly:

### 1. Check Installation

=== "find_package"

    Create a test `CMakeLists.txt`:

    ```cmake
    cmake_minimum_required(VERSION 3.21)
    project(QtPluginTest)

    find_package(QtPlugin REQUIRED COMPONENTS Core)

    add_executable(test_qtplugin main.cpp)
    target_link_libraries(test_qtplugin QtPlugin::Core)
    ```

=== "pkg-config"

    ```bash
    # Check if pkg-config can find QtPlugin
    pkg-config --exists qtplugin && echo "QtPlugin found" || echo "QtPlugin not found"

    # Get version
    pkg-config --modversion qtplugin

    # Get compile flags
    pkg-config --cflags qtplugin

    # Get link flags
    pkg-config --libs qtplugin
    ```

### 2. Test Basic Functionality

Create a simple test program:

```cpp title="main.cpp"
#include <qtplugin/qtplugin.hpp>
#include <iostream>

int main() {
    // Test library initialization
    qtplugin::LibraryInitializer init;
    if (!init.is_initialized()) {
        std::cerr << "Failed to initialize QtPlugin library" << std::endl;
        return -1;
    }

    std::cout << "QtPlugin library initialized successfully!" << std::endl;
    std::cout << "Version: " << qtplugin::version_string() << std::endl;

    // Test plugin manager creation
    auto manager = qtplugin::PluginManager::create();
    if (!manager) {
        std::cerr << "Failed to create plugin manager" << std::endl;
        return -1;
    }

    std::cout << "Plugin manager created successfully!" << std::endl;
    return 0;
}
```

Build and run:

```bash
mkdir test_build && cd test_build
cmake ..
cmake --build .
./test_qtplugin
```

Expected output:

```
QtPlugin library initialized successfully!
Version: 3.0.0
Plugin manager created successfully!
```

### 3. Run Example Programs

If you built with examples:

```bash
# Navigate to build directory
cd build

# Run basic plugin example
./examples/basic_plugin/test_basic_plugin

# Run plugin manager demo
./examples/plugin_manager_demo --help
```

## Troubleshooting

### Common Issues

#### Qt Not Found

**Problem**: CMake cannot find Qt6

**Solutions**:

=== "Set Qt6_DIR"

    ```bash
    # Find your Qt installation
    find /usr -name "Qt6Config.cmake" 2>/dev/null
    # or on Windows
    dir /s Qt6Config.cmake

    # Set Qt6_DIR
    cmake .. -DQt6_DIR=/path/to/qt6/lib/cmake/Qt6
    ```

=== "Add to PATH"

    ```bash
    # Linux/macOS
    export PATH="/path/to/qt6/bin:$PATH"
    export CMAKE_PREFIX_PATH="/path/to/qt6:$CMAKE_PREFIX_PATH"

    # Windows
    set PATH=C:\Qt\6.5.0\msvc2019_64\bin;%PATH%
    set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64;%CMAKE_PREFIX_PATH%
    ```

#### Compiler Issues

**Problem**: C++20 features not available

**Solutions**:

- **GCC**: Use GCC 10 or later
- **Clang**: Use Clang 12 or later
- **MSVC**: Use Visual Studio 2019 16.8 or later

```bash
# Check compiler version
gcc --version
clang --version
# Windows: check Visual Studio version in IDE
```

#### CMake Version

**Problem**: CMake version too old

**Solution**: Install CMake 3.21+

=== "Linux"

    ```bash
    # Remove old version
    sudo apt remove cmake

    # Install from Kitware APT repository
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
    sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
    sudo apt update
    sudo apt install cmake
    ```

=== "macOS"

    ```bash
    brew install cmake
    ```

=== "Windows"

    Download from [cmake.org](https://cmake.org/download/) or use:
    ```powershell
    winget install Kitware.CMake
    ```

### Build Failures

#### Missing Dependencies

Check that all required dependencies are installed:

```bash
# Verify Qt installation
qmake --version  # or qmake6 --version

# Check CMake
cmake --version

# Verify compiler
g++ --version    # or clang++ --version
```

#### Permission Issues

On Linux/macOS, you might need to adjust permissions:

```bash
# Make sure you have write permissions to install directory
sudo chown -R $USER:$USER /usr/local/include/qtplugin
sudo chown -R $USER:$USER /usr/local/lib/libqtplugin*
```

### Getting Help

If you're still having issues:

1. **Check the [FAQ](../appendix/faq.md)** for common questions
2. **Search [GitHub Issues](https://github.com/QtForge/QtPlugin/issues)** for similar problems
3. **Create a new issue** with:
   - Your operating system and version
   - Qt version
   - Compiler version
   - CMake version
   - Complete error messages
   - Steps to reproduce

## Next Steps

Now that QtPlugin is installed, you're ready to:

1. **[Quick Start](quick-start.md)** - Create your first QtPlugin application
2. **[First Plugin](first-plugin.md)** - Build your first plugin
3. **[Examples](../examples/index.md)** - Explore working examples

## Alternative Installation Methods

### Docker

For containerized development:

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    qt6-tools-dev

WORKDIR /app
COPY . .

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cmake --install .
```

### Automated Scripts

QtPlugin provides automated installation scripts:

=== "Linux/macOS"

    ```bash
    curl -sSL https://raw.githubusercontent.com/QtForge/QtPlugin/main/scripts/install.sh | bash
    ```

=== "Windows (PowerShell)"

    ```powershell
    iex ((New-Object System.Net.WebClient).DownloadString('https://raw.githubusercontent.com/QtForge/QtPlugin/main/scripts/install.ps1'))
    ```

These scripts will:

- Detect your system and install dependencies
- Download and build QtPlugin
- Install to appropriate system locations
- Verify the installation
