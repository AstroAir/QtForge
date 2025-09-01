#!/usr/bin/env python3
"""
Asynchronous Patterns Example

This module demonstrates asynchronous usage patterns for QtForge Python bindings.
"""

import sys
import os
import asyncio
import concurrent.futures
from typing import Dict, List, Any, Optional, Tuple, Callable

# Add the build directory to Python path (adjust path as needed)
possible_paths = ['../../build', '../build', './build', 'build']
for path in possible_paths:
    if os.path.exists(os.path.join(path, 'qtforge.cp312-mingw_x86_64_msvcrt_gnu.pyd')) or \
       os.path.exists(os.path.join(path, 'qtforge.cp311-win_amd64.pyd')):
        sys.path.insert(0, path)
        break


class AsyncPatternsExample:
    """Example class demonstrating asynchronous patterns."""

    def __init__(self) -> None:
        """Initialize the async patterns example."""
        import qtforge
        self.qtforge = qtforge
        print("🔄 Async Patterns Example initialized")

    async def async_qtforge_operation(self, operation_name: str, operation: Callable[[], str]) -> Tuple[str, str]:
        """Run QtForge operation asynchronously."""
        loop = asyncio.get_event_loop()
        with concurrent.futures.ThreadPoolExecutor() as executor:
            result = await loop.run_in_executor(executor, operation)
            return operation_name, result

    async def demonstrate_basic_async_operations(self) -> None:
        """Demonstrate basic asynchronous operations."""
        print("\n🔄 Basic Async Operations:")

        # Single async operation
        operation_name, result = await self.async_qtforge_operation(
            "get_version",
            lambda: self.qtforge.get_version()
        )
        print(f"   {operation_name}: {result}")

        # Multiple sequential async operations
        operations = [
            ("test_function", lambda: self.qtforge.test_function()),
            ("create_version", lambda: self.qtforge.create_version(2, 1, 0)),
            ("parse_version", lambda: self.qtforge.parse_version("3.0.0")),
        ]

        for op_name, op_func in operations:
            name, result = await self.async_qtforge_operation(op_name, op_func)
            print(f"   {name}: {result}")

    async def demonstrate_concurrent_operations(self) -> None:
        """Demonstrate concurrent asynchronous operations."""
        print("\n⚡ Concurrent Async Operations:")

        # Run multiple operations concurrently
        operations: list[Tuple[str, Callable[[], str]]] = [
            ("test", lambda: self.qtforge.test_function()),
            ("version", lambda: self.qtforge.get_version()),
            ("create_version", lambda: self.qtforge.create_version(2, 1, 0)),
            ("parse_version", lambda: self.qtforge.parse_version("3.0.0")),
            ("create_error", lambda: self.qtforge.create_error(200, "Async error"))
        ]

        tasks = [self.async_qtforge_operation(
            name, op) for name, op in operations]
        results = await asyncio.gather(*tasks)

        print("   Concurrent results:")
        for name, result in results:
            print(f"     {name}: {result}")

    async def demonstrate_async_batch_processing(self) -> None:
        """Demonstrate asynchronous batch processing."""
        print("\n📦 Async Batch Processing:")

        # Create a batch of version creation tasks
        version_tasks = []
        for i in range(10):
            task = self.async_qtforge_operation(
                f"version_{i}",
                lambda i=i: self.qtforge.create_version(1, 0, i)
            )
            version_tasks.append(task)

        # Process batch concurrently
        batch_results = await asyncio.gather(*version_tasks)

        print(f"   Processed {len(batch_results)} versions concurrently:")
        for name, result in batch_results[:5]:  # Show first 5
            print(f"     {name}: {result}")

    async def demonstrate_async_error_handling(self) -> None:
        """Demonstrate error handling in async operations."""
        print("\n🛡️ Async Error Handling:")

        async def safe_operation(op_name: str, operation: Callable[[], str]) -> Tuple[str, str, Optional[str]]:
            """Safely execute an operation with error handling."""
            try:
                name, result = await self.async_qtforge_operation(op_name, operation)
                return name, result, None
            except Exception as e:
                return op_name, "", str(e)

        # Test operations that might fail
        operations = [
            ("valid_version", lambda: self.qtforge.create_version(1, 2, 3)),
            ("invalid_parse", lambda: self.qtforge.parse_version("invalid.version")),
            ("valid_error", lambda: self.qtforge.create_error(404, "Not found")),
        ]

        tasks = [safe_operation(name, op) for name, op in operations]
        results = await asyncio.gather(*tasks)

        print("   Error handling results:")
        for name, result, error in results:
            if error:
                print(f"     {name}: ❌ Error - {error}")
            else:
                print(f"     {name}: ✅ Success - {result}")

    async def demonstrate_async_streaming(self) -> None:
        """Demonstrate streaming/generator-like async patterns."""
        print("\n🌊 Async Streaming Patterns:")

        async def version_stream(count: int):
            """Generate versions asynchronously."""
            for i in range(count):
                version = await self.async_qtforge_operation(
                    f"stream_{i}",
                    lambda i=i: self.qtforge.create_version(2, 0, i)
                )
                yield version
                # Small delay to simulate streaming
                await asyncio.sleep(0.01)

        print("   Streaming version generation:")
        async for name, version in version_stream(5):
            print(f"     {name}: {version}")

    async def demonstrate_async_rate_limiting(self) -> None:
        """Demonstrate rate limiting in async operations."""
        print("\n⏱️ Async Rate Limiting:")

        class RateLimiter:
            def __init__(self, max_concurrent: int = 3):
                self.semaphore = asyncio.Semaphore(max_concurrent)

            async def execute(self, operation: Callable):
                async with self.semaphore:
                    return await operation()

        rate_limiter = RateLimiter(max_concurrent=2)

        # Create many operations
        operations = []
        for i in range(8):
            async def limited_operation(i=i):
                return await self.async_qtforge_operation(
                    f"limited_{i}",
                    lambda: self.qtforge.create_version(3, 0, i)
                )
            operations.append(rate_limiter.execute(limited_operation))

        print("   Rate-limited operations (max 2 concurrent):")
        results = await asyncio.gather(*operations)
        for name, result in results:
            print(f"     {name}: {result}")

    async def demonstrate_async_timeout_handling(self) -> None:
        """Demonstrate timeout handling in async operations."""
        print("\n⏰ Async Timeout Handling:")

        async def operation_with_timeout(timeout_seconds: float):
            """Execute operation with timeout."""
            try:
                result = await asyncio.wait_for(
                    self.async_qtforge_operation(
                        "timeout_test",
                        lambda: self.qtforge.get_version()
                    ),
                    timeout=timeout_seconds
                )
                return result, None
            except asyncio.TimeoutError:
                return None, "Operation timed out"

        # Test with reasonable timeout
        result, error = await operation_with_timeout(5.0)
        if error:
            print(f"   Timeout test: ❌ {error}")
        else:
            print(f"   Timeout test: ✅ {result}")

    async def run_async_examples(self) -> int:
        """Run all async examples.

        Returns:
            Exit code: 0 for success, 1 for failure
        """
        print("Asynchronous Patterns Examples")
        print("=" * 40)

        try:
            await self.demonstrate_basic_async_operations()
            await self.demonstrate_concurrent_operations()
            await self.demonstrate_async_batch_processing()
            await self.demonstrate_async_error_handling()
            await self.demonstrate_async_streaming()
            await self.demonstrate_async_rate_limiting()
            await self.demonstrate_async_timeout_handling()

            print(f"\n🎉 Async patterns examples completed successfully!")
            return 0

        except Exception as e:
            print(f"❌ Error during async examples: {e}")
            import traceback
            traceback.print_exc()
            return 1


async def main() -> int:
    """Main function to run the async examples.

    Returns:
        Exit code: 0 for success, 1 for failure
    """
    try:
        example = AsyncPatternsExample()
        return await example.run_async_examples()
    except ImportError as e:
        print(f"❌ Failed to import QtForge: {e}")
        print("Make sure QtForge Python bindings are built and in the Python path.")
        return 1


if __name__ == "__main__":
    exit_code = asyncio.run(main())
    print(f"\nAsync examples completed with exit code: {exit_code}")
    sys.exit(exit_code)
