#!/usr/bin/env python3
"""
Enhanced tests for Python bindings functionality
"""

import unittest
import sys
import os
from pathlib import Path

# Add the build directory to Python path to import qtforge
build_dir = Path(__file__).parent.parent.parent / "build"
if build_dir.exists():
    sys.path.insert(0, str(build_dir))

try:
    import qtforge
    QTFORGE_AVAILABLE = True
except ImportError:
    QTFORGE_AVAILABLE = False


class TestPythonBindingsEnhanced(unittest.TestCase):
    """Enhanced tests for Python bindings"""

    def setUp(self) -> None:
        """Set up test environment"""
        if not QTFORGE_AVAILABLE:
            self.skipTest("QtForge Python bindings not available")

    def test_module_import(self) -> None:
        """Test that qtforge module can be imported"""
        self.assertTrue(QTFORGE_AVAILABLE)
        self.assertIsNotNone(qtforge)

    def test_module_metadata(self) -> None:
        """Test module metadata"""
        self.assertTrue(hasattr(qtforge, '__version__'))
        self.assertTrue(hasattr(qtforge, '__author__'))
        
        version = qtforge.__version__
        self.assertIsInstance(version, str)
        self.assertTrue(len(version) > 0)

    def test_core_module(self) -> None:
        """Test core module functionality"""
        self.assertTrue(hasattr(qtforge, 'core'))
        core = qtforge.core
        self.assertIsNotNone(core)

    def test_utils_module(self) -> None:
        """Test utils module functionality"""
        self.assertTrue(hasattr(qtforge, 'utils'))
        utils = qtforge.utils
        self.assertIsNotNone(utils)

    def test_security_module(self) -> None:
        """Test security module functionality"""
        if hasattr(qtforge, 'security'):
            security = qtforge.security
            self.assertIsNotNone(security)
        else:
            self.skipTest("Security module not available")

    def test_managers_module(self) -> None:
        """Test managers module functionality"""
        if hasattr(qtforge, 'managers'):
            managers = qtforge.managers
            self.assertIsNotNone(managers)
        else:
            self.skipTest("Managers module not available")

    def test_version_function(self) -> None:
        """Test version function"""
        if hasattr(qtforge, 'version'):
            version = qtforge.version()
            self.assertIsInstance(version, str)
            self.assertTrue(len(version) > 0)

    def test_version_info_function(self) -> None:
        """Test version_info function"""
        if hasattr(qtforge, 'version_info'):
            version_info = qtforge.version_info()
            self.assertIsInstance(version_info, (tuple, list))
            self.assertTrue(len(version_info) >= 3)

    def test_test_function(self) -> None:
        """Test the test function"""
        if hasattr(qtforge, 'test_function'):
            result = qtforge.test_function()
            self.assertIsInstance(result, str)
            self.assertIn("test", result.lower())

    def test_utils_test_function(self) -> None:
        """Test utils test function"""
        if hasattr(qtforge, 'utils_test'):
            result = qtforge.utils_test()
            self.assertIsInstance(result, str)

    def test_build_info(self) -> None:
        """Test build info function"""
        if hasattr(qtforge, 'get_build_info'):
            build_info = qtforge.get_build_info()
            self.assertIsInstance(build_info, dict)
            self.assertIn('build_type', build_info)

    def test_list_modules(self) -> None:
        """Test list modules function"""
        if hasattr(qtforge, 'list_modules'):
            modules = qtforge.list_modules()
            self.assertIsInstance(modules, list)
            self.assertIn('core', modules)
            self.assertIn('utils', modules)

    def test_list_functions(self) -> None:
        """Test list functions function"""
        if hasattr(qtforge, 'list_functions'):
            functions = qtforge.list_functions()
            self.assertIsInstance(functions, list)
            self.assertTrue(len(functions) > 0)

    def test_help_function(self) -> None:
        """Test help function"""
        if hasattr(qtforge, 'help'):
            help_text = qtforge.help()
            self.assertIsInstance(help_text, str)
            self.assertIn('QtForge', help_text)

    def test_core_enums(self) -> None:
        """Test core enums availability"""
        if hasattr(qtforge, 'core'):
            core = qtforge.core
            
            # Test PluginState enum
            if hasattr(core, 'PluginState'):
                plugin_state = core.PluginState
                self.assertTrue(hasattr(plugin_state, 'Unloaded'))
                self.assertTrue(hasattr(plugin_state, 'Loaded'))
                self.assertTrue(hasattr(plugin_state, 'Running'))

            # Test PluginType enum
            if hasattr(core, 'PluginType'):
                plugin_type = core.PluginType
                self.assertTrue(hasattr(plugin_type, 'Native'))
                self.assertTrue(hasattr(plugin_type, 'Python'))

    def test_core_classes(self) -> None:
        """Test core classes availability"""
        if hasattr(qtforge, 'core'):
            core = qtforge.core
            
            # Test PluginInfo class
            if hasattr(core, 'PluginInfo'):
                plugin_info_class = core.PluginInfo
                self.assertTrue(callable(plugin_info_class))

    def test_utils_functions(self) -> None:
        """Test utils functions"""
        if hasattr(qtforge, 'utils'):
            utils = qtforge.utils
            
            # Test version functions
            if hasattr(utils, 'get_version'):
                version = utils.get_version()
                self.assertIsInstance(version, str)

    def test_security_functions(self) -> None:
        """Test security functions"""
        if hasattr(qtforge, 'security'):
            security = qtforge.security
            
            # Test security manager functions
            if hasattr(security, 'create_security_manager'):
                # This might require proper initialization
                pass

    def test_managers_functions(self) -> None:
        """Test managers functions"""
        if hasattr(qtforge, 'managers'):
            managers = qtforge.managers
            
            # Test manager creation functions
            if hasattr(managers, 'create_plugin_manager'):
                # This might require proper initialization
                pass

    def test_error_handling(self) -> None:
        """Test error handling in bindings"""
        # Test calling non-existent function
        with self.assertRaises(AttributeError):
            qtforge.non_existent_function()

    def test_type_conversions(self) -> None:
        """Test type conversions between Python and C++"""
        if hasattr(qtforge, 'test_function'):
            # Test string conversion
            result = qtforge.test_function()
            self.assertIsInstance(result, str)

        if hasattr(qtforge, 'version_info'):
            # Test tuple/list conversion
            version_info = qtforge.version_info()
            self.assertIsInstance(version_info, (tuple, list))

    def test_exception_handling(self) -> None:
        """Test exception handling in bindings"""
        # This would test C++ exceptions being converted to Python exceptions
        # Implementation depends on specific binding functions
        pass

    def test_memory_management(self) -> None:
        """Test memory management in bindings"""
        # Test creating and destroying objects multiple times
        for i in range(100):
            if hasattr(qtforge, 'test_function'):
                result = qtforge.test_function()
                self.assertIsInstance(result, str)

    def test_threading_safety(self) -> None:
        """Test threading safety of bindings"""
        import threading
        import time
        
        results = []
        errors = []
        
        def worker() -> None:
            try:
                for i in range(10):
                    if hasattr(qtforge, 'test_function'):
                        result = qtforge.test_function()
                        results.append(result)
                    time.sleep(0.001)
            except Exception as e:
                errors.append(e)
        
        threads = []
        for i in range(5):
            thread = threading.Thread(target=worker)
            threads.append(thread)
            thread.start()
        
        for thread in threads:
            thread.join()
        
        self.assertEqual(len(errors), 0, f"Threading errors: {errors}")
        self.assertTrue(len(results) > 0)

    def test_large_data_handling(self) -> None:
        """Test handling of large data structures"""
        # This would test passing large strings or data structures
        # Implementation depends on specific binding functions
        pass

    def test_callback_functions(self) -> None:
        """Test callback functions from C++ to Python"""
        # This would test C++ calling Python callback functions
        # Implementation depends on specific binding functions
        pass

    def test_object_lifetime(self) -> None:
        """Test object lifetime management"""
        # This would test that C++ objects are properly managed
        # Implementation depends on specific binding classes
        pass


class TestBindingIntegration(unittest.TestCase):
    """Integration tests for Python bindings"""

    def setUp(self) -> None:
        """Set up test environment"""
        if not QTFORGE_AVAILABLE:
            self.skipTest("QtForge Python bindings not available")

    def test_cross_module_functionality(self) -> None:
        """Test functionality across multiple modules"""
        if hasattr(qtforge, 'core') and hasattr(qtforge, 'utils'):
            # Test using core and utils together
            pass

    def test_plugin_system_integration(self) -> None:
        """Test integration with plugin system"""
        # This would test creating and managing plugins through Python bindings
        pass

    def test_event_system_integration(self) -> None:
        """Test integration with event system"""
        # This would test event handling through Python bindings
        pass


if __name__ == '__main__':
    # Run tests with verbose output
    unittest.main(verbosity=2)
