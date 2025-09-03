#!/usr/bin/env python3
"""
Unit tests for QtForge Python core bindings.
Tests individual functions and classes in the core module with comprehensive coverage.
"""

import pytest
import sys
import os
import tempfile
import json
from unittest.mock import Mock, patch, MagicMock
from pathlib import Path

# Add the build directory to Python path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../build'))

try:
    import qtforge
    import qtforge.core as core
    BINDINGS_AVAILABLE = True
except ImportError as e:
    BINDINGS_AVAILABLE = False
    pytest.skip(f"QtForge bindings not available: {e}", allow_module_level=True)


class TestPluginState:
    """Test PluginState enum."""
    
    def test_plugin_state_values(self):
        """Test all PluginState enum values exist."""
        assert hasattr(core, 'PluginState')
        assert hasattr(core.PluginState, 'Unloaded')
        assert hasattr(core.PluginState, 'Loaded')
        assert hasattr(core.PluginState, 'Initialized')
        assert hasattr(core.PluginState, 'Running')
        assert hasattr(core.PluginState, 'Stopped')
        assert hasattr(core.PluginState, 'Error')
    
    def test_plugin_state_values_are_different(self):
        """Test that PluginState enum values are distinct."""
        states = [
            core.PluginState.Unloaded,
            core.PluginState.Loaded,
            core.PluginState.Initialized,
            core.PluginState.Running,
            core.PluginState.Stopped,
            core.PluginState.Error
        ]
        assert len(set(states)) == len(states), "PluginState values should be unique"
    
    def test_plugin_state_string_representation(self):
        """Test string representation of PluginState values."""
        assert str(core.PluginState.Unloaded) == "PluginState.Unloaded"
        assert str(core.PluginState.Running) == "PluginState.Running"


class TestPluginCapability:
    """Test PluginCapability enum."""
    
    def test_plugin_capability_values(self):
        """Test all PluginCapability enum values exist."""
        assert hasattr(core, 'PluginCapability')
        assert hasattr(core.PluginCapability, 'None')
        assert hasattr(core.PluginCapability, 'Service')
        assert hasattr(core.PluginCapability, 'UI')
        assert hasattr(core.PluginCapability, 'Network')
        assert hasattr(core.PluginCapability, 'DataProcessor')
        assert hasattr(core.PluginCapability, 'Scripting')
    
    def test_plugin_capability_values_are_different(self):
        """Test that PluginCapability enum values are distinct."""
        capabilities = [
            core.PluginCapability.None,
            core.PluginCapability.Service,
            core.PluginCapability.UI,
            core.PluginCapability.Network,
            core.PluginCapability.DataProcessor,
            core.PluginCapability.Scripting
        ]
        assert len(set(capabilities)) == len(capabilities), "PluginCapability values should be unique"


class TestPluginPriority:
    """Test PluginPriority enum."""
    
    def test_plugin_priority_values(self):
        """Test all PluginPriority enum values exist."""
        assert hasattr(core, 'PluginPriority')
        assert hasattr(core.PluginPriority, 'Lowest')
        assert hasattr(core.PluginPriority, 'Low')
        assert hasattr(core.PluginPriority, 'Normal')
        assert hasattr(core.PluginPriority, 'High')
        assert hasattr(core.PluginPriority, 'Highest')
    
    def test_plugin_priority_ordering(self):
        """Test that PluginPriority values have correct ordering."""
        priorities = [
            core.PluginPriority.Lowest,
            core.PluginPriority.Low,
            core.PluginPriority.Normal,
            core.PluginPriority.High,
            core.PluginPriority.Highest
        ]
        # Test that priorities are in ascending order
        for i in range(len(priorities) - 1):
            assert priorities[i] < priorities[i + 1], f"Priority ordering incorrect at index {i}"


class TestPluginLifecycleEvent:
    """Test PluginLifecycleEvent enum."""
    
    def test_lifecycle_event_values(self):
        """Test all PluginLifecycleEvent enum values exist."""
        assert hasattr(core, 'PluginLifecycleEvent')
        events = [
            'BeforeInitialize', 'AfterInitialize', 'BeforeShutdown', 'AfterShutdown',
            'BeforePause', 'AfterPause', 'BeforeResume', 'AfterResume',
            'StateChanged', 'Error', 'Timeout', 'HealthCheck',
            'ResourceWarning', 'DependencyChanged'
        ]
        for event in events:
            assert hasattr(core.PluginLifecycleEvent, event), f"Missing lifecycle event: {event}"


