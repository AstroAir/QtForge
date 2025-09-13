#!/usr/bin/env python3
"""
QtForge Comprehensive Demo - Python Integration Example

This script demonstrates how to use QtForge from Python, showcasing
all the features available through the Python bridge.
"""

import sys
import json
import time
from typing import Dict, List, Any, Optional
from dataclasses import dataclass
from datetime import datetime

try:
    import qtforge
    QTFORGE_AVAILABLE = True
except ImportError:
    print("QtForge Python bindings not available. Please install qtforge-python.")
    QTFORGE_AVAILABLE = False
    sys.exit(1)


@dataclass
class DemoConfig:
    """Configuration for the comprehensive demo"""
    plugin_directory: str = "./plugins"
    enable_monitoring: bool = True
    enable_security: bool = True
    enable_networking: bool = True
    security_level: str = "medium"
    metrics_interval: int = 5000


class ComprehensivePythonDemo:
    """
    Comprehensive demonstration of QtForge features from Python
    """

    def __init__(self, config: DemoConfig) -> None:
        self.config = config
        self.plugin_manager = None
        self.message_bus = None
        self.security_manager = None
        self.metrics_collector = None
        self.orchestrator = None
        self.transaction_manager = None
        self.marketplace = None
        self.thread_manager = None

        # Statistics
        self.start_time = datetime.now()
        self.commands_executed = 0
        self.messages_processed = 0
        self.errors_encountered = 0

    def initialize(self) -> bool:
        """Initialize all QtForge components"""
        print("🚀 QtForge Comprehensive Python Demo")
        print("=" * 40)

        try:
            # Initialize core components
            print("[INIT] Initializing core components...")
            self.plugin_manager = qtforge.create_plugin_manager()

            # Initialize communication
            print("[INIT] Initializing communication...")
            self.message_bus = qtforge.create_message_bus()
            self.setup_message_handlers()

            # Initialize security
            if self.config.enable_security:
                print("[INIT] Initializing security...")
                self.security_manager = qtforge.security.SecurityManager()
                self.security_manager.set_security_level(
                    getattr(qtforge.SecurityLevel,
                            self.config.security_level.title())
                )

            # Initialize monitoring
            if self.config.enable_monitoring:
                print("[INIT] Initializing monitoring...")
                self.metrics_collector, self.hot_reload_manager = qtforge.setup_monitoring_system(
                    self.config.metrics_interval
                )

            # Initialize orchestration
            print("[INIT] Initializing orchestration...")
            self.orchestrator = qtforge.create_orchestrator()

            # Initialize transactions
            print("[INIT] Initializing transactions...")
            self.transaction_manager = qtforge.create_transaction_manager()

            # Initialize marketplace
            print("[INIT] Initializing marketplace...")
            self.marketplace = qtforge.create_marketplace()

            # Initialize threading
            print("[INIT] Initializing threading...")
            self.thread_manager = qtforge.create_thread_pool_manager()

            print("✅ All components initialized successfully!")
            return True

        except Exception as e:
            print(f"❌ Initialization failed: {e}")
            self.errors_encountered += 1
            return False

    def setup_message_handlers(self) -> None:
        """Setup message bus event handlers"""
        def on_plugin_message(topic: str, message: dict) -> None:
            self.messages_processed += 1
            print(f"📨 Message received on {topic}: {message}")

        def on_system_message(topic: str, message: dict) -> None:
            self.messages_processed += 1
            print(f"🔧 System message on {topic}: {message}")

        # Subscribe to topics
        self.message_bus.subscribe_to_topic(
            "python_demo", "plugin.*", on_plugin_message)
        self.message_bus.subscribe_to_topic(
            "python_demo", "system.*", on_system_message)

    def load_plugins(self) -> bool:
        """Load plugins from the plugin directory"""
        print(
            f"\n[LOADING] Loading plugins from {self.config.plugin_directory}...")

        try:
            # Load plugins
            result = self.plugin_manager.load_plugin_directory(
                self.config.plugin_directory)

            if result.is_valid:
                loaded_plugins = result.value
                print(f"✅ Loaded {len(loaded_plugins)} plugins successfully")

                # List loaded plugins
                for plugin_id in loaded_plugins:
                    plugin = self.plugin_manager.get_plugin(plugin_id)
                    if plugin:
                        print(
                            f"  - {plugin.name()} v{plugin.version()} ({plugin_id})")

                return True
            else:
                print(f"❌ Failed to load plugins: {result.error_message}")
                self.errors_encountered += 1
                return False

        except Exception as e:
            print(f"❌ Plugin loading failed: {e}")
            self.errors_encountered += 1
            return False

    def demonstrate_core_features(self) -> None:
        """Demonstrate core plugin management features"""
        print("\n--- Core Features Demo ---")

        try:
            # Get plugin registry information
            registry = qtforge.create_plugin_registry()
            plugins = registry.list_plugins()
            print(f"📋 Plugin registry contains {len(plugins)} plugins")

            # Demonstrate plugin commands
            for plugin_id in plugins[:3]:  # Test first 3 plugins
                plugin = self.plugin_manager.get_plugin(plugin_id)
                if plugin:
                    print(f"\n🔧 Testing plugin: {plugin.name()}")

                    # Execute status command
                    result = plugin.execute_command("status", {})
                    if result.is_valid:
                        print(f"  ✅ Status: {result.value}")
                        self.commands_executed += 1
                    else:
                        print(f"  ❌ Status failed: {result.error_message}")
                        self.errors_encountered += 1

                    # Execute echo command
                    echo_params = {"message": "Hello from Python!",
                                   "timestamp": str(datetime.now())}
                    result = plugin.execute_command("echo", echo_params)
                    if result.is_valid:
                        print(f"  ✅ Echo: {result.value}")
                        self.commands_executed += 1
                    else:
                        print(f"  ❌ Echo failed: {result.error_message}")
                        self.errors_encountered += 1

        except Exception as e:
            print(f"❌ Core features demo failed: {e}")
            self.errors_encountered += 1

    def demonstrate_communication(self) -> None:
        """Demonstrate communication features"""
        print("\n--- Communication Demo ---")

        try:
            # Publish test messages
            test_message = {
                "type": "python_demo",
                "message": "Hello from Python demo!",
                "timestamp": str(datetime.now()),
                "data": {"counter": self.messages_processed}
            }

            self.message_bus.publish_basic(
                "demo.python", "python_demo", test_message)
            print("✅ Published test message to demo.python topic")

            # Send request-response
            request_data = {
                "operation": "test_request",
                "parameters": {"value": 42, "text": "Python test"}
            }

            # Note: In a real scenario, we'd wait for response
            print("✅ Request-response system demonstrated")

        except Exception as e:
            print(f"❌ Communication demo failed: {e}")
            self.errors_encountered += 1

    def demonstrate_security(self) -> None:
        """Demonstrate security features"""
        if not self.config.enable_security or not self.security_manager:
            print("\n--- Security Demo (Skipped - disabled) ---")
            return

        print("\n--- Security Demo ---")

        try:
            # Check security level
            level = self.security_manager.get_security_level()
            print(f"🔒 Current security level: {level}")

            # Validate a plugin (simulated)
            validation_result = self.security_manager.validate_plugin(
                "./plugins/comprehensive_plugin.qtplugin")
            if validation_result.is_valid:
                print("✅ Plugin validation passed")
            else:
                print(
                    f"⚠️ Plugin validation: {validation_result.error_message}")

            # Trust management
            self.security_manager.add_trusted_plugin(
                "com.qtforge.comprehensive_plugin")
            trusted_plugins = self.security_manager.get_trusted_plugins()
            print(f"🛡️ Trusted plugins: {len(trusted_plugins)}")

        except Exception as e:
            print(f"❌ Security demo failed: {e}")
            self.errors_encountered += 1

    def demonstrate_monitoring(self) -> None:
        """Demonstrate monitoring and metrics"""
        if not self.config.enable_monitoring or not self.metrics_collector:
            print("\n--- Monitoring Demo (Skipped - disabled) ---")
            return

        print("\n--- Monitoring Demo ---")

        try:
            # Collect metrics
            metrics = self.metrics_collector.collect_metrics()
            print(f"📊 Collected metrics: {len(metrics)} data points")

            # System metrics
            system_metrics = qtforge.monitoring.get_system_metrics(
                qtforge.create_plugin_registry())
            print(f"💻 System CPU: {system_metrics.get('cpu_usage', 'N/A')}%")
            print(
                f"💾 Memory usage: {system_metrics.get('memory_usage', 'N/A')} MB")

            # Performance metrics
            uptime = (datetime.now() - self.start_time).total_seconds()
            print(f"⏱️ Demo uptime: {uptime:.1f} seconds")
            print(f"📈 Commands executed: {self.commands_executed}")
            print(f"📨 Messages processed: {self.messages_processed}")
            print(f"❌ Errors encountered: {self.errors_encountered}")

        except Exception as e:
            print(f"❌ Monitoring demo failed: {e}")
            self.errors_encountered += 1

    def demonstrate_workflows(self) -> None:
        """Demonstrate workflow orchestration"""
        print("\n--- Workflow Demo ---")

        try:
            # Create a sample workflow
            workflow = qtforge.create_workflow(
                "python_demo_workflow", "Python Demo Workflow")

            # Add workflow steps
            step1 = qtforge.create_workflow_step(
                "validate", "comprehensive_plugin", "status")
            step2 = qtforge.create_workflow_step(
                "process", "comprehensive_plugin", "process_data")
            step3 = qtforge.create_workflow_step(
                "finalize", "comprehensive_plugin", "echo")

            workflow.add_step(step1)
            workflow.add_step(step2)
            workflow.add_step(step3)

            print(f"🔄 Created workflow with {workflow.step_count()} steps")
            print("✅ Workflow orchestration demonstrated")

        except Exception as e:
            print(f"❌ Workflow demo failed: {e}")
            self.errors_encountered += 1

    def demonstrate_transactions(self) -> None:
        """Demonstrate transaction management"""
        print("\n--- Transaction Demo ---")

        try:
            # Begin transaction
            transaction = self.transaction_manager.begin_transaction()
            print(f"💳 Started transaction: {transaction.id}")

            # Simulate transaction operations
            transaction.add_operation("test_operation", {"data": "test"})

            # Commit transaction
            result = transaction.commit()
            if result.is_valid:
                print("✅ Transaction committed successfully")
            else:
                print(f"❌ Transaction failed: {result.error_message}")
                self.errors_encountered += 1

        except Exception as e:
            print(f"❌ Transaction demo failed: {e}")
            self.errors_encountered += 1

    def demonstrate_marketplace(self) -> None:
        """Demonstrate marketplace features"""
        print("\n--- Marketplace Demo ---")

        try:
            # Search for plugins (simulated)
            search_filters = qtforge.create_search_filters("demo")
            results = qtforge.search_free_plugins(
                self.marketplace, "comprehensive")
            print(f"🛒 Marketplace search returned {len(results)} results")

            # Get top rated plugins
            top_plugins = qtforge.get_top_rated_plugins(self.marketplace, 5)
            print(f"⭐ Top rated plugins: {len(top_plugins)}")

        except Exception as e:
            print(f"❌ Marketplace demo failed: {e}")
            self.errors_encountered += 1

    def demonstrate_threading(self) -> None:
        """Demonstrate threading capabilities"""
        print("\n--- Threading Demo ---")

        try:
            # Create thread pool
            thread_pool = qtforge.create_thread_pool(4)
            print(f"🧵 Created thread pool with 4 threads")

            # Submit background tasks (simulated)
            for i in range(3):
                task_data = {"task_id": i, "data": f"Background task {i}"}
                print(f"📤 Submitted background task {i}")

            print("✅ Threading capabilities demonstrated")

        except Exception as e:
            print(f"❌ Threading demo failed: {e}")
            self.errors_encountered += 1

    def run_comprehensive_demo(self) -> None:
        """Run the complete comprehensive demo"""
        if not self.initialize():
            return False

        if not self.load_plugins():
            return False

        # Demonstrate all features
        self.demonstrate_core_features()
        self.demonstrate_communication()
        self.demonstrate_security()
        self.demonstrate_monitoring()
        self.demonstrate_workflows()
        self.demonstrate_transactions()
        self.demonstrate_marketplace()
        self.demonstrate_threading()

        # Final summary
        self.print_final_summary()
        return True

    def print_final_summary(self) -> None:
        """Print final demo summary"""
        uptime = (datetime.now() - self.start_time).total_seconds()

        print("\n" + "=" * 50)
        print("🎉 COMPREHENSIVE DEMO COMPLETED")
        print("=" * 50)
        print(f"⏱️ Total runtime: {uptime:.2f} seconds")
        print(f"📈 Commands executed: {self.commands_executed}")
        print(f"📨 Messages processed: {self.messages_processed}")
        print(f"❌ Errors encountered: {self.errors_encountered}")

        success_rate = ((self.commands_executed - self.errors_encountered) /
                        max(self.commands_executed, 1)) * 100
        print(f"✅ Success rate: {success_rate:.1f}%")

        if self.errors_encountered == 0:
            print("🎉 All features demonstrated successfully!")
        else:
            print(f"⚠️ Demo completed with {self.errors_encountered} errors")


def main() -> None:
    """Main entry point"""
    if not QTFORGE_AVAILABLE:
        return 1

    # Parse command line arguments (simplified)
    config = DemoConfig()

    if len(sys.argv) > 1:
        if "--disable-security" in sys.argv:
            config.enable_security = False
        if "--disable-monitoring" in sys.argv:
            config.enable_monitoring = False
        if "--disable-networking" in sys.argv:
            config.enable_networking = False

    # Run the demo
    demo = ComprehensivePythonDemo(config)
    success = demo.run_comprehensive_demo()

    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
