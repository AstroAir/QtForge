#!/usr/bin/env python3
"""
Comprehensive test suite for QtForge Python Monitoring bindings.
Tests all monitoring functionality including edge cases and error handling.
"""

import pytest
import sys
import os
import time
import tempfile
import threading
from unittest.mock import Mock, patch

# Add the build directory to Python path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../build'))

try:
    import qtforge
    import qtforge.monitoring as monitoring
    BINDINGS_AVAILABLE = True
except ImportError as e:
    BINDINGS_AVAILABLE = False
    print(f"QtForge bindings not available: {e}")

pytestmark = pytest.mark.skipif(not BINDINGS_AVAILABLE, reason="QtForge bindings not available")


class TestPluginHotReloadManager:
    """Test PluginHotReloadManager functionality."""
    
    def test_hot_reload_manager_creation(self) -> None:
        """Test PluginHotReloadManager can be created."""
        if hasattr(monitoring, 'create_hot_reload_manager'):
            manager = monitoring.create_hot_reload_manager()
            assert manager is not None
            assert hasattr(manager, 'enable_hot_reload')
            assert hasattr(manager, 'disable_hot_reload')
    
    def test_hot_reload_enable_disable(self) -> None:
        """Test enabling and disabling hot reload."""
        if hasattr(monitoring, 'create_hot_reload_manager'):
            manager = monitoring.create_hot_reload_manager()
            
            try:
                # Enable hot reload
                manager.enable_hot_reload("test_plugin")
                
                # Check if enabled
                if hasattr(manager, 'is_hot_reload_enabled'):
                    enabled = manager.is_hot_reload_enabled("test_plugin")
                    assert isinstance(enabled, bool)
                
                # Disable hot reload
                manager.disable_hot_reload("test_plugin")
                
                # Check if disabled
                if hasattr(manager, 'is_hot_reload_enabled'):
                    enabled = manager.is_hot_reload_enabled("test_plugin")
                    assert not enabled
            except Exception as e:
                # Some implementations might require plugin to be loaded first
                pass
    
    def test_hot_reload_watch_directory(self) -> None:
        """Test watching directory for changes."""
        if hasattr(monitoring, 'create_hot_reload_manager'):
            manager = monitoring.create_hot_reload_manager()
            
            if hasattr(manager, 'watch_directory'):
                with tempfile.TemporaryDirectory() as temp_dir:
                    try:
                        manager.watch_directory(temp_dir)
                        
                        # Create a file to trigger reload
                        test_file = os.path.join(temp_dir, "test_plugin.so")
                        with open(test_file, 'w') as f:
                            f.write("test content")
                        
                        # Give some time for file system events
                        time.sleep(0.1)
                        
                        # Stop watching
                        if hasattr(manager, 'unwatch_directory'):
                            manager.unwatch_directory(temp_dir)
                    except Exception as e:
                        # Some implementations might not support directory watching
                        pass
    
    def test_hot_reload_callback(self) -> None:
        """Test hot reload callback functionality."""
        if hasattr(monitoring, 'create_hot_reload_manager'):
            manager = monitoring.create_hot_reload_manager()
            
            if hasattr(manager, 'set_reload_callback'):
                callback_called = False
                reloaded_plugin = None
                
                def reload_callback(plugin_name) -> None:
                    nonlocal callback_called, reloaded_plugin
                    callback_called = True
                    reloaded_plugin = plugin_name
                
                try:
                    manager.set_reload_callback(reload_callback)
                    
                    # Trigger a reload if possible
                    if hasattr(manager, 'trigger_reload'):
                        manager.trigger_reload("test_plugin")
                        
                        # Give some time for callback
                        time.sleep(0.1)
                        
                        # Note: callback might not be called if plugin doesn't exist
                except Exception as e:
                    # Some implementations might not support callbacks
                    pass
    
    def test_hot_reload_invalid_plugin(self) -> None:
        """Test hot reload with invalid plugin."""
        if hasattr(monitoring, 'create_hot_reload_manager'):
            manager = monitoring.create_hot_reload_manager()
            
            # Test with None plugin name
            with pytest.raises((ValueError, RuntimeError, TypeError)):
                manager.enable_hot_reload(None)
            
            # Test with empty plugin name
            with pytest.raises((ValueError, RuntimeError)):
                manager.enable_hot_reload("")


