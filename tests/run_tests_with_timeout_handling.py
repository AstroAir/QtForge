#!/usr/bin/env python3
"""
Enhanced test runner for QtForge with timeout handling and better error reporting.
Handles test failures gracefully and provides detailed diagnostics.
"""

import subprocess
import sys
import os
import time
import signal
import json
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import argparse


class TestResult:
    """Represents the result of a single test."""
    
    def __init__(self, name: str, status: str, duration: float = 0.0, 
                 output: str = "", error: str = ""):
        self.name = name
        self.status = status  # "PASSED", "FAILED", "TIMEOUT", "SKIPPED"
        self.duration = duration
        self.output = output
        self.error = error


class QtForgeTestRunner:
    """Enhanced test runner with timeout handling."""
    
    def __init__(self, build_dir: str = "build", timeout_multiplier: float = 1.0):
        self.build_dir = Path(build_dir)
        self.timeout_multiplier = timeout_multiplier
        self.results: List[TestResult] = []
        
        # Test categories with different timeout strategies
        self.test_categories = {
            "core": {"timeout": 30, "critical": True},
            "communication": {"timeout": 30, "critical": True},
            "security": {"timeout": 60, "critical": False},  # Less critical due to sandbox complexity
            "python_bridge": {"timeout": 20, "critical": False},  # Reduced timeout
            "integration": {"timeout": 90, "critical": False},
            "performance": {"timeout": 120, "critical": False}
        }
    
    def run_single_test(self, test_name: str, timeout: int = 30) -> TestResult:
        """Run a single test with timeout handling."""
        print(f"Running test: {test_name} (timeout: {timeout}s)")
        
        start_time = time.time()
        try:
            # Run the test with timeout
            result = subprocess.run(
                ["ctest", "--test-dir", str(self.build_dir), "-R", f"^{test_name}$", 
                 "--output-on-failure", "--verbose"],
                capture_output=True,
                text=True,
                timeout=timeout * self.timeout_multiplier
            )
            
            duration = time.time() - start_time
            
            if result.returncode == 0:
                return TestResult(test_name, "PASSED", duration, result.stdout)
            else:
                return TestResult(test_name, "FAILED", duration, result.stdout, result.stderr)
                
        except subprocess.TimeoutExpired:
            duration = time.time() - start_time
            return TestResult(test_name, "TIMEOUT", duration, "", f"Test timed out after {timeout}s")
        except Exception as e:
            duration = time.time() - start_time
            return TestResult(test_name, "ERROR", duration, "", str(e))
    
    def get_test_list(self) -> List[str]:
        """Get list of available tests."""
        try:
            result = subprocess.run(
                ["ctest", "--test-dir", str(self.build_dir), "--show-only=json-v1"],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode == 0:
                data = json.loads(result.stdout)
                return [test["name"] for test in data.get("tests", [])]
            else:
                print(f"Failed to get test list: {result.stderr}")
                return []
        except Exception as e:
            print(f"Error getting test list: {e}")
            return []
    
    def categorize_test(self, test_name: str) -> Tuple[str, Dict]:
        """Categorize a test and return its configuration."""
        test_lower = test_name.lower()
        
        if "core" in test_lower or "plugin" in test_lower:
            return "core", self.test_categories["core"]
        elif "communication" in test_lower or "message" in test_lower:
            return "communication", self.test_categories["communication"]
        elif "security" in test_lower or "sandbox" in test_lower:
            return "security", self.test_categories["security"]
        elif "python" in test_lower or "bridge" in test_lower:
            return "python_bridge", self.test_categories["python_bridge"]
        elif "integration" in test_lower:
            return "integration", self.test_categories["integration"]
        elif "performance" in test_lower or "benchmark" in test_lower:
            return "performance", self.test_categories["performance"]
        else:
            return "core", self.test_categories["core"]  # Default to core
    
    def run_all_tests(self, skip_non_critical: bool = False) -> bool:
        """Run all tests with appropriate timeouts."""
        tests = self.get_test_list()
        if not tests:
            print("No tests found!")
            return False
        
        print(f"Found {len(tests)} tests")
        
        passed = 0
        failed = 0
        timeouts = 0
        skipped = 0
        
        for test_name in tests:
            category, config = self.categorize_test(test_name)
            
            if skip_non_critical and not config["critical"]:
                print(f"Skipping non-critical test: {test_name}")
                skipped += 1
                continue
            
            result = self.run_single_test(test_name, config["timeout"])
            self.results.append(result)
            
            if result.status == "PASSED":
                print(f"✅ {test_name} - PASSED ({result.duration:.2f}s)")
                passed += 1
            elif result.status == "TIMEOUT":
                print(f"⏰ {test_name} - TIMEOUT ({result.duration:.2f}s)")
                timeouts += 1
            elif result.status == "FAILED":
                print(f"❌ {test_name} - FAILED ({result.duration:.2f}s)")
                if result.error:
                    print(f"   Error: {result.error[:200]}...")
                failed += 1
            else:
                print(f"⚠️  {test_name} - {result.status}")
                failed += 1
        
        # Print summary
        total = passed + failed + timeouts + skipped
        print(f"\n{'='*60}")
        print(f"Test Summary:")
        print(f"  Total: {total}")
        print(f"  Passed: {passed}")
        print(f"  Failed: {failed}")
        print(f"  Timeouts: {timeouts}")
        print(f"  Skipped: {skipped}")
        print(f"  Success Rate: {(passed/max(1, total-skipped))*100:.1f}%")
        
        return failed == 0 and timeouts == 0
    
    def run_critical_tests_only(self) -> bool:
        """Run only critical tests."""
        return self.run_all_tests(skip_non_critical=True)


def main():
    parser = argparse.ArgumentParser(description="QtForge Test Runner with Timeout Handling")
    parser.add_argument("--build-dir", default="build", help="Build directory")
    parser.add_argument("--timeout-multiplier", type=float, default=1.0, 
                       help="Multiply all timeouts by this factor")
    parser.add_argument("--critical-only", action="store_true", 
                       help="Run only critical tests")
    parser.add_argument("--test", help="Run specific test")
    
    args = parser.parse_args()
    
    runner = QtForgeTestRunner(args.build_dir, args.timeout_multiplier)
    
    if args.test:
        # Run specific test
        category, config = runner.categorize_test(args.test)
        result = runner.run_single_test(args.test, config["timeout"])
        print(f"Test {args.test}: {result.status}")
        return result.status == "PASSED"
    elif args.critical_only:
        return runner.run_critical_tests_only()
    else:
        return runner.run_all_tests()


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
