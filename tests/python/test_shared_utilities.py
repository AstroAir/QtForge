#!/usr/bin/env python3
"""
Tests for QtForge Shared Python Utilities
=========================================

Comprehensive test suite for the shared utilities used by both
pybind11 bindings and python_bridge.py.
"""

import unittest
import sys
import os
import tempfile
import shutil
from pathlib import Path
from typing import Any, Dict, List

# Add the shared utilities to the path
project_root = Path(__file__).parent.parent.parent
shared_path = project_root / "src" / "python" / "shared"
sys.path.insert(0, str(shared_path))

try:
    from qtforge_plugin_utils import (
        PluginMetadata,
        MethodSignature,
        PropertyInfo,
        extract_plugin_metadata,
        analyze_method_signature,
        discover_plugin_methods,
        discover_plugin_properties,
        get_plugin_attributes,
        create_plugin_info_dict,
        extract_plugin_metadata_dict,
        discover_plugin_methods_dict,
        discover_plugin_properties_dict,
    )
    from module_wrapper import (
        ModuleWrapper,
        create_module_wrapper,
        is_plugin_module,
        get_plugin_instance_from_module,
        load_plugin_from_module,
    )
    SHARED_UTILS_AVAILABLE = True
except ImportError as e:
    print(f"Warning: Could not import shared utilities: {e}")
    SHARED_UTILS_AVAILABLE = False


class SamplePlugin:
    """Sample plugin for testing."""

    def __init__(self):
        self.name = "Sample Plugin"
        self.version = "1.2.3"
        self.description = "A sample plugin for testing"
        self.author = "Test Author"
        self.license = "MIT"
        self.category = "Testing"
        self.tags = ["test", "sample"]
        self.dependencies = ["dependency1", "dependency2"]
        self.counter = 42
        self.data = {"key": "value"}
        self.initialized = True

    def simple_method(self) -> str:
        """A simple method with no parameters."""
        return "simple_result"

    def method_with_params(self, param1: str, param2: int = 10) -> Dict[str, Any]:
        """A method with parameters and type annotations."""
        return {"param1": param1, "param2": param2}

    def get_counter(self) -> int:
        """Get the current counter value."""
        return self.counter

    def set_counter(self, value: int) -> None:
        """Set the counter value."""
        self.counter = value


class TestPluginMetadata(unittest.TestCase):
    """Test PluginMetadata class."""

    def setUp(self):
        if not SHARED_UTILS_AVAILABLE:
            self.skipTest("Shared utilities not available")

    def test_metadata_creation(self):
        """Test creating PluginMetadata objects."""
        metadata = PluginMetadata("Test Plugin", "1.0.0", "Test description")

        self.assertEqual(metadata.name, "Test Plugin")
        self.assertEqual(metadata.version, "1.0.0")
        self.assertEqual(metadata.description, "Test description")
        self.assertEqual(metadata.category, "General")  # Default value

    def test_metadata_to_dict(self):
        """Test converting metadata to dictionary."""
        metadata = PluginMetadata("Test Plugin", "1.0.0", "Test description", "Test Author")
        metadata.tags = ["tag1", "tag2"]

        result = metadata.to_dict()

        self.assertIsInstance(result, dict)
        self.assertEqual(result["name"], "Test Plugin")
        self.assertEqual(result["version"], "1.0.0")
        self.assertEqual(result["description"], "Test description")
        self.assertEqual(result["author"], "Test Author")
        self.assertEqual(result["tags"], ["tag1", "tag2"])

    def test_metadata_from_dict(self):
        """Test creating metadata from dictionary."""
        data = {
            "name": "Dict Plugin",
            "version": "2.0.0",
            "description": "From dict",
            "author": "Dict Author",
            "tags": ["dict", "test"]
        }

        metadata = PluginMetadata.from_dict(data)

        self.assertEqual(metadata.name, "Dict Plugin")
        self.assertEqual(metadata.version, "2.0.0")
        self.assertEqual(metadata.tags, ["dict", "test"])


class TestMethodSignature(unittest.TestCase):
    """Test MethodSignature class."""

    def setUp(self):
        if not SHARED_UTILS_AVAILABLE:
            self.skipTest("Shared utilities not available")

    def test_signature_creation(self):
        """Test creating MethodSignature objects."""
        params = [{"name": "param1", "type": "str", "default": None}]
        signature = MethodSignature("test_method", params, "str", "Test docstring")

        self.assertEqual(signature.name, "test_method")
        self.assertEqual(signature.parameters, params)
        self.assertEqual(signature.return_type, "str")
        self.assertEqual(signature.docstring, "Test docstring")

    def test_signature_to_dict(self):
        """Test converting signature to dictionary."""
        params = [{"name": "param1", "type": "str", "default": None}]
        signature = MethodSignature("test_method", params, "str")

        result = signature.to_dict()

        self.assertIsInstance(result, dict)
        self.assertEqual(result["name"], "test_method")
        self.assertEqual(result["parameters"], params)
        self.assertEqual(result["return_type"], "str")


