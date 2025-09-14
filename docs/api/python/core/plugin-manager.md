# PluginManager Python API

!!! info "Python Module"
**Module**: `qtforge.core`  
 **Class**: `PluginManager`  
 **C++ Equivalent**: `qtplugin::PluginManager`  
 **Since**: QtForge v3.0.0

## Overview

The PluginManager is the central component for managing plugins in Python applications. It provides a Pythonic interface to the underlying C++ plugin management system with automatic memory management and exception handling.

### Key Features

- **Plugin Lifecycle Management**: Load, unload, and manage plugin states
- **Dependency Resolution**: Automatic plugin dependency handling
- **Plugin Discovery**: Find and enumerate available plugins
- **Error Handling**: Comprehensive error reporting with Python exceptions
- **Thread Safety**: Safe concurrent access to plugin operations

## Class Reference

### Constructor

```python
import qtforge

# Create plugin manager with default settings
manager = qtforge.PluginManager()

# Alternative creation methods
manager = qtforge.core.PluginManager()
manager = qtforge.create_plugin_manager()
```

### Plugin Loading

#### `load_plugin()`

```python
def load_plugin(self, plugin_path: str) -> qtforge.Expected[str, qtforge.PluginError]:
    """Load a plugin from the specified path.

    Args:
        plugin_path: Path to the plugin file (.so, .dll, .dylib)

    Returns:
        Expected containing plugin ID on success, or PluginError on failure

    Example:
        >>> manager = qtforge.PluginManager()
        >>> result = manager.load_plugin("plugins/example.so")
        >>> if result:
        ...     plugin_id = result.value()
        ...     print(f"Loaded plugin: {plugin_id}")
        ... else:
        ...     print(f"Load failed: {result.error().message}")
    """
```

#### `load_plugin_from_data()`

```python
def load_plugin_from_data(self, plugin_data: bytes, plugin_id: str) -> qtforge.Expected[str, qtforge.PluginError]:
    """Load a plugin from memory data.

    Args:
        plugin_data: Plugin binary data
        plugin_id: Unique identifier for the plugin

    Returns:
        Expected containing plugin ID on success, or PluginError on failure
    """
```

#### `unload_plugin()`

```python
def unload_plugin(self, plugin_id: str) -> qtforge.Expected[None, qtforge.PluginError]:
    """Unload a plugin by ID.

    Args:
        plugin_id: Plugin identifier to unload

    Returns:
        Expected indicating success or failure

    Example:
        >>> result = manager.unload_plugin("my_plugin")
        >>> if result:
        ...     print("Plugin unloaded successfully")
        ... else:
        ...     print(f"Unload failed: {result.error().message}")
    """
```

### Plugin Access

#### `get_plugin()`

```python
def get_plugin(self, plugin_id: str) -> Optional[qtforge.IPlugin]:
    """Get a plugin instance by ID.

    Args:
        plugin_id: Plugin identifier

    Returns:
        Plugin instance or None if not found

    Example:
        >>> plugin = manager.get_plugin("my_plugin")
        >>> if plugin:
        ...     result = plugin.execute_command("hello", {"name": "World"})
        ...     if result:
        ...         print(f"Result: {result.value()}")
    """
```

#### `has_plugin()`

```python
def has_plugin(self, plugin_id: str) -> bool:
    """Check if a plugin is loaded.

    Args:
        plugin_id: Plugin identifier to check

    Returns:
        True if plugin is loaded, False otherwise
    """
```

#### `get_loaded_plugins()`

```python
def get_loaded_plugins(self) -> List[str]:
    """Get list of all loaded plugin IDs.

    Returns:
        List of plugin identifiers

    Example:
        >>> plugins = manager.get_loaded_plugins()
        >>> print(f"Loaded plugins: {plugins}")
        ['plugin1', 'plugin2', 'plugin3']
    """
```

### Plugin Discovery

#### `discover_plugins()`

```python
def discover_plugins(self, directory: str) -> List[str]:
    """Discover plugins in a directory.

    Args:
        directory: Directory path to search for plugins

    Returns:
        List of discovered plugin file paths

    Example:
        >>> plugins = manager.discover_plugins("./plugins")
        >>> for plugin_path in plugins:
        ...     print(f"Found plugin: {plugin_path}")
    """
```

#### `scan_plugin_directories()`

