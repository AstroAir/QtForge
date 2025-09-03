# Advanced Plugin Examples

This section covers advanced plugin development patterns and techniques for building sophisticated QtForge applications.

## Plugin Composition

### Composite Plugin Pattern

Create plugins that aggregate functionality from multiple sub-plugins:

```cpp
class CompositeDataProcessor : public qtforge::IPlugin {
private:
    std::vector<std::shared_ptr<IDataProcessor>> processors_;
    
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        // Load and initialize sub-processors
        auto jsonProcessor = loadSubPlugin<JsonProcessor>("json-processor");
        auto xmlProcessor = loadSubPlugin<XmlProcessor>("xml-processor");
        auto csvProcessor = loadSubPlugin<CsvProcessor>("csv-processor");
        
        if (jsonProcessor) processors_.push_back(jsonProcessor.value());
        if (xmlProcessor) processors_.push_back(xmlProcessor.value());
        if (csvProcessor) processors_.push_back(csvProcessor.value());
        
        return {};
    }
    
    ProcessingResult processData(const std::string& data, const std::string& format) {
        for (auto& processor : processors_) {
            if (processor->supportsFormat(format)) {
                return processor->process(data);
            }
        }
        return ProcessingResult::error("Unsupported format: " + format);
    }
};
```

## Message Bus Integration

### Advanced Message Patterns

#### Request-Response with Timeout

```cpp
class DatabasePlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        auto& bus = qtforge::MessageBus::instance();
        
        // Handle database queries
        queryHandle_ = bus.respondTo<DatabaseQuery, DatabaseResult>("database.query",
            [this](const DatabaseQuery& query) -> DatabaseResult {
                return executeQuery(query);
            });
        
        return {};
    }
    
private:
    DatabaseResult executeQuery(const DatabaseQuery& query) {
        try {
            // Execute SQL query
            auto connection = getConnection();
            auto statement = connection->prepare(query.sql);
            
            // Bind parameters
            for (size_t i = 0; i < query.parameters.size(); ++i) {
                statement->bind(i + 1, query.parameters[i]);
            }
            
            // Execute and return results
            auto resultSet = statement->execute();
            return DatabaseResult{resultSet->getRows(), true, ""};
            
        } catch (const std::exception& e) {
            return DatabaseResult{{}, false, e.what()};
        }
    }
    
    qtforge::SubscriptionHandle queryHandle_;
};
```

#### Event Aggregation

```cpp
class EventAggregatorPlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        auto& bus = qtforge::MessageBus::instance();
        
        // Subscribe to various event types
        userEventHandle_ = bus.subscribe<UserEvent>("user.*", 
            [this](const UserEvent& event) { aggregateUserEvent(event); });
        
        systemEventHandle_ = bus.subscribe<SystemEvent>("system.*",
            [this](const SystemEvent& event) { aggregateSystemEvent(event); });
        
        // Publish aggregated events periodically
        aggregationTimer_ = std::make_unique<QTimer>();
        connect(aggregationTimer_.get(), &QTimer::timeout, 
                this, &EventAggregatorPlugin::publishAggregatedEvents);
        aggregationTimer_->start(std::chrono::seconds(60));
        
        return {};
    }
    
private:
    void aggregateUserEvent(const UserEvent& event) {
        std::lock_guard<std::mutex> lock(aggregationMutex_);
        userEventCounts_[event.type]++;
        lastUserEventTime_ = std::chrono::system_clock::now();
    }
    
    void publishAggregatedEvents() {
        std::lock_guard<std::mutex> lock(aggregationMutex_);
        
        if (!userEventCounts_.empty()) {
            AggregatedUserEvents aggregated;
            aggregated.eventCounts = userEventCounts_;
            aggregated.timeWindow = std::chrono::minutes(1);
            aggregated.timestamp = std::chrono::system_clock::now();
            
            auto& bus = qtforge::MessageBus::instance();
            bus.publish("analytics.user_events", aggregated);
            
            userEventCounts_.clear();
        }
    }
    
    std::mutex aggregationMutex_;
    std::map<std::string, int> userEventCounts_;
    std::chrono::system_clock::time_point lastUserEventTime_;
    std::unique_ptr<QTimer> aggregationTimer_;
    qtforge::SubscriptionHandle userEventHandle_;
    qtforge::SubscriptionHandle systemEventHandle_;
};
```

## Security

### Secure Plugin Communication