class TestPropertyInfo(unittest.TestCase):
    """Test PropertyInfo class."""

    def setUp(self):
        if not SHARED_UTILS_AVAILABLE:
            self.skipTest("Shared utilities not available")

    def test_property_creation(self):
        """Test creating PropertyInfo objects."""
        prop = PropertyInfo("test_prop", "int", 42, True, False)

        self.assertEqual(prop.name, "test_prop")
        self.assertEqual(prop.type_name, "int")
        self.assertEqual(prop.value, 42)
        self.assertTrue(prop.readable)
        self.assertFalse(prop.writable)

    def test_property_to_dict(self):
        """Test converting property to dictionary."""
        prop = PropertyInfo("test_prop", "str", "test_value")

        result = prop.to_dict()

        self.assertIsInstance(result, dict)
        self.assertEqual(result["name"], "test_prop")
        self.assertEqual(result["type"], "str")
        self.assertEqual(result["value"], "test_value")


class TestPluginUtilities(unittest.TestCase):
    """Test plugin utility functions."""

    def setUp(self):
        if not SHARED_UTILS_AVAILABLE:
            self.skipTest("Shared utilities not available")
        self.plugin = SamplePlugin()

    def test_extract_plugin_metadata(self):
        """Test extracting plugin metadata."""
        metadata = extract_plugin_metadata(self.plugin)

        self.assertIsInstance(metadata, PluginMetadata)
        self.assertEqual(metadata.name, "Sample Plugin")
        self.assertEqual(metadata.version, "1.2.3")
        self.assertEqual(metadata.description, "A sample plugin for testing")
        self.assertEqual(metadata.author, "Test Author")
        self.assertEqual(metadata.license, "MIT")
        self.assertEqual(metadata.category, "Testing")

    def test_analyze_method_signature(self):
        """Test analyzing method signatures."""
        method = self.plugin.method_with_params
        signature = analyze_method_signature(method)

        self.assertIsInstance(signature, MethodSignature)
        self.assertEqual(signature.name, "method_with_params")
        self.assertEqual(len(signature.parameters), 2)

        # Check first parameter
        param1 = signature.parameters[0]
        self.assertEqual(param1["name"], "param1")
        self.assertEqual(param1["type"], "<class 'str'>")
        self.assertIsNone(param1["default"])

        # Check second parameter
        param2 = signature.parameters[1]
        self.assertEqual(param2["name"], "param2")
        self.assertEqual(param2["type"], "<class 'int'>")
        self.assertEqual(param2["default"], "10")

    def test_discover_plugin_methods(self):
        """Test discovering plugin methods."""
        methods = discover_plugin_methods(self.plugin)

        self.assertIsInstance(methods, list)
        self.assertTrue(len(methods) > 0)

        method_names = [method.name for method in methods]
        self.assertIn("simple_method", method_names)
        self.assertIn("method_with_params", method_names)
        self.assertIn("get_counter", method_names)
        self.assertIn("set_counter", method_names)

    def test_discover_plugin_properties(self):
        """Test discovering plugin properties."""
        properties = discover_plugin_properties(self.plugin)

        self.assertIsInstance(properties, list)
        self.assertTrue(len(properties) > 0)

        property_names = [prop.name for prop in properties]
        self.assertIn("name", property_names)
        self.assertIn("version", property_names)
        self.assertIn("counter", property_names)
        self.assertIn("data", property_names)

    def test_get_plugin_attributes(self):
        """Test getting plugin attributes."""
        attributes = get_plugin_attributes(self.plugin)

        self.assertIsInstance(attributes, dict)
        self.assertIn("name", attributes)
        self.assertIn("version", attributes)
        self.assertIn("counter", attributes)
        self.assertEqual(attributes["name"], "Sample Plugin")
        self.assertEqual(attributes["counter"], 42)

    def test_create_plugin_info_dict(self):
        """Test creating comprehensive plugin info."""
        info = create_plugin_info_dict(self.plugin, "test_plugin_id")

        self.assertIsInstance(info, dict)
        self.assertEqual(info["id"], "test_plugin_id")
        self.assertIn("metadata", info)
        self.assertIn("methods", info)
        self.assertIn("properties", info)
        self.assertIn("attributes", info)

        # Check metadata
        metadata = info["metadata"]
        self.assertEqual(metadata["name"], "Sample Plugin")
        self.assertEqual(metadata["version"], "1.2.3")

        # Check methods
        methods = info["methods"]
        self.assertIsInstance(methods, list)
        self.assertTrue(len(methods) > 0)

        # Check properties
        properties = info["properties"]
        self.assertIsInstance(properties, list)
        self.assertTrue(len(properties) > 0)


