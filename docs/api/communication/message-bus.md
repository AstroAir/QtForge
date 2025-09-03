# MessageBus

The `MessageBus` class provides a centralized communication system for inter-plugin messaging using publish-subscribe and request-response patterns.

## Overview

The MessageBus enables:
- Type-safe message publishing and subscription
- Asynchronous and synchronous communication
- Topic-based message routing
- Message filtering and transformation
- Error handling and delivery guarantees

## Class Declaration

```cpp
namespace qtforge::communication {

class QTFORGE_EXPORT MessageBus {
public:
    // Construction and destruction
    MessageBus();
    explicit MessageBus(const MessageBusConfig& config);
    ~MessageBus();
    
    // Publishing
    template<typename T>
    expected<void, Error> publish(const std::string& topic, const T& message);
    template<typename T>
    expected<void, Error> publish(const std::string& topic, std::shared_ptr<T> message);
    
    // Subscription
    template<typename T>
    SubscriptionHandle subscribe(const std::string& topic, std::function<void(const T&)> handler);
    template<typename T>
    SubscriptionHandle subscribe(const std::string& topic, std::function<void(std::shared_ptr<T>)> handler);
    
    // Pattern-based subscription
    template<typename T>
    SubscriptionHandle subscribePattern(const std::string& pattern, std::function<void(const std::string&, const T&)> handler);
    
    // Unsubscription
    expected<void, Error> unsubscribe(SubscriptionHandle handle);
    expected<void, Error> unsubscribeAll(const std::string& topic);
    
    // Request-Response
    template<typename TRequest, typename TResponse>
    expected<TResponse, Error> request(const std::string& topic, const TRequest& request, std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));
    
    template<typename TRequest, typename TResponse>
    SubscriptionHandle respondTo(const std::string& topic, std::function<TResponse(const TRequest&)> handler);
    
    // Message filtering
    template<typename T>
    SubscriptionHandle subscribeFiltered(const std::string& topic, 
                                       std::function<bool(const T&)> filter,
                                       std::function<void(const T&)> handler);
    
    // Topic management
    std::vector<std::string> getTopics() const;
    size_t getSubscriberCount(const std::string& topic) const;
    
    // Configuration
    void setConfig(const MessageBusConfig& config);
    const MessageBusConfig& getConfig() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

// Subscription handle for managing subscriptions
class QTFORGE_EXPORT SubscriptionHandle {
public:
    SubscriptionHandle();
    SubscriptionHandle(const SubscriptionHandle&) = delete;
    SubscriptionHandle(SubscriptionHandle&& other) noexcept;
    SubscriptionHandle& operator=(SubscriptionHandle&& other) noexcept;
    ~SubscriptionHandle();
    
    bool isValid() const;
    std::string getTopic() const;
    void unsubscribe();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace qtforge::communication
```

## Core Methods

### Publishing Messages

#### publish()

Publishes a message to a specific topic.

```cpp
// Publish by value
MessageBus bus;
MyMessage message{"Hello, World!"};
auto result = bus.publish("notifications", message);
if (!result) {
    std::cerr << "Failed to publish: " << result.error().message() << std::endl;
}

// Publish by shared_ptr
auto sharedMessage = std::make_shared<MyMessage>("Hello, World!");
bus.publish("notifications", sharedMessage);
```

**Template Parameters:**
- `T`: Message type

**Parameters:**
- `topic`: Topic name to publish to
- `message`: Message to publish

**Returns:**
- `expected<void, Error>`: Success or error information

### Subscribing to Messages

#### subscribe()

Subscribes to messages on a specific topic.

```cpp
// Subscribe with lambda
auto handle = bus.subscribe<MyMessage>("notifications", 
    [](const MyMessage& msg) {
        std::cout << "Received: " << msg.content << std::endl;
    });

// Subscribe with member function
class MyPlugin : public IPlugin {
public:
    void initialize() {
        handle_ = bus.subscribe<DataMessage>("data.updates",
            [this](const DataMessage& msg) { handleData(msg); });
    }
    
private:
    void handleData(const DataMessage& msg) {
        // Process data message
    }
    
    SubscriptionHandle handle_;
};
```

