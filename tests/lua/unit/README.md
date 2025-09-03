# QtForge Lua Bindings Unit Tests

This directory contains comprehensive unit tests for the QtForge Lua bindings. Each test file focuses on testing individual functions and classes with 100% API coverage, edge cases, and error conditions.

## 📁 Test Files

### Core Module Tests
- **`test_core_unit.lua`** - Tests for core plugin system components
  - PluginManager, PluginLoader, PluginRegistry
  - Plugin interfaces and lifecycle management
  - Dependency resolution and plugin states
  - Enums: PluginState, PluginCapability, PluginPriority

### Utility Module Tests
- **`test_utils_unit.lua`** - Tests for utility functions and namespaces
  - JSON parsing, validation, and manipulation
  - String manipulation utilities (trim, case conversion, pattern matching)
  - File system operations (existence checks, path manipulation)
  - Logging utilities (debug, info, warning, error, critical)
  - Time and date functions (timestamps, formatting)
  - Error handling utilities

### Communication Module Tests
- **`test_communication_unit.lua`** - Tests for communication system
  - MessageBus creation and operations
  - Message publishing and subscription
  - Request/Response patterns
  - Service contracts and communication protocols
  - JSON serialization for messages

### Security Module Tests
- **`test_security_unit.lua`** - Tests for security components
  - SecurityManager and security policies
  - Plugin signature verification
  - Permission management and trust levels
  - Security validation and threat scanning
  - Plugin integrity checking

### Manager Module Tests
- **`test_managers_unit.lua`** - Tests for management components
  - ConfigurationManager and settings
  - LoggingManager and log handling
  - ResourceManager and resource allocation
  - Plugin version management

### Orchestration Module Tests
- **`test_orchestration_unit.lua`** - Tests for plugin orchestration
  - Workflow creation and execution
  - Step management and sequencing
  - Plugin coordination and dependencies
  - Execution modes and error handling

### Monitoring Module Tests
- **`test_monitoring_unit.lua`** - Tests for monitoring system
  - Hot reload functionality
  - Metrics collection and reporting
  - Performance monitoring
  - Health checks and status reporting

### Transaction Module Tests
- **`test_transactions_unit.lua`** - Tests for transaction management
  - Transaction creation and lifecycle
  - Rollback and commit operations
  - Transaction isolation and consistency
  - Error handling in transactions

### Composition Module Tests
- **`test_composition_unit.lua`** - Tests for plugin composition
  - Composition patterns and strategies
  - Plugin binding and integration
  - Pipeline and facade compositions
  - Composite plugin management

### Marketplace Module Tests
- **`test_marketplace_unit.lua`** - Tests for plugin marketplace
  - Plugin search and discovery
  - Rating and review systems
  - Plugin installation and updates
  - Marketplace integration

### Threading Module Tests
- **`test_threading_unit.lua`** - Tests for threading and concurrency
  - Thread pool management
  - Concurrent plugin execution
  - Thread safety and synchronization
  - Async operation handling

## 🚀 Running Tests

### Run All Unit Tests
```bash
# From the unit test directory
lua test_unit_runner.lua

# Or run individual test files
lua test_core_unit.lua
lua test_utils_unit.lua
```

### Run Specific Test Module
```bash
# Run core tests only
lua test_unit_runner.lua core

# Run utils tests only
lua test_unit_runner.lua utils

# Run communication tests only
lua test_unit_runner.lua communication
```

### Generate Detailed Report
```bash
# Generate comprehensive test report
lua test_unit_runner.lua --report
```

### List Available Modules
```bash
# List all available test modules
lua test_unit_runner.lua --list
```

## 📊 Test Categories

### 1. **API Coverage Tests**
- Test every exposed function, class, and method
- Verify correct return types and values
- Test all public properties and attributes
- Ensure all enums and constants are accessible

### 2. **Edge Case Tests**
- Nil input handling
- Empty string and table handling
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
- Table structure validation

### 5. **Performance Tests**
- Function execution timing
- Memory usage validation
- Resource cleanup verification
- Scalability testing

## 🔧 Test Structure

Each test file follows a consistent structure:

```lua
-- Test framework setup
local test_framework = {}

-- Test class structure
test_framework.describe("ClassName", function()
    test_framework.it("should create objects", function()
        -- Test object creation
    end)
    
    test_framework.it("should have required methods", function()
        -- Test public methods
    end)
    
    test_framework.it("should handle edge cases", function()
        -- Test edge cases and error conditions
    end)
end)
```

