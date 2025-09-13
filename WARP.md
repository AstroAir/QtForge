# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

QtForge v3.2.0 is a fully functional, modern C++ Plugin System for Qt Applications with comprehensive plugin management capabilities. This production-ready library provides a robust foundation for building extensible applications with a focus on security, performance, and ease of use.

**Current Status**: ✅ FULLY FUNCTIONAL with 100% test success rate (3/3 tests passing)
- Core Library: `libqtforge-core.dll` (4.6MB) - Complete implementation
- Security Module: `libqtforge-security.dll` (234KB) - Full security features
- All examples and test applications working correctly

## Build System & Commands

### Primary Build System (CMake)

QtForge uses CMake as its primary build system with comprehensive cross-platform support.

#### Quick Build Commands

```bash
# Configure and build with CMake presets (recommended)
cmake --preset default
cmake --build build/default --config Release

# Alternative: Manual configuration for development
cmake -S . -B build -DQTFORGE_BUILD_EXAMPLES=ON -DQTFORGE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# Cross-platform Python build script (recommended for complex builds)
python scripts/build.py --build-type Release --tests --examples --network --ui

# Build with all features enabled (comprehensive development setup)
cmake -S . -B build -DQTFORGE_BUILD_EXAMPLES=ON -DQTFORGE_BUILD_TESTS=ON -DQTFORGE_BUILD_NETWORK=ON -DQTFORGE_BUILD_UI=ON -DQTFORGE_BUILD_PYTHON_BINDINGS=ON -DQTFORGE_BUILD_LUA_BINDINGS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

#### Available CMake Presets

- `default`: Default configuration with auto-detected settings
- `debug`: Debug build with sanitizers and logging enabled
- `release`: Optimized release build with LTO
- `windows-msvc`: Windows build using MSVC compiler
- `windows-clang`: Windows build using Clang compiler
- `linux-gcc`: Linux build using GCC compiler
- `linux-clang`: Linux build using Clang compiler
- `macos`: macOS build configuration
- `android-arm64`: Android build for ARM64 architecture
- `ios`: iOS build configuration
- `cross-linux-arm64`: Cross-compilation for Linux ARM64
- `embedded-riscv`: Embedded RISC-V build configuration

### Testing Commands

```bash
# Run all tests using CTest
ctest --test-dir build --output-on-failure --parallel

# Run specific test with verbose output
ctest -R "PluginManager" -V --test-dir build

# Run all tests with comprehensive output
powershell -ExecutionPolicy Bypass -File run_test.ps1 -Verbose

# Run individual test executables (examples from successful builds)
./build/default/examples/01-fundamentals/basic-plugin/Release/BasicPluginTest.exe
./build/default/examples/03-services/background-tasks/Release/ServicePluginTest.exe
./build/default/examples/03-services/background-tasks/Release/ServicePluginTaskTest.exe

# Run tests using CMake presets
ctest --preset debug
ctest --preset release

# Test with specific wrapper (Windows)
powershell -File run_test.ps1 -TestExecutable "path/to/test.exe" -WorkingDirectory "path/to/directory"

# Run Python test suite
PYTHONPATH=build python -m pytest tests/python/ -v
python tests/integration/run_integration_tests.py
python tests/performance/run_performance_tests.py

# Run all tests with Python script
python tests/run_all_tests.py
```

#### Python Bindings Testing

```bash
# Set up Python path and run comprehensive tests
PYTHONPATH=build python tests/python/test_comprehensive_core.py
PYTHONPATH=build python tests/python/test_comprehensive_managers.py
PYTHONPATH=build python tests/python/test_comprehensive_communication.py

# Run all Python tests with discovery
python -m unittest discover tests/python -p "test_*.py" -v

# Run specific integration tests
python tests/integration/test_cross_language_integration.py

# Run Python examples to verify bindings
python examples/basic_usage.py
python examples/advanced_plugin_interfaces.py
```

### Quick Development Commands

```bash
# Fast development cycle
cmake --preset debug && cmake --build build/debug --parallel
ctest --test-dir build/debug --output-on-failure

