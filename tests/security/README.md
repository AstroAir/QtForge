# QtForge Sandbox System Tests

## Overview

This directory contains comprehensive tests for the QtForge Sandbox System, ensuring robust security, performance, and functionality of the plugin sandboxing infrastructure.

## Test Structure

### Test Categories

1. **Unit Tests** - Test individual components in isolation

   - `test_plugin_sandbox.cpp` - Core sandbox functionality
   - `test_resource_monitor.cpp` - Cross-platform resource monitoring
   - `test_security_enforcer.cpp` - Security policy enforcement
   - `test_sandbox_manager.cpp` - Sandbox lifecycle management

2. **Integration Tests** - Test component interactions and workflows

   - `test_sandbox_integration.cpp` - End-to-end sandbox workflows

3. **Performance Tests** - Benchmark and stress testing
   - `test_sandbox_performance.cpp` - Performance metrics and scalability

### Mock Plugins

The `mock_plugins/` directory contains utilities for generating test plugins:

- `MockPluginGenerator` - Creates various types of test plugins
- Well-behaved plugins for normal operation testing
- Resource-hungry plugins for limit testing
- Malicious plugins for security testing
- Crashing plugins for error handling testing

## Building Tests

### Prerequisites

- Qt 6.0 or later
- CMake 3.16 or later
- C++17 compatible compiler
- Python 3.6+ (for plugin testing)

### Build Commands

```bash
# Configure build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug

# Build tests
cmake --build build --target run_sandbox_tests

# Or build specific test categories
cmake --build build --target test_plugin_sandbox
cmake --build build --target test_resource_monitor
```

## Running Tests

### Using CMake/CTest

```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test categories
ctest --test-dir build --label-regex "unit"
ctest --test-dir build --label-regex "integration"
ctest --test-dir build --label-regex "performance"

# Run with verbose output
ctest --test-dir build --output-on-failure --verbose
```

### Using Test Runner Script

```bash
# Run all tests with detailed reporting
python3 tests/security/run_tests.py --build-dir build --verbose

# Run specific category
python3 tests/security/run_tests.py --category unit

# Generate JSON report
python3 tests/security/run_tests.py --report test_results.json

# List available tests
python3 tests/security/run_tests.py --list
```

### Manual Execution

```bash
# Run individual test executables
./build/tests/security/test_plugin_sandbox
./build/tests/security/test_resource_monitor
./build/tests/security/test_security_enforcer
```

## Test Coverage

### What's Tested

#### Core Functionality

- [x] Sandbox initialization and shutdown
- [x] Plugin execution with different types (Python, Native)
- [x] Resource usage tracking and limit enforcement
- [x] Security policy validation and enforcement
- [x] Error handling and recovery

#### Resource Monitoring

- [x] Cross-platform CPU usage tracking
- [x] Memory consumption monitoring
- [x] File handle counting
- [x] Network connection tracking
- [x] Disk space usage monitoring

#### Security Enforcement

- [x] File system access validation
- [x] Network access control
- [x] Process creation restrictions
- [x] System call monitoring
- [x] API call blocking
- [x] Security violation detection and reporting

#### Management

- [x] Multiple sandbox lifecycle management
- [x] Policy registration and retrieval
- [x] Concurrent sandbox operations
- [x] Thread safety

#### Performance

- [x] Sandbox creation/destruction overhead
- [x] Resource monitoring performance impact
- [x] Concurrent execution scalability
- [x] Memory footprint analysis

### Test Scenarios

#### Normal Operation

- Well-behaved plugins executing successfully
- Resource usage within limits
- Proper cleanup after execution

#### Resource Limits

- Memory exhaustion handling
- CPU time limit enforcement
- Execution timeout handling
- File handle limit enforcement

#### Security Violations

- Unauthorized file access attempts
- Network access violations
- Process creation attempts
- System call restrictions

#### Error Conditions

- Plugin crashes and recovery
- Invalid plugin paths
- Malformed security policies
- Resource cleanup failures

#### Stress Testing

- High-frequency operations
- Large numbers of concurrent sandboxes
- Long-running plugin monitoring
- Resource exhaustion scenarios

## Test Configuration

### Environment Variables

- `QTPLUGIN_TEST_TIMEOUT` - Override default test timeouts
- `QTPLUGIN_TEST_VERBOSE` - Enable verbose test output
- `QTPLUGIN_TEST_PYTHON` - Python executable path for plugin tests

### Test Data

Test plugins and data files are generated dynamically during test execution to ensure clean, reproducible tests.

## Platform-Specific Considerations

### Windows

- Uses Performance Counters for resource monitoring
- Tests job object functionality for process isolation
- Registry access control testing

### Linux

- Uses `/proc` filesystem for resource monitoring
- Tests cgroups integration where available
- File descriptor limit testing

### macOS

- Uses Mach APIs for resource monitoring
- Tests BSD resource limits
- Process isolation testing

## Troubleshooting

### Common Issues

1. **Python not found**

   - Ensure Python 3.6+ is installed and in PATH
   - Set `QTPLUGIN_TEST_PYTHON` environment variable

2. **Resource monitoring tests fail**

   - Check platform support for resource monitoring APIs
   - Verify sufficient permissions for system resource access

3. **Security tests fail**

   - May indicate actual security vulnerabilities
   - Review test output for specific violation details

4. **Performance tests timeout**
   - Increase timeout values for slower systems
   - Check system resource availability

### Debug Mode

Build tests in debug mode for additional diagnostics:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANDBOX_DEBUG=ON
```

### Logging

Enable detailed logging:

```bash
export QT_LOGGING_RULES="qtplugin.sandbox.debug=true"
./build/tests/security/test_plugin_sandbox
```

## Contributing

### Adding New Tests

1. Create test file following naming convention: `test_<component>.cpp`
2. Use Qt Test framework (`QTEST_MAIN`, `QVERIFY`, etc.)
3. Add test to `CMakeLists.txt`
4. Update this README with test description

### Test Guidelines

- Each test should be independent and not rely on external state
- Use temporary directories and files for test data
- Clean up resources in test cleanup methods
- Use meaningful test names that describe what's being tested
- Add both positive and negative test cases

### Mock Plugin Development

When creating new mock plugins:

- Follow the existing plugin interface
- Include JSON output for result parsing
- Test both success and failure scenarios
- Document expected behavior

## Performance Benchmarks

### Expected Performance Metrics

- Sandbox creation: < 50ms average
- Plugin execution overhead: < 100ms
- Resource monitoring: < 5ms per update
- Security validation: < 1ms per check

### Benchmark Results

Run performance tests to get current benchmark results:

```bash
python3 tests/security/run_tests.py --category performance --verbose
```

Results are platform and hardware dependent. Use for relative performance comparisons and regression detection.

## Continuous Integration

### CI Pipeline Integration

Tests are designed to run in CI environments:

- No interactive components
- Configurable timeouts
- Machine-readable output formats
- Proper exit codes for success/failure

### Test Reports

Generate CI-friendly reports:

```bash
python3 tests/security/run_tests.py --report ci_results.json
```

The JSON report includes:

- Test execution summary
- Individual test results
- Performance metrics
- Error details for failed tests
