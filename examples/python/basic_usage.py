#!/usr/bin/env python3
"""
Basic QtForge Python Bindings Usage Example

This example demonstrates the fundamental usage of QtForge Python bindings,
including module import, version checking, and basic functionality.
"""

import sys
import os

# Add the build directory to Python path (adjust path as needed)
# Try different possible paths for the build directory
possible_paths = ['../../build', '../build', './build', 'build']
for path in possible_paths:
    if os.path.exists(os.path.join(path, 'qtforge.cp312-mingw_x86_64_msvcrt_gnu.pyd')) or \
       os.path.exists(os.path.join(path, 'qtforge.cp311-win_amd64.pyd')):
        sys.path.insert(0, path)
        break


def main() -> int:
    """Demonstrate basic QtForge Python bindings usage.

    Returns:
        Exit code: 0 for success, 1 for failure
    """

    print("QtForge Python Bindings - Basic Usage Example")
    print("=" * 50)

    try:
        # Import the QtForge module
        import qtforge
        print("✅ QtForge module imported successfully!")

        # Check version information
        print(f"\n📋 Version Information:")
        print(f"   Version: {qtforge.version()}")
        print(f"   Version tuple: {qtforge.version_info()}")
        print(
            f"   Major.Minor.Patch: {qtforge.__version_major__}.{qtforge.__version_minor__}.{qtforge.__version_patch__}")

        # Test basic functionality
        print(f"\n🧪 Basic Functionality Tests:")

        # Test core functions
        test_result = qtforge.test_function()
        print(f"   Test function: {test_result}")

        version_result = qtforge.get_version()
        print(f"   Get version: {version_result}")

        # Test plugin manager creation
        manager_result = qtforge.create_plugin_manager()
        print(f"   Plugin manager: {manager_result}")

        # Test plugin loading demo
        plugin_result = qtforge.load_plugin_demo("example_plugin.so")
        print(f"   Plugin loading: {plugin_result}")

        # Test utils functions
        print(f"\n🛠️  Utility Functions:")

        utils_test = qtforge.utils_test()
        print(f"   Utils test: {utils_test}")

        version_str = qtforge.create_version(1, 2, 3)
        print(f"   Create version: {version_str}")

        parsed_version = qtforge.parse_version("2.1.0-beta")
        print(f"   Parse version: {parsed_version}")

        error_msg = qtforge.create_error(404, "Resource not found")
        print(f"   Create error: {error_msg}")

        # Register Qt conversions
        qtforge.utils.register_qt_conversions()
        print(f"   Qt conversions: Registered successfully")

        print(f"\n✅ All basic functionality tests passed!")

    except ImportError as e:
        print(f"❌ Failed to import QtForge: {e}")
        print("Make sure QtForge Python bindings are built and in the Python path.")
        return 1

    except Exception as e:
        print(f"❌ Error during execution: {e}")
        import traceback
        traceback.print_exc()
        return 1

    return 0


if __name__ == "__main__":
    exit_code = main()
    print(f"\nExample completed with exit code: {exit_code}")
    sys.exit(exit_code)
