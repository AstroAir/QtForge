# QtForge Python API Overview

!!! info "Python Integration"
**Module**: `qtforge`  
 **Python Version**: 3.8+  
 **Since**: QtForge v3.0.0  
 **Status**: Stable

## Overview

QtForge provides comprehensive Python bindings that expose the full C++ API to Python applications. The bindings are built using pybind11 and provide a Pythonic interface while maintaining the performance and capabilities of the underlying C++ implementation.

### Key Features

- **Complete API Coverage**: All major QtForge components available in Python
- **Type Safety**: Strong typing with proper error handling
- **Performance**: Minimal overhead with direct C++ integration
- **Pythonic Interface**: Natural Python syntax and conventions
- **Cross-Platform**: Works on Windows, macOS, and Linux
- **Qt Integration**: Seamless integration with PyQt/PySide applications

### Module Structure

The QtForge Python package is organized into the following submodules:

```python
import qtforge

# Core plugin system
qtforge.core          # Plugin management, loading, registry
qtforge.utils         # Utility classes and functions
qtforge.communication # Message bus and inter-plugin communication
qtforge.security     # Security and validation components

# Advanced features
qtforge.orchestration # Workflow management and plugin orchestration
qtforge.monitoring    # Hot reload, metrics collection, performance monitoring
qtforge.transactions  # ACID transactions for plugin operations
qtforge.composition   # Plugin composition and architectural patterns
qtforge.marketplace   # Plugin discovery, installation, and updates
qtforge.threading     # Thread pool management and concurrency utilities

# Management components
qtforge.managers      # Configuration, logging, and resource management
```

## Installation

### Prerequisites

- **Python 3.8+** with development headers
- **Qt6** (6.0 or later) with Python bindings (PyQt6 or PySide6)
- **QtForge C++ library** (v3.0.0 or later)
- **pybind11** (automatically handled by build system)

### Installation Methods

#### Using pip (Recommended)

```bash
# Install from PyPI (when available)
pip install qtforge

# Install with optional dependencies
pip install qtforge[qt,examples]
```

#### Using conda

```bash
# Install from conda-forge
conda install -c conda-forge qtforge

# Or create environment with all dependencies
conda env create -f environment.yml
```

#### Building from Source

```bash
# Clone the repository
git clone https://github.com/AstroAir/QtForge.git
cd QtForge

# Build Python bindings
mkdir build && cd build
cmake .. -DQTFORGE_BUILD_PYTHON=ON -DPYTHON_EXECUTABLE=$(which python)
cmake --build . --target qtforge_python

# Install
pip install .
```

- Python 3.8 or later
- Qt6 Core (and optional components)
- Compatible C++ compiler

### Building from Source

```bash
# Configure with Python bindings enabled
cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON ..
make

# Install Python package
pip install dist/qtforge-*.whl
```

### Package Installation

```bash
# Install from PyPI (when available)
pip install qtforge

# Install development version
pip install git+https://github.com/QtForge/QtForge.git
```

## Quick Start

### Basic Plugin Management

```python
import qtforge

# Create plugin manager
manager = qtforge.PluginManager()

# Load a plugin
result = manager.load_plugin("path/to/plugin.so")
if result:
    plugin_id = result.value()
    print(f"Loaded plugin: {plugin_id}")

    # Get plugin instance
    plugin = manager.get_plugin(plugin_id)
    if plugin:
        # Execute plugin command
        cmd_result = plugin.execute_command("hello", {"name": "World"})
        if cmd_result:
            print(f"Command result: {cmd_result.value()}")

# List loaded plugins
plugins = manager.get_loaded_plugins()
print(f"Loaded plugins: {plugins}")
```

### Message Bus Communication

```python
import qtforge

# Create message bus
bus = qtforge.communication.MessageBus()

# Subscribe to messages
def handle_message(message):
    print(f"Received: {message.topic} - {message.data}")

bus.subscribe("system.events", handle_message)

# Publish message
bus.publish("system.events", {"event": "startup", "timestamp": "2024-12-01"})
```

### Plugin Orchestration

```python
import qtforge

# Create orchestrator
orchestrator = qtforge.orchestration.PluginOrchestrator()

# Create workflow
workflow = qtforge.orchestration.Workflow("data_pipeline", "Data Processing")
workflow.set_execution_mode(qtforge.orchestration.ExecutionMode.Sequential)

# Add workflow steps
step1 = qtforge.orchestration.WorkflowStep("load", "data_loader", "load_csv")
step1.parameters = {"file": "input.csv"}

step2 = qtforge.orchestration.WorkflowStep("process", "data_processor", "transform")
step2.dependencies = ["load"]

workflow.add_step(step1).add_step(step2)

# Register and execute workflow
orchestrator.register_workflow(workflow)
execution_id = orchestrator.execute_workflow("data_pipeline")
```

### Monitoring and Metrics

```python
import qtforge

# Set up monitoring system
hot_reload, metrics = qtforge.monitoring.setup_monitoring_system()

# Enable hot reload for a plugin
result = hot_reload.enable_hot_reload("my_plugin", "/path/to/plugin.so")
if result:
    print("Hot reload enabled")

# Get system metrics
system_metrics = metrics.get_system_metrics(plugin_registry)
print(f"System CPU: {system_metrics['cpu_usage']}%")
print(f"Memory usage: {system_metrics['memory_usage']} MB")
```

## API Reference by Module

### Core Module (`qtforge.core`)

Essential plugin management functionality:

- **[PluginManager](core/plugin-manager.md)** - Central plugin management
- **[IPlugin](core/plugin-interface.md)** - Plugin interface and base classes
- **[PluginLoader](core/plugin-loader.md)** - Dynamic plugin loading
- **[PluginRegistry](core/plugin-registry.md)** - Plugin registration and discovery