```python
def scan_plugin_directories(self) -> List[str]:
    """Scan configured plugin directories for available plugins.

    Returns:
        List of discovered plugin paths
    """
```

### Plugin Information

#### `get_plugin_metadata()`

```python
def get_plugin_metadata(self, plugin_id: str) -> Optional[qtforge.PluginMetadata]:
    """Get metadata for a loaded plugin.

    Args:
        plugin_id: Plugin identifier

    Returns:
        Plugin metadata or None if not found

    Example:
        >>> metadata = manager.get_plugin_metadata("my_plugin")
        >>> if metadata:
        ...     print(f"Name: {metadata.name}")
        ...     print(f"Version: {metadata.version}")
        ...     print(f"Author: {metadata.author}")
    """
```

#### `get_plugin_state()`

```python
def get_plugin_state(self, plugin_id: str) -> qtforge.PluginState:
    """Get the current state of a plugin.

    Args:
        plugin_id: Plugin identifier

    Returns:
        Current plugin state

    Example:
        >>> state = manager.get_plugin_state("my_plugin")
        >>> if state == qtforge.PluginState.Running:
        ...     print("Plugin is running")
    """
```

### Dependency Management

#### `resolve_dependencies()`

```python
def resolve_dependencies(self, plugin_id: str) -> qtforge.Expected[None, qtforge.PluginError]:
    """Resolve dependencies for a plugin.

    Args:
        plugin_id: Plugin identifier

    Returns:
        Expected indicating success or failure
    """
```

#### `get_dependencies()`

```python
def get_dependencies(self, plugin_id: str) -> List[str]:
    """Get dependencies for a plugin.

    Args:
        plugin_id: Plugin identifier

    Returns:
        List of dependency plugin IDs
    """
```

#### `get_dependents()`

```python
def get_dependents(self, plugin_id: str) -> List[str]:
    """Get plugins that depend on the specified plugin.

    Args:
        plugin_id: Plugin identifier

    Returns:
        List of dependent plugin IDs
    """
```

### Configuration

#### `set_plugin_directory()`

```python
def set_plugin_directory(self, directory: str) -> None:
    """Set the default plugin directory.

    Args:
        directory: Path to plugin directory
    """
```

#### `add_plugin_directory()`

```python
def add_plugin_directory(self, directory: str) -> None:
    """Add a plugin search directory.

    Args:
        directory: Path to add to plugin search paths
    """
```

#### `get_plugin_directories()`

```python
def get_plugin_directories(self) -> List[str]:
    """Get list of configured plugin directories.

    Returns:
        List of plugin directory paths
    """
```

## Usage Examples

### Basic Plugin Management

```python
import qtforge

def main():
    # Create plugin manager
    manager = qtforge.PluginManager()

    # Set plugin directory
    manager.set_plugin_directory("./plugins")

    # Discover available plugins
    available_plugins = manager.discover_plugins("./plugins")
    print(f"Found {len(available_plugins)} plugins")

    # Load plugins
    loaded_plugins = []
    for plugin_path in available_plugins:
        result = manager.load_plugin(plugin_path)
        if result:
            plugin_id = result.value()
            loaded_plugins.append(plugin_id)
            print(f"Loaded: {plugin_id}")
        else:
            print(f"Failed to load {plugin_path}: {result.error().message}")

    # Use loaded plugins
    for plugin_id in loaded_plugins:
        plugin = manager.get_plugin(plugin_id)
        if plugin:
            metadata = plugin.metadata()
            print(f"Plugin: {metadata.name} v{metadata.version}")

            # Execute plugin commands
            commands = plugin.available_commands()
            for command in commands:
                result = plugin.execute_command(command)
                if result:
                    print(f"Command '{command}' result: {result.value()}")

if __name__ == "__main__":
    main()
```

### Advanced Plugin Management

