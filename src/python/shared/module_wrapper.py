#!/usr/bin/env python3
"""
Module Wrapper Utilities
========================

This module provides utilities for wrapping Python modules to act as plugins,
shared between the pybind11 bindings and python_bridge.py.
"""

from typing import Any, Optional
try:
    from .qtforge_plugin_utils import extract_plugin_metadata, PluginMetadata
except ImportError:
    # Fallback for direct execution
    from qtforge_plugin_utils import extract_plugin_metadata, PluginMetadata


class ModuleWrapper:
    """
    Wrapper for module-level functions to act as a plugin.

    This class allows Python modules that don't have a dedicated plugin class
    to be treated as plugins by providing a consistent interface.
    """

    def __init__(self, module: Any):
        """
        Initialize the module wrapper.

        Args:
            module: The Python module to wrap
        """
        self.module = module
        self.name = getattr(module, '__name__', 'Unknown Module')
        self.version = getattr(module, '__version__', '1.0.0')
        self.description = getattr(module, '__doc__', '') or ''
        self.author = getattr(module, '__author__', '')
        self.license = getattr(module, '__license__', '')
        self.category = getattr(module, '__category__', 'General')

    def __getattr__(self, name: str) -> Any:
        """
        Delegate attribute access to the wrapped module.

        Args:
            name: The attribute name to access

        Returns:
            The attribute from the wrapped module

        Raises:
            AttributeError: If the attribute doesn't exist in the module
        """
        return getattr(self.module, name)

    def __hasattr__(self, name: str) -> bool:
        """
        Check if the wrapped module has the specified attribute.

        Args:
            name: The attribute name to check

        Returns:
            True if the module has the attribute, False otherwise
        """
        return hasattr(self.module, name)

    def get_metadata(self) -> PluginMetadata:
        """
        Get plugin metadata for this module wrapper.

        Returns:
            PluginMetadata object with information extracted from the module
        """
        return extract_plugin_metadata(self, self.module)

    def list_functions(self) -> list:
        """
        List all callable functions in the wrapped module.

        Returns:
            List of function names that are callable
        """
        functions = []
        for attr_name in dir(self.module):
            if not attr_name.startswith('_'):
                attr = getattr(self.module, attr_name)
                if callable(attr):
                    functions.append(attr_name)
        return functions

    def list_attributes(self) -> list:
        """
        List all non-callable attributes in the wrapped module.

        Returns:
            List of attribute names that are not callable
        """
        attributes = []
        for attr_name in dir(self.module):
            if not attr_name.startswith('_'):
                attr = getattr(self.module, attr_name)
                if not callable(attr):
                    attributes.append(attr_name)
        return attributes

    def __repr__(self) -> str:
        """String representation of the module wrapper."""
        return f"<ModuleWrapper: {self.name} v{self.version}>"


def create_module_wrapper(module: Any) -> ModuleWrapper:
    """
    Factory function to create a module wrapper.

    Args:
        module: The Python module to wrap

    Returns:
        ModuleWrapper instance for the given module
    """
    return ModuleWrapper(module)


def is_plugin_module(module: Any) -> bool:
    """
    Check if a module appears to be a plugin module.

    This function checks for common plugin indicators like:
    - Plugin class
    - create_plugin function
    - Standard plugin attributes

    Args:
        module: The module to check

    Returns:
        True if the module appears to be a plugin, False otherwise
    """
    # Check for common plugin class names
    plugin_class_names = ['Plugin', 'TestPlugin', 'SamplePlugin']
    for class_name in plugin_class_names:
        if hasattr(module, class_name):
            return True

    # Check for plugin factory function
    if hasattr(module, 'create_plugin') and callable(getattr(module, 'create_plugin')):
        return True

    # Check for plugin instance
    if hasattr(module, 'plugin'):
        return True

    # Check for standard plugin metadata
    plugin_attributes = ['__plugin_name__', '__plugin_version__', '__plugin_author__']
    if any(hasattr(module, attr) for attr in plugin_attributes):
        return True

    return False


def get_plugin_instance_from_module(module: Any) -> Optional[Any]:
    """
    Extract a plugin instance from a module.

    This function tries various common patterns to find a plugin instance:
    1. Look for Plugin class and instantiate it
    2. Look for create_plugin function and call it
    3. Look for existing plugin instance
    4. Return None if no plugin found

    Args:
        module: The module to extract plugin from

    Returns:
        Plugin instance if found, None otherwise
    """
    # Try common plugin class names
    plugin_class_names = ['Plugin', 'TestPlugin', 'SamplePlugin']
    for class_name in plugin_class_names:
        if hasattr(module, class_name):
            plugin_class = getattr(module, class_name)
            if callable(plugin_class):
                try:
                    return plugin_class()
                except Exception:
                    # If instantiation fails, continue to next option
                    continue

    # Try plugin factory function
    if hasattr(module, 'create_plugin'):
        create_plugin = getattr(module, 'create_plugin')
        if callable(create_plugin):
            try:
                return create_plugin()
            except Exception:
                # If factory function fails, continue
                pass

    # Try existing plugin instance
    if hasattr(module, 'plugin'):
        plugin_instance = getattr(module, 'plugin')
        if plugin_instance is not None:
            return plugin_instance

    # No plugin instance found
    return None


def load_plugin_from_module(module: Any, plugin_id: Optional[str] = None) -> Any:
    """
    Load a plugin from a module, creating appropriate wrapper if needed.

    Args:
        module: The Python module to load plugin from
        plugin_id: Optional plugin identifier

    Returns:
        Plugin instance or ModuleWrapper
    """
    # First try to get a dedicated plugin instance
    plugin_instance = get_plugin_instance_from_module(module)

    if plugin_instance is not None:
        return plugin_instance

    # If no dedicated plugin found, wrap the module
    return create_module_wrapper(module)


# Backward compatibility
def create_plugin_wrapper(module: Any) -> ModuleWrapper:
    """Backward compatibility alias for create_module_wrapper."""
    return create_module_wrapper(module)
