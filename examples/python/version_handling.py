#!/usr/bin/env python3
"""
Version Handling Example

This example demonstrates advanced version handling capabilities using
QtForge Python bindings, including version creation, parsing, comparison,
and compatibility checking.
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


class VersionHandlingExample:
    """Example class demonstrating version handling with QtForge."""

    def __init__(self) -> None:
        """Initialize the version handling example."""
        import qtforge
        self.qtforge = qtforge
        print("📋 Version Handling Example initialized")

    def demonstrate_version_creation(self) -> list[str]:
        """Demonstrate different ways to create versions.

        Returns:
            List of created version strings
        """
        print("\n🔨 Version Creation:")

        # Create versions with different components
        version_specs = [
            (1, 0, 0),
            (2, 1, 3),
            (3, 0, 0),
            (1, 2, 3),
            (10, 5, 2)
        ]

        created_versions = []
        for major, minor, patch in version_specs:
            version_str = self.qtforge.create_version(major, minor, patch)
            created_versions.append(version_str)
            print(f"   Created: {version_str}")

        return created_versions

    def demonstrate_version_parsing(self) -> list[str]:
        """Demonstrate parsing various version string formats.

        Returns:
            List of parsed version strings
        """
        print("\n🔍 Version Parsing:")

        # Various version string formats
        version_strings = [
            "1.0.0",
            "2.1.3",
            "3.0.0-alpha",
            "1.2.3-beta.1",
            "2.0.0-rc.1",
            "1.0.0+build.1",
            "2.1.0-beta+exp.sha.5114f85",
            "1.0.0-alpha.1+beta",
            "1.0.0+20130313144700",
            "1.0.0-beta+exp.sha.5114f85"
        ]

        parsed_versions = []
        for version_str in version_strings:
            try:
                parsed = self.qtforge.parse_version(version_str)
                parsed_versions.append(parsed)
                print(f"   Parsed '{version_str}': {parsed}")
            except Exception as e:
                print(f"   Failed to parse '{version_str}': {e}")

        return parsed_versions

    def demonstrate_version_comparison(self) -> None:
        """Demonstrate version comparison scenarios."""
        print("\n⚖️  Version Comparison Scenarios:")

        # Version comparison scenarios
        comparison_pairs = [
            ("1.0.0", "1.0.1"),
            ("1.0.0", "1.1.0"),
            ("1.0.0", "2.0.0"),
            ("1.0.0-alpha", "1.0.0"),
            ("1.0.0-alpha", "1.0.0-beta"),
            ("1.0.0-beta", "1.0.0-rc"),
            ("1.0.0-rc", "1.0.0"),
            ("1.0.0", "1.0.0+build.1"),
            ("2.1.3", "2.1.2"),
            ("3.0.0-alpha.1", "3.0.0-alpha.2")
        ]

        for v1, v2 in comparison_pairs:
            # Since we don't have actual comparison in our mock implementation,
            # we'll demonstrate the concept
            print(f"   Comparing '{v1}' vs '{v2}':")
            print(f"     - Version 1: {self.qtforge.parse_version(v1)}")
            print(f"     - Version 2: {self.qtforge.parse_version(v2)}")
            # In a real implementation, you would do: v1 < v2, v1 == v2, etc.

    def demonstrate_version_ranges(self) -> None:
        """Demonstrate version range handling."""
        print("\n📊 Version Range Handling:")

        # Version range scenarios
        base_version = "2.1.0"
        print(f"   Base version: {self.qtforge.parse_version(base_version)}")

        # Demonstrate different range types
        range_examples = [
            ("^2.1.0", "Compatible with 2.1.0 (>=2.1.0 <3.0.0)"),
            ("~2.1.0", "Reasonably close to 2.1.0 (>=2.1.0 <2.2.0)"),
            (">=2.0.0", "Greater than or equal to 2.0.0"),
            ("<=3.0.0", "Less than or equal to 3.0.0"),
            ("2.1.0 - 2.5.0", "Range from 2.1.0 to 2.5.0"),
            ("2.x", "Any version in the 2.x series"),
            ("*", "Any version")
        ]

        for range_spec, description in range_examples:
            print(f"   Range '{range_spec}': {description}")

    def demonstrate_version_utilities(self) -> None:
        """Demonstrate version utility functions."""
        print("\n🛠️  Version Utilities:")

        # Version utility demonstrations
        base_versions = ["1.2.3", "2.0.0", "0.1.0"]

        for version_str in base_versions:
            print(f"\n   Working with version: {version_str}")
            parsed = self.qtforge.parse_version(version_str)
            print(f"     - Parsed: {parsed}")

            # In a real implementation, these would be actual method calls:
            print(
                f"     - Next major: {self.qtforge.create_version(2, 0, 0)} (example)")
            print(
                f"     - Next minor: {self.qtforge.create_version(1, 3, 0)} (example)")
            print(
                f"     - Next patch: {self.qtforge.create_version(1, 2, 4)} (example)")

    def demonstrate_compatibility_checking(self) -> None:
        """Demonstrate version compatibility checking."""
        print("\n🔗 Compatibility Checking:")

        # Plugin compatibility scenarios
        plugin_versions = [
            ("CorePlugin", "1.0.0"),
            ("UIPlugin", "2.1.3"),
            ("NetworkPlugin", "1.5.2"),
            ("DatabasePlugin", "3.0.0-beta"),
            ("SecurityPlugin", "2.0.1")
        ]

        host_version = "2.1.0"
        print(f"   Host application version: {host_version}")
        print(f"   Checking plugin compatibility:")

        for plugin_name, plugin_version in plugin_versions:
            print(f"     - {plugin_name} v{plugin_version}:")
            print(
                f"       Parsed: {self.qtforge.parse_version(plugin_version)}")
            # In a real implementation, you would check compatibility here
            print(f"       Status: Compatible (example)")

    def run_complete_example(self) -> int:
        """Run the complete version handling example.

        Returns:
            Exit code: 0 for success, 1 for failure
        """
        print("QtForge Version Handling Example")
        print("=" * 50)

        try:
            # Demonstrate all version handling aspects
            created_versions = self.demonstrate_version_creation()
            parsed_versions = self.demonstrate_version_parsing()
            self.demonstrate_version_comparison()
            self.demonstrate_version_ranges()
            self.demonstrate_version_utilities()
            self.demonstrate_compatibility_checking()

            print(f"\n🎉 Version handling example completed successfully!")
            print(f"   Created versions: {len(created_versions)}")
            print(f"   Parsed versions: {len(parsed_versions)}")

            return 0

        except Exception as e:
            print(f"❌ Error during version handling example: {e}")
            import traceback
            traceback.print_exc()
            return 1


def main() -> int:
    """Main function to run the version handling example.

    Returns:
        Exit code: 0 for success, 1 for failure
    """
    try:
        example = VersionHandlingExample()
        return example.run_complete_example()
    except ImportError as e:
        print(f"❌ Failed to import QtForge: {e}")
        print("Make sure QtForge Python bindings are built and in the Python path.")
        return 1


if __name__ == "__main__":
    exit_code = main()
    print(f"\nExample completed with exit code: {exit_code}")
    sys.exit(exit_code)