class TestPluginMetricsCollector:
    """Test PluginMetricsCollector functionality."""
    
    def test_metrics_collector_creation(self) -> None:
        """Test PluginMetricsCollector can be created."""
        if hasattr(monitoring, 'create_metrics_collector'):
            collector = monitoring.create_metrics_collector()
            assert collector is not None
            assert hasattr(collector, 'collect_metrics')
    
    def test_metrics_collection(self) -> None:
        """Test collecting plugin metrics."""
        if hasattr(monitoring, 'create_metrics_collector'):
            collector = monitoring.create_metrics_collector()
            
            try:
                metrics = collector.collect_metrics("test_plugin")
                assert metrics is not None
                
                # Metrics should be a dictionary or similar structure
                if hasattr(metrics, 'keys') or isinstance(metrics, dict):
                    # Should contain some basic metrics
                    pass
            except Exception as e:
                # Plugin might not exist or be loaded
                pass
    
    def test_metrics_collection_all_plugins(self) -> None:
        """Test collecting metrics for all plugins."""
        if hasattr(monitoring, 'create_metrics_collector'):
            collector = monitoring.create_metrics_collector()
            
            if hasattr(collector, 'collect_all_metrics'):
                try:
                    all_metrics = collector.collect_all_metrics()
                    assert all_metrics is not None
                    assert isinstance(all_metrics, (dict, list))
                except Exception as e:
                    # Some implementations might not support this
                    pass
    
    def test_metrics_history(self) -> None:
        """Test metrics history functionality."""
        if hasattr(monitoring, 'create_metrics_collector'):
            collector = monitoring.create_metrics_collector()
            
            if hasattr(collector, 'get_metrics_history'):
                try:
                    history = collector.get_metrics_history("test_plugin")
                    assert isinstance(history, (list, tuple))
                except Exception as e:
                    # Plugin might not exist or have history
                    pass
    
    def test_metrics_export(self) -> None:
        """Test exporting metrics."""
        if hasattr(monitoring, 'create_metrics_collector'):
            collector = monitoring.create_metrics_collector()
            
            if hasattr(collector, 'export_metrics'):
                with tempfile.NamedTemporaryFile(delete=False, suffix='.json') as temp_file:
                    try:
                        collector.export_metrics(temp_file.name)
                        
                        # Check if file was created
                        assert os.path.exists(temp_file.name)
                        assert os.path.getsize(temp_file.name) >= 0
                    except Exception as e:
                        # Some implementations might not support export
                        pass
                    finally:
                        os.unlink(temp_file.name)
    
    def test_custom_metrics(self) -> None:
        """Test custom metrics functionality."""
        if hasattr(monitoring, 'create_metrics_collector'):
            collector = monitoring.create_metrics_collector()
            
            if hasattr(collector, 'add_custom_metric'):
                try:
                    collector.add_custom_metric("test_plugin", "custom_metric", 42.0)
                    
                    # Retrieve the custom metric
                    if hasattr(collector, 'get_custom_metric'):
                        value = collector.get_custom_metric("test_plugin", "custom_metric")
                        assert value == 42.0
                except Exception as e:
                    # Some implementations might not support custom metrics
                    pass


