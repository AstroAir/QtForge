#!/usr/bin/env python3
"""
QtForge Simple Python Plugin Example
====================================

This example demonstrates how to create and use a simple Python plugin
with QtForge, showcasing the basic plugin lifecycle and communication.

Author: QtForge Development Team
Version: 3.0.0
"""

import json
import sys
import time
from typing import Any, Dict, Optional

# Add the build directory to Python path
sys.path.insert(0, "../../../build")
sys.path.insert(0, "../../../build_test")


class SimpleQtForgePlugin:
    """A simple Python plugin for QtForge"""

    def __init__(self, plugin_id: str = "simple_python_plugin") -> None:
        """Initialize the plugin"""
        self.plugin_id = plugin_id
        self.name = "Simple Python Plugin"
        self.version = "1.0.0"
        self.description = "A demonstration Python plugin for QtForge"
        self.state = "initialized"
        self.data_store: Dict[str, Any] = {}

    def get_info(self) -> Dict[str, Any]:
        """Get plugin information"""
        return {
            "id": self.plugin_id,
            "name": self.name,
            "version": self.version,
            "description": self.description,
            "state": self.state,
            "capabilities": [
                "data_processing",
                "message_handling",
                "configuration_management",
            ],
        }

    def initialize(self) -> bool:
        """Initialize the plugin"""
        try:
            print(f"🔧 Initializing plugin: {self.name}")
            self.state = "initializing"

            # Perform initialization tasks
            self.data_store = {
                "counter": 0,
                "messages": [],
                "config": {"max_messages": 100, "auto_cleanup": True},
            }

            self.state = "ready"
            print(f"✅ Plugin {self.name} initialized successfully")
            return True

        except Exception as e:
            print(f"❌ Plugin initialization failed: {e}")
            self.state = "error"
            return False

    def start(self) -> bool:
        """Start the plugin"""
        try:
            if self.state != "ready":
                print(f"⚠️  Plugin not ready for start (current state: {self.state})")
                return False

            print(f"▶️  Starting plugin: {self.name}")
            self.state = "running"
            print(f"✅ Plugin {self.name} started successfully")
            return True

        except Exception as e:
            print(f"❌ Plugin start failed: {e}")
            self.state = "error"
            return False

    def stop(self) -> bool:
        """Stop the plugin"""
        try:
            print(f"⏹️  Stopping plugin: {self.name}")
            self.state = "stopped"
            print(f"✅ Plugin {self.name} stopped successfully")
            return True

        except Exception as e:
            print(f"❌ Plugin stop failed: {e}")
            return False

    def process_data(self, data: Any) -> Dict[str, Any]:
        """Process data and return results"""
        try:
            self.data_store["counter"] += 1

            result = {
                "processed": True,
                "input_data": data,
                "processing_count": self.data_store["counter"],
                "timestamp": str(time.time()),
                "plugin_id": self.plugin_id,
            }

            # Store the result
            if (
                len(self.data_store["messages"])
                >= self.data_store["config"]["max_messages"]
            ):
                if self.data_store["config"]["auto_cleanup"]:
                    self.data_store["messages"] = self.data_store["messages"][-50:]

            self.data_store["messages"].append(result)

            print(f"📊 Processed data: {data} (count: {self.data_store['counter']})")
            return result

        except Exception as e:
            print(f"❌ Data processing failed: {e}")
            return {"processed": False, "error": str(e)}

    def handle_message(self, message: Dict[str, Any]) -> Dict[str, Any]:
        """Handle incoming messages"""
        try:
            message_type = message.get("type", "unknown")
            payload = message.get("payload", {})

            print(f"📨 Received message: {message_type}")

            if message_type == "ping":
                return {
                    "type": "pong",
                    "payload": {
                        "original_message": message,
                        "plugin_id": self.plugin_id,
                        "status": self.state,
                    },
                }
            if message_type == "get_status":
                return {"type": "status_response", "payload": self.get_info()}
            if message_type == "process":
                result = self.process_data(payload.get("data"))
                return {"type": "process_response", "payload": result}
            return {
                "type": "error",
                "payload": {
                    "message": f"Unknown message type: {message_type}",
                    "original_message": message,
                },
            }

        except Exception as e:
            return {
                "type": "error",
                "payload": {
                    "message": f"Message handling failed: {e}",
                    "original_message": message,
                },
            }

    def get_statistics(self) -> Dict[str, Any]:
        """Get plugin statistics"""
        return {
            "plugin_id": self.plugin_id,
            "state": self.state,
            "processing_count": self.data_store.get("counter", 0),
            "message_count": len(self.data_store.get("messages", [])),
            "configuration": self.data_store.get("config", {}),
        }


