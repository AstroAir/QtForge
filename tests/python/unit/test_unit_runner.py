#!/usr/bin/env python3
"""
Unit test runner for QtForge Python bindings.
Runs all unit tests with comprehensive reporting and coverage analysis.
"""

import pytest
import sys
import os
import time
import subprocess
from pathlib import Path
from typing import List, Dict, Any

# Add the build directory to Python path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../build'))

try:
    import qtforge
    BINDINGS_AVAILABLE = True
    print("✅ QtForge bindings are available")
except ImportError as e:
    BINDINGS_AVAILABLE = False
    print(f"❌ QtForge bindings not available: {e}")


class UnitTestRunner:
    """Comprehensive unit test runner for QtForge Python bindings."""
    
    def __init__(self):
        self.test_dir = Path(__file__).parent
        self.test_modules = [
            "test_core_unit.py",
            "test_utils_unit.py", 
            "test_communication_unit.py",
            "test_security_unit.py",
            "test_managers_unit.py",
            "test_orchestration_unit.py",
            "test_monitoring_unit.py",
            "test_transactions_unit.py",
            "test_composition_unit.py",
            "test_marketplace_unit.py",
            "test_threading_unit.py"
        ]
        self.results = {}
    
    def check_test_files(self) -> Dict[str, bool]:
        """Check which test files exist."""
        existing_tests = {}
        missing_tests = []
        
        for test_module in self.test_modules:
            test_path = self.test_dir / test_module
            exists = test_path.exists()
            existing_tests[test_module] = exists
            if not exists:
                missing_tests.append(test_module)
        
        if missing_tests:
            print("⚠️  Some unit test modules are missing:")
            for missing in missing_tests:
                print(f"   - {missing}")
            print()
        
        return existing_tests
    
    def run_individual_test(self, test_file: str) -> Dict[str, Any]:
        """Run an individual test file and return results."""
        print(f"🧪 Running {test_file}...")
        
        test_path = self.test_dir / test_file
        if not test_path.exists():
            return {
                'status': 'skipped',
                'reason': 'File not found',
                'duration': 0,
                'tests_run': 0,
                'failures': 0,
                'errors': 0
            }
        
        start_time = time.time()
        
        # Run pytest on the specific file
        pytest_args = [
            str(test_path),
            "-v",
            "--tb=short",
            "--strict-markers",
            "-x",  # Stop on first failure for individual tests
            "--durations=5",  # Show 5 slowest tests
            "-q"  # Quiet output for cleaner results
        ]
        
        try:
            result = pytest.main(pytest_args)
            end_time = time.time()
            duration = end_time - start_time
            
            # Parse pytest exit codes
            if result == 0:
                status = 'passed'
            elif result == 1:
                status = 'failed'
            elif result == 2:
                status = 'interrupted'
            elif result == 3:
                status = 'internal_error'
            elif result == 4:
                status = 'usage_error'
            elif result == 5:
                status = 'no_tests'
            else:
                status = 'unknown'
            
            return {
                'status': status,
                'duration': duration,
                'exit_code': result
            }
            
        except Exception as e:
            end_time = time.time()
            duration = end_time - start_time
            
            return {
                'status': 'error',
                'duration': duration,
                'error': str(e)
            }
    
    def run_all_tests(self) -> Dict[str, Any]:
        """Run all available unit tests."""
        if not BINDINGS_AVAILABLE:
            print("❌ Cannot run unit tests - QtForge bindings not available")
            return {'status': 'skipped', 'reason': 'Bindings not available'}
        
        print("🚀 Running QtForge Python unit tests...")
        print("=" * 60)
        
        existing_tests = self.check_test_files()
        available_tests = [test for test, exists in existing_tests.items() if exists]
        
        if not available_tests:
            print("❌ No unit test files found!")
            return {'status': 'no_tests'}
        
        print(f"📋 Found {len(available_tests)} unit test modules")
        print()
        
        overall_start_time = time.time()
        results = {}
        
        # Run each test module
        for test_file in available_tests:
            result = self.run_individual_test(test_file)
            results[test_file] = result
            
            # Print immediate result
            status = result['status']
            duration = result.get('duration', 0)
            
            if status == 'passed':
                print(f"   ✅ {test_file} - PASSED ({duration:.2f}s)")
            elif status == 'failed':
                print(f"   ❌ {test_file} - FAILED ({duration:.2f}s)")
            elif status == 'skipped':
                print(f"   ⏭️  {test_file} - SKIPPED ({result.get('reason', 'Unknown')})")
            else:
                print(f"   ⚠️  {test_file} - {status.upper()} ({duration:.2f}s)")
        
        overall_end_time = time.time()
        total_duration = overall_end_time - overall_start_time
        
        # Calculate summary statistics
        passed = sum(1 for r in results.values() if r['status'] == 'passed')
        failed = sum(1 for r in results.values() if r['status'] == 'failed')
        skipped = sum(1 for r in results.values() if r['status'] == 'skipped')
        errors = sum(1 for r in results.values() if r['status'] == 'error')
        
        print()
        print("=" * 60)
        print(f"📊 Unit Test Summary:")
        print(f"   Total modules: {len(available_tests)}")
        print(f"   Passed: {passed}")
        print(f"   Failed: {failed}")
        print(f"   Skipped: {skipped}")
        print(f"   Errors: {errors}")
        print(f"   Total time: {total_duration:.2f}s")
        
        if failed == 0 and errors == 0:
            print("✅ All unit tests passed!")
            overall_status = 'passed'
        else:
            print("❌ Some unit tests failed!")
            overall_status = 'failed'
        
        return {
            'status': overall_status,
            'total_duration': total_duration,
            'results': results,
            'summary': {
                'total': len(available_tests),
                'passed': passed,
                'failed': failed,
                'skipped': skipped,
                'errors': errors
            }
        }
    
    def run_coverage_analysis(self) -> Dict[str, Any]:
        """Run coverage analysis on unit tests."""
        print("📈 Running coverage analysis...")
        
        try:
            # Try to run with coverage
            coverage_args = [
                sys.executable, "-m", "coverage", "run",
                "--source=qtforge",
                "-m", "pytest",
                str(self.test_dir),
                "-v", "--tb=short"
            ]
            
            result = subprocess.run(coverage_args, capture_output=True, text=True)
            
            if result.returncode == 0:
                # Generate coverage report
                report_args = [sys.executable, "-m", "coverage", "report"]
                report_result = subprocess.run(report_args, capture_output=True, text=True)
                
                if report_result.returncode == 0:
                    print("Coverage Report:")
                    print(report_result.stdout)
                    return {'status': 'success', 'report': report_result.stdout}
                else:
                    print(f"Coverage report failed: {report_result.stderr}")
                    return {'status': 'report_failed', 'error': report_result.stderr}
            else:
                print(f"Coverage run failed: {result.stderr}")
                return {'status': 'run_failed', 'error': result.stderr}
                
        except FileNotFoundError:
            print("⚠️  Coverage tool not available. Install with: pip install coverage")
            return {'status': 'tool_not_available'}
        except Exception as e:
            print(f"Coverage analysis error: {e}")
            return {'status': 'error', 'error': str(e)}
    
    def generate_test_report(self, results: Dict[str, Any]) -> str:
        """Generate a detailed test report."""
        report_lines = [
            "QtForge Python Bindings Unit Test Report",
            "=" * 50,
            f"Generated: {time.strftime('%Y-%m-%d %H:%M:%S')}",
            ""
        ]
        
        if results['status'] == 'skipped':
            report_lines.extend([
                "❌ Tests were skipped:",
                f"   Reason: {results.get('reason', 'Unknown')}",
                ""
            ])
            return "\n".join(report_lines)
        
        summary = results.get('summary', {})
        report_lines.extend([
            "📊 Summary:",
            f"   Total test modules: {summary.get('total', 0)}",
            f"   Passed: {summary.get('passed', 0)}",
            f"   Failed: {summary.get('failed', 0)}",
            f"   Skipped: {summary.get('skipped', 0)}",
            f"   Errors: {summary.get('errors', 0)}",
            f"   Total duration: {results.get('total_duration', 0):.2f}s",
            ""
        ])
        
        # Detailed results
        test_results = results.get('results', {})
        if test_results:
            report_lines.extend([
                "📋 Detailed Results:",
                ""
            ])
            
            for test_file, result in test_results.items():
                status = result['status']
                duration = result.get('duration', 0)
                
                status_icon = {
                    'passed': '✅',
                    'failed': '❌',
                    'skipped': '⏭️',
                    'error': '⚠️'
                }.get(status, '❓')
                
                report_lines.append(f"   {status_icon} {test_file}: {status.upper()} ({duration:.2f}s)")
                
                if 'error' in result:
                    report_lines.append(f"      Error: {result['error']}")
                if 'reason' in result:
                    report_lines.append(f"      Reason: {result['reason']}")
        
        return "\n".join(report_lines)


