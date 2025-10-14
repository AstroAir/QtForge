"""Test suite for extended QtForge Python bindings
Tests IServicePlugin, PluginPropertySystem, and PluginCapabilityDiscovery.
"""

import os
import sys
import unittest

# Add build directory to path if needed
build_dir = os.path.join(
    os.path.dirname(__file__), "..", "..", "build", "src", "python"
)
if os.path.exists(build_dir):
    sys.path.insert(0, build_dir)

try:
    import qtforge
    from qtforge.core import (
        PluginCapabilityDiscovery,
        PluginCapabilityInfo,
        PluginInterfaceInfo,
        PluginMethodInfo,
        PluginPropertyInfo,
        PluginPropertySystem,
        PropertyBinding,
        PropertyBindingType,
        PropertyChangeEvent,
        PropertyMetadata,
        PropertyNotificationMode,
        PropertyValidationType,
        ServiceExecutionMode,
        ServiceState,
    )

    BINDINGS_AVAILABLE = True
except ImportError:
    BINDINGS_AVAILABLE = False


@unittest.skipUnless(BINDINGS_AVAILABLE, "QtForge bindings not available")
class TestServicePluginEnums(unittest.TestCase):
    """Test ServicePlugin enums."""

    def test_service_execution_mode_enum(self):
        """Test ServiceExecutionMode enum values."""
        assert ServiceExecutionMode.MainThread is not None
        assert ServiceExecutionMode.WorkerThread is not None
        assert ServiceExecutionMode.ThreadPool is not None
        assert ServiceExecutionMode.Async is not None
        assert ServiceExecutionMode.Custom is not None

    def test_service_state_enum(self):
        """Test ServiceState enum values."""
        assert ServiceState.Stopped is not None
        assert ServiceState.Starting is not None
        assert ServiceState.Running is not None
        assert ServiceState.Pausing is not None
        assert ServiceState.Paused is not None
        assert ServiceState.Resuming is not None
        assert ServiceState.Stopping is not None
        assert ServiceState.Error is not None
        assert ServiceState.Restarting is not None


@unittest.skipUnless(BINDINGS_AVAILABLE, "QtForge bindings not available")
class TestPropertySystemEnums(unittest.TestCase):
    """Test PropertySystem enums."""

    def test_property_binding_type_enum(self):
        """Test PropertyBindingType enum values."""
        assert PropertyBindingType.OneWay is not None
        assert PropertyBindingType.TwoWay is not None
        assert PropertyBindingType.OneTime is not None

    def test_property_validation_type_enum(self):
        """Test PropertyValidationType enum values."""
        assert PropertyValidationType.None_ is not None
        assert PropertyValidationType.Range is not None
        assert PropertyValidationType.Enum is not None
        assert PropertyValidationType.Regex is not None
        assert PropertyValidationType.Custom is not None

    def test_property_notification_mode_enum(self):
        """Test PropertyNotificationMode enum values."""
        assert PropertyNotificationMode.Immediate is not None
        assert PropertyNotificationMode.Debounced is not None
        assert PropertyNotificationMode.Throttled is not None
        assert PropertyNotificationMode.Batched is not None


@unittest.skipUnless(BINDINGS_AVAILABLE, "QtForge bindings not available")
class TestPropertySystemStructs(unittest.TestCase):
    """Test PropertySystem structs."""

    def test_property_metadata_creation(self):
        """Test PropertyMetadata struct creation."""
        metadata = PropertyMetadata()
        assert metadata is not None

        # Test setting attributes
        metadata.name = "test_property"
        metadata.display_name = "Test Property"
        metadata.description = "A test property"
        metadata.category = "Testing"
        metadata.is_required = True
        metadata.is_readonly = False

        assert metadata.name == "test_property"
        assert metadata.display_name == "Test Property"
        assert metadata.is_required
        assert not metadata.is_readonly

    def test_property_binding_creation(self):
        """Test PropertyBinding struct creation."""
        binding = PropertyBinding()
        assert binding is not None

        binding.binding_id = "binding_1"
        binding.source_plugin_id = "plugin_a"
        binding.source_property = "prop_a"
        binding.target_plugin_id = "plugin_b"
        binding.target_property = "prop_b"
        binding.binding_type = PropertyBindingType.OneWay
        binding.is_active = True

        assert binding.binding_id == "binding_1"
        assert binding.source_plugin_id == "plugin_a"
        assert binding.is_active

    def test_property_change_event_creation(self):
        """Test PropertyChangeEvent struct creation."""
        event = PropertyChangeEvent()
        assert event is not None

        event.plugin_id = "test_plugin"
        event.property_name = "test_prop"
        event.source = "user"

        assert event.plugin_id == "test_plugin"
        assert event.property_name == "test_prop"