# Alternative development configuration (compatible with AGENTS.md)
cmake -S . -B build -DQTPLUGIN_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure

# Release build with packaging
cmake --preset release && cmake --build build/release --parallel
cpack --config build/release/CPackConfig.cmake

# Lint and format (if available)
clang-format -i src/**/*.cpp include/**/*.hpp
clang-tidy src/**/*.cpp -- -Iinclude
```

# Platform-specific packaging
cpack --preset windows-release  # Creates NSIS, ZIP, WIX packages
cpack --preset linux-release    # Creates DEB, RPM, TGZ packages
cpack --preset macos-release    # Creates DragNDrop, TGZ packages

# Lint and format (if available)
clang-format -i src/**/*.cpp include/**/*.hpp
clang-tidy src/**/*.cpp -- -Iinclude
```

## Architecture Overview
## Architecture Overview

### Core System Architecture

QtForge follows a layered, component-based architecture:

#### Layer 1: Core Plugin System
- **Plugin Interface** (`IPlugin`): Base interface for all plugins with lifecycle management
- **Plugin Manager**: Central orchestrator for plugin loading, initialization, and management
- **Plugin Loader**: Platform-specific dynamic library loading with validation
- **Plugin Registry**: Plugin discovery, registration, and metadata management
- **Plugin Lifecycle Manager**: State machine-driven plugin lifecycle with custom states

#### Layer 2: Advanced Plugin Interfaces
- **Service Plugin Interface** (`IServicePlugin`): Background services and task processing
- **Advanced Plugin Interface** (`IAdvancedPlugin`): Dynamic capability discovery
- **Dynamic Plugin Interface** (`IDynamicPlugin`): Runtime behavior modification
- **Specialized Interfaces**: Data processing, network, UI, and scripting plugins

#### Layer 3: Communication & Messaging
- **Message Bus**: Pub/sub messaging system with type safety and filtering
- **Service Contracts**: Structured communication between plugins with discovery
- **Request-Response System**: Synchronous and asynchronous inter-plugin communication
- **Event System**: Typed event handling with priority and filtering

#### Layer 4: Resource & Configuration Management
- **Configuration Manager**: Hierarchical configuration with validation, merging, and watching
- **Resource Manager**: Memory, file handle, and custom resource lifecycle management
- **Plugin Version Manager**: Versioning, storage management, and compatibility tracking
- **Logging Manager**: Structured logging with multiple backends and filtering

#### Layer 5: Security & Validation
- **Security Manager**: Plugin validation, signature verification, and sandboxing
- **Permission Manager**: Fine-grained permission control for plugin operations
- **Security Policy Engine**: Configurable security policies with enforcement
- **Plugin Validator**: Metadata validation and compatibility checking

#### Layer 6: Multi-Language Bridges
- **Python Plugin Bridge**: Full Python plugin support with type stubs for IDEs
- **Lua Plugin Bridge**: Lua plugin support with sol2 integration and sandboxing
- **JavaScript Support**: Planned JavaScript plugin support

#### Layer 7: Advanced Features
- **Plugin Orchestrator**: Complex multi-plugin workflows and dependencies
- **Plugin Composition**: Dynamic plugin composition and capability aggregation
- **Transaction Manager**: ACID transaction support for plugin operations
- **Hot Reload Manager**: Runtime plugin reloading with state preservation
- **Plugin Marketplace**: Remote plugin discovery, download, and management

### Plugin Types Supported

1. **Native C++ Plugins**: Standard shared libraries with QtPlugin interfaces
2. **Python Plugins**: Python modules with comprehensive QtForge bindings
3. **Lua Plugins**: Lua scripts with full QtForge integration via sol2
4. **Service Plugins**: Background services with task processing capabilities
5. **Remote Plugins**: Network-loaded plugins with HTTP/HTTPS support
6. **Composite Plugins**: Dynamic composition of multiple plugin capabilities

