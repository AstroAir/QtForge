# PluginMetricsCollector API Reference

!!! info "Module Information"
    **Header**: `qtplugin/monitoring/plugin_metrics_collector.hpp`  
    **Namespace**: `qtplugin`  
    **Since**: QtForge v3.0.0  
    **Status**: Stable

## Overview

The PluginMetricsCollector provides comprehensive performance monitoring and metrics collection for plugins. It automatically gathers performance data, resource usage statistics, and system metrics to help optimize plugin performance and identify bottlenecks.

### Key Features

- **Automatic Metrics Collection**: Continuous monitoring of plugin performance
- **System-wide Metrics**: Overall system performance and resource usage
- **Per-Plugin Metrics**: Individual plugin performance tracking
- **Configurable Intervals**: Adjustable monitoring frequency
- **Historical Data**: Time-series metrics storage and retrieval
- **Performance Analytics**: Built-in performance analysis and reporting

### Use Cases

- **Performance Monitoring**: Track plugin performance in production
- **Resource Optimization**: Identify resource usage patterns and bottlenecks
- **System Health**: Monitor overall system performance and stability
- **Development Profiling**: Profile plugin performance during development
- **Capacity Planning**: Analyze resource usage trends for scaling decisions

## Quick Start

```cpp
#include <qtplugin/monitoring/plugin_metrics_collector.hpp>

// Create metrics collector
auto metrics = PluginMetricsCollector::create();

// Start monitoring with 5-second intervals
metrics->start_monitoring(std::chrono::milliseconds(5000));

// Get system-wide metrics
auto system_metrics = metrics->get_system_metrics(plugin_registry);
qDebug() << "System CPU usage:" << system_metrics["cpu_usage"].toDouble();
qDebug() << "Memory usage:" << system_metrics["memory_usage"].toDouble();

// Get metrics for a specific plugin
auto plugin_metrics = metrics->get_plugin_metrics("my_plugin", plugin_registry);
qDebug() << "Plugin load time:" << plugin_metrics["load_time"].toDouble();
qDebug() << "Plugin memory:" << plugin_metrics["memory_usage"].toDouble();

// Update metrics manually
auto result = metrics->update_plugin_metrics("my_plugin", plugin_registry);
if (result) {
    qDebug() << "Metrics updated successfully";
}
```

## Interface: IPluginMetricsCollector

Base interface for metrics collection functionality.

### Virtual Methods

#### `start_monitoring()`
```cpp
virtual void start_monitoring(std::chrono::milliseconds interval) = 0;
```

Starts automatic metrics collection at specified intervals.

**Parameters:**
- `interval` - Monitoring interval in milliseconds

**Example:**
```cpp
// Monitor every 10 seconds
metrics->start_monitoring(std::chrono::milliseconds(10000));
```

#### `stop_monitoring()`
```cpp
virtual void stop_monitoring() = 0;
```

Stops automatic metrics collection.

#### `is_monitoring_active()`
```cpp
virtual bool is_monitoring_active() const = 0;
```

Checks if automatic monitoring is currently active.

**Returns:**
- `bool` - True if monitoring is active

#### `update_plugin_metrics()`
```cpp
virtual qtplugin::expected<void, PluginError> update_plugin_metrics(
    const std::string& plugin_id, 
    IPluginRegistry* plugin_registry) = 0;
```

Updates metrics for a specific plugin.

**Parameters:**
- `plugin_id` - Plugin identifier
- `plugin_registry` - Plugin registry to read from

**Returns:**
- `expected<void, PluginError>` - Success or error

**Errors:**
- `PluginErrorCode::NotFound` - Plugin not found in registry
- `PluginErrorCode::InvalidState` - Plugin not in valid state for metrics collection

#### `get_plugin_metrics()`
```cpp
virtual QJsonObject get_plugin_metrics(
    const std::string& plugin_id,
    IPluginRegistry* plugin_registry) const = 0;
```

Gets current metrics for a specific plugin.

**Parameters:**
- `plugin_id` - Plugin identifier
- `plugin_registry` - Plugin registry to read from

