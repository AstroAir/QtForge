# Python Plugin Registry

The Python Plugin Registry provides a centralized system for managing plugin registration, discovery, and lifecycle operations in Python-based plugins.

## Overview

The `PluginRegistry` class in the Python binding provides a complete interface for plugin management operations, mirroring the functionality of the C++ core system.

## Class Definition

```python
from qtforge.core import PluginRegistry
from qtforge.types import PluginInfo, PluginMetadata
from typing import List, Optional, Dict, Any

class PluginRegistry:
    """Central registry for plugin management operations."""
    
    def __init__(self):
        """Initialize the plugin registry."""
        pass
    
    def register_plugin(self, plugin_info: PluginInfo) -> bool:
        """Register a plugin in the registry."""
        pass
    
    def unregister_plugin(self, plugin_id: str) -> bool:
        """Unregister a plugin from the registry."""
        pass
    
    def get_plugin_info(self, plugin_id: str) -> Optional[PluginInfo]:
        """Get plugin information by ID."""
        pass
    
    def list_plugins(self) -> List[PluginInfo]:
        """List all registered plugins."""
        pass
    
    def find_plugins_by_type(self, plugin_type: str) -> List[PluginInfo]:
        """Find plugins by type."""
        pass
```

## Key Methods

### Registration Methods

#### register_plugin()

```python
def register_plugin(self, plugin_info: PluginInfo) -> bool:
    """
    Register a plugin in the registry.
    
    Args:
        plugin_info: Plugin information object
        
    Returns:
        bool: True if registration successful, False otherwise
        
    Example:
        >>> registry = PluginRegistry()
        >>> info = PluginInfo(
        ...     id="example.plugin",
        ...     name="Example Plugin",
        ...     version="1.0.0",
        ...     type="service"
        ... )
        >>> success = registry.register_plugin(info)
        >>> print(f"Registration: {'Success' if success else 'Failed'}")
    """
```

#### unregister_plugin()

```python
def unregister_plugin(self, plugin_id: str) -> bool:
    """
    Unregister a plugin from the registry.
    
    Args:
        plugin_id: Unique plugin identifier
        
    Returns:
        bool: True if unregistration successful, False otherwise
        
    Example:
        >>> success = registry.unregister_plugin("example.plugin")
        >>> print(f"Unregistration: {'Success' if success else 'Failed'}")
    """
```

### Query Methods

#### get_plugin_info()

```python
def get_plugin_info(self, plugin_id: str) -> Optional[PluginInfo]:
    """
    Get plugin information by ID.
    
    Args:
        plugin_id: Unique plugin identifier
        
    Returns:
        Optional[PluginInfo]: Plugin information or None if not found
        
    Example:
        >>> info = registry.get_plugin_info("example.plugin")
        >>> if info:
        ...     print(f"Found plugin: {info.name}")
        ... else:
        ...     print("Plugin not found")
    """
```

#### list_plugins()

```python
def list_plugins(self) -> List[PluginInfo]:
    """
    List all registered plugins.
    
    Returns:
        List[PluginInfo]: List of all registered plugin information
        
    Example:
        >>> all_plugins = registry.list_plugins()
        >>> print(f"Total plugins: {len(all_plugins)}")
        >>> for plugin in all_plugins:
        ...     print(f"  - {plugin.name} ({plugin.version})")
    """
```

#### find_plugins_by_type()

```python
def find_plugins_by_type(self, plugin_type: str) -> List[PluginInfo]:
    """
    Find plugins by type.
    
    Args:
        plugin_type: Plugin type to search for
        
    Returns:
        List[PluginInfo]: List of plugins matching the type
        
    Example:
        >>> service_plugins = registry.find_plugins_by_type("service")
        >>> print(f"Service plugins: {len(service_plugins)}")
        >>> for plugin in service_plugins:
        ...     print(f"  - {plugin.name}")
    """
```

## Data Types

### PluginInfo

```python
from dataclasses import dataclass
from typing import Dict, Any, Optional

@dataclass
class PluginInfo:
    """Plugin information container."""
    id: str
    name: str
    version: str
    type: str
    description: Optional[str] = None
    author: Optional[str] = None
    dependencies: List[str] = None
    metadata: Dict[str, Any] = None
    
    def __post_init__(self):
        if self.dependencies is None:
            self.dependencies = []
        if self.metadata is None:
            self.metadata = {}
```

