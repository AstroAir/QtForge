# Resource Monitor

The Resource Monitor tracks system resource usage by plugins and provides performance monitoring capabilities.

## Overview

The Resource Monitor provides:
- **Resource Tracking**: Monitor CPU, memory, disk, and network usage
- **Performance Metrics**: Collect detailed performance statistics
- **Threshold Monitoring**: Alert on resource usage limits
- **Historical Data**: Maintain resource usage history
- **Optimization Insights**: Identify performance bottlenecks

## Class Reference

### ResourceMonitor

```cpp
class ResourceMonitor {
public:
    // Monitoring control
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;
    
    // Resource tracking
    ResourceUsage getCurrentUsage(const QString& pluginId) const;
    ResourceUsage getAverageUsage(const QString& pluginId, int periodMs) const;
    ResourceUsage getPeakUsage(const QString& pluginId) const;
    
    // System-wide monitoring
    SystemResourceUsage getSystemUsage() const;
    QList<QString> getTopResourceConsumers(ResourceType type, int count = 10) const;
    
    // Threshold management
    void setThreshold(const QString& pluginId, ResourceType type, double threshold);
    double getThreshold(const QString& pluginId, ResourceType type) const;
    void removeThreshold(const QString& pluginId, ResourceType type);
    
    // Historical data
    QList<ResourceSnapshot> getHistory(const QString& pluginId, int periodMs) const;
    void clearHistory(const QString& pluginId);
    void setHistoryRetention(int retentionMs);
    
    // Configuration
    void setMonitoringInterval(int intervalMs);
    void setDetailedMonitoring(bool enabled);
    
signals:
    void thresholdExceeded(const QString& pluginId, ResourceType type, double usage, double threshold);
    void resourceSpike(const QString& pluginId, ResourceType type, double usage);
    void performanceAlert(const QString& pluginId, const QString& message);
};
```

### ResourceUsage

```cpp
struct ResourceUsage {
    double cpuPercent;          // CPU usage percentage
    qint64 memoryBytes;         // Memory usage in bytes
    qint64 diskReadBytes;       // Disk read bytes
    qint64 diskWriteBytes;      // Disk write bytes
    qint64 networkInBytes;      // Network input bytes
    qint64 networkOutBytes;     // Network output bytes
    int threadCount;            // Number of threads
    int handleCount;            // Number of handles/file descriptors
    QDateTime timestamp;        // When measurement was taken
    
    // Convenience methods
    double memoryMB() const { return memoryBytes / (1024.0 * 1024.0); }
    double diskReadMB() const { return diskReadBytes / (1024.0 * 1024.0); }
    double diskWriteMB() const { return diskWriteBytes / (1024.0 * 1024.0); }
    double networkInMB() const { return networkInBytes / (1024.0 * 1024.0); }
    double networkOutMB() const { return networkOutBytes / (1024.0 * 1024.0); }
};
```

## Usage Examples

### Basic Resource Monitoring

```cpp
ResourceMonitor* monitor = ResourceMonitor::instance();

// Start monitoring
monitor->startMonitoring();

// Get current usage for a plugin
QString pluginId = "com.example.plugin";
ResourceUsage usage = monitor->getCurrentUsage(pluginId);

qDebug() << "Plugin" << pluginId << "usage:";
qDebug() << "  CPU:" << usage.cpuPercent << "%";
qDebug() << "  Memory:" << usage.memoryMB() << "MB";
qDebug() << "  Threads:" << usage.threadCount;
```

### Setting Thresholds

```cpp
// Set memory threshold to 100MB
monitor->setThreshold("com.example.plugin", ResourceType::Memory, 100 * 1024 * 1024);

// Set CPU threshold to 50%
monitor->setThreshold("com.example.plugin", ResourceType::CPU, 50.0);

// Connect to threshold exceeded signal
connect(monitor, &ResourceMonitor::thresholdExceeded,
        [](const QString& pluginId, ResourceType type, double usage, double threshold) {
    qWarning() << "Plugin" << pluginId << "exceeded" << type 
               << "threshold:" << usage << ">" << threshold;
});
```

