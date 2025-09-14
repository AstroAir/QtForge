#!/usr/bin/env python3
"""
QtForge Plugin Utilities - Shared Components
============================================

This module provides shared utilities for plugin metadata extraction, method discovery,
and property introspection that can be used by both the pybind11 bindings and the
python_bridge.py script.

Key Features:
- Plugin metadata extraction with fallback support
- Method signature analysis using Python's inspect module
- Property discovery with type information
- JSON-serializable data structures
- No external dependencies beyond Python standard library

Design Principles:
- Standalone operation (no compiled dependencies)
- Consistent data formats across components
- Comprehensive error handling
- Backward compatibility support
"""

import json
import inspect
import sys
from typing import Any, Dict, List, Optional, Union, Type
from pathlib import Path


class PluginMetadata:
    """Standard plugin metadata structure."""
    
    def __init__(self, name: str = "Unknown Plugin", version: str = "1.0.0", 
                 description: str = "", author: str = "", license: str = "",
                 category: str = "General"):
        self.name = name
        self.version = version
        self.description = description
        self.author = author
        self.license = license
        self.category = category
        self.tags = []
        self.dependencies = []
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization."""
        return {
            "name": self.name,
            "version": self.version,
            "description": self.description,
            "author": self.author,
            "license": self.license,
            "category": self.category,
            "tags": self.tags,
            "dependencies": self.dependencies
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'PluginMetadata':
        """Create from dictionary."""
        metadata = cls(
            name=data.get("name", "Unknown Plugin"),
            version=data.get("version", "1.0.0"),
            description=data.get("description", ""),
            author=data.get("author", ""),
            license=data.get("license", ""),
            category=data.get("category", "General")
        )
        metadata.tags = data.get("tags", [])
        metadata.dependencies = data.get("dependencies", [])
        return metadata


class MethodSignature:
    """Standard method signature representation."""
    
    def __init__(self, name: str, parameters: List[Dict[str, Any]] = None,
                 return_type: str = "Any", docstring: str = ""):
        self.name = name
        self.parameters = parameters or []
        self.return_type = return_type
        self.docstring = docstring
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization."""
        return {
            "name": self.name,
            "parameters": self.parameters,
            "return_type": self.return_type,
            "docstring": self.docstring
        }


