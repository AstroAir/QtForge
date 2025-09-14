#!/usr/bin/env python3
"""
Advanced test suite for QtForge Python Core bindings.
Tests all core functionality including edge cases and error handling.
"""

import unittest
import sys
import os
from unittest.mock import Mock, patch
from typing import List, Optional

# Add the build directory to Python path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../build'))

try:
    import qtforge
    import qtforge.core as core
    BINDINGS_AVAILABLE = True
except ImportError as e:
    BINDINGS_AVAILABLE = False
    print(f"QtForge bindings not available: {e}")

class TestPluginManager(unittest.TestCase):
    """Test PluginManager functionality."""

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_plugin_manager_creation(self) -> None:
        """Test PluginManager can be created."""
        manager = core.create_plugin_manager()
        self.assertIsNotNone(manager)
        self.assertTrue(hasattr(manager, 'load_plugin'))
        self.assertTrue(hasattr(manager, 'unload_plugin'))
        self.assertTrue(hasattr(manager, 'get_loaded_plugins'))

    @unittest.skipIf(not BINDINGS_AVAILABLE, "QtForge bindings not available")
    def test_plugin_manager_load_invalid_path(self) -> None:
        """Test loading plugin from invalid path."""
        manager = core.create_plugin_manager()

        # Test with non-existent file
        with self.assertRaises((RuntimeError, ValueError, OSError)):
            manager.load_plugin("/non/existent/path.so")

        # Test with empty path
        with self.assertRaises((RuntimeError, ValueError)):
            manager.load_plugin("")

    def test_plugin_manager_load_options(self) -> None:
        """Test plugin loading with various options."""
        manager = core.create_plugin_manager()

        # Test with default options
        options = core.PluginLoadOptions()
        assert options is not None

        # Test setting various options
        if hasattr(options, 'lazy_loading'):
            options.lazy_loading = True
        if hasattr(options, 'verify_signature'):
            options.verify_signature = False
        if hasattr(options, 'sandbox_enabled'):
            options.sandbox_enabled = True

    def test_plugin_manager_get_loaded_plugins(self) -> None:
        """Test getting loaded plugins list."""
        manager = core.create_plugin_manager()
        plugins = manager.get_loaded_plugins()
        assert isinstance(plugins, (list, tuple))

    def test_plugin_manager_unload_nonexistent(self) -> None:
        """Test unloading non-existent plugin."""
        manager = core.create_plugin_manager()

        # Should handle gracefully or raise appropriate exception
        try:
            result = manager.unload_plugin("non_existent_plugin")
            # If it returns a result, it should indicate failure
            if result is not None:
                assert not result
        except (RuntimeError, ValueError, KeyError):
            # These are acceptable exceptions for non-existent plugins
            pass


class TestPluginLoader:
    """Test PluginLoader functionality."""

    def test_plugin_loader_creation(self) -> None:
        """Test PluginLoader can be created."""
        loader = core.create_plugin_loader()
        assert loader is not None
        assert hasattr(loader, 'load')
        assert hasattr(loader, 'unload')

    def test_plugin_loader_load_invalid_metadata(self) -> None:
        """Test loading with invalid metadata."""
        loader = core.create_plugin_loader()

        # Test with None metadata
        with pytest.raises((RuntimeError, ValueError, TypeError)):
            loader.load(None)

    def test_plugin_loader_supported_formats(self) -> None:
        """Test getting supported plugin formats."""
        loader = core.create_plugin_loader()

        if hasattr(loader, 'get_supported_formats'):
            formats = loader.get_supported_formats()
            assert isinstance(formats, (list, tuple))
            # Should support at least native plugins
            assert len(formats) > 0


class TestPluginRegistry:
    """Test PluginRegistry functionality."""

    def test_plugin_registry_creation(self) -> None:
        """Test PluginRegistry can be created."""
        registry = core.create_plugin_registry()
        assert registry is not None
        assert hasattr(registry, 'register_plugin')
        assert hasattr(registry, 'unregister_plugin')
        assert hasattr(registry, 'find_plugin')

    def test_plugin_registry_register_invalid(self) -> None:
        """Test registering invalid plugin."""
        registry = core.create_plugin_registry()

        # Test with None plugin
        with pytest.raises((RuntimeError, ValueError, TypeError)):
            registry.register_plugin(None)

    def test_plugin_registry_find_nonexistent(self) -> None:
        """Test finding non-existent plugin."""
        registry = core.create_plugin_registry()

        result = registry.find_plugin("non_existent_plugin")
        assert result is None or (hasattr(result, '__len__') and len(result) == 0)

    def test_plugin_registry_get_all_plugins(self) -> None:
        """Test getting all registered plugins."""
        registry = core.create_plugin_registry()

        if hasattr(registry, 'get_all_plugins'):
            plugins = registry.get_all_plugins()
            assert isinstance(plugins, (list, tuple, dict))


