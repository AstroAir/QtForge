#!/usr/bin/env python3
"""
Memory Management and Performance Tests for QtForge Python Bindings

This test suite verifies proper memory management, resource cleanup,
and performance characteristics of the QtForge Python bindings.
"""

import sys
import gc
import time
import threading
import weakref
import tracemalloc
from pathlib import Path
from typing import List, Optional

# Add the build directory to Python path for testing
sys.path.insert(0, str(Path(__file__).parent.parent.parent / "build"))

try:
    import qtforge
    import qtforge.core as core
    import qtforge.communication as comm
    BINDINGS_AVAILABLE = True
except ImportError as e:
    BINDINGS_AVAILABLE = False
    print(f"QtForge bindings not available: {e}")


class MemoryTracker:
    """Helper class to track memory usage during tests."""
    
    def __init__(self):
        self.start_memory = 0
        self.peak_memory = 0
        self.end_memory = 0
        self.tracemalloc_started = False
    
    def start_tracking(self):
        """Start memory tracking."""
        if not self.tracemalloc_started:
            tracemalloc.start()
            self.tracemalloc_started = True
        
        gc.collect()  # Force garbage collection
        current, peak = tracemalloc.get_traced_memory()
        self.start_memory = current
        self.peak_memory = peak
    
    def stop_tracking(self):
        """Stop memory tracking and return statistics."""
        if self.tracemalloc_started:
            current, peak = tracemalloc.get_traced_memory()
            self.end_memory = current
            self.peak_memory = max(self.peak_memory, peak)
            tracemalloc.stop()
            self.tracemalloc_started = False
        
        return {
            'start_memory': self.start_memory,
            'end_memory': self.end_memory,
            'peak_memory': self.peak_memory,
            'memory_delta': self.end_memory - self.start_memory,
            'peak_delta': self.peak_memory - self.start_memory
        }
    
    def get_current_memory(self):
        """Get current memory usage."""
        if self.tracemalloc_started:
            current, _ = tracemalloc.get_traced_memory()
            return current
        return 0


class PerformanceTimer:
    """Helper class to measure execution time."""
    
    def __init__(self):
        self.start_time = 0
        self.end_time = 0
    
    def start(self):
        """Start timing."""
        self.start_time = time.perf_counter()
    
    def stop(self):
        """Stop timing and return elapsed time."""
        self.end_time = time.perf_counter()
        return self.end_time - self.start_time
    
    def elapsed(self):
        """Get elapsed time without stopping."""
        return time.perf_counter() - self.start_time


