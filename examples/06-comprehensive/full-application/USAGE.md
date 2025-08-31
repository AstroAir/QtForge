# QtForge Comprehensive Example - Usage Guide

This guide provides detailed instructions on how to build, run, and understand the comprehensive example that demonstrates **ALL** QtForge features.

## 🚀 Quick Start

### Prerequisites

- **Qt6** (6.0 or later) with Core, Widgets, Network modules
- **CMake** 3.21 or later
- **C++20** compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- **QtForge** library v3.0.0 or later

### Build and Run

#### Linux/macOS
```bash
# Clone and navigate to the example
cd examples/comprehensive_example

# Build with default settings
./build.sh

# Build with all features enabled
./build.sh --with-python --with-docs --run

# Build and run immediately
./build.sh --clean --run
```

#### Windows
```cmd
# Navigate to the example
cd examples\comprehensive_example

# Build with default settings
build.bat

# Build with all features enabled
build.bat --with-python --with-docs --run

# Build and run immediately
build.bat --clean --run
```

## 📋 Build Options

### Common Build Options

| Option | Description |
|--------|-------------|
| `--help` | Show help message |
| `--clean` | Clean build directory before building |
| `--debug` | Build in Debug mode |
| `--release` | Build in Release mode (default) |
| `--install` | Install after building |
| `--package` | Create distribution package |
| `--run` | Run demo after building |

### Feature Options

| Option | Description |
|--------|-------------|
| `--with-python` | Enable Python bridge support |
| `--with-docs` | Enable documentation generation |
| `--with-coverage` | Enable code coverage |
| `--no-tests` | Disable unit tests |

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `BUILD_TYPE` | Release | Build configuration |
| `BUILD_DIR` | build | Build directory |
| `INSTALL_PREFIX` | /usr/local | Installation prefix |
| `PARALLEL_JOBS` | auto | Number of parallel build jobs |

## 🎯 Running the Demo

### Basic Usage

```bash
# Run with default settings
./build/comprehensive_demo

# Run with custom plugin directory
./build/comprehensive_demo --plugin-dir=./plugins

# Run with high security level
./build/comprehensive_demo --security-level=high

# Run with Python support enabled
./build/comprehensive_demo --enable-python

# Run with UI components enabled
./build/comprehensive_demo --enable-ui
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `--plugin-dir=DIR` | Plugin directory path |
| `--enable-python` | Enable Python bridge |
| `--enable-ui` | Enable UI components |
| `--security-level=LEVEL` | Security level (low/medium/high) |
| `--help` | Show help message |
| `--version` | Show version information |

### Expected Output

```
🚀 QtForge Comprehensive Demo v3.0.0
=====================================

[INIT] Initializing QtForge library...
[CORE] Plugin manager initialized
[SECURITY] Security level set to MEDIUM
[COMMUNICATION] Message bus started
[MONITORING] Metrics collection enabled
[ORCHESTRATION] Workflow engine ready
[TRANSACTIONS] Transaction manager active
[MARKETPLACE] Plugin discovery enabled
[THREADING] Thread pool (8 threads) ready

[LOADING] Loading plugins from ./plugins...
✅ ComprehensivePlugin v3.0.0 (com.qtforge.comprehensive_plugin)

[DEMO] Demonstrating all features...

--- Communication Demo ---
✅ Message published to demo.test topic

--- Security Demo ---
✅ Security validation completed

--- Workflow Demo ---
✅ Workflow created with 3 steps

--- Performance Demo ---
✅ Performance metrics collected

=== System Status ===
Loaded plugins: 1
Security level: 1
Python support: Disabled
UI support: Disabled

=== Performance Metrics ===
Total runtime: 1250ms
Messages processed: 5
Transactions completed: 3
Average message rate: 4.0 msg/s

🎉 [SUCCESS] All features demonstrated successfully!
```

## 🐍 Python Integration

### Running Python Demo

```bash
# Ensure Python support is built
./build.sh --with-python

# Run Python demo
cd build
python3 ../python/comprehensive_demo.py

# Run with options
python3 ../python/comprehensive_demo.py --disable-security --disable-monitoring
```

### Python Demo Features

The Python demo demonstrates:
- Plugin management from Python
- Inter-plugin communication
- Security validation
- Metrics collection
- Workflow orchestration
- Transaction management
- Marketplace integration
- Threading capabilities

## 🧪 Testing

### Running Tests

```bash
# Run all tests
cd build
ctest --output-on-failure

