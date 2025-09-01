# Monitoring Plugin Example

This comprehensive monitoring plugin demonstrates QtForge's advanced monitoring features including hot reload management, performance metrics collection, resource usage tracking, and real-time monitoring dashboards.

## Overview

The Monitoring Plugin showcases **comprehensive monitoring architecture** patterns including:

- ✅ **Hot Reload Management** with file system monitoring
- ✅ **Performance Metrics Collection** and analysis
- ✅ **Resource Usage Tracking** and optimization
- ✅ **Real-time Monitoring Dashboards** with live updates
- ✅ **Alert and Notification Systems** with configurable thresholds
- ✅ **Historical Data Management** with time-series storage
- ✅ **Thread-Safe Monitoring Operations** with proper synchronization
- ✅ **Comprehensive Error Handling** and recovery mechanisms

## Features

### 🔄 Hot Reload Management

- **File system monitoring**: Automatic detection of plugin file changes
- **Dynamic reloading**: Seamless plugin updates without restart
- **State preservation**: Maintain plugin state during reload
- **Dependency tracking**: Handle plugin dependencies during reload
- **Rollback support**: Automatic rollback on reload failures

### 📊 Performance Metrics

- **Real-time collection**: Continuous performance data gathering
- **Multi-dimensional metrics**: CPU, memory, I/O, and custom metrics
- **Historical storage**: Time-series data with configurable retention
- **Aggregation support**: Statistical analysis and trend detection
- **Export capabilities**: JSON, CSV, and custom format support

### 🚨 Alert System

- **Configurable thresholds**: Custom alert conditions and triggers
- **Multiple severity levels**: Info, warning, error, critical alerts
- **Notification channels**: Email, webhook, and custom handlers
- **Alert correlation**: Group related alerts and reduce noise
- **Escalation policies**: Automatic escalation based on severity

### 📈 Monitoring Dashboard

- **Real-time visualization**: Live charts and graphs
- **System overview**: High-level system health indicators
- **Plugin status**: Individual plugin monitoring and status
- **Resource utilization**: CPU, memory, and disk usage tracking
- **Performance trends**: Historical analysis and forecasting

## Commands

### `hot_reload` - Hot Reload Management

Manage hot reload functionality for plugins:

```bash
# Enable hot reload for a plugin
{
    "action": "enable",
    "plugin_id": "my_plugin",
    "file_path": "/path/to/plugin.dll"
}

# Disable hot reload for a plugin
{
    "action": "disable",
    "plugin_id": "my_plugin"
}

# Get hot reload status
{"action": "status"}
```

**Response:**

```json
{
  "action": "enable",
  "plugin_id": "my_plugin",
  "file_path": "/path/to/plugin.dll",
  "success": true,
  "timestamp": "2024-01-15T10:30:00Z"
}
```

### `metrics` - Metrics Collection

Collect and retrieve plugin metrics:

```bash
# Get all plugin metrics
{}

# Get specific plugin metrics
{"plugin_id": "my_plugin"}
```

**Response:**

```json
{
  "action": "get_plugin",
  "plugin_id": "my_plugin",
  "metrics": {
    "cpu_usage": 15.5,
    "memory_usage": 256.0,
    "uptime_ms": 3600000,
    "error_count": 0
  },
  "timestamp": "2024-01-15T10:30:00Z",
  "success": true
}
```

### `dashboard` - Monitoring Dashboard

Get comprehensive monitoring dashboard:

```bash
# Get dashboard data
{}
```

**Response includes:**

- System overview with key metrics
- Plugin status and health information
- Performance summaries and trends
- Active alerts and notifications
- Resource utilization statistics

### `alerts` - Alert Management

Manage monitoring alerts:

```bash
# Setup alerts
{
    "action": "setup",
    "config": {
        "cpu_usage_max": {
            "metric": "cpu_usage",
            "operator": "greater_than",
            "threshold": 80.0,
            "severity": "warning"
        }
    }
}

# Get active alerts
{"action": "get"}

# Clear all alerts
{"action": "clear"}
```

