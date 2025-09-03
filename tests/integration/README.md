# QtForge Cross-Language Integration Tests

This directory contains integration tests that verify interoperability and consistency between QtForge's Python and Lua bindings. These tests ensure that both language bindings provide equivalent functionality and maintain consistent behavior.

## Overview

The integration tests validate:

- **API Consistency** - Same functionality available in both languages
- **Behavioral Consistency** - Identical behavior across language bindings
- **Error Handling Consistency** - Similar error handling patterns
- **Data Type Compatibility** - Proper type mapping and conversion
- **Enum Value Synchronization** - Consistent enumeration values
- **Cross-Language Interoperability** - Ability to work together in hybrid applications

## Test Files

### `test_cross_language_integration.py`

**Python-based integration tests** that verify:
- Plugin manager consistency between Python and Lua
- Message bus interoperability
- Configuration sharing capabilities
- Security policy consistency
- Enum value consistency
- Error handling patterns

**Key Features:**
- Runs Lua scripts from Python to compare results
- Uses subprocess to execute Lua code
- Comprehensive error handling and reporting
- Detailed output with success/warning/error indicators

### `test_cross_language_integration.lua`

**Lua-based integration tests** that verify:
- Plugin manager consistency from Lua perspective
- Message bus creation and functionality
- Enum value verification
- Error handling consistency
- Configuration manager availability

**Key Features:**
- Runs Python scripts from Lua to compare results
- Uses io.popen to execute Python code
- Lua-specific error handling with pcall
- Consistent output formatting with Python tests

### `run_integration_tests.py`

**Unified test runner** that:
- Executes both Python and Lua integration tests
- Provides comprehensive reporting
- Checks prerequisites and dependencies
- Generates summary reports with recommendations

## Running the Tests

### Prerequisites

1. **Build QtForge with both bindings:**
   ```bash
   cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON -DQTFORGE_BUILD_LUA_BINDINGS=ON ..
   make
   ```

2. **Install required interpreters:**
   ```bash
   # Python (usually pre-installed)
   python --version
   
   # Lua
   lua -v
   ```

### Running All Integration Tests

```bash
# From QtForge root directory
python tests/integration/run_integration_tests.py
```

### Running Individual Test Suites

```bash
# Python integration tests only
python tests/integration/test_cross_language_integration.py

# Lua integration tests only
lua tests/integration/test_cross_language_integration.lua
```

## Test Categories

### 1. Bindings Availability Tests

**Purpose:** Verify both Python and Lua bindings are properly installed and accessible.

**What it tests:**
- Python qtforge module import
- Lua qtforge global availability
- Basic module structure verification

**Expected Results:**
- Both bindings should be available for integration tests to proceed
- Clear error messages if bindings are missing

### 2. Plugin Manager Consistency Tests

**Purpose:** Ensure plugin managers behave identically across languages.

**What it tests:**
- Plugin manager creation
- Plugin enumeration
- Loaded plugin counts
- Method availability

**Expected Results:**
- Same number of loaded plugins reported by both languages
- Consistent method availability
- Identical behavior for basic operations

### 3. Message Bus Interoperability Tests

**Purpose:** Verify message bus systems work consistently.

**What it tests:**
- Message bus creation in both languages
- Basic functionality availability
- API consistency

**Expected Results:**
- Both languages can create message buses
- Consistent API structure
- Compatible functionality

### 4. Configuration Sharing Tests

**Purpose:** Test configuration system consistency.

**What it tests:**
- Configuration manager creation
- Basic configuration operations
- API availability

**Expected Results:**
- Configuration managers work in both languages
- Consistent API structure
- Potential for shared configuration

### 5. Security Policy Consistency Tests

**Purpose:** Verify security systems are consistent.

**What it tests:**
- Security manager creation
- Policy evaluation capabilities
- API consistency

**Expected Results:**
- Security managers available in both languages
- Consistent security policy handling
- Compatible security models

### 6. Enum Value Consistency Tests

**Purpose:** Ensure enumeration values are synchronized.

**What it tests:**
- PluginState enum values
- Enum value consistency across languages
- Complete enum coverage

**Expected Results:**
- Identical enum values in both languages
- Same enum members available
- Consistent value mappings

### 7. Error Handling Consistency Tests

**Purpose:** Verify error handling patterns are consistent.

**What it tests:**
- Exception throwing for invalid operations
- Error message consistency
- Error handling patterns

