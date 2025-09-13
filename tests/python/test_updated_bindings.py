#!/usr/bin/env python3
"""
Test script for updated QtForge Python bindings
Tests all new functionality added to the bindings.
"""

import sys
import traceback

def test_core_bindings() -> None:
    """Test core module bindings including new interfaces."""
    print("Testing core bindings...")

    try:
        import qtforge.core as core

        # Test enums
        print("  Testing enums...")
        assert hasattr(core, 'PluginState')
        assert hasattr(core, 'PluginCapability')
        assert hasattr(core, 'PluginPriority')
        assert hasattr(core, 'PluginLifecycleEvent')
        assert hasattr(core, 'PluginType')

        # Test new PluginType enum
        assert hasattr(core.PluginType, 'Native')
        assert hasattr(core.PluginType, 'Python')
        assert hasattr(core.PluginType, 'Lua')
        assert hasattr(core.PluginType, 'Remote')
        assert hasattr(core.PluginType, 'Composite')

        # Test core classes
        print("  Testing core classes...")
        assert hasattr(core, 'IPlugin')
        assert hasattr(core, 'PluginManager')
        assert hasattr(core, 'PluginLoader')
        assert hasattr(core, 'PluginRegistry')
        assert hasattr(core, 'PluginDependencyResolver')
        assert hasattr(core, 'PluginLifecycleManager')

        # Test new interface classes
        print("  Testing new interface classes...")
        assert hasattr(core, 'InterfaceCapability')
        assert hasattr(core, 'InterfaceDescriptor')
        assert hasattr(core, 'IAdvancedPlugin')
        assert hasattr(core, 'IDynamicPlugin')
        assert hasattr(core, 'IServicePlugin')

        # Test service-related enums
        print("  Testing service enums...")
        assert hasattr(core, 'ServiceExecutionMode')
        assert hasattr(core, 'ServiceState')
        assert hasattr(core, 'ServicePriority')
        assert hasattr(core, 'ServiceHealth')

        # Test service enum values
        assert hasattr(core.ServiceExecutionMode, 'MainThread')
        assert hasattr(core.ServiceExecutionMode, 'WorkerThread')
        assert hasattr(core.ServiceState, 'Running')
        assert hasattr(core.ServiceState, 'Stopped')
        assert hasattr(core.ServicePriority, 'Normal')
        assert hasattr(core.ServicePriority, 'High')
        assert hasattr(core.ServiceHealth, 'Healthy')
        assert hasattr(core.ServiceHealth, 'Unhealthy')

        # Test factory functions
        print("  Testing factory functions...")
        manager = core.create_plugin_manager()
        assert manager is not None

        loader = core.create_plugin_loader()
        assert loader is not None

        registry = core.create_plugin_registry()
        assert registry is not None

        resolver = core.create_plugin_dependency_resolver()
        assert resolver is not None

        lifecycle_manager = core.create_plugin_lifecycle_manager()
        assert lifecycle_manager is not None

        print("  Core bindings test passed!")
        return True

    except Exception as e:
        print(f"  Core bindings test failed: {e}")
        traceback.print_exc()
        return False

def test_communication_bindings() -> None:
    """Test communication module bindings including service contracts."""
    print("Testing communication bindings...")

    try:
        import qtforge.communication as comm

        # Test enums
        print("  Testing communication enums...")
        assert hasattr(comm, 'DeliveryMode')
        assert hasattr(comm, 'MessagePriority')
        assert hasattr(comm, 'ServiceCapability')

        # Test service capability enum values
        assert hasattr(comm.ServiceCapability, 'Synchronous')
        assert hasattr(comm.ServiceCapability, 'Asynchronous')
        assert hasattr(comm.ServiceCapability, 'Streaming')
        assert hasattr(comm.ServiceCapability, 'Transactional')

        # Test message classes
        print("  Testing message classes...")
        assert hasattr(comm, 'IMessage')
        assert hasattr(comm, 'BasicMessage')
        assert hasattr(comm, 'MessageBus')

        # Test service contract classes
        print("  Testing service contract classes...")
        assert hasattr(comm, 'ServiceVersion')
        assert hasattr(comm, 'ServiceMethodDescriptor')
        assert hasattr(comm, 'ServiceContract')

        # Test factory functions
        print("  Testing communication factory functions...")
        message_bus = comm.create_message_bus()
        assert message_bus is not None

        print("  Communication bindings test passed!")
        return True

    except Exception as e:
        print(f"  Communication bindings test failed: {e}")
        traceback.print_exc()
        return False

def test_security_bindings() -> None:
    """Test security module bindings including new enums."""
    print("Testing security bindings...")

    try:
        import qtforge.security as security

        # Test enums
        print("  Testing security enums...")
        assert hasattr(security, 'SecurityLevel')
        assert hasattr(security, 'PluginPermission')
        assert hasattr(security, 'TrustLevel')

        # Test permission enum values
        assert hasattr(security.PluginPermission, 'FileSystemRead')
        assert hasattr(security.PluginPermission, 'FileSystemWrite')
        assert hasattr(security.PluginPermission, 'NetworkAccess')
        assert hasattr(security.PluginPermission, 'ProcessCreation')

        # Test trust level enum values
        assert hasattr(security.TrustLevel, 'Untrusted')
        assert hasattr(security.TrustLevel, 'Limited')
        assert hasattr(security.TrustLevel, 'Trusted')
        assert hasattr(security.TrustLevel, 'FullyTrusted')

        # Test security classes
        print("  Testing security classes...")
        assert hasattr(security, 'SecurityManager')
        assert hasattr(security, 'SecurityValidator')
        assert hasattr(security, 'SignatureVerifier')
        assert hasattr(security, 'PermissionManager')
        assert hasattr(security, 'SecurityPolicyEngine')

        # Test factory functions
        print("  Testing security factory functions...")
        manager = security.create_security_manager()
        assert manager is not None

        validator = security.create_security_validator()
        assert validator is not None

        verifier = security.create_signature_verifier()
        assert verifier is not None

        perm_manager = security.create_permission_manager()
        assert perm_manager is not None

        policy_engine = security.create_security_policy_engine()
        assert policy_engine is not None

        print("  Security bindings test passed!")
        return True

    except Exception as e:
        print(f"  Security bindings test failed: {e}")
        traceback.print_exc()
        return False

