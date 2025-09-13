#!/usr/bin/env python3
"""
QtForge v3.2.0 - Advanced Plugin Interfaces Example

This example demonstrates the new advanced plugin interfaces introduced in QtForge v3.2.0:
- IAdvancedPlugin: Service contracts and advanced communication
- IDynamicPlugin: Runtime interface adaptation and capability negotiation
- Service Contract System: Complete communication system with service discovery
- Enhanced Security: Advanced sandboxing and policy validation
"""

import sys
import os
from pathlib import Path
from typing import List, Dict, Any

try:
    import qtforge
    from qtforge.core import (
        IAdvancedPlugin, IDynamicPlugin, InterfaceCapability, InterfaceDescriptor,
        PluginType, PluginManager
    )
    from qtforge.communication import (
        ServiceContract, ServiceVersion, ServiceCapability, MessageBus
    )
    from qtforge.security import (
        PluginPermission, TrustLevel, SecurityPolicy, PluginSandbox
    )
    from qtforge.managers import (
        ConfigurationManager, ConfigurationScope, VersionManager
    )
except ImportError as e:
    print(f"Import error: {e}")
    print("Make sure QtForge v3.2.0 Python bindings are installed and accessible")
    sys.exit(1)

def demonstrate_advanced_plugin_interface() -> None:
    """Demonstrate IAdvancedPlugin interface features."""
    print("\n" + "=" * 60)
    print("Advanced Plugin Interface (IAdvancedPlugin) Demonstration")
    print("=" * 60)
    
    try:
        # Create an advanced plugin manager
        manager = qtforge.core.create_plugin_manager()
        print(f"Created plugin manager: {manager}")
        
        # Demonstrate service contract creation
        contract = ServiceContract()
        contract.name = "DataProcessingService"
        contract.version = ServiceVersion(2, 1, 0)
        contract.description = "Advanced data processing service"
        contract.capabilities = [
            ServiceCapability.DataTransformation,
            ServiceCapability.AsyncProcessing,
            ServiceCapability.BatchProcessing
        ]
        
        print(f"Created service contract: {contract.name} v{contract.version}")
        print(f"Service capabilities: {contract.capabilities}")
        
        # Demonstrate interface capabilities
        capability = InterfaceCapability()
        capability.name = "DataProcessor"
        capability.version = "2.1.0"
        capability.methods = ["process_data", "validate_input", "get_status"]
        capability.properties = ["processing_mode", "batch_size", "timeout"]
        
        print(f"Interface capability: {capability.name}")
        print(f"Available methods: {capability.methods}")
        print(f"Available properties: {capability.properties}")
        
    except Exception as e:
        print(f"Error demonstrating advanced plugin interface: {e}")

def demonstrate_dynamic_plugin_interface() -> None:
    """Demonstrate IDynamicPlugin interface features."""
    print("\n" + "=" * 60)
    print("Dynamic Plugin Interface (IDynamicPlugin) Demonstration")
    print("=" * 60)
    
    try:
        # Demonstrate interface descriptor
        descriptor = InterfaceDescriptor()
        descriptor.name = "DynamicDataProcessor"
        descriptor.version = "1.0.0"
        descriptor.description = "Dynamic interface for data processing"
        descriptor.supported_types = [
            PluginType.Native,
            PluginType.Python,
            PluginType.Lua
        ]
        
        print(f"Interface descriptor: {descriptor.name}")
        print(f"Supported plugin types: {descriptor.supported_types}")
        
        # Demonstrate capability negotiation
        print("\nCapability negotiation example:")
        requested_capabilities = ["data_processing", "async_support", "batch_mode"]
        available_capabilities = ["data_processing", "async_support", "validation", "logging"]
        
        negotiated = list(set(requested_capabilities) & set(available_capabilities))
        print(f"Requested: {requested_capabilities}")
        print(f"Available: {available_capabilities}")
        print(f"Negotiated: {negotiated}")
        
    except Exception as e:
        print(f"Error demonstrating dynamic plugin interface: {e}")

