# Python Plugin Bridge Example

This example demonstrates the complete Python plugin bridge functionality in QtForge.

## Overview

The Python plugin bridge allows you to:
- Load Python scripts as QtForge plugins
- Execute Python code dynamically
- Call Python methods from C++
- Access and modify Python object properties
- Subscribe to and emit events
- Hot-reload Python plugins during development

## Files

- `sample_plugin.py` - Example Python plugin demonstrating various features
- `test_bridge.cpp` - C++ test program that exercises the bridge functionality
- `CMakeLists.txt` - Build configuration for the test program

## Python Plugin Structure

A QtForge Python plugin should be a Python class with the following structure:

```python
class MyPlugin:
    def __init__(self):
        # Plugin metadata
        self.name = "My Plugin"
        self.version = "1.0.0"
        self.description = "Plugin description"
        self.author = "Author Name"
        self.license = "MIT"
        
        # Plugin state
        self.initialized = False
    
    def initialize(self):
        """Initialize the plugin"""
        self.initialized = True
        return {"success": True}
    
    def shutdown(self):
        """Shutdown the plugin"""
        self.initialized = False
        return {"success": True}
    
    # Add your plugin methods here
    def my_method(self, param1, param2):
        return {"result": param1 + param2}

# Factory function (optional)
def create_plugin():
    return MyPlugin()
```

## Bridge Features Implemented

### Core Functionality
- ✅ Plugin loading and initialization
- ✅ Method invocation with parameter passing
- ✅ Property access (get/set)
- ✅ Dynamic code execution
- ✅ Plugin metadata extraction
- ✅ Method and property discovery
- ✅ Hot reload support
- ✅ Event system (basic implementation)

### Python Bindings
- ✅ Core QtForge types and enums
- ✅ Security manager bindings
- ✅ Utility functions
- 🔄 Communication system (planned)
- 🔄 Manager classes (planned)
- 🔄 Orchestration system (planned)

## Building and Running

1. Ensure Python 3.x is installed and available in PATH
2. Build QtForge with Python bindings enabled:
   ```bash
   cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON ..
   make
   ```
3. Run the test program:
   ```bash
   ./test_python_bridge
   ```

## Usage from C++

```cpp
#include <qtplugin/bridges/python_plugin_bridge.hpp>

// Create bridge for a Python plugin
auto bridge = std::make_unique<qtplugin::PythonPluginBridge>("path/to/plugin.py");

// Initialize the bridge and load the plugin
auto result = bridge->initialize();
if (result) {
    // Call plugin methods
    QJsonObject params;
    params["input"] = "test data";
    auto method_result = bridge->execute_command("process_data", params);
    
    // Access properties
    auto prop_result = bridge->get_property("", "counter");
    
    // Set properties
    bridge->set_property("", QVariant(42), "counter");
    
    // Execute arbitrary Python code
    auto code_result = bridge->execute_code("plugin.get_info()");
}
```

## Error Handling

The bridge provides comprehensive error handling:
- Plugin loading errors
- Method execution errors
- Property access errors
- Python runtime errors
- Communication timeouts

All errors are returned as `qtplugin::PluginError` objects with detailed error messages and stack traces.

## Security Considerations

- Python plugins run in a separate process for isolation
- Code execution is sandboxed with limited built-in functions
- Plugin paths should be validated before loading
- Consider using the security manager for additional validation

## Troubleshooting

1. **Python not found**: Ensure Python is in PATH or specify the path in PythonExecutionEnvironment constructor
2. **Plugin loading fails**: Check that the Python file is valid and contains the expected class
3. **Method calls fail**: Verify method names and parameter types
4. **Bridge timeout**: Increase timeout values for complex operations

## Next Steps

- Implement full event system with Python-to-C++ event propagation
- Add more Python binding modules
- Implement plugin dependency management
- Add plugin marketplace integration
- Enhance security and sandboxing features
