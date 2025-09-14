#!/usr/bin/env python3
"""
Test data generator for QtForge tests.
Generates consistent test configurations, metadata, and mock data.
"""

import json
import uuid
import os
from pathlib import Path
from typing import Dict, Any, List, Optional
from datetime import datetime, timedelta


class TestDataGenerator:
    """Generates test data for QtForge testing."""
    
    def __init__(self, output_dir: str = "test_data"):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(exist_ok=True)
    
    def generate_plugin_metadata(self, 
                                name: str = "TestPlugin",
                                version: str = "1.0.0",
                                plugin_type: str = "native",
                                **kwargs) -> Dict[str, Any]:
        """Generate plugin metadata JSON."""
        
        metadata = {
            "id": str(uuid.uuid4()),
            "name": name,
            "version": version,
            "type": plugin_type,
            "description": kwargs.get("description", f"Test plugin {name}"),
            "author": kwargs.get("author", "QtForge Test Suite"),
            "license": kwargs.get("license", "MIT"),
            "created": datetime.now().isoformat(),
            "api_version": kwargs.get("api_version", "3.2.0"),
            
            "dependencies": kwargs.get("dependencies", {}),
            
            "capabilities": {
                "supports_hot_reload": kwargs.get("hot_reload", True),
                "thread_safe": kwargs.get("thread_safe", True),
                "supports_configuration": kwargs.get("configurable", True),
                "supports_events": kwargs.get("events", True)
            },
            
            "interfaces": kwargs.get("interfaces", ["IPlugin"]),
            
            "configuration_schema": {
                "type": "object",
                "properties": {
                    "enabled": {"type": "boolean", "default": True},
                    "log_level": {"type": "string", "default": "info"},
                    "timeout": {"type": "integer", "default": 30000}
                }
            },
            
            "commands": kwargs.get("commands", ["status", "configure", "test"]),
            
            "resources": {
                "memory_limit": kwargs.get("memory_limit", "100MB"),
                "cpu_limit": kwargs.get("cpu_limit", "50%"),
                "file_access": kwargs.get("file_access", ["read", "write"])
            }
        }
        
        return metadata
    
    def generate_configuration(self, 
                             scope: str = "global",
                             plugin_id: Optional[str] = None) -> Dict[str, Any]:
        """Generate test configuration data."""
        
        config = {
            "scope": scope,
            "plugin_id": plugin_id,
            "timestamp": datetime.now().isoformat(),
            
            "logging": {
                "level": "debug",
                "output": ["console", "file"],
                "file_path": "test_logs.log",
                "max_file_size": "10MB",
                "rotation": True
            },
            
            "security": {
                "validation_enabled": True,
                "signature_verification": True,
                "sandbox_enabled": True,
                "trust_level": "medium"
            },
            
            "performance": {
                "max_plugins": 100,
                "load_timeout": 30000,
                "hot_reload_enabled": True,
                "metrics_collection": True
            },
            
            "communication": {
                "message_bus_enabled": True,
                "max_message_size": "1MB",
                "message_timeout": 5000,
                "compression_enabled": False
            }
        }
        
        if plugin_id:
            config["plugin_specific"] = {
                "enabled": True,
                "auto_start": True,
                "priority": "normal",
                "custom_settings": {}
            }
        
        return config
    
    def generate_test_plugins(self, count: int = 5) -> List[Dict[str, Any]]:
        """Generate multiple test plugin metadata."""
        
        plugins = []
        plugin_types = ["native", "python", "lua", "remote"]
        
        for i in range(count):
            plugin_type = plugin_types[i % len(plugin_types)]
            name = f"TestPlugin{i+1}"
            
            # Add some variety to the plugins
            kwargs = {}
            if i % 2 == 0:
                kwargs["hot_reload"] = False
            if i % 3 == 0:
                kwargs["thread_safe"] = False
                kwargs["interfaces"] = ["IPlugin", "IAdvancedPlugin"]
            
            plugin = self.generate_plugin_metadata(
                name=name,
                version=f"1.{i}.0",
                plugin_type=plugin_type,
                **kwargs
            )
            
            plugins.append(plugin)
        
        return plugins
    
    def generate_performance_test_data(self) -> Dict[str, Any]:
        """Generate data for performance testing."""
        
        return {
            "test_scenarios": [
                {
                    "name": "load_many_plugins",
                    "plugin_count": 50,
                    "concurrent_loads": True,
                    "expected_max_time": 5000
                },
                {
                    "name": "message_throughput",
                    "message_count": 1000,
                    "message_size": 1024,
                    "expected_min_throughput": 100
                },
                {
                    "name": "memory_usage",
                    "plugin_count": 20,
                    "expected_max_memory": "500MB",
                    "leak_tolerance": "1MB"
                }
            ],
            
            "benchmarks": {
                "plugin_load_time": {"target": 100, "unit": "ms"},
                "message_latency": {"target": 10, "unit": "ms"},
                "memory_overhead": {"target": 50, "unit": "MB"}
            }
        }
    
    def generate_security_test_data(self) -> Dict[str, Any]:
        """Generate data for security testing."""
        
        return {
            "valid_signatures": [
                "valid_signature_1.sig",
                "valid_signature_2.sig"
            ],
            
            "invalid_signatures": [
                "invalid_signature.sig",
                "corrupted_signature.sig"
            ],
            
            "malicious_plugins": [
                {
                    "name": "malicious_plugin_1",
                    "threat_type": "file_access_violation",
                    "expected_block": True
                },
                {
                    "name": "malicious_plugin_2", 
                    "threat_type": "network_access_violation",
                    "expected_block": True
                }
            ],
            
            "security_policies": [
                {
                    "name": "strict_policy",
                    "file_access": "none",
                    "network_access": "none",
                    "system_access": "none"
                },
                {
                    "name": "permissive_policy",
                    "file_access": "read_write",
                    "network_access": "full",
                    "system_access": "limited"
                }
            ]
        }
    
    def save_test_data(self, data: Dict[str, Any], filename: str) -> Path:
        """Save test data to JSON file."""
        
        filepath = self.output_dir / filename
        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        
        return filepath
    
    def generate_all_test_data(self) -> Dict[str, Path]:
        """Generate all test data files."""
        
        generated_files = {}
        
        # Plugin metadata
        plugins = self.generate_test_plugins(10)
        generated_files["plugins"] = self.save_test_data(
            {"plugins": plugins}, "test_plugins.json"
        )
        
        # Configurations
        global_config = self.generate_configuration("global")
        generated_files["global_config"] = self.save_test_data(
            global_config, "global_config.json"
        )
        
        plugin_config = self.generate_configuration("plugin", "test_plugin_1")
        generated_files["plugin_config"] = self.save_test_data(
            plugin_config, "plugin_config.json"
        )
        
        # Performance test data
        perf_data = self.generate_performance_test_data()
        generated_files["performance"] = self.save_test_data(
            perf_data, "performance_test_data.json"
        )
        
        # Security test data
        security_data = self.generate_security_test_data()
        generated_files["security"] = self.save_test_data(
            security_data, "security_test_data.json"
        )
        
        return generated_files


def main():
    """Generate all test data."""
    
    generator = TestDataGenerator()
    
    print("Generating QtForge test data...")
    generated_files = generator.generate_all_test_data()
    
    print("\nGenerated test data files:")
    for name, filepath in generated_files.items():
        print(f"  {name}: {filepath}")
    
    print(f"\nAll test data saved to: {generator.output_dir}")


if __name__ == "__main__":
    main()
