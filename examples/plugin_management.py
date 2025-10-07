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
        PluginManager, PluginState, PluginCapability, PluginPriority
    )
    # Try to import optional classes that might not exist
    try:
        from qtforge.core import PluginLoadOptions, PluginInfo, PluginMetadata, Version  # type: ignore
    except ImportError:
        # Create dummy classes for missing types
        class PluginLoadOptions:  # type: ignore
            def __init__(self) -> None:
                self.validate_signature = False
                self.check_dependencies = False
                self.initialize_immediately = False
                self.enable_hot_reload = False

        class PluginInfo:  # type: ignore
            pass

        class PluginMetadata:  # type: ignore
            def __init__(self) -> None:
                self.name = ""
                self.version = None
                self.description = ""
                self.author = ""
                self.license = ""
                self.dependencies: List[str] = []
                self.tags: List[str] = []

        class Version:  # type: ignore
            def __init__(self, major: int, minor: int, patch: int) -> None:
                self.major = major
                self.minor = minor
                self.patch = patch

            def __str__(self) -> str:
                return f"{self.major}.{self.minor}.{self.patch}"

            def __lt__(self, other: 'Version') -> bool:
                return (self.major, self.minor, self.patch) < (other.major, other.minor, other.patch)

except ImportError:
    # Try to find the bindings in the build directory
    build_dir = Path(__file__).parent.parent / "build" / "src" / "python"
    if build_dir.exists():
        sys.path.insert(0, str(build_dir))
    import qtforge
    from qtforge.core import (
        PluginManager, PluginState, PluginCapability, PluginPriority
    )
    # Try to import optional classes that might not exist
    try:
        from qtforge.core import PluginLoadOptions, PluginInfo, PluginMetadata, Version  # type: ignore
    except ImportError:
        # Create dummy classes for missing types
        class PluginLoadOptions:  # type: ignore
            def __init__(self) -> None:
                self.validate_signature = False
                self.check_dependencies = False
                self.initialize_immediately = False
                self.enable_hot_reload = False

        class PluginInfo:  # type: ignore
            pass

        class PluginMetadata:  # type: ignore
            def __init__(self) -> None:
                self.name = ""
                self.version = None
                self.description = ""
                self.author = ""
                self.license = ""
                self.dependencies: List[str] = []
                self.tags: List[str] = []

        class Version:  # type: ignore
            def __init__(self, major: int, minor: int, patch: int) -> None:
                self.major = major
                self.minor = minor
                self.patch = patch

            def __str__(self) -> str:
                return f"{self.major}.{self.minor}.{self.patch}"

            def __lt__(self, other: 'Version') -> bool:
                return (self.major, self.minor, self.patch) < (other.major, other.minor, other.patch)

def demonstrate_plugin_manager() -> Any:
    """Demonstrate basic plugin manager functionality."""
    print("\n" + "=" * 50)
    print("Plugin Manager Demonstration")
    print("=" * 50)

    # Create a plugin manager
    manager = PluginManager()
    print(f"Created plugin manager: {manager}")

    # Get initial state
    try:
        loaded_plugins: Any = getattr(manager, 'loaded_plugins', lambda: [])()
        print(f"Initially loaded plugins: {loaded_plugins}")
    except Exception as e:
        print(f"Could not get loaded plugins: {e}")

    try:
        search_paths: Any = getattr(manager, 'search_paths', lambda: [])()
        print(f"Search paths: {search_paths}")
    except Exception as e:
        print(f"Could not get search paths: {e}")

    # Add some search paths (these might not exist, but that's okay for demo)
    demo_paths = ["./plugins", "../plugins", "/usr/local/lib/qtforge/plugins"]
    for path in demo_paths:
        try:
            add_search_path = getattr(manager, 'add_search_path', None)
            if add_search_path:
                add_search_path(path)
                print(f"Added search path: {path}")
            else:
                print(f"add_search_path method not available")
        except Exception as e:
            print(f"Could not add search path {path}: {e}")

    try:
        search_paths = getattr(manager, 'search_paths', lambda: [])()
        print(f"Updated search paths: {search_paths}")
    except Exception as e:
        print(f"Could not get updated search paths: {e}")

    # Try to discover plugins
    try:
        discover_plugins = getattr(manager, 'discover_plugins', None)
        if discover_plugins:
            discovered = discover_plugins(".", recursive=True)
            print(f"Discovered plugins: {discovered}")
        else:
            print("discover_plugins method not available")
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
    capability_names = [
        'UI', 'Service', 'Network', 'DataProcessing', 'Scripting',
        'FileSystem', 'Database', 'AsyncInit', 'HotReload', 'Configuration',
        'Logging', 'Security', 'Threading', 'Monitoring'
    ]

    capabilities = []
    for name in capability_names:
        cap = getattr(PluginCapability, name, None)
        if cap is not None:
            capabilities.append(cap)

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
    state_names = [
        'Unloaded', 'Loading', 'Loaded', 'Initializing', 'Running',
        'Paused', 'Stopping', 'Stopped', 'Error', 'Reloading'
    ]

    states = []
    for name in state_names:
        state = getattr(PluginState, name, None)
        if state is not None:
            states.append(state)

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
        status = qtforge.get_system_status()  # type: ignore
        print("System status:")
        for key, value in status.items():
            print(f"  {key}: {value}")
    except Exception as e:
        print(f"Could not get system status: {e}")

    # Get system info
    try:
        info = qtforge.get_system_info()  # type: ignore
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
        print(f"Connection test: {qtforge.test_connection()}")  # type: ignore

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
