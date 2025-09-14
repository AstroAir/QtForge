#!/usr/bin/env python3
"""
QtForge Python Bridge Script
============================

This script acts as a bridge between the QtForge C++ plugin system and Python plugins.
It provides a JSON-based communication interface for loading and executing Python plugins
in an isolated process environment.

Features:
- JSON-based inter-process communication
- Plugin loading and lifecycle management
- Method invocation with parameter passing
- Error handling and reporting
- Hot-reload support

Usage:
    python python_bridge.py

Communication Protocol:
    Input: JSON messages via stdin
    Output: JSON responses via stdout
    Format: {"id": int, "method": str, "params": dict}
"""

import sys
import json
import traceback
import importlib
import importlib.util
import os
from pathlib import Path
from typing import Any, Dict, Optional, List

# Try to import shared utilities (graceful fallback if not available)
try:
    # Add the shared utilities path
    shared_utils_path = Path(__file__).parent / "src" / "python" / "shared"
    if shared_utils_path.exists():
        sys.path.insert(0, str(shared_utils_path))

    from qtforge_plugin_utils import (
        extract_plugin_metadata_dict,
        discover_plugin_methods_dict,
        discover_plugin_properties_dict,
        get_plugin_attributes,
        create_plugin_info_dict
    )
    from module_wrapper import ModuleWrapper, load_plugin_from_module
    SHARED_UTILS_AVAILABLE = True
except ImportError:
    SHARED_UTILS_AVAILABLE = False

    # Fallback ModuleWrapper implementation when shared utilities aren't available
    class ModuleWrapper:
        """Fallback wrapper for module-level functions to act as a plugin."""

        def __init__(self, module):
            self.module = module
            self.name = getattr(module, '__name__', 'Unknown')
            self.version = getattr(module, '__version__', '1.0.0')

        def __getattr__(self, name):
            return getattr(self.module, name)


