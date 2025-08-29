# QtForge Communication Examples

This directory contains comprehensive examples demonstrating QtForge's communication system capabilities, including advanced MessageBus patterns, request-response communication, event broadcasting, and message filtering.

## 🚀 Examples Overview

### 📡 Advanced MessageBus (`advanced_message_bus`)

Demonstrates sophisticated MessageBus usage patterns including:

**Features:**
- ✅ Type-safe message publishing and subscription
- ✅ Message priority and delivery modes
- ✅ Subscription management and lifecycle
- ✅ Message statistics and monitoring
- ✅ Error handling and recovery
- ✅ Performance optimization techniques

**Use Cases:**
- High-throughput messaging systems
- Real-time event processing
- Plugin coordination and orchestration
- System-wide notifications

### 🔄 Request-Response Communication (`request_response_example`)

Shows how to implement synchronous and asynchronous request-response patterns.

**Features:**
- ✅ Synchronous request-response with timeouts
- ✅ Asynchronous request handling with futures
- ✅ Request routing and load balancing
- ✅ Error propagation and handling
- ✅ Request correlation and tracking
- ✅ Performance metrics and monitoring

**Use Cases:**
- Service-oriented architectures
- Remote procedure calls (RPC)
- API gateway patterns
- Microservice communication

### 📢 Event Broadcasting (`event_broadcasting_example`)

Demonstrates event-driven architecture patterns with broadcasting.

**Features:**
- ✅ Event publishing to multiple subscribers
- ✅ Event filtering and routing
- ✅ Event aggregation and batching
- ✅ Event persistence and replay
- ✅ Subscriber priority and ordering
- ✅ Event lifecycle management

**Use Cases:**
- Event sourcing systems
- Real-time notifications
- State synchronization
- Audit logging and monitoring

### 🔍 Message Filtering (`message_filtering_example`)

Shows advanced message filtering and routing capabilities.

**Features:**
- ✅ Content-based message filtering
- ✅ Topic-based routing patterns
- ✅ Dynamic filter registration
- ✅ Filter composition and chaining
- ✅ Performance-optimized filtering
- ✅ Filter statistics and monitoring

**Use Cases:**
- Message routing systems
- Content filtering and validation
- Selective message processing
- Traffic shaping and throttling

## 🛠️ Building Examples

### Prerequisites

- QtForge library v3.0.0+ with Communication module
- Qt6 with Core and Network modules
- CMake 3.21 or later
- C++20 compatible compiler

### Build All Examples

```bash
# From the communication_examples directory
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# Run tests
ctest --output-on-failure
```

### Build Individual Example

```bash
# Build specific example
cmake --build . --target advanced_message_bus

# Run the example
./advanced_message_bus
```

## 🚀 Running Examples

### Advanced MessageBus Example

```bash
./advanced_message_bus

# Expected output:
# QtForge Communication Examples - Advanced MessageBus
# ================================================
# Creating message bus...
# Demonstrating type-safe messaging...
# Publishing high-priority message...
# Message received: {"type": "system", "priority": "high", "data": "..."}
# Statistics: 15 messages sent, 12 delivered, 0 failed
```

### Request-Response Example

```bash
./request_response_example

# Expected output:
# QtForge Communication Examples - Request-Response
# ===============================================
# Setting up request-response handlers...
# Sending synchronous request...
# Response received: {"status": "success", "data": "..."}
# Async request completed in 45ms
```

## 📖 Learning Path

### 🎯 Beginner Level

1. **Advanced MessageBus** - Learn core messaging concepts and patterns
2. **Event Broadcasting** - Understand event-driven architecture
3. **Message Filtering** - Master message routing and filtering

### 🚀 Intermediate Level

4. **Request-Response** - Implement synchronous communication patterns
5. **Performance Optimization** - Optimize messaging performance
6. **Error Handling** - Handle communication failures gracefully

### 🏆 Advanced Level

7. **Custom Message Types** - Create domain-specific message types
8. **Message Persistence** - Implement message durability and replay
9. **Distributed Messaging** - Scale across multiple processes/machines

## 🔧 Common Patterns

### Type-Safe Message Publishing

```cpp
// Define custom message type
struct DataUpdateMessage : public qtplugin::IMessage {
    std::string data_id;
    QJsonObject payload;
    std::chrono::system_clock::time_point timestamp;
    
    std::string type() const override { return "data_update"; }
    QJsonObject to_json() const override;
};

// Publish message
auto message = std::make_shared<DataUpdateMessage>();
message->data_id = "user_profile_123";
message->payload = user_data;
message->timestamp = std::chrono::system_clock::now();

bus.publish(message, qtplugin::DeliveryMode::Reliable);
```

### Request-Response Pattern

```cpp
// Send request and wait for response
auto request = qtplugin::RequestMessage::create("user_service", "get_profile");
request->set_parameter("user_id", "123");
request->set_timeout(std::chrono::seconds(5));

auto response_future = bus.send_request(request);
auto response = response_future.get(); // Blocks until response or timeout

if (response.has_value()) {
    auto profile = response.value()->get_parameter("profile");
    // Process profile data
}
```

### Event Subscription with Filtering

```cpp
// Subscribe with content filter
bus.subscribe<DataUpdateMessage>("my_plugin", 
    [](const DataUpdateMessage& msg) {
        // Handle message
        qDebug() << "Data updated:" << msg.data_id;
    },
    [](const DataUpdateMessage& msg) -> bool {
        // Filter: only process messages for specific data types
        return msg.data_id.starts_with("user_profile_");
    }
);
```

## ✅ Best Practices

### 🏗️ Architecture

1. **Use Type-Safe Messages** - Define strongly-typed message classes
2. **Implement Proper Error Handling** - Handle communication failures gracefully
3. **Design for Scalability** - Consider message volume and performance
4. **Use Appropriate Delivery Modes** - Choose between reliable and fast delivery

### 🔧 Implementation

5. **Validate Message Content** - Always validate incoming messages
6. **Implement Timeouts** - Set appropriate timeouts for requests
7. **Monitor Performance** - Track message throughput and latency
8. **Handle Backpressure** - Implement flow control for high-volume scenarios

### 🧪 Testing

9. **Test Message Flows** - Verify end-to-end message delivery
10. **Test Error Scenarios** - Ensure proper error handling
11. **Performance Testing** - Validate performance under load
12. **Integration Testing** - Test with real plugin scenarios

## 🔧 Troubleshooting

### Common Issues

#### 🚫 Message Delivery Failures

**Symptoms:**
- Messages not reaching subscribers
- Timeout errors in request-response
- High message loss rates

**Solutions:**
- Check subscriber registration and lifecycle
- Verify message bus configuration
- Monitor system resources and performance
- Implement proper error handling and retry logic

#### ⚙️ Performance Issues

**Symptoms:**
- High message latency
- Memory usage growth
- CPU usage spikes during messaging

**Solutions:**
- Optimize message serialization
- Implement message batching
- Use appropriate delivery modes
- Monitor and tune message bus parameters

## 📄 License

All examples are provided under the same MIT license as the QtForge library.
