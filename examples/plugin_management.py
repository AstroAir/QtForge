#!/usr/bin/env python3
"""
QtForge Python Bindings - Plugin Management Example

This example demonstrates advanced plugin management features including:
- Plugin discovery and loading
- Plugin lifecycle management
- Plugin information and metadata
- Plugin capabilities and dependencies
"""

import sys
import os
from pathlib import Path
from typing import List, Dict, Any

try:
    import qtforge
    from qtforge.core import (
        PluginManager, PluginState, PluginCapability, PluginPriority,
        PluginLoadOptions, PluginInfo, PluginMetadata, Version
    )
except ImportError:
    # Try to find the bindings in the build directory
    build_dir = Path(__file__).parent.parent / "build" / "src" / "python"
    if build_dir.exists():
        sys.path.insert(0, str(build_dir))
    import qtforge
    from qtforge.core import (
        PluginManager, PluginState, PluginCapability, PluginPriority,
        PluginLoadOptions, PluginInfo, PluginMetadata, Version
    )

def demonstrate_plugin_manager() -> None:
    """Demonstrate basic plugin manager functionality."""
    print("\n" + "=" * 50)
    print("Plugin Manager Demonstration")
    print("=" * 50)
    
    # Create a plugin manager
    manager = PluginManager()
    print(f"Created plugin manager: {manager}")
    
    # Get initial state
    print(f"Initially loaded plugins: {manager.loaded_plugins()}")
    print(f"Search paths: {manager.search_paths()}")
    
    # Add some search paths (these might not exist, but that's okay for demo)
    demo_paths = ["./plugins", "../plugins", "/usr/local/lib/qtforge/plugins"]
    for path in demo_paths:
        try:
            manager.add_search_path(path)
            print(f"Added search path: {path}")
        except Exception as e:
            print(f"Could not add search path {path}: {e}")
    
    print(f"Updated search paths: {manager.search_paths()}")
    
    # Try to discover plugins
    try:
        discovered = manager.discover_plugins(".", recursive=True)
        print(f"Discovered plugins: {discovered}")
    except Exception as e:
        print(f"Plugin discovery failed: {e}")
    
    return manager

def demonstrate_plugin_metadata() -> None:
    """Demonstrate plugin metadata creation and manipulation."""
    print("\n" + "=" * 50)
    print("Plugin Metadata Demonstration")
    print("=" * 50)
    
    # Create version objects
    version1 = Version(1, 0, 0)
    version2 = Version(2, 1, 3)
    
    print(f"Version 1: {version1}")
    print(f"Version 2: {version2}")
    print(f"Version comparison: {version1} < {version2} = {version1 < version2}")
    
    # Create plugin metadata
    metadata = PluginMetadata()
    metadata.name = "ExamplePlugin"
    metadata.version = version1
    metadata.description = "An example plugin for demonstration"
    metadata.author = "QtForge Team"
    metadata.license = "MIT"
    metadata.dependencies = ["CorePlugin", "UtilsPlugin"]
    metadata.tags = ["example", "demo", "test"]
    
    print(f"Created metadata: {metadata}")
    print(f"Plugin name: {metadata.name}")
    print(f"Plugin version: {metadata.version}")
    print(f"Dependencies: {metadata.dependencies}")
    print(f"Tags: {metadata.tags}")

def demonstrate_plugin_capabilities() -> None:
    """Demonstrate plugin capabilities and priorities."""
    print("\n" + "=" * 50)
    print("Plugin Capabilities Demonstration")
    print("=" * 50)
    
    # Show all available capabilities
    capabilities = [
        PluginCapability.UI,
        PluginCapability.Service,
        PluginCapability.Network,
        PluginCapability.DataProcessing,
        PluginCapability.Scripting,
        PluginCapability.FileSystem,
        PluginCapability.Database,
        PluginCapability.AsyncInit,
        PluginCapability.HotReload,
        PluginCapability.Configuration,
        PluginCapability.Logging,
        PluginCapability.Security,
        PluginCapability.Threading,
        PluginCapability.Monitoring
    ]
    
    print("Available plugin capabilities:")
    for cap in capabilities:
        print(f"  - {cap}")
    
    # Show all available priorities
    priorities = [
        PluginPriority.Lowest,
        PluginPriority.Low,
        PluginPriority.Normal,
        PluginPriority.High,
        PluginPriority.Highest
    ]
    
    print("\nAvailable plugin priorities:")
    for priority in priorities:
        print(f"  - {priority}")
    
    # Show all available states
    states = [
        PluginState.Unloaded,
        PluginState.Loading,
        PluginState.Loaded,
        PluginState.Initializing,
        PluginState.Running,
        PluginState.Paused,
        PluginState.Stopping,
        PluginState.Stopped,
        PluginState.Error,
        PluginState.Reloading
    ]
    
    print("\nAvailable plugin states:")
    for state in states:
        print(f"  - {state}")

def demonstrate_plugin_load_options() -> None:
    """Demonstrate plugin loading options."""
    print("\n" + "=" * 50)
    print("Plugin Load Options Demonstration")
    print("=" * 50)
    
    # Create load options
    options = PluginLoadOptions()
    print(f"Default load options: {options}")
    
    # Configure options
    options.validate_signature = True
    options.check_dependencies = True
    options.initialize_immediately = False
    options.enable_hot_reload = True
    
    print("Configured load options:")
    print(f"  Validate signature: {options.validate_signature}")
    print(f"  Check dependencies: {options.check_dependencies}")
    print(f"  Initialize immediately: {options.initialize_immediately}")
    print(f"  Enable hot reload: {options.enable_hot_reload}")

def demonstrate_system_information() -> None:
    """Demonstrate system information retrieval."""
    print("\n" + "=" * 50)
    print("System Information Demonstration")
    print("=" * 50)
    
    # Get system status
    try:
        status = qtforge.get_system_status()
        print("System status:")
        for key, value in status.items():
            print(f"  {key}: {value}")
    except Exception as e:
        print(f"Could not get system status: {e}")
    
    # Get system info
    try:
        info = qtforge.get_system_info()
        print("\nSystem info:")
        for key, value in info.items():
            if isinstance(value, dict):
                print(f"  {key}:")
                for sub_key, sub_value in value.items():
                    print(f"    {sub_key}: {sub_value}")
            else:
                print(f"  {key}: {value}")
    except Exception as e:
        print(f"Could not get system info: {e}")

def main() -> None:
    """Main function demonstrating plugin management."""
    print("QtForge Python Bindings - Plugin Management Example")
    
    try:
        # Test basic connection
        print(f"Connection test: {qtforge.test_connection()}")
        
        # Demonstrate various aspects
        manager = demonstrate_plugin_manager()
        demonstrate_plugin_metadata()
        demonstrate_plugin_capabilities()
        demonstrate_plugin_load_options()
        demonstrate_system_information()
        
        print("\n" + "=" * 50)
        print("Plugin management example completed successfully!")
        print("=" * 50)
        
    except Exception as e:
        print(f"Error during demonstration: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
