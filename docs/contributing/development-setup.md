# Development Setup

This guide will help you set up a development environment for contributing to QtForge.

## Prerequisites

### Required Tools

- **C++ Compiler**: GCC 10+ (Linux), Clang 12+ (macOS), MSVC 2019+ (Windows)
- **CMake**: Version 3.20 or later
- **Qt6**: Version 6.5 or later
- **Git**: For version control
- **Python**: 3.8+ (for Python bindings and tools)

### Optional Tools

- **Ninja**: Fast build system (recommended)
- **Conan**: C++ package manager
- **vcpkg**: Microsoft C++ package manager
- **Visual Studio Code**: IDE with C++ extensions
- **Qt Creator**: Qt-specific IDE

## Environment Setup

### Windows

1. **Install Visual Studio Community 2019/2022**

   - Include C++ development tools
   - Install CMake tools

2. **Install Qt6**

   ```batch
   # Via Qt online installer
   # Or via vcpkg
   vcpkg install qt6-base qt6-tools
   ```

3. **Install Python**
   ```batch
   # Download from python.org or use winget
   winget install Python.Python.3
   ```

### Linux (Ubuntu/Debian)

1. **Install development tools**

   ```bash
   sudo apt update
   sudo apt install build-essential cmake git python3-dev
   ```

2. **Install Qt6**

   ```bash
   sudo apt install qt6-base-dev qt6-tools-dev qt6-tools-dev-tools
   ```

3. **Install additional dependencies**
   ```bash
   sudo apt install ninja-build pkg-config
   ```

### macOS

1. **Install Xcode Command Line Tools**

   ```bash
   xcode-select --install
   ```

2. **Install Homebrew** (if not already installed)

   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

3. **Install dependencies**
   ```bash
   brew install cmake qt6 python3 ninja
   ```

## Source Code Setup

### Fork and Clone

1. **Fork the repository** on GitHub

2. **Clone your fork**

   ```bash
   git clone https://github.com/YOUR_USERNAME/QtForge.git
   cd QtForge
   ```

3. **Add upstream remote**
   ```bash
   git remote add upstream https://github.com/AstroAir/QtForge.git
   ```

### Configure Git

```bash
git config user.name "Your Name"
git config user.email "your.email@example.com"
```

## Build Configuration

### CMake Presets

QtForge provides CMake presets for common configurations:

```bash
# List available presets
cmake --list-presets

# Configure debug build
cmake --preset debug

# Configure release build
cmake --preset release

# Configure with testing enabled
cmake --preset debug-testing
```

### Manual Configuration

```bash
# Create build directory
mkdir build && cd build

# Configure CMake
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DQTFORGE_BUILD_TESTS=ON \
  -DQTFORGE_BUILD_EXAMPLES=ON \
  -DQTFORGE_BUILD_PYTHON_BINDINGS=ON \
  -DCMAKE_PREFIX_PATH=/path/to/qt6

# Build the project
cmake --build . --config Debug -j $(nproc)
```

### Build Options

| Option                          | Default | Description               |
| ------------------------------- | ------- | ------------------------- |
| `QTFORGE_BUILD_TESTS`           | `OFF`   | Build unit tests          |
| `QTFORGE_BUILD_EXAMPLES`        | `OFF`   | Build example projects    |
| `QTFORGE_BUILD_PYTHON_BINDINGS` | `OFF`   | Build Python bindings     |
| `QTFORGE_BUILD_LUA_BINDINGS`    | `OFF`   | Build Lua bindings        |
| `QTFORGE_BUILD_DOCS`            | `OFF`   | Build documentation       |
| `QTFORGE_ENABLE_SECURITY`       | `ON`    | Enable security features  |
| `QTFORGE_ENABLE_NETWORKING`     | `ON`    | Enable networking support |

## IDE Configuration

### Visual Studio Code

1. **Install extensions**:

   - C/C++ Extension Pack
   - CMake Tools
   - Qt tools

