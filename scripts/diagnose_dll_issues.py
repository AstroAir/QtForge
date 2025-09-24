#!/usr/bin/env python3
"""
DLL Dependency Diagnostic Tool for QtForge Python Bindings

This script helps diagnose DLL loading issues by checking dependencies,
architecture compatibility, and providing detailed error information.
"""

import os
import platform
import subprocess
import sys
from pathlib import Path
from typing import List, Optional


def run_command(cmd: List[str], timeout: int = 30) -> tuple[bool, str]:
    """Run a command and return success status and output."""
    try:
        result = subprocess.run(
            cmd,
            check=False,
            capture_output=True,
            text=True,
            timeout=timeout,
            shell=True if platform.system() == "Windows" else False,
        )
        return result.returncode == 0, result.stdout + result.stderr
    except subprocess.TimeoutExpired:
        return False, "Command timed out"
    except Exception as e:
        return False, f"Command failed: {e}"


def check_file_architecture(file_path: Path) -> Optional[str]:
    """Check the architecture of a DLL/PYD file."""
    if not file_path.exists():
        return None

    if platform.system() == "Windows":
        # Try using file command if available
        success, output = run_command(["file", str(file_path)])
        if success and "PE32+" in output:
            return "x64"
        if success and "PE32" in output and "PE32+" not in output:
            return "x86"

        # Fallback: check file size and common patterns
        file_size = file_path.stat().st_size
        if file_size > 1000000:  # Large files are likely x64
            return "x64 (estimated)"
        return "unknown"

    return "unknown"


def find_missing_dependencies(pyd_file: Path) -> List[str]:
    """Find missing DLL dependencies for a PYD file."""
    missing_deps = []

    if platform.system() == "Windows":
        # Try using dependency walker or similar tools
        # For now, we'll check common Qt dependencies manually
        common_qt_deps = [
            "Qt6Core.dll",
            "Qt6Gui.dll",
            "Qt6Network.dll",
            "Qt6Widgets.dll",
            "libgcc_s_seh-1.dll",
            "libstdc++-6.dll",
            "libwinpthread-1.dll",
        ]

        # Check if these DLLs are in PATH
        path_dirs = os.environ.get("PATH", "").split(";")

        for dep in common_qt_deps:
            found = False
            for path_dir in path_dirs:
                if path_dir and Path(path_dir) / dep:
                    dll_path = Path(path_dir) / dep
                    if dll_path.exists():
                        found = True
                        break

            if not found:
                missing_deps.append(dep)

    return missing_deps


def test_minimal_import() -> tuple[bool, str]:
    """Test minimal Python import to isolate issues."""
    test_scripts = [
        # Test 1: Basic Python functionality
        ("Basic Python", "print('Python works')"),
        # Test 2: Import sys and os
        ("System modules", "import sys, os; print('System modules work')"),
        # Test 3: Try importing pybind11 (if available)
        (
            "Pybind11",
            "try:\n    import pybind11\n    print('pybind11 available')\nexcept ImportError:\n    print('pybind11 not available')",
        ),
        # Test 4: Try importing Qt (if available)
        (
            "Qt Python",
            "try:\n    from PyQt6 import QtCore\n    print('PyQt6 available')\nexcept ImportError:\n    try:\n        from PySide6 import QtCore\n        print('PySide6 available')\n    except ImportError:\n        print('No Qt Python bindings found')",
        ),
    ]

    results = []
    for test_name, script in test_scripts:
        success, output = run_command([sys.executable, "-c", script])
        status = "✓" if success else "✗"
        results.append(f"{status} {test_name}: {output.strip()}")

    return True, "\n".join(results)


def create_minimal_test_module() -> bool:
    """Create a minimal test module to verify pybind11 functionality."""
    test_cpp = """
#include <pybind11/pybind11.h>

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(test_minimal, m) {
    m.doc() = "Minimal test module";
    m.def("add", &add, "A function which adds two numbers");
}
"""

    # Save test module
    test_dir = Path("build-test") / "test_minimal"
    test_dir.mkdir(exist_ok=True)

    with open(test_dir / "test_minimal.cpp", "w") as f:
        f.write(test_cpp)

    return True


def main():
    """Main diagnostic function."""
    print("QtForge Python Bindings - DLL Diagnostic Tool")
    print("=" * 60)

    project_root = Path(__file__).parent.parent
    build_dir = project_root / "build-test"
    pyd_file = build_dir / "qtforge.cp313-win_amd64.pyd"

    # Basic file checks
    print("1. File Existence Check:")
    print(f"   PYD file: {'✓' if pyd_file.exists() else '✗'} {pyd_file}")
    print(f"   Size: {pyd_file.stat().st_size if pyd_file.exists() else 'N/A'} bytes")

    # Architecture check
    print("\n2. Architecture Check:")
    arch = check_file_architecture(pyd_file)
    print(f"   PYD architecture: {arch}")
    print(f"   Python architecture: {platform.architecture()[0]}")
    print(f"   System architecture: {platform.machine()}")

    # Python version check
    print("\n3. Python Version Check:")
    print(f"   Python version: {sys.version}")
    print(f"   Python executable: {sys.executable}")

    # Environment check
    print("\n4. Environment Check:")
    path_entries = os.environ.get("PATH", "").split(";")[:5]  # Show first 5 entries
    print("   PATH (first 5 entries):")
    for entry in path_entries:
        if entry:
            print(f"     {entry}")

    pythonpath = os.environ.get("PYTHONPATH", "Not set")
    print(f"   PYTHONPATH: {pythonpath}")

    # Dependency check
    print("\n5. Dependency Check:")
    missing_deps = find_missing_dependencies(pyd_file)
    if missing_deps:
        print("   Missing dependencies:")
        for dep in missing_deps:
            print(f"     ✗ {dep}")
    else:
        print("   ✓ All common dependencies appear to be available")

    # Test minimal imports
    print("\n6. Minimal Import Tests:")
    success, results = test_minimal_import()
    print(results)

    # Final import test with detailed error
    print("\n7. QtForge Import Test:")
    try:
        # Set up environment
        env = os.environ.copy()
        env["PATH"] = f"D:\\msys64\\mingw64\\bin;{build_dir};{env.get('PATH', '')}"
        env["PYTHONPATH"] = str(build_dir / "python")

        # Try import with detailed error reporting
        test_script = (
            '''
import sys
import traceback
sys.path.insert(0, r"'''
            + str(build_dir / "python")
            + """")

try:
    print("Attempting to import qtforge...")
    import qtforge
    print("SUCCESS: QtForge imported!")
    print(f"Version: {qtforge.get_version()}")
except ImportError as e:
    print(f"ImportError: {e}")
    print("Traceback:")
    traceback.print_exc()
except Exception as e:
    print(f"Other error: {e}")
    print("Traceback:")
    traceback.print_exc()
"""
        )

        result = subprocess.run(
            [sys.executable, "-c", test_script],
            check=False,
            capture_output=True,
            text=True,
            env=env,
            timeout=30,
        )

        print(result.stdout)
        if result.stderr:
            print("STDERR:", result.stderr)

    except Exception as e:
        print(f"   Test execution failed: {e}")

    # Recommendations
    print("\n8. Recommendations:")
    print("   - Ensure Qt6 DLLs are in PATH")
    print("   - Check that Python and QtForge bindings have matching architecture")
    print("   - Verify all MinGW runtime dependencies are available")
    print("   - Consider rebuilding Python bindings with current environment")

    return 0


if __name__ == "__main__":
    sys.exit(main())
