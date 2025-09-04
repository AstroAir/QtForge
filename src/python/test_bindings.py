#!/usr/bin/env python3
"""
Comprehensive test script for QtForge Python bindings.

This script tests all the enhanced Python bindings to ensure they work correctly.
"""

import sys
import traceback
from typing import Any, Dict, List

def test_imports():
    """Test that all modules can be imported."""
    print("Testing imports...")
    
    try:
        import qtforge
        print("✓ qtforge imported successfully")
        
        # Test submodules
        from qtforge import core, utils, security, communication, managers
        print("✓ All submodules imported successfully")
        
        # Test version info
        print(f"✓ QtForge version: {qtforge.version()}")
        print(f"✓ Version info: {qtforge.version_info()}")
        
        return True
    except ImportError as e:
        print(f"✗ Import failed: {e}")
        return False

def test_core_enums():
    """Test core enumeration types."""
    print("\nTesting core enums...")
    
    try:
        from qtforge.core import PluginState, PluginCapability, PluginPriority, MessagePriority, DeliveryMode
        
        # Test PluginState
        assert PluginState.Unloaded != PluginState.Running
        print("✓ PluginState enum works")
        
        # Test PluginCapability
        caps = PluginCapability.Service | PluginCapability.Network
        assert caps != PluginCapability.None
        print("✓ PluginCapability enum and bitwise operations work")
        
        # Test PluginPriority
        assert PluginPriority.High > PluginPriority.Low
        print("✓ PluginPriority enum works")
        
        # Test MessagePriority
        assert MessagePriority.Critical > MessagePriority.Normal
        print("✓ MessagePriority enum works")
        
        # Test DeliveryMode
        assert DeliveryMode.Broadcast != DeliveryMode.Unicast
        print("✓ DeliveryMode enum works")
        
        return True
    except Exception as e:
        print(f"✗ Core enums test failed: {e}")
        traceback.print_exc()
        return False

def test_version_class():
    """Test Version class functionality."""
    print("\nTesting Version class...")
    
    try:
        from qtforge.core import Version, make_version
        
        # Test construction
        v1 = Version(1, 2, 3)
        assert v1.major == 1 and v1.minor == 2 and v1.patch == 3
        print("✓ Version construction works")
        
        # Test string representation
        version_str = str(v1)
        assert "1.2.3" in version_str
        print(f"✓ Version string representation: {version_str}")
        
        # Test comparison
        v2 = Version(1, 2, 4)
        assert v2 > v1
        print("✓ Version comparison works")
        
        # Test factory function
        v3 = make_version(2, 0, 0)
        assert v3.major == 2
        print("✓ make_version factory function works")
        
        # Test parsing (if available)
        try:
            v4 = Version.parse("1.0.0")
            print("✓ Version parsing works")
        except:
            print("! Version parsing not available (expected)")
        
        return True
    except Exception as e:
        print(f"✗ Version class test failed: {e}")
        traceback.print_exc()
        return False

def test_plugin_metadata():
    """Test PluginMetadata structure."""
    print("\nTesting PluginMetadata...")
    
    try:
        from qtforge.core import PluginMetadata, Version, PluginCapability, PluginPriority
        
        # Test construction
        metadata = PluginMetadata()
        metadata.name = "Test Plugin"
        metadata.version = Version(1, 0, 0)
        metadata.description = "A test plugin"
        metadata.author = "Test Author"
        metadata.capabilities = PluginCapability.Service | PluginCapability.Network
        metadata.priority = PluginPriority.High
        
        assert metadata.name == "Test Plugin"
        assert metadata.version.major == 1
        print("✓ PluginMetadata construction and assignment works")
        
        # Test string representation
        repr_str = repr(metadata)
        assert "Test Plugin" in repr_str
        print(f"✓ PluginMetadata representation: {repr_str}")
        
        return True
    except Exception as e:
        print(f"✗ PluginMetadata test failed: {e}")
        traceback.print_exc()
        return False

def test_error_handling():
    """Test error handling types."""
    print("\nTesting error handling...")
    
    try:
        from qtforge.core import make_success, make_error, PluginErrorCode
        
        # Test success result
        success = make_success()
        print("✓ make_success works")
        
        # Test error result
        error = make_error(PluginErrorCode.FileNotFound, "Test error message")
        print("✓ make_error works")
        
        # Test error codes
        assert PluginErrorCode.Success != PluginErrorCode.FileNotFound
        print("✓ PluginErrorCode enum works")
        
        return True
    except Exception as e:
        print(f"✗ Error handling test failed: {e}")
        traceback.print_exc()
        return False

def test_plugin_manager():
    """Test PluginManager functionality."""
    print("\nTesting PluginManager...")
    
    try:
        from qtforge.core import create_plugin_manager, PluginLoadOptions
        
        # Test creation
        manager = create_plugin_manager()
        assert manager is not None
        print("✓ PluginManager creation works")
        
        # Test load options
        options = PluginLoadOptions()
        options.validate_signature = True
        options.check_dependencies = True
        print("✓ PluginLoadOptions works")
        
        # Test basic methods (without actual plugins)
        plugins = manager.loaded_plugins()
        assert isinstance(plugins, list)
        print("✓ loaded_plugins() works")
        
        search_paths = manager.search_paths()
        assert isinstance(search_paths, list)
        print("✓ search_paths() works")
        
        return True
    except Exception as e:
        print(f"✗ PluginManager test failed: {e}")
        traceback.print_exc()
        return False

def test_communication():
    """Test communication system."""
    print("\nTesting communication system...")
    
    try:
        from qtforge.communication import create_message_bus
        
        # Test MessageBus creation
        bus = create_message_bus()
        assert bus is not None
        print("✓ MessageBus creation works")
        
        # Test basic methods
        stats = bus.statistics()
        print("✓ MessageBus statistics() works")
        
        logging_enabled = bus.is_logging_enabled()
        print(f"✓ MessageBus logging status: {logging_enabled}")
        
        return True
    except Exception as e:
        print(f"✗ Communication test failed: {e}")
        traceback.print_exc()
        return False

def test_configuration():
    """Test configuration management."""
    print("\nTesting configuration management...")
    
    try:
        from qtforge.managers import create_configuration_manager, ConfigurationScope
        
        # Test ConfigurationManager creation
        config_mgr = create_configuration_manager()
        assert config_mgr is not None
        print("✓ ConfigurationManager creation works")
        
        # Test configuration scopes
        assert ConfigurationScope.Global != ConfigurationScope.Plugin
        print("✓ ConfigurationScope enum works")
        
        return True
    except Exception as e:
        print(f"✗ Configuration test failed: {e}")
        traceback.print_exc()
        return False

def run_all_tests():
    """Run all tests and report results."""
    print("=" * 60)
    print("QtForge Python Bindings Test Suite")
    print("=" * 60)
    
    tests = [
        test_imports,
        test_core_enums,
        test_version_class,
        test_plugin_metadata,
        test_error_handling,
        test_plugin_manager,
        test_communication,
        test_configuration,
    ]
    
    passed = 0
    failed = 0
    
    for test in tests:
        try:
            if test():
                passed += 1
            else:
                failed += 1
        except Exception as e:
            print(f"✗ Test {test.__name__} crashed: {e}")
            failed += 1
    
    print("\n" + "=" * 60)
    print(f"Test Results: {passed} passed, {failed} failed")
    print("=" * 60)
    
    return failed == 0

if __name__ == "__main__":
    success = run_all_tests()
    sys.exit(0 if success else 1)
