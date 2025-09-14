# Python Plugin Loader

The Python Plugin Loader provides functionality for loading, managing, and executing Python-based plugins within the QtForge framework.

## Overview

The Python Plugin Loader enables:
- **Dynamic Loading**: Load Python plugins at runtime
- **Module Management**: Manage Python modules and dependencies
- **Isolation**: Provide isolated execution environments
- **Integration**: Seamless integration with C++ plugin system
- **Hot Reload**: Support for hot reloading of Python plugins

## Core Classes

### PluginLoader

```python
from qtforge.core import PluginLoader, PluginMetadata
from typing import Optional, List, Dict, Any
import importlib.util
import sys

class PythonPluginLoader:
    """Loads and manages Python plugins."""
    
    def __init__(self):
        self._loaded_plugins: Dict[str, Any] = {}
        self._plugin_modules: Dict[str, Any] = {}
        self._plugin_paths: Dict[str, str] = {}
    
    def load_plugin(self, plugin_path: str) -> Optional[Any]:
        """
        Load a Python plugin from the specified path.
        
        Args:
            plugin_path: Path to the Python plugin file
            
        Returns:
            Plugin instance if successful, None otherwise
        """
        try:
            # Load plugin metadata
            metadata = self._load_plugin_metadata(plugin_path)
            if not metadata:
                return None
            
            # Load plugin module
            module = self._load_plugin_module(plugin_path, metadata.id)
            if not module:
                return None
            
            # Create plugin instance
            plugin_instance = self._create_plugin_instance(module, metadata)
            if not plugin_instance:
                return None
            
            # Store plugin information
            self._loaded_plugins[metadata.id] = plugin_instance
            self._plugin_modules[metadata.id] = module
            self._plugin_paths[metadata.id] = plugin_path
            
            return plugin_instance
            
        except Exception as e:
            print(f"Failed to load plugin {plugin_path}: {e}")
            return None
    
    def unload_plugin(self, plugin_id: str) -> bool:
        """
        Unload a Python plugin.
        
        Args:
            plugin_id: ID of the plugin to unload
            
        Returns:
            True if successful, False otherwise
        """
        try:
            if plugin_id not in self._loaded_plugins:
                return False
            
            # Shutdown plugin
            plugin = self._loaded_plugins[plugin_id]
            if hasattr(plugin, 'shutdown'):
                plugin.shutdown()
            
            # Remove from loaded plugins
            del self._loaded_plugins[plugin_id]
            
            # Remove module from sys.modules if it's safe
            if plugin_id in self._plugin_modules:
                module = self._plugin_modules[plugin_id]
                module_name = getattr(module, '__name__', None)
                if module_name and module_name in sys.modules:
                    del sys.modules[module_name]
                del self._plugin_modules[plugin_id]
            
            # Remove path mapping
            if plugin_id in self._plugin_paths:
                del self._plugin_paths[plugin_id]
            
            return True
            
        except Exception as e:
            print(f"Failed to unload plugin {plugin_id}: {e}")
            return False
    
    def reload_plugin(self, plugin_id: str) -> bool:
        """
        Reload a Python plugin.
        
        Args:
            plugin_id: ID of the plugin to reload
            
        Returns:
            True if successful, False otherwise
        """
        if plugin_id not in self._plugin_paths:
            return False
        
        plugin_path = self._plugin_paths[plugin_id]
        
        # Unload current plugin
        if not self.unload_plugin(plugin_id):
            return False
        
        # Load plugin again
        return self.load_plugin(plugin_path) is not None
    
    def get_plugin(self, plugin_id: str) -> Optional[Any]:
        """
        Get a loaded plugin instance.
        
        Args:
            plugin_id: ID of the plugin
            
        Returns:
            Plugin instance if found, None otherwise
        """
        return self._loaded_plugins.get(plugin_id)
    
    def get_loaded_plugins(self) -> List[str]:
        """
        Get list of loaded plugin IDs.
        
        Returns:
            List of plugin IDs
        """
        return list(self._loaded_plugins.keys())
    
    def is_plugin_loaded(self, plugin_id: str) -> bool:
        """
        Check if a plugin is loaded.
        
        Args:
            plugin_id: ID of the plugin
            
        Returns:
            True if loaded, False otherwise
        """
        return plugin_id in self._loaded_plugins
    
    def _load_plugin_metadata(self, plugin_path: str) -> Optional[PluginMetadata]:
        """Load plugin metadata from file."""
        import json
        import os
        
        # Look for metadata file
        plugin_dir = os.path.dirname(plugin_path)
        metadata_file = os.path.join(plugin_dir, 'plugin.json')
        
        if not os.path.exists(metadata_file):
            # Try to extract metadata from plugin file
            return self._extract_metadata_from_plugin(plugin_path)
        
        try:
            with open(metadata_file, 'r') as f:
                metadata_dict = json.load(f)
                return PluginMetadata.from_dict(metadata_dict)
        except Exception as e:
            print(f"Failed to load metadata from {metadata_file}: {e}")
            return None
    
    def _extract_metadata_from_plugin(self, plugin_path: str) -> Optional[PluginMetadata]:
        """Extract metadata from plugin file docstring or attributes."""
        try:
            spec = importlib.util.spec_from_file_location("temp_plugin", plugin_path)
            if not spec or not spec.loader:
                return None
            
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            
            # Look for metadata in module
            if hasattr(module, 'PLUGIN_METADATA'):
                metadata_dict = module.PLUGIN_METADATA
                return PluginMetadata.from_dict(metadata_dict)
            
            # Look for plugin class with metadata
            for attr_name in dir(module):
                attr = getattr(module, attr_name)
                if hasattr(attr, 'get_metadata'):
                    try:
                        instance = attr()
                        return instance.get_metadata()
                    except:
                        continue
            
            return None
            
        except Exception as e:
            print(f"Failed to extract metadata from {plugin_path}: {e}")
            return None
    
    def _load_plugin_module(self, plugin_path: str, plugin_id: str) -> Optional[Any]:
        """Load plugin module from file."""
        try:
            spec = importlib.util.spec_from_file_location(plugin_id, plugin_path)
            if not spec or not spec.loader:
                return None
            
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            
            return module
            
        except Exception as e:
            print(f"Failed to load module from {plugin_path}: {e}")
            return None
    
    def _create_plugin_instance(self, module: Any, metadata: PluginMetadata) -> Optional[Any]:
        """Create plugin instance from module."""
        try:
            # Look for plugin factory function
            if hasattr(module, 'create_plugin'):
                return module.create_plugin()
            
            # Look for plugin class
            plugin_class = None
            for attr_name in dir(module):
                attr = getattr(module, attr_name)
                if (isinstance(attr, type) and 
                    hasattr(attr, 'get_metadata') and 
                    hasattr(attr, 'initialize')):
                    plugin_class = attr
                    break
            
            if plugin_class:
                return plugin_class()
            
            print(f"No plugin class or factory found in module")
            return None
            
        except Exception as e:
            print(f"Failed to create plugin instance: {e}")
            return None
```