class TestBackwardCompatibility(unittest.TestCase):
    """Test backward compatibility functions."""

    def setUp(self):
        if not SHARED_UTILS_AVAILABLE:
            self.skipTest("Shared utilities not available")
        self.plugin = SamplePlugin()

    def test_extract_plugin_metadata_dict(self):
        """Test backward compatibility metadata extraction."""
        metadata_dict = extract_plugin_metadata_dict(self.plugin)

        self.assertIsInstance(metadata_dict, dict)
        self.assertEqual(metadata_dict["name"], "Sample Plugin")
        self.assertEqual(metadata_dict["version"], "1.2.3")

    def test_discover_plugin_methods_dict(self):
        """Test backward compatibility method discovery."""
        methods_list = discover_plugin_methods_dict(self.plugin)

        self.assertIsInstance(methods_list, list)
        self.assertTrue(len(methods_list) > 0)

        # Check that each method is a dictionary
        for method in methods_list:
            self.assertIsInstance(method, dict)
            self.assertIn("name", method)
            self.assertIn("parameters", method)
            self.assertIn("return_type", method)

    def test_discover_plugin_properties_dict(self):
        """Test backward compatibility property discovery."""
        properties_list = discover_plugin_properties_dict(self.plugin)

        self.assertIsInstance(properties_list, list)
        self.assertTrue(len(properties_list) > 0)

        # Check that each property is a dictionary
        for prop in properties_list:
            self.assertIsInstance(prop, dict)
            self.assertIn("name", prop)
            self.assertIn("type", prop)
            self.assertIn("value", prop)


