# QtForge Tests

This directory contains comprehensive tests for the QtForge library, organized by module for better maintainability and clarity.

## Directory Structure

The tests are organized into modules that correspond to the main QtForge architecture:

```
tests/
├── core/                    # Core plugin system tests
│   ├── test_plugin_interface*.cpp
│   ├── test_plugin_manager*.cpp
│   ├── test_lifecycle_*.cpp
│   └── test_component_architecture.cpp
├── communication/           # Inter-plugin communication tests
│   ├── test_message_bus*.cpp
│   └── test_service_contracts.cpp
├── security/               # Security and validation tests
│   └── test_security_manager*.cpp
├── managers/               # Manager components tests
│   ├── test_configuration_manager.cpp
│   ├── test_plugin_version_manager.cpp
│   ├── test_resource_management.cpp
│   └── test_manager_component_integration.cpp
├── monitoring/             # Performance and monitoring tests
│   ├── test_performance.cpp
│   └── test_component_performance.cpp
├── orchestration/          # Plugin orchestration tests
│   └── test_plugin_orchestration.cpp
├── utils/                  # Utility classes tests
│   ├── test_version*.cpp
│   ├── test_error_handling*.cpp
│   └── test_expected_comprehensive.cpp
├── platform/               # Platform-specific tests
│   └── test_cross_platform.cpp
├── integration/            # Cross-module integration tests
│   └── (reserved for future integration tests)
└── build_system/           # Build system tests
    └── test_build_system.cmake
```

## Test Categories

### Basic Tests

- Simple, focused tests for individual components
- Quick execution (< 60 seconds)
- Essential functionality verification

### Comprehensive Tests

- Detailed tests with extensive coverage
- Longer execution time (60-300 seconds)
- Advanced scenarios and edge cases
- Enabled with `QTFORGE_BUILD_COMPREHENSIVE_TESTS=ON`

### Performance Tests

- Benchmarking and performance validation
- Resource usage monitoring
- Enabled with `QTFORGE_BUILD_PERFORMANCE_TESTS=ON`

### Cross-Platform Tests

- Platform-specific behavior validation
- Cross-compilation verification
- Enabled with `QTFORGE_BUILD_CROSS_PLATFORM_TESTS=ON`

## Building and Running Tests

### Quick Start

```bash
# Configure with tests enabled
cmake -S . -B build -DQTFORGE_BUILD_TESTS=ON

# Build all tests
cmake --build build

# Run all tests
cd build && ctest --verbose
```

### Module-Specific Testing

```bash
# Run only core tests
ctest -R "Core" --verbose

# Run only communication tests
ctest -R "Communication" --verbose

# Run only security tests
ctest -R "Security" --verbose
```

### Test Configuration Options

```bash
# Enable comprehensive tests (longer execution)
-DQTFORGE_BUILD_COMPREHENSIVE_TESTS=ON

# Enable performance tests
-DQTFORGE_BUILD_PERFORMANCE_TESTS=ON

# Enable cross-platform tests
-DQTFORGE_BUILD_CROSS_PLATFORM_TESTS=ON

# Enable memory leak detection (Linux/macOS with Valgrind)
-DQTFORGE_BUILD_MEMORY_TESTS=ON
```

## Test Naming Convention

- **Basic tests**: `test_<component>.cpp`
- **Comprehensive tests**: `test_<component>_comprehensive.cpp` or `test_<component>.cpp` (when comprehensive)
- **Simple tests**: `test_<component>_simple.cpp` (legacy, being phased out)
- **Performance tests**: `test_<component>_performance.cpp`

## Migration from Legacy Structure

The tests have been reorganized from a flat structure to a modular one:

- **Old**: All tests in `tests/` root directory
- **New**: Tests organized by module in subdirectories
- **Benefits**: Better organization, easier maintenance, clearer module boundaries

Legacy test files have been moved to appropriate module directories and renamed for consistency.

Run with:

```
mkdir build && cd build
cmake .. -DQTPLUGIN_BUILD_TESTS=ON
ctest --output-on-failure
```