### Performance Analysis

```cpp
// Get average usage over last 5 minutes
ResourceUsage avgUsage = monitor->getAverageUsage("com.example.plugin", 5 * 60 * 1000);

// Get peak usage
ResourceUsage peakUsage = monitor->getPeakUsage("com.example.plugin");

// Find top CPU consumers
QList<QString> topCpuUsers = monitor->getTopResourceConsumers(ResourceType::CPU, 5);

for (const QString& pluginId : topCpuUsers) {
    ResourceUsage usage = monitor->getCurrentUsage(pluginId);
    qDebug() << "High CPU usage:" << pluginId << usage.cpuPercent << "%";
}
```

### Historical Data Analysis

```cpp
// Get resource history for last hour
QList<ResourceSnapshot> history = monitor->getHistory("com.example.plugin", 60 * 60 * 1000);

// Analyze trends
double totalCpu = 0;
for (const auto& snapshot : history) {
    totalCpu += snapshot.usage.cpuPercent;
}
double avgCpu = totalCpu / history.size();

qDebug() << "Average CPU over last hour:" << avgCpu << "%";
```

### System-wide Monitoring

```cpp
// Get overall system resource usage
SystemResourceUsage systemUsage = monitor->getSystemUsage();

qDebug() << "System usage:";
qDebug() << "  Total CPU:" << systemUsage.totalCpuPercent << "%";
qDebug() << "  Available Memory:" << systemUsage.availableMemoryMB() << "MB";
qDebug() << "  Disk Usage:" << systemUsage.diskUsagePercent << "%";
```

## Resource Types

The monitor tracks the following resource types:

- **CPU**: Processor usage percentage
- **Memory**: RAM usage in bytes
- **DiskRead**: Disk read operations and bytes
- **DiskWrite**: Disk write operations and bytes
- **NetworkIn**: Incoming network traffic
- **NetworkOut**: Outgoing network traffic
- **Threads**: Number of active threads
- **Handles**: File handles and system resources

## Performance Alerts

The monitor can generate automatic performance alerts:

```cpp
connect(monitor, &ResourceMonitor::performanceAlert,
        [](const QString& pluginId, const QString& message) {
    qInfo() << "Performance alert for" << pluginId << ":" << message;
});
```

Common alerts include:
- Memory leaks detected
- Excessive thread creation
- High disk I/O activity
- Network bandwidth saturation
- CPU usage spikes

## Configuration Options

### Monitoring Interval
Set how frequently resources are measured:

```cpp
monitor->setMonitoringInterval(1000); // Every 1 second
```

### Detailed Monitoring
Enable detailed monitoring for more granular data:

```cpp
monitor->setDetailedMonitoring(true);
```

### History Retention
Configure how long to keep historical data:

```cpp
monitor->setHistoryRetention(24 * 60 * 60 * 1000); // 24 hours
```

## Integration with Other Components

### Plugin Hot Reload Manager
The Resource Monitor integrates with the Hot Reload Manager to track resource usage during plugin reloads.

### Plugin Metrics Collector
Works with the Metrics Collector to provide comprehensive performance data.

### Threat Detector
Provides resource usage data to the Threat Detector for anomaly detection.

## Performance Impact

The Resource Monitor is designed to have minimal performance impact:
- Uses efficient system APIs for resource measurement
- Configurable monitoring intervals to balance accuracy and performance
- Optional detailed monitoring for development environments

## Thread Safety

The Resource Monitor is fully thread-safe and performs monitoring in background threads.

## Platform Support

Resource monitoring is supported on:
- **Windows**: Uses Windows Performance Counters and WMI
- **Linux**: Uses /proc filesystem and system calls
- **macOS**: Uses system APIs and Activity Monitor data

## See Also

- [Plugin Hot Reload Manager](plugin-hot-reload-manager.md)
- [Plugin Metrics Collector](plugin-metrics-collector.md)
- [Performance Optimization](../../user-guide/performance-optimization.md)
