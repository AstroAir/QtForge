# Message Types

QtForge provides a comprehensive set of message types for inter-plugin communication through the MessageBus system.

## Overview

Message types in QtForge are designed to:
- Provide type-safe communication
- Enable structured data exchange
- Support various communication patterns
- Facilitate debugging and monitoring
- Ensure compatibility across plugin versions

## Base Message Interface

### IMessage

All messages inherit from the base `IMessage` interface:

```cpp
namespace qtforge::communication {

class IMessage {
public:
    virtual ~IMessage() = default;
    
    // Message identification
    virtual std::string getType() const = 0;
    virtual std::string getMessageId() const = 0;
    virtual std::string getSender() const = 0;
    virtual std::string getReceiver() const = 0;
    
    // Timing information
    virtual std::chrono::system_clock::time_point getTimestamp() const = 0;
    virtual std::chrono::milliseconds getTtl() const = 0;
    
    // Message properties
    virtual MessagePriority getPriority() const = 0;
    virtual bool requiresAcknowledgment() const = 0;
    virtual std::map<std::string, std::string> getHeaders() const = 0;
    
    // Serialization
    virtual std::string serialize() const = 0;
    virtual void deserialize(const std::string& data) = 0;
};

enum class MessagePriority {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

} // namespace qtforge::communication
```

## Core Message Types

### TextMessage

Simple text-based message for basic communication:

```cpp
struct TextMessage : public IMessage {
    std::string content;
    std::string sender;
    std::string receiver;
    std::chrono::system_clock::time_point timestamp;
    MessagePriority priority = MessagePriority::Normal;
    
    // IMessage implementation
    std::string getType() const override { return "text"; }
    std::string getMessageId() const override { return messageId; }
    std::string getSender() const override { return sender; }
    std::string getReceiver() const override { return receiver; }
    std::chrono::system_clock::time_point getTimestamp() const override { return timestamp; }
    
    // Usage example
    TextMessage msg;
    msg.content = "Hello from plugin A";
    msg.sender = "PluginA";
    msg.receiver = "PluginB";
    msg.timestamp = std::chrono::system_clock::now();
    
    messageBus.publish("text.messages", msg);
};
```

### DataMessage

Binary data message for efficient data transfer:

```cpp
struct DataMessage : public IMessage {
    std::vector<uint8_t> data;
    std::string contentType;
    std::string encoding;
    size_t dataSize;
    std::map<std::string, std::string> metadata;
    
    std::string getType() const override { return "data"; }
    
    // Convenience methods
    void setTextData(const std::string& text) {
        data.assign(text.begin(), text.end());
        contentType = "text/plain";
        dataSize = text.size();
    }
    
    void setJsonData(const QJsonObject& json) {
        QJsonDocument doc(json);
        auto jsonBytes = doc.toJson(QJsonDocument::Compact);
        data.assign(jsonBytes.begin(), jsonBytes.end());
        contentType = "application/json";
        dataSize = jsonBytes.size();
    }
    
    void setBinaryData(const std::vector<uint8_t>& binaryData, 
                      const std::string& mimeType) {
        data = binaryData;
        contentType = mimeType;
        dataSize = binaryData.size();
    }
};
```

### EventMessage

Event notification message for publish-subscribe patterns:

```cpp
struct EventMessage : public IMessage {
    std::string eventType;
    std::string source;
    std::map<std::string, std::any> properties;
    std::chrono::system_clock::time_point eventTime;
    
    std::string getType() const override { return "event"; }
    
    // Event type helpers
    static EventMessage createSystemEvent(const std::string& eventType, 
                                        const std::string& source) {
        EventMessage msg;
        msg.eventType = "system." + eventType;
        msg.source = source;
        msg.eventTime = std::chrono::system_clock::now();
        return msg;
    }
    
    static EventMessage createPluginEvent(const std::string& eventType, 
                                        const std::string& pluginName) {
        EventMessage msg;
        msg.eventType = "plugin." + eventType;
        msg.source = pluginName;
        msg.eventTime = std::chrono::system_clock::now();
        return msg;
    }
    
    // Property helpers
    template<typename T>
    void setProperty(const std::string& key, const T& value) {
        properties[key] = value;
    }
    
    template<typename T>
    T getProperty(const std::string& key, const T& defaultValue = T{}) const {
        auto it = properties.find(key);
        if (it != properties.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
};
```

## Request-Response Messages

### RequestMessage

Base class for request messages:

