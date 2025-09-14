# Python Plugin Interface

The Python Plugin Interface provides the base classes and protocols for developing plugins in Python.

## Overview

The Python Plugin Interface enables:
- **Plugin Development**: Create plugins using Python
- **Lifecycle Management**: Handle plugin initialization and cleanup
- **Event Handling**: Respond to system and plugin events
- **Service Integration**: Integrate with QtForge services
- **Cross-language Communication**: Interact with C++ components

## Base Classes

### PluginInterface

```python
from abc import ABC, abstractmethod
from typing import Dict, Any, Optional
from qtforge.core import PluginMetadata, PluginContext

class PluginInterface(ABC):
    """Base interface for all Python plugins."""
    
    def __init__(self):
        self._metadata: Optional[PluginMetadata] = None
        self._context: Optional[PluginContext] = None
        self._initialized = False
    
    @abstractmethod
    def get_metadata(self) -> PluginMetadata:
        """Return plugin metadata."""
        pass
    
    @abstractmethod
    def initialize(self, context: PluginContext) -> bool:
        """Initialize the plugin."""
        pass
    
    @abstractmethod
    def shutdown(self) -> bool:
        """Shutdown the plugin."""
        pass
    
    def is_initialized(self) -> bool:
        """Check if plugin is initialized."""
        return self._initialized
    
    def get_context(self) -> Optional[PluginContext]:
        """Get plugin context."""
        return self._context
```

### ServicePlugin

```python
from qtforge.core import ServiceInterface

class ServicePlugin(PluginInterface):
    """Base class for service-providing plugins."""
    
    @abstractmethod
    def get_service_interface(self) -> ServiceInterface:
        """Return the service interface."""
        pass
    
    @abstractmethod
    def start_service(self) -> bool:
        """Start the service."""
        pass
    
    @abstractmethod
    def stop_service(self) -> bool:
        """Stop the service."""
        pass
    
    def is_service_running(self) -> bool:
        """Check if service is running."""
        return hasattr(self, '_service_running') and self._service_running
```

## Plugin Metadata

```python
from dataclasses import dataclass
from typing import List, Dict, Any
from enum import Enum

class PluginType(Enum):
    SERVICE = "service"
    UTILITY = "utility"
    UI = "ui"
    FILTER = "filter"
    EXTENSION = "extension"

@dataclass
class PluginMetadata:
    """Plugin metadata information."""
    id: str
    name: str
    version: str
    description: str
    author: str
    plugin_type: PluginType
    dependencies: List[str] = None
    capabilities: List[str] = None
    configuration: Dict[str, Any] = None
    
    def __post_init__(self):
        if self.dependencies is None:
            self.dependencies = []
        if self.capabilities is None:
            self.capabilities = []
        if self.configuration is None:
            self.configuration = {}
```

## Usage Examples

### Basic Plugin Implementation

```python
from qtforge.core import PluginInterface, PluginMetadata, PluginType

class MyPlugin(PluginInterface):
    def get_metadata(self) -> PluginMetadata:
        return PluginMetadata(
            id="com.example.myplugin",
            name="My Plugin",
            version="1.0.0",
            description="Example Python plugin",
            author="Example Author",
            plugin_type=PluginType.UTILITY,
            capabilities=["data_processing", "file_handling"]
        )
    
    def initialize(self, context) -> bool:
        self._context = context
        self._initialized = True
        
        # Initialize plugin resources
        self.setup_resources()
        
        # Register event handlers
        context.get_event_bus().subscribe("data_event", self.handle_data_event)
        
        return True
    
    def shutdown(self) -> bool:
        if self._context:
            # Unregister event handlers
            self._context.get_event_bus().unsubscribe("data_event", self.handle_data_event)
            
            # Cleanup resources
            self.cleanup_resources()
        
        self._initialized = False
        return True
    
    def setup_resources(self):
        """Setup plugin-specific resources."""
        pass
    
    def cleanup_resources(self):
        """Cleanup plugin-specific resources."""
        pass
    
    def handle_data_event(self, event):
        """Handle data events."""
        print(f"Received data event: {event}")
```

### Service Plugin Implementation

