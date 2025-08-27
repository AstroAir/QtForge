#!/usr/bin/env python3
"""
Run All QtForge Python Binding Examples

This script runs all the QtForge Python binding examples in sequence,
providing a comprehensive test of the bindings and demonstrating all features.
"""

import sys
import os
import time
import subprocess
from pathlib import Path
from typing import Dict, List, Any, Tuple


def run_example(example_file: str) -> tuple[int, float, str]:
    """
    Run a single example and return results.

    Args:
        example_file: Path to the example Python file

    Returns:
        Tuple of (exit_code, execution_time, output)
    """
    print(f"🚀 Running {example_file}...")

    start_time = time.time()
    try:
        result = subprocess.run(
            [sys.executable, example_file],
            capture_output=True,
            text=True,
            timeout=60  # 60 second timeout
        )
        end_time = time.time()
        execution_time = end_time - start_time

        return result.returncode, execution_time, result.stdout + result.stderr

    except subprocess.TimeoutExpired:
        end_time = time.time()
        execution_time = end_time - start_time
        return -1, execution_time, "Example timed out after 60 seconds"
    except Exception as e:
        end_time = time.time()
        execution_time = end_time - start_time
        return -2, execution_time, f"Failed to run example: {e}"


def main() -> int:
    """Main function to run all examples.

    Returns:
        Exit code: 0 for success, 1 for failure
    """

    print("=" * 70)
    print("QtForge Python Bindings - Complete Example Suite")
    print("=" * 70)

    # Define examples in order of complexity
    examples = [
        ("basic_usage.py", "Basic Usage and Getting Started"),
        ("plugin_management.py", "Plugin Management Operations"),
        ("version_handling.py", "Version Handling and Comparison"),
        ("error_handling.py", "Error Handling and Recovery"),
        ("advanced_usage.py", "Advanced Patterns and Performance")
    ]

    # Track results
    results: List[Dict[str, Any]] = []
    total_start_time = time.time()

    # Check if examples exist
    missing_examples = []
    for example_file, _ in examples:
        if not Path(example_file).exists():
            missing_examples.append(example_file)

    if missing_examples:
        print(f"❌ Missing example files: {missing_examples}")
        print("Please ensure all example files are present in the current directory.")
        return 1

    # Run each example
    for i, (example_file, description) in enumerate(examples, 1):
        print(f"\n📋 Example {i}/{len(examples)}: {description}")
        print(f"   File: {example_file}")
        print("-" * 50)

        exit_code, execution_time, output = run_example(example_file)

        # Store results
        results.append({
            'file': example_file,
            'description': description,
            'exit_code': exit_code,
            'execution_time': execution_time,
            'success': exit_code == 0
        })

        # Show immediate results
        if exit_code == 0:
            print(f"✅ SUCCESS - Completed in {execution_time:.2f} seconds")
        elif exit_code == -1:
            print(f"⏰ TIMEOUT - Exceeded 60 seconds")
        elif exit_code == -2:
            print(f"💥 FAILED - Could not execute")
        else:
            print(f"❌ FAILED - Exit code {exit_code}")

        # Show output summary (first and last few lines)
        if output:
            lines = output.strip().split('\n')
            if len(lines) <= 10:
                print("   Output:")
                for line in lines:
                    print(f"     {line}")
            else:
                print("   Output (first 5 and last 5 lines):")
                for line in lines[:5]:
                    print(f"     {line}")
                print(f"     ... ({len(lines) - 10} lines omitted) ...")
                for line in lines[-5:]:
                    print(f"     {line}")

        print("-" * 50)

    # Calculate total execution time
    total_end_time = time.time()
    total_execution_time = total_end_time - total_start_time

    # Generate summary report
    print(f"\n📊 EXECUTION SUMMARY")
    print("=" * 70)

    successful_examples = [r for r in results if r['success']]
    failed_examples = [r for r in results if not r['success']]

    print(f"Total Examples: {len(examples)}")
    print(f"Successful: {len(successful_examples)}")
    print(f"Failed: {len(failed_examples)}")
    print(f"Success Rate: {len(successful_examples)/len(examples)*100:.1f}%")
    print(f"Total Execution Time: {total_execution_time:.2f} seconds")

    # Detailed results
    print(f"\n📋 DETAILED RESULTS:")
    for result in results:
        status = "✅ PASS" if result['success'] else "❌ FAIL"
        print(
            f"   {status} {result['file']:<25} ({result['execution_time']:.2f}s) - {result['description']}")

    # Performance analysis
    if successful_examples:
        execution_times: List[float] = [r['execution_time']
                                        for r in successful_examples]
        avg_time = sum(execution_times) / len(execution_times)
        min_time = min(execution_times)
        max_time = max(execution_times)

        print(f"\n⚡ PERFORMANCE ANALYSIS:")
        print(f"   Average execution time: {avg_time:.2f} seconds")
        print(f"   Fastest example: {min_time:.2f} seconds")
        print(f"   Slowest example: {max_time:.2f} seconds")

    # Failure analysis
    if failed_examples:
        print(f"\n🔍 FAILURE ANALYSIS:")
        for result in failed_examples:
            print(f"   ❌ {result['file']}: Exit code {result['exit_code']}")

    # Recommendations
    print(f"\n🎯 RECOMMENDATIONS:")
    if len(successful_examples) == len(examples):
        print("   🎉 All examples passed! QtForge Python bindings are working perfectly.")
        print("   ✅ You can now use QtForge Python bindings in your projects.")
        print("   📚 Review the example code to learn best practices.")
    elif len(successful_examples) > 0:
        print("   ⚠️  Some examples failed. Check the error messages above.")
        print("   🔧 Verify your QtForge installation and Python environment.")
        print("   📋 Start with the basic_usage.py example to debug issues.")
    else:
        print("   💥 All examples failed. There may be a fundamental issue.")
        print("   🔧 Check that QtForge Python bindings are properly built.")
        print("   📋 Verify that the qtforge module can be imported.")
        print("   🛠️  Review the build configuration and dependencies.")

    # Next steps
    print(f"\n🚀 NEXT STEPS:")
    print("   1. Review any failed examples and fix issues")
    print("   2. Study the successful examples for usage patterns")
    print("   3. Integrate QtForge Python bindings into your projects")
    print("   4. Contribute improvements and additional examples")

    print("=" * 70)

    # Return appropriate exit code
    return 0 if len(failed_examples) == 0 else 1


if __name__ == "__main__":
    try:
        exit_code = main()
        print(f"\nExample suite completed with exit code: {exit_code}")
        sys.exit(exit_code)
    except KeyboardInterrupt:
        print(f"\n\n⚠️  Example suite interrupted by user")
        sys.exit(130)
    except Exception as e:
        print(f"\n\n💥 Unexpected error running example suite: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