## 📋 Test Requirements

### Prerequisites
- Lua 5.1 or later (compatible with LuaJIT)
- QtForge Lua bindings built and available
- sol2 library for C++ binding
- Qt6 Core (and optional components)

### Installation
```bash
# Build QtForge with Lua bindings
cmake -DQTFORGE_BUILD_LUA_BINDINGS=ON -B build -S .
cmake --build build

# Ensure Lua can find the QtForge module
export LUA_PATH="./build/?.lua;./build/?/init.lua;$LUA_PATH"
export LUA_CPATH="./build/?.so;./build/?/?.so;$LUA_CPATH"
```

## 🎯 Coverage Goals

- **100% API Coverage**: Every public function, class, and method tested
- **Edge Case Coverage**: All boundary conditions and error scenarios
- **Error Handling Coverage**: All exception paths and error conditions
- **Integration Coverage**: Cross-module interactions and dependencies

## 📈 Test Metrics

The test runner provides detailed metrics:
- Total test count and execution time
- Pass/fail/error counts
- Individual test timing
- Module-specific results
- Error details and stack traces

## 🐛 Debugging Tests

### Verbose Output
```bash
# Run with detailed output
lua -e "debug.traceback = function(msg) print(debug.traceback(msg, 2)) end" test_core_unit.lua
```

### Debug Specific Test
```bash
# Add debug prints to specific test
lua -e "
local original_test = test_framework.it
test_framework.it = function(desc, func)
    print('Running: ' .. desc)
    return original_test(desc, func)
end
" test_core_unit.lua
```

### Error Handling
```bash
# Run with error handling
lua -e "
local success, result = pcall(dofile, 'test_core_unit.lua')
if not success then
    print('Error:', result)
end
"
```

## 🔄 Continuous Integration

These unit tests are designed to run in CI/CD environments:

```yaml
# Example GitHub Actions workflow
- name: Run Lua Unit Tests
  run: |
    cd tests/lua/unit
    lua test_unit_runner.lua --report
    
- name: Upload Test Results
  uses: actions/upload-artifact@v2
  with:
    name: lua-test-results
    path: tests/lua/unit/lua_unit_test_report.txt
```

## 📝 Contributing

When adding new unit tests:

1. **Follow Naming Convention**: `test_<module>_unit.lua`
2. **Comprehensive Coverage**: Test all public APIs
3. **Error Scenarios**: Include edge cases and error conditions
4. **Documentation**: Add clear comments for test purposes
5. **Isolation**: Ensure tests don't depend on external resources
6. **Performance**: Keep tests fast and efficient

## 🔍 Test Discovery

Tests are automatically discovered by:
- The custom test runner (`test_unit_runner.lua`)
- Manual execution of individual test files
- IDE Lua runners (if configured)

Test files must:
- Start with `test_` prefix
- End with `_unit.lua` suffix
- Use the test framework structure
- Return appropriate exit codes

## 📚 Additional Resources

- [Lua 5.1 Reference Manual](https://www.lua.org/manual/5.1/)
- [sol2 Documentation](https://sol2.readthedocs.io/)
- [QtForge Documentation](../../docs/)
- [Lua Testing Best Practices](https://github.com/lunarmodules/busted)

## 🔧 Test Framework

The tests use a simple custom test framework with the following functions:

- `test_framework.describe(name, func)` - Group related tests
- `test_framework.it(description, func)` - Individual test case
- `test_framework.assert_true(condition, message)` - Assert true condition
- `test_framework.assert_false(condition, message)` - Assert false condition
- `test_framework.assert_equal(actual, expected, message)` - Assert equality
- `test_framework.assert_not_nil(value, message)` - Assert non-nil value
- `test_framework.assert_nil(value, message)` - Assert nil value
- `test_framework.assert_type(value, type, message)` - Assert type

## 🚨 Common Issues

### QtForge Module Not Found
```bash
# Ensure the module is in Lua's search path
export LUA_CPATH="./build/?.so;$LUA_CPATH"
```

### Sol2 Binding Errors
```bash
# Check that sol2 is properly linked
ldd build/qtforge.so | grep sol
```

### Test Execution Errors
```bash
# Run with error details
lua -e "
local success, err = pcall(dofile, 'test_core_unit.lua')
if not success then
    print('Detailed error:', err)
    print(debug.traceback())
end
"
```
