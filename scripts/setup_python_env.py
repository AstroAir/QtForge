#!/usr/bin/env python3
"""
QtForge Python Environment Setup Script

This script sets up the proper environment for QtForge Python bindings,
including PATH configuration, dependency checking, and troubleshooting.
"""

import os
import platform
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Tuple


class QtForgeEnvironmentSetup:
    """Setup and configure QtForge Python environment."""

    def __init__(self):
        self.project_root = Path(__file__).parent.parent
        self.build_dir = self.project_root / "build-test"
        self.python_dir = self.build_dir / "python"
        self.qt_paths = []
        self.missing_dlls = []

    def detect_qt_installation(self) -> List[Path]:
        """Detect Qt6 installation paths."""
        possible_qt_paths = []

        if platform.system() == "Windows":
            # Common Qt installation paths on Windows
            possible_paths = [
                Path("D:/msys64/mingw64/bin"),
                Path("C:/Qt/6.*/*/bin"),
                Path("C:/msys64/mingw64/bin"),
                Path("D:/Qt/6.*/*/bin"),
            ]

            for path_pattern in possible_paths:
                if "*" in str(path_pattern):
                    # Handle wildcard patterns
                    parent = path_pattern.parent.parent
                    if parent.exists():
                        for qt_dir in parent.glob("6.*"):
                            for subdir in qt_dir.iterdir():
                                bin_dir = subdir / "bin"
                                if (
                                    bin_dir.exists()
                                    and (bin_dir / "Qt6Core.dll").exists()
                                ):
                                    possible_qt_paths.append(bin_dir)
                elif path_pattern.exists() and (path_pattern / "Qt6Core.dll").exists():
                    possible_qt_paths.append(path_pattern)

        return possible_qt_paths

    def check_python_binding_dependencies(self) -> Dict[str, bool]:
        """Check if Python binding dependencies are available."""
        dependencies = {}

        # Check for Python binding file
        pyd_file = self.build_dir / "qtforge.cp313-win_amd64.pyd"
        dependencies["python_binding"] = pyd_file.exists()

        # Check for QtForge DLLs
        core_dll = self.build_dir / "libqtforge-core.dll"
        remote_dll = self.build_dir / "libqtforge-remote.dll"
        dependencies["qtforge_core_dll"] = core_dll.exists()
        dependencies["qtforge_remote_dll"] = remote_dll.exists()

        # Check for Python __init__.py
        init_file = self.python_dir / "__init__.py"
        dependencies["python_init"] = init_file.exists()

        return dependencies

    def setup_environment_variables(self) -> Dict[str, str]:
        """Set up environment variables for QtForge Python bindings."""
        env_vars = {}

        # Find Qt installation
        qt_paths = self.detect_qt_installation()
        if qt_paths:
            self.qt_paths = qt_paths
            # Add Qt bin directory to PATH
            current_path = os.environ.get("PATH", "")
            qt_path_str = str(qt_paths[0])
            if qt_path_str not in current_path:
                env_vars["PATH"] = f"{qt_path_str};{self.build_dir};{current_path}"

        # Set PYTHONPATH to include our Python bindings
        current_pythonpath = os.environ.get("PYTHONPATH", "")
        python_path_str = str(self.python_dir)
        if python_path_str not in current_pythonpath:
            if current_pythonpath:
                env_vars["PYTHONPATH"] = f"{python_path_str};{current_pythonpath}"
            else:
                env_vars["PYTHONPATH"] = python_path_str

        return env_vars

    def test_import(self) -> Tuple[bool, str]:
        """Test importing QtForge with proper environment."""
        env_vars = self.setup_environment_variables()

        # Create test script
        test_script = """
import sys
import os
try:
    import qtforge
    print("SUCCESS: QtForge imported successfully")
    print(f"Version: {qtforge.get_version()}")
    print(f"Available modules: {qtforge.list_available_modules()}")
    sys.exit(0)
except ImportError as e:
    print(f"IMPORT_ERROR: {e}")
    sys.exit(1)
except Exception as e:
    print(f"OTHER_ERROR: {e}")
    sys.exit(2)
"""

        try:
            # Run test with modified environment
            env = os.environ.copy()
            env.update(env_vars)

            result = subprocess.run(
                [sys.executable, "-c", test_script],
                check=False,
                capture_output=True,
                text=True,
                env=env,
                timeout=30,
            )

            if result.returncode == 0:
                return True, result.stdout
            return False, result.stdout + result.stderr

        except subprocess.TimeoutExpired:
            return False, "Test timed out"
        except Exception as e:
            return False, f"Test execution failed: {e}"

    def diagnose_issues(self) -> List[str]:
        """Diagnose common issues with Python bindings."""
        issues = []

        # Check dependencies
        deps = self.check_python_binding_dependencies()
        for dep_name, available in deps.items():
            if not available:
                issues.append(f"Missing dependency: {dep_name}")

        # Check Qt installation
        if not self.qt_paths:
            issues.append("Qt6 installation not found")

        # Check Python version compatibility
        if sys.version_info < (3, 8):
            issues.append(f"Python version {sys.version} is too old (requires 3.8+)")

        return issues

    def generate_setup_script(self) -> str:
        """Generate a setup script for the current environment."""
        env_vars = self.setup_environment_variables()

        if platform.system() == "Windows":
            script_lines = ["@echo off", "REM QtForge Python Environment Setup"]
            for var, value in env_vars.items():
                script_lines.append(f"set {var}={value}")
            script_lines.append("echo QtForge Python environment configured")
            script_lines.append("echo Testing import...")
            script_lines.append(
                "python -c \"import qtforge; print('SUCCESS:', qtforge.get_version())\""
            )
            return "\n".join(script_lines)
        script_lines = ["#!/bin/bash", "# QtForge Python Environment Setup"]
        for var, value in env_vars.items():
            script_lines.append(f"export {var}='{value}'")
        script_lines.append("echo 'QtForge Python environment configured'")
        script_lines.append("echo 'Testing import...'")
        script_lines.append(
            "python -c \"import qtforge; print('SUCCESS:', qtforge.get_version())\""
        )
        return "\n".join(script_lines)

    def run_setup(self) -> bool:
        """Run the complete setup process."""
        print("QtForge Python Environment Setup")
        print("=" * 50)

        # Check dependencies
        print("Checking dependencies...")
        deps = self.check_python_binding_dependencies()
        for dep_name, available in deps.items():
            status = "✓" if available else "✗"
            print(f"  {status} {dep_name}")

        # Detect Qt
        print("\nDetecting Qt installation...")
        qt_paths = self.detect_qt_installation()
        if qt_paths:
            print(f"  ✓ Found Qt6 at: {qt_paths[0]}")
            self.qt_paths = qt_paths
        else:
            print("  ✗ Qt6 not found")

        # Test import
        print("\nTesting QtForge import...")
        success, message = self.test_import()
        if success:
            print("  ✓ QtForge import successful")
            print(f"  {message}")
            return True
        print("  ✗ QtForge import failed")
        print(f"  {message}")

        # Diagnose issues
        print("\nDiagnosing issues...")
        issues = self.diagnose_issues()
        for issue in issues:
            print(f"  - {issue}")

        return False


def main():
    """Main entry point."""
    setup = QtForgeEnvironmentSetup()

    if len(sys.argv) > 1 and sys.argv[1] == "--generate-script":
        # Generate setup script
        script_content = setup.generate_setup_script()
        script_name = (
            "setup_qtforge_env.bat"
            if platform.system() == "Windows"
            else "setup_qtforge_env.sh"
        )

        with open(script_name, "w") as f:
            f.write(script_content)

        print(f"Generated setup script: {script_name}")
        return 0

    # Run setup
    success = setup.run_setup()
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