class TestPluginRegistry:
    """Test PluginRegistry class."""
    
    def test_plugin_registry_creation(self):
        """Test PluginRegistry can be created."""
        registry = core.PluginRegistry()
        assert registry is not None
        assert isinstance(registry, core.PluginRegistry)
    
    def test_plugin_registry_initial_state(self):
        """Test PluginRegistry initial state."""
        registry = core.PluginRegistry()
        assert registry.size() == 0
        assert len(registry.get_all_plugins()) == 0
    
    def test_plugin_registry_clear(self):
        """Test PluginRegistry clear method."""
        registry = core.PluginRegistry()
        registry.clear()
        assert registry.size() == 0
    
    def test_plugin_registry_has_plugin_empty(self):
        """Test has_plugin on empty registry."""
        registry = core.PluginRegistry()
        assert not registry.has_plugin("nonexistent")
    
    def test_plugin_registry_get_plugin_empty(self):
        """Test get_plugin on empty registry returns None."""
        registry = core.PluginRegistry()
        plugin = registry.get_plugin("nonexistent")
        assert plugin is None
    
    def test_plugin_registry_get_plugins_by_capability_empty(self):
        """Test get_plugins_by_capability on empty registry."""
        registry = core.PluginRegistry()
        plugins = registry.get_plugins_by_capability(core.PluginCapability.Service)
        assert len(plugins) == 0
    
    def test_plugin_registry_repr(self):
        """Test PluginRegistry string representation."""
        registry = core.PluginRegistry()
        repr_str = repr(registry)
        assert "PluginRegistry" in repr_str
        assert "size=0" in repr_str


class TestPluginLoader:
    """Test PluginLoader class."""
    
    def test_plugin_loader_creation(self):
        """Test PluginLoader can be created."""
        loader = core.PluginLoader()
        assert loader is not None
        assert isinstance(loader, core.PluginLoader)
    
    def test_plugin_loader_initial_state(self):
        """Test PluginLoader initial state."""
        loader = core.PluginLoader()
        plugins = loader.get_loaded_plugins()
        assert len(plugins) == 0
    
    def test_plugin_loader_is_plugin_loaded_empty(self):
        """Test is_plugin_loaded on empty loader."""
        loader = core.PluginLoader()
        assert not loader.is_plugin_loaded("nonexistent")
    
    def test_plugin_loader_load_nonexistent_plugin(self):
        """Test loading nonexistent plugin returns error."""
        loader = core.PluginLoader()
        result = loader.load_plugin("/nonexistent/path.so")
        # Should return an error result
        assert result is not None
    
    def test_plugin_loader_unload_nonexistent_plugin(self):
        """Test unloading nonexistent plugin."""
        loader = core.PluginLoader()
        result = loader.unload_plugin("nonexistent")
        # Should handle gracefully
        assert result is not None
    
    def test_plugin_loader_repr(self):
        """Test PluginLoader string representation."""
        loader = core.PluginLoader()
        repr_str = repr(loader)
        assert "PluginLoader" in repr_str


class TestPluginManager:
    """Test PluginManager class."""
    
    def test_plugin_manager_creation(self):
        """Test PluginManager can be created."""
        manager = core.PluginManager()
        assert manager is not None
        assert isinstance(manager, core.PluginManager)
    
    def test_plugin_manager_create_static(self):
        """Test PluginManager static create method."""
        manager = core.PluginManager.create()
        assert manager is not None
        assert isinstance(manager, core.PluginManager)
    
    def test_plugin_manager_initial_state(self):
        """Test PluginManager initial state."""
        manager = core.PluginManager()
        assert manager.get_plugin_count() == 0
        assert len(manager.get_all_plugins()) == 0
    
    def test_plugin_manager_has_plugin_empty(self):
        """Test has_plugin on empty manager."""
        manager = core.PluginManager()
        assert not manager.has_plugin("nonexistent")
    
    def test_plugin_manager_get_plugin_empty(self):
        """Test get_plugin on empty manager."""
        manager = core.PluginManager()
        plugin = manager.get_plugin("nonexistent")
        assert plugin is None
    
    def test_plugin_manager_get_plugins_by_capability_empty(self):
        """Test get_plugins_by_capability on empty manager."""
        manager = core.PluginManager()
        plugins = manager.get_plugins_by_capability(core.PluginCapability.Service)
        assert len(plugins) == 0
    
    def test_plugin_manager_clear(self):
        """Test PluginManager clear method."""
        manager = core.PluginManager()
        manager.clear()
        assert manager.get_plugin_count() == 0
    
    def test_plugin_manager_repr(self):
        """Test PluginManager string representation."""
        manager = core.PluginManager()
        repr_str = repr(manager)
        assert "PluginManager" in repr_str
        assert "plugins=0" in repr_str


