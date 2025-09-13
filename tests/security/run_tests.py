#!/usr/bin/env python3
"""
Test runner script for QtForge Sandbox System
Provides comprehensive test execution with reporting and analysis
"""

import os
import sys
import subprocess
import argparse
import json
import time
from pathlib import Path
from typing import List, Dict, Any, Optional


class TestResult:
    def __init__(self, name: str, passed: bool, duration: float, output: str = "", error: str = "") -> None:
        self.name = name
        self.passed = passed
        self.duration = duration
        self.output = output
        self.error = error


class TestRunner:
    def __init__(self, build_dir: str, verbose: bool = False) -> None:
        self.build_dir = Path(build_dir)
        self.verbose = verbose
        self.results: List[TestResult] = []

        # Test executables
        self.test_executables = {
            "unit": [
                "test_plugin_sandbox",
                "test_resource_monitor",
                "test_security_enforcer",
                "test_sandbox_manager"
            ],
            "integration": [
                "test_sandbox_integration"
            ],
            "performance": [
                "test_sandbox_performance"
            ]
        }

    def run_test(self, test_name: str) -> TestResult:
        """Run a single test executable"""
        test_path = self.build_dir / "tests" / "security" / test_name

        if not test_path.exists():
            test_path = self.build_dir / test_name

        if not test_path.exists():
            return TestResult(test_name, False, 0.0, "", f"Test executable not found: {test_path}")

        print(f"Running {test_name}...")

        start_time = time.time()
        try:
            result = subprocess.run(
                [str(test_path)],
                capture_output=True,
                text=True,
                timeout=300  # 5 minute timeout
            )
            duration = time.time() - start_time

            passed = result.returncode == 0
            output = result.stdout
            error = result.stderr

            if self.verbose:
                print(f"  Duration: {duration:.2f}s")
                print(f"  Return code: {result.returncode}")
                if output:
                    print(f"  Output: {output[:200]}...")
                if error:
                    print(f"  Error: {error[:200]}...")

            return TestResult(test_name, passed, duration, output, error)

        except subprocess.TimeoutExpired:
            duration = time.time() - start_time
            return TestResult(test_name, False, duration, "", "Test timed out")
        except Exception as e:
            duration = time.time() - start_time
            return TestResult(test_name, False, duration, "", str(e))

    def run_test_category(self, category: str) -> List[TestResult]:
        """Run all tests in a category"""
        if category not in self.test_executables:
            print(f"Unknown test category: {category}")
            return []

        print(f"\n=== Running {category.upper()} Tests ===")
        results = []

        for test_name in self.test_executables[category]:
            result = self.run_test(test_name)
            results.append(result)
            self.results.append(result)

            status = "PASS" if result.passed else "FAIL"
            print(f"  {test_name}: {status} ({result.duration:.2f}s)")

            if not result.passed and result.error:
                print(f"    Error: {result.error}")

        return results

    def run_all_tests(self) -> Dict[str, List[TestResult]]:
        """Run all test categories"""
        all_results = {}

        for category in self.test_executables.keys():
            all_results[category] = self.run_test_category(category)

        return all_results

    def generate_report(self) -> Dict[str, Any]:
        """Generate a comprehensive test report"""
        total_tests = len(self.results)
        passed_tests = sum(1 for r in self.results if r.passed)
        failed_tests = total_tests - passed_tests
        total_duration = sum(r.duration for r in self.results)

        report = {
            "summary": {
                "total_tests": total_tests,
                "passed": passed_tests,
                "failed": failed_tests,
                "success_rate": (passed_tests / total_tests * 100) if total_tests > 0 else 0,
                "total_duration": total_duration
            },
            "results": []
        }

        for result in self.results:
            report["results"].append({
                "name": result.name,
                "passed": result.passed,
                "duration": result.duration,
                "error": result.error if not result.passed else None
            })

        return report

    def print_summary(self) -> None:
        """Print test summary"""
        total_tests = len(self.results)
        passed_tests = sum(1 for r in self.results if r.passed)
        failed_tests = total_tests - passed_tests
        total_duration = sum(r.duration for r in self.results)

        print(f"\n{'='*60}")
        print(f"TEST SUMMARY")
        print(f"{'='*60}")
        print(f"Total Tests:    {total_tests}")
        print(f"Passed:         {passed_tests}")
        print(f"Failed:         {failed_tests}")
        print(
            f"Success Rate:   {(passed_tests/total_tests*100):.1f}%" if total_tests > 0 else "N/A")
        print(f"Total Duration: {total_duration:.2f}s")

        if failed_tests > 0:
            print(f"\nFAILED TESTS:")
            for result in self.results:
                if not result.passed:
                    print(f"  - {result.name}: {result.error}")

        print(f"{'='*60}")

    def save_report(self, filename: str) -> None:
        """Save test report to JSON file"""
        report = self.generate_report()

        with open(filename, 'w') as f:
            json.dump(report, f, indent=2)

        print(f"Test report saved to: {filename}")


def check_build_environment(build_dir: Path) -> bool:
    """Check if the build environment is properly set up"""
    if not build_dir.exists():
        print(f"Build directory does not exist: {build_dir}")
        return False

    # Check for at least one test executable
    test_dir = build_dir / "tests" / "security"
    if not test_dir.exists():
        test_dir = build_dir

    found_tests = False
    for test_file in ["test_plugin_sandbox", "test_resource_monitor"]:
        if (test_dir / test_file).exists() or (test_dir / f"{test_file}.exe").exists():
            found_tests = True
            break

    if not found_tests:
        print(f"No test executables found in: {test_dir}")
        print("Please build the tests first using CMake")
        return False

    return True


def main() -> None:
    parser = argparse.ArgumentParser(description="Run QtForge Sandbox Tests")
    parser.add_argument("--build-dir", "-b", default="build",
                        help="Build directory containing test executables")
    parser.add_argument("--category", "-c", choices=["unit", "integration", "performance", "all"],
                        default="all", help="Test category to run")
    parser.add_argument("--verbose", "-v", action="store_true",
                        help="Enable verbose output")
    parser.add_argument("--report", "-r", help="Save test report to JSON file")
    parser.add_argument("--list", "-l", action="store_true",
                        help="List available tests")

    args = parser.parse_args()

    build_dir = Path(args.build_dir).resolve()

    if not check_build_environment(build_dir):
        return 1

    runner = TestRunner(build_dir, args.verbose)

    if args.list:
        print("Available test categories:")
        for category, tests in runner.test_executables.items():
            print(f"  {category}:")
            for test in tests:
                print(f"    - {test}")
        return 0

    # Run tests
    if args.category == "all":
        runner.run_all_tests()
    else:
        runner.run_test_category(args.category)

    # Print summary
    runner.print_summary()

    # Save report if requested
    if args.report:
        runner.save_report(args.report)

    # Return appropriate exit code
    failed_tests = sum(1 for r in runner.results if not r.passed)
    return 1 if failed_tests > 0 else 0


if __name__ == "__main__":
    sys.exit(main())
