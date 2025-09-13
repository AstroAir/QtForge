#!/usr/bin/env python3
"""
QtForge Python Bindings - Test Runner

This script runs all available tests for the QtForge Python bindings
and provides a comprehensive report of the results.
"""

import sys
import os
import subprocess
import time
from pathlib import Path
from typing import List, Dict, Any, Tuple

def find_qtforge_bindings() -> bool:
    """Try to find and import QtForge bindings."""
    try:
        import qtforge
        return True
    except ImportError:
        # Try to find the bindings in the build directory
        current_dir = Path(__file__).parent
        possible_paths = [
            current_dir.parent / "build" / "src" / "python",
            current_dir.parent / "build" / "Release" / "src" / "python",
            current_dir.parent / "build" / "Debug" / "src" / "python",
            current_dir.parent / "src" / "python",
        ]
        
        for path in possible_paths:
            if path.exists():
                sys.path.insert(0, str(path))
                try:
                    import qtforge
                    print(f"Found QtForge bindings at: {path}")
                    return True
                except ImportError:
                    continue
        
        return False

def run_basic_tests() -> Tuple[bool, str]:
    """Run basic functionality tests."""
    print("Running basic functionality tests...")
    
    try:
        import qtforge
        
        # Test basic connection
        result = qtforge.test_connection()
        if "QtForge" not in result:
            return False, "Connection test failed"
        
        # Test version info
        version = qtforge.get_version()
        if not version or not isinstance(version, str):
            return False, "Version test failed"
        
        # Test module listing
        modules = qtforge.list_available_modules()
        if not isinstance(modules, list) or "core" not in modules:
            return False, "Module listing test failed"
        
        return True, "All basic tests passed"
        
    except Exception as e:
        return False, f"Basic tests failed: {e}"

def run_unittest_suite() -> Tuple[bool, str]:
    """Run the comprehensive unittest suite."""
    print("Running comprehensive unittest suite...")
    
    try:
        # Import and run the test suite
        from test_qtforge_bindings import run_tests
        
        result = run_tests()
        
        if result.wasSuccessful():
            return True, f"All {result.testsRun} tests passed"
        else:
            failures = len(result.failures)
            errors = len(result.errors)
            return False, f"{failures} failures, {errors} errors out of {result.testsRun} tests"
            
    except Exception as e:
        return False, f"Unittest suite failed: {e}"

def run_example_tests() -> Tuple[bool, str]:
    """Run example scripts to verify they work."""
    print("Running example scripts...")
    
    examples_dir = Path(__file__).parent.parent / "examples"
    example_files = [
        "basic_usage.py",
        "plugin_management.py"
    ]
    
    results = []
    
    for example_file in example_files:
        example_path = examples_dir / example_file
        if not example_path.exists():
            results.append(f"❌ {example_file}: File not found")
            continue
        
        try:
            # Run the example script
            result = subprocess.run(
                [sys.executable, str(example_path)],
                capture_output=True,
                text=True,
                timeout=30
            )
            
            if result.returncode == 0:
                results.append(f"✅ {example_file}: Success")
            else:
                results.append(f"❌ {example_file}: Failed with code {result.returncode}")
                if result.stderr:
                    results.append(f"   Error: {result.stderr[:200]}...")
                    
        except subprocess.TimeoutExpired:
            results.append(f"❌ {example_file}: Timeout")
        except Exception as e:
            results.append(f"❌ {example_file}: Exception - {e}")
    
    success_count = sum(1 for r in results if r.startswith("✅"))
    total_count = len(example_files)
    
    success = success_count == total_count
    message = f"{success_count}/{total_count} examples passed\n" + "\n".join(results)
    
    return success, message

