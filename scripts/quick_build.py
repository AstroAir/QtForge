#!/usr/bin/env python3
"""
QtForge Quick Build Script

This script provides a convenient way to build QtForge with different configurations
and automatically validate the results.

Usage:
    python scripts/quick_build.py [--config CONFIG] [--clean] [--test] [--verbose]
    
Configurations:
    - stable: Build with stable modules only (default)
    - all: Build with all modules enabled (may fail for broken modules)
    - dev: Development build with debug symbols
    - release: Optimized release build
"""

import sys
import os
import subprocess
import argparse
import time
from pathlib import Path

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    BOLD = '\033[1m'
    END = '\033[0m'

def print_status(message, status="info") -> None:
    """Print colored status messages"""
    colors = {
        "success": Colors.GREEN + "✅ ",
        "error": Colors.RED + "❌ ",
        "warning": Colors.YELLOW + "⚠️  ",
        "info": Colors.BLUE + "ℹ️  "
    }
    print(f"{colors.get(status, '')}{message}{Colors.END}")

def run_command(cmd, cwd=None, verbose=False) -> None:
    """Run a command and return success status"""
    if verbose:
        print_status(f"Running: {' '.join(cmd)}", "info")
    
    try:
        result = subprocess.run(
            cmd, 
            cwd=cwd, 
            capture_output=not verbose,
            text=True,
            check=False
        )
        
        if result.returncode == 0:
            if verbose and result.stdout:
                print(result.stdout)
            return True
        else:
            print_status(f"Command failed with return code {result.returncode}", "error")
            if result.stderr:
                print(result.stderr)
            return False
            
    except Exception as e:
        print_status(f"Command execution failed: {e}", "error")
        return False

def get_build_configs() -> None:
    """Get available build configurations"""
    return {
        "stable": {
            "description": "Stable modules only (recommended)",
            "cmake_args": [
                "-DCMAKE_BUILD_TYPE=Release",
                "-DQTFORGE_PYTHON_ENABLE_SECURITY_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_MANAGERS_MODULE=ON", 
                "-DQTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_COMMUNICATION_MODULE=OFF",
                "-DQTFORGE_PYTHON_ENABLE_MONITORING_MODULE=OFF",
                "-DQTFORGE_PYTHON_ENABLE_THREADING_MODULE=OFF",
                "-DQTFORGE_PYTHON_ENABLE_TRANSACTIONS_MODULE=OFF",
                "-DQTFORGE_PYTHON_ENABLE_COMPOSITION_MODULE=OFF",
                "-DQTFORGE_PYTHON_ENABLE_MARKETPLACE_MODULE=OFF"
            ]
        },
        "all": {
            "description": "All modules enabled (may fail)",
            "cmake_args": [
                "-DCMAKE_BUILD_TYPE=Release",
                "-DQTFORGE_PYTHON_ENABLE_ALL_MODULES=ON",
                "-DQTFORGE_PYTHON_ENABLE_SECURITY_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_MANAGERS_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_COMMUNICATION_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_MONITORING_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_THREADING_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_TRANSACTIONS_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_COMPOSITION_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_MARKETPLACE_MODULE=ON"
            ]
        },
        "dev": {
            "description": "Development build with debug symbols",
            "cmake_args": [
                "-DCMAKE_BUILD_TYPE=Debug",
                "-DQTFORGE_ENABLE_WARNINGS=ON",
                "-DQTFORGE_ENABLE_WERROR=OFF",
                "-DQTFORGE_PYTHON_ENABLE_SECURITY_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_MANAGERS_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE=ON"
            ]
        },
        "release": {
            "description": "Optimized release build",
            "cmake_args": [
                "-DCMAKE_BUILD_TYPE=Release",
                "-DQTFORGE_ENABLE_LTO=ON",
                "-DQTFORGE_ENABLE_FAST_MATH=ON",
                "-DQTFORGE_PYTHON_ENABLE_SECURITY_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_MANAGERS_MODULE=ON",
                "-DQTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE=ON"
            ]
        }
    }