def main():
    """Main entry point for unit test runner."""
    runner = UnitTestRunner()
    
    # Parse command line arguments
    if len(sys.argv) > 1:
        arg = sys.argv[1]
        
        if arg in ["-h", "--help"]:
            print("QtForge Python Unit Test Runner")
            print()
            print("Usage:")
            print("  python test_unit_runner.py                # Run all unit tests")
            print("  python test_unit_runner.py --coverage     # Run with coverage analysis")
            print("  python test_unit_runner.py --report       # Generate detailed report")
            print("  python test_unit_runner.py --help         # Show this help")
            return 0
        
        elif arg == "--coverage":
            # Run tests with coverage
            results = runner.run_all_tests()
            if results['status'] != 'skipped':
                runner.run_coverage_analysis()
            return 0 if results['status'] == 'passed' else 1
        
        elif arg == "--report":
            # Run tests and generate detailed report
            results = runner.run_all_tests()
            report = runner.generate_test_report(results)
            
            # Save report to file
            report_file = runner.test_dir / "unit_test_report.txt"
            with open(report_file, 'w') as f:
                f.write(report)
            
            print(f"\n📄 Detailed report saved to: {report_file}")
            return 0 if results['status'] == 'passed' else 1
    
    # Default: run all tests
    results = runner.run_all_tests()
    return 0 if results['status'] == 'passed' else 1


if __name__ == "__main__":
    sys.exit(main())