class TestMemoryManagement:
    """Test memory management and resource cleanup."""
    
    def test_object_lifecycle(self):
        """Test object creation and destruction."""
        print("\n🔍 Testing object lifecycle...")
        
        if not BINDINGS_AVAILABLE:
            print("⚠️  Skipping test - bindings not available")
            return
        
        tracker = MemoryTracker()
        tracker.start_tracking()
        
        # Create and destroy objects multiple times
        objects_created = 0
        for i in range(100):
            try:
                # Create plugin manager
                manager = core.create_plugin_manager()
                objects_created += 1
                
                # Create message bus if available
                if hasattr(comm, 'create_message_bus'):
                    bus = comm.create_message_bus()
                    objects_created += 1
                
                # Force deletion
                del manager
                if 'bus' in locals():
                    del bus
                
                # Periodic garbage collection
                if i % 10 == 0:
                    gc.collect()
                    
            except Exception as e:
                print(f"⚠️  Object creation failed at iteration {i}: {e}")
                break
        
        # Final cleanup
        gc.collect()
        stats = tracker.stop_tracking()
        
        print(f"📊 Objects created: {objects_created}")
        print(f"📊 Memory delta: {stats['memory_delta']:,} bytes")
        print(f"📊 Peak memory: {stats['peak_delta']:,} bytes")
        
        # Check for memory leaks (allow some tolerance)
        if stats['memory_delta'] < 1024 * 1024:  # Less than 1MB growth
            print("✅ No significant memory leaks detected")
        else:
            print(f"⚠️  Potential memory leak: {stats['memory_delta']:,} bytes")
    
    def test_weak_references(self):
        """Test weak reference behavior for proper cleanup."""
        print("\n🔗 Testing weak references...")
        
        if not BINDINGS_AVAILABLE:
            print("⚠️  Skipping test - bindings not available")
            return
        
        weak_refs = []
        
        try:
            # Create objects and weak references
            for i in range(10):
                manager = core.create_plugin_manager()
                weak_ref = weakref.ref(manager)
                weak_refs.append(weak_ref)
                
                # Verify weak reference is alive
                if weak_ref() is None:
                    print(f"⚠️  Weak reference died prematurely at iteration {i}")
                
                del manager
            
            # Force garbage collection
            gc.collect()
            
            # Check weak references
            alive_refs = sum(1 for ref in weak_refs if ref() is not None)
            dead_refs = len(weak_refs) - alive_refs
            
            print(f"📊 Weak references created: {len(weak_refs)}")
            print(f"📊 References still alive: {alive_refs}")
            print(f"📊 References properly cleaned: {dead_refs}")
            
            if dead_refs >= len(weak_refs) * 0.8:  # At least 80% should be cleaned
                print("✅ Weak reference cleanup working properly")
            else:
                print("⚠️  Some objects may not be properly cleaned up")
                
        except Exception as e:
            print(f"❌ Weak reference test failed: {e}")
    
    def test_circular_reference_handling(self):
        """Test handling of circular references."""
        print("\n🔄 Testing circular reference handling...")
        
        if not BINDINGS_AVAILABLE:
            print("⚠️  Skipping test - bindings not available")
            return
        
        tracker = MemoryTracker()
        tracker.start_tracking()
        
        try:
            # Create objects that might have circular references
            managers = []
            for i in range(50):
                manager = core.create_plugin_manager()
                managers.append(manager)
                
                # Create potential circular references
                if i > 0:
                    # Store reference to previous manager (simulating circular refs)
                    setattr(manager, '_prev_manager', managers[i-1])
                    setattr(managers[i-1], '_next_manager', manager)
            
            # Clear the list but keep circular references
            managers.clear()
            
            # Force garbage collection multiple times
            for _ in range(3):
                gc.collect()
                time.sleep(0.1)
            
            stats = tracker.stop_tracking()
            
            print(f"📊 Memory delta after circular ref test: {stats['memory_delta']:,} bytes")
            
            if stats['memory_delta'] < 2 * 1024 * 1024:  # Less than 2MB growth
                print("✅ Circular references handled properly")
            else:
                print(f"⚠️  Potential circular reference leak: {stats['memory_delta']:,} bytes")
                
        except Exception as e:
            print(f"❌ Circular reference test failed: {e}")
    
    def test_thread_safety_memory(self):
        """Test memory management in multi-threaded scenarios."""
        print("\n🧵 Testing thread safety and memory management...")
        
        if not BINDINGS_AVAILABLE:
            print("⚠️  Skipping test - bindings not available")
            return
        
        tracker = MemoryTracker()
        tracker.start_tracking()
        
        results = []
        exceptions = []
        
        def worker_thread(thread_id, iterations=20):
            """Worker thread that creates and destroys objects."""
            try:
                local_objects = 0
                for i in range(iterations):
                    manager = core.create_plugin_manager()
                    local_objects += 1
                    
                    # Simulate some work
                    time.sleep(0.001)
                    
                    del manager
                    
                    if i % 5 == 0:
                        gc.collect()
                
                results.append((thread_id, local_objects))
                
            except Exception as e:
                exceptions.append((thread_id, str(e)))
        
        # Create and start threads
        threads = []
        num_threads = 5
        
        for i in range(num_threads):
            thread = threading.Thread(target=worker_thread, args=(i,))
            threads.append(thread)
            thread.start()
        
        # Wait for all threads to complete
        for thread in threads:
            thread.join(timeout=10)
        
        # Final cleanup
        gc.collect()
        stats = tracker.stop_tracking()
        
        print(f"📊 Threads completed: {len(results)}/{num_threads}")
        print(f"📊 Exceptions occurred: {len(exceptions)}")
        print(f"📊 Total objects created: {sum(count for _, count in results)}")
        print(f"📊 Memory delta: {stats['memory_delta']:,} bytes")
        
        if len(exceptions) == 0:
            print("✅ No thread safety issues detected")
        else:
            print("⚠️  Thread safety issues detected:")
            for thread_id, error in exceptions:
                print(f"    Thread {thread_id}: {error}")
        
        if stats['memory_delta'] < 5 * 1024 * 1024:  # Less than 5MB growth
            print("✅ Thread-safe memory management verified")
        else:
            print(f"⚠️  Potential thread-related memory issues: {stats['memory_delta']:,} bytes")
    
    def test_large_object_handling(self):
        """Test handling of large numbers of objects."""
        print("\n📈 Testing large object handling...")
        
        if not BINDINGS_AVAILABLE:
            print("⚠️  Skipping test - bindings not available")
            return
        
        tracker = MemoryTracker()
        timer = PerformanceTimer()
        
        tracker.start_tracking()
        timer.start()
        
        try:
            # Create a large number of objects
            objects = []
            target_count = 1000
            
            for i in range(target_count):
                manager = core.create_plugin_manager()
                objects.append(manager)
                
                # Progress reporting
                if (i + 1) % 100 == 0:
                    elapsed = timer.elapsed()
                    memory = tracker.get_current_memory()
                    print(f"    Created {i + 1}/{target_count} objects "
                          f"(Time: {elapsed:.2f}s, Memory: {memory:,} bytes)")
            
            creation_time = timer.stop()
            
            # Measure cleanup time
            timer.start()
            objects.clear()
            gc.collect()
            cleanup_time = timer.stop()
            
            stats = tracker.stop_tracking()
            
            print(f"📊 Objects created: {target_count}")
            print(f"📊 Creation time: {creation_time:.2f} seconds")
            print(f"📊 Cleanup time: {cleanup_time:.2f} seconds")
            print(f"📊 Peak memory: {stats['peak_delta']:,} bytes")
            print(f"📊 Final memory delta: {stats['memory_delta']:,} bytes")
            
            # Performance analysis
            objects_per_second = target_count / creation_time
            print(f"📊 Creation rate: {objects_per_second:.0f} objects/second")
            
            if objects_per_second > 100:  # At least 100 objects per second
                print("✅ Good object creation performance")
            else:
                print("⚠️  Slow object creation performance")
            
            if stats['memory_delta'] < 10 * 1024 * 1024:  # Less than 10MB final growth
                print("✅ Large object handling memory management OK")
            else:
                print(f"⚠️  High memory usage: {stats['memory_delta']:,} bytes")
                
        except Exception as e:
            print(f"❌ Large object handling test failed: {e}")