### Threading & Concurrency

- **Thread-Safe Design**: All public APIs use proper synchronization (shared_mutex, atomic operations)
- **Plugin Thread Pool**: Dedicated thread pool for plugin task execution
- **Background Services**: Asynchronous task processing with priority queues
- **Concurrent Plugin Loading**: Parallel plugin initialization with dependency resolution

### Error Handling Philosophy

QtForge uses modern C++20 `std::expected<T, E>` pattern throughout:

```cpp
// Plugin operations return expected values
auto result = manager.load_plugin("plugin.qtplugin");
if (result.has_value()) {
    auto plugin_id = result.value();
    // Success path
} else {
    auto error = result.error();
    // Handle specific error types
}
```

## Development Guidelines

### Code Style & Standards

- **C++20 Standard**: Modern C++ features, concepts, ranges, and coroutines
- **Qt6 Integration**: Full Qt6 framework integration with MOC support
- **RAII Principles**: Automatic resource management with smart pointers
- **Thread Safety**: Concurrent access patterns with proper synchronization
- **Error Handling**: `std::expected<T,E>` for all fallible operations
- **Testing**: Comprehensive test coverage with Qt Test framework and Python unittest

### Memory Management

```cpp
// Use smart pointers and RAII
std::shared_ptr<IPlugin> plugin = manager.get_plugin(plugin_id);
std::unique_ptr<PluginLoader> loader = std::make_unique<PluginLoader>();

// Automatic cleanup through RAII
class ResourceHandle {
    std::string m_resource_id;
    ResourceManager* m_manager;
public:
    ~ResourceHandle() {
        if (m_manager) {
            m_manager->release_resource(m_resource_id);
        }
    }
};
```

### Plugin Development Patterns

```cpp
// Modern plugin implementation
class MyPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    QTPLUGIN_DECLARE_PLUGIN(MyPlugin, "com.example.MyPlugin/1.0", "metadata.json")

public:
    std::expected<void, qtplugin::PluginError> initialize() override {
        // Plugin initialization logic
        return {};
    }
    
    std::expected<nlohmann::json, qtplugin::PluginError> 
    execute_command(std::string_view command, const nlohmann::json& params) override {
        // Handle plugin commands
        return nlohmann::json{{"status", "success"}};
    }
};
```

### Testing Requirements

- **Unit Tests**: Test individual components in isolation
- **Integration Tests**: Test component interactions and workflows
- **Error Path Tests**: Comprehensive error handling validation
- **Performance Tests**: Validate performance requirements and benchmarks
- **Multi-Language Tests**: Python bindings and Lua bridge testing

### Build Configuration Options

Key CMake options for development:

- `QTFORGE_BUILD_EXAMPLES=ON`: Build working examples and demos
- `QTFORGE_BUILD_TESTS=ON`: Build comprehensive test suite
- `QTFORGE_BUILD_PYTHON_BINDINGS=ON`: Build Python bindings with pybind11
- `QTFORGE_BUILD_LUA_BINDINGS=ON`: Build Lua bindings with sol2
- `QTFORGE_BUILD_NETWORK=ON`: Build network plugin support
- `QTFORGE_BUILD_UI=ON`: Build UI plugin support
- `QTFORGE_ENABLE_SANITIZERS=ON`: Enable AddressSanitizer and UBSan (debug builds)
- `QTFORGE_ENABLE_LTO=ON`: Enable Link Time Optimization (release builds)

### Dependencies & Requirements

**Core Requirements:**
- CMake 3.21 or later
- Qt6 (Core module required, others optional)
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)

**Optional Dependencies:**
- Python 3.8+ and pybind11 (for Python bindings)
- Lua 5.4+ and sol2 (for Lua bindings)
- Qt6 Network (for remote plugin support)
- Qt6 Widgets (for UI plugin support)
- Doxygen (for documentation generation)

### Environment Setup Tips