```python
from qtforge.core import ServicePlugin, ServiceInterface
import threading

class DataProcessingService(ServiceInterface):
    def process_data(self, data):
        """Process data and return result."""
        return f"Processed: {data}"

class DataProcessingPlugin(ServicePlugin):
    def __init__(self):
        super().__init__()
        self._service = DataProcessingService()
        self._service_running = False
        self._worker_thread = None
    
    def get_metadata(self) -> PluginMetadata:
        return PluginMetadata(
            id="com.example.dataprocessor",
            name="Data Processing Plugin",
            version="1.0.0",
            description="Provides data processing services",
            author="Example Author",
            plugin_type=PluginType.SERVICE,
            capabilities=["data_processing", "background_service"]
        )
    
    def get_service_interface(self) -> ServiceInterface:
        return self._service
    
    def start_service(self) -> bool:
        if not self._service_running:
            self._service_running = True
            self._worker_thread = threading.Thread(target=self._service_worker)
            self._worker_thread.start()
        return True
    
    def stop_service(self) -> bool:
        if self._service_running:
            self._service_running = False
            if self._worker_thread:
                self._worker_thread.join()
        return True
    
    def _service_worker(self):
        """Background service worker."""
        while self._service_running:
            # Perform background processing
            time.sleep(1)
```

### Plugin with Configuration

```python
class ConfigurablePlugin(PluginInterface):
    def get_metadata(self) -> PluginMetadata:
        return PluginMetadata(
            id="com.example.configurable",
            name="Configurable Plugin",
            version="1.0.0",
            description="Plugin with configuration support",
            author="Example Author",
            plugin_type=PluginType.UTILITY,
            configuration={
                "max_items": 100,
                "timeout": 30,
                "enabled_features": ["feature1", "feature2"]
            }
        )
    
    def initialize(self, context) -> bool:
        self._context = context
        
        # Load configuration
        config = context.get_configuration_manager().get_plugin_config(
            self.get_metadata().id
        )
        
        self.max_items = config.get("max_items", 100)
        self.timeout = config.get("timeout", 30)
        self.enabled_features = config.get("enabled_features", [])
        
        self._initialized = True
        return True
```

## Event Handling

```python
def initialize(self, context) -> bool:
    # Subscribe to events
    event_bus = context.get_event_bus()
    event_bus.subscribe("plugin_loaded", self.on_plugin_loaded)
    event_bus.subscribe("system_shutdown", self.on_system_shutdown)
    
    return True

def on_plugin_loaded(self, event):
    """Handle plugin loaded event."""
    plugin_id = event.get("plugin_id")
    print(f"Plugin loaded: {plugin_id}")

def on_system_shutdown(self, event):
    """Handle system shutdown event."""
    print("System is shutting down, preparing for cleanup")
```

## Error Handling

```python
from qtforge.core import PluginException

def initialize(self, context) -> bool:
    try:
        # Plugin initialization code
        self.setup_resources()
        self._initialized = True
        return True
    except Exception as e:
        raise PluginException(f"Failed to initialize plugin: {e}")

def setup_resources(self):
    """Setup resources with error handling."""
    try:
        # Resource setup code
        pass
    except IOError as e:
        raise PluginException(f"Failed to setup resources: {e}")
```

## Plugin Registration

```python
# Plugin entry point
def create_plugin():
    """Factory function to create plugin instance."""
    return MyPlugin()

# Alternative: Class-based registration
class PluginFactory:
    @staticmethod
    def create():
        return MyPlugin()
```

## Thread Safety

Python plugins should be thread-safe when accessing shared resources:

```python
import threading

class ThreadSafePlugin(PluginInterface):
    def __init__(self):
        super().__init__()
        self._lock = threading.Lock()
        self._data = {}
    
    def set_data(self, key, value):
        with self._lock:
            self._data[key] = value
    
    def get_data(self, key):
        with self._lock:
            return self._data.get(key)
```

## See Also

- [Plugin Manager](plugin-manager.md)
- [Plugin Loader](plugin-loader.md)
- [Plugin Registry](plugin-registry.md)
- [Python Integration Tutorial](../../../tutorials/python-integration-tutorial.md)
