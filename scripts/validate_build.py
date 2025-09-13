#!/usr/bin/env python3
"""
QtForge Build Validation Script

This script validates the QtForge build system and Python bindings.
It checks for successful compilation, proper module loading, and basic functionality.

Usage:
    python scripts/validate_build.py [--verbose] [--module MODULE_NAME]
"""

import sys
import os
import subprocess
import argparse
from pathlib import Path

# Add build directory to Python path
build_dir = Path(__file__).parent.parent / "build"
sys.path.insert(0, str(build_dir))

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    BOLD = '\033[1m'
    END = '\033[0m'

def print_status(message, status="info") -> None:
    """Print colored status messages"""
    colors = {
        "success": Colors.GREEN + "✅ ",
        "error": Colors.RED + "❌ ",
        "warning": Colors.YELLOW + "⚠️  ",
        "info": Colors.BLUE + "ℹ️  "
    }
    print(f"{colors.get(status, '')}{message}{Colors.END}")

def check_build_files() -> None:
    """Check if required build files exist"""
    print_status("Checking build files...", "info")

    required_files = [
        "libqtforge-core.dll",
        "libqtforge-security.dll",
        "qtforge.cp312-mingw_x86_64_msvcrt_gnu.pyd"
    ]

    missing_files = []
    for file in required_files:
        if not (build_dir / file).exists():
            missing_files.append(file)

    if missing_files:
        print_status(f"Missing build files: {', '.join(missing_files)}", "error")
        return False
    else:
        print_status("All required build files present", "success")
        return True

def test_python_import() -> None:
    """Test basic Python import"""
    print_status("Testing Python import...", "info")

    try:
        import qtforge
        print_status("QtForge Python module imported successfully", "success")
        return True
    except ImportError as e:
        print_status(f"Failed to import QtForge: {e}", "error")
        return False

def test_basic_functionality() -> None:
    """Test basic QtForge functionality"""
    print_status("Testing basic functionality...", "info")

    try:
        import qtforge

        # Test connection
        result = qtforge.test_connection()
        expected = "Hello from QtForge! Complete plugin system ready."

        if result == expected:
            print_status("Basic functionality test passed", "success")
            return True
        else:
            print_status(f"Unexpected result: {result}", "error")
            return False

    except Exception as e:
        print_status(f"Functionality test failed: {e}", "error")
        return False

def test_module_availability() -> None:
    """Test availability of Python modules"""
    print_status("Testing module availability...", "info")

    expected_modules = {
        'core': 'Core plugin system components',
        'utils': 'Utility classes and functions',
        'security': 'Security and validation components',
        'managers': 'Configuration, logging, and resource management',
        'orchestration': 'Plugin orchestration and workflow management'
    }

    try:
        import qtforge
        available_modules = [attr for attr in dir(qtforge) if not attr.startswith('_')]

        results = {}
        for module, description in expected_modules.items():
            if module in available_modules:
                print_status(f"{module}: Available - {description}", "success")
                results[module] = True
            else:
                print_status(f"{module}: Missing", "error")
                results[module] = False

        # Check for additional modules
        extra_modules = set(available_modules) - set(expected_modules.keys()) - {
            'test_connection', 'get_version', 'get_build_info', 'get_system_info',
            'get_version_info', 'get_help', 'list_available_modules',
            'create_plugin_manager', 'create_version', 'create_metadata'
        }

        if extra_modules:
            print_status(f"Additional modules found: {', '.join(extra_modules)}", "info")

        success_count = sum(results.values())
        total_count = len(expected_modules)
        print_status(f"Module availability: {success_count}/{total_count} modules working", "info")

        return success_count == total_count

    except Exception as e:
        print_status(f"Module availability test failed: {e}", "error")
        return False