2. **Configure workspace** (`.vscode/settings.json`):

   ```json
   {
     "cmake.configureOnOpen": true,
     "cmake.preferredGenerators": ["Ninja"],
     "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
   }
   ```

### Qt Creator

1. **Open CMakeLists.txt** as project
2. **Configure kit** with Qt6
3. **Set CMake arguments**:
   ```
   -DQTFORGE_BUILD_TESTS=ON
   -DQTFORGE_BUILD_EXAMPLES=ON
   ```

### CLion

1. **Open project directory**
2. **Configure CMake settings** in Settings → Build → CMake
3. **Set CMake options** for development build

## Running Tests

### Unit Tests

```bash
# Run all tests
ctest --output-on-failure

# Run specific test suite
ctest -R CoreTests

# Run tests with verbose output
ctest --verbose
```

### Integration Tests

```bash
# Run integration tests
ctest -R IntegrationTests

# Run with specific configuration
ctest -C Debug --output-on-failure
```

### Python Binding Tests

```bash
# Install Python test dependencies
pip install pytest pytest-qt

# Run Python tests
python -m pytest tests/python/
```

## Code Style and Formatting

### C++ Code Style

QtForge follows these C++ coding standards:

- **Modern C++**: Use C++17/20 features where appropriate
- **Qt Style**: Follow Qt naming conventions for Qt-related code
- **Standard Style**: Use standard C++ conventions for pure C++ code

### Formatting Tools

1. **Install clang-format**

   ```bash
   # Ubuntu/Debian
   sudo apt install clang-format

   # macOS
   brew install clang-format

   # Windows (via LLVM)
   # Download from LLVM releases
   ```

2. **Format code**

   ```bash
   # Format single file
   clang-format -i src/core/plugin_manager.cpp

   # Format all source files
   find src/ -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
   ```

### Pre-commit Hooks

1. **Install pre-commit**

   ```bash
   pip install pre-commit
   ```

2. **Install hooks**

   ```bash
   pre-commit install
   ```

3. **Run manually**
   ```bash
   pre-commit run --all-files
   ```

## Documentation Setup

### Building Documentation

```bash
# Install documentation dependencies
pip install mkdocs mkdocs-material

# Build documentation
mkdocs build

# Serve locally
mkdocs serve
```

### API Documentation

```bash
# Install Doxygen
sudo apt install doxygen graphviz  # Linux
brew install doxygen graphviz      # macOS

# Generate API docs
doxygen Doxyfile
```

## Common Issues and Solutions

### Qt6 Not Found

```bash
# Set Qt6 path explicitly
export CMAKE_PREFIX_PATH=/path/to/qt6:$CMAKE_PREFIX_PATH

# Or use Qt6_DIR
cmake .. -DQt6_DIR=/path/to/qt6/lib/cmake/Qt6
```

### Python Binding Build Fails

```bash
# Install pybind11
pip install pybind11

# Or use system package
sudo apt install pybind11-dev  # Linux
brew install pybind11           # macOS
```

### Test Failures

```bash
# Run tests with detailed output
ctest --verbose --output-on-failure

# Run specific failing test
ctest -R FailingTestName --verbose
```

### Build Performance

```bash
# Use Ninja generator for faster builds
cmake .. -GNinja

# Use parallel builds
cmake --build . -j $(nproc)

# Enable compiler cache
export CCACHE_DIR=~/.ccache
cmake .. -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

## Next Steps

After setting up your development environment:

1. **Read the [Contributing Guidelines](index.md)**
2. **Check out [Coding Standards](coding-standards.md)**
3. **Review [Testing Guidelines](testing.md)**
4. **Browse [Open Issues](https://github.com/AstroAir/QtForge/issues)**

## Getting Help

If you encounter issues during setup:

- **Check the [FAQ](../appendix/faq.md)**
- **Search [GitHub Issues](https://github.com/AstroAir/QtForge/issues)**
- **Join our [Discord Community](https://discord.gg/qtforge)**
- **Ask on [GitHub Discussions](https://github.com/AstroAir/QtForge/discussions)**