### Plugin Discovery

```python
class PythonPluginDiscovery:
    """Discovers Python plugins in specified directories."""
    
    def __init__(self):
        self._search_paths: List[str] = []
        self._discovered_plugins: Dict[str, str] = {}
    
    def add_search_path(self, path: str) -> None:
        """
        Add a directory to search for plugins.
        
        Args:
            path: Directory path to search
        """
        if path not in self._search_paths:
            self._search_paths.append(path)
    
    def discover_plugins(self) -> Dict[str, str]:
        """
        Discover all Python plugins in search paths.
        
        Returns:
            Dictionary mapping plugin IDs to file paths
        """
        import os
        import glob
        
        discovered = {}
        
        for search_path in self._search_paths:
            if not os.path.exists(search_path):
                continue
            
            # Look for Python files
            pattern = os.path.join(search_path, "**", "*.py")
            python_files = glob.glob(pattern, recursive=True)
            
            for python_file in python_files:
                plugin_info = self._analyze_python_file(python_file)
                if plugin_info:
                    plugin_id, _ = plugin_info
                    discovered[plugin_id] = python_file
        
        self._discovered_plugins = discovered
        return discovered
    
    def _analyze_python_file(self, file_path: str) -> Optional[tuple]:
        """Analyze Python file to determine if it's a plugin."""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Simple heuristic: look for plugin-related imports or classes
            plugin_indicators = [
                'from qtforge',
                'import qtforge',
                'PluginInterface',
                'ServicePlugin',
                'PLUGIN_METADATA'
            ]
            
            for indicator in plugin_indicators:
                if indicator in content:
                    # Try to extract plugin ID
                    plugin_id = self._extract_plugin_id(file_path, content)
                    if plugin_id:
                        return (plugin_id, file_path)
            
            return None
            
        except Exception:
            return None
    
    def _extract_plugin_id(self, file_path: str, content: str) -> Optional[str]:
        """Extract plugin ID from file content."""
        import re
        import os
        
        # Look for PLUGIN_METADATA
        metadata_match = re.search(r'PLUGIN_METADATA\s*=\s*{[^}]*"id"\s*:\s*"([^"]+)"', content)
        if metadata_match:
            return metadata_match.group(1)
        
        # Look for plugin class with metadata
        class_matches = re.findall(r'class\s+(\w+).*?PluginInterface', content, re.DOTALL)
        if class_matches:
            # Use filename as fallback
            filename = os.path.splitext(os.path.basename(file_path))[0]
            return f"python.{filename}"
        
        return None
```

