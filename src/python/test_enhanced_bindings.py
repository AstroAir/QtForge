#!/usr/bin/env python3
"""
Test script for enhanced QtForge Python bindings
Tests the newly added functionality and interfaces
"""

import sys
import os

def test_enhanced_bindings():
    """Test the enhanced Python bindings"""
    print("Testing Enhanced QtForge Python Bindings")
    print("=" * 50)
    
    try:
        # Test basic import
        print("1. Testing basic import...")
        import qtforge
        print(f"   ✓ QtForge version: {qtforge.version()}")
        print(f"   ✓ Version info: {qtforge.version_info()}")
        
        # Test core module enhancements
        print("\n2. Testing core module enhancements...")
        from qtforge import core
        
        # Test new enums
        print("   Testing new enums...")
        if hasattr(core, 'PluginType'):
            print(f"   ✓ PluginType.Native: {core.PluginType.Native}")
            print(f"   ✓ PluginType.Python: {core.PluginType.Python}")
        
        # Test new structures
        if hasattr(core, 'InterfaceCapability'):
            print("   ✓ InterfaceCapability class available")
            cap = core.InterfaceCapability()
            print(f"   ✓ Created InterfaceCapability: {cap}")
        
        if hasattr(core, 'InterfaceDescriptor'):
            print("   ✓ InterfaceDescriptor class available")
            desc = core.InterfaceDescriptor()
            print(f"   ✓ Created InterfaceDescriptor: {desc}")
        
        # Test advanced plugin interfaces
        if hasattr(core, 'IAdvancedPlugin'):
            print("   ✓ IAdvancedPlugin interface available")
        
        if hasattr(core, 'IDynamicPlugin'):
            print("   ✓ IDynamicPlugin interface available")
        
        # Test security module enhancements
        print("\n3. Testing security module enhancements...")
        from qtforge import security
        
        # Test security enums
        print("   Testing security enums...")
        if hasattr(security, 'SecurityLevel'):
            print(f"   ✓ SecurityLevel.None: {security.SecurityLevel.None}")
            print(f"   ✓ SecurityLevel.Basic: {security.SecurityLevel.Basic}")
            print(f"   ✓ SecurityLevel.Maximum: {security.SecurityLevel.Maximum}")
        
        # Test security validation result
        if hasattr(security, 'SecurityValidationResult'):
            print("   ✓ SecurityValidationResult class available")
            result = security.SecurityValidationResult()
            print(f"   ✓ Created SecurityValidationResult: {result}")
            print(f"   ✓ Result passed: {result.passed()}")
        
        # Test security manager
        if hasattr(security, 'SecurityManager'):
            print("   ✓ SecurityManager class available")
            if hasattr(security, 'create_security_manager'):
                manager = security.create_security_manager()
                print(f"   ✓ Created SecurityManager: {manager}")
                print(f"   ✓ Security level: {manager.security_level()}")
        
        # Test convenience functions
        print("\n4. Testing convenience functions...")
        if hasattr(qtforge, 'test_function'):
            result = qtforge.test_function()
            print(f"   ✓ test_function(): {result}")
        
        if hasattr(qtforge, 'create_plugin_manager'):
            manager = qtforge.create_plugin_manager()
            print(f"   ✓ create_plugin_manager(): {manager}")
        
        if hasattr(qtforge, 'utils_test'):
            result = qtforge.utils_test()
            print(f"   ✓ utils_test(): {result}")
        
        # Test enabled modules
        print("\n5. Testing enabled modules...")
        modules_to_test = [
            'core', 'utils', 'security', 'communication', 'managers',
            'orchestration', 'monitoring', 'transactions', 'composition',
            'marketplace', 'threading'
        ]
        
        for module_name in modules_to_test:
            if hasattr(qtforge, module_name):
                module = getattr(qtforge, module_name)
                print(f"   ✓ {module_name} module available: {module}")
            else:
                print(f"   ✗ {module_name} module not available")
        
        print("\n" + "=" * 50)
        print("✓ Enhanced bindings test completed successfully!")
        return True
        
    except ImportError as e:
        print(f"✗ Import error: {e}")
        return False
    except Exception as e:
        print(f"✗ Test error: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_version_compatibility():
    """Test version compatibility"""
    print("\n6. Testing version compatibility...")
    try:
        import qtforge
        from qtforge import core
        
        # Test Version class
        if hasattr(core, 'Version'):
            v1 = core.Version(1, 2, 3)
            v2 = core.Version(1, 2, 4)
            print(f"   ✓ Version v1: {v1}")
            print(f"   ✓ Version v2: {v2}")
            print(f"   ✓ v1 < v2: {v1 < v2}")
            print(f"   ✓ v1.is_valid(): {v1.is_valid()}")
        
        return True
    except Exception as e:
        print(f"   ✗ Version test error: {e}")
        return False

if __name__ == "__main__":
    success = test_enhanced_bindings()
    success &= test_version_compatibility()
    
    if success:
        print("\n🎉 All tests passed!")
        sys.exit(0)
    else:
        print("\n❌ Some tests failed!")
        sys.exit(1)
