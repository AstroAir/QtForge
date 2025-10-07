#!/usr/bin/env python3
"""
QtForge Python Bindings - Comprehensive Examples

This file demonstrates all the features and capabilities of the
official QtForge Python bindings with practical examples.
"""

import sys
import time
from pathlib import Path
from typing import Optional, Any

# Add the build/python directory to the path for QtForge bindings
build_path = Path(__file__).parent.parent / "build" / "python"
if build_path.exists():
    sys.path.insert(0, str(build_path))

try:
    import qtforge
    from qtforge import (
        ConfigurationManager,
        IPlugin,
        MessageBus,
        PluginCapability,
        PluginLoader,
        PluginManager,
        PluginPriority,
        PluginRegistry,
        PluginState,
        Version,
    )

    QTFORGE_AVAILABLE = True
    print("✅ QtForge Python bindings loaded successfully!")
except ImportError as e:
    print(f"⚠️  QtForge Python bindings not available: {e}")
    print("Please build QtForge with Python bindings enabled.")
    QTFORGE_AVAILABLE = False

    # Create dummy classes for when QtForge is not available
    class DummyIPlugin:
        def __init__(self):
            pass

        def get_plugin_id(self) -> str:
            return ""

        def get_name(self) -> str:
            return ""

        def get_version(self) -> str:
            return ""

        def initialize(self) -> bool:
            return True

        def start(self) -> bool:
            return True

        def stop(self) -> bool:
            return True

        def cleanup(self) -> bool:
            return True

    # Create dummy qtforge module
    class DummyQtForge:
        @staticmethod
        def create_plugin_manager():
            return None

        @staticmethod
        def get_version():
            return "N/A"

        @staticmethod
        def get_version_info():
            return (0, 0, 0)

        @staticmethod
        def get_build_info():
            return {}

        @staticmethod
        def list_available_modules():
            return []

        @staticmethod
        def test_connection():
            return "QtForge not available"

    qtforge = DummyQtForge()  # type: ignore
    IPlugin = DummyIPlugin  # type: ignore


# Example 1: Basic Plugin Implementation using QtForge
class DatabasePlugin(IPlugin):  # type: ignore
    """Example database plugin with full lifecycle."""

    def __init__(self) -> None:
        super().__init__()
        self.connection: Optional[str] = None
        self.connected = False
        self._plugin_id: str = "database_plugin"
        self._name: str = "Database Plugin"
        self._version: str = "1.0.0"

    def get_plugin_id(self) -> str:
        """Get the plugin ID."""
        return self._plugin_id

    def get_name(self) -> str:
        """Get the plugin name."""
        return self._name

    def get_version(self) -> str:
        """Get the plugin version."""
        return self._version

    def initialize(self) -> bool:
        """Initialize the database plugin."""
        print(f"[{self._name}] Initializing database plugin")
        # Simulate database initialization
        self.connection = f"db_connection_{self._plugin_id}"
        return True

    def start(self) -> bool:
        """Start the database plugin."""
        if not self.connection:
            print(f"[{self._name}] Cannot start: database not initialized")
            return False

        print(f"[{self._name}] Starting database connection")
        # Simulate connection establishment
        time.sleep(0.1)
        self.connected = True
        print(f"[{self._name}] Database connected: {self.connection}")
        return True

    def stop(self) -> bool:
        """Stop the database plugin."""
        if self.connected:
            print(f"[{self._name}] Closing database connection")
            self.connected = False
            self.connection = None
        return True

    def cleanup(self) -> bool:
        """Cleanup database resources."""
        if self.connection:
            print(f"[{self._name}] Cleaning up database resources")
            self.connection = None
        return True

    def execute_query(self, query: str):
        """Execute a database query."""
        if not self.connected:
            raise RuntimeError("Database not connected")

        start_time = time.time()
        # Simulate query execution
        time.sleep(0.01)
        result = {"query": query, "rows": 42, "status": "success"}
        elapsed = time.time() - start_time

        print(f"[{self._name}] Query executed in {elapsed:.3f}s: {query[:50]}")
        return result