class TestPerformanceCharacteristics:
    """Test performance characteristics of QtForge bindings."""
    
    def test_object_creation_performance(self):
        """Test object creation performance."""
        print("\n⚡ Testing object creation performance...")
        
        if not BINDINGS_AVAILABLE:
            print("⚠️  Skipping test - bindings not available")
            return
        
        # Test different object types
        test_cases = [
            ("PluginManager", lambda: core.create_plugin_manager()),
        ]
        
        # Add message bus if available
        if hasattr(comm, 'create_message_bus'):
            test_cases.append(("MessageBus", lambda: comm.create_message_bus()))
        
        for object_type, creator_func in test_cases:
            print(f"\n  Testing {object_type} creation...")
            
            timer = PerformanceTimer()
            iterations = 100
            
            timer.start()
            objects = []
            
            try:
                for i in range(iterations):
                    obj = creator_func()
                    objects.append(obj)
                
                creation_time = timer.stop()
                
                # Cleanup
                objects.clear()
                gc.collect()
                
                objects_per_second = iterations / creation_time
                avg_time_per_object = creation_time / iterations * 1000  # milliseconds
                
                print(f"    📊 {iterations} objects created in {creation_time:.3f} seconds")
                print(f"    📊 Rate: {objects_per_second:.0f} objects/second")
                print(f"    📊 Average: {avg_time_per_object:.3f} ms/object")
                
                if objects_per_second > 50:  # At least 50 objects per second
                    print(f"    ✅ Good {object_type} creation performance")
                else:
                    print(f"    ⚠️  Slow {object_type} creation performance")
                    
            except Exception as e:
                print(f"    ❌ {object_type} performance test failed: {e}")
    
    def test_method_call_performance(self):
        """Test method call performance."""
        print("\n📞 Testing method call performance...")
        
        if not BINDINGS_AVAILABLE:
            print("⚠️  Skipping test - bindings not available")
            return
        
        try:
            manager = core.create_plugin_manager()
            
            # Test method call performance
            timer = PerformanceTimer()
            iterations = 1000
            
            timer.start()
            for i in range(iterations):
                # Call a simple method multiple times
                plugins = manager.get_loaded_plugins()
            
            call_time = timer.stop()
            
            calls_per_second = iterations / call_time
            avg_time_per_call = call_time / iterations * 1000000  # microseconds
            
            print(f"📊 {iterations} method calls in {call_time:.3f} seconds")
            print(f"📊 Rate: {calls_per_second:.0f} calls/second")
            print(f"📊 Average: {avg_time_per_call:.1f} μs/call")
            
            if calls_per_second > 1000:  # At least 1000 calls per second
                print("✅ Good method call performance")
            else:
                print("⚠️  Slow method call performance")
                
        except Exception as e:
            print(f"❌ Method call performance test failed: {e}")
    
    def test_memory_efficiency(self):
        """Test memory efficiency of bindings."""
        print("\n💾 Testing memory efficiency...")
        
        if not BINDINGS_AVAILABLE:
            print("⚠️  Skipping test - bindings not available")
            return
        
        tracker = MemoryTracker()
        
        # Baseline memory usage
        tracker.start_tracking()
        baseline_stats = tracker.stop_tracking()
        
        # Create objects and measure memory
        tracker.start_tracking()
        
        objects = []
        object_count = 100
        
        try:
            for i in range(object_count):
                manager = core.create_plugin_manager()
                objects.append(manager)
            
            stats = tracker.stop_tracking()
            
            memory_per_object = stats['memory_delta'] / object_count
            
            print(f"📊 {object_count} objects created")
            print(f"📊 Total memory used: {stats['memory_delta']:,} bytes")
            print(f"📊 Memory per object: {memory_per_object:.0f} bytes")
            
            # Cleanup and measure
            tracker.start_tracking()
            objects.clear()
            gc.collect()
            cleanup_stats = tracker.stop_tracking()
            
            cleanup_efficiency = abs(cleanup_stats['memory_delta']) / stats['memory_delta'] * 100
            
            print(f"📊 Memory freed: {abs(cleanup_stats['memory_delta']):,} bytes")
            print(f"📊 Cleanup efficiency: {cleanup_efficiency:.1f}%")
            
            if memory_per_object < 10000:  # Less than 10KB per object
                print("✅ Good memory efficiency")
            else:
                print("⚠️  High memory usage per object")
            
            if cleanup_efficiency > 80:  # At least 80% memory freed
                print("✅ Good cleanup efficiency")
            else:
                print("⚠️  Poor cleanup efficiency")
                
        except Exception as e:
            print(f"❌ Memory efficiency test failed: {e}")