### Plugin Environment

```python
class PythonPluginEnvironment:
    """Manages Python plugin execution environment."""
    
    def __init__(self):
        self._isolated_environments: Dict[str, Dict[str, Any]] = {}
        self._shared_context = {}
    
    def create_isolated_environment(self, plugin_id: str) -> Dict[str, Any]:
        """
        Create an isolated environment for a plugin.
        
        Args:
            plugin_id: ID of the plugin
            
        Returns:
            Environment dictionary
        """
        environment = {
            'plugin_id': plugin_id,
            'globals': {},
            'locals': {},
            'imports': set(),
            'resources': {}
        }
        
        self._isolated_environments[plugin_id] = environment
        return environment
    
    def execute_in_environment(self, plugin_id: str, code: str) -> Any:
        """
        Execute code in plugin's isolated environment.
        
        Args:
            plugin_id: ID of the plugin
            code: Python code to execute
            
        Returns:
            Execution result
        """
        if plugin_id not in self._isolated_environments:
            self.create_isolated_environment(plugin_id)
        
        env = self._isolated_environments[plugin_id]
        
        try:
            # Prepare execution context
            exec_globals = {
                '__builtins__': __builtins__,
                'qtforge': self._get_qtforge_api(),
                **env['globals']
            }
            
            exec_locals = env['locals'].copy()
            
            # Execute code
            result = exec(code, exec_globals, exec_locals)
            
            # Update environment
            env['locals'].update(exec_locals)
            
            return result
            
        except Exception as e:
            print(f"Execution error in plugin {plugin_id}: {e}")
            raise
    
    def cleanup_environment(self, plugin_id: str) -> None:
        """
        Clean up plugin environment.
        
        Args:
            plugin_id: ID of the plugin
        """
        if plugin_id in self._isolated_environments:
            env = self._isolated_environments[plugin_id]
            
            # Cleanup resources
            for resource in env.get('resources', {}).values():
                if hasattr(resource, 'cleanup'):
                    try:
                        resource.cleanup()
                    except:
                        pass
            
            del self._isolated_environments[plugin_id]
    
    def _get_qtforge_api(self) -> Dict[str, Any]:
        """Get QtForge API for plugins."""
        return {
            'PluginInterface': PluginInterface,
            'ServicePlugin': ServicePlugin,
            'PluginMetadata': PluginMetadata,
            'PluginContext': PluginContext,
            # Add other API classes as needed
        }
```

## Usage Examples

### Loading a Python Plugin

```python
# Create plugin loader
loader = PythonPluginLoader()

# Load plugin
plugin = loader.load_plugin("/path/to/my_plugin.py")

if plugin:
    # Initialize plugin
    context = PluginContext()
    if plugin.initialize(context):
        print("Plugin loaded and initialized successfully")
    else:
        print("Plugin initialization failed")
else:
    print("Failed to load plugin")
```

### Plugin Discovery

```python
# Create discovery service
discovery = PythonPluginDiscovery()

# Add search paths
discovery.add_search_path("/usr/local/lib/qtforge/plugins")
discovery.add_search_path("./plugins")

# Discover plugins
plugins = discovery.discover_plugins()

print(f"Found {len(plugins)} plugins:")
for plugin_id, path in plugins.items():
    print(f"  {plugin_id}: {path}")
```

### Hot Reload

```python
# Reload a plugin
if loader.reload_plugin("com.example.myplugin"):
    print("Plugin reloaded successfully")
else:
    print("Failed to reload plugin")
```

## Error Handling

```python
class PluginLoadError(Exception):
    """Exception raised when plugin loading fails."""
    pass

class PluginExecutionError(Exception):
    """Exception raised when plugin execution fails."""
    pass

# Usage in loader
try:
    plugin = loader.load_plugin(plugin_path)
except PluginLoadError as e:
    print(f"Plugin load error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")
```

## Best Practices

### Plugin Development Guidelines

1. **Use proper metadata**: Always provide complete plugin metadata
2. **Handle errors gracefully**: Implement proper error handling
3. **Clean up resources**: Implement proper cleanup in shutdown method
4. **Follow naming conventions**: Use consistent naming for plugin files and classes
5. **Document your plugins**: Provide clear documentation

### Performance Considerations

1. **Lazy loading**: Load plugins only when needed
2. **Module caching**: Cache loaded modules appropriately
3. **Resource management**: Properly manage plugin resources
4. **Isolation**: Use isolated environments for security

## See Also

- [Python Plugin Interface](plugin-interface.md)
- [Python Plugin Manager](plugin-manager.md)
- [Plugin Development Guide](../../../user-guide/plugin-development.md)
- [Python Integration Tutorial](../../../tutorials/python-integration-tutorial.md)