#### subscribePattern()

Subscribes to messages using topic patterns.

```cpp
// Subscribe to all topics starting with "system."
auto handle = bus.subscribePattern<SystemMessage>("system.*",
    [](const std::string& topic, const SystemMessage& msg) {
        std::cout << "System message on " << topic << ": " << msg.content << std::endl;
    });

// Subscribe to all error topics
auto errorHandle = bus.subscribePattern<ErrorMessage>("*.error",
    [](const std::string& topic, const ErrorMessage& msg) {
        logError(topic, msg);
    });
```

### Request-Response Communication

#### request()

Sends a request and waits for a response.

```cpp
// Synchronous request
DatabaseQuery query{"SELECT * FROM users"};
auto result = bus.request<DatabaseQuery, DatabaseResult>("database.query", query);
if (result) {
    auto response = result.value();
    std::cout << "Query returned " << response.rows.size() << " rows" << std::endl;
} else {
    std::cerr << "Query failed: " << result.error().message() << std::endl;
}

// Request with custom timeout
auto result = bus.request<DatabaseQuery, DatabaseResult>("database.query", query, 
                                                        std::chrono::seconds(10));
```

#### respondTo()

Registers a handler to respond to requests.

```cpp
// Register request handler
auto handle = bus.respondTo<DatabaseQuery, DatabaseResult>("database.query",
    [this](const DatabaseQuery& query) -> DatabaseResult {
        return executeQuery(query);
    });

// Async request handler
auto handle = bus.respondTo<FileRequest, FileContent>("file.read",
    [this](const FileRequest& request) -> FileContent {
        return readFile(request.filename);
    });
```

## Message Filtering

### subscribeFiltered()

Subscribes to messages with filtering criteria.

```cpp
// Filter by message priority
auto handle = bus.subscribeFiltered<LogMessage>("logs",
    [](const LogMessage& msg) { return msg.priority >= LogPriority::Warning; },
    [](const LogMessage& msg) { 
        std::cout << "Warning/Error: " << msg.message << std::endl; 
    });

// Filter by user ID
auto handle = bus.subscribeFiltered<UserAction>("user.actions",
    [userId](const UserAction& action) { return action.userId == userId; },
    [this](const UserAction& action) { handleUserAction(action); });
```

## Configuration

### MessageBusConfig

Configuration options for the MessageBus:

```cpp
struct MessageBusConfig {
    // Threading
    size_t workerThreads = 4;
    bool useAsyncDelivery = true;
    
    // Queue management
    size_t maxQueueSize = 10000;
    QueueOverflowPolicy overflowPolicy = QueueOverflowPolicy::DropOldest;
    
    // Delivery guarantees
    DeliveryMode defaultDeliveryMode = DeliveryMode::AtLeastOnce;
    std::chrono::milliseconds deliveryTimeout = std::chrono::seconds(30);
    
    // Message persistence
    bool enablePersistence = false;
    std::string persistenceDirectory = "message_store";
    
    // Monitoring
    bool enableMetrics = true;
    std::chrono::milliseconds metricsInterval = std::chrono::seconds(60);
};
```

### Usage Example

```cpp
MessageBusConfig config;
config.workerThreads = 8;
config.maxQueueSize = 50000;
config.enablePersistence = true;
config.enableMetrics = true;

MessageBus bus(config);
```

## Message Types

### Base Message Interface

```cpp
class IMessage {
public:
    virtual ~IMessage() = default;
    virtual std::string getType() const = 0;
    virtual std::chrono::system_clock::time_point getTimestamp() const = 0;
    virtual std::string getSender() const = 0;
};
```

### Standard Message Types

