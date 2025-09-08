#!/usr/bin/env python3
"""
QtForge Python Bindings - Basic Usage Example

This example demonstrates the basic functionality of QtForge Python bindings,
including plugin management, loading, and basic operations.
"""

import sys
import os
from pathlib import Path

# Add the QtForge Python bindings to the path if needed
# In a real installation, this wouldn't be necessary
try:
    import qtforge
except ImportError:
    # Try to find the bindings in the build directory
    build_dir = Path(__file__).parent.parent / "build" / "src" / "python"
    if build_dir.exists():
        sys.path.insert(0, str(build_dir))
    import qtforge

def main():
    """Main example function demonstrating QtForge basic usage."""
    
    print("=" * 60)
    print("QtForge Python Bindings - Basic Usage Example")
    print("=" * 60)
    
    # Test the connection
    print("\n1. Testing Connection:")
    print(f"   {qtforge.test_connection()}")
    
    # Get version information
    print("\n2. Version Information:")
    print(f"   QtForge Version: {qtforge.get_version()}")
    print(f"   Version Info: {qtforge.get_version_info()}")
    
    # List available modules
    print("\n3. Available Modules:")
    modules = qtforge.list_available_modules()
    for module in modules:
        print(f"   - {module}")
    
    # Get build information
    print("\n4. Build Information:")
    build_info = qtforge.get_build_info()
    for key, value in build_info.items():
        if isinstance(value, dict):
            print(f"   {key}:")
            for sub_key, sub_value in value.items():
                print(f"     {sub_key}: {sub_value}")
        else:
            print(f"   {key}: {value}")
    
    # Create a plugin manager
    print("\n5. Plugin Manager:")
    try:
        manager = qtforge.create_plugin_manager()
        print(f"   Created: {manager}")
        print(f"   Loaded plugins: {manager.loaded_plugins()}")
        print(f"   Search paths: {manager.search_paths()}")
    except Exception as e:
        print(f"   Error creating plugin manager: {e}")
    
    # Test core module functionality
    print("\n6. Core Module:")
    try:
        from qtforge.core import PluginState, PluginCapability, PluginPriority
        print(f"   PluginState.Running: {PluginState.Running}")
        print(f"   PluginCapability.Service: {PluginCapability.Service}")
        print(f"   PluginPriority.High: {PluginPriority.High}")
        
        # Create version objects
        version = qtforge.create_version(1, 2, 3)
        print(f"   Created version: {version}")
        
        # Create metadata
        metadata = qtforge.create_metadata("TestPlugin", "A test plugin")
        print(f"   Created metadata: {metadata}")
        
    except Exception as e:
        print(f"   Error with core module: {e}")
    
    # Test utils module
    print("\n7. Utils Module:")
    try:
        from qtforge import utils
        test_result = utils.test_utils()
        print(f"   Utils test: {test_result}")
    except Exception as e:
        print(f"   Error with utils module: {e}")
    
    # Test other modules if available
    modules_to_test = [
        ('communication', 'Communication'),
        ('security', 'Security'),
        ('managers', 'Managers'),
        ('orchestration', 'Orchestration'),
        ('monitoring', 'Monitoring'),
        ('threading', 'Threading'),
        ('transactions', 'Transactions'),
        ('composition', 'Composition'),
        ('marketplace', 'Marketplace')
    ]
    
    for module_name, display_name in modules_to_test:
        print(f"\n8. {display_name} Module:")
        try:
            module = getattr(qtforge, module_name)
            test_func = getattr(module, f'test_{module_name}')
            result = test_func()
            print(f"   {display_name} test: {result}")
        except AttributeError:
            print(f"   {display_name} module not available or no test function")
        except Exception as e:
            print(f"   Error with {display_name} module: {e}")
    
    print("\n" + "=" * 60)
    print("Basic usage example completed!")
    print("=" * 60)

if __name__ == "__main__":
    main()