**Returns:**
- `QJsonObject` - Plugin metrics data or empty object if not found

**Metrics Fields:**
- `load_time` - Plugin load time in milliseconds
- `memory_usage` - Current memory usage in bytes
- `cpu_usage` - CPU usage percentage
- `call_count` - Number of method calls
- `error_count` - Number of errors encountered
- `uptime` - Plugin uptime in milliseconds
- `last_activity` - Timestamp of last activity

#### `get_system_metrics()`
```cpp
virtual QJsonObject get_system_metrics(IPluginRegistry* plugin_registry) const = 0;
```

Gets system-wide performance metrics.

**Parameters:**
- `plugin_registry` - Plugin registry to read from

**Returns:**
- `QJsonObject` - System metrics data

**System Metrics Fields:**
- `total_plugins` - Total number of loaded plugins
- `active_plugins` - Number of active plugins
- `total_memory` - Total system memory usage
- `cpu_usage` - Overall CPU usage percentage
- `plugin_load_time_avg` - Average plugin load time
- `system_uptime` - System uptime in milliseconds

#### `update_all_metrics()`
```cpp
virtual void update_all_metrics(IPluginRegistry* plugin_registry) = 0;
```

Updates metrics for all plugins in the registry.

**Parameters:**
- `plugin_registry` - Plugin registry to read from

#### `clear_metrics()`
```cpp
virtual void clear_metrics() = 0;
```

Clears all collected metrics data.

#### `set_monitoring_interval()`
```cpp
virtual void set_monitoring_interval(std::chrono::milliseconds interval) = 0;
```

Sets the monitoring interval for automatic collection.

**Parameters:**
- `interval` - New monitoring interval

#### `get_monitoring_interval()`
```cpp
virtual std::chrono::milliseconds get_monitoring_interval() const = 0;
```

Gets the current monitoring interval.

**Returns:**
- `std::chrono::milliseconds` - Current monitoring interval

## Class: PluginMetricsCollector

Concrete implementation of the metrics collector with timer-based collection.

### Constructor

```cpp
explicit PluginMetricsCollector(QObject* parent = nullptr);
```

### Static Methods

#### `create()`
```cpp
static std::shared_ptr<PluginMetricsCollector> create();
```

Creates a new metrics collector instance.

**Returns:**
- `std::shared_ptr<PluginMetricsCollector>` - Shared pointer to new instance

### Signals

The PluginMetricsCollector emits the following Qt signals:

#### `metrics_updated`
```cpp
void metrics_updated(const QString& plugin_id);
```

Emitted when metrics are updated for a plugin.

#### `monitoring_started`
```cpp
void monitoring_started(int interval_ms);
```

Emitted when automatic monitoring starts.

#### `monitoring_stopped`
```cpp
void monitoring_stopped();
```

Emitted when automatic monitoring stops.

#### `system_metrics_updated`
```cpp
void system_metrics_updated(const QJsonObject& metrics);
```

Emitted when system-wide metrics are updated.

## Error Handling

Common error codes and their meanings:

| Error Code | Description | Resolution |
|------------|-------------|------------|
| `NotFound` | Plugin not found in registry | Verify plugin ID is correct and plugin is loaded |
| `InvalidState` | Plugin not in valid state | Check plugin state before collecting metrics |
| `PermissionDenied` | Cannot access plugin metrics | Check plugin permissions and access rights |
| `ResourceExhausted` | Cannot collect metrics due to resource limits | Reduce monitoring frequency or clear old data |

## Thread Safety

- **Thread-safe methods**: All public methods are thread-safe
- **Signal emissions**: Signals are emitted from the main thread
- **Timer operations**: Monitoring timer runs in the main thread
- **Data access**: Metrics data access is synchronized

## Performance Considerations

- **Memory usage**: Approximately 1-5KB per monitored plugin
- **CPU usage**: Low overhead, typically <1% CPU usage
- **Collection frequency**: Higher frequencies increase overhead
- **Data retention**: Consider clearing old metrics periodically