@unittest.skipUnless(BINDINGS_AVAILABLE, "QtForge bindings not available")
class TestPluginPropertySystem(unittest.TestCase):
    """Test PluginPropertySystem class."""

    def test_property_system_creation(self):
        """Test PluginPropertySystem creation."""
        prop_system = PluginPropertySystem()
        assert prop_system is not None
        assert "PluginPropertySystem" in repr(prop_system)


@unittest.skipUnless(BINDINGS_AVAILABLE, "QtForge bindings not available")
class TestCapabilityDiscoveryStructs(unittest.TestCase):
    """Test CapabilityDiscovery structs."""

    def test_plugin_capability_info_creation(self):
        """Test PluginCapabilityInfo struct creation."""
        info = PluginCapabilityInfo()
        assert info is not None

        info.name = "TestCapability"
        info.description = "A test capability"

        assert info.name == "TestCapability"
        assert info.description == "A test capability"

    def test_plugin_method_info_creation(self):
        """Test PluginMethodInfo struct creation."""
        method_info = PluginMethodInfo()
        assert method_info is not None

        method_info.name = "test_method"
        method_info.signature = "void test_method()"
        method_info.return_type = "void"
        method_info.is_invokable = True
        method_info.is_slot = False
        method_info.is_signal = False

        assert method_info.name == "test_method"
        assert method_info.is_invokable
        assert not method_info.is_slot

    def test_plugin_property_info_creation(self):
        """Test PluginPropertyInfo struct creation."""
        prop_info = PluginPropertyInfo()
        assert prop_info is not None

        prop_info.name = "test_property"
        prop_info.type = "int"
        prop_info.is_readable = True
        prop_info.is_writable = True
        prop_info.is_resettable = False
        prop_info.has_notify_signal = True

        assert prop_info.name == "test_property"
        assert prop_info.is_readable
        assert prop_info.is_writable

    def test_plugin_interface_info_creation(self):
        """Test PluginInterfaceInfo struct creation."""
        interface_info = PluginInterfaceInfo()
        assert interface_info is not None

        interface_info.interface_id = "com.example.ITestInterface"
        interface_info.interface_name = "ITestInterface"
        interface_info.version = "1.0.0"

        assert interface_info.interface_id == "com.example.ITestInterface"
        assert interface_info.interface_name == "ITestInterface"


@unittest.skipUnless(BINDINGS_AVAILABLE, "QtForge bindings not available")
class TestPluginCapabilityDiscovery(unittest.TestCase):
    """Test PluginCapabilityDiscovery class."""

    def test_capability_discovery_creation(self):
        """Test PluginCapabilityDiscovery creation."""
        discovery = PluginCapabilityDiscovery()
        assert discovery is not None
        assert "PluginCapabilityDiscovery" in repr(discovery)


@unittest.skipUnless(BINDINGS_AVAILABLE, "QtForge bindings not available")
class TestSystemStatus(unittest.TestCase):
    """Test system status includes new features."""

    def test_system_status_includes_new_features(self):
        """Test that system status reports new features."""
        status = qtforge.core.get_system_status()
        assert status is not None
        assert "features" in status

        features = status["features"]
        assert "service_plugin" in features
        assert "property_system" in features
        assert "capability_discovery" in features


def run_tests():
    """Run all tests."""
    if not BINDINGS_AVAILABLE:
        return 1

    # Create test suite
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()

    # Add all test classes
    suite.addTests(loader.loadTestsFromTestCase(TestServicePluginEnums))
    suite.addTests(loader.loadTestsFromTestCase(TestPropertySystemEnums))
    suite.addTests(loader.loadTestsFromTestCase(TestPropertySystemStructs))
    suite.addTests(loader.loadTestsFromTestCase(TestPluginPropertySystem))
    suite.addTests(loader.loadTestsFromTestCase(TestCapabilityDiscoveryStructs))
    suite.addTests(loader.loadTestsFromTestCase(TestPluginCapabilityDiscovery))
    suite.addTests(loader.loadTestsFromTestCase(TestSystemStatus))

    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)

    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(run_tests())
