#!/usr/bin/env python3
"""
QtForge Python Bindings Example 1: Basic Plugin Management

This example demonstrates the fundamental plugin management operations
using the QtForge Python bindings, including:
- Creating a plugin manager
- Loading and unloading plugins
- Querying plugin information
- Handling plugin states and lifecycle
"""

import sys
import os
from pathlib import Path

# Add the build directory to Python path
sys.path.insert(0, str(Path(__file__).parent.parent.parent / "build"))

try:
    import qtforge
    import qtforge.core as core
    print("✅ QtForge Python bindings loaded successfully")
    print(f"📦 QtForge version: {qtforge.__version__}")
except ImportError as e:
    print(f"❌ Failed to import QtForge: {e}")
    print("Make sure QtForge is built with Python bindings enabled:")
    print("  cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON ..")
    print("  make")
    sys.exit(1)


def demonstrate_plugin_manager_creation():
    """Demonstrate creating and configuring a plugin manager."""
    print("\n" + "="*50)
    print("🔧 Creating Plugin Manager")
    print("="*50)
    
    try:
        # Create a plugin manager
        manager = core.create_plugin_manager()
        print("✅ Plugin manager created successfully")
        
        # Check available methods
        methods = ['load_plugin', 'unload_plugin', 'get_loaded_plugins', 'is_plugin_loaded']
        available_methods = []
        
        for method in methods:
            if hasattr(manager, method):
                available_methods.append(method)
        
        print(f"📋 Available methods: {', '.join(available_methods)}")
        
        return manager
        
    except Exception as e:
        print(f"❌ Failed to create plugin manager: {e}")
        return None


def demonstrate_plugin_loading(manager):
    """Demonstrate plugin loading operations."""
    print("\n" + "="*50)
    print("📥 Plugin Loading Operations")
    print("="*50)
    
    if not manager:
        print("❌ No plugin manager available")
        return
    
    # Example plugin paths (these may not exist in a real scenario)
    example_plugins = [
        "libexample_plugin.so",
        "/path/to/plugin.dll",
        "example_plugin.dylib"
    ]
    
    for plugin_path in example_plugins:
        print(f"\n🔍 Attempting to load: {plugin_path}")
        
        try:
            # Attempt to load the plugin
            result = manager.load_plugin(plugin_path)
            
            if result:
                print(f"✅ Successfully loaded: {plugin_path}")
                
                # Check if plugin is loaded
                if hasattr(manager, 'is_plugin_loaded'):
                    is_loaded = manager.is_plugin_loaded(plugin_path)
                    print(f"📊 Plugin loaded status: {is_loaded}")
                    
            else:
                print(f"⚠️  Plugin loading returned false: {plugin_path}")
                
        except FileNotFoundError:
            print(f"📁 Plugin file not found: {plugin_path}")
        except Exception as e:
            print(f"❌ Failed to load plugin: {e}")


def demonstrate_plugin_enumeration(manager):
    """Demonstrate enumerating loaded plugins."""
    print("\n" + "="*50)
    print("📋 Plugin Enumeration")
    print("="*50)
    
    if not manager:
        print("❌ No plugin manager available")
        return
    
    try:
        # Get list of loaded plugins
        loaded_plugins = manager.get_loaded_plugins()
        
        print(f"📊 Number of loaded plugins: {len(loaded_plugins)}")
        
        if loaded_plugins:
            print("📋 Loaded plugins:")
            for i, plugin in enumerate(loaded_plugins, 1):
                print(f"  {i}. {plugin}")
        else:
            print("📭 No plugins currently loaded")
            
        return loaded_plugins
        
    except Exception as e:
        print(f"❌ Failed to enumerate plugins: {e}")
        return []