### `status` - Monitoring Status

Get comprehensive monitoring status:

```bash
# Get status
{}
```

### `history` - Historical Metrics

Get historical metrics data:

```bash
# Get metrics for time range
{
    "time_range": {
        "start": "2024-01-15T09:00:00Z",
        "end": "2024-01-15T10:00:00Z"
    },
    "plugin_id": "my_plugin"  // Optional filter
}
```

## Configuration

### Monitoring Intervals

- **monitoring_interval**: General monitoring cycle (default: 5000ms)
- **metrics_collection_interval**: Metrics collection frequency (default: 10000ms)
- **alert_check_interval**: Alert evaluation frequency (default: 15000ms)

### Default Configuration

```json
{
  "hot_reload_enabled": true,
  "metrics_collection_enabled": true,
  "alerts_enabled": true,
  "monitoring_interval": 5000,
  "metrics_collection_interval": 10000,
  "alert_check_interval": 15000,
  "metrics_history_size": 1000,
  "watched_directories": ["plugins"],
  "metric_types": ["cpu_usage", "memory_usage", "plugin_count", "error_rate"],
  "alert_thresholds": {
    "cpu_usage_max": 80.0,
    "memory_usage_max": 1024.0,
    "error_rate_max": 5.0,
    "plugin_load_time_max": 5000
  },
  "dashboard_refresh_rate": 2000,
  "enable_file_monitoring": true,
  "enable_performance_tracking": true
}
```

## Building

### Prerequisites

- QtForge library v3.0.0+ with Monitoring module
- Qt6 with Core module
- CMake 3.21 or later
- C++20 compatible compiler

### Build Commands

```bash
# Configure and build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# Run tests
ctest --output-on-failure

# Install
cmake --install . --prefix /usr/local
```

### Build Options

- `QTFORGE_BUILD_DOCS=ON`: Generate documentation
- `QTFORGE_ENABLE_STATIC_ANALYSIS=ON`: Enable static analysis
- `QTFORGE_ENABLE_COVERAGE=ON`: Enable code coverage

## Testing

### Test Types

```bash
# Basic functionality test
./test_monitoring_plugin basic

# Hot reload functionality test
./test_monitoring_plugin hot_reload

# Metrics functionality test
./test_monitoring_plugin metrics

# Alerts functionality test
./test_monitoring_plugin alerts

# All tests
./test_monitoring_plugin all
```

### Test Coverage

- ✅ Plugin initialization and shutdown
- ✅ Configuration management
- ✅ Hot reload enable/disable/status
- ✅ Metrics collection (all and specific plugins)
- ✅ Alert setup, retrieval, and clearing
- ✅ Dashboard data generation
- ✅ Historical metrics retrieval
- ✅ File system monitoring
- ✅ Performance tracking
- ✅ Error handling and recovery

## Monitoring Best Practices

### Performance Optimization

1. **Adjust collection intervals** based on system load
2. **Limit metrics history size** to prevent memory issues
3. **Use efficient alert conditions** to reduce CPU overhead
4. **Monitor the monitor** - track monitoring plugin performance

### Alert Configuration

1. **Set appropriate thresholds** based on system characteristics
2. **Use severity levels** to prioritize alerts
3. **Implement alert correlation** to reduce noise
4. **Test alert conditions** before production deployment

### Hot Reload Guidelines

1. **Test reload scenarios** thoroughly before enabling
2. **Monitor reload performance** and success rates
3. **Implement rollback strategies** for failed reloads
4. **Document reload dependencies** and constraints

## Dependencies

- **Required**: QtForge::Monitoring, QtForge::Core
- **Optional**: QtForge::MessageBus, QtForge::ConfigurationManager

## License

MIT License - Same as QtForge library