def run_module_tests() -> Tuple[bool, str]:
    """Test individual module functionality."""
    print("Running module-specific tests...")
    
    try:
        import qtforge
        
        results = []
        available_modules = qtforge.list_available_modules()
        
        # Test core module
        if "core" in available_modules:
            try:
                from qtforge.core import PluginManager, PluginState, Version
                manager = PluginManager()
                version = Version(1, 0, 0)
                results.append("✅ Core module: Success")
            except Exception as e:
                results.append(f"❌ Core module: {e}")
        
        # Test utils module
        if "utils" in available_modules:
            try:
                from qtforge import utils
                test_result = utils.test_utils()
                results.append("✅ Utils module: Success")
            except Exception as e:
                results.append(f"❌ Utils module: {e}")
        
        # Test optional modules
        optional_modules = [
            'communication', 'security', 'managers', 'orchestration',
            'monitoring', 'threading', 'transactions', 'composition', 'marketplace'
        ]
        
        for module_name in optional_modules:
            if module_name in available_modules:
                try:
                    module = getattr(qtforge, module_name)
                    test_func = getattr(module, f'test_{module_name}')
                    test_result = test_func()
                    results.append(f"✅ {module_name.title()} module: Success")
                except Exception as e:
                    results.append(f"❌ {module_name.title()} module: {e}")
        
        success_count = sum(1 for r in results if r.startswith("✅"))
        total_count = len(results)
        
        success = success_count == total_count
        message = f"{success_count}/{total_count} modules passed\n" + "\n".join(results)
        
        return success, message
        
    except Exception as e:
        return False, f"Module tests failed: {e}"

def print_system_info() -> None:
    """Print system information."""
    print("\n" + "=" * 60)
    print("SYSTEM INFORMATION")
    print("=" * 60)
    
    print(f"Python Version: {sys.version}")
    print(f"Python Executable: {sys.executable}")
    print(f"Platform: {sys.platform}")
    
    try:
        import qtforge
        print(f"QtForge Version: {qtforge.get_version()}")
        print(f"Available Modules: {', '.join(qtforge.list_available_modules())}")
        
        build_info = qtforge.get_build_info()
        print("Build Information:")
        for key, value in build_info.items():
            if isinstance(value, dict):
                print(f"  {key}:")
                for sub_key, sub_value in value.items():
                    print(f"    {sub_key}: {sub_value}")
            else:
                print(f"  {key}: {value}")
                
    except Exception as e:
        print(f"Could not get QtForge information: {e}")

def main() -> None:
    """Main test runner function."""
    print("QtForge Python Bindings - Comprehensive Test Runner")
    print("=" * 60)
    
    start_time = time.time()
    
    # Check if QtForge bindings are available
    if not find_qtforge_bindings():
        print("❌ FATAL: Could not find QtForge Python bindings!")
        print("\nPlease ensure:")
        print("1. QtForge is built with Python bindings enabled")
        print("2. The Python bindings are installed or in the Python path")
        print("3. All dependencies (Qt6, pybind11) are available")
        return 1
    
    print("✅ QtForge Python bindings found and importable")
    
    # Print system information
    print_system_info()
    
    # Run all test suites
    test_suites = [
        ("Basic Functionality", run_basic_tests),
        ("Unittest Suite", run_unittest_suite),
        ("Example Scripts", run_example_tests),
        ("Module Tests", run_module_tests),
    ]
    
    results = []
    
    print("\n" + "=" * 60)
    print("RUNNING TEST SUITES")
    print("=" * 60)
    
    for suite_name, test_func in test_suites:
        print(f"\n--- {suite_name} ---")
        try:
            success, message = test_func()
            results.append((suite_name, success, message))
            
            if success:
                print(f"✅ {suite_name}: PASSED")
            else:
                print(f"❌ {suite_name}: FAILED")
            
            print(f"   {message}")
            
        except Exception as e:
            results.append((suite_name, False, str(e)))
            print(f"❌ {suite_name}: EXCEPTION - {e}")
    
    # Print final results
    end_time = time.time()
    duration = end_time - start_time
    
    print("\n" + "=" * 60)
    print("FINAL RESULTS")
    print("=" * 60)
    
    passed_count = sum(1 for _, success, _ in results if success)
    total_count = len(results)
    
    for suite_name, success, message in results:
        status = "✅ PASSED" if success else "❌ FAILED"
        print(f"{suite_name}: {status}")
    
    print(f"\nOverall: {passed_count}/{total_count} test suites passed")
    print(f"Duration: {duration:.2f} seconds")
    
    if passed_count == total_count:
        print("\n🎉 ALL TESTS PASSED! QtForge Python bindings are working correctly.")
        return 0
    else:
        print(f"\n⚠️  {total_count - passed_count} test suite(s) failed. Please check the output above.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
