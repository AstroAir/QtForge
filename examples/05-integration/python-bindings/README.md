# QtForge Python Bindings Examples

This directory contains comprehensive examples demonstrating how to use the QtForge Python bindings effectively. Each example focuses on different aspects of the bindings and provides practical usage patterns.

## 🚀 **New Complete Integration Examples**

### 🎯 `complete_integration_example.py` - Full Feature Demonstration

**Purpose**: Comprehensive demonstration of all QtForge Python modules
**Topics Covered**:

- All available modules (core, communication, orchestration, monitoring, etc.)
- Complete feature testing and validation
- Integration status reporting
- Advanced functionality showcase

**Run**: `python complete_integration_example.py`

### 🔧 `simple_plugin_example.py` - Python Plugin Development

**Purpose**: Complete Python plugin development example
**Topics Covered**:

- Python plugin creation and lifecycle
- Data processing and message handling
- Plugin statistics and monitoring
- Integration with QtForge core systems

**Run**: `python simple_plugin_example.py`

## 📁 Original Example Files

### 1. `basic_usage.py` - Getting Started

**Purpose**: Introduction to QtForge Python bindings
**Topics Covered**:

- Module import and initialization
- Version information access
- Basic function calls
- Error handling fundamentals

**Run**: `python basic_usage.py`

### 2. `plugin_management.py` - Plugin Operations

**Purpose**: Demonstrate plugin management capabilities
**Topics Covered**:

- Plugin lifecycle management
- Plugin discovery and loading
- Plugin manager operations
- Module introspection

**Run**: `python plugin_management.py`

### 3. `version_handling.py` - Version Management

**Purpose**: Advanced version handling and comparison  
**Topics Covered**:

- Version creation and parsing
- Version comparison scenarios
- Version range handling
- Compatibility checking

**Run**: `python version_handling.py`

### 4. `error_handling.py` - Error Management

**Purpose**: Comprehensive error handling patterns  
**Topics Covered**:

- Error creation and categorization
- Exception handling strategies
- Error recovery patterns
- Error logging and reporting

**Run**: `python error_handling.py`

### 5. `advanced_usage.py` - Advanced Patterns

**Purpose**: Advanced usage patterns and best practices  
**Topics Covered**:

- Performance optimization
- Batch operations
- Integration patterns
- Asynchronous usage
- Caching strategies

**Run**: `python advanced_usage.py`

## 🚀 Quick Start

1. **Build QtForge with Python bindings** (now enabled by default):

   ```bash
   # Python bindings are now enabled by default!
   cmake -B build -S .
   cmake --build build

   # Or explicitly enable all Python features:
   cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON \
         -DQTFORGE_PYTHON_ENABLE_ALL_MODULES=ON \
         -DQTFORGE_PYTHON_BINDINGS_EXAMPLES=ON \
         -B build -S .
   cmake --build build
   ```

2. **Run the complete integration example**:

   ```bash
   cd examples/05-integration/python-bindings
   python complete_integration_example.py
   ```

3. **Try the simple plugin example**:

   ```bash
   python simple_plugin_example.py
   ```

4. **Explore other examples**:

   ```bash
   python basic_usage.py
   python plugin_management.py
   python version_handling.py
   python error_handling.py
   ```

## ✨ **New Features in Python Integration**

### 🎯 **Complete Module Coverage**
All QtForge modules are now available in Python:

- **Core**: Plugin management and lifecycle
- **Communication**: Message bus and request-response system
- **Security**: Security validation and permission management
- **Managers**: Configuration, logging, and resource management
- **Orchestration**: Plugin workflow and orchestration
- **Monitoring**: Hot reload and metrics collection
- **Threading**: Thread management and concurrency
- **Transactions**: Transaction management and atomic operations
- **Composition**: Plugin composition and aggregation
- **Marketplace**: Plugin marketplace and distribution

### 🔧 **Enhanced Configuration**
- Python bindings enabled by default
- Automatic type stub generation (.pyi files)
- Complete example suite
- Improved error handling and diagnostics

### 📦 **Easy Installation**
- No manual configuration required
- Automatic dependency detection
- Built-in examples and documentation

## 📋 Prerequisites

- QtForge built with Python bindings (enabled by default)
- Python 3.8 or later (recommended)
- QtForge Python module in Python path

## 🔧 Configuration

Each example automatically adds the build directory to the Python path:

```python
sys.path.insert(0, '../../build')
```

Adjust this path if your build directory is located elsewhere.

## 📊 Example Output

Each example provides detailed output showing:

- ✅ Successful operations
- ❌ Error conditions and handling
- 📋 Information and metrics
- 🎉 Completion status

## 🛠️ Customization

### Modifying Examples

- Change file paths in examples to match your setup
- Adjust operation parameters to test different scenarios
- Add your own test cases and operations

### Creating New Examples

Use this template structure:

```python
#!/usr/bin/env python3
"""
Your Example Description
"""

import sys
import os

# Add the build directory to Python path
sys.path.insert(0, '../../build')

class YourExample:
    def __init__(self):
        import qtforge
        self.qtforge = qtforge

    def demonstrate_feature(self):
        # Your demonstration code here
        pass

    def run_complete_example(self):
        try:
            self.demonstrate_feature()
            return 0
        except Exception as e:
            print(f"Error: {e}")
            return 1

def main():
    try:
        example = YourExample()
        return example.run_complete_example()
    except ImportError as e:
        print(f"Failed to import QtForge: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())
```

## 🧪 Testing

Run all examples to verify your QtForge Python bindings installation:

```bash
# Run all examples
for example in *.py; do
    echo "Running $example..."
    python "$example"
    echo "---"
done
```

## 📚 Additional Resources

- **QtForge Documentation**: See main project documentation
- **Python Bindings API**: Check the generated Python module documentation
- **pybind11 Documentation**: <https://pybind11.readthedocs.io/>

## 🐛 Troubleshooting

### Common Issues

1. **Import Error**: `ModuleNotFoundError: No module named 'qtforge'`

   - Ensure QtForge is built with Python bindings enabled
   - Check that the build directory is in your Python path
   - Verify the Python module file exists in the build directory

2. **Version Mismatch**: Python version compatibility issues

   - Ensure you're using the same Python version for building and running
   - Rebuild with the correct Python executable specified

3. **Missing Dependencies**: pybind11 or Qt libraries not found
   - Install pybind11: `pip install pybind11`
   - Ensure Qt libraries are in your system PATH

### Getting Help

- Check the main QtForge documentation
- Review the CMake configuration for Python bindings
- Examine the build output for error messages
- Test with the basic example first before advanced ones

## 🎯 Next Steps

After running these examples:

1. **Integrate into your project**: Use the patterns shown in your own applications
2. **Extend functionality**: Add your own wrapper functions and classes
3. **Contribute**: Submit improvements and additional examples
4. **Performance tune**: Use the performance patterns for optimization

Happy coding with QtForge Python bindings! 🚀