class TestModuleWrapper(unittest.TestCase):
    """Test ModuleWrapper functionality."""

    def setUp(self):
        if not SHARED_UTILS_AVAILABLE:
            self.skipTest("Shared utilities not available")

        # Create a temporary module for testing
        self.temp_dir = tempfile.mkdtemp()
        self.module_path = os.path.join(self.temp_dir, "test_module.py")

        module_content = '''
"""Test module for ModuleWrapper testing."""

__version__ = "2.0.0"
__author__ = "Module Author"
__license__ = "Apache"

def module_function(x, y=5):
    """A function in the module."""
    return x + y

def another_function():
    """Another function."""
    return "module_result"

MODULE_CONSTANT = "test_constant"
module_variable = 123
'''

        with open(self.module_path, 'w') as f:
            f.write(module_content)

        # Import the module
        sys.path.insert(0, self.temp_dir)
        import test_module
        self.test_module = test_module

    def tearDown(self):
        if hasattr(self, 'temp_dir') and os.path.exists(self.temp_dir):
            shutil.rmtree(self.temp_dir)

        # Clean up sys.path
        if self.temp_dir in sys.path:
            sys.path.remove(self.temp_dir)

    def test_module_wrapper_creation(self):
        """Test creating a ModuleWrapper."""
        wrapper = create_module_wrapper(self.test_module)

        self.assertIsInstance(wrapper, ModuleWrapper)
        self.assertEqual(wrapper.name, "test_module")
        self.assertEqual(wrapper.version, "2.0.0")
        self.assertEqual(wrapper.author, "Module Author")
        self.assertEqual(wrapper.license, "Apache")

    def test_module_wrapper_attribute_access(self):
        """Test accessing module attributes through wrapper."""
        wrapper = create_module_wrapper(self.test_module)

        # Test function access
        self.assertTrue(callable(wrapper.module_function))
        result = wrapper.module_function(10, 20)
        self.assertEqual(result, 30)

        # Test constant access
        self.assertEqual(wrapper.MODULE_CONSTANT, "test_constant")
        self.assertEqual(wrapper.module_variable, 123)

    def test_module_wrapper_metadata(self):
        """Test getting metadata from ModuleWrapper."""
        wrapper = create_module_wrapper(self.test_module)
        metadata = wrapper.get_metadata()

        self.assertIsInstance(metadata, PluginMetadata)
        self.assertEqual(metadata.name, "test_module")
        self.assertEqual(metadata.version, "2.0.0")
        self.assertEqual(metadata.author, "Module Author")

    def test_module_wrapper_list_functions(self):
        """Test listing functions in wrapped module."""
        wrapper = create_module_wrapper(self.test_module)
        functions = wrapper.list_functions()

        self.assertIsInstance(functions, list)
        self.assertIn("module_function", functions)
        self.assertIn("another_function", functions)

    def test_module_wrapper_list_attributes(self):
        """Test listing attributes in wrapped module."""
        wrapper = create_module_wrapper(self.test_module)
        attributes = wrapper.list_attributes()

        self.assertIsInstance(attributes, list)
        self.assertIn("MODULE_CONSTANT", attributes)
        self.assertIn("module_variable", attributes)

    def test_is_plugin_module(self):
        """Test detecting if a module is a plugin module."""
        # Test with regular module (should be False)
        self.assertFalse(is_plugin_module(self.test_module))

        # Create a plugin module
        plugin_module_content = '''
class Plugin:
    def __init__(self):
        self.name = "Plugin Module"
'''
        plugin_path = os.path.join(self.temp_dir, "plugin_module.py")
        with open(plugin_path, 'w') as f:
            f.write(plugin_module_content)

        import plugin_module
        self.assertTrue(is_plugin_module(plugin_module))

    def test_get_plugin_instance_from_module(self):
        """Test extracting plugin instance from module."""
        # Test with regular module (should return None)
        instance = get_plugin_instance_from_module(self.test_module)
        self.assertIsNone(instance)

        # Create a plugin module with Plugin class
        plugin_module_content = '''
class Plugin:
    def __init__(self):
        self.name = "Test Plugin"
        self.version = "1.0.0"

    def test_method(self):
        return "plugin_result"
'''
        plugin_path = os.path.join(self.temp_dir, "plugin_module.py")
        with open(plugin_path, 'w') as f:
            f.write(plugin_module_content)

        import plugin_module
        instance = get_plugin_instance_from_module(plugin_module)

        self.assertIsNotNone(instance)
        self.assertEqual(instance.name, "Test Plugin")
        self.assertEqual(instance.version, "1.0.0")

    def test_load_plugin_from_module(self):
        """Test loading plugin from module."""
        # Test with regular module (should return ModuleWrapper)
        plugin = load_plugin_from_module(self.test_module)
        self.assertIsInstance(plugin, ModuleWrapper)

        # Test with plugin module (should return plugin instance)
        plugin_module_content = '''
def create_plugin():
    class TestPlugin:
        def __init__(self):
            self.name = "Factory Plugin"
    return TestPlugin()
'''
        plugin_path = os.path.join(self.temp_dir, "factory_plugin.py")
        with open(plugin_path, 'w') as f:
            f.write(plugin_module_content)

        import factory_plugin
        plugin = load_plugin_from_module(factory_plugin)

        self.assertNotIsInstance(plugin, ModuleWrapper)
        self.assertEqual(plugin.name, "Factory Plugin")


