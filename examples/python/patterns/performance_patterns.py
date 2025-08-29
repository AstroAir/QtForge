#!/usr/bin/env python3
"""
Performance Patterns Example

This module demonstrates performance considerations and optimization patterns
for QtForge Python bindings.
"""

import sys
import os
import time
from typing import Dict, List, Any, Optional, Tuple, Callable

# Add the build directory to Python path (adjust path as needed)
possible_paths = ['../../build', '../build', './build', 'build']
for path in possible_paths:
    if os.path.exists(os.path.join(path, 'qtforge.cp312-mingw_x86_64_msvcrt_gnu.pyd')) or \
       os.path.exists(os.path.join(path, 'qtforge.cp311-win_amd64.pyd')):
        sys.path.insert(0, path)
        break


class PerformancePatternsExample:
    """Example class demonstrating performance patterns."""

    def __init__(self) -> None:
        """Initialize the performance patterns example."""
        import qtforge
        self.qtforge = qtforge
        self.performance_metrics: Dict[str, float] = {}
        print("⚡ Performance Patterns Example initialized")

    def demonstrate_function_call_overhead(self) -> None:
        """Demonstrate and measure function call overhead."""
        print("\n📊 Function Call Overhead Analysis:")

        # Measure function call overhead
        operations: list[tuple[str, Callable[[], str]]] = [
            ("test_function", lambda: self.qtforge.test_function()),
            ("get_version", lambda: self.qtforge.get_version()),
            ("create_version", lambda: self.qtforge.create_version(1, 2, 3)),
            ("parse_version", lambda: self.qtforge.parse_version("1.2.3")),
            ("create_error", lambda: self.qtforge.create_error(404, "Not found"))
        ]

        for op_name, operation in operations:
            # Warm up
            for _ in range(10):
                operation()

            # Measure performance
            start_time = time.time()
            iterations = 1000

            for _ in range(iterations):
                operation()

            end_time = time.time()
            total_time = end_time - start_time
            avg_time = (total_time / iterations) * 1000  # Convert to milliseconds

            self.performance_metrics[op_name] = avg_time
            print(f"   {op_name}: {avg_time:.4f} ms/call ({iterations} iterations)")

    def demonstrate_batch_operations(self) -> None:
        """Demonstrate efficient batch operations."""
        print("\n📦 Batch Operations Performance:")

        # Batch version creation
        print(f"   Batch Version Creation:")
        version_specs = [(i, j, k) for i in range(1, 4)
                         for j in range(0, 3) for k in range(0, 3)]

        start_time = time.time()
        created_versions = []
        for major, minor, patch in version_specs:
            version_str = self.qtforge.create_version(major, minor, patch)
            created_versions.append(version_str)

        batch_time = time.time() - start_time
        print(f"     Created {len(created_versions)} versions in {batch_time:.4f} seconds")

        # Batch error creation
        print(f"   Batch Error Creation:")
        error_codes = range(100, 200, 10)

        start_time = time.time()
        created_errors = []
        for code in error_codes:
            error_msg = self.qtforge.create_error(code, f"Error message {code}")
            created_errors.append(error_msg)

        batch_time = time.time() - start_time
        print(f"     Created {len(created_errors)} errors in {batch_time:.4f} seconds")

    def demonstrate_memory_efficiency(self) -> None:
        """Demonstrate memory-efficient patterns."""
        print("\n💾 Memory Efficiency Patterns:")

        # Test object creation and cleanup
        print("   Object Creation Patterns:")

        # Pattern 1: Create and immediately use
        start_time = time.time()
        for i in range(100):
            version = self.qtforge.create_version(1, 0, i)
            # Use version immediately
            _ = len(version)

        pattern1_time = time.time() - start_time
        print(f"     Pattern 1 (immediate use): {pattern1_time:.4f} seconds")

        # Pattern 2: Batch create then process
        start_time = time.time()
        versions = []
        for i in range(100):
            version = self.qtforge.create_version(1, 0, i)
            versions.append(version)

        # Process all at once
        for version in versions:
            _ = len(version)

        pattern2_time = time.time() - start_time
        print(f"     Pattern 2 (batch process): {pattern2_time:.4f} seconds")

    def demonstrate_caching_strategies(self) -> None:
        """Demonstrate caching strategies for performance."""
        print("\n🗄️  Caching Strategies:")

        # Simple function result cache
        class QtForgeCache:
            def __init__(self) -> None:
                self.cache: Dict[str, str] = {}
                self.hits = 0
                self.misses = 0

            def get_version_cached(self, qtforge_module: Any) -> str:
                if 'version' not in self.cache:
                    self.cache['version'] = qtforge_module.get_version()
                    self.misses += 1
                else:
                    self.hits += 1
                return self.cache['version']

            def create_version_cached(self, qtforge_module: Any, major: int, minor: int, patch: int) -> str:
                key = f"version_{major}_{minor}_{patch}"
                if key not in self.cache:
                    self.cache[key] = qtforge_module.create_version(major, minor, patch)
                    self.misses += 1
                else:
                    self.hits += 1
                return self.cache[key]

        cache = QtForgeCache()

        # Test caching performance
        print(f"   Testing cache performance:")
        for _ in range(100):
            version = cache.get_version_cached(self.qtforge)
            version_str = cache.create_version_cached(self.qtforge, 1, 2, 3)

        print(f"     Cache hits: {cache.hits}, misses: {cache.misses}")
        print(f"     Hit ratio: {cache.hits / (cache.hits + cache.misses) * 100:.1f}%")

    def demonstrate_optimization_techniques(self) -> None:
        """Demonstrate various optimization techniques."""
        print("\n🚀 Optimization Techniques:")

        # Technique 1: Pre-compile frequently used operations
        print("   Pre-compilation Technique:")
        
        # Create a list of pre-compiled operations
        operations = [
            lambda: self.qtforge.get_version(),
            lambda: self.qtforge.create_version(1, 0, 0),
            lambda: self.qtforge.parse_version("2.0.0"),
        ]

        start_time = time.time()
        for _ in range(100):
            for op in operations:
                result = op()

        precompiled_time = time.time() - start_time
        print(f"     Pre-compiled operations: {precompiled_time:.4f} seconds")

        # Technique 2: String formatting optimization
        print("   String Formatting Optimization:")
        
        start_time = time.time()
        for i in range(1000):
            # Efficient string formatting
            error_msg = f"Error {i}: Operation failed"
            
        efficient_time = time.time() - start_time
        print(f"     Efficient formatting: {efficient_time:.4f} seconds")

    def demonstrate_profiling_integration(self) -> None:
        """Demonstrate integration with Python profiling tools."""
        print("\n🔍 Profiling Integration:")

        import cProfile
        import io
        import pstats

        # Profile a sample operation
        profiler = cProfile.Profile()
        
        profiler.enable()
        
        # Sample operations to profile
        for i in range(50):
            version = self.qtforge.create_version(1, i % 10, 0)
            parsed = self.qtforge.parse_version(version)
            
        profiler.disable()

        # Analyze results
        s = io.StringIO()
        ps = pstats.Stats(profiler, stream=s)
        ps.sort_stats('cumulative')
        ps.print_stats(10)  # Top 10 functions
        
        profile_output = s.getvalue()
        lines = profile_output.split('\n')[:15]  # First 15 lines
        
        print("   Profiling Results (top functions):")
        for line in lines:
            if line.strip():
                print(f"     {line}")

    def get_performance_summary(self) -> Dict[str, Any]:
        """Get a summary of performance metrics."""
        return {
            "metrics": self.performance_metrics.copy(),
            "total_operations": len(self.performance_metrics),
            "average_time": sum(self.performance_metrics.values()) / len(self.performance_metrics) if self.performance_metrics else 0,
            "fastest_operation": min(self.performance_metrics.items(), key=lambda x: x[1]) if self.performance_metrics else None,
            "slowest_operation": max(self.performance_metrics.items(), key=lambda x: x[1]) if self.performance_metrics else None,
        }

    def run_performance_examples(self) -> int:
        """Run all performance examples.

        Returns:
            Exit code: 0 for success, 1 for failure
        """
        print("Performance Patterns Examples")
        print("=" * 40)

        try:
            self.demonstrate_function_call_overhead()
            self.demonstrate_batch_operations()
            self.demonstrate_memory_efficiency()
            self.demonstrate_caching_strategies()
            self.demonstrate_optimization_techniques()
            self.demonstrate_profiling_integration()

            # Show performance summary
            summary = self.get_performance_summary()
            print(f"\n📈 Performance Summary:")
            print(f"   Total operations measured: {summary['total_operations']}")
            if summary['average_time'] > 0:
                print(f"   Average time: {summary['average_time']:.4f} ms")
            if summary['fastest_operation']:
                print(f"   Fastest: {summary['fastest_operation'][0]} ({summary['fastest_operation'][1]:.4f} ms)")
            if summary['slowest_operation']:
                print(f"   Slowest: {summary['slowest_operation'][0]} ({summary['slowest_operation'][1]:.4f} ms)")

            print(f"\n🎉 Performance patterns examples completed successfully!")
            return 0

        except Exception as e:
            print(f"❌ Error during performance examples: {e}")
            import traceback
            traceback.print_exc()
            return 1


def main() -> int:
    """Main function to run the performance examples.

    Returns:
        Exit code: 0 for success, 1 for failure
    """
    try:
        example = PerformancePatternsExample()
        return example.run_performance_examples()
    except ImportError as e:
        print(f"❌ Failed to import QtForge: {e}")
        print("Make sure QtForge Python bindings are built and in the Python path.")
        return 1


if __name__ == "__main__":
    exit_code = main()
    print(f"\nPerformance examples completed with exit code: {exit_code}")
    sys.exit(exit_code)