class PropertyInfo:
    """Standard property information structure."""
    
    def __init__(self, name: str, type_name: str, value: Any = None,
                 readable: bool = True, writable: bool = True):
        self.name = name
        self.type_name = type_name
        self.value = value
        self.readable = readable
        self.writable = writable
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization."""
        return {
            "name": self.name,
            "type": self.type_name,
            "value": self.value,
            "readable": self.readable,
            "writable": self.writable
        }


def extract_plugin_metadata(plugin_instance: Any, fallback_module: Any = None) -> PluginMetadata:
    """
    Extract metadata from a plugin instance or module.
    
    Args:
        plugin_instance: The plugin instance to extract metadata from
        fallback_module: Optional module to use for fallback metadata
    
    Returns:
        PluginMetadata object with extracted information
    """
    # Extract basic metadata from plugin instance
    metadata = PluginMetadata(
        name=getattr(plugin_instance, 'name', 'Unknown Plugin'),
        version=getattr(plugin_instance, 'version', '1.0.0'),
        description=getattr(plugin_instance, 'description', ''),
        author=getattr(plugin_instance, 'author', ''),
        license=getattr(plugin_instance, 'license', ''),
        category=getattr(plugin_instance, 'category', 'General')
    )
    
    # Extract additional metadata
    metadata.tags = getattr(plugin_instance, 'tags', [])
    metadata.dependencies = getattr(plugin_instance, 'dependencies', [])
    
    # Try fallback to module-level attributes if available
    if fallback_module and hasattr(plugin_instance, 'module'):
        module = plugin_instance.module
        if metadata.name == 'Unknown Plugin':
            metadata.name = getattr(module, '__name__', 'Unknown Plugin')
        if metadata.version == '1.0.0':
            metadata.version = getattr(module, '__version__', '1.0.0')
        if not metadata.description:
            metadata.description = getattr(module, '__doc__', '') or ''
        if not metadata.author:
            metadata.author = getattr(module, '__author__', '')
        if not metadata.license:
            metadata.license = getattr(module, '__license__', '')
    
    return metadata


def analyze_method_signature(method: Any) -> MethodSignature:
    """
    Analyze a method's signature using Python's inspect module.
    
    Args:
        method: The method to analyze
    
    Returns:
        MethodSignature object with detailed signature information
    """
    method_name = getattr(method, '__name__', 'unknown_method')
    
    try:
        sig = inspect.signature(method)
        parameters = []
        
        for param_name, param in sig.parameters.items():
            if param_name != 'self':  # Skip 'self' parameter
                param_info = {
                    "name": param_name,
                    "type": str(param.annotation) if param.annotation != inspect.Parameter.empty else "Any",
                    "default": str(param.default) if param.default != inspect.Parameter.empty else None,
                    "kind": str(param.kind)
                }
                parameters.append(param_info)
        
        return_type = str(sig.return_annotation) if sig.return_annotation != inspect.Signature.empty else "Any"
        docstring = getattr(method, '__doc__', '') or ''
        
        return MethodSignature(method_name, parameters, return_type, docstring.strip())
        
    except (ValueError, TypeError):
        # Fallback for methods that can't be inspected
        return MethodSignature(method_name, [], "Any", "")


def discover_plugin_methods(plugin_instance: Any) -> List[MethodSignature]:
    """
    Discover all callable methods on a plugin instance.
    
    Args:
        plugin_instance: The plugin instance to introspect
    
    Returns:
        List of MethodSignature objects for all discovered methods
    """
    methods = []
    
    for attr_name in dir(plugin_instance):
        if not attr_name.startswith('_'):
            try:
                attr = getattr(plugin_instance, attr_name)
                if callable(attr):
                    method_sig = analyze_method_signature(attr)
                    methods.append(method_sig)
            except (AttributeError, TypeError):
                # Skip attributes that can't be accessed
                continue
    
    return methods


def discover_plugin_properties(plugin_instance: Any) -> List[PropertyInfo]:
    """
    Discover all non-callable properties on a plugin instance.
    
    Args:
        plugin_instance: The plugin instance to introspect
    
    Returns:
        List of PropertyInfo objects for all discovered properties
    """
    properties = []
    
    for attr_name in dir(plugin_instance):
        if not attr_name.startswith('_'):
            try:
                attr = getattr(plugin_instance, attr_name)
                if not callable(attr):
                    # Check if the value is JSON serializable
                    try:
                        json.dumps(attr)
                        value = attr
                    except (TypeError, ValueError):
                        value = str(attr)
                    
                    prop_info = PropertyInfo(
                        name=attr_name,
                        type_name=type(attr).__name__,
                        value=value,
                        readable=True,
                        writable=True  # Assume writable unless we can determine otherwise
                    )
                    properties.append(prop_info)
            except (AttributeError, TypeError):
                # Skip attributes that can't be accessed
                continue
    
    return properties


def get_plugin_attributes(plugin_instance: Any) -> Dict[str, Any]:
    """
    Get all non-callable attributes from a plugin instance as a dictionary.
    
    Args:
        plugin_instance: The plugin instance to introspect
    
    Returns:
        Dictionary of attribute names to values (JSON-serializable)
    """
    attributes = {}
    
    for attr_name in dir(plugin_instance):
        if not attr_name.startswith('_'):
            try:
                attr = getattr(plugin_instance, attr_name)
                if not callable(attr):
                    try:
                        # Only include serializable attributes
                        json.dumps(attr)
                        attributes[attr_name] = attr
                    except (TypeError, ValueError):
                        attributes[attr_name] = str(attr)
            except (AttributeError, TypeError):
                # Skip attributes that can't be accessed
                continue
    
    return attributes


def create_plugin_info_dict(plugin_instance: Any, plugin_id: str = None) -> Dict[str, Any]:
    """
    Create a comprehensive plugin information dictionary.
    
    Args:
        plugin_instance: The plugin instance to analyze
        plugin_id: Optional plugin identifier
    
    Returns:
        Dictionary containing all plugin information in a standardized format
    """
    metadata = extract_plugin_metadata(plugin_instance)
    methods = discover_plugin_methods(plugin_instance)
    properties = discover_plugin_properties(plugin_instance)
    
    return {
        "id": plugin_id or metadata.name,
        "metadata": metadata.to_dict(),
        "methods": [method.to_dict() for method in methods],
        "properties": [prop.to_dict() for prop in properties],
        "attributes": get_plugin_attributes(plugin_instance)
    }


# Backward compatibility aliases
def extract_plugin_metadata_dict(plugin_instance: Any, fallback_module: Any = None) -> Dict[str, Any]:
    """Backward compatibility wrapper that returns a dictionary."""
    return extract_plugin_metadata(plugin_instance, fallback_module).to_dict()


def discover_plugin_methods_dict(plugin_instance: Any) -> List[Dict[str, Any]]:
    """Backward compatibility wrapper that returns a list of dictionaries."""
    return [method.to_dict() for method in discover_plugin_methods(plugin_instance)]


def discover_plugin_properties_dict(plugin_instance: Any) -> List[Dict[str, Any]]:
    """Backward compatibility wrapper that returns a list of dictionaries."""
    return [prop.to_dict() for prop in discover_plugin_properties(plugin_instance)]