def demonstrate_plugin_registry():
    """Demonstrate plugin registry operations."""
    print("\n" + "="*50)
    print("📚 Plugin Registry Operations")
    print("="*50)
    
    try:
        # Create a plugin registry
        registry = core.create_plugin_registry()
        print("✅ Plugin registry created successfully")
        
        # Demonstrate registry operations
        if hasattr(registry, 'get_all_plugins'):
            all_plugins = registry.get_all_plugins()
            print(f"📊 Total registered plugins: {len(all_plugins) if all_plugins else 0}")
        
        # Demonstrate plugin search
        if hasattr(registry, 'find_plugin'):
            print("\n🔍 Searching for plugins...")
            
            search_terms = ["example", "test", "demo"]
            for term in search_terms:
                try:
                    results = registry.find_plugin(term)
                    if results:
                        print(f"  Found {len(results)} plugins matching '{term}'")
                    else:
                        print(f"  No plugins found matching '{term}'")
                except Exception as e:
                    print(f"  Search for '{term}' failed: {e}")
        
        return registry
        
    except Exception as e:
        print(f"❌ Failed to create plugin registry: {e}")
        return None


def demonstrate_plugin_lifecycle():
    """Demonstrate plugin lifecycle management."""
    print("\n" + "="*50)
    print("🔄 Plugin Lifecycle Management")
    print("="*50)
    
    try:
        # Create a lifecycle manager
        lifecycle_manager = core.create_plugin_lifecycle_manager()
        print("✅ Plugin lifecycle manager created successfully")
        
        # Demonstrate lifecycle operations
        example_plugin = "example_plugin"
        
        print(f"\n🔄 Managing lifecycle for: {example_plugin}")
        
        # Check initial state
        if hasattr(lifecycle_manager, 'get_plugin_state'):
            try:
                state = lifecycle_manager.get_plugin_state(example_plugin)
                print(f"📊 Initial state: {state}")
            except Exception as e:
                print(f"⚠️  Could not get initial state: {e}")
        
        # Attempt to start plugin
        if hasattr(lifecycle_manager, 'start_plugin'):
            try:
                result = lifecycle_manager.start_plugin(example_plugin)
                print(f"▶️  Start plugin result: {result}")
            except Exception as e:
                print(f"⚠️  Could not start plugin: {e}")
        
        # Attempt to stop plugin
        if hasattr(lifecycle_manager, 'stop_plugin'):
            try:
                result = lifecycle_manager.stop_plugin(example_plugin)
                print(f"⏹️  Stop plugin result: {result}")
            except Exception as e:
                print(f"⚠️  Could not stop plugin: {e}")
        
        return lifecycle_manager
        
    except Exception as e:
        print(f"❌ Failed to create lifecycle manager: {e}")
        return None


def demonstrate_plugin_dependencies():
    """Demonstrate plugin dependency resolution."""
    print("\n" + "="*50)
    print("🔗 Plugin Dependency Resolution")
    print("="*50)
    
    try:
        # Create a dependency resolver
        resolver = core.create_plugin_dependency_resolver()
        print("✅ Plugin dependency resolver created successfully")
        
        # Demonstrate dependency resolution with empty list
        print("\n🔍 Resolving dependencies for empty plugin list...")
        try:
            resolved = resolver.resolve_dependencies([])
            print(f"✅ Resolved dependencies: {len(resolved)} plugins")
        except Exception as e:
            print(f"❌ Dependency resolution failed: {e}")
        
        # Create mock plugin metadata if available
        if hasattr(core, 'PluginMetadata'):
            print("\n📋 Creating mock plugin metadata...")
            try:
                metadata1 = core.PluginMetadata()
                metadata2 = core.PluginMetadata()
                
                # Set basic properties if available
                if hasattr(metadata1, 'name'):
                    metadata1.name = "plugin_a"
                    metadata2.name = "plugin_b"
                
                print("✅ Mock plugin metadata created")
                
                # Attempt dependency resolution
                try:
                    resolved = resolver.resolve_dependencies([metadata1, metadata2])
                    print(f"✅ Resolved {len(resolved)} plugins with dependencies")
                except Exception as e:
                    print(f"⚠️  Dependency resolution with metadata failed: {e}")
                    
            except Exception as e:
                print(f"❌ Failed to create plugin metadata: {e}")
        
        return resolver
        
    except Exception as e:
        print(f"❌ Failed to create dependency resolver: {e}")
        return None