**Qt6 Discovery:**
```bash
# Ensure Qt6 is discoverable
export CMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64:$CMAKE_PREFIX_PATH
# OR
export Qt6_DIR=/path/to/Qt/6.x.x/gcc_64/lib/cmake/Qt6
```

**Python Bindings Development:**
```bash
# Set Python path for local testing
export PYTHONPATH=build/src/python:$PYTHONPATH
# OR run tests with path prefix
PYTHONPATH=build python tests/run_all_tests.py
```

### Key Files & Locations

- **Main Header**: `include/qtplugin/qtplugin.hpp` - Primary API entry point
- **Core Implementation**: `src/` - All source code organized by component
- **Examples**: `examples/` - Working demonstrations (C++, Python, Lua)
- **Tests**: `tests/` - Comprehensive test suites organized by module
- **Build Scripts**: `scripts/build.py` - Cross-platform build automation
- **CMake Configuration**: `cmake/` - Modular CMake system with presets
- **Documentation**: `docs/` - API reference and guides
- **Python Bindings**: `src/python/` - pybind11 bindings with type stubs
- **Lua Bindings**: `src/lua/` - sol2 Lua integration
- **Build Outputs**: `build/` - Generated build artifacts and libraries

### Directory Structure Deep Dive

```
QtForge/
├── include/qtplugin/           # Public API headers
│   ├── core/                   # Core plugin system interfaces
│   ├── communication/          # Inter-plugin messaging system
│   ├── managers/               # Resource and lifecycle managers
│   ├── security/               # Security and validation components
│   ├── bridges/                # Multi-language plugin bridges
│   ├── interfaces/             # Specialized plugin interfaces
│   └── utils/                  # Utility classes and helpers
├── src/                        # Implementation organized by domain
│   ├── core/                   # Plugin manager, loader, registry
│   ├── communication/          # Message bus, event system
│   ├── managers/               # Configuration, logging, resources
│   ├── security/               # Security policies, validation
│   ├── python/                 # Python bindings (pybind11)
│   └── lua/                    # Lua bindings (sol2)
├── examples/                   # Organized by complexity
│   ├── 01-fundamentals/        # Basic plugin examples
│   ├── 02-communication/       # Inter-plugin communication
│   ├── 03-services/            # Background services
│   ├── 04-specialized/         # Security, network, monitoring
│   └── python/                 # Python plugin examples
├── tests/                      # Test infrastructure
│   ├── unit/                   # Component unit tests
│   ├── integration/            # Cross-component tests
│   ├── performance/            # Performance benchmarks
│   └── python/                 # Python bindings tests
├── cmake/                      # Build system helpers
│   ├── toolchains/             # Cross-compilation toolchains
│   ├── QtPluginComponents.cmake # Component discovery helpers
│   └── QtPluginConfig.cmake.in # Package config template
└── scripts/                    # Automation scripts
    ├── build.py                # Cross-platform build automation
    ├── build.sh/.bat           # Platform-specific build scripts
    └── install.py              # Installation automation
```

### Performance Considerations

- **Plugin Loading**: Lazy loading with dependency resolution
- **Memory Usage**: Resource pools and lifecycle management
- **Thread Safety**: Read-write locks for concurrent access
- **Hot Reload**: Minimal disruption plugin reloading
- **Background Tasks**: Efficient task queuing and processing

## Common Development Tasks

### Running a Single Test
```bash
# Run specific test executable
./build/default/examples/01-fundamentals/basic-plugin/Release/BasicPluginTest.exe

# Run with debug output
QT_LOGGING_RULES="qtforge.*=true" ./build/debug/examples/01-fundamentals/basic-plugin/Debug/BasicPluginTest

# Run test using CTest pattern matching
ctest --test-dir build -R "BasicPlugin" --verbose
```

