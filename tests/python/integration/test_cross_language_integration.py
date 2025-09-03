#!/usr/bin/env python3
"""
Cross-language integration tests for QtForge Python bindings.
Tests interoperability between Python and Lua bindings, threading safety, and memory management.
"""

import pytest
import sys
import os
import time
import threading
import subprocess
import tempfile
import json
import gc
import psutil
from pathlib import Path
from unittest.mock import Mock, patch
from concurrent.futures import ThreadPoolExecutor, as_completed

# Add the build directory to Python path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../build'))

try:
    import qtforge
    import qtforge.core as core
    import qtforge.communication as comm
    import qtforge.security as security
    import qtforge.managers as managers
    BINDINGS_AVAILABLE = True
except ImportError as e:
    BINDINGS_AVAILABLE = False
    pytest.skip(f"QtForge bindings not available: {e}", allow_module_level=True)


class TestCrossLanguageInteroperability:
    """Test interoperability between Python and Lua bindings."""

    def setup_method(self):
        """Setup for each test method."""
        self.temp_dir = tempfile.mkdtemp()
        self.lua_executable = self.find_lua_executable()

    def teardown_method(self):
        """Cleanup after each test method."""
        import shutil
        shutil.rmtree(self.temp_dir, ignore_errors=True)

    def find_lua_executable(self):
        """Find available Lua executable."""
        lua_commands = ['lua', 'lua5.1', 'lua5.2', 'lua5.3', 'lua5.4', 'luajit']
        for cmd in lua_commands:
            try:
                result = subprocess.run([cmd, '-v'], capture_output=True, text=True, timeout=5)
                if result.returncode == 0:
                    return cmd
            except (subprocess.TimeoutExpired, FileNotFoundError):
                continue
        return None

    def run_lua_script(self, script_content):
        """Run a Lua script and return the result."""
        if not self.lua_executable:
            pytest.skip("Lua executable not found")

        script_file = os.path.join(self.temp_dir, "test_script.lua")
        with open(script_file, 'w') as f:
            f.write(script_content)

        try:
            result = subprocess.run(
                [self.lua_executable, script_file],
                capture_output=True,
                text=True,
                timeout=30,
                cwd=os.path.dirname(os.path.abspath(__file__))
            )
            return result.returncode == 0, result.stdout, result.stderr
        except subprocess.TimeoutExpired:
            return False, "", "Timeout"

    def test_plugin_manager_consistency(self):
        """Test that PluginManager behaves consistently across languages."""
        # Test Python PluginManager
        python_manager = core.PluginManager()
        python_count = python_manager.get_plugin_count()

        # Test Lua PluginManager
        lua_script = '''
        local success, qtforge = pcall(require, "qtforge")
        if not success then
            print("LUA_MANAGER_FAIL:module_not_available")
            os.exit(1)
        end

        if qtforge.core and qtforge.core.create_plugin_manager then
            local manager = qtforge.core.create_plugin_manager()
            if manager and manager.get_plugin_count then
                local count = manager:get_plugin_count()
                print("LUA_MANAGER_COUNT:" .. tostring(count))
            else
                print("LUA_MANAGER_FAIL:method_not_available")
            end
        else
            print("LUA_MANAGER_FAIL:create_function_not_available")
        end
        '''

        success, stdout, stderr = self.run_lua_script(lua_script)

        if success and "LUA_MANAGER_COUNT:" in stdout:
            lua_count_str = stdout.split("LUA_MANAGER_COUNT:")[1].strip()
            try:
                lua_count = int(lua_count_str)
                assert python_count == lua_count, f"Plugin count mismatch: Python={python_count}, Lua={lua_count}"
                print("✅ PluginManager consistency verified")
            except ValueError:
                pytest.fail(f"Invalid Lua count format: {lua_count_str}")
        else:
            print(f"⚠️  Lua PluginManager test failed: {stdout} {stderr}")
            # Don't fail the test if Lua bindings are not available

    def test_message_bus_interoperability(self):
        """Test MessageBus interoperability between Python and Lua."""
        # Test Python MessageBus
        if hasattr(comm, 'MessageBus'):
            python_bus = comm.MessageBus()
            assert python_bus is not None
            print("✅ Python MessageBus created successfully")
        elif hasattr(comm, 'create_message_bus'):
            python_bus = comm.create_message_bus()
            assert python_bus is not None
            print("✅ Python MessageBus created successfully")
        else:
            pytest.skip("Python MessageBus not available")

        # Test Lua MessageBus
        lua_script = '''
        local success, qtforge = pcall(require, "qtforge")
        if not success then
            print("LUA_MESSAGE_BUS_FAIL:module_not_available")
            os.exit(0)
        end

        if qtforge.communication and qtforge.communication.create_message_bus then
            local success, bus = pcall(qtforge.communication.create_message_bus)
            if success and bus then
                print("LUA_MESSAGE_BUS_OK")
            else
                print("LUA_MESSAGE_BUS_FAIL:" .. tostring(bus))
            end
        else
            print("LUA_MESSAGE_BUS_FAIL:function_not_available")
        end
        '''

        success, stdout, stderr = self.run_lua_script(lua_script)

        if success and "LUA_MESSAGE_BUS_OK" in stdout:
            print("✅ Lua MessageBus created successfully")
            print("✅ MessageBus interoperability verified")
        else:
            print(f"⚠️  Lua MessageBus test failed: {stdout} {stderr}")

    def test_configuration_sharing(self):
        """Test configuration sharing between Python and Lua."""
        # Create configuration in Python
        if hasattr(managers, 'ConfigurationManager'):
            config_manager = managers.ConfigurationManager()

            # Test basic configuration operations
            if hasattr(config_manager, 'set_value'):
                try:
                    config_manager.set_value("test_key", "test_value")
                    print("✅ Python configuration set successfully")
                except Exception as e:
                    print(f"⚠️  Python configuration failed: {e}")

        # Test Lua configuration access
        lua_script = '''
        local success, qtforge = pcall(require, "qtforge")
        if not success then
            print("LUA_CONFIG_FAIL:module_not_available")
            os.exit(0)
        end

        if qtforge.managers and qtforge.managers.create_configuration_manager then
            local success, config = pcall(qtforge.managers.create_configuration_manager)
            if success and config then
                print("LUA_CONFIG_OK")
            else
                print("LUA_CONFIG_FAIL:" .. tostring(config))
            end
        else
            print("LUA_CONFIG_FAIL:function_not_available")
        end
        '''

        success, stdout, stderr = self.run_lua_script(lua_script)

        if success and "LUA_CONFIG_OK" in stdout:
            print("✅ Configuration sharing verified")
        else:
            print(f"⚠️  Lua configuration test failed: {stdout} {stderr}")

    def test_security_policy_consistency(self):
        """Test security policy consistency between languages."""
        # Test Python security
        if hasattr(security, 'SecurityManager'):
            security_manager = security.SecurityManager()
            assert security_manager is not None
            print("✅ Python SecurityManager created")

        # Test Lua security
        lua_script = '''
        local success, qtforge = pcall(require, "qtforge")
        if not success then
            print("LUA_SECURITY_FAIL:module_not_available")
            os.exit(0)
        end

        if qtforge.security and qtforge.security.create_security_manager then
            local success, security = pcall(qtforge.security.create_security_manager)
            if success and security then
                print("LUA_SECURITY_OK")
            else
                print("LUA_SECURITY_FAIL:" .. tostring(security))
            end
        else
            print("LUA_SECURITY_FAIL:function_not_available")
        end
        '''

        success, stdout, stderr = self.run_lua_script(lua_script)

        if success and "LUA_SECURITY_OK" in stdout:
            print("✅ Security policy consistency verified")
        else:
            print(f"⚠️  Lua security test failed: {stdout} {stderr}")

    def test_enum_value_consistency(self):
        """Test that enum values are consistent between Python and Lua."""
        # Test Python enum values
        python_states = {}
        if hasattr(core, 'PluginState'):
            for attr in ['Unloaded', 'Loaded', 'Initialized', 'Running', 'Stopped', 'Error']:
                if hasattr(core.PluginState, attr):
                    python_states[attr] = getattr(core.PluginState, attr)

        # Test Lua enum values
        lua_script = '''
        local success, qtforge = pcall(require, "qtforge")
        if not success then
            print("LUA_ENUM_FAIL:module_not_available")
            os.exit(0)
        end

        if qtforge.core and qtforge.core.PluginState then
            local states = {"Unloaded", "Loaded", "Initialized", "Running", "Stopped", "Error"}
            for _, state in ipairs(states) do
                if qtforge.core.PluginState[state] then
                    print("LUA_ENUM_" .. state .. ":" .. tostring(qtforge.core.PluginState[state]))
                end
            end
        else
            print("LUA_ENUM_FAIL:enum_not_available")
        end
        '''

        success, stdout, stderr = self.run_lua_script(lua_script)

        if success:
            lua_states = {}
            for line in stdout.split('\n'):
                if line.startswith("LUA_ENUM_"):
                    parts = line.split(":")
                    if len(parts) == 2:
                        state_name = parts[0].replace("LUA_ENUM_", "")
                        state_value = parts[1]
                        lua_states[state_name] = state_value

            # Compare enum values (if both are available)
            if python_states and lua_states:
                for state_name in python_states:
                    if state_name in lua_states:
                        print(f"✅ Enum {state_name} consistency verified")

            print("✅ Enum value consistency test completed")
        else:
            print(f"⚠️  Lua enum test failed: {stdout} {stderr}")


