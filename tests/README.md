# QtForge Tests

This directory contains comprehensive tests for the QtForge library, including both C++ core tests and language binding tests (Python and Lua).

## Directory Structure

The tests are organized into multiple categories:

```
tests/
├── core/                    # Core plugin system tests (C++)
│   ├── test_plugin_interface*.cpp
│   ├── test_plugin_manager*.cpp
│   ├── test_lifecycle_*.cpp
│   └── test_component_architecture.cpp
├── communication/           # Inter-plugin communication tests (C++)
│   ├── test_message_bus*.cpp
│   └── test_service_contracts.cpp
├── security/               # Security and validation tests (C++)
│   └── test_security_manager*.cpp
├── managers/               # Manager components tests (C++)
│   ├── test_configuration_manager.cpp
│   ├── test_plugin_version_manager.cpp
│   ├── test_resource_management.cpp
│   └── test_manager_component_integration.cpp
├── monitoring/             # Performance and monitoring tests (C++)
│   ├── test_performance.cpp
│   └── test_component_performance.cpp
├── orchestration/          # Plugin orchestration tests (C++)
│   └── test_plugin_orchestration.cpp
├── utils/                  # Utility classes tests (C++)
│   ├── test_version*.cpp
│   ├── test_error_handling*.cpp
│   └── test_expected_comprehensive.cpp
├── platform/               # Platform-specific tests (C++)
│   └── test_cross_platform.cpp
├── integration/            # Cross-module integration tests (C++)
│   └── (reserved for future integration tests)
├── build_system/           # Build system tests
│   └── test_build_system.cmake
├── python/                 # Python binding tests
│   ├── unit/               # Unit tests for individual functions/classes
│   │   ├── test_core_unit.py
│   │   ├── test_utils_unit.py
│   │   ├── test_communication_unit.py
│   │   ├── test_security_unit.py
│   │   ├── test_unit_runner.py
│   │   └── README.md
│   ├── integration/        # Integration and cross-language tests
│   │   ├── test_cross_language_integration.py
│   │   └── README.md
│   ├── test_comprehensive_*.py  # Legacy comprehensive tests
│   └── test_runner.py
└── lua/                    # Lua binding tests
    ├── unit/               # Unit tests for individual functions/classes
    │   ├── test_core_unit.lua
    │   ├── test_utils_unit.lua
    │   ├── test_unit_runner.lua
    │   └── README.md
    ├── integration/        # Integration and cross-language tests
    │   ├── test_cross_language_integration.lua
    │   └── README.md
    ├── test_comprehensive_*.lua  # Legacy comprehensive tests
    └── test_runner.lua
```

## Test Categories

### C++ Core Tests

- **Basic Tests** - Simple, focused tests for individual components
- **Comprehensive Tests** - Detailed testing with edge cases and error handling
- **Integration Tests** - Cross-module interaction testing
- **Performance Tests** - Benchmarking and performance validation

### Python Binding Tests

- **`test_comprehensive_core.py`** - Core plugin system functionality
- **`test_comprehensive_utils.py`** - Utility functions and helpers
- **`test_comprehensive_communication.py`** - Message bus and communication
- **`test_comprehensive_security.py`** - Security and validation systems
- **`test_comprehensive_managers.py`** - Configuration, logging, and resource management
- **`test_comprehensive_orchestration.py`** - Workflow and orchestration
- **`test_comprehensive_monitoring.py`** - Hot reload and metrics collection
- **`test_comprehensive_remaining_modules.py`** - Transactions, composition, marketplace, threading
- **`test_runner.py`** - Unified test runner with detailed reporting

### Lua Binding Tests

- **`test_comprehensive_core.lua`** - Core plugin system functionality
- **`test_comprehensive_utils.lua`** - Utility functions and helpers
- **`test_comprehensive_remaining_modules.lua`** - All other modules (communication, security, etc.)
- **`test_runner.lua`** - Unified test runner with detailed reporting

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

### Language Binding Tests

#### New Comprehensive Binding Test Suite

The binding tests have been completely redesigned for comprehensive coverage:

**Python Binding Tests:**
- **Unit Tests** (`tests/python/unit/`) - Individual function/class testing with 100% API coverage
  - `test_core_unit.py` - Core plugin system components
  - `test_utils_unit.py` - Utility functions and helpers
  - `test_communication_unit.py` - Message bus and communication
  - `test_security_unit.py` - Security and validation systems
  - `test_unit_runner.py` - Automated test runner with coverage analysis
- **Integration Tests** (`tests/python/integration/`) - Cross-language and threading safety
  - `test_cross_language_integration.py` - Python ↔ Lua interoperability testing

**Lua Binding Tests:**
- **Unit Tests** (`tests/lua/unit/`) - Individual function/class testing with 100% API coverage
  - `test_core_unit.lua` - Core plugin system components
  - `test_utils_unit.lua` - Utility functions and helpers
  - `test_unit_runner.lua` - Automated test runner with detailed reporting
- **Integration Tests** (`tests/lua/integration/`) - Cross-language and performance testing
  - `test_cross_language_integration.lua` - Lua ↔ Python interoperability testing

**Key Features:**
- **100% API Coverage** - Every exposed function, class, and method tested
- **Edge Case Testing** - Null inputs, boundary conditions, invalid parameters
- **Error Handling** - Exception propagation and resource cleanup validation
- **Memory Management** - Resource leak detection and cleanup verification
- **Threading Safety** - Concurrent access and thread pool testing
- **Performance Benchmarking** - Function timing and memory usage analysis
- **Cross-Language Consistency** - Identical behavior across Python and Lua bindings

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

# Enable Python bindings and tests
-DQTFORGE_BUILD_PYTHON_BINDINGS=ON

# Enable Lua bindings and tests
-DQTFORGE_BUILD_LUA_BINDINGS=ON
```

### Running Language Binding Tests

#### Python Tests

```bash
# Prerequisites
pip install pytest pytest-cov coverage psutil

# Run all Python unit tests
cd tests/python/unit && python test_unit_runner.py

# Run with coverage analysis
cd tests/python/unit && python test_unit_runner.py --coverage

# Run specific module tests
python -m pytest tests/python/unit/test_core_unit.py -v
python -m pytest tests/python/unit/test_utils_unit.py -v

# Run integration tests
python -m pytest tests/python/integration/ -v

# Run legacy comprehensive tests
python tests/python/test_runner.py

# Run with pytest directly
cd tests/python && pytest -v
```

#### Lua Tests

```bash
# Run all Lua unit tests
cd tests/lua/unit && lua test_unit_runner.lua

# Run specific module tests
cd tests/lua/unit && lua test_unit_runner.lua core
cd tests/lua/unit && lua test_unit_runner.lua utils

# Generate detailed report
cd tests/lua/unit && lua test_unit_runner.lua --report

# Run integration tests
lua tests/lua/integration/test_cross_language_integration.lua

# Run legacy comprehensive tests
lua tests/lua/test_runner.lua

# Run specific test file
lua tests/lua/unit/test_core_unit.lua
lua tests/lua/unit/test_utils_unit.lua
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

## Complete Test Suite Execution

Run all tests (C++, Python, and Lua):

```bash
# Build with all bindings enabled
cmake -S . -B build \
  -DQTFORGE_BUILD_TESTS=ON \
  -DQTFORGE_BUILD_PYTHON_BINDINGS=ON \
  -DQTFORGE_BUILD_LUA_BINDINGS=ON

# Build
cmake --build build

# Run C++ tests
cd build && ctest --verbose && cd ..

# Run Python tests
python tests/python/test_runner.py

# Run Lua tests
lua tests/lua/test_runner.lua
```

## Test Coverage and Quality

The comprehensive test suite provides:

- **100% API Coverage** - Every public function and class tested
- **Error Path Testing** - Invalid inputs and edge cases
- **Cross-Language Consistency** - Same behavior across Python and Lua
- **Memory Safety** - Resource cleanup and leak detection
- **Thread Safety** - Concurrent operation testing
- **Performance Validation** - Benchmarking and scalability testing

For detailed information about language binding tests, see:

- `tests/python/README.md` - Python binding test documentation
- `tests/lua/README.md` - Lua binding test documentation