class PythonPluginBridge:
    """Main bridge class for handling Python plugin communication."""

    def __init__(self):
        self.plugins = {}
        self.current_plugin = None
        self.plugin_modules = {}
        self.initialized = False

    def handle_request(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Handle a request from the C++ side."""
        try:
            request_type = request.get("type", "")
            request_id = request.get("id", 0)

            if request_type == "initialize":
                return self._handle_initialize(request_id)
            elif request_type == "load_plugin":
                return self._handle_load_plugin(request)
            elif request_type == "call_method":
                return self._handle_call_method(request)
            elif request_type == "get_plugin_info":
                return self._handle_get_plugin_info(request)
            elif request_type == "unload_plugin":
                return self._handle_unload_plugin(request)
            elif request_type == "shutdown":
                return self._handle_shutdown(request_id)
            else:
                return {
                    "success": False,
                    "error": f"Unknown request type: {request_type}",
                    "id": request_id
                }
        except Exception as e:
            return {
                "success": False,
                "error": f"Request handling error: {str(e)}",
                "traceback": traceback.format_exc(),
                "id": request.get("id", 0)
            }

    def _handle_initialize(self, request_id: int) -> Dict[str, Any]:
        """Handle initialization request."""
        self.initialized = True
        return {
            "success": True,
            "message": "Python bridge initialized",
            "id": request_id
        }

    def _handle_load_plugin(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Handle plugin loading request."""
        plugin_path = request.get("plugin_path", "")
        plugin_class = request.get("plugin_class", "")
        plugin_id = request.get("plugin_id")

        result = self.load_plugin(plugin_path, plugin_id)
        result["id"] = request.get("id", 0)

        if "error" not in result:
            result["success"] = True
        else:
            result["success"] = False

        return result

    def _handle_call_method(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Handle method call request."""
        plugin_id = request.get("plugin_id", "")
        method_name = request.get("method_name", "")
        # Support both "params" and "parameters" for backward compatibility
        params = request.get("params", request.get("parameters", []))

        result = self.call_method(plugin_id, method_name, params)
        result["id"] = request.get("id", 0)

        if "error" not in result:
            result["success"] = True
        else:
            result["success"] = False

        return result

    def _handle_get_plugin_info(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Handle plugin info request."""
        plugin_id = request.get("plugin_id", "")

        if plugin_id not in self.plugins:
            return {
                "success": False,
                "error": f"Plugin not found: {plugin_id}",
                "id": request.get("id", 0)
            }

        plugin_instance = self.plugins[plugin_id]
        return {
            "success": True,
            "metadata": self._get_plugin_metadata(plugin_instance),
            "methods": self._get_plugin_methods(plugin_instance),
            "properties": self._get_plugin_properties(plugin_instance),
            "id": request.get("id", 0)
        }

    def _handle_unload_plugin(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Handle plugin unloading request."""
        plugin_id = request.get("plugin_id", "")

        if plugin_id in self.plugins:
            del self.plugins[plugin_id]
            if plugin_id in self.plugin_modules:
                del self.plugin_modules[plugin_id]

        return {
            "success": True,
            "message": f"Plugin {plugin_id} unloaded",
            "id": request.get("id", 0)
        }

    def _handle_shutdown(self, request_id: int) -> Dict[str, Any]:
        """Handle shutdown request."""
        self.plugins.clear()
        self.plugin_modules.clear()
        self.initialized = False

        return {
            "success": True,
            "message": "Python bridge shutdown",
            "id": request_id
        }

    def load_plugin(self, plugin_path: str, plugin_id: Optional[str] = None) -> Dict[str, Any]:
        """Load a Python plugin from the specified path."""
        try:
            if not os.path.exists(plugin_path):
                return {"error": f"Plugin file not found: {plugin_path}"}

            # Generate plugin ID if not provided
            if plugin_id is None:
                plugin_id = Path(plugin_path).stem

            # Load the module
            spec = importlib.util.spec_from_file_location(plugin_id, plugin_path)
            if spec is None or spec.loader is None:
                return {"error": f"Could not load plugin spec from {plugin_path}"}

            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            # Store the module
            self.plugin_modules[plugin_id] = module

            # Look for plugin class or create wrapper
            if SHARED_UTILS_AVAILABLE:
                # Use shared utilities for plugin loading
                plugin_instance = load_plugin_from_module(module, plugin_id)
            else:
                # Fallback to original implementation
                plugin_instance = None
                if hasattr(module, 'Plugin'):
                    plugin_instance = module.Plugin()
                elif hasattr(module, 'TestPlugin'):
                    plugin_instance = module.TestPlugin()
                elif hasattr(module, 'plugin'):
                    plugin_instance = module.plugin
                else:
                    # Create a wrapper for module-level functions
                    plugin_instance = ModuleWrapper(module)

            self.plugins[plugin_id] = plugin_instance
            self.current_plugin = plugin_id

            # Initialize if possible
            if hasattr(plugin_instance, 'initialize'):
                init_result = plugin_instance.initialize()
                if init_result is False:
                    return {"error": "Plugin initialization failed"}

            return {
                "success": True,
                "plugin_id": plugin_id,
                "metadata": self._get_plugin_metadata(plugin_instance),
                "methods": self._get_plugin_methods(plugin_instance),
                "properties": self._get_plugin_properties(plugin_instance)
            }

        except Exception as e:
            return {"error": f"Failed to load plugin: {str(e)}", "traceback": traceback.format_exc()}

    def call_method(self, plugin_id: str, method_name: str, params: Optional[List[Any]] = None) -> Dict[str, Any]:
        """Call a method on the specified plugin."""
        try:
            if plugin_id not in self.plugins:
                return {"error": f"Plugin not found: {plugin_id}"}

            plugin = self.plugins[plugin_id]
            if not hasattr(plugin, method_name):
                return {"error": f"Method not found: {method_name}"}

            method = getattr(plugin, method_name)
            if not callable(method):
                return {"error": f"Attribute is not callable: {method_name}"}

            # Call the method with parameters
            if params is None:
                params = []

            result = method(*params)
            return {"success": True, "result": result}

        except Exception as e:
            return {"error": f"Method call failed: {str(e)}", "traceback": traceback.format_exc()}

    def get_plugin_info(self, plugin_id: str) -> Dict[str, Any]:
        """Get information about the specified plugin."""
        try:
            if plugin_id not in self.plugins:
                return {"error": f"Plugin not found: {plugin_id}"}

            plugin = self.plugins[plugin_id]
            info = {
                "plugin_id": plugin_id,
                "methods": self._get_plugin_methods(plugin),
                "attributes": self._get_plugin_attributes(plugin)
            }

            # Get plugin metadata if available
            for attr in ['name', 'version', 'description', 'author']:
                if hasattr(plugin, attr):
                    info[attr] = getattr(plugin, attr)

            return {"success": True, "info": info}

        except Exception as e:
            return {"error": f"Failed to get plugin info: {str(e)}"}

    def unload_plugin(self, plugin_id: str) -> Dict[str, Any]:
        """Unload the specified plugin."""
        try:
            if plugin_id not in self.plugins:
                return {"error": f"Plugin not found: {plugin_id}"}

            plugin = self.plugins[plugin_id]

            # Call cleanup if available
            if hasattr(plugin, 'cleanup'):
                plugin.cleanup()

            # Remove from storage
            del self.plugins[plugin_id]
            if plugin_id in self.plugin_modules:
                del self.plugin_modules[plugin_id]

            if self.current_plugin == plugin_id:
                self.current_plugin = None

            return {"success": True}

        except Exception as e:
            return {"error": f"Failed to unload plugin: {str(e)}"}

    def _get_plugin_methods(self, plugin) -> List[Dict[str, Any]]:
        """Get list of callable methods from plugin."""
        if SHARED_UTILS_AVAILABLE:
            # Use shared utilities for method discovery
            return discover_plugin_methods_dict(plugin)
        else:
            # Fallback to original implementation
            return self._get_plugin_methods_original(plugin)

    def _get_plugin_methods_original(self, plugin) -> List[Dict[str, Any]]:
        """Original method discovery implementation (fallback)."""
        methods = []
        for attr_name in dir(plugin):
            if not attr_name.startswith('_'):
                attr = getattr(plugin, attr_name)
                if callable(attr):
                    # Get method signature information
                    import inspect
                    try:
                        sig = inspect.signature(attr)
                        params = []
                        for param_name, param in sig.parameters.items():
                            if param_name != 'self':  # Skip 'self' parameter
                                param_info = {
                                    "name": param_name,
                                    "type": str(param.annotation) if param.annotation != inspect.Parameter.empty else "Any",
                                    "default": str(param.default) if param.default != inspect.Parameter.empty else None
                                }
                                params.append(param_info)

                        method_info = {
                            "name": attr_name,
                            "parameters": params,
                            "return_type": str(sig.return_annotation) if sig.return_annotation != inspect.Signature.empty else "Any"
                        }
                        methods.append(method_info)
                    except (ValueError, TypeError):
                        # Fallback for methods that can't be inspected
                        methods.append({
                            "name": attr_name,
                            "parameters": [],
                            "return_type": "Any"
                        })
        return methods

    def _get_plugin_attributes(self, plugin) -> Dict[str, Any]:
        """Get list of non-callable attributes from plugin."""
        if SHARED_UTILS_AVAILABLE:
            # Use shared utilities for attribute discovery
            return get_plugin_attributes(plugin)
        else:
            # Fallback to original implementation
            return self._get_plugin_attributes_original(plugin)

    def _get_plugin_attributes_original(self, plugin) -> Dict[str, Any]:
        """Original attribute discovery implementation (fallback)."""
        attributes = {}
        for attr_name in dir(plugin):
            if not attr_name.startswith('_'):
                attr = getattr(plugin, attr_name)
                if not callable(attr):
                    try:
                        # Only include serializable attributes
                        json.dumps(attr)
                        attributes[attr_name] = attr
                    except (TypeError, ValueError):
                        attributes[attr_name] = str(attr)
        return attributes

    def _get_plugin_metadata(self, plugin_instance) -> Dict[str, Any]:
        """Extract metadata from a plugin instance."""
        if SHARED_UTILS_AVAILABLE:
            # Use shared utilities for metadata extraction
            return extract_plugin_metadata_dict(plugin_instance)
        else:
            # Fallback to original implementation
            return self._get_plugin_metadata_original(plugin_instance)

    def _get_plugin_metadata_original(self, plugin_instance) -> Dict[str, Any]:
        """Original metadata extraction implementation (fallback)."""
        metadata = {
            "name": getattr(plugin_instance, 'name', 'Unknown Plugin'),
            "version": getattr(plugin_instance, 'version', '1.0.0'),
            "description": getattr(plugin_instance, 'description', ''),
            "author": getattr(plugin_instance, 'author', ''),
            "category": getattr(plugin_instance, 'category', 'General')
        }

        # Try to get metadata from module if it's a ModuleWrapper (only if not already set)
        if hasattr(plugin_instance, 'module'):  # Check for module attribute instead of isinstance
            module = plugin_instance.module
            if metadata["name"] == 'Unknown Plugin':
                metadata["name"] = getattr(module, '__name__', 'Unknown Plugin')
            if metadata["version"] == '1.0.0':
                metadata["version"] = getattr(module, '__version__', '1.0.0')
            if not metadata["description"]:
                metadata["description"] = getattr(module, '__doc__', '')
            if not metadata["author"]:
                metadata["author"] = getattr(module, '__author__', '')

        return metadata

    def extract_plugin_metadata(self, plugin_instance) -> Dict[str, Any]:
        """Public method to extract plugin metadata."""
        return self._get_plugin_metadata(plugin_instance)

    def discover_plugin_methods(self, plugin_instance) -> List[Dict[str, Any]]:
        """Public method to discover plugin methods."""
        return self._get_plugin_methods(plugin_instance)

    def discover_plugin_properties(self, plugin_instance) -> List[Dict[str, Any]]:
        """Public method to discover plugin properties."""
        return self._get_plugin_properties(plugin_instance)

    def _get_plugin_properties(self, plugin_instance) -> List[Dict[str, Any]]:
        """Get properties from a plugin instance."""
        if SHARED_UTILS_AVAILABLE:
            # Use shared utilities for property discovery
            return discover_plugin_properties_dict(plugin_instance)
        else:
            # Fallback to original implementation
            return self._get_plugin_properties_original(plugin_instance)

    def _get_plugin_properties_original(self, plugin_instance) -> List[Dict[str, Any]]:
        """Original property discovery implementation (fallback)."""
        properties = []

        for attr_name in dir(plugin_instance):
            if not attr_name.startswith('_'):
                attr = getattr(plugin_instance, attr_name)
                if not callable(attr):
                    try:
                        # Only include serializable properties
                        json.dumps(attr)
                        properties.append({
                            "name": attr_name,
                            "type": type(attr).__name__,
                            "value": attr,
                            "readable": True,
                            "writable": True
                        })
                    except (TypeError, ValueError):
                        properties.append({
                            "name": attr_name,
                            "type": type(attr).__name__,
                            "value": str(attr),
                            "readable": True,
                            "writable": True
                        })

        return properties


# ModuleWrapper is now imported from shared utilities


def main():
    """Main communication loop."""
    bridge = PythonPluginBridge()

    # Send ready signal
    print(json.dumps({"ready": True}), flush=True)

    try:
        for line in sys.stdin:
            line = line.strip()
            if not line:
                continue

            try:
                request = json.loads(line)

                # Use the new handle_request method
                result = bridge.handle_request(request)

                # Send response
                print(json.dumps(result), flush=True)

            except json.JSONDecodeError as e:
                error_response = {
                    "error": f"Invalid JSON: {str(e)}",
                    "id": 0
                }
                print(json.dumps(error_response), flush=True)
            except Exception as e:
                error_response = {
                    "error": f"Unexpected error: {str(e)}",
                    "traceback": traceback.format_exc(),
                    "id": 0
                }
                print(json.dumps(error_response), flush=True)

    except KeyboardInterrupt:
        pass
    except Exception as e:
        error_response = {
            "error": f"Bridge error: {str(e)}",
            "traceback": traceback.format_exc()
        }
        print(json.dumps(error_response), flush=True)


if __name__ == '__main__':
    main()
