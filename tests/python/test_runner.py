#!/usr/bin/env python3
"""
Comprehensive test runner for QtForge Python bindings.
Runs all test modules and provides detailed reporting.
"""

import sys
import os
import pytest
import time
from pathlib import Path

# Add the build directory to Python path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../build'))

def check_bindings_availability() -> None:
    """Check if QtForge bindings are available."""
    try:
        import qtforge
        return True, None
    except ImportError as e:
        return False, str(e)

def run_comprehensive_tests() -> None:
    """Run all comprehensive tests."""
    print("QtForge Python Bindings - Comprehensive Test Suite")
    print("=" * 60)
    
    # Check if bindings are available
    available, error = check_bindings_availability()
    if not available:
        print(f"❌ QtForge bindings not available: {error}")
        print("\nTo run tests, ensure QtForge is built with Python bindings enabled:")
        print("  cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON ..")
        print("  make")
        return 1
    
    print("✅ QtForge bindings are available")
    print()
    
    # Define test modules
    test_modules = [
        "test_comprehensive_core.py",
        "test_comprehensive_utils.py", 
        "test_comprehensive_communication.py",
        "test_comprehensive_security.py",
        "test_comprehensive_managers.py",
        "test_comprehensive_orchestration.py",
        "test_comprehensive_monitoring.py",
        "test_comprehensive_remaining_modules.py"
    ]
    
    # Get the directory containing this script
    test_dir = Path(__file__).parent
    
    # Check which test files exist
    existing_tests = []
    missing_tests = []
    
    for test_module in test_modules:
        test_path = test_dir / test_module
        if test_path.exists():
            existing_tests.append(str(test_path))
        else:
            missing_tests.append(test_module)
    
    if missing_tests:
        print("⚠️  Some test modules are missing:")
        for missing in missing_tests:
            print(f"   - {missing}")
        print()
    
    if not existing_tests:
        print("❌ No test modules found!")
        return 1
    
    print(f"🧪 Running {len(existing_tests)} test modules...")
    print()
    
    # Configure pytest arguments
    pytest_args = [
        "-v",  # Verbose output
        "--tb=short",  # Short traceback format
        "--strict-markers",  # Strict marker checking
        "-x",  # Stop on first failure (optional)
        "--durations=10",  # Show 10 slowest tests
    ] + existing_tests
    
    # Run the tests
    start_time = time.time()
    exit_code = pytest.main(pytest_args)
    end_time = time.time()
    
    # Print summary
    print()
    print("=" * 60)
    print(f"Test execution completed in {end_time - start_time:.2f} seconds")
    
    if exit_code == 0:
        print("✅ All tests passed!")
    else:
        print("❌ Some tests failed!")
    
    return exit_code

def run_specific_module(module_name) -> None:
    """Run tests for a specific module."""
    test_dir = Path(__file__).parent
    test_path = test_dir / f"test_comprehensive_{module_name}.py"
    
    if not test_path.exists():
        print(f"❌ Test module not found: {test_path}")
        return 1
    
    print(f"🧪 Running tests for {module_name} module...")
    
    pytest_args = [
        "-v",
        "--tb=short",
        str(test_path)
    ]
    
    return pytest.main(pytest_args)

def list_available_modules() -> None:
    """List available test modules."""
    test_dir = Path(__file__).parent
    
    print("Available test modules:")
    print("-" * 30)
    
    modules = [
        ("core", "Core plugin system functionality"),
        ("utils", "Utility functions and helpers"),
        ("communication", "Message bus and communication"),
        ("security", "Security and validation"),
        ("managers", "Configuration, logging, and resource management"),
        ("orchestration", "Workflow and orchestration"),
        ("monitoring", "Hot reload and metrics collection"),
        ("remaining_modules", "Transactions, composition, marketplace, threading")
    ]
    
    for module, description in modules:
        test_path = test_dir / f"test_comprehensive_{module}.py"
        status = "✅" if test_path.exists() else "❌"
        print(f"{status} {module:<20} - {description}")

def main() -> None:
    """Main entry point."""
    if len(sys.argv) == 1:
        # Run all tests
        return run_comprehensive_tests()
    
    elif len(sys.argv) == 2:
        arg = sys.argv[1]
        
        if arg in ["-h", "--help"]:
            print("QtForge Python Bindings Test Runner")
            print()
            print("Usage:")
            print("  python test_runner.py                    # Run all tests")
            print("  python test_runner.py <module>           # Run specific module tests")
            print("  python test_runner.py --list             # List available modules")
            print("  python test_runner.py --help             # Show this help")
            print()
            print("Examples:")
            print("  python test_runner.py core               # Test core module")
            print("  python test_runner.py security           # Test security module")
            return 0
        
        elif arg == "--list":
            list_available_modules()
            return 0
        
        else:
            # Run specific module
            return run_specific_module(arg)
    
    else:
        print("❌ Too many arguments. Use --help for usage information.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
