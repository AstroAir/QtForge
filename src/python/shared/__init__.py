#!/usr/bin/env python3
"""
QtForge Shared Python Utilities
===============================

This package provides shared utilities for plugin management that can be used
by both the pybind11 bindings and the python_bridge.py script.

The utilities are designed to be:
- Standalone (no compiled dependencies)
- Consistent across components
- JSON-serializable
- Backward compatible

Main Components:
- qtforge_plugin_utils: Core plugin introspection utilities
- module_wrapper: Module wrapping functionality
"""

# Import main utility functions
from .qtforge_plugin_utils import (
    # Core classes
    PluginMetadata,
    MethodSignature,
    PropertyInfo,
    
    # Main functions
    extract_plugin_metadata,
    analyze_method_signature,
    discover_plugin_methods,
    discover_plugin_properties,
    get_plugin_attributes,
    create_plugin_info_dict,
    
    # Backward compatibility functions
    extract_plugin_metadata_dict,
    discover_plugin_methods_dict,
    discover_plugin_properties_dict,
)

from .module_wrapper import (
    # Main class
    ModuleWrapper,
    
    # Utility functions
    create_module_wrapper,
    is_plugin_module,
    get_plugin_instance_from_module,
    load_plugin_from_module,
    
    # Backward compatibility
    create_plugin_wrapper,
)

# Version information
__version__ = "1.0.0"
__author__ = "QtForge Team"
__license__ = "MIT"

# Public API
__all__ = [
    # Core classes
    'PluginMetadata',
    'MethodSignature',
    'PropertyInfo',
    'ModuleWrapper',
    
    # Main functions
    'extract_plugin_metadata',
    'analyze_method_signature',
    'discover_plugin_methods',
    'discover_plugin_properties',
    'get_plugin_attributes',
    'create_plugin_info_dict',
    
    # Module wrapper functions
    'create_module_wrapper',
    'is_plugin_module',
    'get_plugin_instance_from_module',
    'load_plugin_from_module',
    
    # Backward compatibility functions
    'extract_plugin_metadata_dict',
    'discover_plugin_methods_dict',
    'discover_plugin_properties_dict',
    'create_plugin_wrapper',
]


def get_version() -> str:
    """Get the version of the shared utilities."""
    return __version__


def get_info() -> dict:
    """Get information about the shared utilities package."""
    return {
        "name": "QtForge Shared Python Utilities",
        "version": __version__,
        "author": __author__,
        "license": __license__,
        "description": "Shared utilities for QtForge Python components",
        "components": [
            "qtforge_plugin_utils",
            "module_wrapper"
        ]
    }


# Convenience function for quick plugin analysis
def analyze_plugin(plugin_instance, plugin_id: str = None) -> dict:
    """
    Convenience function to perform complete plugin analysis.
    
    Args:
        plugin_instance: The plugin instance to analyze
        plugin_id: Optional plugin identifier
        
    Returns:
        Dictionary with complete plugin information
    """
    return create_plugin_info_dict(plugin_instance, plugin_id)


# Convenience function for module-based plugins
def analyze_module_as_plugin(module, plugin_id: str = None) -> dict:
    """
    Convenience function to analyze a module as a plugin.
    
    Args:
        module: The Python module to analyze
        plugin_id: Optional plugin identifier
        
    Returns:
        Dictionary with complete plugin information
    """
    plugin_instance = load_plugin_from_module(module, plugin_id)
    return analyze_plugin(plugin_instance, plugin_id)
