# QtForge Python Bindings Unit Tests

This directory contains comprehensive unit tests for the QtForge Python bindings. Each test file focuses on testing individual functions and classes with 100% API coverage, edge cases, and error conditions.

## 📁 Test Files

### Core Module Tests
- **`test_core_unit.py`** - Tests for core plugin system components
  - PluginManager, PluginLoader, PluginRegistry
  - Plugin interfaces and lifecycle management
  - Dependency resolution and plugin states
  - Enums: PluginState, PluginCapability, PluginPriority, PluginLifecycleEvent

### Utility Module Tests
- **`test_utils_unit.py`** - Tests for utility functions and classes
  - Version handling and comparison
  - String manipulation utilities
  - File system operations
  - JSON parsing and validation
  - Logging utilities
  - Time and date functions
  - Error handling utilities

### Communication Module Tests
- **`test_communication_unit.py`** - Tests for communication system
  - MessageBus creation and operations
  - Message publishing and subscription
  - Request/Response patterns
  - Service contracts and communication protocols
  - JSON serialization for messages

### Security Module Tests
- **`test_security_unit.py`** - Tests for security components
  - SecurityManager and security policies
  - Plugin signature verification
  - Permission management and trust levels
  - Security validation and threat scanning
  - Plugin integrity checking

### Manager Module Tests
- **`test_managers_unit.py`** - Tests for management components
  - ConfigurationManager and settings
  - LoggingManager and log handling
  - ResourceManager and resource allocation
  - Plugin version management

### Orchestration Module Tests
- **`test_orchestration_unit.py`** - Tests for plugin orchestration
  - Workflow creation and execution
  - Step management and sequencing
  - Plugin coordination and dependencies
  - Execution modes and error handling

### Monitoring Module Tests
- **`test_monitoring_unit.py`** - Tests for monitoring system
  - Hot reload functionality
  - Metrics collection and reporting
  - Performance monitoring
  - Health checks and status reporting

### Transaction Module Tests
- **`test_transactions_unit.py`** - Tests for transaction management
  - Transaction creation and lifecycle
  - Rollback and commit operations
  - Transaction isolation and consistency
  - Error handling in transactions

### Composition Module Tests
- **`test_composition_unit.py`** - Tests for plugin composition
  - Composition patterns and strategies
  - Plugin binding and integration
  - Pipeline and facade compositions
  - Composite plugin management

### Marketplace Module Tests
- **`test_marketplace_unit.py`** - Tests for plugin marketplace
  - Plugin search and discovery
  - Rating and review systems
  - Plugin installation and updates
  - Marketplace integration

### Threading Module Tests
- **`test_threading_unit.py`** - Tests for threading and concurrency
  - Thread pool management
  - Concurrent plugin execution
  - Thread safety and synchronization
  - Async operation handling

## 🚀 Running Tests

### Run All Unit Tests
```bash
# From the unit test directory
python test_unit_runner.py

# Or using pytest directly
pytest -v
```

### Run Specific Test Module
```bash
# Run core tests only
python -m pytest test_core_unit.py -v

# Run utils tests only
python -m pytest test_utils_unit.py -v

# Run communication tests only
python -m pytest test_communication_unit.py -v
```

### Run with Coverage Analysis
```bash
# Run all tests with coverage
python test_unit_runner.py --coverage

# Or using coverage directly
coverage run -m pytest
coverage report
coverage html  # Generate HTML report
```

### Generate Detailed Report
```bash
# Generate comprehensive test report
python test_unit_runner.py --report
```

## 📊 Test Categories

### 1. **API Coverage Tests**
- Test every exposed function, class, and method
- Verify correct return types and values
- Test all public properties and attributes
- Ensure all enums and constants are accessible

### 2. **Edge Case Tests**
- Null/None input handling
- Empty string and container handling
- Boundary value testing
- Large input handling
- Invalid parameter combinations

### 3. **Error Condition Tests**
- Exception handling and propagation
- Invalid file paths and missing resources
- Network failures and timeouts
- Memory exhaustion scenarios
- Concurrent access issues

### 4. **Type Safety Tests**
- Parameter type validation
- Return type verification
- Type conversion handling
- Generic type parameter testing

### 5. **Performance Tests**
- Function execution timing
- Memory usage validation
- Resource cleanup verification
- Scalability testing

## 🔧 Test Structure

Each test file follows a consistent structure:

```python
class TestClassName:
    """Test specific class functionality."""
    
    def test_creation(self):
        """Test object creation."""
        
    def test_methods(self):
        """Test public methods."""
        
    def test_properties(self):
        """Test properties and attributes."""
        
    def test_edge_cases(self):
        """Test edge cases and error conditions."""
        
    def test_repr(self):
        """Test string representation."""
```

## 📋 Test Requirements

### Prerequisites
- Python 3.8 or later
- pytest framework
- QtForge Python bindings built and available
- Optional: coverage tool for coverage analysis

### Installation
```bash
# Install test dependencies
pip install pytest pytest-cov coverage

# Build QtForge with Python bindings
cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON -B build -S .
cmake --build build
```

## 🎯 Coverage Goals

- **100% API Coverage**: Every public function, class, and method tested
- **Edge Case Coverage**: All boundary conditions and error scenarios
- **Error Handling Coverage**: All exception paths and error conditions
- **Integration Coverage**: Cross-module interactions and dependencies

## 📈 Test Metrics

The test runner provides detailed metrics:
- Total test count and execution time
- Pass/fail/skip/error counts
- Individual test timing
- Coverage percentages
- Memory usage statistics

## 🐛 Debugging Tests

### Verbose Output
```bash
# Run with maximum verbosity
pytest -vvv test_core_unit.py

# Show local variables on failure
pytest --tb=long test_core_unit.py
```

### Debug Specific Test
```bash
# Run single test with debugging
pytest -vvv test_core_unit.py::TestPluginManager::test_plugin_manager_creation
```

### Capture Output
```bash
# Show print statements
pytest -s test_core_unit.py

# Capture and show on failure
pytest --capture=no test_core_unit.py
```

## 🔄 Continuous Integration

These unit tests are designed to run in CI/CD environments:

```yaml
# Example GitHub Actions workflow
- name: Run Unit Tests
  run: |
    cd tests/python/unit
    python test_unit_runner.py --coverage
    
- name: Upload Coverage
  uses: codecov/codecov-action@v1
  with:
    file: ./coverage.xml
```

## 📝 Contributing

When adding new unit tests:

1. **Follow Naming Convention**: `test_<module>_unit.py`
2. **Comprehensive Coverage**: Test all public APIs
3. **Error Scenarios**: Include edge cases and error conditions
4. **Documentation**: Add clear docstrings for test purposes
5. **Isolation**: Ensure tests don't depend on external resources
6. **Performance**: Keep tests fast and efficient

## 🔍 Test Discovery

Tests are automatically discovered by:
- pytest's automatic discovery
- The custom test runner
- IDE test runners (PyCharm, VSCode)

Test files must:
- Start with `test_` prefix
- Contain classes starting with `Test`
- Have methods starting with `test_`

## 📚 Additional Resources

- [pytest Documentation](https://docs.pytest.org/)
- [Python unittest Documentation](https://docs.python.org/3/library/unittest.html)
- [Coverage.py Documentation](https://coverage.readthedocs.io/)
- [QtForge Documentation](../../docs/)