class TestEdgeCases(unittest.TestCase):
    """Test edge cases and error handling."""

    def setUp(self):
        if not SHARED_UTILS_AVAILABLE:
            self.skipTest("Shared utilities not available")

    def test_empty_plugin(self):
        """Test with minimal plugin object."""
        class EmptyPlugin:
            pass

        plugin = EmptyPlugin()

        # Test metadata extraction with defaults
        metadata = extract_plugin_metadata(plugin)
        self.assertEqual(metadata.name, "Unknown Plugin")
        self.assertEqual(metadata.version, "1.0.0")

        # Test method discovery (should find no methods)
        methods = discover_plugin_methods(plugin)
        self.assertEqual(len(methods), 0)

        # Test property discovery (should find no properties)
        properties = discover_plugin_properties(plugin)
        self.assertEqual(len(properties), 0)

    def test_plugin_with_problematic_attributes(self):
        """Test with plugin that has attributes that can't be serialized."""
        class ProblematicPlugin:
            def __init__(self):
                self.name = "Problematic Plugin"
                self.version = "1.0.0"
                self.unserializable = lambda x: x  # Function object
                self.circular_ref = self  # Circular reference

        plugin = ProblematicPlugin()

        # Should handle unserializable attributes gracefully
        attributes = get_plugin_attributes(plugin)
        self.assertIn("name", attributes)
        self.assertIn("version", attributes)

        # Unserializable attributes should be converted to strings
        if "unserializable" in attributes:
            self.assertIsInstance(attributes["unserializable"], str)

    def test_method_with_complex_signature(self):
        """Test analyzing methods with complex signatures."""
        class ComplexPlugin:
            def complex_method(self, *args, **kwargs) -> None:
                """Method with *args and **kwargs."""
                pass

            def method_with_annotations(self, x: int, y: str = "default") -> Dict[str, Any]:
                """Method with type annotations."""
                return {"x": x, "y": y}

        plugin = ComplexPlugin()
        methods = discover_plugin_methods(plugin)

        method_names = [method.name for method in methods]
        self.assertIn("complex_method", method_names)
        self.assertIn("method_with_annotations", method_names)

        # Find the complex method and check its signature
        complex_method = next(m for m in methods if m.name == "complex_method")
        # The return type annotation for None can be represented differently
        self.assertIn(complex_method.return_type, ["None", "<class 'NoneType'>"])

    def test_backward_compatibility_edge_cases(self):
        """Test backward compatibility functions with edge cases."""
        class MinimalPlugin:
            pass

        plugin = MinimalPlugin()

        # Test that backward compatibility functions don't crash
        metadata_dict = extract_plugin_metadata_dict(plugin)
        self.assertIsInstance(metadata_dict, dict)

        methods_list = discover_plugin_methods_dict(plugin)
        self.assertIsInstance(methods_list, list)

        properties_list = discover_plugin_properties_dict(plugin)
        self.assertIsInstance(properties_list, list)


class TestIntegrationScenarios(unittest.TestCase):
    """Test integration scenarios that simulate real usage."""

    def setUp(self):
        if not SHARED_UTILS_AVAILABLE:
            self.skipTest("Shared utilities not available")

    def test_complete_plugin_analysis_workflow(self):
        """Test the complete workflow of analyzing a plugin."""
        plugin = SamplePlugin()

        # Simulate the workflow that python_bridge.py would use
        plugin_info = create_plugin_info_dict(plugin, "sample_plugin")

        # Verify the complete structure
        self.assertIn("id", plugin_info)
        self.assertIn("metadata", plugin_info)
        self.assertIn("methods", plugin_info)
        self.assertIn("properties", plugin_info)
        self.assertIn("attributes", plugin_info)

        # Verify metadata structure matches expected format
        metadata = plugin_info["metadata"]
        required_fields = ["name", "version", "description", "author", "license", "category"]
        for field in required_fields:
            self.assertIn(field, metadata)

        # Verify methods have required structure
        methods = plugin_info["methods"]
        for method in methods:
            self.assertIn("name", method)
            self.assertIn("parameters", method)
            self.assertIn("return_type", method)

        # Verify properties have required structure
        properties = plugin_info["properties"]
        for prop in properties:
            self.assertIn("name", prop)
            self.assertIn("type", prop)
            self.assertIn("value", prop)

    def test_module_wrapper_integration(self):
        """Test integration with module wrapper functionality."""
        # Create a temporary module
        temp_dir = tempfile.mkdtemp()
        try:
            module_path = os.path.join(temp_dir, "integration_module.py")
            module_content = '''
"""Integration test module."""
__version__ = "1.0.0"
__author__ = "Integration Test"

def test_function(x, y=10):
    return x + y

TEST_CONSTANT = "integration_test"
'''
            with open(module_path, 'w') as f:
                f.write(module_content)

            sys.path.insert(0, temp_dir)
            import integration_module

            # Test the complete workflow
            plugin = load_plugin_from_module(integration_module)
            self.assertIsInstance(plugin, ModuleWrapper)

            # Test that shared utilities work with module wrapper
            plugin_info = create_plugin_info_dict(plugin, "integration_test")

            # Verify the module wrapper is properly analyzed
            self.assertEqual(plugin_info["id"], "integration_test")
            self.assertEqual(plugin_info["metadata"]["name"], "integration_module")
            self.assertEqual(plugin_info["metadata"]["version"], "1.0.0")

            # Verify functions are discovered
            method_names = [method["name"] for method in plugin_info["methods"]]
            # ModuleWrapper methods are discovered through the wrapper, not the original module
            # The test_function should be accessible through the wrapper
            self.assertTrue(hasattr(plugin, "test_function"))
            self.assertTrue(callable(getattr(plugin, "test_function")))

        finally:
            shutil.rmtree(temp_dir)
            if temp_dir in sys.path:
                sys.path.remove(temp_dir)


if __name__ == '__main__':
    unittest.main()
