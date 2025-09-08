#!/usr/bin/env python3
"""
Comprehensive tests for the Python bridge script functionality
"""

import unittest
import json
import sys
import os
import tempfile
import shutil
from pathlib import Path

# Add the project root to Python path
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root))

from python_bridge import PythonPluginBridge


class TestPythonBridgeScript(unittest.TestCase):
    """Test cases for Python bridge script functionality"""

    def setUp(self):
        """Set up test environment"""
        self.bridge = PythonPluginBridge()
        self.temp_dir = tempfile.mkdtemp()
        self.test_plugin_path = None
        
        # Create a test plugin
        self.create_test_plugin()

    def tearDown(self):
        """Clean up test environment"""
        if self.temp_dir and os.path.exists(self.temp_dir):
            shutil.rmtree(self.temp_dir)

    def create_test_plugin(self):
        """Create a test plugin for testing"""
        plugin_content = '''
class TestPlugin:
    def __init__(self):
        self.name = "Test Plugin"
        self.version = "1.0.0"
        self.description = "A test plugin"
        self.author = "Test Suite"
        self.license = "MIT"
        self.counter = 0
        self.data = {}
        self.initialized = False

    def initialize(self):
        self.initialized = True
        return {"success": True, "message": "Initialized"}

    def shutdown(self):
        self.initialized = False
        return {"success": True, "message": "Shutdown"}

    def simple_method(self):
        return "simple_result"

    def method_with_params(self, param1, param2=None):
        return {"param1": param1, "param2": param2}

    def get_counter(self):
        return self.counter

    def set_counter(self, value):
        self.counter = int(value)

    def increment_counter(self, amount=1):
        self.counter += amount
        return self.counter

    def store_data(self, key, value):
        self.data[key] = value
        return {"stored": True}

    def get_data(self, key=None):
        if key is None:
            return self.data
        return self.data.get(key)

    def raise_error(self):
        raise ValueError("Test error")

    def handle_event(self, event_name, event_data):
        return {"handled": True, "event_name": event_name, "event_data": event_data}

def create_plugin():
    return TestPlugin()
'''
        
        self.test_plugin_path = os.path.join(self.temp_dir, "test_plugin.py")
        with open(self.test_plugin_path, 'w') as f:
            f.write(plugin_content)

    def test_bridge_initialization(self):
        """Test bridge initialization"""
        request = {
            "type": "initialize",
            "id": 1
        }
        
        response = self.bridge.handle_request(request)
        
        self.assertTrue(response["success"])
        self.assertEqual(response["id"], 1)

    def test_plugin_loading(self):
        """Test plugin loading functionality"""
        request = {
            "type": "load_plugin",
            "id": 1,
            "plugin_path": self.test_plugin_path,
            "plugin_class": "TestPlugin"
        }
        
        response = self.bridge.handle_request(request)
        
        self.assertTrue(response["success"])
        self.assertIn("plugin_id", response)
        self.assertIn("metadata", response)
        self.assertIn("methods", response)
        self.assertIn("properties", response)
        
        # Verify metadata
        metadata = response["metadata"]
        self.assertEqual(metadata["name"], "Test Plugin")
        self.assertEqual(metadata["version"], "1.0.0")
        
        # Verify methods were discovered
        methods = response["methods"]
        method_names = [m["name"] for m in methods]
        self.assertIn("simple_method", method_names)
        self.assertIn("method_with_params", method_names)
        
        return response["plugin_id"]

    def test_method_calling(self):
        """Test method calling functionality"""
        # First load a plugin
        plugin_id = self.test_plugin_loading()
        
        # Test simple method call
        request = {
            "type": "call_method",
            "id": 2,
            "plugin_id": plugin_id,
            "method_name": "simple_method",
            "parameters": []
        }
        
        response = self.bridge.handle_request(request)
        
        self.assertTrue(response["success"])
        self.assertEqual(response["result"], "simple_result")

    def test_method_with_parameters(self):
        """Test method calling with parameters"""
        plugin_id = self.test_plugin_loading()
        
        request = {
            "type": "call_method",
            "id": 3,
            "plugin_id": plugin_id,
            "method_name": "method_with_params",
            "parameters": ["param1_value", "param2_value"]
        }
        
        response = self.bridge.handle_request(request)
        
        self.assertTrue(response["success"])
        result = response["result"]
        self.assertEqual(result["param1"], "param1_value")
        self.assertEqual(result["param2"], "param2_value")

    def test_property_access(self):
        """Test property access functionality"""
        plugin_id = self.test_plugin_loading()
        
        # Test getting property
        get_request = {
            "type": "get_property",
            "id": 4,
            "plugin_id": plugin_id,
            "property_name": "counter"
        }
        
        response = self.bridge.handle_request(get_request)
        
        self.assertTrue(response["success"])
        self.assertEqual(response["value"], 0)
        
        # Test setting property
        set_request = {
            "type": "set_property",
            "id": 5,
            "plugin_id": plugin_id,
            "property_name": "counter",
            "value": 42
        }
        
        response = self.bridge.handle_request(set_request)
        self.assertTrue(response["success"])
        
        # Verify the change
        response = self.bridge.handle_request(get_request)
        self.assertTrue(response["success"])
        self.assertEqual(response["value"], 42)

    def test_event_handling(self):
        """Test event handling functionality"""
        plugin_id = self.test_plugin_loading()
        
        # Test event subscription
        sub_request = {
            "type": "subscribe_events",
            "id": 6,
            "plugin_id": plugin_id,
            "event_names": ["test_event"]
        }
        
        response = self.bridge.handle_request(sub_request)
        self.assertTrue(response["success"])
        
        # Test event emission
        emit_request = {
            "type": "emit_event",
            "id": 7,
            "plugin_id": plugin_id,
            "event_name": "test_event",
            "event_data": {"message": "test"}
        }
        
        response = self.bridge.handle_request(emit_request)
        self.assertTrue(response["success"])
        
        # Test event unsubscription
        unsub_request = {
            "type": "unsubscribe_events",
            "id": 8,
            "plugin_id": plugin_id,
            "event_names": ["test_event"]
        }
        
        response = self.bridge.handle_request(unsub_request)
        self.assertTrue(response["success"])

    def test_plugin_info_retrieval(self):
        """Test plugin information retrieval"""
        plugin_id = self.test_plugin_loading()
        
        request = {
            "type": "get_plugin_info",
            "id": 9,
            "plugin_id": plugin_id
        }
        
        response = self.bridge.handle_request(request)
        
        self.assertTrue(response["success"])
        self.assertIn("metadata", response)
        self.assertIn("methods", response)
        self.assertIn("properties", response)

    def test_error_handling(self):
        """Test error handling"""
        plugin_id = self.test_plugin_loading()
        
        # Test calling non-existent method
        request = {
            "type": "call_method",
            "id": 10,
            "plugin_id": plugin_id,
            "method_name": "non_existent_method",
            "parameters": []
        }
        
        response = self.bridge.handle_request(request)
        self.assertFalse(response["success"])
        self.assertIn("error", response)
        
        # Test method that raises an error
        request = {
            "type": "call_method",
            "id": 11,
            "plugin_id": plugin_id,
            "method_name": "raise_error",
            "parameters": []
        }
        
        response = self.bridge.handle_request(request)
        self.assertFalse(response["success"])
        self.assertIn("error", response)
        self.assertIn("traceback", response)

    def test_code_execution(self):
        """Test code execution functionality"""
        request = {
            "type": "execute_code",
            "id": 12,
            "code": "2 + 2",
            "context": {}
        }
        
        response = self.bridge.handle_request(request)
        
        self.assertTrue(response["success"])
        self.assertEqual(response["result"], 4)

    def test_metadata_extraction(self):
        """Test metadata extraction functionality"""
        plugin_id = self.test_plugin_loading()
        plugin = self.bridge.plugins[plugin_id]
        
        metadata = self.bridge.extract_plugin_metadata(plugin)
        
        self.assertEqual(metadata["name"], "Test Plugin")
        self.assertEqual(metadata["version"], "1.0.0")
        self.assertEqual(metadata["description"], "A test plugin")
        self.assertEqual(metadata["author"], "Test Suite")
        self.assertEqual(metadata["license"], "MIT")

    def test_method_discovery(self):
        """Test method discovery functionality"""
        plugin_id = self.test_plugin_loading()
        plugin = self.bridge.plugins[plugin_id]
        
        methods = self.bridge.discover_plugin_methods(plugin)
        
        method_names = [m["name"] for m in methods]
        self.assertIn("simple_method", method_names)
        self.assertIn("method_with_params", method_names)
        self.assertIn("get_counter", method_names)
        
        # Check that method info includes signatures
        method_with_params = next(m for m in methods if m["name"] == "method_with_params")
        self.assertIn("signature", method_with_params)
        self.assertIn("parameters", method_with_params)

    def test_property_discovery(self):
        """Test property discovery functionality"""
        plugin_id = self.test_plugin_loading()
        plugin = self.bridge.plugins[plugin_id]
        
        properties = self.bridge.discover_plugin_properties(plugin)
        
        prop_names = [p["name"] for p in properties]
        self.assertIn("name", prop_names)
        self.assertIn("version", prop_names)
        self.assertIn("counter", prop_names)


if __name__ == '__main__':
    unittest.main()
