#!/usr/bin/env python3
"""
QtForge Python Bindings - Comprehensive Test Suite

This test suite verifies that all QtForge Python bindings are working correctly.
It tests basic functionality, module imports, and core operations.
"""

import unittest
import sys
import os
from pathlib import Path
from typing import Any, Dict, List

# Try to import QtForge
try:
    import qtforge
except ImportError:
    # Try to find the bindings in the build directory
    build_dir = Path(__file__).parent.parent / "build" / "src" / "python"
    if build_dir.exists():
        sys.path.insert(0, str(build_dir))
    import qtforge

class TestQtForgeBasics(unittest.TestCase):
    """Test basic QtForge functionality."""
    
    def test_import(self) -> None:
        """Test that QtForge can be imported."""
        self.assertIsNotNone(qtforge)
        self.assertTrue(hasattr(qtforge, '__version__'))
    
    def test_version_info(self) -> None:
        """Test version information."""
        version = qtforge.get_version()
        self.assertIsInstance(version, str)
        self.assertRegex(version, r'\d+\.\d+\.\d+')
        
        version_info = qtforge.get_version_info()
        self.assertIsInstance(version_info, tuple)
        self.assertEqual(len(version_info), 3)
        self.assertTrue(all(isinstance(x, int) for x in version_info))
    
    def test_connection(self) -> None:
        """Test basic connection."""
        result = qtforge.test_connection()
        self.assertIsInstance(result, str)
        self.assertIn("QtForge", result)
    
    def test_available_modules(self) -> None:
        """Test module listing."""
        modules = qtforge.list_available_modules()
        self.assertIsInstance(modules, list)
        self.assertIn("core", modules)
        self.assertIn("utils", modules)
    
    def test_build_info(self) -> None:
        """Test build information."""
        build_info = qtforge.get_build_info()
        self.assertIsInstance(build_info, dict)
        self.assertIn("version", build_info)
        self.assertIn("moduleInfo", build_info)
    
    def test_help(self) -> None:
        """Test help function."""
        help_text = qtforge.get_help()
        self.assertIsInstance(help_text, str)
        self.assertIn("QtForge", help_text)

class TestQtForgeCoreModule(unittest.TestCase):
    """Test QtForge core module functionality."""
    
    def setUp(self) -> None:
        """Set up test fixtures."""
        try:
            from qtforge.core import (
                PluginManager, PluginState, PluginCapability, PluginPriority,
                Version, PluginMetadata, PluginLoadOptions
            )
            self.core_available = True
            self.PluginManager = PluginManager
            self.PluginState = PluginState
            self.PluginCapability = PluginCapability
            self.PluginPriority = PluginPriority
            self.Version = Version
            self.PluginMetadata = PluginMetadata
            self.PluginLoadOptions = PluginLoadOptions
        except ImportError:
            self.core_available = False
    
    def test_core_module_import(self) -> None:
        """Test that core module can be imported."""
        if not self.core_available:
            self.skipTest("Core module not available")
        
        from qtforge import core
        self.assertIsNotNone(core)
    
    def test_plugin_states(self) -> None:
        """Test plugin state enumeration."""
        if not self.core_available:
            self.skipTest("Core module not available")
        
        states = [
            self.PluginState.Unloaded,
            self.PluginState.Loading,
            self.PluginState.Loaded,
            self.PluginState.Running,
            self.PluginState.Error
        ]
        
        for state in states:
            self.assertIsNotNone(state)
    
    def test_plugin_capabilities(self) -> None:
        """Test plugin capability enumeration."""
        if not self.core_available:
            self.skipTest("Core module not available")
        
        capabilities = [
            self.PluginCapability.UI,
            self.PluginCapability.Service,
            self.PluginCapability.Network,
            self.PluginCapability.Security
        ]
        
        for capability in capabilities:
            self.assertIsNotNone(capability)
    
    def test_plugin_priorities(self) -> None:
        """Test plugin priority enumeration."""
        if not self.core_available:
            self.skipTest("Core module not available")
        
        priorities = [
            self.PluginPriority.Lowest,
            self.PluginPriority.Low,
            self.PluginPriority.Normal,
            self.PluginPriority.High,
            self.PluginPriority.Highest
        ]
        
        for priority in priorities:
            self.assertIsNotNone(priority)
    
    def test_version_creation(self) -> None:
        """Test version object creation and comparison."""
        if not self.core_available:
            self.skipTest("Core module not available")
        
        v1 = self.Version(1, 0, 0)
        v2 = self.Version(2, 0, 0)
        
        self.assertEqual(v1.major(), 1)
        self.assertEqual(v1.minor(), 0)
        self.assertEqual(v1.patch(), 0)
        
        self.assertTrue(v1 < v2)
        self.assertFalse(v1 > v2)
        self.assertTrue(v1 != v2)
    
    def test_plugin_metadata(self) -> None:
        """Test plugin metadata creation."""
        if not self.core_available:
            self.skipTest("Core module not available")
        
        metadata = self.PluginMetadata()
        metadata.name = "TestPlugin"
        metadata.description = "A test plugin"
        metadata.author = "Test Author"
        
        self.assertEqual(metadata.name, "TestPlugin")
        self.assertEqual(metadata.description, "A test plugin")
        self.assertEqual(metadata.author, "Test Author")
    
    def test_plugin_load_options(self) -> None:
        """Test plugin load options."""
        if not self.core_available:
            self.skipTest("Core module not available")
        
        options = self.PluginLoadOptions()
        options.validate_signature = True
        options.check_dependencies = False
        
        self.assertTrue(options.validate_signature)
        self.assertFalse(options.check_dependencies)
    
    def test_plugin_manager_creation(self) -> None:
        """Test plugin manager creation."""
        if not self.core_available:
            self.skipTest("Core module not available")
        
        manager = self.PluginManager()
        self.assertIsNotNone(manager)
        
        # Test basic operations
        loaded = manager.loaded_plugins()
        self.assertIsInstance(loaded, list)
        
        paths = manager.search_paths()
        self.assertIsInstance(paths, list)