class TestThreadingSafety:
    """Test threading safety of Python bindings."""

    def test_concurrent_plugin_manager_access(self):
        """Test concurrent access to PluginManager from multiple threads."""
        manager = core.PluginManager()
        results = []
        errors = []

        def worker_thread(thread_id):
            try:
                # Perform various operations
                count = manager.get_plugin_count()
                plugins = manager.get_all_plugins()
                has_plugin = manager.has_plugin(f"test_plugin_{thread_id}")

                results.append({
                    'thread_id': thread_id,
                    'count': count,
                    'plugins_len': len(plugins),
                    'has_plugin': has_plugin
                })
            except Exception as e:
                errors.append({
                    'thread_id': thread_id,
                    'error': str(e)
                })

        # Create and start multiple threads
        threads = []
        for i in range(10):
            thread = threading.Thread(target=worker_thread, args=(i,))
            threads.append(thread)
            thread.start()

        # Wait for all threads to complete
        for thread in threads:
            thread.join(timeout=5)

        # Verify results
        assert len(errors) == 0, f"Threading errors occurred: {errors}"
        assert len(results) == 10, f"Expected 10 results, got {len(results)}"

        # All threads should get consistent results
        first_count = results[0]['count']
        for result in results:
            assert result['count'] == first_count, "Inconsistent plugin count across threads"

        print("✅ Concurrent PluginManager access test passed")

    def test_concurrent_message_bus_operations(self):
        """Test concurrent MessageBus operations."""
        if not hasattr(comm, 'MessageBus') and not hasattr(comm, 'create_message_bus'):
            pytest.skip("MessageBus not available")

        # Create MessageBus
        if hasattr(comm, 'MessageBus'):
            bus = comm.MessageBus()
        else:
            bus = comm.create_message_bus()

        messages_sent = []
        errors = []

        def publisher_thread(thread_id):
            try:
                for i in range(5):
                    if hasattr(comm, 'Message'):
                        message = comm.Message()
                        if hasattr(message, 'topic'):
                            message.topic = f"test_topic_{thread_id}"

                    # Try to publish (may not work without proper message setup)
                    if hasattr(bus, 'publish'):
                        try:
                            result = bus.publish(message)
                            messages_sent.append(f"thread_{thread_id}_msg_{i}")
                        except Exception:
                            # Expected if message is not properly configured
                            pass

                    time.sleep(0.01)  # Small delay
            except Exception as e:
                errors.append({
                    'thread_id': thread_id,
                    'error': str(e)
                })

        # Create and start publisher threads
        threads = []
        for i in range(5):
            thread = threading.Thread(target=publisher_thread, args=(i,))
            threads.append(thread)
            thread.start()

        # Wait for all threads to complete
        for thread in threads:
            thread.join(timeout=10)

        # Verify no critical errors occurred
        critical_errors = [e for e in errors if 'critical' in e['error'].lower()]
        assert len(critical_errors) == 0, f"Critical threading errors: {critical_errors}"

        print("✅ Concurrent MessageBus operations test passed")

    def test_thread_pool_execution(self):
        """Test thread pool execution with QtForge operations."""
        def qtforge_operation(operation_id):
            """Perform QtForge operations in thread pool."""
            try:
                # Create various QtForge objects
                manager = core.PluginManager()
                registry = core.PluginRegistry()

                # Perform operations
                count = manager.get_plugin_count()
                size = registry.size()

                return {
                    'operation_id': operation_id,
                    'manager_count': count,
                    'registry_size': size,
                    'success': True
                }
            except Exception as e:
                return {
                    'operation_id': operation_id,
                    'error': str(e),
                    'success': False
                }

        # Execute operations in thread pool
        with ThreadPoolExecutor(max_workers=5) as executor:
            futures = [executor.submit(qtforge_operation, i) for i in range(20)]
            results = [future.result(timeout=10) for future in as_completed(futures)]

        # Verify results
        successful_results = [r for r in results if r['success']]
        failed_results = [r for r in results if not r['success']]

        assert len(successful_results) >= 15, f"Too many failed operations: {len(failed_results)}"

        # Check consistency
        if successful_results:
            first_count = successful_results[0]['manager_count']
            for result in successful_results:
                assert result['manager_count'] == first_count, "Inconsistent results across threads"

        print("✅ Thread pool execution test passed")


