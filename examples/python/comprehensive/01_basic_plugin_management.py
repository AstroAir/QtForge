#!/usr/bin/env python3
"""
QtForge Python Bindings Example: Basic Plugin Management

This example demonstrates the fundamental plugin management operations using
the QtForge Python bindings. It covers:

1. Creating and configuring a PluginManager
2. Loading and unloading plugins
3. Querying plugin information
4. Managing plugin lifecycle
5. Error handling and best practices

Prerequisites:
- QtForge built with Python bindings enabled
- Python 3.8 or later
"""
# type: ignore

import sys
import os
from pathlib import Path
from typing import Any, Optional

# Add the build directory to Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../build'))

try:
    import qtforge
    import qtforge.core as core
    print("✅ QtForge Python bindings loaded successfully")
    print(f"QtForge version: {qtforge.__version__ if hasattr(qtforge, '__version__') else 'Unknown'}")
except ImportError as e:
    print(f"❌ Failed to import QtForge: {e}")
    print("Make sure QtForge is built with Python bindings enabled")
    sys.exit(1)


class PluginManagerExample:
    """Comprehensive example of plugin management operations."""
    
    def __init__(self) -> None:
        """Initialize the plugin manager example."""
        self.manager: Optional[Any] = None
        self.registry: Optional[Any] = None
        self.loader: Optional[Any] = None
        self.setup_components()
    
    def setup_components(self) -> None:
        """Set up the core plugin management components."""
        print("\n🔧 Setting up plugin management components...")
        
        try:
            # Create PluginManager
            if hasattr(core, 'PluginManager'):
                self.manager = core.PluginManager()
                print("✅ PluginManager created")
            elif hasattr(core, 'create_plugin_manager'):
                self.manager = core.create_plugin_manager()
                print("✅ PluginManager created via factory function")
            else:
                raise RuntimeError("PluginManager not available")
            
            # Create PluginRegistry
            if hasattr(core, 'PluginRegistry'):
                self.registry = core.PluginRegistry()
                print("✅ PluginRegistry created")
            elif hasattr(core, 'create_plugin_registry'):
                self.registry = core.create_plugin_registry()
                print("✅ PluginRegistry created via factory function")
            
            # Create PluginLoader
            if hasattr(core, 'PluginLoader'):
                self.loader = core.PluginLoader()
                print("✅ PluginLoader created")
            elif hasattr(core, 'create_plugin_loader'):
                self.loader = core.create_plugin_loader()
                print("✅ PluginLoader created via factory function")
            
        except Exception as e:
            print(f"❌ Failed to setup components: {e}")
            raise
    
    def demonstrate_basic_operations(self) -> None:
        """Demonstrate basic plugin management operations."""
        print("\n📋 Demonstrating basic plugin operations...")
        
        # Check initial state
        if self.manager and hasattr(self.manager, 'get_plugin_count'):
            initial_count = self.manager.get_plugin_count()
            print(f"Initial plugin count: {initial_count}")
        
        # Get all plugins (should be empty initially)
        if self.manager and hasattr(self.manager, 'get_all_plugins'):
            plugins = self.manager.get_all_plugins()
            print(f"Initial plugins list length: {len(plugins)}")
        
        # Test plugin existence check
        if self.manager and hasattr(self.manager, 'has_plugin'):
            has_test_plugin = self.manager.has_plugin("test_plugin")
            print(f"Has 'test_plugin': {has_test_plugin}")
        
        # Test getting non-existent plugin
        if self.manager and hasattr(self.manager, 'get_plugin'):
            plugin = self.manager.get_plugin("non_existent_plugin")
            print(f"Get non-existent plugin result: {plugin}")
    
    def demonstrate_plugin_loading(self) -> None:
        """Demonstrate plugin loading operations."""
        print("\n🔌 Demonstrating plugin loading...")
        
        # Create some example plugin paths (these won't exist, but demonstrate the API)
        example_plugins = [
            "/path/to/example_plugin.so",
            "/path/to/another_plugin.dll",
            "relative/path/plugin.dylib"
        ]
        
        for plugin_path in example_plugins:
            print(f"\nAttempting to load plugin: {plugin_path}")
            
            try:
                if self.manager and hasattr(self.manager, 'load_plugin'):
                    result = self.manager.load_plugin(plugin_path)
                    print(f"Load result: {result}")
                elif self.loader and hasattr(self.loader, 'load_plugin'):
                    result = self.loader.load_plugin(plugin_path)
                    print(f"Load result: {result}")
                else:
                    print("⚠️  Plugin loading not available")
                    
            except Exception as e:
                print(f"Expected error (file doesn't exist): {e}")
    
    def demonstrate_plugin_queries(self) -> None:
        """Demonstrate plugin query operations."""
        print("\n🔍 Demonstrating plugin queries...")
        
        # Query plugins by capability
        if self.manager and hasattr(self.manager, 'get_plugins_by_capability'):
            try:
                # Test different capabilities if available
                capabilities = []
                if hasattr(core, 'PluginCapability'):
                    cap_attrs = ['Service', 'UI', 'Network', 'DataProcessor']
                    for attr in cap_attrs:
                        if hasattr(core.PluginCapability, attr):
                            capabilities.append(getattr(core.PluginCapability, attr))
                
                for capability in capabilities:
                    plugins = self.manager.get_plugins_by_capability(capability)
                    print(f"Plugins with capability {capability}: {len(plugins)}")
                    
            except Exception as e:
                print(f"Plugin capability query error: {e}")
        
        # Test registry operations if available
        if self.registry:
            print("\n📚 Testing registry operations...")
            
            if hasattr(self.registry, 'size'):
                size = self.registry.size()
                print(f"Registry size: {size}")
            
            if hasattr(self.registry, 'get_all_plugins'):
                all_plugins = self.registry.get_all_plugins()
                print(f"Registry plugins: {len(all_plugins)}")
    
    def demonstrate_plugin_states(self) -> None:
        """Demonstrate plugin state management."""
        print("\n🔄 Demonstrating plugin states...")
        
        # Show available plugin states
        if hasattr(core, 'PluginState'):
            print("Available plugin states:")
            state_attrs = ['Unloaded', 'Loaded', 'Initialized', 'Running', 'Stopped', 'Error']
            for attr in state_attrs:
                if hasattr(core.PluginState, attr):
                    state = getattr(core.PluginState, attr)
                    print(f"  - {attr}: {state}")
        
        # Show available plugin capabilities
        if hasattr(core, 'PluginCapability'):
            print("\nAvailable plugin capabilities:")
            cap_attrs = ['None', 'Service', 'UI', 'Network', 'DataProcessor', 'Scripting']
            for attr in cap_attrs:
                if hasattr(core.PluginCapability, attr):
                    capability = getattr(core.PluginCapability, attr)
                    print(f"  - {attr}: {capability}")
        
        # Show available plugin priorities
        if hasattr(core, 'PluginPriority'):
            print("\nAvailable plugin priorities:")
            priority_attrs = ['Lowest', 'Low', 'Normal', 'High', 'Highest']
            for attr in priority_attrs:
                if hasattr(core.PluginPriority, attr):
                    priority = getattr(core.PluginPriority, attr)
                    print(f"  - {attr}: {priority}")
    
    def demonstrate_dependency_management(self) -> None:
        """Demonstrate plugin dependency management."""
        print("\n🔗 Demonstrating dependency management...")
        
        # Create dependency resolver if available
        resolver = None
        try:
            if hasattr(core, 'PluginDependencyResolver'):
                resolver = core.PluginDependencyResolver()
                print("✅ PluginDependencyResolver created")
            elif hasattr(core, 'create_plugin_dependency_resolver'):
                resolver = core.create_plugin_dependency_resolver()
                print("✅ PluginDependencyResolver created via factory")
        except Exception as e:
            print(f"⚠️  Dependency resolver not available: {e}")
            return
        
        if resolver:
            # Test dependency operations
            try:
                if hasattr(resolver, 'get_load_order'):
                    load_order = resolver.get_load_order()
                    print(f"Current load order: {load_order}")
                
                if hasattr(resolver, 'has_circular_dependencies'):
                    has_circular = resolver.has_circular_dependencies()
                    print(f"Has circular dependencies: {has_circular}")
                
                if hasattr(resolver, 'get_dependency_graph'):
                    graph = resolver.get_dependency_graph()
                    print(f"Dependency graph type: {type(graph)}")
                
            except Exception as e:
                print(f"Dependency operation error: {e}")
    
    def demonstrate_lifecycle_management(self) -> None:
        """Demonstrate plugin lifecycle management."""
        print("\n♻️  Demonstrating lifecycle management...")
        
        # Test lifecycle configuration if available
        if hasattr(core, 'PluginLifecycleConfig'):
            try:
                config = core.PluginLifecycleConfig()
                print("✅ PluginLifecycleConfig created")
                
                # Configure lifecycle settings
                if hasattr(config, 'initialization_timeout'):
                    config.initialization_timeout = 5000  # type: ignore  # 5 seconds
                    print(f"Set initialization timeout: {config.initialization_timeout}ms")
                
                if hasattr(config, 'enable_health_monitoring'):
                    config.enable_health_monitoring = True
                    print(f"Health monitoring enabled: {config.enable_health_monitoring}")
                
                if hasattr(config, 'auto_restart_on_failure'):
                    config.auto_restart_on_failure = True
                    print(f"Auto-restart enabled: {config.auto_restart_on_failure}")
                
                # Test JSON serialization
                if hasattr(config, 'to_json'):
                    json_data = config.to_json()
                    print(f"Config serialized to JSON: {type(json_data)}")
                
            except Exception as e:
                print(f"Lifecycle configuration error: {e}")
        
        # Show lifecycle events if available
        if hasattr(core, 'PluginLifecycleEvent'):
            print("\nAvailable lifecycle events:")
            event_attrs = [
                'BeforeInitialize', 'AfterInitialize', 'BeforeShutdown', 'AfterShutdown',
                'StateChanged', 'Error', 'HealthCheck'
            ]
            for attr in event_attrs:
                if hasattr(core.PluginLifecycleEvent, attr):
                    event = getattr(core.PluginLifecycleEvent, attr)
                    print(f"  - {attr}: {event}")
    
    def demonstrate_error_handling(self) -> None:
        """Demonstrate proper error handling patterns."""
        print("\n⚠️  Demonstrating error handling...")
        
        # Test various error conditions
        error_tests = [
            ("Loading non-existent plugin", lambda: self.manager.load_plugin("/non/existent/path.so") if self.manager and hasattr(self.manager, 'load_plugin') else None),
            ("Getting non-existent plugin", lambda: self.manager.get_plugin("non_existent") if self.manager and hasattr(self.manager, 'get_plugin') else None),
            ("Unloading non-existent plugin", lambda: self.manager.unload_plugin("non_existent") if self.manager and hasattr(self.manager, 'unload_plugin') else None),
        ]
        
        for test_name, test_func in error_tests:
            print(f"\nTesting: {test_name}")
            try:
                result = test_func()
                print(f"Result: {result} (type: {type(result)})")
            except Exception as e:
                print(f"Exception caught: {type(e).__name__}: {e}")
    
    def cleanup(self) -> None:
        """Clean up resources."""
        print("\n🧹 Cleaning up resources...")
        
        # Clear manager if available
        if self.manager and hasattr(self.manager, 'clear'):
            try:
                self.manager.clear()
                print("✅ PluginManager cleared")
            except Exception as e:
                print(f"Manager cleanup error: {e}")
        
        # Clear registry if available
        if self.registry and hasattr(self.registry, 'clear'):
            try:
                self.registry.clear()
                print("✅ PluginRegistry cleared")
            except Exception as e:
                print(f"Registry cleanup error: {e}")
    
    def run_complete_example(self) -> int:
        """Run the complete plugin management example."""
        print("🚀 QtForge Python Bindings - Basic Plugin Management Example")
        print("=" * 70)
        
        try:
            self.demonstrate_basic_operations()
            self.demonstrate_plugin_loading()
            self.demonstrate_plugin_queries()
            self.demonstrate_plugin_states()
            self.demonstrate_dependency_management()
            self.demonstrate_lifecycle_management()
            self.demonstrate_error_handling()
            
        except Exception as e:
            print(f"\n❌ Example failed with error: {e}")
            return 1
        
        finally:
            self.cleanup()
        
        print("\n✅ Basic plugin management example completed successfully!")
        return 0


def main() -> int:
    """Main entry point for the example."""
    try:
        example = PluginManagerExample()
        return example.run_complete_example()
    except Exception as e:
        print(f"❌ Failed to run example: {e}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
