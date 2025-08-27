#!/usr/bin/env python3
"""
Advanced QtForge Python Bindings Usage Example

This example demonstrates advanced usage patterns including module introspection,
performance considerations, integration patterns, and best practices.
"""

import sys
import os
import time
from typing import Dict, List, Any, Optional, Tuple, Callable, Union

# Add the build directory to Python path (adjust path as needed)
# Try different possible paths for the build directory
possible_paths = ['../../build', '../build', './build', 'build']
for path in possible_paths:
    if os.path.exists(os.path.join(path, 'qtforge.cp312-mingw_x86_64_msvcrt_gnu.pyd')) or \
       os.path.exists(os.path.join(path, 'qtforge.cp311-win_amd64.pyd')):
        sys.path.insert(0, path)
        break


class AdvancedUsageExample:
    """Example class demonstrating advanced QtForge usage patterns."""

    def __init__(self) -> None:
        """Initialize the advanced usage example."""
        import qtforge
        self.qtforge = qtforge
        self.performance_metrics: Dict[str, float] = {}
        print("🚀 Advanced Usage Example initialized")

    def demonstrate_module_introspection(self) -> None:
        """Demonstrate comprehensive module introspection."""
        print("\n🔍 Module Introspection:")

        # Analyze main module
        print(f"   Main Module Analysis:")
        print(f"     - Name: {self.qtforge.__name__}")
        print(f"     - File: {self.qtforge.__file__}")
        print(f"     - Version: {self.qtforge.__version__}")
        doc = self.qtforge.__doc__ or "No documentation available"
        print(f"     - Documentation: {doc[:100]}...")

        # Get all attributes and categorize them
        all_attrs = dir(self.qtforge)
        functions = []
        modules = []
        constants = []

        for attr in all_attrs:
            if not attr.startswith('_'):
                obj = getattr(self.qtforge, attr)
                if callable(obj):
                    functions.append(attr)
                elif hasattr(obj, '__file__'):  # Module
                    modules.append(attr)
                else:
                    constants.append(attr)

        print(f"     - Functions: {len(functions)} - {functions}")
        print(f"     - Modules: {len(modules)} - {modules}")
        print(f"     - Constants: {len(constants)} - {constants}")

        # Analyze submodules
        for module_name in modules:
            module = getattr(self.qtforge, module_name)
            module_attrs = [attr for attr in dir(
                module) if not attr.startswith('_')]
            print(
                f"     - {module_name} module: {len(module_attrs)} attributes")

    def demonstrate_performance_patterns(self) -> None:
        """Demonstrate performance considerations and patterns."""
        print("\n⚡ Performance Patterns:")

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
            avg_time = (total_time / iterations) * \
                1000  # Convert to milliseconds

            self.performance_metrics[op_name] = avg_time
            print(
                f"   {op_name}: {avg_time:.4f} ms/call ({iterations} iterations)")

    def demonstrate_batch_operations(self) -> None:
        """Demonstrate efficient batch operations."""
        print("\n📦 Batch Operations:")

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
        print(
            f"     Created {len(created_versions)} versions in {batch_time:.4f} seconds")

        # Batch error creation
        print(f"   Batch Error Creation:")
        error_codes = range(100, 200, 10)

        start_time = time.time()
        created_errors = []
        for code in error_codes:
            error_msg = self.qtforge.create_error(
                code, f"Error message {code}")
            created_errors.append(error_msg)

        batch_time = time.time() - start_time
        print(
            f"     Created {len(created_errors)} errors in {batch_time:.4f} seconds")

    def demonstrate_integration_patterns(self) -> None:
        """Demonstrate integration patterns with other Python libraries."""
        print("\n🔗 Integration Patterns:")

        # Integration with logging
        import logging
        logging.basicConfig(level=logging.INFO)
        logger = logging.getLogger(__name__)

        print(f"   Logging Integration:")
        try:
            result = self.qtforge.test_function()
            logger.info(f"QtForge test function result: {result}")

            version = self.qtforge.get_version()
            logger.info(f"QtForge version: {version}")

        except Exception as e:
            logger.error(f"QtForge operation failed: {e}")

        # Integration with configuration
        config = {
            "qtforge": {
                "version_format": "semantic",
                "error_handling": "strict",
                "logging_level": "info"
            }
        }

        print(f"   Configuration Integration:")
        print(f"     Config: {config['qtforge']}")

        # Use configuration to control behavior
        if config["qtforge"]["version_format"] == "semantic":
            version_str = self.qtforge.create_version(1, 0, 0)
            print(f"     Semantic version created: {version_str}")

    def demonstrate_error_handling_patterns(self) -> None:
        """Demonstrate advanced error handling patterns."""
        print("\n🛡️  Advanced Error Handling:")

        # Context manager for error handling
        class QtForgeErrorHandler:
            def __init__(self, qtforge_module: Any) -> None:
                self.qtforge = qtforge_module
                self.errors: list[str] = []

            def __enter__(self) -> 'QtForgeErrorHandler':
                return self

            def __exit__(self, exc_type: Any, exc_val: Any, exc_tb: Any) -> bool:
                if exc_type:
                    error_msg = self.qtforge.create_error(999, str(exc_val))
                    self.errors.append(error_msg)
                    print(f"     Caught and logged: {error_msg}")
                return True  # Suppress the exception

        print(f"   Context Manager Pattern:")
        with QtForgeErrorHandler(self.qtforge) as handler:
            # Simulate operations that might fail
            result1 = self.qtforge.test_function()
            print(f"     Operation 1 success: {result1}")

            # This would normally raise an exception in a real implementation
            result2 = self.qtforge.parse_version("invalid.version")
            print(f"     Operation 2 success: {result2}")

        print(f"     Errors handled: {len(handler.errors)}")

    def demonstrate_caching_patterns(self) -> None:
        """Demonstrate caching patterns for performance."""
        print("\n💾 Caching Patterns:")

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
                    self.cache[key] = qtforge_module.create_version(
                        major, minor, patch)
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
        print(
            f"     Hit ratio: {cache.hits / (cache.hits + cache.misses) * 100:.1f}%")

    def demonstrate_async_patterns(self) -> None:
        """Demonstrate asynchronous usage patterns."""
        print("\n🔄 Asynchronous Patterns:")

        import asyncio
        import concurrent.futures

        async def async_qtforge_operation(operation_name: str, operation: Callable[[], str]) -> Tuple[str, str]:
            """Run QtForge operation asynchronously."""
            loop = asyncio.get_event_loop()
            with concurrent.futures.ThreadPoolExecutor() as executor:
                result = await loop.run_in_executor(executor, operation)
                return operation_name, result

        async def run_async_operations() -> list[Tuple[str, str]]:
            """Run multiple QtForge operations concurrently."""
            operations: list[Tuple[str, Callable[[], str]]] = [
                ("test", lambda: self.qtforge.test_function()),
                ("version", lambda: self.qtforge.get_version()),
                ("create_version", lambda: self.qtforge.create_version(2, 1, 0)),
                ("parse_version", lambda: self.qtforge.parse_version("3.0.0")),
                ("create_error", lambda: self.qtforge.create_error(200, "Async error"))
            ]

            tasks = [async_qtforge_operation(name, op)
                     for name, op in operations]
            results = await asyncio.gather(*tasks)

            return results

        # Run async operations
        print(f"   Running async operations:")
        try:
            results = asyncio.run(run_async_operations())
            for name, result in results:
                print(f"     {name}: {result}")
        except Exception as e:
            print(f"     Async operations failed: {e}")

    def run_complete_example(self) -> int:
        """Run the complete advanced usage example.

        Returns:
            Exit code: 0 for success, 1 for failure
        """
        print("QtForge Advanced Usage Example")
        print("=" * 50)

        try:
            # Demonstrate all advanced patterns
            self.demonstrate_module_introspection()
            self.demonstrate_performance_patterns()
            self.demonstrate_batch_operations()
            self.demonstrate_integration_patterns()
            self.demonstrate_error_handling_patterns()
            self.demonstrate_caching_patterns()
            self.demonstrate_async_patterns()

            print(f"\n🎉 Advanced usage example completed successfully!")
            print(
                f"   Performance metrics collected: {len(self.performance_metrics)}")

            # Show performance summary
            if self.performance_metrics:
                print(f"   Performance Summary:")
                for op, time_ms in self.performance_metrics.items():
                    print(f"     - {op}: {time_ms:.4f} ms/call")

            return 0

        except Exception as e:
            print(f"❌ Error during advanced usage example: {e}")
            import traceback
            traceback.print_exc()
            return 1


def main() -> int:
    """Main function to run the advanced usage example.

    Returns:
        Exit code: 0 for success, 1 for failure
    """
    try:
        example = AdvancedUsageExample()
        return example.run_complete_example()
    except ImportError as e:
        print(f"❌ Failed to import QtForge: {e}")
        print("Make sure QtForge Python bindings are built and in the Python path.")
        return 1


if __name__ == "__main__":
    exit_code = main()
    print(f"\nExample completed with exit code: {exit_code}")
    sys.exit(exit_code)