def test_specific_module(module_name) -> None:
    """Test a specific module in detail"""
    print_status(f"Testing module: {module_name}", "info")

    try:
        import qtforge

        if not hasattr(qtforge, module_name):
            print_status(f"Module {module_name} not available", "error")
            return False

        module = getattr(qtforge, module_name)
        module_attrs = [attr for attr in dir(module) if not attr.startswith('_')]

        print_status(f"Module {module_name} attributes: {', '.join(module_attrs)}", "info")

        # Try to access some common attributes
        if hasattr(module, '__doc__'):
            print_status(f"Module documentation: {module.__doc__}", "info")

        print_status(f"Module {module_name} test completed", "success")
        return True

    except Exception as e:
        print_status(f"Module {module_name} test failed: {e}", "error")
        return False

def check_cmake_configuration() -> None:
    """Check CMake configuration status"""
    print_status("Checking CMake configuration...", "info")

    try:
        result = subprocess.run([
            "cmake", "-LA", "build"
        ], capture_output=True, text=True, encoding='utf-8', errors='ignore',
        cwd=Path(__file__).parent.parent)

        if result.returncode != 0:
            print_status("Failed to read CMake configuration", "error")
            return False

        # Check for Python module flags
        python_modules = [
            "QTFORGE_PYTHON_ENABLE_SECURITY_MODULE",
            "QTFORGE_PYTHON_ENABLE_MANAGERS_MODULE",
            "QTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE",
            "QTFORGE_PYTHON_ENABLE_COMMUNICATION_MODULE",
            "QTFORGE_PYTHON_ENABLE_MONITORING_MODULE",
            "QTFORGE_PYTHON_ENABLE_THREADING_MODULE",
            "QTFORGE_PYTHON_ENABLE_TRANSACTIONS_MODULE",
            "QTFORGE_PYTHON_ENABLE_COMPOSITION_MODULE",
            "QTFORGE_PYTHON_ENABLE_MARKETPLACE_MODULE"
        ]

        enabled_count = 0
        for module in python_modules:
            if f"{module}:BOOL=ON" in result.stdout:
                enabled_count += 1

        print_status(f"Python modules enabled: {enabled_count}/{len(python_modules)}", "info")
        return True

    except Exception as e:
        print_status(f"CMake configuration check failed: {e}", "error")
        return False

def run_comprehensive_validation(verbose=False, specific_module=None) -> None:
    """Run comprehensive build validation"""
    print_status("QtForge Build Validation", "info")
    print_status("=" * 50, "info")

    tests = [
        ("Build Files", check_build_files),
        ("CMake Configuration", check_cmake_configuration),
        ("Python Import", test_python_import),
        ("Basic Functionality", test_basic_functionality),
        ("Module Availability", test_module_availability)
    ]

    if specific_module:
        tests.append((f"Specific Module ({specific_module})", lambda: test_specific_module(specific_module)))

    results = {}
    for test_name, test_func in tests:
        print_status(f"\n--- {test_name} ---", "info")
        try:
            results[test_name] = test_func()
        except Exception as e:
            print_status(f"Test {test_name} crashed: {e}", "error")
            results[test_name] = False

    # Summary
    print_status("\n" + "=" * 50, "info")
    print_status("VALIDATION SUMMARY", "info")
    print_status("=" * 50, "info")

    passed = sum(results.values())
    total = len(results)

    for test_name, result in results.items():
        status = "success" if result else "error"
        print_status(f"{test_name}: {'PASS' if result else 'FAIL'}", status)

    print_status(f"\nOverall Result: {passed}/{total} tests passed",
                "success" if passed == total else "warning")

    if passed == total:
        print_status("🎉 QtForge build system is fully functional!", "success")
    else:
        print_status("⚠️  Some issues found. Check BUILD_OPTIMIZATION_REPORT.md for details.", "warning")

    return passed == total

def main() -> None:
    parser = argparse.ArgumentParser(description="Validate QtForge build system")
    parser.add_argument("--verbose", "-v", action="store_true", help="Verbose output")
    parser.add_argument("--module", "-m", help="Test specific module")

    args = parser.parse_args()

    success = run_comprehensive_validation(args.verbose, args.module)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
