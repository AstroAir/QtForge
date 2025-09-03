# QtForge Performance and Memory Management Tests

This directory contains comprehensive performance and memory management tests for QtForge's Python and Lua bindings. These tests verify proper resource cleanup, memory efficiency, and performance characteristics under various conditions.

## Overview

The performance test suite validates:

- **Memory Management** - Proper object lifecycle and cleanup
- **Resource Efficiency** - Memory usage and resource allocation
- **Performance Characteristics** - Object creation and method call performance
- **Thread Safety** - Concurrent access and memory management
- **Garbage Collection** - Proper cleanup in both Python and Lua
- **Large-Scale Operations** - Performance under high load

## Test Files

### `test_memory_management.py`

**Python memory management and performance tests** that verify:
- Object lifecycle management and cleanup
- Weak reference behavior for proper garbage collection
- Circular reference handling
- Thread safety in multi-threaded scenarios
- Large object handling and performance
- Memory efficiency and cleanup effectiveness

**Key Features:**
- Uses Python's `tracemalloc` for precise memory tracking
- Comprehensive weak reference testing
- Multi-threaded memory management validation
- Performance benchmarking with detailed metrics
- Memory leak detection and analysis

### `test_memory_management.lua`

**Lua memory management and performance tests** that verify:
- Object lifecycle management with Lua garbage collection
- Weak reference behavior using Lua's weak tables
- Memory efficiency and cleanup patterns
- Performance characteristics specific to Lua
- Large object handling in Lua environment

**Key Features:**
- Uses Lua's `collectgarbage()` for memory management
- Weak table testing for proper cleanup
- Lua-specific performance patterns
- Memory tracking using `collectgarbage("count")`
- Performance optimization recommendations

### `run_performance_tests.py`

**Unified performance test runner** that:
- Executes both Python and Lua performance tests
- Provides comparative performance analysis
- Generates comprehensive performance reports
- Saves performance metrics for trend analysis
- Offers optimization recommendations

## Running the Tests

### Prerequisites

1. **Build QtForge with both bindings:**
   ```bash
   cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON -DQTFORGE_BUILD_LUA_BINDINGS=ON ..
   make
   ```

2. **Install required tools:**
   ```bash
   # Python (with tracemalloc support)
   python --version  # Should be 3.4+
   
   # Lua
   lua -v
   ```

### Running All Performance Tests

```bash
# From QtForge root directory
python tests/performance/run_performance_tests.py
```

### Running Individual Test Suites

```bash
# Python performance tests only
python tests/performance/test_memory_management.py

# Lua performance tests only
lua tests/performance/test_memory_management.lua
```

## Test Categories

### 1. Object Lifecycle Tests

**Purpose:** Verify proper object creation, management, and destruction.

**What it tests:**
- Object creation and destruction cycles
- Memory usage during object lifecycle
- Proper cleanup after object deletion
- Memory leak detection

**Expected Results:**
- No significant memory leaks after object destruction
- Consistent memory usage patterns
- Proper cleanup of resources

### 2. Weak Reference Tests

**Purpose:** Ensure weak references work correctly for garbage collection.

**What it tests:**
- Weak reference creation and behavior
- Garbage collection effectiveness
- Object cleanup verification

**Expected Results:**
- Weak references become invalid after object deletion
- Garbage collection properly cleans up objects
- No circular reference issues

### 3. Thread Safety Tests

**Purpose:** Verify memory management in multi-threaded scenarios.

**What it tests:**
- Concurrent object creation and destruction
- Thread-safe memory management
- Resource contention handling

**Expected Results:**
- No crashes or memory corruption in multi-threaded use
- Consistent memory management across threads
- Proper synchronization of resource access

### 4. Performance Benchmarks

**Purpose:** Measure and compare performance characteristics.

**What it tests:**
- Object creation performance
- Method call performance
- Memory efficiency per object
- Cleanup performance

**Expected Results:**
- Acceptable performance for production use
- Consistent performance across test runs
- Reasonable memory usage per object

### 5. Large-Scale Operation Tests

**Purpose:** Test behavior under high load conditions.

**What it tests:**
- Performance with large numbers of objects
- Memory usage scaling
- Cleanup efficiency at scale

**Expected Results:**
- Linear or sub-linear performance scaling
- Manageable memory usage growth
- Effective cleanup at scale

## Understanding Test Output

### Performance Metrics

- **📊 Objects/second** - Object creation rate
- **📊 Calls/second** - Method call performance
- **📊 Memory per object** - Memory efficiency
- **📊 Cleanup efficiency** - Percentage of memory freed

### Success Indicators

- ✅ **Green checkmarks** - Tests passed successfully
- 📊 **Performance data** - Benchmark results and metrics
- 🏆 **Performance comparisons** - Language-specific advantages

### Warning Indicators