```cpp
class SecureMessagingPlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        // Initialize encryption
        cryptoProvider_ = std::make_unique<CryptoProvider>();
        auto keyResult = cryptoProvider_->generateKeyPair();
        if (!keyResult) {
            return qtforge::Error("Failed to generate encryption keys");
        }
        
        keyPair_ = keyResult.value();
        
        // Subscribe to secure messages
        auto& bus = qtforge::MessageBus::instance();
        secureHandle_ = bus.subscribe<EncryptedMessage>("secure.messages",
            [this](const EncryptedMessage& msg) { handleSecureMessage(msg); });
        
        return {};
    }
    
    void sendSecureMessage(const std::string& recipient, const std::string& content) {
        // Get recipient's public key
        auto publicKey = getRecipientPublicKey(recipient);
        if (!publicKey) {
            qtforge::Logger::error("SecureMessaging", "Public key not found for: " + recipient);
            return;
        }
        
        // Encrypt message
        auto encryptedContent = cryptoProvider_->encrypt(content, publicKey.value());
        if (!encryptedContent) {
            qtforge::Logger::error("SecureMessaging", "Failed to encrypt message");
            return;
        }
        
        // Create encrypted message
        EncryptedMessage msg;
        msg.sender = name();
        msg.recipient = recipient;
        msg.encryptedContent = encryptedContent.value();
        msg.signature = cryptoProvider_->sign(content, keyPair_.privateKey);
        msg.timestamp = std::chrono::system_clock::now();
        
        // Send message
        auto& bus = qtforge::MessageBus::instance();
        bus.publish("secure.messages", msg);
    }
    
private:
    void handleSecureMessage(const EncryptedMessage& msg) {
        if (msg.recipient != name()) {
            return; // Not for us
        }
        
        // Verify signature
        auto senderPublicKey = getRecipientPublicKey(msg.sender);
        if (!senderPublicKey) {
            qtforge::Logger::warning("SecureMessaging", "Unknown sender: " + msg.sender);
            return;
        }
        
        // Decrypt message
        auto decryptedContent = cryptoProvider_->decrypt(msg.encryptedContent, keyPair_.privateKey);
        if (!decryptedContent) {
            qtforge::Logger::error("SecureMessaging", "Failed to decrypt message");
            return;
        }
        
        // Verify signature
        if (!cryptoProvider_->verify(decryptedContent.value(), msg.signature, senderPublicKey.value())) {
            qtforge::Logger::error("SecureMessaging", "Message signature verification failed");
            return;
        }
        
        // Process decrypted message
        processDecryptedMessage(msg.sender, decryptedContent.value());
    }
    
    std::unique_ptr<CryptoProvider> cryptoProvider_;
    KeyPair keyPair_;
    qtforge::SubscriptionHandle secureHandle_;
};
```

### Permission-Based Access Control

```cpp
class PermissionControlledPlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        // Request required permissions
        auto& permissionManager = qtforge::PermissionManager::instance();
        
        auto filePermission = permissionManager.requestPermission(name(), 
            qtforge::Permission::FileAccess, "/data/plugins/");
        if (!filePermission) {
            return qtforge::Error("File access permission denied");
        }
        
        auto networkPermission = permissionManager.requestPermission(name(),
            qtforge::Permission::NetworkAccess, "api.example.com:443");
        if (!networkPermission) {
            return qtforge::Error("Network access permission denied");
        }
        
        return {};
    }
    
    qtforge::expected<std::string, qtforge::Error> readFile(const std::string& filename) {
        // Check permission before file access
        auto& permissionManager = qtforge::PermissionManager::instance();
        auto hasPermission = permissionManager.checkPermission(name(), 
            qtforge::Permission::FileAccess, filename);
        
        if (!hasPermission) {
            return qtforge::Error("Permission denied: " + filename);
        }
        
        // Perform file operation
        std::ifstream file(filename);
        if (!file.is_open()) {
            return qtforge::Error("Failed to open file: " + filename);
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
    }
};
```

## Performance Optimization

### Async Processing with Thread Pools