### Plugin Development Workflow
```bash
# 1. Create plugin in examples/ following existing structure
# 2. Add CMakeLists.txt using qtplugin_add_plugin helper
# 3. Build and test
cmake --build build --target your_plugin_test
./build/examples/your-plugin/YourPluginTest

# Alternative: Use qtplugin CMake helpers
# qtplugin_add_plugin(my_plugin
#     TYPE service
#     SOURCES src/myplugin.cpp
#     HEADERS include/myplugin.hpp
#     METADATA metadata.json
# )
```

### Working with Python Bindings
```bash
# Build Python module with presets
cmake --preset debug -DQTFORGE_BUILD_PYTHON_BINDINGS=ON
cmake --build build/debug --target qtforge_python

# Test Python bindings
PYTHONPATH=build python -c "import qtforge; print(qtforge.version())"

# Run Python examples
python examples/basic_usage.py
python examples/advanced_plugin_interfaces.py
```

### Working with Lua Bindings
```bash
# Build Lua module
cmake --preset debug -DQTFORGE_BUILD_LUA_BINDINGS=ON
cmake --build build/debug --target qtforge_lua

# Test Lua integration
lua examples/lua/basic_plugin.lua
```

### Memory and Performance Analysis
```bash
# Build with sanitizers (Debug mode)
cmake --preset debug -DQTFORGE_ENABLE_SANITIZERS=ON
cmake --build build/debug

# Run performance tests
python tests/performance/run_performance_tests.py
```

### Troubleshooting Common Issues

**Build Issues:**
```bash
# Qt6 not found
# Solution: Set CMAKE_PREFIX_PATH or Qt6_DIR (see Environment Setup)

# Python bindings compilation errors
# Solution: Ensure Python 3.8+ and pybind11 are available
# Check: python --version && python -c "import pybind11; print(pybind11.__version__)"

# Network/SQL features disabled
# Solution: Install Qt6 Network/SQL modules or use feature flags to disable
```

**Test Failures:**
```bash
# Python tests can't import qtforge
# Solution: Set PYTHONPATH=build/src/python before running tests

# CTest failures
# Solution: Run with verbose output: ctest --test-dir build -V

### MSYS2 Development
```bash
# Build in MSYS2 environment
export MSYSTEM=MINGW64  # or UCRT64
python scripts/build.py --build-type Debug --tests --examples

# Install dependencies in MSYS2
pacman -S mingw-w64-x86_64-qt6-base mingw-w64-x86_64-cmake
```

## Development Workflow

### Commit Guidelines

- **Follow Conventional Commits**: Use `feat:`, `fix:`, `docs:`, `refactor:` prefixes
- **Use imperative mood**: "Add feature" not "Added feature"
- **Examples from git history**: `feat(ui_plugin): add new widget support`, `fix(loader): resolve memory leak in plugin cleanup`

### Pull Request Requirements

- Clear description with linked issues
- Include test results (CMake build + CTest passing)
- Update documentation when user-visible changes are made
- Screenshots for UI-related changes
- Code formatting with `clang-format` (uses `.clang-format` config)
- Update `CHANGELOG.md` when applicable

### Pre-merge Checklist

```bash
# Ensure clean build
cmake --build build --parallel

# All tests passing
ctest --test-dir build --output-on-failure

# Python tests passing
python tests/run_all_tests.py

# Code formatting applied
clang-format -i src/**/*.cpp include/**/*.hpp
```

## Important Notes

- **Production Status**: QtForge v3.2.0 is fully functional with 100% test success rate (3/3 tests passing)
- **Platform Support**: Windows, Linux, macOS, Android, iOS with cross-compilation toolchains
- **Thread Safety**: All public APIs are thread-safe using shared_mutex and atomic operations
- **Error Handling**: Modern C++20 `std::expected<T,E>` pattern throughout the codebase
- **Plugin Hot Reload**: Runtime plugin reloading with state preservation during development
- **Security**: Plugin sandboxing with configurable isolation and permission policies
- **Language Support**: Complete Python bindings with type stubs and Lua integration via sol2
- **Build System**: Modular CMake with presets for different platforms and configurations
- **Testing**: Comprehensive test coverage including unit, integration, and performance tests