```cpp
// Simple text message
struct TextMessage : public IMessage {
    std::string content;
    std::string sender;
    std::chrono::system_clock::time_point timestamp;
    
    std::string getType() const override { return "text"; }
    std::chrono::system_clock::time_point getTimestamp() const override { return timestamp; }
    std::string getSender() const override { return sender; }
};

// Data message with binary payload
struct DataMessage : public IMessage {
    std::vector<uint8_t> data;
    std::string contentType;
    std::map<std::string, std::string> headers;
    
    std::string getType() const override { return "data"; }
    // ... other implementations
};

// Event message
struct EventMessage : public IMessage {
    std::string eventType;
    std::map<std::string, std::any> properties;
    
    std::string getType() const override { return "event"; }
    // ... other implementations
};
```

## Advanced Features

### Message Transformation

```cpp
// Transform messages before delivery
bus.addTransformer("logs", [](const LogMessage& input) -> LogMessage {
    LogMessage transformed = input;
    transformed.message = "[TRANSFORMED] " + input.message;
    return transformed;
});
```

### Topic Hierarchies

```cpp
// Hierarchical topic structure
bus.publish("system.plugins.loaded", PluginLoadedEvent{});
bus.publish("system.plugins.unloaded", PluginUnloadedEvent{});
bus.publish("system.errors.critical", CriticalErrorEvent{});

// Subscribe to all system events
auto handle = bus.subscribePattern<SystemEvent>("system.*", handler);

// Subscribe to all plugin events
auto pluginHandle = bus.subscribePattern<PluginEvent>("system.plugins.*", pluginHandler);
```

### Message Priorities

```cpp
enum class MessagePriority {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

// Publish with priority
bus.publishWithPriority("alerts", criticalAlert, MessagePriority::Critical);
```

## Error Handling

### Common Error Types

```cpp
enum class MessageBusError {
    TopicNotFound,
    SubscriberNotFound,
    DeliveryFailed,
    TimeoutExpired,
    QueueFull,
    SerializationFailed,
    InvalidMessage
};
```

### Error Handling Example

```cpp
auto result = bus.publish("invalid.topic", message);
if (!result) {
    auto error = result.error();
    switch (error.code()) {
        case MessageBusError::QueueFull:
            std::cerr << "Message queue is full" << std::endl;
            break;
        case MessageBusError::SerializationFailed:
            std::cerr << "Failed to serialize message" << std::endl;
            break;
        default:
            std::cerr << "Unknown error: " << error.message() << std::endl;
    }
}
```

## Thread Safety

The MessageBus is fully thread-safe and can be used from multiple threads concurrently.

```cpp
// Safe to publish from multiple threads
std::thread t1([&bus]() {
    bus.publish("thread1", Message{"From thread 1"});
});

std::thread t2([&bus]() {
    bus.publish("thread2", Message{"From thread 2"});
});

t1.join();
t2.join();
```

## Performance Considerations

### Async vs Sync Delivery

```cpp
// Async delivery (default) - non-blocking
bus.publish("topic", message);  // Returns immediately

// Sync delivery - blocks until delivered
MessageBusConfig config;
config.useAsyncDelivery = false;
MessageBus syncBus(config);
syncBus.publish("topic", message);  // Blocks until delivered
```

### Message Batching

```cpp
// Batch multiple messages for efficiency
std::vector<LogMessage> messages = getLogMessages();
bus.publishBatch("logs", messages);
```

## Best Practices

1. **Topic Naming**: Use hierarchical topic names (e.g., "system.plugins.loaded")
2. **Message Design**: Keep messages immutable and serializable
3. **Error Handling**: Always check return values and handle errors
4. **Resource Management**: Use RAII for subscription handles
5. **Performance**: Consider async delivery for high-throughput scenarios

## See Also

- **[Message Types](message-types.md)**: Standard message type definitions
- **[Plugin Communication](../../user-guide/plugin-development.md#plugin-communication)**: Plugin communication patterns
- **[Error Handling](../utils/error-handling.md)**: Error handling utilities
