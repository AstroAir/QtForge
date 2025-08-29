#!/usr/bin/env python3
"""
Advanced Usage Runner

This script runs all the reorganized advanced usage examples for QtForge Python bindings.
"""

import sys
import os
import asyncio
from typing import Dict, List, Any, Optional

# Add the build directory to Python path (adjust path as needed)
possible_paths = ['../../build', '../build', './build', 'build']
for path in possible_paths:
    if os.path.exists(os.path.join(path, 'qtforge.cp312-mingw_x86_64_msvcrt_gnu.pyd')) or \
       os.path.exists(os.path.join(path, 'qtforge.cp311-win_amd64.pyd')):
        sys.path.insert(0, path)
        break


class AdvancedUsageRunner:
    """Runner for all advanced usage examples."""

    def __init__(self) -> None:
        """Initialize the runner."""
        self.results: Dict[str, int] = {}
        print("🚀 Advanced Usage Runner initialized")

    def run_module_introspection(self) -> int:
        """Run module introspection examples."""
        print("\n" + "="*60)
        print("RUNNING MODULE INTROSPECTION EXAMPLES")
        print("="*60)
        
        try:
            from patterns.module_introspection import ModuleIntrospectionExample
            example = ModuleIntrospectionExample()
            return example.run_introspection_examples()
        except Exception as e:
            print(f"❌ Failed to run module introspection: {e}")
            return 1

    def run_performance_patterns(self) -> int:
        """Run performance patterns examples."""
        print("\n" + "="*60)
        print("RUNNING PERFORMANCE PATTERNS EXAMPLES")
        print("="*60)
        
        try:
            from patterns.performance_patterns import PerformancePatternsExample
            example = PerformancePatternsExample()
            return example.run_performance_examples()
        except Exception as e:
            print(f"❌ Failed to run performance patterns: {e}")
            return 1

    async def run_async_patterns(self) -> int:
        """Run async patterns examples."""
        print("\n" + "="*60)
        print("RUNNING ASYNC PATTERNS EXAMPLES")
        print("="*60)
        
        try:
            from patterns.async_patterns import AsyncPatternsExample
            example = AsyncPatternsExample()
            return await example.run_async_examples()
        except Exception as e:
            print(f"❌ Failed to run async patterns: {e}")
            return 1

    def run_integration_patterns(self) -> int:
        """Run integration patterns examples."""
        print("\n" + "="*60)
        print("RUNNING INTEGRATION PATTERNS EXAMPLES")
        print("="*60)
        
        try:
            # Import the original integration patterns from advanced_usage
            import qtforge
            
            # Integration with logging
            import logging
            logging.basicConfig(level=logging.INFO)
            logger = logging.getLogger(__name__)

            print("🔗 Integration Patterns:")
            print("   Logging Integration:")
            try:
                result = qtforge.test_function()
                logger.info(f"QtForge test function result: {result}")

                version = qtforge.get_version()
                logger.info(f"QtForge version: {version}")
                print("   ✅ Logging integration successful")

            except Exception as e:
                logger.error(f"QtForge operation failed: {e}")
                print("   ❌ Logging integration failed")

            # Integration with configuration
            config = {
                "qtforge": {
                    "version_format": "semantic",
                    "error_handling": "strict",
                    "logging_level": "info"
                }
            }

            print("   Configuration Integration:")
            print(f"     Config: {config['qtforge']}")

            # Use configuration to control behavior
            if config["qtforge"]["version_format"] == "semantic":
                version_str = qtforge.create_version(1, 0, 0)
                print(f"     Semantic version created: {version_str}")
                print("   ✅ Configuration integration successful")

            return 0

        except Exception as e:
            print(f"❌ Failed to run integration patterns: {e}")
            return 1

    def run_error_handling_patterns(self) -> int:
        """Run error handling patterns examples."""
        print("\n" + "="*60)
        print("RUNNING ERROR HANDLING PATTERNS EXAMPLES")
        print("="*60)
        
        try:
            import qtforge
            
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

            print("🛡️ Advanced Error Handling:")
            print("   Context Manager Pattern:")
            with QtForgeErrorHandler(qtforge) as handler:
                # Simulate operations that might fail
                result1 = qtforge.test_function()
                print(f"     Operation 1 success: {result1}")

                # This would normally raise an exception in a real implementation
                result2 = qtforge.parse_version("invalid.version")
                print(f"     Operation 2 success: {result2}")

            print(f"     Errors handled: {len(handler.errors)}")
            print("   ✅ Error handling patterns successful")
            return 0

        except Exception as e:
            print(f"❌ Failed to run error handling patterns: {e}")
            return 1

    def run_caching_patterns(self) -> int:
        """Run caching patterns examples."""
        print("\n" + "="*60)
        print("RUNNING CACHING PATTERNS EXAMPLES")
        print("="*60)
        
        try:
            import qtforge
            
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

            print("💾 Caching Patterns:")
            print("   Testing cache performance:")
            for _ in range(100):
                version = cache.get_version_cached(qtforge)
                version_str = cache.create_version_cached(qtforge, 1, 2, 3)

            print(f"     Cache hits: {cache.hits}, misses: {cache.misses}")
            print(f"     Hit ratio: {cache.hits / (cache.hits + cache.misses) * 100:.1f}%")
            print("   ✅ Caching patterns successful")
            return 0

        except Exception as e:
            print(f"❌ Failed to run caching patterns: {e}")
            return 1

    async def run_all_examples(self) -> int:
        """Run all advanced usage examples.

        Returns:
            Exit code: 0 for success, 1 for failure
        """
        print("QtForge Advanced Usage Examples - Reorganized")
        print("=" * 60)

        # Run synchronous examples
        sync_examples = [
            ("Module Introspection", self.run_module_introspection),
            ("Performance Patterns", self.run_performance_patterns),
            ("Integration Patterns", self.run_integration_patterns),
            ("Error Handling Patterns", self.run_error_handling_patterns),
            ("Caching Patterns", self.run_caching_patterns),
        ]

        for name, example_func in sync_examples:
            try:
                result = example_func()
                self.results[name] = result
                if result == 0:
                    print(f"\n✅ {name} completed successfully")
                else:
                    print(f"\n❌ {name} failed with code {result}")
            except Exception as e:
                print(f"\n❌ {name} failed with exception: {e}")
                self.results[name] = 1

        # Run async examples
        try:
            result = await self.run_async_patterns()
            self.results["Async Patterns"] = result
            if result == 0:
                print(f"\n✅ Async Patterns completed successfully")
            else:
                print(f"\n❌ Async Patterns failed with code {result}")
        except Exception as e:
            print(f"\n❌ Async Patterns failed with exception: {e}")
            self.results["Async Patterns"] = 1

        # Print summary
        print("\n" + "="*60)
        print("SUMMARY")
        print("="*60)
        
        total_examples = len(self.results)
        successful_examples = sum(1 for result in self.results.values() if result == 0)
        
        print(f"Total examples: {total_examples}")
        print(f"Successful: {successful_examples}")
        print(f"Failed: {total_examples - successful_examples}")
        
        print("\nDetailed Results:")
        for name, result in self.results.items():
            status = "✅ PASS" if result == 0 else "❌ FAIL"
            print(f"  {name}: {status}")

        # Return overall success/failure
        overall_result = 0 if all(result == 0 for result in self.results.values()) else 1
        
        if overall_result == 0:
            print(f"\n🎉 All advanced usage examples completed successfully!")
        else:
            print(f"\n❌ Some advanced usage examples failed!")
            
        return overall_result


async def main() -> int:
    """Main function to run all advanced usage examples.

    Returns:
        Exit code: 0 for success, 1 for failure
    """
    try:
        runner = AdvancedUsageRunner()
        return await runner.run_all_examples()
    except ImportError as e:
        print(f"❌ Failed to import QtForge: {e}")
        print("Make sure QtForge Python bindings are built and in the Python path.")
        return 1


if __name__ == "__main__":
    exit_code = asyncio.run(main())
    print(f"\nAdvanced usage examples completed with exit code: {exit_code}")
    sys.exit(exit_code)
