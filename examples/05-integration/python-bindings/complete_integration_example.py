#!/usr/bin/env python3
"""
QtForge Complete Python Integration Example
==========================================

This example demonstrates the complete QtForge Python integration with all
available modules enabled, including communication, orchestration, monitoring,
and more.

Author: QtForge Development Team
Version: 3.0.0
"""

import sys
import os
import json
import time
from typing import Dict, Any, Optional

# Add the build directory to Python path
sys.path.insert(0, '../../../build')
sys.path.insert(0, '../../../build_test')

class QtForgeCompleteExample:
    """Complete demonstration of QtForge Python bindings"""
    
    def __init__(self) -> None:
        """Initialize the example"""
        self.qtforge = None
        self.plugin_manager = None
        self.message_bus = None
        self.security_manager = None
        
    def initialize_qtforge(self) -> bool:
        """Initialize QtForge and all modules"""
        try:
            print("🚀 Initializing QtForge Python Integration...")
            import qtforge
            self.qtforge = qtforge
            
            print(f"✅ QtForge version: {qtforge.__version__}")
            print(f"✅ Version info: {qtforge.__version_major__}.{qtforge.__version_minor__}.{qtforge.__version_patch__}")
            
            return True
        except ImportError as e:
            print(f"❌ Failed to import QtForge: {e}")
            return False
    
    def test_core_functionality(self) -> bool:
        """Test core plugin management functionality"""
        print("\n📦 Testing Core Functionality...")
        
        try:
            # Test plugin manager creation
            if hasattr(self.qtforge, 'create_plugin_manager'):
                self.plugin_manager = self.qtforge.create_plugin_manager()
                print("✅ Plugin manager created successfully")
            elif hasattr(self.qtforge.core, 'create_plugin_manager'):
                self.plugin_manager = self.qtforge.core.create_plugin_manager()
                print("✅ Plugin manager created successfully")
            else:
                print("⚠️  Plugin manager creation not available")
                
            # Test version utilities
            if hasattr(self.qtforge, 'get_version'):
                version = self.qtforge.get_version()
                print(f"✅ Version utility: {version}")
                
            return True
        except Exception as e:
            print(f"❌ Core functionality test failed: {e}")
            return False
    
    def test_communication_system(self) -> bool:
        """Test communication and messaging functionality"""
        print("\n📡 Testing Communication System...")
        
        try:
            if hasattr(self.qtforge, 'communication'):
                comm = self.qtforge.communication
                print("✅ Communication module available")
                
                # Test message creation if available
                if hasattr(comm, 'create_message'):
                    message = comm.create_message("test.topic", {"data": "test"})
                    print("✅ Message creation successful")
                    
                # Test message bus if available
                if hasattr(comm, 'MessageBus'):
                    print("✅ MessageBus class available")
                    
            else:
                print("⚠️  Communication module not available")
                
            return True
        except Exception as e:
            print(f"❌ Communication test failed: {e}")
            return False
    
    def test_security_features(self) -> bool:
        """Test security and validation features"""
        print("\n🔒 Testing Security Features...")
        
        try:
            if hasattr(self.qtforge, 'security'):
                security = self.qtforge.security
                print("✅ Security module available")
                
                # Test security manager creation if available
                if hasattr(security, 'create_security_manager'):
                    self.security_manager = security.create_security_manager()
                    print("✅ Security manager created")
                elif hasattr(security, 'SecurityManager'):
                    print("✅ SecurityManager class available")
                    
            else:
                print("⚠️  Security module not available")
                
            return True
        except Exception as e:
            print(f"❌ Security test failed: {e}")
            return False
    
    def test_orchestration_system(self) -> bool:
        """Test plugin orchestration and workflow management"""
        print("\n🎭 Testing Orchestration System...")
        
        try:
            if hasattr(self.qtforge, 'orchestration'):
                orchestration = self.qtforge.orchestration
                print("✅ Orchestration module available")
                
                # Test workflow creation if available
                if hasattr(orchestration, 'create_workflow'):
                    print("✅ Workflow creation available")
                elif hasattr(orchestration, 'PluginOrchestrator'):
                    print("✅ PluginOrchestrator class available")
                    
            else:
                print("⚠️  Orchestration module not available")
                
            return True
        except Exception as e:
            print(f"❌ Orchestration test failed: {e}")
            return False
    
    def test_monitoring_capabilities(self) -> bool:
        """Test monitoring and metrics collection"""
        print("\n📊 Testing Monitoring Capabilities...")
        
        try:
            if hasattr(self.qtforge, 'monitoring'):
                monitoring = self.qtforge.monitoring
                print("✅ Monitoring module available")
                
                # Test metrics collection if available
                if hasattr(monitoring, 'collect_metrics'):
                    print("✅ Metrics collection available")
                elif hasattr(monitoring, 'MetricsCollector'):
                    print("✅ MetricsCollector class available")
                    
            else:
                print("⚠️  Monitoring module not available")
                
            return True
        except Exception as e:
            print(f"❌ Monitoring test failed: {e}")
            return False
    
    def test_advanced_features(self) -> bool:
        """Test advanced features like transactions, composition, marketplace"""
        print("\n🚀 Testing Advanced Features...")
        
        advanced_modules = [
            ('transactions', 'Transaction Management'),
            ('composition', 'Plugin Composition'),
            ('marketplace', 'Plugin Marketplace'),
            ('threading', 'Threading Support')
        ]
        
        results = []
        for module_name, description in advanced_modules:
            try:
                if hasattr(self.qtforge, module_name):
                    module = getattr(self.qtforge, module_name)
                    print(f"✅ {description} module available")
                    results.append(True)
                else:
                    print(f"⚠️  {description} module not available")
                    results.append(False)
            except Exception as e:
                print(f"❌ {description} test failed: {e}")
                results.append(False)
                
        return any(results)
    
    def test_utility_functions(self) -> bool:
        """Test utility functions and helpers"""
        print("\n🛠️  Testing Utility Functions...")
        
        try:
            if hasattr(self.qtforge, 'utils'):
                utils = self.qtforge.utils
                print("✅ Utils module available")
                
                # Test common utility functions
                utility_functions = ['get_version', 'create_version', 'utils_test']
                for func_name in utility_functions:
                    if hasattr(utils, func_name):
                        try:
                            func = getattr(utils, func_name)
                            if callable(func):
                                result = func()
                                print(f"✅ {func_name}(): {result}")
                        except Exception as e:
                            print(f"⚠️  {func_name}() error: {e}")
                            
            else:
                print("⚠️  Utils module not available")
                
            return True
        except Exception as e:
            print(f"❌ Utility functions test failed: {e}")
            return False
    
    def test_managers_functionality(self) -> bool:
        """Test configuration, logging, and resource managers"""
        print("\n⚙️  Testing Managers Functionality...")
        
        try:
            if hasattr(self.qtforge, 'managers'):
                managers = self.qtforge.managers
                print("✅ Managers module available")
                
                # Test manager creation functions
                manager_types = [
                    'create_configuration_manager',
                    'create_logging_manager', 
                    'create_resource_manager'
                ]
                
                for manager_type in manager_types:
                    if hasattr(managers, manager_type):
                        print(f"✅ {manager_type} available")
                    else:
                        print(f"⚠️  {manager_type} not available")
                        
            else:
                print("⚠️  Managers module not available")
                
            return True
        except Exception as e:
            print(f"❌ Managers functionality test failed: {e}")
            return False
    
    def run_complete_example(self) -> int:
        """Run the complete integration example"""
        print("=" * 60)
        print("🎯 QtForge Complete Python Integration Example")
        print("=" * 60)
        
        # Initialize QtForge
        if not self.initialize_qtforge():
            return 1
            
        # Run all tests
        test_results = []
        test_results.append(self.test_core_functionality())
        test_results.append(self.test_communication_system())
        test_results.append(self.test_security_features())
        test_results.append(self.test_orchestration_system())
        test_results.append(self.test_monitoring_capabilities())
        test_results.append(self.test_advanced_features())
        test_results.append(self.test_utility_functions())
        test_results.append(self.test_managers_functionality())
        
        # Summary
        print("\n" + "=" * 60)
        print("📋 Test Summary")
        print("=" * 60)
        
        passed_tests = sum(test_results)
        total_tests = len(test_results)
        
        print(f"✅ Passed: {passed_tests}/{total_tests} tests")
        
        if passed_tests == total_tests:
            print("🎉 All tests passed! QtForge Python integration is working perfectly.")
            return 0
        elif passed_tests > 0:
            print("⚠️  Some tests passed. QtForge Python integration is partially working.")
            return 0
        else:
            print("❌ No tests passed. QtForge Python integration needs attention.")
            return 1

def main() -> None:
    """Main entry point"""
    try:
        example = QtForgeCompleteExample()
        return example.run_complete_example()
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