## Integration Examples

### Basic Metrics Monitoring

```cpp
#include <qtplugin/monitoring/plugin_metrics_collector.hpp>
#include <qtplugin/core/plugin_registry.hpp>

class PerformanceMonitor {
private:
    std::shared_ptr<PluginMetricsCollector> m_metrics;
    std::shared_ptr<IPluginRegistry> m_registry;
    
public:
    bool initialize() {
        m_metrics = PluginMetricsCollector::create();
        
        // Connect to metrics signals
        connect(m_metrics.get(), &PluginMetricsCollector::metrics_updated,
                this, &PerformanceMonitor::on_metrics_updated);
        
        connect(m_metrics.get(), &PluginMetricsCollector::system_metrics_updated,
                this, &PerformanceMonitor::on_system_metrics_updated);
        
        // Start monitoring every 30 seconds
        m_metrics->start_monitoring(std::chrono::milliseconds(30000));
        
        return true;
    }
    
    void generate_performance_report() {
        auto system_metrics = m_metrics->get_system_metrics(m_registry.get());
        
        qDebug() << "=== Performance Report ===";
        qDebug() << "Total Plugins:" << system_metrics["total_plugins"].toInt();
        qDebug() << "Active Plugins:" << system_metrics["active_plugins"].toInt();
        qDebug() << "System CPU:" << system_metrics["cpu_usage"].toDouble() << "%";
        qDebug() << "Total Memory:" << system_metrics["total_memory"].toDouble() << "MB";
        
        // Get individual plugin metrics
        auto plugin_ids = m_registry->get_loaded_plugin_ids();
        for (const auto& plugin_id : plugin_ids) {
            auto plugin_metrics = m_metrics->get_plugin_metrics(plugin_id, m_registry.get());
            
            qDebug() << "Plugin:" << QString::fromStdString(plugin_id);
            qDebug() << "  Memory:" << plugin_metrics["memory_usage"].toDouble() << "KB";
            qDebug() << "  CPU:" << plugin_metrics["cpu_usage"].toDouble() << "%";
            qDebug() << "  Calls:" << plugin_metrics["call_count"].toInt();
            qDebug() << "  Errors:" << plugin_metrics["error_count"].toInt();
        }
    }
    
private slots:
    void on_metrics_updated(const QString& plugin_id) {
        // Handle individual plugin metrics update
        auto metrics = m_metrics->get_plugin_metrics(plugin_id.toStdString(), m_registry.get());
        
        // Check for performance issues
        double cpu_usage = metrics["cpu_usage"].toDouble();
        if (cpu_usage > 80.0) {
            qWarning() << "High CPU usage detected for plugin:" << plugin_id << cpu_usage << "%";
        }
        
        double memory_usage = metrics["memory_usage"].toDouble();
        if (memory_usage > 100 * 1024 * 1024) { // 100MB
            qWarning() << "High memory usage detected for plugin:" << plugin_id << memory_usage << "bytes";
        }
    }
    
    void on_system_metrics_updated(const QJsonObject& metrics) {
        // Handle system-wide metrics update
        double system_cpu = metrics["cpu_usage"].toDouble();
        if (system_cpu > 90.0) {
            qWarning() << "System CPU usage critical:" << system_cpu << "%";
        }
    }
};
```

### Advanced Metrics Analysis