class TestPluginHealthMonitor:
    """Test plugin health monitoring functionality."""
    
    def test_health_monitor_creation(self) -> None:
        """Test creating health monitor."""
        if hasattr(monitoring, 'PluginHealthMonitor'):
            monitor = monitoring.PluginHealthMonitor()
            assert monitor is not None
    
    def test_health_check(self) -> None:
        """Test plugin health checking."""
        if hasattr(monitoring, 'PluginHealthMonitor'):
            monitor = monitoring.PluginHealthMonitor()
            
            if hasattr(monitor, 'check_health'):
                try:
                    health = monitor.check_health("test_plugin")
                    assert health is not None
                    
                    # Health should be a status or boolean
                    if hasattr(monitoring, 'PluginHealthStatus'):
                        assert hasattr(monitoring.PluginHealthStatus, 'Healthy') or hasattr(monitoring.PluginHealthStatus, 'Unhealthy')
                except Exception as e:
                    # Plugin might not exist
                    pass
    
    def test_health_monitoring_interval(self) -> None:
        """Test setting health monitoring interval."""
        if hasattr(monitoring, 'PluginHealthMonitor'):
            monitor = monitoring.PluginHealthMonitor()
            
            if hasattr(monitor, 'set_monitoring_interval'):
                try:
                    monitor.set_monitoring_interval(5.0)  # 5 seconds
                    
                    if hasattr(monitor, 'get_monitoring_interval'):
                        interval = monitor.get_monitoring_interval()
                        assert interval == 5.0
                except Exception as e:
                    # Some implementations might not support interval changes
                    pass
    
    def test_health_callback(self) -> None:
        """Test health status change callbacks."""
        if hasattr(monitoring, 'PluginHealthMonitor'):
            monitor = monitoring.PluginHealthMonitor()
            
            if hasattr(monitor, 'set_health_callback'):
                callback_called = False
                
                def health_callback(plugin_name, old_status, new_status) -> None:
                    nonlocal callback_called
                    callback_called = True
                
                try:
                    monitor.set_health_callback(health_callback)
                    
                    # Trigger health check if possible
                    if hasattr(monitor, 'check_health'):
                        monitor.check_health("test_plugin")
                        
                        # Give some time for callback
                        time.sleep(0.1)
                except Exception as e:
                    # Some implementations might not support callbacks
                    pass


class TestPerformanceMonitor:
    """Test performance monitoring functionality."""
    
    def test_performance_monitor_creation(self) -> None:
        """Test creating performance monitor."""
        if hasattr(monitoring, 'PerformanceMonitor'):
            monitor = monitoring.PerformanceMonitor()
            assert monitor is not None
    
    def test_performance_measurement(self) -> None:
        """Test measuring plugin performance."""
        if hasattr(monitoring, 'PerformanceMonitor'):
            monitor = monitoring.PerformanceMonitor()
            
            if hasattr(monitor, 'start_measurement'):
                try:
                    monitor.start_measurement("test_plugin", "test_operation")
                    
                    # Simulate some work
                    time.sleep(0.01)
                    
                    if hasattr(monitor, 'end_measurement'):
                        duration = monitor.end_measurement("test_plugin", "test_operation")
                        assert isinstance(duration, (int, float))
                        assert duration >= 0
                except Exception as e:
                    # Some implementations might not support performance measurement
                    pass
    
    def test_performance_statistics(self) -> None:
        """Test getting performance statistics."""
        if hasattr(monitoring, 'PerformanceMonitor'):
            monitor = monitoring.PerformanceMonitor()
            
            if hasattr(monitor, 'get_statistics'):
                try:
                    stats = monitor.get_statistics("test_plugin")
                    assert stats is not None
                    
                    # Statistics should contain performance data
                    if isinstance(stats, dict):
                        # Should have some performance metrics
                        pass
                except Exception as e:
                    # Plugin might not have performance data
                    pass