class WebServerPlugin(IPlugin):  # type: ignore
    """Example web server plugin."""

    def __init__(self) -> None:
        super().__init__()
        self.server_running = False
        self.port = 8080
        self._plugin_id: str = "webserver_plugin"
        self._name: str = "Web Server Plugin"
        self._version: str = "1.0.0"

    def get_plugin_id(self) -> str:
        return self._plugin_id

    def get_name(self) -> str:
        return self._name

    def get_version(self) -> str:
        return self._version

    def initialize(self) -> bool:
        """Initialize the web server."""
        print(f"[{self._name}] Initializing web server on port {self.port}")
        return True

    def start(self) -> bool:
        """Start the web server."""
        print(f"[{self._name}] Starting web server on port {self.port}")
        # Simulate server startup
        time.sleep(0.05)
        self.server_running = True
        print(f"[{self._name}] Web server started successfully")
        return True

    def stop(self) -> bool:
        """Stop the web server."""
        if self.server_running:
            print(f"[{self._name}] Stopping web server")
            self.server_running = False
        return True

    def cleanup(self) -> bool:
        """Cleanup web server resources."""
        print(f"[{self._name}] Cleaning up web server resources")
        return True

    def handle_request(self, method: str, path: str):
        """Handle an HTTP request."""
        if not self.server_running:
            raise RuntimeError("Web server not running")

        print(f"[{self._name}] Handling {method} request to {path}")
        # Simulate request processing
        time.sleep(0.005)
        return {"status": 200, "method": method, "path": path}


def example_1_basic_plugin_management():
    """Example 1: Basic Plugin Management with QtForge."""
    print("\n" + "=" * 60)
    print("🔌 Example 1: Basic Plugin Management")
    print("=" * 60)

    if not QTFORGE_AVAILABLE:
        print("❌ QtForge not available - skipping example")
        return

    try:
        # Create plugin manager
        manager = qtforge.create_plugin_manager()
        print("✅ Created plugin manager")

        # Create plugins
        db_plugin = DatabasePlugin()
        web_plugin = WebServerPlugin()

        # Register plugins (if the API supports it)
        print(f"📝 Created plugins: {db_plugin.get_name()}, {web_plugin.get_name()}")

        # Initialize plugins
        if db_plugin.initialize():
            print(f"✅ {db_plugin.get_name()} initialized")

        if web_plugin.initialize():
            print(f"✅ {web_plugin.get_name()} initialized")

        # Start plugins
        if db_plugin.start():
            print(f"🚀 {db_plugin.get_name()} started")

        if web_plugin.start():
            print(f"🚀 {web_plugin.get_name()} started")

        # Use plugins
        result = db_plugin.execute_query("SELECT * FROM users")
        print(f"📊 Database query result: {result}")

        response = web_plugin.handle_request("GET", "/api/users")
        print(f"🌐 Web request response: {response}")

        # Stop plugins
        db_plugin.stop()
        web_plugin.stop()
        print("🛑 Plugins stopped")

        # Cleanup
        db_plugin.cleanup()
        web_plugin.cleanup()
        print("🧹 Plugins cleaned up")

    except Exception as e:
        print(f"❌ Error in plugin management example: {e}")
        import traceback

        traceback.print_exc()


def example_2_qtforge_system_info():
    """Example 2: QtForge System Information."""
    print("\n" + "=" * 60)
    print("ℹ️  Example 2: QtForge System Information")
    print("=" * 60)

    if not QTFORGE_AVAILABLE:
        print("❌ QtForge not available - skipping example")
        return

    try:
        # Get version information
        version = qtforge.get_version()  # type: ignore
        version_info = qtforge.get_version_info()  # type: ignore
        print(f"📦 QtForge Version: {version}")
        print(f"📦 Version Info: {version_info}")

        # Get build information
        build_info = qtforge.get_build_info()  # type: ignore
        print(f"🔧 Build Info: {build_info}")

        # List available modules
        modules = qtforge.list_available_modules()  # type: ignore
        print(f"📚 Available Modules: {modules}")

        # Test connection
        test_result = qtforge.test_connection()  # type: ignore
        print(f"🔗 Connection Test: {test_result}")

    except Exception as e:
        print(f"❌ Error getting system info: {e}")
        import traceback

        traceback.print_exc()


def main():
    """Run all QtForge Python binding examples."""
    print("🎯 QtForge Python Bindings - Comprehensive Examples")
    print("=" * 70)

    try:
        # Run examples
        example_1_basic_plugin_management()
        example_2_qtforge_system_info()

        print("\n" + "=" * 70)
        print("🎉 ALL EXAMPLES COMPLETED SUCCESSFULLY!")
        print("=" * 70)

    except Exception as e:
        print(f"\n❌ Error running examples: {e}")
        import traceback

        traceback.print_exc()
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