# Run specific test
./test_comprehensive_plugin

# Run with verbose output
ctest -V

# Run performance tests
ctest -R performance
```

### Test Coverage

```bash
# Build with coverage
./build.sh --with-coverage

# Generate coverage report
cd build
make coverage

# View coverage report
open coverage/index.html  # macOS
xdg-open coverage/index.html  # Linux
```

## 📊 Features Demonstrated

### Core Plugin System
- ✅ Plugin loading and management
- ✅ Plugin lifecycle management
- ✅ Plugin dependency resolution
- ✅ Plugin registry and discovery
- ✅ Hot reload capabilities

### Communication System
- ✅ Message bus for inter-plugin communication
- ✅ Request-response system
- ✅ Event publishing and subscription
- ✅ Message filtering and routing

### Security Management
- ✅ Plugin validation and verification
- ✅ Security level enforcement
- ✅ Trust management
- ✅ Permission control

### Monitoring & Metrics
- ✅ Real-time plugin monitoring
- ✅ Performance metrics collection
- ✅ System health monitoring
- ✅ Hot reload management

### Orchestration & Workflows
- ✅ Plugin orchestration
- ✅ Workflow definition and execution
- ✅ Step-by-step processing

### Transaction Management
- ✅ ACID transaction support
- ✅ Rollback capabilities
- ✅ Transaction monitoring

### Additional Features
- ✅ Plugin composition patterns
- ✅ Marketplace integration
- ✅ Threading and concurrency
- ✅ Python bridge support
- ✅ Configuration management
- ✅ Error handling with expected<T,E>

## 🔧 Configuration

### Application Configuration

Edit `config/application.json` to customize:

```json
{
  "core": {
    "plugin_directory": "./plugins",
    "auto_load_plugins": true,
    "hot_reload_enabled": true
  },
  "security": {
    "enabled": true,
    "level": "medium",
    "signature_verification": true
  },
  "monitoring": {
    "enabled": true,
    "metrics_collection": {
      "interval_ms": 5000
    }
  }
}
```

### Plugin Configuration

The comprehensive plugin supports these configuration options:

```json
{
  "communication_enabled": true,
  "monitoring_enabled": true,
  "security_enabled": true,
  "networking_enabled": true,
  "background_processing_enabled": true,
  "python_integration_enabled": false,
  "metrics_interval": 5000,
  "health_check_interval": 10000
}
```

## 🐛 Troubleshooting

### Common Issues

#### Build Failures

**Qt6 not found:**
```bash
# Install Qt6 development packages
sudo apt-get install qt6-base-dev  # Ubuntu/Debian
brew install qt6  # macOS
```

**CMake version too old:**
```bash
# Install newer CMake
pip install cmake  # Via pip
# Or download from https://cmake.org/download/
```

#### Runtime Issues

**Plugin loading fails:**
- Check plugin directory exists: `ls -la plugins/`
- Verify plugin file permissions
- Check QtForge library is properly installed

**Python integration fails:**
- Ensure Python support was built: `--with-python`
- Check Python path: `export PYTHONPATH=$PYTHONPATH:$(pwd)`
- Verify qtforge Python module is available

### Debug Mode

```bash
# Build in debug mode
./build.sh --debug

# Run with debug output
QT_LOGGING_RULES="qtforge.*=true" ./build/comprehensive_demo_d

# Enable verbose logging
export QTFORGE_DEBUG=1
export QTFORGE_LOG_LEVEL=debug
```

## 📚 Learning Path

### Beginner
1. Run the basic demo to see all features in action
2. Examine the main.cpp to understand initialization
3. Study the plugin interface implementation
4. Explore the configuration system

### Intermediate
5. Modify the plugin to add custom commands
6. Implement inter-plugin communication
7. Add custom monitoring metrics
8. Create workflow definitions

### Advanced
9. Implement custom security policies
10. Add marketplace integration
11. Create Python plugin extensions
12. Optimize performance and memory usage

## 🤝 Contributing

To extend this example:

1. **Add new features** to the comprehensive plugin
2. **Create additional plugins** demonstrating specific capabilities
3. **Enhance Python integration** with more examples
4. **Improve documentation** and tutorials
5. **Add performance benchmarks** and optimizations

## 📄 License

This comprehensive example is provided under the MIT License, same as the QtForge library.