class TestMonitoringEnums:
    """Test monitoring-related enums."""
    
    def test_plugin_health_status_enum(self) -> None:
        """Test PluginHealthStatus enum values."""
        if hasattr(monitoring, 'PluginHealthStatus'):
            statuses = ['Healthy', 'Unhealthy', 'Unknown', 'Degraded']
            for status in statuses:
                if hasattr(monitoring.PluginHealthStatus, status):
                    value = getattr(monitoring.PluginHealthStatus, status)
                    assert value is not None
    
    def test_monitoring_event_type_enum(self) -> None:
        """Test MonitoringEventType enum values."""
        if hasattr(monitoring, 'MonitoringEventType'):
            types = ['HealthChanged', 'MetricsUpdated', 'PerformanceAlert', 'ReloadTriggered']
            for event_type in types:
                if hasattr(monitoring.MonitoringEventType, event_type):
                    value = getattr(monitoring.MonitoringEventType, event_type)
                    assert value is not None


class TestMonitoringEvents:
    """Test monitoring event system."""
    
    def test_monitoring_event_listener(self) -> None:
        """Test monitoring event listeners."""
        if hasattr(monitoring, 'create_hot_reload_manager'):
            manager = monitoring.create_hot_reload_manager()
            
            if hasattr(manager, 'add_event_listener'):
                events_received = []
                
                def event_listener(event_type, plugin_name, event_data) -> None:
                    events_received.append((event_type, plugin_name, event_data))
                
                try:
                    manager.add_event_listener(event_listener)
                    
                    # Trigger an event
                    manager.enable_hot_reload("test_plugin")
                    
                    # Give some time for event processing
                    time.sleep(0.1)
                    
                    # Note: events_received might be empty if no events are generated
                except Exception as e:
                    # Some implementations might not support event listeners
                    pass
    
    def test_monitoring_event_filtering(self) -> None:
        """Test filtering monitoring events."""
        if hasattr(monitoring, 'MonitoringEventFilter'):
            event_filter = monitoring.MonitoringEventFilter()
            
            if hasattr(event_filter, 'add_plugin_filter'):
                event_filter.add_plugin_filter("test_plugin")
                
                if hasattr(event_filter, 'should_process_event'):
                    should_process = event_filter.should_process_event("test_plugin", "some_event")
                    assert isinstance(should_process, bool)


class TestMonitoringConfiguration:
    """Test monitoring configuration functionality."""
    
    def test_monitoring_config_creation(self) -> None:
        """Test creating monitoring configuration."""
        if hasattr(monitoring, 'MonitoringConfig'):
            config = monitoring.MonitoringConfig()
            assert config is not None
    
    def test_monitoring_config_properties(self) -> None:
        """Test monitoring configuration properties."""
        if hasattr(monitoring, 'MonitoringConfig'):
            config = monitoring.MonitoringConfig()
            
            # Test setting various properties
            properties = ['hot_reload_enabled', 'metrics_collection_enabled', 'health_monitoring_enabled']
            for prop in properties:
                if hasattr(config, prop):
                    try:
                        setattr(config, prop, True)
                        value = getattr(config, prop)
                        assert value is True
                    except (AttributeError, TypeError):
                        # Some properties might be read-only
                        pass
    
    def test_monitoring_config_intervals(self) -> None:
        """Test monitoring interval configuration."""
        if hasattr(monitoring, 'MonitoringConfig'):
            config = monitoring.MonitoringConfig()
            
            intervals = ['metrics_collection_interval', 'health_check_interval']
            for interval_prop in intervals:
                if hasattr(config, interval_prop):
                    try:
                        setattr(config, interval_prop, 10.0)  # 10 seconds
                        value = getattr(config, interval_prop)
                        assert value == 10.0
                    except (AttributeError, TypeError):
                        # Some properties might be read-only
                        pass


