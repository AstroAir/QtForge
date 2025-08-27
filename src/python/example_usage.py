#!/usr/bin/env python3
"""
Example usage of QtForge Python bindings

This file demonstrates how to use the QtForge Python bindings
once they are properly built and installed.
"""

import sys
import os

# Add the build directory to Python path for testing
# In production, the module would be installed in site-packages
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../build'))

try:
    import qtforge
    print(f"QtForge Python bindings loaded successfully!")
    print(f"Version: {qtforge.version()}")
    print(f"Version info: {qtforge.version_info()}")

    # Example 1: Create a plugin manager
    print("\n=== Plugin Manager Example ===")
    manager = qtforge.PluginManager()
    print(f"Created plugin manager: {manager}")
    print(f"Plugin count: {manager.plugin_count()}")

    # Example 2: Work with versions
    print("\n=== Version Example ===")
    version1 = qtforge.Version(1, 2, 3)
    version2 = qtforge.Version("2.0.0")
    print(f"Version 1: {version1}")
    print(f"Version 2: {version2}")
    print(f"Version 1 < Version 2: {version1 < version2}")

    # Example 3: Create a message bus
    print("\n=== Message Bus Example ===")
    bus = qtforge.communication.MessageBus()
    print(f"Created message bus: {bus}")

    # Example 4: Work with plugin metadata
    print("\n=== Plugin Metadata Example ===")
    metadata = qtforge.PluginMetadata()
    metadata.name = "Example Plugin"
    metadata.version = version1
    metadata.description = "An example plugin for testing"
    metadata.author = "QtForge Team"
    print(f"Plugin metadata: {metadata}")

    # Example 5: Security manager
    print("\n=== Security Manager Example ===")
    security = qtforge.security.SecurityManager()
    print(f"Security manager: {security}")
    print(f"Security level: {security.get_security_level()}")

    # Example 6: Configuration manager
    print("\n=== Configuration Manager Example ===")
    config = qtforge.managers.ConfigurationManager()
    print(f"Configuration manager: {config}")

    # Example 7: Error handling
    print("\n=== Error Handling Example ===")
    error = qtforge.PluginError(
        qtforge.PluginErrorCode.InvalidArgument, "Test error")
    print(f"Plugin error: {error}")

    print("\n=== All examples completed successfully! ===")

except ImportError as e:
    print(f"Failed to import QtForge: {e}")
    print("\nThis is expected if the Python bindings haven't been built yet.")
    print("To build the Python bindings:")
    print("1. Install pybind11: pip install pybind11")
    print("2. Configure with Python bindings: cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON ..")
    print("3. Build: cmake --build .")

except Exception as e:
    print(f"Error running examples: {e}")
    import traceback
    traceback.print_exc()

if __name__ == "__main__":
    print("QtForge Python Bindings Example")
    print("=" * 40)
