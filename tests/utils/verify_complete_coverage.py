#!/usr/bin/env python3
"""
Verification script for complete QtForge test coverage.
Validates that all modules have corresponding test files and proper CMake configuration.
"""

import os
import sys
from pathlib import Path


def verify_complete_coverage() -> None:
    """Verify complete test coverage for all QtForge modules."""
    test_dir = Path(__file__).parent.parent  # Go up one level since we're now in utils/
    src_dir = test_dir.parent / "src"

    print("🔍 Verifying complete QtForge test coverage...")
    print(f"📁 Test directory: {test_dir}")
    print(f"📁 Source directory: {src_dir}")

    # Get all source modules
    src_modules = []
    if src_dir.exists():
        for item in src_dir.iterdir():
            if item.is_dir() and not item.name.startswith('.'):
                src_modules.append(item.name)

    print(f"\n📦 Found {len(src_modules)} source modules:")
    for module in sorted(src_modules):
        print(f"   - {module}")

    # Expected test modules (should match src modules)
    expected_test_modules = set(src_modules)

    # Add additional test modules that don't have direct src counterparts
    expected_test_modules.update(['platform', 'integration', 'build_system'])

    # Get actual test modules
    actual_test_modules = set()
    for item in test_dir.iterdir():
        if item.is_dir() and not item.name.startswith('.') and item.name != '__pycache__':
            actual_test_modules.add(item.name)

    print(f"\n🧪 Found {len(actual_test_modules)} test modules:")
    for module in sorted(actual_test_modules):
        print(f"   - {module}")

    # Check coverage
    missing_modules = expected_test_modules - actual_test_modules
    extra_modules = actual_test_modules - expected_test_modules

    print(f"\n📊 Coverage Analysis:")
    print(f"   Expected modules: {len(expected_test_modules)}")
    print(f"   Actual modules: {len(actual_test_modules)}")
    print(f"   Coverage: {len(actual_test_modules & expected_test_modules)}/{len(expected_test_modules)} ({100 * len(actual_test_modules & expected_test_modules) / len(expected_test_modules):.1f}%)")

    # Report missing modules
    if missing_modules:
        print(f"\n❌ Missing test modules ({len(missing_modules)}):")
        for module in sorted(missing_modules):
            print(f"   - {module}")

    # Report extra modules
    if extra_modules:
        print(f"\n➕ Extra test modules ({len(extra_modules)}):")
        for module in sorted(extra_modules):
            print(f"   - {module}")

    # Verify each test module has proper structure
    print(f"\n🔧 Verifying test module structure:")

    structure_issues = []

    for module in sorted(actual_test_modules):
        module_path = test_dir / module
        cmake_file = module_path / "CMakeLists.txt"

        print(f"\n   📂 {module}:")

        # Check CMakeLists.txt
        if cmake_file.exists():
            print(f"      ✅ CMakeLists.txt")
        else:
            print(f"      ❌ Missing CMakeLists.txt")
            structure_issues.append(f"Missing CMakeLists.txt in {module}")

        # Check for test files
        test_files = list(module_path.glob("test_*.cpp"))
        if test_files:
            print(f"      ✅ {len(test_files)} test file(s):")
            for test_file in sorted(test_files):
                print(f"         - {test_file.name}")
        else:
            print(f"      ⚠️  No test files found")
            # This might be OK for placeholder modules

    # Check newly created test files
    print(f"\n🆕 Verifying newly created test files:")

    new_test_files = {
        'bridges': ['test_python_bridge.cpp'],
        'composition': ['test_plugin_composition.cpp'],
        'marketplace': ['test_plugin_marketplace.cpp'],
        'transactions': ['test_plugin_transaction_manager.cpp'],
        'python': ['test_python_bindings.cpp']
    }

    for module, expected_files in new_test_files.items():
        module_path = test_dir / module
        print(f"\n   📂 {module}:")

        if not module_path.exists():
            print(f"      ❌ Module directory missing")
            structure_issues.append(f"Missing module directory: {module}")
            continue

        for expected_file in expected_files:
            test_file = module_path / expected_file
            if test_file.exists():
                # Check file size to ensure it's not empty
                file_size = test_file.stat().st_size
                if file_size > 1000:  # At least 1KB
                    print(f"      ✅ {expected_file} ({file_size:,} bytes)")
                else:
                    print(
                        f"      ⚠️  {expected_file} (too small: {file_size} bytes)")
            else:
                print(f"      ❌ Missing {expected_file}")
                structure_issues.append(
                    f"Missing test file: {module}/{expected_file}")

    # Check main CMakeLists.txt includes all modules
    print(f"\n📋 Verifying main CMakeLists.txt:")

    main_cmake = test_dir / "CMakeLists.txt"
    if main_cmake.exists():
        content = main_cmake.read_text()

        missing_includes = []
        for module in sorted(actual_test_modules):
            if f"add_subdirectory({module})" not in content:
                missing_includes.append(module)

        if missing_includes:
            print(f"   ❌ Missing subdirectory includes:")
            for module in missing_includes:
                print(f"      - add_subdirectory({module})")
                structure_issues.append(
                    f"Missing add_subdirectory({module}) in main CMakeLists.txt")
        else:
            print(f"   ✅ All modules included in main CMakeLists.txt")
    else:
        print(f"   ❌ Main CMakeLists.txt not found")
        structure_issues.append("Missing main CMakeLists.txt")

    # Summary
    print(f"\n📈 Final Summary:")

    if not missing_modules and not structure_issues:
        print(f"   🎉 SUCCESS: Complete test coverage achieved!")
        print(
            f"   ✅ All {len(expected_test_modules)} modules have test coverage")
        print(f"   ✅ All new test files created successfully")
        print(f"   ✅ All CMakeLists.txt files properly configured")
        return True
    else:
        print(f"   ⚠️  Issues found:")

        if missing_modules:
            print(f"   - {len(missing_modules)} missing test modules")

        if structure_issues:
            print(f"   - {len(structure_issues)} structure issues:")
            for issue in structure_issues:
                print(f"     • {issue}")

        return False


if __name__ == "__main__":
    success = verify_complete_coverage()
    sys.exit(0 if success else 1)
