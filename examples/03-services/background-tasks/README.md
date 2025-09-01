# Service Plugin Example

This comprehensive service plugin demonstrates advanced QtPlugin system capabilities including background processing, MessageBus integration, service registration, and real-world service patterns.

## Overview

The Service Plugin showcases **advanced service architecture** patterns including:

- ✅ **Background Processing** with QTimer and QThread
- ✅ **MessageBus Integration** for inter-plugin communication
- ✅ **Service Registration** and discovery
- ✅ **Worker Thread Management** with task queuing
- ✅ **Resource Monitoring** and performance tracking
- ✅ **Real-world Service Patterns** and lifecycle management
- ✅ **Thread Safety** with comprehensive synchronization
- ✅ **Comprehensive Error Handling** and logging

## Architecture

### Core Components

1. **ServicePlugin** - Main plugin class implementing IPlugin interface
2. **ServiceWorker** - Background worker for processing tasks
3. **ServiceStatusMessage** - Message type for status updates
4. **ServiceRequestMessage** - Message type for service requests

### Threading Model

- **Main Thread**: Plugin lifecycle, timers, MessageBus communication
- **Worker Thread**: Background task processing, isolated from main thread
- **Thread Safety**: Full synchronization with multiple mutex types

### Communication Flow

```
Plugin Manager → ServicePlugin → MessageBus → Other Plugins
                      ↓
                 Worker Thread → Task Processing → Results
                      ↓
                 Service Discovery → Registration → Heartbeat
```

## Features

### Background Processing

The plugin demonstrates sophisticated background processing:

- **Processing Timer**: Periodic task generation and queuing
- **Heartbeat Timer**: Regular status updates and health monitoring
- **Worker Thread**: Isolated task processing with proper lifecycle management
- **Task Queue**: Thread-safe task queuing and processing

### MessageBus Integration

Comprehensive MessageBus usage patterns:

- **Status Publishing**: Regular status updates to other plugins
- **Service Requests**: Handling incoming service requests
- **Message Subscriptions**: Listening for specific message types
- **Typed Messages**: Custom message classes with serialization

### Service Registration

Real-world service discovery patterns:

- **Service Registration**: Automatic service registration on initialization
- **Service Metadata**: Comprehensive service information and endpoints
- **Heartbeat System**: Regular service health updates
- **Service Unregistration**: Proper cleanup on shutdown

## Available Commands

### Service Management Commands

- **`service`** - Service registration and management
  - `register` - Register service with discovery system
  - `unregister` - Unregister service
  - `info` - Get service information and endpoints

### Task Management Commands

- **`task`** - Background task management
  - `submit` - Submit task for background processing
  - `stats` - Get task processing statistics

### Message Bus Commands

- **`message`** - MessageBus operations
  - `publish` - Publish custom status message
  - `stats` - Get message bus statistics

### Monitoring Commands

- **`monitoring`** - Comprehensive monitoring data
  - `performance` - Get performance metrics
  - `resources` - Get resource usage information
  - `service` - Get service-specific monitoring data
  - `all` - Get complete monitoring information

### Status Command

- **`status`** - Get comprehensive plugin status

## Configuration

The service plugin supports extensive configuration:

```json
{
  "processing_interval": 5000, // Processing timer interval (1000-300000ms)
  "heartbeat_interval": 30000, // Heartbeat timer interval (5000-600000ms)
  "logging_enabled": true, // Enable/disable logging
  "service_name": "ExampleService", // Service name (max 100 chars)
  "max_concurrent_tasks": 10, // Max concurrent tasks (1-100)
  "auto_register_service": true, // Auto-register on initialization
  "message_bus_enabled": true // Enable MessageBus integration
}
```

## Dependencies

- **Required**: `qtplugin.MessageBus` - For inter-plugin communication
- **Optional**:
  - `qtplugin.ConfigurationManager` - For centralized configuration
  - `qtplugin.PluginServiceDiscovery` - For service registration

## Building

```bash
cd examples/service_plugin
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Testing

```bash
# Run the comprehensive test application
./ServicePluginTest

# Run unit tests
ctest --output-on-failure
```

## Usage Examples

### Basic Service Usage

```cpp
#include "service_plugin.hpp"

// Create and initialize service plugin
auto plugin = std::make_unique<ServicePlugin>();
auto init_result = plugin->initialize();

if (init_result) {
    // Register service
    auto register_result = plugin->execute_command("service",
        QJsonObject{{"action", "register"}});

    // Submit background task
    QJsonObject task{
        {"id", "task_001"},
        {"type", "data_processing"},
        {"processing_time", 2000},
        {"data", QJsonObject{{"input", "test_data"}}}
    };

    auto task_result = plugin->execute_command("task",
        QJsonObject{{"action", "submit"}, {"task", task}});

    // Monitor performance
    auto metrics = plugin->execute_command("monitoring",
        QJsonObject{{"type", "performance"}});
}
```

### Advanced MessageBus Integration

```cpp
// Custom message handling
class CustomServiceMessage : public qtplugin::Message<CustomServiceMessage> {
public:
    CustomServiceMessage(std::string_view sender, const QJsonObject& data)
        : qtplugin::Message<CustomServiceMessage>(sender), m_data(data) {}

    std::string_view type() const noexcept override { return "CustomService"; }
    QJsonObject to_json() const override { return m_data; }

private:
    QJsonObject m_data;
};

// Publish custom message
plugin->execute_command("message", QJsonObject{
    {"action", "publish"},
    {"status", "custom_event"},
    {"data", QJsonObject{{"event_type", "user_action"}}}
});
```

### Service Discovery Integration

```cpp
// Get service information
auto service_info = plugin->execute_command("service",
    QJsonObject{{"action", "info"}});

if (service_info) {
    auto info = service_info.value();
    QString service_id = info.value("service_id").toString();
    QString service_name = info.value("service_name").toString();

    // Use service endpoints
    auto endpoints = info.value("endpoints").toObject();
    QString status_endpoint = endpoints.value("status").toString();
}
```

## Performance Characteristics

- **Memory Usage**: ~1MB base + task queue overhead
- **CPU Usage**: ~1% base + processing load
- **Thread Count**: 2 (main + worker)
- **Message Throughput**: High-performance MessageBus integration
- **Task Processing**: Configurable concurrent task limits

## Thread Safety

The service plugin is **fully thread-safe** with:

- `std::shared_mutex m_state_mutex` - State management
- `std::mutex m_config_mutex` - Configuration access
- `std::mutex m_error_mutex` - Error handling
- `std::mutex m_metrics_mutex` - Metrics collection
- `std::mutex m_queue_mutex` - Task queue access (in worker)

## Error Handling

Comprehensive error handling includes:

- **Thread-safe error logging** with size limits
- **Graceful degradation** when dependencies unavailable
- **Recovery mechanisms** for worker thread failures
- **Detailed error reporting** with context information

## Real-World Applications

This service plugin pattern is suitable for:

- **Background Data Processing** - ETL pipelines, data transformation
- **Microservice Architecture** - Service mesh integration
- **Event-Driven Systems** - Event processing and routing
- **Monitoring Systems** - Metrics collection and reporting
- **Task Scheduling** - Distributed task execution
- **API Gateways** - Request routing and processing

## Integration with Other Examples

The service plugin demonstrates patterns used by:

- **Message Bus Example** - Inter-plugin communication
- **Configuration Example** - Advanced configuration management
- **Security Example** - Service authentication and authorization
- **Performance Monitoring** - Comprehensive metrics collection

## License

MIT License - Same as QtPlugin library
