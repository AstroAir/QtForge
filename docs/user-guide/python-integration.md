# Python Integration

QtForge provides comprehensive Python integration, allowing you to write plugins in Python and seamlessly integrate them with C++ applications.

## Overview

Python integration in QtForge enables:
- Writing plugins entirely in Python
- Calling Python code from C++ plugins
- Sharing data between Python and C++ components
- Using Python libraries in QtForge applications
- Hot-reloading Python plugins during development

## Python Plugin Development

### Basic Python Plugin

Create a Python plugin by implementing the plugin interface:

```python
# my_python_plugin.py
import qtforge
from qtforge.core import IPlugin, PluginState, PluginError
from qtforge.communication import MessageBus
import json

class MyPythonPlugin(IPlugin):
    def __init__(self):
        super().__init__()
        self._state = PluginState.UNLOADED
        self._message_bus = None
        
    def name(self) -> str:
        return "MyPythonPlugin"
    
    def version(self) -> str:
        return "1.0.0"
    
    def description(self) -> str:
        return "A Python plugin example"
    
    def author(self) -> str:
        return "Python Developer"
    
    def dependencies(self) -> list:
        return []
    
    def initialize(self) -> bool:
        try:
            self._message_bus = MessageBus.instance()
            self._message_bus.subscribe("python.messages", self._handle_message)
            self._state = PluginState.INITIALIZED
            return True
        except Exception as e:
            print(f"Initialization failed: {e}")
            self._state = PluginState.ERROR
            return False
    
    def activate(self) -> bool:
        if self._state != PluginState.INITIALIZED:
            return False
        
        try:
            # Publish activation message
            self._message_bus.publish("plugin.events", {
                "type": "activation",
                "plugin": self.name(),
                "timestamp": qtforge.utils.current_timestamp()
            })
            
            self._state = PluginState.ACTIVE
            return True
        except Exception as e:
            print(f"Activation failed: {e}")
            self._state = PluginState.ERROR
            return False
    
    def deactivate(self) -> bool:
        if self._state != PluginState.ACTIVE:
            return False
        
        try:
            self._message_bus.publish("plugin.events", {
                "type": "deactivation",
                "plugin": self.name(),
                "timestamp": qtforge.utils.current_timestamp()
            })
            
            self._state = PluginState.INITIALIZED
            return True
        except Exception as e:
            print(f"Deactivation failed: {e}")
            self._state = PluginState.ERROR
            return False
    
    def cleanup(self):
        if self._message_bus:
            self._message_bus.unsubscribe_all(self)
        self._state = PluginState.UNLOADED
    
    def state(self) -> PluginState:
        return self._state
    
    def is_compatible(self, version: str) -> bool:
        return version >= "1.0.0"
    
    def _handle_message(self, message):
        print(f"Python plugin received message: {message}")
        
        # Echo the message back
        response = {
            "original": message,
            "processed_by": self.name(),
            "timestamp": qtforge.utils.current_timestamp()
        }
        
        self._message_bus.publish("python.responses", response)

# Plugin factory function
def create_plugin():
    return MyPythonPlugin()
```

### Plugin Manifest

Create a plugin manifest file (`plugin.json`):

```json
{
    "name": "MyPythonPlugin",
    "version": "1.0.0",
    "description": "A Python plugin example",
    "author": "Python Developer",
    "license": "MIT",
    "language": "python",
    "entry_point": "my_python_plugin.py",
    "main_class": "MyPythonPlugin",
    "dependencies": [],
    "python_requirements": [
        "numpy>=1.20.0",
        "requests>=2.25.0"
    ],
    "capabilities": [
        "message-processing",
        "data-analysis"
    ],
    "configuration": {
        "debug_mode": {
            "type": "boolean",
            "default": false,
            "description": "Enable debug logging"
        },
        "processing_timeout": {
            "type": "integer",
            "default": 30,
            "description": "Processing timeout in seconds"
        }
    }
}
```

## C++ Integration

### Loading Python Plugins

Load Python plugins from C++ using the PluginManager:

```cpp
#include <qtforge/python/python_plugin_loader.hpp>
#include <qtforge/core/plugin_manager.hpp>

// Initialize Python integration
qtforge::python::PythonIntegration::initialize();

// Create plugin manager with Python support
qtforge::PluginManager manager;
manager.addLoader(std::make_unique<qtforge::python::PythonPluginLoader>());

// Load Python plugin
auto result = manager.loadPlugin("python_plugins/my_python_plugin/");
if (result) {
    auto plugin = result.value();
    std::cout << "Loaded Python plugin: " << plugin->name() << std::endl;
    
    // Initialize and activate
    plugin->initialize();
    plugin->activate();
} else {
    std::cerr << "Failed to load Python plugin: " << result.error().message() << std::endl;
}
```

### Calling Python from C++

Execute Python code from C++ plugins:

```cpp
#include <qtforge/python/python_executor.hpp>

class MyCppPlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        pythonExecutor_ = std::make_unique<qtforge::python::PythonExecutor>();
        
        // Execute Python script
        std::string pythonCode = R"(
import numpy as np
import json

def process_data(data):
    # Convert to numpy array
    arr = np.array(data)
    
    # Perform some processing
    result = {
        'mean': float(np.mean(arr)),
        'std': float(np.std(arr)),
        'min': float(np.min(arr)),
        'max': float(np.max(arr))
    }
    
    return json.dumps(result)
)";
        
        auto result = pythonExecutor_->execute(pythonCode);
        if (!result) {
            return qtforge::Error("Failed to execute Python code");
        }
        
        return {};
    }
    
    std::string processData(const std::vector<double>& data) {
        // Call Python function
        auto result = pythonExecutor_->callFunction("process_data", data);
        if (result) {
            return result.value();
        } else {
            return "{}"; // Empty JSON on error
        }
    }
    
private:
    std::unique_ptr<qtforge::python::PythonExecutor> pythonExecutor_;
};
```

## Data Exchange

### JSON-Based Communication

Use JSON for data exchange between C++ and Python:

```cpp
// C++ side
QJsonObject data;
data["values"] = QJsonArray{1.0, 2.0, 3.0, 4.0, 5.0};
data["operation"] = "statistics";

auto& messageBus = qtforge::MessageBus::instance();
messageBus.publish("python.data_processing", data);
```

```python
# Python side
def handle_data_processing(message):
    values = message.get("values", [])
    operation = message.get("operation", "")
    
    if operation == "statistics":
        import statistics
        result = {
            "mean": statistics.mean(values),
            "median": statistics.median(values),
            "stdev": statistics.stdev(values) if len(values) > 1 else 0
        }
        
        message_bus.publish("python.processing_results", result)
```

### Binary Data Exchange

For performance-critical applications, use binary data exchange:

```cpp
// C++ side - send binary data
std::vector<uint8_t> binaryData = getBinaryData();
qtforge::BinaryMessage message;
message.data = binaryData;
message.contentType = "application/octet-stream";

messageBus.publish("python.binary_processing", message);
```

```python
# Python side - process binary data
import numpy as np

def handle_binary_processing(message):
    # Convert binary data to numpy array
    data = np.frombuffer(message.data, dtype=np.uint8)
    
    # Process data
    processed = apply_image_filter(data)
    
    # Send back processed data
    result_message = {
        "data": processed.tobytes(),
        "shape": processed.shape,
        "dtype": str(processed.dtype)
    }
    
    message_bus.publish("python.binary_results", result_message)
```

## Advanced Features

### Async Python Operations

Handle asynchronous operations in Python plugins:

```python
import asyncio
import qtforge
from qtforge.core import IPlugin

class AsyncPythonPlugin(IPlugin):
    def __init__(self):
        super().__init__()
        self._loop = None
        self._tasks = []
    
    def initialize(self) -> bool:
        # Create event loop for async operations
        self._loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self._loop)
        
        # Start background task
        task = self._loop.create_task(self._background_worker())
        self._tasks.append(task)
        
        return True
    
    async def _background_worker(self):
        while True:
            try:
                # Perform async work
                await self._process_queue()
                await asyncio.sleep(1.0)  # Wait 1 second
            except asyncio.CancelledError:
                break
            except Exception as e:
                print(f"Background worker error: {e}")
    
    async def _process_queue(self):
        # Simulate async processing
        await asyncio.sleep(0.1)
        
        # Publish status update
        message_bus = qtforge.MessageBus.instance()
        message_bus.publish("async.status", {
            "plugin": self.name(),
            "status": "processing",
            "timestamp": qtforge.utils.current_timestamp()
        })
    
    def cleanup(self):
        if self._loop:
            # Cancel all tasks
            for task in self._tasks:
                task.cancel()
            
            # Close event loop
            self._loop.close()
```

### Python Package Management

Manage Python dependencies automatically:

```cpp
#include <qtforge/python/package_manager.hpp>

// Install required packages
qtforge::python::PackageManager packageManager;

std::vector<std::string> requirements = {
    "numpy>=1.20.0",
    "scipy>=1.7.0",
    "matplotlib>=3.4.0"
};

auto result = packageManager.installPackages(requirements);
if (!result) {
    std::cerr << "Failed to install packages: " << result.error().message() << std::endl;
}
```

## Configuration

### Python Environment Setup

Configure Python integration in your application:

```cpp
qtforge::python::PythonConfig config;
config.pythonPath = "/usr/bin/python3";
config.virtualEnvPath = "./venv";
config.enableHotReload = true;
config.maxMemoryUsage = 512 * 1024 * 1024; // 512MB
config.executionTimeout = std::chrono::seconds(30);

qtforge::python::PythonIntegration::initialize(config);
```

### Plugin Configuration

Configure Python plugins through the manifest:

```json
{
    "python_config": {
        "interpreter": "python3",
        "virtual_env": "./plugin_venv",
        "sys_path_additions": [
            "./lib",
            "./external"
        ],
        "environment_variables": {
            "PYTHONPATH": "./lib:./external",
            "PLUGIN_DEBUG": "1"
        }
    }
}
```

## Debugging Python Plugins

### Debug Mode

Enable debug mode for Python plugins:

```python
import qtforge
import logging

# Enable debug logging
logging.basicConfig(level=logging.DEBUG)
qtforge.set_debug_mode(True)

class DebugPythonPlugin(IPlugin):
    def initialize(self) -> bool:
        qtforge.logger.debug(f"Initializing {self.name()}")
        
        try:
            # Plugin initialization code
            return True
        except Exception as e:
            qtforge.logger.error(f"Initialization failed: {e}")
            return False
```

### Hot Reloading

Enable hot reloading for development:

```cpp
// Enable hot reloading in C++
qtforge::python::PythonConfig config;
config.enableHotReload = true;
config.hotReloadInterval = std::chrono::seconds(2);

qtforge::python::PythonIntegration::initialize(config);
```

## Best Practices

1. **Error Handling**: Always handle exceptions in Python code
2. **Memory Management**: Be aware of memory usage in long-running plugins
3. **Threading**: Use proper synchronization when accessing shared resources
4. **Performance**: Use binary data exchange for large datasets
5. **Dependencies**: Manage Python package dependencies carefully

## Troubleshooting

### Common Issues

**Python interpreter not found**
```bash
# Ensure Python is in PATH or specify full path
export PATH=/usr/bin:$PATH
```

**Module import errors**
```python
# Add paths to sys.path
import sys
sys.path.append('./lib')
sys.path.append('./external')
```

**Memory leaks**
```python
# Properly clean up resources
def cleanup(self):
    # Close files, connections, etc.
    if hasattr(self, '_file'):
        self._file.close()
```

## See Also

- **[Python API Reference](../api/python/overview.md)**: Python API documentation
- **[Plugin Development Guide](plugin-development.md)**: General plugin development
- **[Examples](../examples/python-examples.md)**: Python plugin examples
- **[Advanced Plugin Development](advanced-plugin-development.md)**: Advanced techniques
