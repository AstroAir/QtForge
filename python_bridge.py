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

            logger.info(f"Loaded Python plugin: {plugin_id} from {plugin_path}")

            return {
                "id": request_id,
                "success": True,
                "plugin_id": plugin_id,
                "message": f"Plugin loaded successfully"
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
