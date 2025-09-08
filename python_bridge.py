#!/usr/bin/env python3
"""
QtForge Python Plugin Bridge
A bridge script that allows QtForge to communicate with Python plugins.
"""

import json
import sys
import os
import importlib.util
import traceback
from typing import Dict, Any, Optional
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class PythonPluginBridge:
    """Bridge between QtForge and Python plugins"""

    def __init__(self):
        self.plugins: Dict[str, Any] = {}
        self.plugin_modules: Dict[str, Any] = {}
        self.running = True
        self.event_subscriptions: Dict[str, Dict[str, list]] = {}  # plugin_id -> {event_name: [callbacks]}

    def handle_request(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Handle a request from QtForge"""
        try:
            request_type = request.get("type")
            request_id = request.get("id", 0)

            if request_type == "initialize":
                return self.handle_initialize(request_id)
            elif request_type == "shutdown":
                return self.handle_shutdown(request_id)
            elif request_type == "execute_code":
                return self.handle_execute_code(request)
            elif request_type == "load_plugin":
                return self.handle_load_plugin(request)
            elif request_type == "call_method":
                return self.handle_call_method(request)
            elif request_type == "get_plugin_info":
                return self.handle_get_plugin_info(request)
            elif request_type == "get_property":
                return self.handle_get_property(request)
            elif request_type == "set_property":
                return self.handle_set_property(request)
            elif request_type == "subscribe_events":
                return self.handle_subscribe_events(request)
            elif request_type == "unsubscribe_events":
                return self.handle_unsubscribe_events(request)
            elif request_type == "emit_event":
                return self.handle_emit_event(request)
            else:
                return {
                    "id": request_id,
                    "success": False,
                    "error": f"Unknown request type: {request_type}"
                }

        except Exception as e:
            logger.error(f"Error handling request: {e}")
            return {
                "id": request.get("id", 0),
                "success": False,
                "error": str(e),
                "traceback": traceback.format_exc()
            }

    def handle_initialize(self, request_id: int) -> Dict[str, Any]:
        """Initialize the Python bridge"""
        logger.info("Initializing Python plugin bridge")
        return {
            "id": request_id,
            "success": True,
            "message": "Python bridge initialized"
        }

    def handle_shutdown(self, request_id: int) -> Dict[str, Any]:
        """Shutdown the Python bridge"""
        logger.info("Shutting down Python plugin bridge")
        self.running = False
        return {
            "id": request_id,
            "success": True,
            "message": "Python bridge shutdown"
        }

    def handle_execute_code(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Execute Python code"""
        request_id = request.get("id", 0)
        code = request.get("code", "")
        context = request.get("context", {})

        try:
            # Create a safe execution environment
            safe_globals = {
                "__builtins__": {
                    "print": print,
                    "len": len,
                    "str": str,
                    "int": int,
                    "float": float,
                    "bool": bool,
                    "list": list,
                    "dict": dict,
                    "tuple": tuple,
                    "set": set,
                    "range": range,
                    "enumerate": enumerate,
                    "zip": zip,
                    "map": map,
                    "filter": filter,
                    "sum": sum,
                    "min": min,
                    "max": max,
                    "abs": abs,
                    "round": round,
                }
            }
            safe_globals.update(context)

            # Execute the code
            result = eval(code, safe_globals)

            return {
                "id": request_id,
                "success": True,
                "result": result
            }

        except Exception as e:
            return {
                "id": request_id,
                "success": False,
                "error": str(e),
                "traceback": traceback.format_exc()
            }

    def handle_load_plugin(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Load a Python plugin"""
        request_id = request.get("id", 0)
        plugin_path = request.get("plugin_path", "")
        plugin_class = request.get("plugin_class", "")

        try:
            # Load the plugin module
            spec = importlib.util.spec_from_file_location("plugin_module", plugin_path)
            if spec is None or spec.loader is None:
                raise ImportError(f"Cannot load plugin from {plugin_path}")

            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            # Get the plugin class
            if hasattr(module, plugin_class):
                plugin_cls = getattr(module, plugin_class)
            elif hasattr(module, "create_plugin"):
                # Try the factory function
                plugin_cls = module.create_plugin
            else:
                raise AttributeError(f"Plugin class {plugin_class} not found in {plugin_path}")

            # Create plugin instance
            if callable(plugin_cls):
                plugin_instance = plugin_cls()
            else:
                plugin_instance = plugin_cls

            # Generate plugin ID
            plugin_id = f"python_{len(self.plugins)}"

            # Store plugin
            self.plugins[plugin_id] = plugin_instance
            self.plugin_modules[plugin_id] = module

            # Extract plugin metadata and methods
            metadata = self.extract_plugin_metadata(plugin_instance)
            methods = self.discover_plugin_methods(plugin_instance)
            properties = self.discover_plugin_properties(plugin_instance)

            logger.info(f"Loaded Python plugin: {plugin_id} from {plugin_path}")

            return {
                "id": request_id,
                "success": True,
                "plugin_id": plugin_id,
                "message": f"Plugin loaded successfully",
                "metadata": metadata,
                "methods": methods,
                "properties": properties
            }

        except Exception as e:
            return {
                "id": request_id,
                "success": False,
                "error": str(e),
                "traceback": traceback.format_exc()
            }

    def handle_call_method(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Call a method on a loaded plugin"""
        request_id = request.get("id", 0)
        plugin_id = request.get("plugin_id", "")
        method_name = request.get("method_name", "")
        parameters = request.get("parameters", [])

        try:
            if plugin_id not in self.plugins:
                raise ValueError(f"Plugin {plugin_id} not found")

            plugin = self.plugins[plugin_id]

            if not hasattr(plugin, method_name):
                raise AttributeError(f"Method {method_name} not found in plugin {plugin_id}")

            method = getattr(plugin, method_name)
            if not callable(method):
                raise TypeError(f"{method_name} is not callable")

            # Call the method
            if parameters:
                result = method(*parameters)
            else:
                result = method()

            # Convert result to JSON-serializable format
            if hasattr(result, 'to_dict'):
                result = result.to_dict()
            elif hasattr(result, '__dict__'):
                result = result.__dict__

            return {
                "id": request_id,
                "success": True,
                "result": result
            }

        except Exception as e:
            return {
                "id": request_id,
                "success": False,
                "error": str(e),
                "traceback": traceback.format_exc()
            }

    def handle_get_plugin_info(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Get plugin information including metadata, methods, and properties"""
        request_id = request.get("id", 0)
        plugin_id = request.get("plugin_id", "")

        try:
            if plugin_id not in self.plugins:
                raise ValueError(f"Plugin {plugin_id} not found")

            plugin = self.plugins[plugin_id]

            metadata = self.extract_plugin_metadata(plugin)
            methods = self.discover_plugin_methods(plugin)
            properties = self.discover_plugin_properties(plugin)

            return {
                "id": request_id,
                "success": True,
                "metadata": metadata,
                "methods": methods,
                "properties": properties
            }

        except Exception as e:
            return {
                "id": request_id,
                "success": False,
                "error": str(e),
                "traceback": traceback.format_exc()
            }

    def handle_get_property(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Get a property value from a plugin"""
        request_id = request.get("id", 0)
        plugin_id = request.get("plugin_id", "")
        property_name = request.get("property_name", "")

        try:
            if plugin_id not in self.plugins:
                raise ValueError(f"Plugin {plugin_id} not found")

            plugin = self.plugins[plugin_id]

            if not hasattr(plugin, property_name):
                raise AttributeError(f"Property {property_name} not found in plugin {plugin_id}")

            value = getattr(plugin, property_name)

            return {
                "id": request_id,
                "success": True,
                "value": value,
                "type": type(value).__name__
            }

        except Exception as e:
            return {
                "id": request_id,
                "success": False,
                "error": str(e),
                "traceback": traceback.format_exc()
            }

    def handle_set_property(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Set a property value in a plugin"""
        request_id = request.get("id", 0)
        plugin_id = request.get("plugin_id", "")
        property_name = request.get("property_name", "")
        value = request.get("value")

        try:
            if plugin_id not in self.plugins:
                raise ValueError(f"Plugin {plugin_id} not found")

            plugin = self.plugins[plugin_id]

            # Set the property
            setattr(plugin, property_name, value)

            return {
                "id": request_id,
                "success": True,
                "message": f"Property {property_name} set successfully"
            }

        except Exception as e:
            return {
                "id": request_id,
                "success": False,
                "error": str(e),
                "traceback": traceback.format_exc()
            }

    def handle_subscribe_events(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Subscribe to events from a plugin"""
        request_id = request.get("id", 0)
        plugin_id = request.get("plugin_id", "")
        event_names = request.get("event_names", [])

        try:
            if plugin_id not in self.plugins:
                raise ValueError(f"Plugin {plugin_id} not found")

            # Initialize event subscriptions for this plugin if needed
            if plugin_id not in self.event_subscriptions:
                self.event_subscriptions[plugin_id] = {}

            # Subscribe to each event
            for event_name in event_names:
                if event_name not in self.event_subscriptions[plugin_id]:
                    self.event_subscriptions[plugin_id][event_name] = []

                # Add a callback placeholder (actual callback will be managed by C++)
                self.event_subscriptions[plugin_id][event_name].append("cpp_callback")

            return {
                "id": request_id,
                "success": True,
                "message": f"Subscribed to {len(event_names)} events for plugin {plugin_id}"
            }

        except Exception as e:
            return {
                "id": request_id,
                "success": False,
                "error": str(e),
                "traceback": traceback.format_exc()
            }

    def handle_unsubscribe_events(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Unsubscribe from events from a plugin"""
        request_id = request.get("id", 0)
        plugin_id = request.get("plugin_id", "")
        event_names = request.get("event_names", [])

        try:
            if plugin_id not in self.plugins:
                raise ValueError(f"Plugin {plugin_id} not found")

            # Remove event subscriptions
            if plugin_id in self.event_subscriptions:
                for event_name in event_names:
                    if event_name in self.event_subscriptions[plugin_id]:
                        del self.event_subscriptions[plugin_id][event_name]

            return {
                "id": request_id,
                "success": True,
                "message": f"Unsubscribed from {len(event_names)} events for plugin {plugin_id}"
            }

        except Exception as e:
            return {
                "id": request_id,
                "success": False,
                "error": str(e),
                "traceback": traceback.format_exc()
            }

    def handle_emit_event(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Emit an event from a plugin"""
        request_id = request.get("id", 0)
        plugin_id = request.get("plugin_id", "")
        event_name = request.get("event_name", "")
        event_data = request.get("event_data", {})

        try:
            if plugin_id not in self.plugins:
                raise ValueError(f"Plugin {plugin_id} not found")

            plugin = self.plugins[plugin_id]

            # Try to call an event handler method on the plugin if it exists
            handler_method = f"on_{event_name}"
            if hasattr(plugin, handler_method):
                handler = getattr(plugin, handler_method)
                if callable(handler):
                    handler(event_data)

            # Also try a generic event handler
            if hasattr(plugin, "handle_event"):
                handler = getattr(plugin, "handle_event")
                if callable(handler):
                    handler(event_name, event_data)

            return {
                "id": request_id,
                "success": True,
                "message": f"Event {event_name} emitted for plugin {plugin_id}",
                "event_name": event_name,
                "event_data": event_data
            }

        except Exception as e:
            return {
                "id": request_id,
                "success": False,
                "error": str(e),
                "traceback": traceback.format_exc()
            }

    def extract_plugin_metadata(self, plugin_instance: Any) -> Dict[str, Any]:
        """Extract metadata from plugin instance"""
        metadata = {}

        # Try to get standard attributes
        for attr in ['name', 'version', 'description', 'author', 'license']:
            if hasattr(plugin_instance, attr):
                metadata[attr] = getattr(plugin_instance, attr)

        # Try to get metadata from class docstring
        if hasattr(plugin_instance, '__doc__') and plugin_instance.__doc__:
            metadata['docstring'] = plugin_instance.__doc__.strip()

        # Try to get metadata from class attributes
        if hasattr(plugin_instance, '__class__'):
            cls = plugin_instance.__class__
            if hasattr(cls, '__name__'):
                metadata['class_name'] = cls.__name__
            if hasattr(cls, '__module__'):
                metadata['module_name'] = cls.__module__

        return metadata

    def discover_plugin_methods(self, plugin_instance: Any) -> list:
        """Discover callable methods in plugin instance"""
        methods = []

        for attr_name in dir(plugin_instance):
            if not attr_name.startswith('_'):  # Skip private methods
                attr = getattr(plugin_instance, attr_name)
                if callable(attr):
                    method_info = {
                        'name': attr_name,
                        'callable': True
                    }

                    # Try to get method signature
                    try:
                        import inspect
                        sig = inspect.signature(attr)
                        method_info['signature'] = str(sig)
                        method_info['parameters'] = list(sig.parameters.keys())
                    except (ValueError, TypeError):
                        pass

                    # Try to get docstring
                    if hasattr(attr, '__doc__') and attr.__doc__:
                        method_info['docstring'] = attr.__doc__.strip()

                    methods.append(method_info)

        return methods

    def discover_plugin_properties(self, plugin_instance: Any) -> list:
        """Discover properties in plugin instance"""
        properties = []

        for attr_name in dir(plugin_instance):
            if not attr_name.startswith('_'):  # Skip private attributes
                attr = getattr(plugin_instance, attr_name)
                if not callable(attr):
                    prop_info = {
                        'name': attr_name,
                        'type': type(attr).__name__,
                        'value': str(attr) if not isinstance(attr, (dict, list)) else type(attr).__name__
                    }
                    properties.append(prop_info)

        return properties

    def run(self):
        """Main bridge loop"""
        logger.info("Python plugin bridge started")

        try:
            while self.running:
                # Read request from stdin
                line = sys.stdin.readline()
                if not line:
                    break

                line = line.strip()
                if not line:
                    continue

                try:
                    request = json.loads(line)
                    response = self.handle_request(request)

                    # Send response to stdout
                    response_json = json.dumps(response)
                    print(response_json, flush=True)

                except json.JSONDecodeError as e:
                    logger.error(f"Invalid JSON request: {line}")
                    error_response = {
                        "id": 0,
                        "success": False,
                        "error": f"Invalid JSON: {str(e)}"
                    }
                    print(json.dumps(error_response), flush=True)

        except KeyboardInterrupt:
            logger.info("Bridge interrupted by user")
        except Exception as e:
            logger.error(f"Bridge error: {e}")
        finally:
            logger.info("Python plugin bridge stopped")

def main():
    """Main entry point"""
    bridge = PythonPluginBridge()
    bridge.run()

if __name__ == "__main__":
    main()