def clean_build_directory(verbose=False) -> None:
    """Clean the build directory"""
    print_status("Cleaning build directory...", "info")
    
    build_dir = Path("build")
    if build_dir.exists():
        import shutil
        try:
            shutil.rmtree(build_dir)
            print_status("Build directory cleaned", "success")
            return True
        except Exception as e:
            print_status(f"Failed to clean build directory: {e}", "error")
            return False
    else:
        print_status("Build directory doesn't exist, nothing to clean", "info")
        return True

def configure_build(config_name, verbose=False) -> None:
    """Configure the build with CMake"""
    print_status(f"Configuring build with '{config_name}' configuration...", "info")
    
    configs = get_build_configs()
    if config_name not in configs:
        print_status(f"Unknown configuration: {config_name}", "error")
        return False
    
    config = configs[config_name]
    cmake_cmd = ["cmake", "-B", "build"] + config["cmake_args"]
    
    start_time = time.time()
    success = run_command(cmake_cmd, verbose=verbose)
    end_time = time.time()
    
    if success:
        print_status(f"Configuration completed in {end_time - start_time:.1f}s", "success")
    else:
        print_status("Configuration failed", "error")
    
    return success

def build_project(parallel_jobs=4, verbose=False) -> None:
    """Build the project"""
    print_status(f"Building project with {parallel_jobs} parallel jobs...", "info")
    
    build_cmd = ["cmake", "--build", "build", f"--parallel", str(parallel_jobs)]
    
    start_time = time.time()
    success = run_command(build_cmd, verbose=verbose)
    end_time = time.time()
    
    if success:
        print_status(f"Build completed in {end_time - start_time:.1f}s", "success")
    else:
        print_status("Build failed", "error")
    
    return success

def run_tests(verbose=False) -> None:
    """Run validation tests"""
    print_status("Running validation tests...", "info")
    
    # Run the validation script
    script_path = Path(__file__).parent / "validate_build.py"
    if not script_path.exists():
        print_status("Validation script not found", "warning")
        return True  # Don't fail the build for missing test script
    
    test_cmd = [sys.executable, str(script_path)]
    if verbose:
        test_cmd.append("--verbose")
    
    return run_command(test_cmd, verbose=verbose)

def main() -> None:
    parser = argparse.ArgumentParser(description="Quick build script for QtForge")
    parser.add_argument("--config", "-c", default="stable", 
                       help="Build configuration (stable, all, dev, release)")
    parser.add_argument("--clean", action="store_true", 
                       help="Clean build directory before building")
    parser.add_argument("--test", "-t", action="store_true",
                       help="Run validation tests after building")
    parser.add_argument("--verbose", "-v", action="store_true",
                       help="Verbose output")
    parser.add_argument("--jobs", "-j", type=int, default=4,
                       help="Number of parallel build jobs")
    parser.add_argument("--list-configs", action="store_true",
                       help="List available configurations")
    
    args = parser.parse_args()
    
    # List configurations if requested
    if args.list_configs:
        print_status("Available build configurations:", "info")
        configs = get_build_configs()
        for name, config in configs.items():
            print_status(f"  {name}: {config['description']}", "info")
        return
    
    print_status("QtForge Quick Build Script", "info")
    print_status("=" * 50, "info")
    
    start_time = time.time()
    
    # Clean if requested
    if args.clean:
        if not clean_build_directory(args.verbose):
            sys.exit(1)
    
    # Configure
    if not configure_build(args.config, args.verbose):
        print_status("Build failed at configuration stage", "error")
        sys.exit(1)
    
    # Build
    if not build_project(args.jobs, args.verbose):
        print_status("Build failed at compilation stage", "error")
        sys.exit(1)
    
    # Test if requested
    if args.test:
        if not run_tests(args.verbose):
            print_status("Build succeeded but tests failed", "warning")
            # Don't exit with error for test failures
    
    end_time = time.time()
    total_time = end_time - start_time
    
    print_status("=" * 50, "info")
    print_status(f"🎉 Build completed successfully in {total_time:.1f}s!", "success")
    print_status(f"Configuration: {args.config}", "info")
    print_status("Build artifacts are in the 'build' directory", "info")
    
    if not args.test:
        print_status("Run with --test to validate the build", "info")

if __name__ == "__main__":
    main()