```cpp
class AsyncProcessingPlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        // Create thread pool
        threadPool_ = std::make_unique<qtforge::ThreadPool>(
            std::thread::hardware_concurrency());
        
        // Subscribe to processing requests
        auto& bus = qtforge::MessageBus::instance();
        requestHandle_ = bus.subscribe<ProcessingRequest>("processing.requests",
            [this](const ProcessingRequest& request) { 
                handleProcessingRequest(request); 
            });
        
        return {};
    }
    
private:
    void handleProcessingRequest(const ProcessingRequest& request) {
        // Submit async task
        auto future = threadPool_->submit([this, request]() -> ProcessingResult {
            return performHeavyProcessing(request);
        });
        
        // Store future for later retrieval
        std::lock_guard<std::mutex> lock(futuresMutex_);
        pendingResults_[request.id] = std::move(future);
        
        // Schedule result publication
        QTimer::singleShot(0, [this, requestId = request.id]() {
            publishResultWhenReady(requestId);
        });
    }
    
    void publishResultWhenReady(const std::string& requestId) {
        std::unique_lock<std::mutex> lock(futuresMutex_);
        auto it = pendingResults_.find(requestId);
        if (it == pendingResults_.end()) {
            return;
        }
        
        auto& future = it->second;
        if (future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            try {
                auto result = future.get();
                lock.unlock();
                
                // Publish result
                ProcessingResponse response;
                response.requestId = requestId;
                response.result = result;
                response.timestamp = std::chrono::system_clock::now();
                
                auto& bus = qtforge::MessageBus::instance();
                bus.publish("processing.responses", response);
                
                // Clean up
                lock.lock();
                pendingResults_.erase(it);
                
            } catch (const std::exception& e) {
                lock.unlock();
                
                // Publish error
                ProcessingResponse errorResponse;
                errorResponse.requestId = requestId;
                errorResponse.error = e.what();
                errorResponse.timestamp = std::chrono::system_clock::now();
                
                auto& bus = qtforge::MessageBus::instance();
                bus.publish("processing.responses", errorResponse);
                
                lock.lock();
                pendingResults_.erase(it);
            }
        } else {
            // Not ready yet, check again later
            lock.unlock();
            QTimer::singleShot(100, [this, requestId]() {
                publishResultWhenReady(requestId);
            });
        }
    }
    
    ProcessingResult performHeavyProcessing(const ProcessingRequest& request) {
        // Simulate heavy processing
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        ProcessingResult result;
        result.data = "Processed: " + request.inputData;
        result.processingTime = std::chrono::seconds(2);
        result.success = true;
        
        return result;
    }
    
    std::unique_ptr<qtforge::ThreadPool> threadPool_;
    std::mutex futuresMutex_;
    std::map<std::string, std::future<ProcessingResult>> pendingResults_;
    qtforge::SubscriptionHandle requestHandle_;
};
```

### Memory-Efficient Data Processing

```cpp
class StreamProcessingPlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        // Initialize memory pool
        memoryPool_ = std::make_unique<qtforge::MemoryPool>(1024 * 1024); // 1MB pool
        
        // Subscribe to data streams
        auto& bus = qtforge::MessageBus::instance();
        streamHandle_ = bus.subscribe<DataChunk>("data.stream",
            [this](const DataChunk& chunk) { processDataChunk(chunk); });
        
        return {};
    }
    
private:
    void processDataChunk(const DataChunk& chunk) {
        // Allocate from memory pool
        auto buffer = memoryPool_->allocate(chunk.size);
        if (!buffer) {
            qtforge::Logger::warning("StreamProcessing", "Memory pool exhausted");
            return;
        }
        
        // Process data in-place
        std::memcpy(buffer.get(), chunk.data.data(), chunk.size);
        
        // Apply transformations
        applyTransformations(buffer.get(), chunk.size);
        
        // Create result chunk
        DataChunk processedChunk;
        processedChunk.id = chunk.id;
        processedChunk.data.assign(buffer.get(), buffer.get() + chunk.size);
        processedChunk.size = chunk.size;
        processedChunk.timestamp = std::chrono::system_clock::now();
        
        // Publish processed data
        auto& bus = qtforge::MessageBus::instance();
        bus.publish("data.processed", processedChunk);
        
        // Buffer automatically returned to pool when buffer goes out of scope
    }
    
    void applyTransformations(uint8_t* data, size_t size) {
        // Example: simple XOR transformation
        for (size_t i = 0; i < size; ++i) {
            data[i] ^= 0xAA;
        }
    }
    
    std::unique_ptr<qtforge::MemoryPool> memoryPool_;
    qtforge::SubscriptionHandle streamHandle_;
};
```

## Key Advanced Concepts

### 1. Plugin Composition
- Aggregate multiple plugins into composite functionality
- Dynamic plugin loading and unloading
- Hierarchical plugin structures

### 2. Advanced Messaging
- Request-response patterns with timeouts
- Event aggregation and analytics
- Message transformation and filtering

### 3. Security
- Encrypted inter-plugin communication
- Permission-based access control
- Digital signatures and verification

### 4. Performance
- Asynchronous processing with thread pools
- Memory-efficient data handling
- Resource pooling and reuse

## Best Practices

1. **Resource Management**: Use RAII and smart pointers consistently
2. **Error Handling**: Implement comprehensive error handling and recovery
3. **Thread Safety**: Design for concurrent access from the start
4. **Performance**: Profile and optimize critical paths
5. **Security**: Validate all inputs and implement proper access controls

## See Also

- **[Basic Plugin Example](basic-plugin.md)**: Start with basic concepts
- **[Service Plugin Example](service-plugin.md)**: Service-oriented plugins
- **[Plugin Architecture](../user-guide/plugin-architecture.md)**: Architectural patterns
- **[API Reference](../api/index.md)**: Detailed API documentation
