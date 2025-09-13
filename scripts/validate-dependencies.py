#!/usr/bin/env python3
"""
QtForge Dependency Validation Script
Validates build dependencies, versions, and system requirements
"""

import os
import sys
import subprocess
import platform
import shutil
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import json
import re

class DependencyValidator:
    """Validates build dependencies and system requirements"""
    
    def __init__(self) -> None:
        self.system = platform.system().lower()
        self.arch = platform.machine().lower()
        self.errors = []
        self.warnings = []
        self.requirements = self._load_requirements()
    
    def _load_requirements(self) -> Dict:
        """Load dependency requirements from configuration"""
        return {
            "cmake": {
                "min_version": "3.21.0",
                "required": True,
                "description": "CMake build system"
            },
            "qt6": {
                "min_version": "6.0.0",
                "required": True,
                "description": "Qt6 framework",
                "components": ["Core", "Network", "Widgets", "Sql"]
            },
            "ninja": {
                "min_version": "1.10.0",
                "required": False,
                "description": "Ninja build system (recommended)"
            },
            "python": {
                "min_version": "3.8.0",
                "required": False,
                "description": "Python interpreter (for advanced features)"
            }
        }
    
    def _run_command(self, cmd: List[str]) -> Tuple[bool, str, str]:
        """Run command and return success, stdout, stderr"""
        try:
            result = subprocess.run(
                cmd, 
                capture_output=True, 
                text=True, 
                timeout=30
            )
            return result.returncode == 0, result.stdout, result.stderr
        except (subprocess.TimeoutExpired, FileNotFoundError) as e:
            return False, "", str(e)
    
    def _parse_version(self, version_str: str) -> Tuple[int, int, int]:
        """Parse version string into tuple of integers"""
        # Extract version numbers using regex
        match = re.search(r'(\d+)\.(\d+)\.(\d+)', version_str)
        if match:
            return tuple(map(int, match.groups()))
        # Fallback for simpler version formats
        match = re.search(r'(\d+)\.(\d+)', version_str)
        if match:
            return tuple(map(int, match.groups())) + (0,)
        return (0, 0, 0)
    
    def _compare_versions(self, current: str, required: str) -> bool:
        """Compare version strings, return True if current >= required"""
        current_tuple = self._parse_version(current)
        required_tuple = self._parse_version(required)
        return current_tuple >= required_tuple
    
    def validate_cmake(self) -> bool:
        """Validate CMake installation and version"""
        if not shutil.which("cmake"):
            self.errors.append("CMake not found in PATH")
            return False
        
        success, stdout, stderr = self._run_command(["cmake", "--version"])
        if not success:
            self.errors.append(f"Failed to get CMake version: {stderr}")
            return False
        
        version_line = stdout.split('\n')[0]
        version_match = re.search(r'cmake version (\d+\.\d+\.\d+)', version_line)
        if not version_match:
            self.errors.append("Unable to parse CMake version")
            return False
        
        current_version = version_match.group(1)
        required_version = self.requirements["cmake"]["min_version"]
        
        if not self._compare_versions(current_version, required_version):
            self.errors.append(
                f"CMake version {current_version} is too old. "
                f"Required: {required_version} or newer"
            )
            return False
        
        print(f"✓ CMake {current_version} found")
        return True
    
    def validate_qt6(self) -> bool:
        """Validate Qt6 installation"""
        # Try to find Qt6 using various methods
        qt_found = False
        qt_version = None
        qt_path = None
        
        # Method 1: Check for qmake6
        if shutil.which("qmake6"):
            success, stdout, stderr = self._run_command(["qmake6", "-query", "QT_VERSION"])
            if success:
                qt_version = stdout.strip()
                qt_found = True
                success, stdout, stderr = self._run_command(["qmake6", "-query", "QT_INSTALL_PREFIX"])
                if success:
                    qt_path = stdout.strip()
        
        # Method 2: Check for qmake
        elif shutil.which("qmake"):
            success, stdout, stderr = self._run_command(["qmake", "-query", "QT_VERSION"])
            if success:
                version = stdout.strip()
                if version.startswith("6."):
                    qt_version = version
                    qt_found = True
                    success, stdout, stderr = self._run_command(["qmake", "-query", "QT_INSTALL_PREFIX"])
                    if success:
                        qt_path = stdout.strip()
        
        # Method 3: Check common installation paths
        if not qt_found:
            common_paths = []
            if self.system == "windows":
                common_paths = [
                    "C:/Qt/6.*/msvc*/",
                    "C:/Qt/*/6.*/msvc*/"
                ]
            elif self.system == "darwin":
                common_paths = [
                    "/usr/local/Qt-6.*",
                    "/opt/homebrew/Cellar/qt@6/*"
                ]
            elif self.system == "linux":
                common_paths = [
                    "/usr/lib/qt6",
                    "/usr/local/Qt-6.*"
                ]
            
            # This is a simplified check - in practice, you'd use glob
            for path_pattern in common_paths:
                # Simplified path checking
                if "Qt" in path_pattern:
                    self.warnings.append(f"Please ensure Qt6 is installed and accessible")
                    break
        
        if not qt_found:
            self.errors.append(
                "Qt6 not found. Please install Qt6 and ensure qmake6 is in PATH"
            )
            return False
        
        required_version = self.requirements["qt6"]["min_version"]
        if not self._compare_versions(qt_version, required_version):
            self.errors.append(
                f"Qt version {qt_version} is too old. "
                f"Required: {required_version} or newer"
            )
            return False
        
        print(f"✓ Qt {qt_version} found at {qt_path}")
        return True
    
    def validate_ninja(self) -> bool:
        """Validate Ninja build system (optional)"""
        if not shutil.which("ninja"):
            self.warnings.append("Ninja not found - using default generator")
            return False
        
        success, stdout, stderr = self._run_command(["ninja", "--version"])
        if not success:
            self.warnings.append("Failed to get Ninja version")
            return False
        
        version = stdout.strip()
        required_version = self.requirements["ninja"]["min_version"]
        
        if not self._compare_versions(version, required_version):
            self.warnings.append(
                f"Ninja version {version} is old. "
                f"Recommended: {required_version} or newer"
            )
        else:
            print(f"✓ Ninja {version} found")
        
        return True
    
    def validate_python(self) -> bool:
        """Validate Python installation (optional)"""
        python_cmd = "python3" if shutil.which("python3") else "python"
        
        if not shutil.which(python_cmd):
            self.warnings.append("Python not found - some features may be unavailable")
            return False
        
        success, stdout, stderr = self._run_command([python_cmd, "--version"])
        if not success:
            self.warnings.append("Failed to get Python version")
            return False
        
        version_match = re.search(r'Python (\d+\.\d+\.\d+)', stdout)
        if not version_match:
            self.warnings.append("Unable to parse Python version")
            return False
        
        version = version_match.group(1)
        required_version = self.requirements["python"]["min_version"]
        
        if not self._compare_versions(version, required_version):
            self.warnings.append(
                f"Python version {version} is old. "
                f"Recommended: {required_version} or newer"
            )
        else:
            print(f"✓ Python {version} found")
        
        return True
    
    def validate_system_requirements(self) -> bool:
        """Validate system-specific requirements"""
        print(f"System: {self.system} ({self.arch})")
        
        # Check available memory (simplified)
        try:
            if self.system == "linux":
                with open("/proc/meminfo", "r") as f:
                    for line in f:
                        if line.startswith("MemTotal:"):
                            mem_kb = int(line.split()[1])
                            mem_gb = mem_kb / (1024 * 1024)
                            if mem_gb < 4:
                                self.warnings.append(
                                    f"Low memory detected ({mem_gb:.1f}GB). "
                                    "Consider using fewer parallel jobs."
                                )
                            break
        except Exception:
            pass  # Memory check is optional
        
        return True
    
    def validate_all(self) -> bool:
        """Run all validations"""
        print("🔍 Validating build dependencies...")
        print("=" * 50)
        
        # Required dependencies
        cmake_ok = self.validate_cmake()
        qt_ok = self.validate_qt6()
        
        # Optional dependencies
        self.validate_ninja()
        self.validate_python()
        
        # System requirements
        self.validate_system_requirements()
        
        print("=" * 50)
        
        # Report results
        if self.warnings:
            print("⚠️  Warnings:")
            for warning in self.warnings:
                print(f"   • {warning}")
            print()
        
        if self.errors:
            print("❌ Errors:")
            for error in self.errors:
                print(f"   • {error}")
            print()
            print("Please fix the above errors before building.")
            return False
        
        print("✅ All required dependencies validated successfully!")
        return True

def main() -> None:
    """Main entry point"""
    validator = DependencyValidator()
    
    if not validator.validate_all():
        sys.exit(1)
    
    print("\n🚀 Ready to build QtForge!")
    return 0

if __name__ == "__main__":
    sys.exit(main())