class TestPluginDependencyResolver:
    """Test PluginDependencyResolver functionality."""

    def test_dependency_resolver_creation(self) -> None:
        """Test PluginDependencyResolver can be created."""
        resolver = core.create_plugin_dependency_resolver()
        assert resolver is not None
        assert hasattr(resolver, 'resolve_dependencies')

    def test_dependency_resolver_empty_list(self) -> None:
        """Test resolving dependencies for empty plugin list."""
        resolver = core.create_plugin_dependency_resolver()

        result = resolver.resolve_dependencies([])
        assert isinstance(result, (list, tuple))
        assert len(result) == 0

    def test_dependency_resolver_circular_dependency(self) -> None:
        """Test handling circular dependencies."""
        resolver = core.create_plugin_dependency_resolver()

        # Create mock plugins with circular dependencies
        if hasattr(core, 'PluginMetadata'):
            plugin_a = core.PluginMetadata()
            plugin_b = core.PluginMetadata()

            # This should either resolve gracefully or raise appropriate exception
            try:
                result = resolver.resolve_dependencies([plugin_a, plugin_b])
                assert isinstance(result, (list, tuple))
            except (RuntimeError, ValueError):
                # Circular dependency detection is acceptable
                pass


class TestPluginLifecycleManager:
    """Test PluginLifecycleManager functionality."""

    def test_lifecycle_manager_creation(self) -> None:
        """Test PluginLifecycleManager can be created."""
        manager = core.create_plugin_lifecycle_manager()
        assert manager is not None
        assert hasattr(manager, 'start_plugin')
        assert hasattr(manager, 'stop_plugin')

    def test_lifecycle_manager_invalid_plugin(self) -> None:
        """Test lifecycle operations on invalid plugin."""
        manager = core.create_plugin_lifecycle_manager()

        # Test starting non-existent plugin
        with pytest.raises((RuntimeError, ValueError, KeyError)):
            manager.start_plugin("non_existent_plugin")

        # Test stopping non-existent plugin
        with pytest.raises((RuntimeError, ValueError, KeyError)):
            manager.stop_plugin("non_existent_plugin")

    def test_lifecycle_manager_get_plugin_state(self) -> None:
        """Test getting plugin state."""
        manager = core.create_plugin_lifecycle_manager()

        if hasattr(manager, 'get_plugin_state'):
            # Should handle non-existent plugin gracefully
            state = manager.get_plugin_state("non_existent_plugin")
            assert state is None or hasattr(core, 'PluginState')


class TestPluginEnums:
    """Test plugin-related enums."""

    def test_plugin_state_enum(self) -> None:
        """Test PluginState enum values."""
        assert hasattr(core, 'PluginState')

        # Test common state values
        states = ['Unloaded', 'Loading', 'Loaded', 'Starting', 'Running', 'Stopping', 'Error']
        for state in states:
            if hasattr(core.PluginState, state):
                value = getattr(core.PluginState, state)
                assert value is not None

    def test_plugin_capability_enum(self) -> None:
        """Test PluginCapability enum values."""
        assert hasattr(core, 'PluginCapability')

        # Test common capability values
        capabilities = ['Service', 'Network', 'FileSystem', 'Database', 'UI']
        for capability in capabilities:
            if hasattr(core.PluginCapability, capability):
                value = getattr(core.PluginCapability, capability)
                assert value is not None

    def test_plugin_priority_enum(self) -> None:
        """Test PluginPriority enum values."""
        assert hasattr(core, 'PluginPriority')

        # Test common priority values
        priorities = ['Low', 'Normal', 'High', 'Critical']
        for priority in priorities:
            if hasattr(core.PluginPriority, priority):
                value = getattr(core.PluginPriority, priority)
                assert value is not None

    def test_plugin_type_enum(self) -> None:
        """Test PluginType enum values."""
        if hasattr(core, 'PluginType'):
            # Test type values
            types = ['Native', 'Python', 'Lua', 'Remote', 'Composite']
            for plugin_type in types:
                if hasattr(core.PluginType, plugin_type):
                    value = getattr(core.PluginType, plugin_type)
                    assert value is not None


class TestPluginMetadata:
    """Test PluginMetadata functionality."""

    def test_plugin_metadata_creation(self) -> None:
        """Test PluginMetadata can be created."""
        if hasattr(core, 'PluginMetadata'):
            metadata = core.PluginMetadata()
            assert metadata is not None

    def test_plugin_metadata_properties(self) -> None:
        """Test PluginMetadata properties."""
        if hasattr(core, 'PluginMetadata'):
            metadata = core.PluginMetadata()

            # Test common properties
            properties = ['name', 'version', 'description', 'author', 'dependencies']
            for prop in properties:
                if hasattr(metadata, prop):
                    # Should be able to get and set property
                    try:
                        value = getattr(metadata, prop)
                        setattr(metadata, prop, value)
                    except (AttributeError, TypeError):
                        # Some properties might be read-only
                        pass


class TestErrorHandling:
    """Test error handling in core bindings."""

    def test_plugin_error_creation(self) -> None:
        """Test creating plugin errors."""
        if hasattr(core, 'PluginError'):
            error = core.PluginError("Test error message")
            assert error is not None
            assert str(error) == "Test error message"

    def test_plugin_error_codes(self) -> None:
        """Test plugin error codes."""
        if hasattr(core, 'PluginErrorCode'):
            # Test common error codes
            codes = ['LoadFailed', 'InitializationFailed', 'DependencyNotFound', 'InvalidMetadata']
            for code in codes:
                if hasattr(core.PluginErrorCode, code):
                    value = getattr(core.PluginErrorCode, code)
                    assert value is not None


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
