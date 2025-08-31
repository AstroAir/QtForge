#!/usr/bin/env python3
"""
Plugin Management Example

This example demonstrates how to use QtForge Python bindings for plugin
management tasks, including loading, unloading, and managing plugins.
"""

import sys
import os
from pathlib import Path

# Add the build directory to Python path (adjust path as needed)
# Try different possible paths for the build directory
possible_paths = ['../../build', '../build', './build', 'build']
for path in possible_paths:
    if os.path.exists(os.path.join(path, 'qtforge.cp312-mingw_x86_64_msvcrt_gnu.pyd')) or \
       os.path.exists(os.path.join(path, 'qtforge.cp311-win_amd64.pyd')):
        sys.path.insert(0, path)
        break


class PluginManagerExample:
    """Example class demonstrating plugin management with QtForge."""

    def __init__(self) -> None:
        """Initialize the plugin manager example."""
        import qtforge
        self.qtforge = qtforge
        print("🔧 Plugin Manager Example initialized")

    def demonstrate_plugin_lifecycle(self) -> list[str]:
        """Demonstrate the complete plugin lifecycle.

        Returns:
            List of loaded plugin results
        """
        print("\n📦 Plugin Lifecycle Demonstration:")

        # Create a plugin manager
        manager_result = self.qtforge.create_plugin_manager()
        print(f"   Manager creation: {manager_result}")

        # Simulate plugin discovery
        print(f"\n🔍 Plugin Discovery:")
        plugins_to_load = [
            "plugins/core_plugin.so",
            "plugins/ui_plugin.dll",
            "plugins/network_plugin.dylib"
        ]

        loaded_plugins = []
        for plugin_path in plugins_to_load:
            try:
                result = self.qtforge.load_plugin_demo(plugin_path)
                print(f"   Loading {plugin_path}: {result}")
                loaded_plugins.append(plugin_path)
            except Exception as e:
                print(f"   Failed to load {plugin_path}: {e}")

        print(f"\n✅ Successfully loaded {len(loaded_plugins)} plugins")
        return loaded_plugins

    def demonstrate_version_management(self) -> None:
        """Demonstrate version management for plugins."""
        print(f"\n📋 Version Management:")

        # Create different versions
        versions = [
            (1, 0, 0),
            (1, 2, 3),
            (2, 0, 0),
            (2, 1, 5)
        ]

        for major, minor, patch in versions:
            version_str = self.qtforge.create_version(major, minor, patch)
            print(f"   Created version: {version_str}")

        # Parse version strings
        version_strings = [
            "1.0.0",
            "2.1.3-beta",
            "3.0.0-rc1+build.123"
        ]

        for version_str in version_strings:
            parsed = self.qtforge.parse_version(version_str)
            print(f"   Parsed '{version_str}': {parsed}")

    def demonstrate_error_handling(self) -> None:
        """Demonstrate error handling in plugin operations."""
        print(f"\n⚠️  Error Handling Demonstration:")

        # Common error scenarios
        error_scenarios = [
            (404, "Plugin not found"),
            (500, "Plugin initialization failed"),
            (403, "Permission denied"),
            (409, "Plugin already loaded"),
            (422, "Invalid plugin configuration")
        ]

        for code, message in error_scenarios:
            error_msg = self.qtforge.create_error(code, message)
            print(f"   Error {code}: {error_msg}")

    def demonstrate_module_introspection(self) -> None:
        """Demonstrate module introspection capabilities."""
        print(f"\n🔍 Module Introspection:")

        # Show available modules
        print(f"   Main module attributes:")
        main_attrs = [attr for attr in dir(
            self.qtforge) if not attr.startswith('_')]
        for attr in sorted(main_attrs):
            print(f"     - {attr}")

        print(f"\n   Core module attributes:")
        core_attrs = [attr for attr in dir(
            self.qtforge.core) if not attr.startswith('_')]
        for attr in sorted(core_attrs):
            print(f"     - {attr}")

        print(f"\n   Utils module attributes:")
        utils_attrs = [attr for attr in dir(
            self.qtforge.utils) if not attr.startswith('_')]
        for attr in sorted(utils_attrs):
            print(f"     - {attr}")

    def run_complete_example(self) -> int:
        """Run the complete plugin management example.

        Returns:
            Exit code: 0 for success, 1 for failure
        """
        print("QtForge Plugin Management Example")
        print("=" * 50)

        try:
            # Demonstrate all aspects
            loaded_plugins = self.demonstrate_plugin_lifecycle()
            self.demonstrate_version_management()
            self.demonstrate_error_handling()
            self.demonstrate_module_introspection()

            print(f"\n🎉 Plugin management example completed successfully!")
            print(f"   Total plugins processed: {len(loaded_plugins)}")

            return 0

        except Exception as e:
            print(f"❌ Error during plugin management example: {e}")
            import traceback
            traceback.print_exc()
            return 1


def main() -> int:
    """Main function to run the plugin management example.

    Returns:
        Exit code: 0 for success, 1 for failure
    """
    try:
        example = PluginManagerExample()
        return example.run_complete_example()
    except ImportError as e:
        print(f"❌ Failed to import QtForge: {e}")
        print("Make sure QtForge Python bindings are built and in the Python path.")
        return 1


if __name__ == "__main__":
    exit_code = main()
    print(f"\nExample completed with exit code: {exit_code}")
    sys.exit(exit_code)