class QtForgePluginExample:
    """Example demonstrating QtForge Python plugin integration"""

    def __init__(self) -> None:
        """Initialize the example"""
        self.qtforge: Optional[Any] = None
        self.plugin: Optional[SimpleQtForgePlugin] = None

    def initialize_qtforge(self) -> bool:
        """Initialize QtForge"""
        try:
            print("🚀 Initializing QtForge for plugin example...")
            import qtforge

            self.qtforge = qtforge
            print(f"✅ QtForge version: {qtforge.__version__}")
            return True
        except ImportError as e:
            print(f"❌ Failed to import QtForge: {e}")
            print("💡 Make sure QtForge is built with Python bindings enabled")
            return False

    def create_and_test_plugin(self) -> bool:
        """Create and test the Python plugin"""
        try:
            print("\n📦 Creating Python Plugin...")
            self.plugin = SimpleQtForgePlugin("demo_plugin")

            # Test plugin lifecycle
            print("\n🔄 Testing Plugin Lifecycle...")

            # Initialize
            if not self.plugin.initialize():
                return False

            # Start
            if not self.plugin.start():
                return False

            # Test functionality
            print("\n🧪 Testing Plugin Functionality...")

            # Test data processing
            test_data = [1, 2, 3, 4, 5]
            result = self.plugin.process_data(test_data)
            print(f"📊 Data processing result: {result}")

            # Test message handling
            test_messages = [
                {"type": "ping", "payload": {"sender": "test"}},
                {"type": "get_status", "payload": {}},
                {"type": "process", "payload": {"data": "hello world"}},
                {"type": "unknown", "payload": {"test": True}},
            ]

            for msg in test_messages:
                response = self.plugin.handle_message(msg)
                print(f"📨 Message response: {response['type']}")

            # Get statistics
            stats = self.plugin.get_statistics()
            print(f"📈 Plugin statistics: {json.dumps(stats, indent=2)}")

            # Stop plugin
            self.plugin.stop()

            return True

        except Exception as e:
            print(f"❌ Plugin test failed: {e}")
            return False

    def test_qtforge_integration(self) -> bool:
        """Test integration with QtForge core"""
        try:
            print("\n🔗 Testing QtForge Integration...")

            # Test if we can create QtForge components
            if hasattr(self.qtforge, "create_plugin_manager"):
                manager = self.qtforge.create_plugin_manager()  # type: ignore
                print("✅ Plugin manager created")
            elif hasattr(self.qtforge, "core") and hasattr(
                self.qtforge.core, "create_plugin_manager"
            ):  # type: ignore
                manager = self.qtforge.core.create_plugin_manager()  # type: ignore
                print("✅ Plugin manager created via core module")
            else:
                print("⚠️  Plugin manager creation not available")

            # Test available modules
            modules = ["core", "utils", "security", "communication", "managers"]
            available_modules = []

            for module_name in modules:
                if hasattr(self.qtforge, module_name):
                    available_modules.append(module_name)
                    print(f"✅ {module_name} module available")
                else:
                    print(f"⚠️  {module_name} module not available")

            print(f"📋 Available modules: {available_modules}")
            return len(available_modules) > 0

        except Exception as e:
            print(f"❌ QtForge integration test failed: {e}")
            return False

    def run_example(self) -> int:
        """Run the complete plugin example"""
        print("=" * 60)
        print("🎯 QtForge Simple Python Plugin Example")
        print("=" * 60)

        # Initialize QtForge
        if not self.initialize_qtforge():
            return 1

        # Test QtForge integration
        if not self.test_qtforge_integration():
            print("⚠️  QtForge integration limited, but continuing with plugin test...")

        # Create and test plugin
        if not self.create_and_test_plugin():
            return 1

        print("\n" + "=" * 60)
        print("🎉 Plugin Example Completed Successfully!")
        print("=" * 60)
        print("💡 This example demonstrated:")
        print("   • Python plugin creation and lifecycle")
        print("   • Data processing capabilities")
        print("   • Message handling system")
        print("   • Integration with QtForge core")
        print("   • Plugin statistics and monitoring")

        return 0


def main() -> int:
    """Main entry point"""

    try:
        example = QtForgePluginExample()
        return example.run_example()
    except KeyboardInterrupt:
        print("\n⏹️  Example interrupted by user")
        return 1
    except Exception as e:
        print(f"\n💥 Unexpected error: {e}")
        import traceback

        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