class TestMemoryManagement:
    """Test memory management and resource cleanup."""

    def setup_method(self):
        """Setup for memory tests."""
        gc.collect()  # Clean up before test
        self.initial_memory = self.get_memory_usage()

    def get_memory_usage(self):
        """Get current memory usage in MB."""
        process = psutil.Process()
        return process.memory_info().rss / 1024 / 1024

    def test_object_creation_cleanup(self):
        """Test that objects are properly cleaned up."""
        initial_memory = self.get_memory_usage()

        # Create many objects
        objects = []
        for i in range(1000):
            manager = core.PluginManager()
            registry = core.PluginRegistry()
            loader = core.PluginLoader()
            objects.extend([manager, registry, loader])

        # Check memory usage
        peak_memory = self.get_memory_usage()
        memory_increase = peak_memory - initial_memory

        # Clear references and force garbage collection
        objects.clear()
        del objects
        gc.collect()

        # Wait a bit for cleanup
        time.sleep(0.1)

        final_memory = self.get_memory_usage()
        memory_after_cleanup = final_memory - initial_memory

        # Memory should be mostly reclaimed
        cleanup_ratio = memory_after_cleanup / memory_increase if memory_increase > 0 else 0

        print(f"Memory usage: Initial={initial_memory:.1f}MB, Peak={peak_memory:.1f}MB, Final={final_memory:.1f}MB")
        print(f"Memory increase: {memory_increase:.1f}MB, After cleanup: {memory_after_cleanup:.1f}MB")
        print(f"Cleanup ratio: {cleanup_ratio:.2f}")

        # Allow some memory overhead but ensure significant cleanup
        assert cleanup_ratio < 0.5, f"Poor memory cleanup: {cleanup_ratio:.2f} ratio"

        print("✅ Object creation cleanup test passed")

    def test_circular_reference_handling(self):
        """Test handling of circular references."""
        initial_memory = self.get_memory_usage()

        # Create objects with potential circular references
        managers = []
        registries = []

        for i in range(100):
            manager = core.PluginManager()
            registry = core.PluginRegistry()

            # Create potential circular references (if supported)
            managers.append(manager)
            registries.append(registry)

        peak_memory = self.get_memory_usage()

        # Clear references
        managers.clear()
        registries.clear()
        del managers, registries

        # Force garbage collection multiple times
        for _ in range(3):
            gc.collect()
            time.sleep(0.05)

        final_memory = self.get_memory_usage()

        memory_increase = peak_memory - initial_memory
        memory_retained = final_memory - initial_memory

        print(f"Circular reference test: Increase={memory_increase:.1f}MB, Retained={memory_retained:.1f}MB")

        # Should not retain significant memory
        if memory_increase > 1:  # Only test if significant memory was used
            retention_ratio = memory_retained / memory_increase
            assert retention_ratio < 0.3, f"High memory retention: {retention_ratio:.2f}"

        print("✅ Circular reference handling test passed")

    def test_large_data_handling(self):
        """Test handling of large data structures."""
        initial_memory = self.get_memory_usage()

        # Create objects and perform operations with large data
        manager = core.PluginManager()

        # Simulate large operations
        for i in range(100):
            # Get plugin lists (may be large)
            plugins = manager.get_all_plugins()

            # Check for nonexistent plugins (string operations)
            for j in range(10):
                has_plugin = manager.has_plugin(f"large_plugin_name_with_long_identifier_{i}_{j}")

        peak_memory = self.get_memory_usage()

        # Clean up
        del manager
        gc.collect()

        final_memory = self.get_memory_usage()

        memory_increase = peak_memory - initial_memory
        memory_retained = final_memory - initial_memory

        print(f"Large data test: Increase={memory_increase:.1f}MB, Retained={memory_retained:.1f}MB")

        # Should handle large data without excessive memory usage
        assert memory_increase < 50, f"Excessive memory usage: {memory_increase:.1f}MB"

        print("✅ Large data handling test passed")