```python
import qtforge
from typing import Dict, List, Optional

class PluginService:
    def __init__(self):
        self.manager = qtforge.PluginManager()
        self.manager.add_plugin_directory("./plugins")
        self.manager.add_plugin_directory("./system_plugins")

    def load_all_plugins(self) -> Dict[str, bool]:
        """Load all discovered plugins and return success status."""
        results = {}

        for directory in self.manager.get_plugin_directories():
            plugins = self.manager.discover_plugins(directory)

            for plugin_path in plugins:
                result = self.manager.load_plugin(plugin_path)
                if result:
                    plugin_id = result.value()
                    results[plugin_id] = True

                    # Resolve dependencies
                    dep_result = self.manager.resolve_dependencies(plugin_id)
                    if not dep_result:
                        print(f"Warning: Failed to resolve dependencies for {plugin_id}")
                else:
                    results[plugin_path] = False
                    print(f"Failed to load {plugin_path}: {result.error().message}")

        return results

    def get_plugin_info(self, plugin_id: str) -> Optional[Dict]:
        """Get comprehensive plugin information."""
        if not self.manager.has_plugin(plugin_id):
            return None

        plugin = self.manager.get_plugin(plugin_id)
        metadata = self.manager.get_plugin_metadata(plugin_id)
        state = self.manager.get_plugin_state(plugin_id)
        dependencies = self.manager.get_dependencies(plugin_id)
        dependents = self.manager.get_dependents(plugin_id)

        return {
            "id": plugin_id,
            "name": metadata.name if metadata else "Unknown",
            "version": str(metadata.version) if metadata else "Unknown",
            "author": metadata.author if metadata else "Unknown",
            "state": state.name,
            "dependencies": dependencies,
            "dependents": dependents,
            "commands": plugin.available_commands() if plugin else []
        }

    def execute_plugin_command(self, plugin_id: str, command: str, params: Dict = None) -> Optional[Dict]:
        """Execute a command on a plugin with error handling."""
        plugin = self.manager.get_plugin(plugin_id)
        if not plugin:
            return {"error": f"Plugin {plugin_id} not found"}

        try:
            result = plugin.execute_command(command, params or {})
            if result:
                return {"success": True, "result": result.value()}
            else:
                return {"error": result.error().message}
        except Exception as e:
            return {"error": str(e)}

# Usage
service = PluginService()
load_results = service.load_all_plugins()
print(f"Loaded {sum(load_results.values())} plugins successfully")

# Get info for all loaded plugins
for plugin_id in service.manager.get_loaded_plugins():
    info = service.get_plugin_info(plugin_id)
    if info:
        print(f"Plugin: {info['name']} ({info['id']}) - State: {info['state']}")
```

## Error Handling

```python
import qtforge

def safe_plugin_operation():
    manager = qtforge.PluginManager()

    try:
        # Load plugin with error handling
        result = manager.load_plugin("plugins/example.so")
        if not result:
            error = result.error()
            if error.code == qtforge.PluginErrorCode.FileNotFound:
                print("Plugin file not found")
            elif error.code == qtforge.PluginErrorCode.LoadFailed:
                print("Plugin failed to load")
            else:
                print(f"Unknown error: {error.message}")
            return

        plugin_id = result.value()
        plugin = manager.get_plugin(plugin_id)

        if plugin:
            # Execute command with error handling
            cmd_result = plugin.execute_command("process_data", {"input": "test"})
            if cmd_result:
                print(f"Success: {cmd_result.value()}")
            else:
                print(f"Command failed: {cmd_result.error().message}")

    except qtforge.PluginError as e:
        print(f"Plugin system error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")
```

## Thread Safety

```python
import qtforge
import threading
from concurrent.futures import ThreadPoolExecutor

def thread_safe_plugin_usage():
    manager = qtforge.PluginManager()

    # Load plugins (thread-safe)
    result = manager.load_plugin("plugins/worker.so")
    if not result:
        return

    plugin_id = result.value()

    def worker_task(task_id: int):
        """Worker function that uses plugin in thread-safe manner."""
        plugin = manager.get_plugin(plugin_id)  # Thread-safe
        if plugin:
            result = plugin.execute_command("process", {"task_id": task_id})
            if result:
                print(f"Task {task_id} completed: {result.value()}")
            else:
                print(f"Task {task_id} failed: {result.error().message}")

    # Execute plugin commands from multiple threads
    with ThreadPoolExecutor(max_workers=4) as executor:
        futures = [executor.submit(worker_task, i) for i in range(10)]

        # Wait for all tasks to complete
        for future in futures:
            future.result()
```

## See Also

- [IPlugin Python API](plugin-interface.md)
- [PluginLoader Python API](plugin-loader.md)
- [PluginRegistry Python API](plugin-registry.md)
- [Python Integration Guide](../../../user-guide/python-integration.md)

---

_Last updated: December 2024 | QtForge v3.0.0_
