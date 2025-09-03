# QtForge Python Bindings Examples

This directory contains comprehensive examples demonstrating the QtForge Python bindings. Each example focuses on a specific aspect of the QtForge plugin system and provides practical, runnable code with detailed explanations.

## Prerequisites

Before running these examples, ensure that:

1. **QtForge is built with Python bindings enabled:**
   ```bash
   cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON ..
   make
   ```

2. **Python 3.7+ is installed** with the following packages:
   ```bash
   pip install pytest  # For running tests
   ```

3. **The QtForge build directory is accessible** (examples automatically add it to the Python path)

## Examples Overview

### 1. Basic Plugin Management (`01_basic_plugin_management.py`)

**What it demonstrates:**
- Creating and configuring plugin managers
- Loading and unloading plugins
- Plugin registry operations
- Plugin lifecycle management
- Dependency resolution
- Error handling

**Key concepts:**
- Plugin states and transitions
- Plugin enumeration and discovery
- Dependency graphs
- Plugin metadata handling

**Run it:**
```bash
python examples/python/01_basic_plugin_management.py
```

### 2. Communication and Messaging (`02_communication_and_messaging.py`)

**What it demonstrates:**
- Message bus creation and configuration
- Publish/subscribe messaging patterns
- Service contracts and discovery
- Request-response communication
- Message priorities and delivery modes
- Asynchronous communication

**Key concepts:**
- Inter-plugin communication
- Message routing and filtering
- Service-oriented architecture
- Event-driven programming

**Run it:**
```bash
python examples/python/02_communication_and_messaging.py
```

### 3. Security and Validation (`03_security_and_validation.py`)

**What it demonstrates:**
- Security manager operations
- Plugin validation and verification
- Permission management
- Trust levels and security contexts
- Policy engines and rule evaluation
- Signature verification

**Key concepts:**
- Plugin sandboxing
- Access control mechanisms
- Security policy definition
- Cryptographic verification

**Run it:**
```bash
python examples/python/03_security_and_validation.py
```

### 4. Orchestration and Workflows (`04_orchestration_and_workflows.py`)

**What it demonstrates:**
- Workflow creation and configuration
- Step definition and dependency management
- Different execution modes (sequential, parallel)
- Workflow monitoring and progress tracking
- Result handling and error management
- Asynchronous workflow execution

**Key concepts:**
- Complex plugin coordination
- Dependency-driven execution
- Workflow state management
- Performance optimization

**Run it:**
```bash
python examples/python/04_orchestration_and_workflows.py
```

## Running All Examples

To run all examples in sequence:

```bash
# From the QtForge root directory
for example in examples/python/[0-9]*.py; do
    echo "Running $example..."
    python "$example"
    echo "---"
done
```

## Example Output

Each example produces detailed output showing:
- ✅ Successful operations
- ⚠️  Warnings and expected failures
- ❌ Unexpected errors
- 📊 Status information and results
- 📋 Configuration details
- 🔧 Operation descriptions

## Understanding the Examples

### Code Structure

Each example follows a consistent structure:
1. **Import and setup** - Loading QtForge bindings
2. **Demonstration functions** - Each focusing on specific functionality
3. **Error handling** - Showing robust error management
4. **Main function** - Orchestrating all demonstrations
5. **Key takeaways** - Summary of important concepts

### Error Handling Philosophy

The examples demonstrate that:
- Many operations may fail in a development environment (plugins not available, services not running)
- This is expected and handled gracefully
- Real applications should implement similar error handling patterns
- Logging and monitoring are crucial for production deployments

### Extensibility

Each example can be extended by:
- Adding custom plugin implementations
- Implementing real service endpoints
- Creating production-ready error handling
- Adding persistence and configuration management

## Integration with Tests

These examples complement the comprehensive test suite in `tests/python/`. While tests focus on correctness and edge cases, examples focus on practical usage patterns and real-world scenarios.

## Troubleshooting

### Common Issues

1. **Import Error: "No module named 'qtforge'"**
   - Ensure QtForge is built with Python bindings enabled
   - Check that the build directory contains the Python modules
   - Verify Python version compatibility

2. **"QtForge bindings not available"**
   - Rebuild QtForge with `-DQTFORGE_BUILD_PYTHON_BINDINGS=ON`
   - Check for build errors in the Python binding compilation
   - Ensure all dependencies are installed

3. **Many operations show warnings**
   - This is expected in a development environment
   - Examples are designed to work without actual plugins
   - Focus on the successful creation of managers and basic operations

### Getting Help

- Check the comprehensive test suite for additional usage patterns
- Review the QtForge documentation for detailed API reference
- Examine the C++ source code for implementation details
- Use the Python `help()` function on QtForge objects for runtime documentation

## Next Steps

After running these examples:

1. **Explore the test suite** in `tests/python/` for comprehensive API coverage
2. **Create custom plugins** using the patterns demonstrated
3. **Build real applications** incorporating QtForge plugin architecture
4. **Contribute examples** for additional use cases and patterns

## Contributing

To contribute new examples:

1. Follow the existing naming convention (`NN_descriptive_name.py`)
2. Include comprehensive error handling and documentation
3. Add corresponding entries to this README
4. Ensure examples work in development environments
5. Test with the comprehensive test suite