```cpp
template<typename TResponse>
struct RequestMessage : public IMessage {
    std::string requestId;
    std::string operation;
    std::map<std::string, std::any> parameters;
    std::chrono::milliseconds timeout = std::chrono::seconds(30);
    std::string responseChannel;
    
    std::string getType() const override { return "request"; }
    std::string getMessageId() const override { return requestId; }
    
    // Generate unique request ID
    void generateRequestId() {
        requestId = generateUuid();
        responseChannel = "response." + requestId;
    }
    
    // Parameter helpers
    template<typename T>
    void setParameter(const std::string& key, const T& value) {
        parameters[key] = value;
    }
    
    template<typename T>
    T getParameter(const std::string& key, const T& defaultValue = T{}) const {
        auto it = parameters.find(key);
        if (it != parameters.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
};
```

### ResponseMessage

Base class for response messages:

```cpp
template<typename TData>
struct ResponseMessage : public IMessage {
    std::string requestId;
    bool success;
    TData data;
    std::string errorMessage;
    int errorCode = 0;
    std::chrono::milliseconds processingTime;
    
    std::string getType() const override { return "response"; }
    std::string getMessageId() const override { return requestId; }
    
    // Success response
    static ResponseMessage<TData> createSuccess(const std::string& requestId, 
                                              const TData& responseData) {
        ResponseMessage<TData> msg;
        msg.requestId = requestId;
        msg.success = true;
        msg.data = responseData;
        msg.timestamp = std::chrono::system_clock::now();
        return msg;
    }
    
    // Error response
    static ResponseMessage<TData> createError(const std::string& requestId, 
                                            const std::string& error, 
                                            int code = -1) {
        ResponseMessage<TData> msg;
        msg.requestId = requestId;
        msg.success = false;
        msg.errorMessage = error;
        msg.errorCode = code;
        msg.timestamp = std::chrono::system_clock::now();
        return msg;
    }
};
```

## Specialized Message Types

### DatabaseMessage

Messages for database operations:

```cpp
struct DatabaseQueryMessage : public RequestMessage<DatabaseResult> {
    std::string query;
    std::vector<std::any> parameters;
    std::string database;
    
    DatabaseQueryMessage(const std::string& sql) : query(sql) {
        operation = "query";
        generateRequestId();
    }
};

struct DatabaseResult {
    std::vector<std::map<std::string, std::any>> rows;
    size_t affectedRows = 0;
    std::string lastInsertId;
    std::chrono::milliseconds executionTime;
};

using DatabaseResponseMessage = ResponseMessage<DatabaseResult>;
```

### FileOperationMessage

Messages for file system operations:

```cpp
struct FileOperationMessage : public RequestMessage<FileOperationResult> {
    enum class Operation {
        Read,
        Write,
        Delete,
        Copy,
        Move,
        List
    };
    
    Operation fileOperation;
    std::string filePath;
    std::string destinationPath; // For copy/move operations
    std::vector<uint8_t> fileData; // For write operations
    
    FileOperationMessage(Operation op, const std::string& path) 
        : fileOperation(op), filePath(path) {
        operation = "file_operation";
        generateRequestId();
    }
};

struct FileOperationResult {
    bool success;
    std::vector<uint8_t> fileData; // For read operations
    std::vector<std::string> fileList; // For list operations
    size_t fileSize = 0;
    std::chrono::system_clock::time_point lastModified;
};
```

### NetworkMessage

Messages for network operations:

```cpp
struct HttpRequestMessage : public RequestMessage<HttpResponse> {
    std::string method;
    std::string url;
    std::map<std::string, std::string> headers;
    std::string body;
    
    HttpRequestMessage(const std::string& httpMethod, const std::string& httpUrl) 
        : method(httpMethod), url(httpUrl) {
        operation = "http_request";
        generateRequestId();
    }
};

struct HttpResponse {
    int statusCode;
    std::string statusText;
    std::map<std::string, std::string> headers;
    std::string body;
    std::chrono::milliseconds responseTime;
};

struct WebSocketMessage : public IMessage {
    enum class MessageType {
        Text,
        Binary,
        Ping,
        Pong,
        Close
    };
    
    MessageType messageType;
    std::string textData;
    std::vector<uint8_t> binaryData;
    std::string connectionId;
    
    std::string getType() const override { return "websocket"; }
};
```

## Plugin Lifecycle Messages

### PluginLifecycleMessage

Messages for plugin lifecycle events:

```cpp
struct PluginLifecycleMessage : public EventMessage {
    enum class LifecycleEvent {
        Loading,
        Loaded,
        Initializing,
        Initialized,
        Activating,
        Activated,
        Deactivating,
        Deactivated,
        Unloading,
        Unloaded,
        Error
    };
    
    LifecycleEvent lifecycleEvent;
    std::string pluginName;
    std::string pluginVersion;
    std::string errorMessage; // For error events
    
    PluginLifecycleMessage(LifecycleEvent event, const std::string& name) 
        : lifecycleEvent(event), pluginName(name) {
        eventType = "plugin.lifecycle";
        source = name;
        eventTime = std::chrono::system_clock::now();
    }
};
```

