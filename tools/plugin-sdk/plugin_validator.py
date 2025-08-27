#!/usr/bin/env python3
"""
QtForge Plugin Validator
A comprehensive tool for validating plugin projects, metadata, and interfaces.
"""

import argparse
import json
import os
import sys
from pathlib import Path
from typing import Dict, List, Optional, Any, Tuple
import re
import subprocess

class ValidationResult:
    """Represents the result of a validation check"""

    def __init__(self, check_name: str, passed: bool, message: str, severity: str = "error"):
        self.check_name = check_name
        self.passed = passed
        self.message = message
        self.severity = severity  # "error", "warning", "info"

    def __str__(self):
        status = "✅" if self.passed else ("⚠️" if self.severity == "warning" else "❌")
        return f"{status} {self.check_name}: {self.message}"

class PluginValidator:
    """Main plugin validator class"""

    def __init__(self):
        self.results = []

    def validate_plugin(self, plugin_path: Path) -> List[ValidationResult]:
        """Validate a plugin project"""
        self.results = []

        # Basic structure validation
        self._validate_structure(plugin_path)

        # Metadata validation
        self._validate_metadata(plugin_path)

        # Source code validation
        self._validate_source_code(plugin_path)

        # CMake validation (for native plugins)
        self._validate_cmake(plugin_path)

        # Interface validation
        self._validate_interfaces(plugin_path)

        return self.results

    def _add_result(self, check_name: str, passed: bool, message: str, severity: str = "error"):
        """Add a validation result"""
        result = ValidationResult(check_name, passed, message, severity)
        self.results.append(result)

    def _validate_structure(self, plugin_path: Path):
        """Validate plugin directory structure"""
        # Check if directory exists
        if not plugin_path.exists():
            self._add_result("Directory Exists", False, f"Plugin directory does not exist: {plugin_path}")
            return

        if not plugin_path.is_dir():
            self._add_result("Directory Valid", False, f"Path is not a directory: {plugin_path}")
            return

        self._add_result("Directory Exists", True, "Plugin directory exists")

        # Check for metadata.json
        metadata_file = plugin_path / "metadata.json"
        if metadata_file.exists():
            self._add_result("Metadata File", True, "metadata.json found")
        else:
            self._add_result("Metadata File", False, "metadata.json not found")

        # Check for source files
        has_cpp_files = any(plugin_path.glob("**/*.cpp"))
        has_hpp_files = any(plugin_path.glob("**/*.hpp"))
        has_py_files = any(plugin_path.glob("**/*.py"))

        if has_cpp_files or has_hpp_files:
            self._add_result("Source Files", True, "C++ source files found")

            # Check for typical C++ structure
            src_dir = plugin_path / "src"
            include_dir = plugin_path / "include"

            if src_dir.exists():
                self._add_result("Source Directory", True, "src/ directory found")
            else:
                self._add_result("Source Directory", False, "src/ directory not found", "warning")

            if include_dir.exists():
                self._add_result("Include Directory", True, "include/ directory found")
            else:
                self._add_result("Include Directory", False, "include/ directory not found", "warning")

        elif has_py_files:
            self._add_result("Source Files", True, "Python source files found")
        else:
            self._add_result("Source Files", False, "No source files found")

    def _validate_metadata(self, plugin_path: Path):
        """Validate plugin metadata"""
        metadata_file = plugin_path / "metadata.json"

        if not metadata_file.exists():
            return  # Already reported in structure validation

        try:
            with open(metadata_file, 'r', encoding='utf-8') as f:
                metadata = json.load(f)

            self._add_result("Metadata Parse", True, "metadata.json is valid JSON")

            # Check required fields
            required_fields = ["id", "name", "version", "description", "author"]
            for field in required_fields:
                if field in metadata and metadata[field]:
                    self._add_result(f"Metadata {field.title()}", True, f"{field} field present")
                else:
                    self._add_result(f"Metadata {field.title()}", False, f"Missing or empty {field} field")

            # Validate plugin ID format
            if "id" in metadata:
                plugin_id = metadata["id"]
                if re.match(r'^[a-z0-9]+(\.[a-z0-9]+)*$', plugin_id):
                    self._add_result("Plugin ID Format", True, "Plugin ID format is valid")
                else:
                    self._add_result("Plugin ID Format", False,
                                   "Plugin ID should use reverse domain notation (e.g., com.example.plugin)")

            # Validate version format
            if "version" in metadata:
                version = metadata["version"]
                if re.match(r'^\d+\.\d+\.\d+(-[a-zA-Z0-9]+)?$', version):
                    self._add_result("Version Format", True, "Version format is valid")
                else:
                    self._add_result("Version Format", False,
                                   "Version should follow semantic versioning (e.g., 1.0.0)")

            # Check optional but recommended fields
            recommended_fields = ["license", "category", "capabilities"]
            for field in recommended_fields:
                if field in metadata and metadata[field]:
                    self._add_result(f"Metadata {field.title()}", True, f"{field} field present", "info")
                else:
                    self._add_result(f"Metadata {field.title()}", False,
                                   f"Missing {field} field (recommended)", "warning")

            # Validate interfaces if present
            if "interfaces" in metadata and isinstance(metadata["interfaces"], list):
                self._add_result("Interfaces Defined", True,
                               f"{len(metadata['interfaces'])} interface(s) defined")

                for i, interface in enumerate(metadata["interfaces"]):
                    if isinstance(interface, dict):
                        if "id" in interface and "version" in interface:
                            self._add_result(f"Interface {i+1}", True,
                                           f"Interface {interface['id']} is valid")
                        else:
                            self._add_result(f"Interface {i+1}", False,
                                           "Interface missing id or version")
                    else:
                        self._add_result(f"Interface {i+1}", False, "Interface is not an object")

        except json.JSONDecodeError as e:
            self._add_result("Metadata Parse", False, f"Invalid JSON: {e}")
        except Exception as e:
            self._add_result("Metadata Parse", False, f"Error reading metadata: {e}")

    def _validate_source_code(self, plugin_path: Path):
        """Validate source code"""
        # Find source files
        cpp_files = list(plugin_path.glob("**/*.cpp"))
        hpp_files = list(plugin_path.glob("**/*.hpp"))
        py_files = list(plugin_path.glob("**/*.py"))

        if cpp_files or hpp_files:
            self._validate_cpp_code(cpp_files, hpp_files)

        if py_files:
            self._validate_python_code(py_files)

    def _validate_cpp_code(self, cpp_files: List[Path], hpp_files: List[Path]):
        """Validate C++ source code"""
        # Check for plugin interface implementation
        found_plugin_interface = False
        found_q_object = False
        found_q_interfaces = False

        for hpp_file in hpp_files:
            try:
                content = hpp_file.read_text(encoding='utf-8')

                # Check for plugin interface
                if "IPlugin" in content or "IDynamicPlugin" in content:
                    found_plugin_interface = True

                # Check for Qt plugin macros
                if "Q_OBJECT" in content:
                    found_q_object = True

                if "Q_INTERFACES" in content:
                    found_q_interfaces = True

            except Exception as e:
                self._add_result("Source Code Read", False, f"Error reading {hpp_file}: {e}")

        if found_plugin_interface:
            self._add_result("Plugin Interface", True, "Plugin interface implementation found")
        else:
            self._add_result("Plugin Interface", False, "No plugin interface implementation found")

        if found_q_object:
            self._add_result("Qt Object", True, "Q_OBJECT macro found")
        else:
            self._add_result("Qt Object", False, "Q_OBJECT macro not found", "warning")

        if found_q_interfaces:
            self._add_result("Qt Interfaces", True, "Q_INTERFACES macro found")
        else:
            self._add_result("Qt Interfaces", False, "Q_INTERFACES macro not found", "warning")

        # Check for proper includes
        required_includes = ["qtplugin/core/plugin_interface.hpp", "QObject"]
        for cpp_file in cpp_files + hpp_files:
            try:
                content = cpp_file.read_text(encoding='utf-8')
                for include in required_includes:
                    if include in content:
                        self._add_result(f"Include {include}", True, f"Required include found in {cpp_file.name}")
                        break
            except Exception:
                pass

    def _validate_python_code(self, py_files: List[Path]):
        """Validate Python source code"""
        found_plugin_class = False

        for py_file in py_files:
            try:
                content = py_file.read_text(encoding='utf-8')

                # Check for plugin class methods
                required_methods = ["initialize", "shutdown", "execute_command"]
                method_count = sum(1 for method in required_methods if f"def {method}" in content)

                if method_count >= len(required_methods):
                    found_plugin_class = True
                    self._add_result("Plugin Methods", True, f"Plugin methods found in {py_file.name}")

                # Check for encoding declaration in the first two lines
                first_two_lines = "\n".join(content.splitlines()[:2])
                if re.search(r"#.*coding[:=]\s*([-\w.]+)", first_two_lines):
                    self._add_result("File Encoding", True, f"Encoding declaration found in {py_file.name}")

            except Exception as e:
                self._add_result("Python Code Read", False, f"Error reading {py_file}: {e}")

        if not found_plugin_class:
            self._add_result("Plugin Class", False, "No plugin class with required methods found")

    def _validate_cmake(self, plugin_path: Path):
        """Validate CMakeLists.txt"""
        cmake_file = plugin_path / "CMakeLists.txt"

        if not cmake_file.exists():
            self._add_result("CMakeLists.txt", False, "CMakeLists.txt not found", "warning")
            return

        try:
            content = cmake_file.read_text(encoding='utf-8')

            # Check for required CMake commands
            required_commands = ["cmake_minimum_required", "project", "add_library"]
            for command in required_commands:
                if command in content:
                    self._add_result(f"CMake {command}", True, f"{command} found")
                else:
                    self._add_result(f"CMake {command}", False, f"{command} not found")

            # Check for Qt and QtForge dependencies
            if "find_package(Qt6" in content:
                self._add_result("Qt Dependency", True, "Qt6 dependency found")
            else:
                self._add_result("Qt Dependency", False, "Qt6 dependency not found", "warning")

            if "QtForge" in content:
                self._add_result("QtForge Dependency", True, "QtForge dependency found")
            else:
                self._add_result("QtForge Dependency", False, "QtForge dependency not found", "warning")

            # Check for plugin-specific settings
            if ".qtplugin" in content:
                self._add_result("Plugin Extension", True, "Plugin file extension configured")
            else:
                self._add_result("Plugin Extension", False, "Plugin file extension not configured", "warning")

        except Exception as e:
            self._add_result("CMakeLists.txt Read", False, f"Error reading CMakeLists.txt: {e}")

    def _validate_interfaces(self, plugin_path: Path):
        """Validate plugin interfaces"""
        metadata_file = plugin_path / "metadata.json"

        if not metadata_file.exists():
            return

        try:
            with open(metadata_file, 'r', encoding='utf-8') as f:
                metadata = json.load(f)

            if "interfaces" not in metadata:
                self._add_result("Interface Definition", False, "No interfaces defined in metadata", "warning")
                return

            interfaces = metadata["interfaces"]
            if not isinstance(interfaces, list) or len(interfaces) == 0:
                self._add_result("Interface Definition", False, "No valid interfaces defined", "warning")
                return

            # Validate each interface
            for i, interface in enumerate(interfaces):
                if not isinstance(interface, dict):
                    self._add_result(f"Interface {i+1} Format", False, "Interface is not an object")
                    continue

                # Check required fields
                required_fields = ["id", "version"]
                for field in required_fields:
                    if field not in interface:
                        self._add_result(f"Interface {i+1} {field.title()}", False,
                                       f"Interface missing {field} field")

                # Validate interface ID format
                if "id" in interface:
                    interface_id = interface["id"]
                    if re.match(r'^[a-z0-9]+(\.[a-z0-9]+)*$', interface_id):
                        self._add_result(f"Interface {i+1} ID Format", True, "Interface ID format is valid")
                    else:
                        self._add_result(f"Interface {i+1} ID Format", False,
                                       "Interface ID should use dot notation")

                # Check for description
                if "description" in interface and interface["description"]:
                    self._add_result(f"Interface {i+1} Description", True, "Interface has description", "info")
                else:
                    self._add_result(f"Interface {i+1} Description", False,
                                   "Interface missing description", "warning")

        except Exception as e:
            self._add_result("Interface Validation", False, f"Error validating interfaces: {e}")

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="QtForge Plugin Validator - Validate plugin projects and metadata"
    )

    parser.add_argument("plugin_path", type=Path, help="Path to plugin directory")
    parser.add_argument("--format", choices=["text", "json"], default="text",
                       help="Output format")
    parser.add_argument("--severity", choices=["error", "warning", "info"], default="error",
                       help="Minimum severity level to report")
    parser.add_argument("--quiet", "-q", action="store_true",
                       help="Only show failed checks")

    args = parser.parse_args()

    validator = PluginValidator()
    results = validator.validate_plugin(args.plugin_path)

    # Filter results by severity
    severity_levels = {"error": 0, "warning": 1, "info": 2}
    min_level = severity_levels[args.severity]

    filtered_results = [
        result for result in results
        if severity_levels.get(result.severity, 0) <= min_level
    ]

    if args.quiet:
        filtered_results = [result for result in filtered_results if not result.passed]

    # Output results
    if args.format == "json":
        output = {
            "plugin_path": str(args.plugin_path),
            "total_checks": len(results),
            "passed_checks": sum(1 for r in results if r.passed),
            "failed_checks": sum(1 for r in results if not r.passed),
            "results": [
                {
                    "check_name": r.check_name,
                    "passed": r.passed,
                    "message": r.message,
                    "severity": r.severity
                }
                for r in filtered_results
            ]
        }
        print(json.dumps(output, indent=2))
    else:
        print(f"🔍 Validating plugin: {args.plugin_path}")
        print(f"📊 Total checks: {len(results)}")
        print(f"✅ Passed: {sum(1 for r in results if r.passed)}")
        print(f"❌ Failed: {sum(1 for r in results if not r.passed)}")
        print()

        for result in filtered_results:
            print(result)

    # Return exit code based on validation results
    failed_errors = sum(1 for r in results if not r.passed and r.severity == "error")
    return 1 if failed_errors > 0 else 0

if __name__ == "__main__":
    sys.exit(main())