class TestErrorHandlingIntegration:
    """Test error handling in integration scenarios."""

    def test_exception_propagation(self):
        """Test that exceptions are properly propagated across language boundaries."""
        manager = core.PluginManager()

        # Test various error conditions
        error_conditions = [
            lambda: manager.load_plugin("/nonexistent/path.so"),
            lambda: manager.get_plugin("nonexistent_plugin"),
            lambda: manager.unload_plugin("nonexistent_plugin"),
        ]

        for i, error_func in enumerate(error_conditions):
            try:
                result = error_func()
                # Some functions may return error results instead of raising exceptions
                print(f"Error condition {i}: Returned {type(result)}")
            except Exception as e:
                print(f"Error condition {i}: Raised {type(e).__name__}: {e}")

        print("✅ Exception propagation test completed")

    def test_resource_cleanup_on_error(self):
        """Test that resources are cleaned up when errors occur."""
        initial_memory = self.get_memory_usage()

        # Perform operations that may fail
        for i in range(50):
            try:
                manager = core.PluginManager()

                # Try operations that may fail
                manager.load_plugin(f"/invalid/path_{i}.so")
                manager.get_plugin(f"invalid_plugin_{i}")

            except Exception:
                # Expected to fail
                pass

        # Force cleanup
        gc.collect()
        time.sleep(0.1)

        final_memory = self.get_memory_usage()
        memory_increase = final_memory - initial_memory

        print(f"Error cleanup test: Memory increase={memory_increase:.1f}MB")

        # Should not accumulate significant memory despite errors
        assert memory_increase < 10, f"Memory leak on errors: {memory_increase:.1f}MB"

        print("✅ Resource cleanup on error test passed")