### ConfigurationMessage

Messages for configuration updates:

```cpp
struct ConfigurationMessage : public IMessage {
    std::string configurationKey;
    std::any configurationValue;
    std::string targetPlugin; // Empty for global config
    
    std::string getType() const override { return "configuration"; }
    
    template<typename T>
    void setValue(const T& value) {
        configurationValue = value;
    }
    
    template<typename T>
    T getValue(const T& defaultValue = T{}) const {
        try {
            return std::any_cast<T>(configurationValue);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
};
```

## Message Serialization

### JSON Serialization

```cpp
class JsonMessageSerializer {
public:
    static std::string serialize(const IMessage& message) {
        QJsonObject json;
        json["type"] = QString::fromStdString(message.getType());
        json["messageId"] = QString::fromStdString(message.getMessageId());
        json["sender"] = QString::fromStdString(message.getSender());
        json["receiver"] = QString::fromStdString(message.getReceiver());
        json["timestamp"] = static_cast<qint64>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                message.getTimestamp().time_since_epoch()).count());
        json["priority"] = static_cast<int>(message.getPriority());
        
        // Add message-specific data
        json["data"] = serializeMessageData(message);
        
        QJsonDocument doc(json);
        return doc.toJson(QJsonDocument::Compact).toStdString();
    }
    
    static std::unique_ptr<IMessage> deserialize(const std::string& jsonStr) {
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(jsonStr));
        if (doc.isNull()) {
            return nullptr;
        }
        
        QJsonObject json = doc.object();
        std::string messageType = json["type"].toString().toStdString();
        
        // Create message based on type
        return createMessageFromType(messageType, json);
    }

private:
    static QJsonValue serializeMessageData(const IMessage& message);
    static std::unique_ptr<IMessage> createMessageFromType(const std::string& type, 
                                                          const QJsonObject& json);
};
```

## Message Filtering

### Message Filters

```cpp
class MessageFilter {
public:
    virtual ~MessageFilter() = default;
    virtual bool matches(const IMessage& message) const = 0;
};

class TypeFilter : public MessageFilter {
public:
    TypeFilter(const std::string& messageType) : type_(messageType) {}
    
    bool matches(const IMessage& message) const override {
        return message.getType() == type_;
    }

private:
    std::string type_;
};

class SenderFilter : public MessageFilter {
public:
    SenderFilter(const std::string& sender) : sender_(sender) {}
    
    bool matches(const IMessage& message) const override {
        return message.getSender() == sender_;
    }

private:
    std::string sender_;
};

class PriorityFilter : public MessageFilter {
public:
    PriorityFilter(MessagePriority minPriority) : minPriority_(minPriority) {}
    
    bool matches(const IMessage& message) const override {
        return message.getPriority() >= minPriority_;
    }

private:
    MessagePriority minPriority_;
};
```

## Usage Examples

### Basic Message Exchange

```cpp
// Publisher
TextMessage msg;
msg.content = "Hello World";
msg.sender = "PublisherPlugin";
msg.timestamp = std::chrono::system_clock::now();

messageBus.publish("greetings", msg);

// Subscriber
auto handle = messageBus.subscribe<TextMessage>("greetings",
    [](const TextMessage& msg) {
        std::cout << "Received: " << msg.content 
                  << " from " << msg.sender << std::endl;
    });
```

### Request-Response Pattern

```cpp
// Client
DatabaseQueryMessage request("SELECT * FROM users WHERE active = ?");
request.parameters.push_back(true);

auto response = messageBus.request<DatabaseQueryMessage, DatabaseResponseMessage>(
    "database.query", request, std::chrono::seconds(10));

if (response && response.value().success) {
    auto result = response.value().data;
    std::cout << "Query returned " << result.rows.size() << " rows" << std::endl;
}

// Server
auto handle = messageBus.respondTo<DatabaseQueryMessage, DatabaseResponseMessage>(
    "database.query",
    [](const DatabaseQueryMessage& request) -> DatabaseResponseMessage {
        // Execute database query
        auto result = executeQuery(request.query, request.parameters);
        return DatabaseResponseMessage::createSuccess(request.requestId, result);
    });
```

## Best Practices

1. **Type Safety**: Use strongly typed messages for better error detection
2. **Versioning**: Include version information in message types
3. **Documentation**: Document message contracts clearly
4. **Error Handling**: Always handle message processing errors
5. **Performance**: Use appropriate message types for data size and frequency

## See Also

- **[MessageBus](message-bus.md)**: Message bus implementation
- **[Plugin Communication](../../user-guide/plugin-development.md#plugin-communication)**: Communication patterns
- **[Serialization](../utils/serialization.md)**: Message serialization utilities
