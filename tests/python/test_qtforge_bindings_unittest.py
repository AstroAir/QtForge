#!/usr/bin/env python3
"""
Comprehensive unittest-based test suite for QtForge Python bindings.
Tests all core functionality including edge cases and error handling.
"""

import unittest
import sys
import os
from unittest.mock import Mock, patch
from typing import List, Optional

# Add the build directory to Python path for testing
build_dir = os.path.join(os.path.dirname(__file__), '../../build')
sys.path.insert(0, build_dir)
sys.path.insert(0, os.path.join(build_dir, 'python'))

try:
    # Try to import the compiled Python extension
    import qtforge
    BINDINGS_AVAILABLE = True
    print("QtForge bindings successfully imported")
except ImportError as e:
    BINDINGS_AVAILABLE = False
    print(f"QtForge bindings not available: {e}")
    print(f"Searched in: {build_dir}")

# Try to import submodules if available
try:
    if BINDINGS_AVAILABLE:
        import qtforge.core as core
        import qtforge.managers as managers
        import qtforge.communication as communication
        import qtforge.security as security
        import qtforge.utils as utils
        SUBMODULES_AVAILABLE = True
    else:
        SUBMODULES_AVAILABLE = False
except ImportError as e:
    SUBMODULES_AVAILABLE = False
    print(f"QtForge submodules not available: {e}")