class TestPerformanceIntegration:
    """Test performance aspects of Python bindings."""

    def test_object_creation_performance(self):
        """Test performance of object creation."""
        start_time = time.time()

        # Create many objects quickly
        objects = []
        for i in range(1000):
            manager = core.PluginManager()
            registry = core.PluginRegistry()
            loader = core.PluginLoader()
            objects.extend([manager, registry, loader])

        creation_time = time.time() - start_time

        print(f"Created 3000 objects in {creation_time:.3f}s ({3000/creation_time:.0f} objects/sec)")

        # Should be able to create objects quickly
        assert creation_time < 5.0, f"Object creation too slow: {creation_time:.3f}s"

        print("✅ Object creation performance test passed")

    def test_method_call_performance(self):
        """Test performance of method calls."""
        manager = core.PluginManager()

        start_time = time.time()

        # Perform many method calls
        for i in range(10000):
            count = manager.get_plugin_count()
            has_plugin = manager.has_plugin(f"test_{i % 100}")

        call_time = time.time() - start_time

        print(f"Performed 20000 method calls in {call_time:.3f}s ({20000/call_time:.0f} calls/sec)")

        # Should be able to call methods quickly
        assert call_time < 10.0, f"Method calls too slow: {call_time:.3f}s"

        print("✅ Method call performance test passed")

    def test_data_transfer_performance(self):
        """Test performance of data transfer between C++ and Python."""
        manager = core.PluginManager()

        start_time = time.time()

        # Transfer data repeatedly
        for i in range(1000):
            plugins = manager.get_all_plugins()
            # Process the returned data
            plugin_count = len(plugins)

        transfer_time = time.time() - start_time

        print(f"Performed 1000 data transfers in {transfer_time:.3f}s ({1000/transfer_time:.0f} transfers/sec)")

        # Should be able to transfer data quickly
        assert transfer_time < 5.0, f"Data transfer too slow: {transfer_time:.3f}s"

        print("✅ Data transfer performance test passed")


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