```cpp
class MetricsAnalyzer {
private:
    std::shared_ptr<PluginMetricsCollector> m_metrics;
    std::map<std::string, std::vector<double>> m_historical_cpu;
    std::map<std::string, std::vector<double>> m_historical_memory;

public:
    void collect_historical_data() {
        // Collect metrics for trend analysis
        auto plugin_ids = m_registry->get_loaded_plugin_ids();

        for (const auto& plugin_id : plugin_ids) {
            auto metrics = m_metrics->get_plugin_metrics(plugin_id, m_registry.get());

            // Store historical data
            m_historical_cpu[plugin_id].push_back(metrics["cpu_usage"].toDouble());
            m_historical_memory[plugin_id].push_back(metrics["memory_usage"].toDouble());

            // Keep only last 100 data points
            if (m_historical_cpu[plugin_id].size() > 100) {
                m_historical_cpu[plugin_id].erase(m_historical_cpu[plugin_id].begin());
            }
            if (m_historical_memory[plugin_id].size() > 100) {
                m_historical_memory[plugin_id].erase(m_historical_memory[plugin_id].begin());
            }
        }
    }

    QJsonObject analyze_trends(const std::string& plugin_id) {
        QJsonObject analysis;

        if (m_historical_cpu.find(plugin_id) != m_historical_cpu.end()) {
            auto& cpu_data = m_historical_cpu[plugin_id];
            auto& memory_data = m_historical_memory[plugin_id];

            // Calculate averages
            double avg_cpu = std::accumulate(cpu_data.begin(), cpu_data.end(), 0.0) / cpu_data.size();
            double avg_memory = std::accumulate(memory_data.begin(), memory_data.end(), 0.0) / memory_data.size();

            // Calculate trends (simple linear regression slope)
            double cpu_trend = calculate_trend(cpu_data);
            double memory_trend = calculate_trend(memory_data);

            analysis["average_cpu"] = avg_cpu;
            analysis["average_memory"] = avg_memory;
            analysis["cpu_trend"] = cpu_trend;
            analysis["memory_trend"] = memory_trend;
            analysis["data_points"] = static_cast<int>(cpu_data.size());
        }

        return analysis;
    }

private:
    double calculate_trend(const std::vector<double>& data) {
        if (data.size() < 2) return 0.0;

        // Simple linear regression to calculate trend
        double n = data.size();
        double sum_x = n * (n - 1) / 2;
        double sum_y = std::accumulate(data.begin(), data.end(), 0.0);
        double sum_xy = 0.0;
        double sum_x2 = n * (n - 1) * (2 * n - 1) / 6;

        for (size_t i = 0; i < data.size(); ++i) {
            sum_xy += i * data[i];
        }

        return (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
    }
};
```

## Python Bindings

!!! note "Python Support"
    This component is available in Python through the `qtforge.monitoring` module.

```python
import qtforge

# Create metrics collector
metrics = qtforge.monitoring.PluginMetricsCollector()

# Start monitoring
metrics.start_monitoring(5000)  # 5 second intervals

# Get system metrics
system_metrics = metrics.get_system_metrics(plugin_registry)
print(f"System CPU: {system_metrics['cpu_usage']}%")
print(f"Total Memory: {system_metrics['total_memory']} bytes")

# Get plugin metrics
plugin_metrics = metrics.get_plugin_metrics("my_plugin", plugin_registry)
print(f"Plugin Memory: {plugin_metrics['memory_usage']} bytes")
print(f"Plugin CPU: {plugin_metrics['cpu_usage']}%")

# Update metrics manually
result = metrics.update_plugin_metrics("my_plugin", plugin_registry)
if result:
    print("Metrics updated successfully")

# Stop monitoring
metrics.stop_monitoring()
```

## Related Components

- **[PluginHotReloadManager](plugin-hot-reload-manager.md)**: Hot reload monitoring integration
- **[ResourceMonitor](resource-monitor.md)**: Advanced resource usage monitoring
- **[PluginManager](../core/plugin-manager.md)**: Core plugin management for metrics source
- **[PluginRegistry](../core/plugin-registry.md)**: Plugin registry for metrics collection

## Migration Notes

### From v2.x to v3.0

- **New Features**: Historical data tracking, trend analysis, enhanced system metrics
- **API Changes**: None (backward compatible)
- **Performance**: Improved efficiency with reduced overhead

## See Also

- [Monitoring User Guide](../../user-guide/monitoring-optimization.md)
- [Performance Optimization Guide](../../user-guide/performance.md)
- [Metrics Collection Examples](../../examples/metrics-examples.md)
- [Architecture Overview](../../architecture/system-design.md)

---

*Last updated: December 2024 | QtForge v3.0.0*