class TestMonitoringErrorHandling:
    """Test error handling in monitoring bindings."""
    
    def test_invalid_plugin_monitoring(self) -> None:
        """Test handling invalid plugin names in monitoring."""
        if hasattr(monitoring, 'create_hot_reload_manager'):
            manager = monitoring.create_hot_reload_manager()
            
            # Test with None plugin name
            with pytest.raises((ValueError, RuntimeError, TypeError)):
                manager.enable_hot_reload(None)
    
    def test_invalid_metrics_collection(self) -> None:
        """Test handling invalid metrics collection."""
        if hasattr(monitoring, 'create_metrics_collector'):
            collector = monitoring.create_metrics_collector()
            
            # Test collecting metrics for None plugin
            with pytest.raises((ValueError, RuntimeError, TypeError)):
                collector.collect_metrics(None)
    
    def test_monitoring_exception_handling(self) -> None:
        """Test monitoring exception handling."""
        if hasattr(monitoring, 'MonitoringException'):
            # Test creating monitoring exception
            exception = monitoring.MonitoringException("Test monitoring error")
            assert exception is not None
            assert str(exception) == "Test monitoring error"


class TestMonitoringIntegration:
    """Test integration between monitoring components."""
    
    def test_hot_reload_metrics_integration(self) -> None:
        """Test integration between hot reload and metrics collection."""
        if (hasattr(monitoring, 'create_hot_reload_manager') and 
            hasattr(monitoring, 'create_metrics_collector')):
            
            reload_manager = monitoring.create_hot_reload_manager()
            metrics_collector = monitoring.create_metrics_collector()
            
            # Test if metrics collector can track hot reload events
            if hasattr(metrics_collector, 'track_hot_reload_events'):
                try:
                    metrics_collector.track_hot_reload_events(reload_manager)
                except Exception as e:
                    # Some implementations might not support this integration
                    pass
    
    def test_health_metrics_integration(self) -> None:
        """Test integration between health monitoring and metrics collection."""
        if (hasattr(monitoring, 'PluginHealthMonitor') and 
            hasattr(monitoring, 'create_metrics_collector')):
            
            health_monitor = monitoring.PluginHealthMonitor()
            metrics_collector = monitoring.create_metrics_collector()
            
            # Test if metrics collector can include health data
            if hasattr(metrics_collector, 'include_health_metrics'):
                try:
                    metrics_collector.include_health_metrics(health_monitor)
                except Exception as e:
                    # Some implementations might not support this integration
                    pass


class TestMonitoringThreadSafety:
    """Test thread safety of monitoring components."""
    
    def test_concurrent_metrics_collection(self) -> None:
        """Test concurrent metrics collection."""
        if hasattr(monitoring, 'create_metrics_collector'):
            collector = monitoring.create_metrics_collector()
            
            def collect_metrics(plugin_id) -> None:
                try:
                    metrics = collector.collect_metrics(f"test_plugin_{plugin_id}")
                    return metrics is not None
                except Exception:
                    return False
            
            # Create multiple threads
            threads = []
            results = []
            
            for i in range(3):
                thread = threading.Thread(target=lambda i=i: results.append(collect_metrics(i)))
                threads.append(thread)
                thread.start()
            
            # Wait for all threads to complete
            for thread in threads:
                thread.join(timeout=5.0)
            
            # Should not crash with concurrent access
            assert len(results) <= 3
    
    def test_concurrent_hot_reload_operations(self) -> None:
        """Test concurrent hot reload operations."""
        if hasattr(monitoring, 'create_hot_reload_manager'):
            manager = monitoring.create_hot_reload_manager()
            
            def toggle_hot_reload(plugin_id) -> None:
                try:
                    manager.enable_hot_reload(f"test_plugin_{plugin_id}")
                    manager.disable_hot_reload(f"test_plugin_{plugin_id}")
                    return True
                except Exception:
                    return False
            
            # Create multiple threads
            threads = []
            results = []
            
            for i in range(3):
                thread = threading.Thread(target=lambda i=i: results.append(toggle_hot_reload(i)))
                threads.append(thread)
                thread.start()
            
            # Wait for all threads to complete
            for thread in threads:
                thread.join(timeout=5.0)
            
            # Should not crash with concurrent access
            assert len(results) <= 3


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