**Expected Results:**
- Both languages throw errors for invalid operations
- Consistent error handling behavior
- Similar error reporting

## Understanding Test Output

### Success Indicators

- ✅ **Green checkmarks** - Tests passed successfully
- 📊 **Status information** - Operational data and comparisons
- 📋 **Configuration details** - Test setup and parameters

### Warning Indicators

- ⚠️  **Yellow warnings** - Expected issues or partial functionality
- 🔧 **Partial success** - Some aspects work, others don't
- 📁 **Missing features** - Functionality not available in development environment

### Error Indicators

- ❌ **Red errors** - Unexpected failures requiring investigation
- 💥 **Critical failures** - Binding or system issues
- 🚨 **Integration problems** - Cross-language compatibility issues

## Expected Behavior

### Development Environment

The integration tests are designed to work in development environments where:
- Actual plugins may not be loaded
- Services may not be running
- Some functionality may be limited

This results in many "expected warnings" that demonstrate the tests are working correctly even without a complete plugin ecosystem.

### Production Environment

In production environments with loaded plugins and running services:
- More functionality will be available
- Fewer warnings should occur
- More comprehensive testing is possible

## Troubleshooting

### Common Issues

1. **"Bindings not available"**
   ```
   ❌ Python bindings are not available
   ❌ Lua bindings are not available
   ```
   **Solution:** Rebuild QtForge with both bindings enabled

2. **"Lua interpreter not found"**
   ```
   ❌ Lua interpreter not found
   ```
   **Solution:** Install Lua: `sudo apt-get install lua5.3` (Ubuntu) or equivalent

3. **"Python module import failed"**
   ```
   ImportError: No module named 'qtforge'
   ```
   **Solution:** Check build directory and Python path configuration

4. **"Inconsistent enum values"**
   ```
   ❌ PluginState: inconsistent values
   ```
   **Solution:** This indicates a binding synchronization issue - check binding generation

### Debugging Steps

1. **Check Build Configuration:**
   ```bash
   cmake -LA | grep QTFORGE_BUILD
   ```

2. **Verify Individual Bindings:**
   ```bash
   # Test Python bindings
   python -c "import qtforge; print('Python OK')"
   
   # Test Lua bindings
   lua -e "print(qtforge and 'Lua OK' or 'Lua FAIL')"
   ```

3. **Run Individual Tests:**
   ```bash
   # Debug Python integration tests
   python tests/integration/test_cross_language_integration.py
   
   # Debug Lua integration tests
   lua tests/integration/test_cross_language_integration.lua
   ```

4. **Check Dependencies:**
   ```bash
   # Verify interpreters
   python --version
   lua -v
   
   # Check build artifacts
   ls -la build/
   ```

## Integration with CI/CD

### Automated Testing

The integration tests are designed for CI/CD environments:

```yaml
# Example GitHub Actions workflow
- name: Run Integration Tests
  run: |
    cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON -DQTFORGE_BUILD_LUA_BINDINGS=ON ..
    make
    python tests/integration/run_integration_tests.py
```

### Exit Codes

- **0** - All tests passed
- **1** - Partial success (some tests passed)
- **2** - Complete failure (all tests failed)

## Contributing

### Adding New Integration Tests

1. **Follow existing patterns** in test structure and output formatting
2. **Test both directions** - Python calling Lua and Lua calling Python
3. **Include error handling** for missing functionality
4. **Add comprehensive documentation** explaining what is being tested
5. **Update this README** with new test descriptions

### Best Practices

- **Use consistent output formatting** with emoji indicators
- **Handle missing functionality gracefully** with warnings
- **Provide clear error messages** for debugging
- **Test both success and failure paths**
- **Include performance considerations** where relevant

## Future Enhancements

### Planned Improvements

- **Shared State Testing** - Tests for shared configuration and state
- **Performance Benchmarking** - Cross-language performance comparisons
- **Memory Management Testing** - Resource cleanup verification
- **Concurrent Access Testing** - Thread safety across languages
- **Plugin Communication Testing** - Cross-language plugin interaction

### Advanced Integration Scenarios

- **Hybrid Applications** - Applications using both Python and Lua
- **Plugin Bridges** - Plugins that expose functionality to both languages
- **Shared Services** - Services accessible from both language environments
- **Configuration Synchronization** - Real-time configuration sharing

The integration tests provide a solid foundation for ensuring QtForge's multi-language support is robust, consistent, and reliable across different deployment scenarios.
