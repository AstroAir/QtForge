# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

QtForge is a modern C++ Plugin System for Qt Applications with comprehensive plugin management capabilities. It features a modular architecture supporting multiple build systems (CMake, Meson, XMake), multi-language plugin bridges (Python, Lua), and advanced plugin orchestration.

## Build System & Commands

### Primary Build System (CMake)

QtForge uses CMake as its primary build system with comprehensive cross-platform support.

#### Quick Build Commands

```bash
# Configure and build with CMake presets (recommended)
cmake --preset default
cmake --build build/default --config Release

# Manual configuration for development
cmake -S . -B build -DQTFORGE_BUILD_EXAMPLES=ON -DQTFORGE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# Build with all features enabled
cmake -S . -B build -DQTFORGE_BUILD_EXAMPLES=ON -DQTFORGE_BUILD_TESTS=ON -DQTFORGE_BUILD_NETWORK=ON -DQTFORGE_BUILD_UI=ON -DQTFORGE_BUILD_PYTHON_BINDINGS=ON -DQTFORGE_BUILD_LUA_BINDINGS=ON
```

#### Available CMake Presets

- `default`: Default configuration with auto-detected settings
- `debug`: Debug build with sanitizers and logging enabled
- `release`: Optimized release build with LTO
- `windows-msvc`: Windows build using MSVC compiler
- `linux-gcc`: Linux build using GCC compiler
- `macos`: macOS build configuration

#### Testing Commands

```bash
# Run all tests using CTest
ctest --test-dir build --output-on-failure

# Run specific test with verbose output
ctest -R "PluginManager" -V

# Run automated test suite (PowerShell on Windows)
powershell -ExecutionPolicy Bypass -File run_all_tests.ps1

# Run individual test executables
./build/default/examples/01-fundamentals/basic-plugin/Release/BasicPluginTest.exe
```

#### Python Bindings Testing

```bash
# Set up Python path and run tests
PYTHONPATH=build/src/python python tests/run_all_tests.py

# Run specific Python tests
python tests/test_qtforge_bindings.py
python -m unittest discover tests -p "test_*.py"
```

### Alternative Build Systems

#### Meson Build

```bash
# Configure and build with Meson
meson setup build --buildtype=release
meson compile -C build

# Run tests
meson test -C build --verbose
```

#### XMake Build

```bash
# Configure and build with XMake
xmake config --mode=release
xmake build

# Run tests
xmake test
```

### Cross-Platform Build Scripts

```bash
# Windows
scripts\build.bat

# Unix/Linux/macOS
scripts/build.sh

# Python cross-platform helper
python scripts/build.py --build-type Release --tests --examples
```

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

### Key Files & Locations

- **Main Header**: `include/qtplugin/qtplugin.hpp`
- **Examples**: `examples/` (C++ and Python usage patterns)
- **Tests**: `tests/` (comprehensive test coverage)
- **Build Scripts**: `scripts/` (cross-platform build helpers)
- **Documentation**: `docs/` (user and developer guides)
- **Python Stubs**: `qtforge/` (type hints for Python IDEs)

### Performance Considerations

- **Plugin Loading**: Lazy loading with dependency resolution
- **Memory Usage**: Resource pools and lifecycle management
- **Thread Safety**: Read-write locks for concurrent access
- **Hot Reload**: Minimal disruption plugin reloading
- **Background Tasks**: Efficient task queuing and processing

## Important Notes

- QtForge is fully functional with 100% test success rate (3/3 tests passing)
- The build system supports multiple platforms: Windows, Linux, macOS, Android, iOS
- Plugin hot reloading preserves application state during development
- Security sandbox provides configurable isolation for untrusted plugins
- All APIs are thread-safe and support concurrent plugin operations
- The library includes comprehensive error handling with structured error types
- Python bindings include full type stubs for excellent IDE support
- Lua plugins run in sandboxed environments with configurable permissions