- ⚠️  **Yellow warnings** - Performance concerns or partial functionality
- 🔧 **Optimization suggestions** - Areas for improvement
- 📈 **Scaling issues** - Performance degradation at scale

### Error Indicators

- ❌ **Red errors** - Test failures or critical issues
- 💥 **Memory leaks** - Significant memory growth
- 🚨 **Performance problems** - Unacceptable performance characteristics

## Performance Baselines

### Expected Performance Ranges

**Object Creation:**
- Python: 50-1000+ objects/second
- Lua: 50-500+ objects/second

**Method Calls:**
- Python: 1000-10000+ calls/second
- Lua: 1000-5000+ calls/second

**Memory Efficiency:**
- Python: < 10KB per object
- Lua: < 10KB per object

**Cleanup Efficiency:**
- Python: > 80% memory freed
- Lua: > 50% memory freed (due to GC differences)

### Performance Factors

**Factors affecting performance:**
- System specifications (CPU, memory)
- Build configuration (debug vs release)
- Compiler optimizations
- System load and available memory
- Garbage collection settings

## Troubleshooting

### Common Issues

1. **"Bindings not available"**
   ```
   ❌ QtForge bindings not available
   ```
   **Solution:** Rebuild QtForge with bindings enabled

2. **"Poor performance"**
   ```
   ⚠️  Slow object creation performance
   ```
   **Solution:** Check build configuration, use release builds for performance testing

3. **"Memory leaks detected"**
   ```
   ⚠️  Potential memory leak: 1024 KB
   ```
   **Solution:** This may be expected in development environments; verify in production

4. **"Thread safety issues"**
   ```
   ⚠️  Thread safety issues detected
   ```
   **Solution:** Check for race conditions and proper synchronization

### Debugging Performance Issues

1. **Profile Individual Operations:**
   ```python
   # Python profiling
   import cProfile
   cProfile.run('your_code_here()')
   ```

2. **Monitor Memory Usage:**
   ```python
   # Python memory monitoring
   import tracemalloc
   tracemalloc.start()
   # ... your code ...
   current, peak = tracemalloc.get_traced_memory()
   ```

3. **Lua Memory Monitoring:**
   ```lua
   -- Lua memory monitoring
   local before = collectgarbage("count")
   -- ... your code ...
   local after = collectgarbage("count")
   print("Memory used:", after - before, "KB")
   ```

## Optimization Recommendations

### Python Optimizations

1. **Use object pooling** for frequently created/destroyed objects
2. **Implement weak references** for observer patterns
3. **Call gc.collect()** strategically in long-running applications
4. **Use __slots__** for memory-efficient classes
5. **Profile with cProfile** and memory_profiler

### Lua Optimizations

1. **Call collectgarbage()** strategically in long-running scripts
2. **Use weak tables** for caches and observer patterns
3. **Minimize table creation** in hot code paths
4. **Reuse objects** where possible
5. **Monitor memory with collectgarbage("count")**

### General Optimizations

1. **Use release builds** for performance testing
2. **Enable compiler optimizations** (-O2, -O3)
3. **Consider memory alignment** for data structures
4. **Implement lazy initialization** where appropriate
5. **Profile under realistic workloads**

## Integration with CI/CD

### Automated Performance Testing

```yaml
# Example GitHub Actions workflow
- name: Run Performance Tests
  run: |
    cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON -DQTFORGE_BUILD_LUA_BINDINGS=ON ..
    make
    python tests/performance/run_performance_tests.py
```

### Performance Regression Detection

The test runner saves performance metrics to `performance_metrics.json` for trend analysis:

```json
{
  "timestamp": 1234567890,
  "python_metrics": {
    "object_creation_rate": 500,
    "method_call_rate": 2000,
    "memory_efficiency": 8192
  },
  "lua_metrics": {
    "object_creation_rate": 300,
    "method_call_rate": 1500,
    "memory_efficiency": 6144
  }
}
```

## Contributing

### Adding New Performance Tests

1. **Follow existing patterns** in test structure and metrics collection
2. **Include both success and failure scenarios**
3. **Add comprehensive documentation** explaining what is being measured
4. **Update this README** with new test descriptions
5. **Consider cross-language comparisons** where applicable

### Performance Test Guidelines

- **Use consistent measurement techniques** across languages
- **Account for garbage collection** in timing measurements
- **Test under various load conditions**
- **Include memory efficiency measurements**
- **Provide clear performance baselines**

## Future Enhancements

### Planned Improvements

- **Automated performance regression detection**
- **Historical performance trend analysis**
- **Memory fragmentation testing**
- **Concurrent performance testing**
- **Platform-specific optimizations**

### Advanced Testing Scenarios

- **Long-running stability tests**
- **Memory pressure testing**
- **Performance under resource constraints**
- **Cross-platform performance comparison**
- **Real-world workload simulation**

The performance test suite provides essential insights into QtForge's runtime characteristics and helps ensure optimal performance across different deployment scenarios and programming languages.