def test_managers_bindings() -> None:
    """Test managers module bindings."""
    print("Testing managers bindings...")

    try:
        import qtforge.managers as managers

        # Test enums
        print("  Testing manager enums...")
        assert hasattr(managers, 'ConfigurationScope')
        assert hasattr(managers, 'ConfigurationChangeType')
        assert hasattr(managers, 'LogLevel')

        # Test manager classes
        print("  Testing manager classes...")
        assert hasattr(managers, 'ConfigurationManager')
        assert hasattr(managers, 'LoggingManager')
        assert hasattr(managers, 'ResourceManager')
        assert hasattr(managers, 'PluginVersionManager')

        # Test factory functions
        print("  Testing manager factory functions...")
        config_manager = managers.create_configuration_manager()
        assert config_manager is not None

        logging_manager = managers.create_logging_manager()
        assert logging_manager is not None

        resource_manager = managers.create_resource_manager()
        assert resource_manager is not None

        print("  Managers bindings test passed!")
        return True

    except Exception as e:
        print(f"  Managers bindings test failed: {e}")
        traceback.print_exc()
        return False

def test_monitoring_bindings() -> None:
    """Test monitoring module bindings."""
    print("Testing monitoring bindings...")

    try:
        import qtforge.monitoring as monitoring

        # Test monitoring classes
        print("  Testing monitoring classes...")
        assert hasattr(monitoring, 'PluginHotReloadManager')
        assert hasattr(monitoring, 'PluginMetricsCollector')

        # Test factory functions
        print("  Testing monitoring factory functions...")
        hot_reload_manager = monitoring.create_hot_reload_manager()
        assert hot_reload_manager is not None

        metrics_collector = monitoring.create_metrics_collector()
        assert metrics_collector is not None

        print("  Monitoring bindings test passed!")
        return True

    except Exception as e:
        print(f"  Monitoring bindings test failed: {e}")
        traceback.print_exc()
        return False

def test_transactions_bindings() -> None:
    """Test transactions module bindings."""
    print("Testing transactions bindings...")

    try:
        import qtforge.transactions as transactions

        # Test transaction enums
        print("  Testing transaction enums...")
        assert hasattr(transactions, 'TransactionState')
        assert hasattr(transactions, 'IsolationLevel')

        # Test transaction classes
        print("  Testing transaction classes...")
        assert hasattr(transactions, 'PluginTransactionManager')

        # Test singleton access
        print("  Testing transaction manager singleton...")
        manager = transactions.get_transaction_manager()
        assert manager is not None

        print("  Transactions bindings test passed!")
        return True

    except Exception as e:
        print(f"  Transactions bindings test failed: {e}")
        traceback.print_exc()
        return False

def test_composition_bindings() -> None:
    """Test composition module bindings."""
    print("Testing composition bindings...")

    try:
        import qtforge.composition as composition

        # Test composition enums
        print("  Testing composition enums...")
        assert hasattr(composition, 'CompositionStrategy')
        assert hasattr(composition, 'PluginRole')

        # Test composition classes
        print("  Testing composition classes...")
        assert hasattr(composition, 'CompositionManager')

        # Test singleton access
        print("  Testing composition manager singleton...")
        manager = composition.get_composition_manager()
        assert manager is not None

        print("  Composition bindings test passed!")
        return True

    except Exception as e:
        print(f"  Composition bindings test failed: {e}")
        traceback.print_exc()
        return False

def test_marketplace_bindings() -> None:
    """Test marketplace module bindings."""
    print("Testing marketplace bindings...")

    try:
        import qtforge.marketplace as marketplace

        # Test marketplace classes
        print("  Testing marketplace classes...")
        assert hasattr(marketplace, 'PluginMarketplace')

        print("  Marketplace bindings test passed!")
        return True

    except Exception as e:
        print(f"  Marketplace bindings test failed: {e}")
        traceback.print_exc()
        return False

def main() -> None:
    """Run all binding tests."""
    print("QtForge Python Bindings Test Suite")
    print("=" * 40)

    tests = [
        test_core_bindings,
        test_communication_bindings,
        test_security_bindings,
        test_managers_bindings,
        test_monitoring_bindings,
        test_transactions_bindings,
        test_composition_bindings,
        test_marketplace_bindings,
    ]

    passed = 0
    failed = 0

    for test in tests:
        try:
            if test():
                passed += 1
            else:
                failed += 1
        except Exception as e:
            print(f"Test {test.__name__} crashed: {e}")
            failed += 1
        print()

    print("=" * 40)
    print(f"Test Results: {passed} passed, {failed} failed")

    if failed == 0:
        print("All tests passed!")
        return 0
    else:
        print("Some tests failed!")
        return 1

if __name__ == "__main__":
    sys.exit(main())