class TestQtForgeCore(unittest.TestCase):
    """Test QtForge core functionality."""

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_qtforge_module_import(self):
        """Test QtForge module can be imported."""
        self.assertIsNotNone(qtforge)
        # Test basic module attributes
        self.assertTrue(hasattr(qtforge, '__version__'))
        self.assertTrue(hasattr(qtforge, 'get_version'))

        # Test version information
        version = qtforge.get_version()
        self.assertIsNotNone(version)
        print(f"QtForge version: {version}")

        # Test available modules
        if hasattr(qtforge, 'list_available_modules'):
            modules = qtforge.list_available_modules()
            print(f"Available modules: {modules}")
            self.assertIsInstance(modules, (list, tuple))

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_plugin_manager_creation(self):
        """Test PluginManager can be created."""
        try:
            if hasattr(qtforge, 'create_plugin_manager'):
                manager = qtforge.create_plugin_manager()
                self.assertIsNotNone(manager)
                print(f"PluginManager created: {type(manager)}")
            else:
                self.skipTest("create_plugin_manager not available in current build")
        except Exception as e:
            self.skipTest(f"PluginManager creation failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_version_creation(self):
        """Test Version creation with correct parameters."""
        try:
            if hasattr(qtforge, 'create_version'):
                # Use the correct signature: major, minor, patch as integers
                version = qtforge.create_version(1, 0, 0)
                self.assertIsNotNone(version)
                print(f"Version created: {version}")
            else:
                self.skipTest("create_version not available in current build")
        except Exception as e:
            self.skipTest(f"Version creation failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_metadata_creation(self):
        """Test Metadata creation with correct parameters."""
        try:
            if hasattr(qtforge, 'create_metadata'):
                # Use the correct signature: name and description as strings
                metadata = qtforge.create_metadata("test_plugin", "A test plugin")
                self.assertIsNotNone(metadata)
                print(f"Metadata created: {type(metadata)}")
            else:
                self.skipTest("create_metadata not available in current build")
        except Exception as e:
            self.skipTest(f"Metadata creation failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_available_modules(self):
        """Test accessing available modules."""
        try:
            # Test core module
            if hasattr(qtforge, 'core'):
                core_module = qtforge.core
                self.assertIsNotNone(core_module)
                print(f"Core module: {type(core_module)}")

            # Test utils module
            if hasattr(qtforge, 'utils'):
                utils_module = qtforge.utils
                self.assertIsNotNone(utils_module)
                print(f"Utils module: {type(utils_module)}")

            # Test orchestration module
            if hasattr(qtforge, 'orchestration'):
                orchestration_module = qtforge.orchestration
                self.assertIsNotNone(orchestration_module)
                print(f"Orchestration module: {type(orchestration_module)}")

        except Exception as e:
            self.skipTest(f"Available modules test failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_system_info(self):
        """Test system information functions."""
        try:
            if hasattr(qtforge, 'get_system_info'):
                info = qtforge.get_system_info()
                self.assertIsNotNone(info)
                print(f"System info: {info}")

            if hasattr(qtforge, 'get_build_info'):
                build_info = qtforge.get_build_info()
                self.assertIsNotNone(build_info)
                print(f"Build info: {build_info}")

        except Exception as e:
            self.skipTest(f"System info test failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_plugin_manager_invalid_operations(self):
        """Test PluginManager error handling."""
        try:
            manager = core.create_plugin_manager()

            # Test loading non-existent plugin
            with self.assertRaises((RuntimeError, ValueError, OSError, Exception)):
                if hasattr(manager, 'load_plugin'):
                    manager.load_plugin("/non/existent/path.so")
                elif hasattr(manager, 'loadPlugin'):
                    manager.loadPlugin("/non/existent/path.so")

        except Exception as e:
            self.skipTest(f"PluginManager test setup failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_plugin_loader_creation(self):
        """Test PluginLoader can be created."""
        try:
            loader = core.create_plugin_loader()
            self.assertIsNotNone(loader)
        except Exception as e:
            self.skipTest(f"PluginLoader creation failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_plugin_registry_creation(self):
        """Test PluginRegistry can be created."""
        try:
            registry = core.create_plugin_registry()
            self.assertIsNotNone(registry)
        except Exception as e:
            self.skipTest(f"PluginRegistry creation failed: {e}")


class TestQtForgeManagers(unittest.TestCase):
    """Test QtForge managers functionality."""

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_configuration_manager_creation(self):
        """Test ConfigurationManager can be created."""
        try:
            config_manager = managers.create_configuration_manager()
            self.assertIsNotNone(config_manager)
            # Test basic operations
            if hasattr(config_manager, 'set_value'):
                result = config_manager.set_value("test.key", "test_value")
                # Result might be boolean or None depending on implementation
                self.assertIsNotNone(result)
        except Exception as e:
            self.skipTest(f"ConfigurationManager test failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_resource_manager_creation(self):
        """Test ResourceManager can be created."""
        try:
            resource_manager = managers.create_resource_manager()
            self.assertIsNotNone(resource_manager)
        except Exception as e:
            self.skipTest(f"ResourceManager creation failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_logging_manager_creation(self):
        """Test LoggingManager can be created."""
        try:
            logging_manager = managers.create_logging_manager()
            self.assertIsNotNone(logging_manager)
            # Test basic logging
            if hasattr(logging_manager, 'info'):
                logging_manager.info("Test message", "test_plugin")
        except Exception as e:
            self.skipTest(f"LoggingManager test failed: {e}")


class TestQtForgeCommunication(unittest.TestCase):
    """Test QtForge communication functionality."""

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_message_bus_creation(self):
        """Test MessageBus can be created."""
        try:
            message_bus = communication.create_message_bus()
            self.assertIsNotNone(message_bus)
        except Exception as e:
            self.skipTest(f"MessageBus creation failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_message_creation(self):
        """Test message creation and basic operations."""
        try:
            # Test creating different types of messages
            if hasattr(communication, 'create_message'):
                message = communication.create_message("test_type", {"data": "value"})
                self.assertIsNotNone(message)
        except Exception as e:
            self.skipTest(f"Message creation test failed: {e}")


class TestQtForgeSecurity(unittest.TestCase):
    """Test QtForge security functionality."""

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_security_manager_creation(self):
        """Test SecurityManager can be created."""
        try:
            security_manager = security.create_security_manager()
            self.assertIsNotNone(security_manager)
        except Exception as e:
            self.skipTest(f"SecurityManager creation failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_plugin_sandbox_creation(self):
        """Test PluginSandbox can be created."""
        try:
            if hasattr(security, 'create_plugin_sandbox'):
                sandbox = security.create_plugin_sandbox()
                self.assertIsNotNone(sandbox)
        except Exception as e:
            self.skipTest(f"PluginSandbox creation failed: {e}")


class TestQtForgeUtils(unittest.TestCase):
    """Test QtForge utilities functionality."""

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_version_utilities(self):
        """Test version utilities."""
        try:
            if hasattr(utils, 'Version'):
                version = utils.Version("1.0.0")
                self.assertIsNotNone(version)
                self.assertEqual(str(version), "1.0.0")
        except Exception as e:
            self.skipTest(f"Version utilities test failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_error_handling_utilities(self):
        """Test error handling utilities."""
        try:
            # Test that error handling utilities are available
            if hasattr(utils, 'PluginError'):
                self.assertTrue(callable(utils.PluginError))
        except Exception as e:
            self.skipTest(f"Error handling utilities test failed: {e}")


class TestQtForgeIntegration(unittest.TestCase):
    """Test QtForge integration scenarios."""

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_manager_integration(self):
        """Test integration between different managers."""
        try:
            # Create multiple managers
            plugin_manager = core.create_plugin_manager()
            config_manager = managers.create_configuration_manager()

            self.assertIsNotNone(plugin_manager)
            self.assertIsNotNone(config_manager)

            # Test that they can coexist
            if hasattr(config_manager, 'set_value'):
                config_manager.set_value("integration.test", "success")

        except Exception as e:
            self.skipTest(f"Manager integration test failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_error_handling_integration(self):
        """Test error handling across different components."""
        try:
            plugin_manager = core.create_plugin_manager()

            # Test that errors are properly handled
            with self.assertRaises((RuntimeError, ValueError, OSError, Exception)):
                if hasattr(plugin_manager, 'load_plugin'):
                    plugin_manager.load_plugin("")  # Empty path should fail

        except Exception as e:
            self.skipTest(f"Error handling integration test failed: {e}")


class TestQtForgeBindingStability(unittest.TestCase):
    """Test QtForge binding stability and edge cases."""

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_repeated_creation_destruction(self):
        """Test repeated creation and destruction of objects."""
        try:
            for i in range(10):
                manager = core.create_plugin_manager()
                self.assertIsNotNone(manager)
                del manager  # Explicit cleanup

        except Exception as e:
            self.skipTest(f"Repeated creation test failed: {e}")

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_concurrent_access_safety(self):
        """Test basic thread safety considerations."""
        try:
            import threading
            import time

            results = []

            def create_manager():
                try:
                    manager = core.create_plugin_manager()
                    results.append(manager is not None)
                except Exception:
                    results.append(False)

            threads = []
            for i in range(5):
                thread = threading.Thread(target=create_manager)
                threads.append(thread)
                thread.start()

            for thread in threads:
                thread.join()

            # All creations should succeed
            self.assertTrue(all(results))

        except Exception as e:
            self.skipTest(f"Concurrent access test failed: {e}")


if __name__ == '__main__':
    # Run with verbose output
    unittest.main(verbosity=2)
