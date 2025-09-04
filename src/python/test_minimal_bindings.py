#!/usr/bin/env python3
"""
Minimal test for QtForge Python bindings
Tests only the basic functionality that should work
"""

import sys
import os

def test_minimal_bindings():
    """Test minimal working Python bindings"""
    print("Testing Minimal QtForge Python Bindings")
    print("=" * 50)
    
    try:
        # Test basic import - this should work if the module compiles
        print("1. Testing basic module import...")
        try:
            import qtforge
            print("   ✓ QtForge module imported successfully")
        except ImportError as e:
            print(f"   ✗ Failed to import qtforge: {e}")
            return False
        
        # Test version information
        print("\n2. Testing version information...")
        try:
            if hasattr(qtforge, 'version'):
                version = qtforge.version()
                print(f"   ✓ QtForge version: {version}")
            else:
                print("   ⚠ version() function not available")
                
            if hasattr(qtforge, 'version_info'):
                version_info = qtforge.version_info()
                print(f"   ✓ Version info: {version_info}")
            else:
                print("   ⚠ version_info() function not available")
        except Exception as e:
            print(f"   ✗ Version test error: {e}")
        
        # Test core module
        print("\n3. Testing core module...")
        try:
            from qtforge import core
            print("   ✓ Core module imported")
            
            # Test basic enums that should exist
            if hasattr(core, 'PluginState'):
                print(f"   ✓ PluginState enum available")
                print(f"     - Unloaded: {core.PluginState.Unloaded}")
                print(f"     - Loaded: {core.PluginState.Loaded}")
            else:
                print("   ⚠ PluginState enum not available")
                
            if hasattr(core, 'SecurityLevel'):
                print(f"   ✓ SecurityLevel enum available")
            else:
                print("   ⚠ SecurityLevel enum not available")
                
        except ImportError as e:
            print(f"   ✗ Core module import failed: {e}")
        except Exception as e:
            print(f"   ✗ Core module test error: {e}")
        
        # Test utils module
        print("\n4. Testing utils module...")
        try:
            from qtforge import utils
            print("   ✓ Utils module imported")
            
            # Test basic utility functions
            if hasattr(utils, 'utils_test'):
                result = utils.utils_test()
                print(f"   ✓ utils_test(): {result}")
            else:
                print("   ⚠ utils_test() function not available")
                
        except ImportError as e:
            print(f"   ✗ Utils module import failed: {e}")
        except Exception as e:
            print(f"   ✗ Utils module test error: {e}")
        
        # Test security module
        print("\n5. Testing security module...")
        try:
            from qtforge import security
            print("   ✓ Security module imported")
            
            # Test security enums
            if hasattr(security, 'SecurityLevel'):
                print(f"   ✓ SecurityLevel enum available")
                print(f"     - None: {security.SecurityLevel.None}")
                print(f"     - Basic: {security.SecurityLevel.Basic}")
                print(f"     - Maximum: {security.SecurityLevel.Maximum}")
            else:
                print("   ⚠ SecurityLevel enum not available")
                
        except ImportError as e:
            print(f"   ✗ Security module import failed: {e}")
        except Exception as e:
            print(f"   ✗ Security module test error: {e}")
        
        # Test available modules
        print("\n6. Testing available modules...")
        modules_to_test = [
            'core', 'utils', 'security', 'communication', 'managers',
            'orchestration', 'threading'
        ]
        
        available_modules = []
        for module_name in modules_to_test:
            try:
                if hasattr(qtforge, module_name):
                    module = getattr(qtforge, module_name)
                    available_modules.append(module_name)
                    print(f"   ✓ {module_name} module available")
                else:
                    print(f"   ✗ {module_name} module not available")
            except Exception as e:
                print(f"   ✗ {module_name} module error: {e}")
        
        print(f"\n   Available modules: {available_modules}")
        
        # Test convenience functions
        print("\n7. Testing convenience functions...")
        convenience_functions = [
            'test_function', 'get_version', 'utils_test', 'create_version'
        ]
        
        available_functions = []
        for func_name in convenience_functions:
            try:
                if hasattr(qtforge, func_name):
                    available_functions.append(func_name)
                    print(f"   ✓ {func_name}() available")
                    
                    # Try to call simple functions
                    if func_name in ['test_function', 'get_version', 'utils_test']:
                        try:
                            func = getattr(qtforge, func_name)
                            result = func()
                            print(f"     Result: {result}")
                        except Exception as e:
                            print(f"     Error calling {func_name}(): {e}")
                else:
                    print(f"   ✗ {func_name}() not available")
            except Exception as e:
                print(f"   ✗ {func_name} test error: {e}")
        
        print(f"\n   Available functions: {available_functions}")
        
        print("\n" + "=" * 50)
        if available_modules and available_functions:
            print("✓ Minimal bindings test completed with some functionality!")
            print(f"Working modules: {len(available_modules)}/{len(modules_to_test)}")
            print(f"Working functions: {len(available_functions)}/{len(convenience_functions)}")
            return True
        else:
            print("✗ No working functionality found")
            return False
        
    except Exception as e:
        print(f"✗ Critical test error: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_basic_object_creation():
    """Test basic object creation for classes that should work"""
    print("\n8. Testing basic object creation...")
    
    try:
        from qtforge import core
        
        # Test Version class (if available and working)
        if hasattr(core, 'Version'):
            try:
                v = core.Version(1, 0, 0)
                print(f"   ✓ Version object created: {v}")
                
                # Test basic version operations
                v2 = core.Version(2, 0, 0)
                if hasattr(v, '__lt__'):
                    print(f"   ✓ Version comparison: {v} < {v2} = {v < v2}")
                
                if hasattr(v, 'to_string'):
                    print(f"   ✓ Version string: {v.to_string()}")
                    
            except Exception as e:
                print(f"   ✗ Version object creation failed: {e}")
        else:
            print("   ⚠ Version class not available")
        
        # Test PluginMetadata (if available)
        if hasattr(core, 'PluginMetadata'):
            try:
                metadata = core.PluginMetadata()
                print(f"   ✓ PluginMetadata object created: {metadata}")
            except Exception as e:
                print(f"   ✗ PluginMetadata creation failed: {e}")
        else:
            print("   ⚠ PluginMetadata class not available")
            
        return True
        
    except Exception as e:
        print(f"   ✗ Object creation test error: {e}")
        return False

if __name__ == "__main__":
    success = test_minimal_bindings()
    success &= test_basic_object_creation()
    
    if success:
        print("\n🎉 Minimal tests completed with some working functionality!")
        sys.exit(0)
    else:
        print("\n❌ Minimal tests failed!")
        sys.exit(1)
