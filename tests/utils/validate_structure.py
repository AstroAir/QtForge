#!/usr/bin/env python3
"""
Validation script for the new modular test structure.
Checks that all test files are properly organized and CMakeLists.txt files exist.
"""

import os
import sys
from pathlib import Path


def validate_test_structure() -> None:
    """Validate the new modular test structure."""
    test_dir = Path(__file__).parent.parent  # Go up one level since we're now in utils/
    errors = []
    warnings = []

    # Expected module directories
    expected_modules = [
        'core', 'communication', 'security', 'managers',
        'monitoring', 'orchestration', 'utils', 'platform',
        'integration', 'build_system', 'bridges', 'composition',
        'marketplace', 'transactions', 'python'
    ]

    print("🔍 Validating QtForge test structure...")
    print(f"📁 Test directory: {test_dir}")

    # Check if all expected module directories exist
    for module in expected_modules:
        module_path = test_dir / module
        if not module_path.exists():
            errors.append(f"Missing module directory: {module}")
        elif not module_path.is_dir():
            errors.append(f"Expected directory but found file: {module}")
        else:
            # Check if CMakeLists.txt exists in module
            cmake_file = module_path / "CMakeLists.txt"
            if not cmake_file.exists():
                errors.append(f"Missing CMakeLists.txt in module: {module}")
            print(f"✅ Module {module}: OK")

    # Check for orphaned test files in root
    test_files_in_root = list(test_dir.glob("test_*.cpp"))
    if test_files_in_root:
        warnings.append(
            f"Found {len(test_files_in_root)} test files in root directory")
        for file in test_files_in_root:
            warnings.append(f"  - {file.name}")

    # Check main CMakeLists.txt
    main_cmake = test_dir / "CMakeLists.txt"
    if not main_cmake.exists():
        errors.append("Missing main CMakeLists.txt")
    else:
        # Check if it includes all modules
        content = main_cmake.read_text()
        for module in expected_modules:
            if f"add_subdirectory({module})" not in content:
                warnings.append(
                    f"Module {module} not included in main CMakeLists.txt")

    # Summary
    print("\n📊 Validation Summary:")
    print(
        f"✅ Modules found: {len([m for m in expected_modules if (test_dir / m).exists()])}/{len(expected_modules)}")

    if warnings:
        print(f"⚠️  Warnings: {len(warnings)}")
        for warning in warnings:
            print(f"   {warning}")

    if errors:
        print(f"❌ Errors: {len(errors)}")
        for error in errors:
            print(f"   {error}")
        return False
    else:
        print("🎉 All validations passed!")
        return True


if __name__ == "__main__":
    success = validate_test_structure()
    sys.exit(0 if success else 1)