## Usage Examples

### Basic Registration

```python
from qtforge.core import PluginRegistry
from qtforge.types import PluginInfo

# Create registry instance
registry = PluginRegistry()

# Create plugin info
plugin_info = PluginInfo(
    id="data.processor",
    name="Data Processor Plugin",
    version="2.1.0",
    type="service",
    description="Handles data processing operations",
    author="QtForge Team",
    dependencies=["core.utils", "data.validator"],
    metadata={
        "priority": 10,
        "auto_start": True,
        "config_schema": "data_processor_schema.json"
    }
)

# Register the plugin
if registry.register_plugin(plugin_info):
    print("Plugin registered successfully")
else:
    print("Plugin registration failed")
```

### Plugin Discovery

```python
# List all plugins
all_plugins = registry.list_plugins()
print(f"Total registered plugins: {len(all_plugins)}")

# Find plugins by type
service_plugins = registry.find_plugins_by_type("service")
ui_plugins = registry.find_plugins_by_type("ui")

print(f"Service plugins: {len(service_plugins)}")
print(f"UI plugins: {len(ui_plugins)}")

# Get specific plugin info
plugin_info = registry.get_plugin_info("data.processor")
if plugin_info:
    print(f"Plugin: {plugin_info.name}")
    print(f"Version: {plugin_info.version}")
    print(f"Type: {plugin_info.type}")
    print(f"Dependencies: {', '.join(plugin_info.dependencies)}")
```

### Batch Operations

```python
# Register multiple plugins
plugins_to_register = [
    PluginInfo(id="plugin1", name="Plugin 1", version="1.0.0", type="core"),
    PluginInfo(id="plugin2", name="Plugin 2", version="1.0.0", type="service"),
    PluginInfo(id="plugin3", name="Plugin 3", version="1.0.0", type="ui"),
]

for plugin_info in plugins_to_register:
    success = registry.register_plugin(plugin_info)
    print(f"Registered {plugin_info.name}: {success}")

# Verify all plugins are registered
all_plugins = registry.list_plugins()
print(f"After batch registration: {len(all_plugins)} plugins")
```

## Integration with Plugin Manager

The Plugin Registry works closely with the [Plugin Manager](plugin-manager.md):

```python
from qtforge.core import PluginManager, PluginRegistry

# The plugin manager typically manages its own registry
manager = PluginManager()
registry = manager.get_registry()  # Get the internal registry

# Or you can provide your own registry
custom_registry = PluginRegistry()
manager = PluginManager(registry=custom_registry)
```

## Error Handling

```python
try:
    plugin_info = PluginInfo(
        id="example.plugin",
        name="Example Plugin",
        version="1.0.0",
        type="invalid_type"  # This might cause validation errors
    )
    
    if not registry.register_plugin(plugin_info):
        print("Registration failed - check plugin information")
        
except Exception as e:
    print(f"Error during registration: {e}")
```

## Thread Safety

The Python Plugin Registry is thread-safe and can be used from multiple threads:

```python
import threading
from qtforge.core import PluginRegistry

registry = PluginRegistry()

def register_plugin_worker(plugin_info):
    """Worker function for registering plugins in threads."""
    success = registry.register_plugin(plugin_info)
    print(f"Thread {threading.current_thread().name}: {success}")

# Create multiple threads to register plugins
threads = []
for i in range(5):
    plugin_info = PluginInfo(
        id=f"plugin{i}",
        name=f"Plugin {i}",
        version="1.0.0",
        type="service"
    )
    thread = threading.Thread(
        target=register_plugin_worker,
        args=(plugin_info,),
        name=f"PluginRegister-{i}"
    )
    threads.append(thread)
    thread.start()

# Wait for all threads to complete
for thread in threads:
    thread.join()
```

## See Also

- [Plugin Manager](plugin-manager.md) - Main plugin management interface
- [Plugin Interface](plugin-interface.md) - Plugin implementation interface  
- [Plugin Loader](plugin-loader.md) - Plugin loading operations
- [Python Overview](../overview.md) - Python bindings overview