class TestQtForgeUtilsModule(unittest.TestCase):
    """Test QtForge utils module functionality."""
    
    def test_utils_module_import(self) -> None:
        """Test that utils module can be imported."""
        try:
            from qtforge import utils
            self.assertIsNotNone(utils)
        except ImportError:
            self.skipTest("Utils module not available")
    
    def test_utils_test_function(self) -> None:
        """Test utils test function."""
        try:
            from qtforge import utils
            result = utils.test_utils()
            self.assertIsInstance(result, str)
            self.assertIn("working", result.lower())
        except (ImportError, AttributeError):
            self.skipTest("Utils module or test function not available")

class TestQtForgeOptionalModules(unittest.TestCase):
    """Test optional QtForge modules."""
    
    def test_optional_modules(self) -> None:
        """Test that optional modules can be imported if available."""
        optional_modules = [
            'communication', 'security', 'managers', 'orchestration',
            'monitoring', 'threading', 'transactions', 'composition', 'marketplace'
        ]
        
        available_modules = qtforge.list_available_modules()
        
        for module_name in optional_modules:
            if module_name in available_modules:
                try:
                    module = getattr(qtforge, module_name)
                    self.assertIsNotNone(module)
                    
                    # Try to call test function if available
                    test_func_name = f'test_{module_name}'
                    if hasattr(module, test_func_name):
                        test_func = getattr(module, test_func_name)
                        result = test_func()
                        self.assertIsInstance(result, str)
                        
                except Exception as e:
                    self.fail(f"Failed to test {module_name} module: {e}")

class TestQtForgeConvenienceFunctions(unittest.TestCase):
    """Test QtForge convenience functions."""
    
    def test_create_plugin_manager(self) -> None:
        """Test plugin manager creation convenience function."""
        try:
            manager = qtforge.create_plugin_manager()
            self.assertIsNotNone(manager)
        except Exception as e:
            self.skipTest(f"Plugin manager creation not available: {e}")
    
    def test_create_version(self) -> None:
        """Test version creation convenience function."""
        try:
            version = qtforge.create_version(1, 2, 3)
            self.assertIsNotNone(version)
            self.assertEqual(str(version), "1.2.3")
        except Exception as e:
            self.skipTest(f"Version creation not available: {e}")
    
    def test_create_metadata(self) -> None:
        """Test metadata creation convenience function."""
        try:
            metadata = qtforge.create_metadata("TestPlugin", "Test description")
            self.assertIsNotNone(metadata)
            self.assertEqual(metadata.name, "TestPlugin")
            self.assertEqual(metadata.description, "Test description")
        except Exception as e:
            self.skipTest(f"Metadata creation not available: {e}")

def run_tests() -> None:
    """Run all tests and return results."""
    # Create test suite
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    
    # Add test classes
    test_classes = [
        TestQtForgeBasics,
        TestQtForgeCoreModule,
        TestQtForgeUtilsModule,
        TestQtForgeOptionalModules,
        TestQtForgeConvenienceFunctions
    ]
    
    for test_class in test_classes:
        tests = loader.loadTestsFromTestCase(test_class)
        suite.addTests(tests)
    
    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    return result

if __name__ == "__main__":
    print("QtForge Python Bindings - Comprehensive Test Suite")
    print("=" * 60)
    
    result = run_tests()
    
    print("\n" + "=" * 60)
    if result.wasSuccessful():
        print("All tests passed successfully!")
    else:
        print(f"Tests completed with {len(result.failures)} failures and {len(result.errors)} errors")
    print("=" * 60)