def demonstrate_plugin_enums():
    """Demonstrate plugin-related enumerations."""
    print("\n" + "="*50)
    print("📊 Plugin Enumerations")
    print("="*50)
    
    # Plugin State enum
    if hasattr(core, 'PluginState'):
        print("🔄 Plugin States:")
        states = ['Unloaded', 'Loading', 'Loaded', 'Starting', 'Running', 'Stopping', 'Error']
        for state in states:
            if hasattr(core.PluginState, state):
                value = getattr(core.PluginState, state)
                print(f"  • {state}: {value}")
    
    # Plugin Capability enum
    if hasattr(core, 'PluginCapability'):
        print("\n🛠️  Plugin Capabilities:")
        capabilities = ['Service', 'Network', 'FileSystem', 'Database', 'UI']
        for capability in capabilities:
            if hasattr(core.PluginCapability, capability):
                value = getattr(core.PluginCapability, capability)
                print(f"  • {capability}: {value}")
    
    # Plugin Priority enum
    if hasattr(core, 'PluginPriority'):
        print("\n⚡ Plugin Priorities:")
        priorities = ['Low', 'Normal', 'High', 'Critical']
        for priority in priorities:
            if hasattr(core.PluginPriority, priority):
                value = getattr(core.PluginPriority, priority)
                print(f"  • {priority}: {value}")
    
    # Plugin Type enum
    if hasattr(core, 'PluginType'):
        print("\n🏷️  Plugin Types:")
        types = ['Native', 'Python', 'Lua', 'Remote', 'Composite']
        for plugin_type in types:
            if hasattr(core.PluginType, plugin_type):
                value = getattr(core.PluginType, plugin_type)
                print(f"  • {plugin_type}: {value}")


def demonstrate_error_handling():
    """Demonstrate error handling in plugin operations."""
    print("\n" + "="*50)
    print("⚠️  Error Handling Demonstration")
    print("="*50)
    
    # Create manager for error testing
    try:
        manager = core.create_plugin_manager()
        
        # Test loading non-existent plugin
        print("🧪 Testing error handling with non-existent plugin...")
        try:
            result = manager.load_plugin("/definitely/does/not/exist.so")
            print(f"⚠️  Unexpected success: {result}")
        except FileNotFoundError as e:
            print(f"✅ Correctly caught FileNotFoundError: {e}")
        except Exception as e:
            print(f"✅ Correctly caught exception: {type(e).__name__}: {e}")
        
        # Test with invalid parameters
        print("\n🧪 Testing error handling with invalid parameters...")
        try:
            result = manager.load_plugin("")  # Empty path
            print(f"⚠️  Unexpected success with empty path: {result}")
        except Exception as e:
            print(f"✅ Correctly caught exception for empty path: {type(e).__name__}: {e}")
        
        try:
            result = manager.load_plugin(None)  # None path
            print(f"⚠️  Unexpected success with None path: {result}")
        except Exception as e:
            print(f"✅ Correctly caught exception for None path: {type(e).__name__}: {e}")
            
    except Exception as e:
        print(f"❌ Failed to create manager for error testing: {e}")


def main():
    """Main demonstration function."""
    print("QtForge Python Bindings - Basic Plugin Management Example")
    print("=" * 60)
    
    # Demonstrate each aspect of plugin management
    manager = demonstrate_plugin_manager_creation()
    demonstrate_plugin_loading(manager)
    demonstrate_plugin_enumeration(manager)
    demonstrate_plugin_registry()
    demonstrate_plugin_lifecycle()
    demonstrate_plugin_dependencies()
    demonstrate_plugin_enums()
    demonstrate_error_handling()
    
    print("\n" + "="*60)
    print("🎉 Basic Plugin Management Example Complete!")
    print("="*60)
    
    print("\n📚 Key Takeaways:")
    print("• Plugin managers coordinate all plugin operations")
    print("• Plugin registries help discover and organize plugins")
    print("• Lifecycle managers handle plugin state transitions")
    print("• Dependency resolvers ensure proper loading order")
    print("• Proper error handling is essential for robust applications")
    
    print("\n🔗 Next Steps:")
    print("• Explore communication between plugins")
    print("• Learn about security and validation")
    print("• Implement custom plugin interfaces")
    print("• Set up plugin orchestration workflows")


if __name__ == "__main__":
    main()