def demonstrate_enhanced_security() -> None:
    """Demonstrate enhanced security features in v3.2.0."""
    print("\n" + "=" * 60)
    print("Enhanced Security Features Demonstration")
    print("=" * 60)
    
    try:
        # Demonstrate plugin permissions
        permissions = [
            PluginPermission.FileSystemRead,
            PluginPermission.FileSystemWrite,
            PluginPermission.NetworkAccess,
            PluginPermission.SystemInfo
        ]
        
        print("Available plugin permissions:")
        for perm in permissions:
            print(f"  - {perm}")
        
        # Demonstrate trust levels
        trust_levels = [
            TrustLevel.Untrusted,
            TrustLevel.Low,
            TrustLevel.Medium,
            TrustLevel.High,
            TrustLevel.Trusted
        ]
        
        print("\nAvailable trust levels:")
        for level in trust_levels:
            print(f"  - {level}")
        
        # Create a security policy
        policy = SecurityPolicy()
        policy.name = "StandardPolicy"
        policy.description = "Standard security policy for plugins"
        policy.allowed_permissions = [
            PluginPermission.FileSystemRead,
            PluginPermission.SystemInfo
        ]
        policy.minimum_trust_level = TrustLevel.Medium
        
        print(f"\nCreated security policy: {policy.name}")
        print(f"Allowed permissions: {policy.allowed_permissions}")
        print(f"Minimum trust level: {policy.minimum_trust_level}")
        
        # Demonstrate plugin sandbox
        sandbox = PluginSandbox()
        sandbox.policy = policy
        sandbox.enable_resource_monitoring = True
        sandbox.max_memory_usage = 100 * 1024 * 1024  # 100MB
        sandbox.max_cpu_time = 30.0  # 30 seconds
        
        print(f"\nPlugin sandbox configuration:")
        print(f"  Resource monitoring: {sandbox.enable_resource_monitoring}")
        print(f"  Max memory: {sandbox.max_memory_usage / (1024*1024):.0f}MB")
        print(f"  Max CPU time: {sandbox.max_cpu_time}s")
        
    except Exception as e:
        print(f"Error demonstrating enhanced security: {e}")

def demonstrate_configuration_management() -> None:
    """Demonstrate enhanced configuration management in v3.2.0."""
    print("\n" + "=" * 60)
    print("Enhanced Configuration Management Demonstration")
    print("=" * 60)
    
    try:
        # Create configuration manager with scoped access
        config = qtforge.managers.create_configuration_manager()
        print(f"Created configuration manager: {config}")
        
        # Demonstrate configuration scopes
        scopes = [
            ConfigurationScope.Global,
            ConfigurationScope.User,
            ConfigurationScope.Plugin,
            ConfigurationScope.Session
        ]
        
        print("\nAvailable configuration scopes:")
        for scope in scopes:
            print(f"  - {scope}")
        
        # Set configuration values with different scopes
        config.set_value("app.name", "QtForge Example", ConfigurationScope.Global)
        config.set_value("user.theme", "dark", ConfigurationScope.User)
        config.set_value("plugin.timeout", 30, ConfigurationScope.Plugin)
        config.set_value("session.id", "12345", ConfigurationScope.Session)
        
        print("\nConfiguration values set:")
        print(f"  Global - app.name: {config.get_value('app.name', ConfigurationScope.Global)}")
        print(f"  User - user.theme: {config.get_value('user.theme', ConfigurationScope.User)}")
        print(f"  Plugin - plugin.timeout: {config.get_value('plugin.timeout', ConfigurationScope.Plugin)}")
        print(f"  Session - session.id: {config.get_value('session.id', ConfigurationScope.Session)}")
        
    except Exception as e:
        print(f"Error demonstrating configuration management: {e}")

def demonstrate_version_management() -> None:
    """Demonstrate enhanced version management in v3.2.0."""
    print("\n" + "=" * 60)
    print("Enhanced Version Management Demonstration")
    print("=" * 60)
    
    try:
        # Create version manager
        version_mgr = qtforge.managers.create_version_manager()
        print(f"Created version manager: {version_mgr}")
        
        # Demonstrate version tracking
        plugin_id = "com.example.testplugin"
        versions = ["1.0.0", "1.1.0", "1.2.0", "2.0.0"]
        
        print(f"\nTracking versions for plugin: {plugin_id}")
        for version in versions:
            version_mgr.register_version(plugin_id, version)
            print(f"  Registered version: {version}")
        
        # Get version information
        current_version = version_mgr.get_current_version(plugin_id)
        available_versions = version_mgr.get_available_versions(plugin_id)
        
        print(f"\nCurrent version: {current_version}")
        print(f"Available versions: {available_versions}")
        
        # Demonstrate version compatibility
        is_compatible = version_mgr.is_compatible(plugin_id, "1.1.0", "2.0.0")
        print(f"Version 1.1.0 compatible with 2.0.0: {is_compatible}")
        
    except Exception as e:
        print(f"Error demonstrating version management: {e}")

def main() -> None:
    """Main function demonstrating QtForge v3.2.0 advanced features."""
    print("QtForge v3.2.0 - Advanced Plugin Interfaces Example")
    print("This example demonstrates the new features introduced in QtForge v3.2.0")
    
    try:
        # Test basic connection
        print(f"\nConnection test: {qtforge.test_connection()}")
        print(f"QtForge version: {qtforge.get_version()}")
        
        # Demonstrate new features
        demonstrate_advanced_plugin_interface()
        demonstrate_dynamic_plugin_interface()
        demonstrate_enhanced_security()
        demonstrate_configuration_management()
        demonstrate_version_management()
        
        print("\n" + "=" * 60)
        print("QtForge v3.2.0 advanced features demonstration completed!")
        print("=" * 60)
        
    except Exception as e:
        print(f"Error during demonstration: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