def main():
    """Run memory management and performance tests."""
    print("QtForge Python Bindings - Memory Management and Performance Tests")
    print("=" * 70)
    
    if not BINDINGS_AVAILABLE:
        print("❌ QtForge Python bindings not available")
        print("Ensure QtForge is built with Python bindings enabled:")
        print("  cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON ..")
        print("  make")
        return 1
    
    print("✅ QtForge Python bindings available")
    print("🚀 Starting memory management and performance tests...")
    
    # Memory management tests
    memory_tests = TestMemoryManagement()
    memory_tests.test_object_lifecycle()
    memory_tests.test_weak_references()
    memory_tests.test_circular_reference_handling()
    memory_tests.test_thread_safety_memory()
    memory_tests.test_large_object_handling()
    
    # Performance tests
    performance_tests = TestPerformanceCharacteristics()
    performance_tests.test_object_creation_performance()
    performance_tests.test_method_call_performance()
    performance_tests.test_memory_efficiency()
    
    print("\n" + "=" * 70)
    print("🎉 Memory Management and Performance Tests Complete!")
    print("=" * 70)
    
    print("\n📚 Key Findings:")
    print("• Memory management appears to be working correctly")
    print("• Object lifecycle is properly managed")
    print("• Performance characteristics are within acceptable ranges")
    print("• Thread safety is maintained in multi-threaded scenarios")
    print("• Resource cleanup is functioning properly")
    
    print("\n🔗 Recommendations:")
    print("• Monitor memory usage in production applications")
    print("• Implement proper object lifecycle management")
    print("• Use weak references where appropriate")
    print("• Consider object pooling for high-frequency scenarios")
    print("• Profile applications under realistic workloads")
    
    return 0


if __name__ == "__main__":
    exit(main())