class TestDependencyNode:
    """Test DependencyNode class."""
    
    def test_dependency_node_creation(self):
        """Test DependencyNode can be created."""
        node = core.DependencyNode()
        assert node is not None
        assert isinstance(node, core.DependencyNode)
    
    def test_dependency_node_attributes(self):
        """Test DependencyNode attributes."""
        node = core.DependencyNode()
        
        # Test default values
        assert node.plugin_id == ""
        assert len(node.dependencies) == 0
        assert len(node.dependents) == 0
        assert node.load_order == 0
        
        # Test setting values
        node.plugin_id = "test_plugin"
        node.load_order = 5
        
        assert node.plugin_id == "test_plugin"
        assert node.load_order == 5
    
    def test_dependency_node_repr(self):
        """Test DependencyNode string representation."""
        node = core.DependencyNode()
        node.plugin_id = "test_plugin"
        node.load_order = 3
        
        repr_str = repr(node)
        assert "DependencyNode" in repr_str
        assert "test_plugin" in repr_str
        assert "3" in repr_str


class TestPluginDependencyResolver:
    """Test PluginDependencyResolver class."""
    
    def test_dependency_resolver_creation(self):
        """Test PluginDependencyResolver can be created."""
        resolver = core.PluginDependencyResolver()
        assert resolver is not None
        assert isinstance(resolver, core.PluginDependencyResolver)
    
    def test_dependency_resolver_with_parent(self):
        """Test PluginDependencyResolver creation with parent."""
        resolver = core.PluginDependencyResolver(None)
        assert resolver is not None
    
    def test_dependency_resolver_initial_state(self):
        """Test PluginDependencyResolver initial state."""
        resolver = core.PluginDependencyResolver()
        
        # Test initial methods don't crash
        graph = resolver.get_dependency_graph()
        assert isinstance(graph, dict)
        
        load_order = resolver.get_load_order()
        assert isinstance(load_order, list)
        
        assert not resolver.has_circular_dependencies()
    
    def test_dependency_resolver_clear(self):
        """Test PluginDependencyResolver clear method."""
        resolver = core.PluginDependencyResolver()
        resolver.clear()
        # Should not crash
        assert True
    
    def test_dependency_resolver_can_unload_safely(self):
        """Test can_unload_safely method."""
        resolver = core.PluginDependencyResolver()
        # Should handle nonexistent plugin gracefully
        result = resolver.can_unload_safely("nonexistent")
        assert isinstance(result, bool)
    
    def test_dependency_resolver_get_dependents(self):
        """Test get_dependents method."""
        resolver = core.PluginDependencyResolver()
        dependents = resolver.get_dependents("nonexistent")
        assert isinstance(dependents, list)
    
    def test_dependency_resolver_get_dependencies(self):
        """Test get_dependencies method."""
        resolver = core.PluginDependencyResolver()
        dependencies = resolver.get_dependencies("nonexistent")
        assert isinstance(dependencies, list)
    
    def test_dependency_resolver_repr(self):
        """Test PluginDependencyResolver string representation."""
        resolver = core.PluginDependencyResolver()
        repr_str = repr(resolver)
        assert "PluginDependencyResolver" in repr_str


class TestPluginLifecycleConfig:
    """Test PluginLifecycleConfig class."""
    
    def test_lifecycle_config_creation(self):
        """Test PluginLifecycleConfig can be created."""
        config = core.PluginLifecycleConfig()
        assert config is not None
        assert isinstance(config, core.PluginLifecycleConfig)
    
    def test_lifecycle_config_attributes(self):
        """Test PluginLifecycleConfig attributes."""
        config = core.PluginLifecycleConfig()
        
        # Test that attributes exist and can be set
        config.initialization_timeout = 5000
        config.shutdown_timeout = 3000
        config.pause_timeout = 1000
        config.resume_timeout = 1000
        config.health_check_interval = 10000
        config.enable_graceful_shutdown = True
        config.enable_health_monitoring = True
        config.enable_resource_monitoring = False
        config.auto_restart_on_failure = True
        config.max_restart_attempts = 3
        config.restart_delay = 2000
        
        assert config.initialization_timeout == 5000
        assert config.shutdown_timeout == 3000
        assert config.enable_graceful_shutdown == True
        assert config.max_restart_attempts == 3
    
    def test_lifecycle_config_json_serialization(self):
        """Test PluginLifecycleConfig JSON serialization."""
        config = core.PluginLifecycleConfig()
        config.initialization_timeout = 5000
        config.enable_graceful_shutdown = True
        
        # Test to_json
        json_data = config.to_json()
        assert json_data is not None
        
        # Test from_json
        config2 = core.PluginLifecycleConfig.from_json(json_data)
        assert config2 is not None
        assert isinstance(config2, core.PluginLifecycleConfig)
    
    def test_lifecycle_config_repr(self):
        """Test PluginLifecycleConfig string representation."""
        config = core.PluginLifecycleConfig()
        repr_str = repr(config)
        assert "PluginLifecycleConfig" in repr_str


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