### Communication Module (`qtforge.communication`)

Inter-plugin messaging and events:

- **[MessageBus](communication/message-bus.md)** - Publish/subscribe messaging
- **[Message Types](communication/message-types.md)** - Message definitions and filters

### Security Module (`qtforge.security`)

Plugin validation and trust management:

- **[SecurityManager](security/security-manager.md)** - Security policy enforcement
- **[PluginValidator](security/plugin-validator.md)** - Plugin validation and verification

### Orchestration Module (`qtforge.orchestration`)

Workflow management and plugin coordination:

- **[PluginOrchestrator](orchestration/plugin-orchestrator.md)** - Workflow execution engine
- **[Workflow](orchestration/workflow.md)** - Workflow definition and management
- **[WorkflowStep](orchestration/workflow-step.md)** - Individual workflow steps

### Monitoring Module (`qtforge.monitoring`)

Performance monitoring and hot reload:

- **[PluginHotReloadManager](monitoring/hot-reload-manager.md)** - Automatic plugin reloading
- **[PluginMetricsCollector](monitoring/metrics-collector.md)** - Performance metrics collection

### Transactions Module (`qtforge.transactions`)

ACID transaction support:

- **[PluginTransactionManager](transactions/transaction-manager.md)** - Transaction management
- **[TransactionOperation](transactions/transaction-operation.md)** - Transaction operations

### Composition Module (`qtforge.composition`)

Plugin composition and architectural patterns:

- **[CompositePlugin](composition/composite-plugin.md)** - Plugin composition framework
- **[PluginComposition](composition/plugin-composition.md)** - Composition definitions

### Marketplace Module (`qtforge.marketplace`)

Plugin discovery and installation:

- **[PluginMarketplace](marketplace/plugin-marketplace.md)** - Marketplace client
- **[MarketplaceManager](marketplace/marketplace-manager.md)** - Multi-marketplace management

### Threading Module (`qtforge.threading`)

Thread pool management and concurrency:

- **[PluginThreadPool](threading/thread-pool.md)** - Thread pool management
- **[ThreadPoolManager](threading/thread-pool-manager.md)** - Multi-pool coordination

### Managers Module (`qtforge.managers`)

Configuration and resource management:

- **[ConfigurationManager](managers/configuration-manager.md)** - Configuration management
- **[LoggingManager](managers/logging-manager.md)** - Logging system integration
- **[ResourceManager](managers/resource-manager.md)** - Resource allocation and monitoring

### Utils Module (`qtforge.utils`)

Utility classes and helper functions:

- **[Version](utils/version.md)** - Version handling and comparison
- **[Error Handling](utils/error-handling.md)** - Error types and exception handling

## Error Handling

QtForge Python bindings use a combination of exceptions and expected-style return values:

```python
import qtforge

try:
    manager = qtforge.PluginManager()
    result = manager.load_plugin("plugin.so")

    if result:
        plugin_id = result.value()
        print(f"Success: {plugin_id}")
    else:
        error = result.error()
        print(f"Error: {error.message}")

except qtforge.PluginError as e:
    print(f"Plugin error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")
```

## Type Hints and IDE Support

QtForge Python bindings include comprehensive type hints for better IDE support:

```python
from typing import Optional, List, Dict, Any
import qtforge

def process_plugins(manager: qtforge.PluginManager) -> List[str]:
    """Process all loaded plugins and return their IDs."""
    plugins: List[str] = manager.get_loaded_plugins()
    results: List[str] = []

    for plugin_id in plugins:
        plugin: Optional[qtforge.IPlugin] = manager.get_plugin(plugin_id)
        if plugin:
            metadata: qtforge.PluginMetadata = plugin.metadata()
            results.append(f"{metadata.name} v{metadata.version}")

    return results
```

## Integration with Qt Applications

QtForge integrates seamlessly with PyQt and PySide applications:

```python
import sys
from PyQt6.QtWidgets import QApplication, QMainWindow
import qtforge

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.plugin_manager = qtforge.PluginManager()
        self.setup_plugins()

    def setup_plugins(self):
        # Load plugins and integrate with Qt application
        result = self.plugin_manager.load_plugin("ui_plugin.so")
        if result:
            plugin = self.plugin_manager.get_plugin(result.value())
            # Plugin can create Qt widgets and integrate with main window

app = QApplication(sys.argv)
window = MainWindow()
window.show()
sys.exit(app.exec())
```

## Performance Considerations

- **Minimal Overhead**: Direct C++ integration with minimal Python overhead
- **Memory Management**: Automatic memory management with proper RAII
- **Threading**: Thread-safe operations where supported by underlying C++ API
- **GIL Considerations**: Long-running operations release the GIL when possible

## Debugging and Development

### Logging

```python
import qtforge
import logging

# Enable QtForge logging
qtforge.utils.enable_logging(logging.DEBUG)

# Use with Python logging
logger = logging.getLogger("qtforge")
logger.info("QtForge operations will be logged")
```

### Development Mode

```python
import qtforge

# Enable development features
qtforge.set_development_mode(True)

# Enable hot reload for development
hot_reload = qtforge.monitoring.PluginHotReloadManager()
hot_reload.set_global_hot_reload_enabled(True)
```

## See Also

- [Python Integration User Guide](../../user-guide/python-integration.md)
- [Python Examples](../../examples/python-examples.md)
- [C++ to Python Migration Guide](../../migration/cpp-to-python.md)
- [Performance Optimization](../../performance/python-optimization.md)

---

_Last updated: December 2024 | QtForge v3.0.0_
