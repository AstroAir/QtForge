# Advanced Patterns

This guide covers advanced architectural patterns and design techniques for building sophisticated QtForge plugins and applications.

## Overview

Advanced patterns in QtForge enable:
- **Complex Architectures**: Sophisticated plugin compositions and interactions
- **Scalable Solutions**: Patterns that scale with application complexity
- **Maintainable Code**: Clean, extensible, and testable designs
- **Performance Optimization**: Efficient resource usage and execution
- **Fault Tolerance**: Robust error handling and recovery mechanisms

## Architectural Patterns

### 1. Plugin Composition Pattern

Create complex functionality by composing multiple plugins:

```cpp
class CompositeDataProcessor : public qtforge::IPlugin {
public:
    CompositeDataProcessor() : currentState_(qtforge::PluginState::Unloaded) {}
    
    // Plugin interface
    std::string name() const override { return "CompositeDataProcessor"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "Composite data processor using multiple specialized plugins";
    }
    
    std::vector<std::string> dependencies() const override {
        return {"DataValidatorPlugin >= 1.0.0", "DataTransformerPlugin >= 1.0.0", 
                "DataEnricherPlugin >= 1.0.0", "DataPersistencePlugin >= 1.0.0"};
    }
    
    qtforge::expected<void, qtforge::Error> initialize() override {
        try {
            // Get component plugins
            auto& pluginManager = qtforge::PluginManager::instance();
            
            validator_ = pluginManager.getPlugin("DataValidatorPlugin");
            transformer_ = pluginManager.getPlugin("DataTransformerPlugin");
            enricher_ = pluginManager.getPlugin("DataEnricherPlugin");
            persistence_ = pluginManager.getPlugin("DataPersistencePlugin");
            
            if (!validator_ || !transformer_ || !enricher_ || !persistence_) {
                return qtforge::Error("Required component plugins not available");
            }
            
            // Setup processing pipeline
            setupProcessingPipeline();
            
            currentState_ = qtforge::PluginState::Initialized;
            return {};
            
        } catch (const std::exception& e) {
            currentState_ = qtforge::PluginState::Error;
            return qtforge::Error("Composite processor initialization failed: " + std::string(e.what()));
        }
    }
    
    qtforge::expected<ProcessingResult, qtforge::Error> processData(const RawData& data) {
        // Execute processing pipeline
        return pipeline_.execute(data);
    }

private:
    void setupProcessingPipeline() {
        pipeline_.addStage("validation", [this](const auto& data) {
            return std::dynamic_pointer_cast<DataValidatorPlugin>(validator_)->validate(data);
        });
        
        pipeline_.addStage("transformation", [this](const auto& data) {
            return std::dynamic_pointer_cast<DataTransformerPlugin>(transformer_)->transform(data);
        });
        
        pipeline_.addStage("enrichment", [this](const auto& data) {
            return std::dynamic_pointer_cast<DataEnricherPlugin>(enricher_)->enrich(data);
        });
        
        pipeline_.addStage("persistence", [this](const auto& data) {
            return std::dynamic_pointer_cast<DataPersistencePlugin>(persistence_)->persist(data);
        });
    }
    
    qtforge::PluginState currentState_;
    std::shared_ptr<qtforge::IPlugin> validator_;
    std::shared_ptr<qtforge::IPlugin> transformer_;
    std::shared_ptr<qtforge::IPlugin> enricher_;
    std::shared_ptr<qtforge::IPlugin> persistence_;
    ProcessingPipeline pipeline_;
};
```

### 2. Strategy Pattern for Plugin Selection

Dynamically select plugins based on runtime conditions:

```cpp
class PluginStrategyManager {
public:
    template<typename TInterface>
    void registerStrategy(const std::string& strategyName, 
                         const std::string& pluginName,
                         std::function<bool(const Context&)> selector) {
        StrategyInfo info;
        info.pluginName = pluginName;
        info.selector = selector;
        
        strategies_[typeid(TInterface).name()][strategyName] = info;
    }
    
    template<typename TInterface>
    qtforge::expected<std::shared_ptr<TInterface>, qtforge::Error> 
    selectStrategy(const Context& context) {
        
        const std::string interfaceName = typeid(TInterface).name();
        auto interfaceIt = strategies_.find(interfaceName);
        
        if (interfaceIt == strategies_.end()) {
            return qtforge::Error("No strategies registered for interface: " + interfaceName);
        }
        
        // Find matching strategy
        for (const auto& [strategyName, info] : interfaceIt->second) {
            if (info.selector(context)) {
                auto& pluginManager = qtforge::PluginManager::instance();
                auto plugin = pluginManager.getPlugin(info.pluginName);
                
                if (plugin) {
                    auto typedPlugin = std::dynamic_pointer_cast<TInterface>(plugin);
                    if (typedPlugin) {
                        return typedPlugin;
                    }
                }
            }
        }
        
        return qtforge::Error("No suitable strategy found for context");
    }

private:
    struct StrategyInfo {
        std::string pluginName;
        std::function<bool(const Context&)> selector;
    };
    
    std::unordered_map<std::string, std::unordered_map<std::string, StrategyInfo>> strategies_;
};

// Usage example
class DataProcessorManager {
public:
    void setupStrategies() {
        strategyManager_.registerStrategy<IDataProcessor>("small_data", "FastDataProcessor",
            [](const Context& ctx) { return ctx.dataSize < 1000; });
        
        strategyManager_.registerStrategy<IDataProcessor>("large_data", "BatchDataProcessor",
            [](const Context& ctx) { return ctx.dataSize >= 1000 && ctx.dataSize < 100000; });
        
        strategyManager_.registerStrategy<IDataProcessor>("huge_data", "StreamingDataProcessor",
            [](const Context& ctx) { return ctx.dataSize >= 100000; });
    }
    
    qtforge::expected<ProcessingResult, qtforge::Error> processData(const RawData& data) {
        Context context;
        context.dataSize = data.size();
        context.dataType = data.type();
        context.priority = data.priority();
        
        auto processor = strategyManager_.selectStrategy<IDataProcessor>(context);
        if (!processor) {
            return processor.error();
        }
        
        return processor.value()->process(data);
    }

private:
    PluginStrategyManager strategyManager_;
};
```

### 3. Observer Pattern for Plugin Events

Implement sophisticated event handling and notification systems:

```cpp
template<typename EventType>
class PluginEventObserver {
public:
    virtual ~PluginEventObserver() = default;
    virtual void onEvent(const EventType& event) = 0;
    virtual bool shouldReceiveEvent(const EventType& event) const { return true; }
};

template<typename EventType>
class PluginEventSubject {
public:
    void addObserver(std::shared_ptr<PluginEventObserver<EventType>> observer) {
        std::lock_guard<std::mutex> lock(observersMutex_);
        observers_.push_back(observer);
    }
    
    void removeObserver(std::shared_ptr<PluginEventObserver<EventType>> observer) {
        std::lock_guard<std::mutex> lock(observersMutex_);
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [observer](const std::weak_ptr<PluginEventObserver<EventType>>& weak) {
                    return weak.expired() || weak.lock() == observer;
                }),
            observers_.end());
    }
    
    void notifyObservers(const EventType& event) {
        std::vector<std::shared_ptr<PluginEventObserver<EventType>>> validObservers;
        
        {
            std::lock_guard<std::mutex> lock(observersMutex_);
            for (auto it = observers_.begin(); it != observers_.end();) {
                if (auto observer = it->lock()) {
                    if (observer->shouldReceiveEvent(event)) {
                        validObservers.push_back(observer);
                    }
                    ++it;
                } else {
                    it = observers_.erase(it); // Remove expired observers
                }
            }
        }
        
        // Notify observers outside of lock to avoid deadlocks
        for (auto& observer : validObservers) {
            try {
                observer->onEvent(event);
            } catch (const std::exception& e) {
                qtforge::Logger::error("EventSubject", 
                    "Observer notification failed: " + std::string(e.what()));
            }
        }
    }

private:
    std::vector<std::weak_ptr<PluginEventObserver<EventType>>> observers_;
    mutable std::mutex observersMutex_;
};

// Usage example
class PluginLifecycleEvent {
public:
    enum class Type { Loading, Loaded, Initializing, Initialized, Activating, Activated, 
                     Deactivating, Deactivated, Unloading, Unloaded, Error };
    
    Type type;
    std::string pluginName;
    std::string pluginVersion;
    std::chrono::system_clock::time_point timestamp;
    std::optional<qtforge::Error> error;
};

class PluginLifecycleMonitor : public PluginEventObserver<PluginLifecycleEvent> {
public:
    void onEvent(const PluginLifecycleEvent& event) override {
        // Log lifecycle events
        std::string eventTypeStr = lifecycleEventTypeToString(event.type);
        qtforge::Logger::info("LifecycleMonitor", 
            "Plugin " + event.pluginName + " " + eventTypeStr);
        
        // Update metrics
        updateLifecycleMetrics(event);
        
        // Check for error conditions
        if (event.error) {
            handlePluginError(event);
        }
    }
    
    bool shouldReceiveEvent(const PluginLifecycleEvent& event) const override {
        // Only monitor specific plugins or event types if needed
        return monitoredPlugins_.empty() || 
               monitoredPlugins_.find(event.pluginName) != monitoredPlugins_.end();
    }

private:
    std::set<std::string> monitoredPlugins_;
    
    void updateLifecycleMetrics(const PluginLifecycleEvent& event) {
        // Update performance metrics
    }
    
    void handlePluginError(const PluginLifecycleEvent& event) {
        // Handle plugin errors
    }
    
    std::string lifecycleEventTypeToString(PluginLifecycleEvent::Type type) {
        // Convert enum to string
        return "Unknown";
    }
};
```

## Communication Patterns

### 1. Request-Response with Timeout and Retry

Implement robust request-response communication:

```cpp
template<typename TRequest, typename TResponse>
class RobustRequestClient {
public:
    struct RequestOptions {
        std::chrono::milliseconds timeout = std::chrono::seconds(30);
        int maxRetries = 3;
        std::chrono::milliseconds retryDelay = std::chrono::milliseconds(100);
        double backoffMultiplier = 2.0;
        std::function<bool(const qtforge::Error&)> shouldRetry = [](const qtforge::Error&) { return true; };
    };
    
    qtforge::expected<TResponse, qtforge::Error> request(
        const std::string& topic,
        const TRequest& request,
        const RequestOptions& options = {}) {
        
        qtforge::Error lastError("No attempts made");
        
        for (int attempt = 0; attempt < options.maxRetries; ++attempt) {
            if (attempt > 0) {
                // Apply backoff delay
                auto delay = std::chrono::milliseconds(
                    static_cast<long long>(options.retryDelay.count() * 
                                         std::pow(options.backoffMultiplier, attempt - 1)));
                std::this_thread::sleep_for(delay);
                
                qtforge::Logger::debug("RequestClient", 
                    "Retrying request (attempt " + std::to_string(attempt + 1) + ")");
            }
            
            auto result = performRequest(topic, request, options.timeout);
            
            if (result) {
                return result.value();
            }
            
            lastError = result.error();
            
            // Check if we should retry this error
            if (!options.shouldRetry(lastError)) {
                break;
            }
        }
        
        qtforge::Error finalError("Request failed after " + std::to_string(options.maxRetries) + " attempts");
        finalError.setCause(lastError);
        return finalError;
    }

private:
    qtforge::expected<TResponse, qtforge::Error> performRequest(
        const std::string& topic,
        const TRequest& request,
        std::chrono::milliseconds timeout) {
        
        auto& messageBus = qtforge::MessageBus::instance();
        
        try {
            return messageBus.request<TRequest, TResponse>(topic, request, timeout);
        } catch (const std::exception& e) {
            return qtforge::Error("Request exception: " + std::string(e.what()));
        }
    }
};
```

### 2. Circuit Breaker Pattern

Prevent cascading failures in plugin communication:

```cpp
template<typename TRequest, typename TResponse>
class CircuitBreakerClient {
public:
    enum class State { Closed, Open, HalfOpen };
    
    struct CircuitBreakerConfig {
        int failureThreshold = 5;
        std::chrono::seconds openTimeout = std::chrono::seconds(60);
        int halfOpenMaxCalls = 3;
        std::function<bool(const qtforge::Error&)> isFailure = [](const qtforge::Error&) { return true; };
    };
    
    CircuitBreakerClient(const std::string& serviceName, const CircuitBreakerConfig& config = {})
        : serviceName_(serviceName), config_(config), state_(State::Closed),
          failureCount_(0), lastFailureTime_(std::chrono::system_clock::time_point::min()),
          halfOpenCalls_(0) {}
    
    qtforge::expected<TResponse, qtforge::Error> request(
        const std::string& topic,
        const TRequest& request,
        std::chrono::milliseconds timeout = std::chrono::seconds(30)) {
        
        std::lock_guard<std::mutex> lock(stateMutex_);
        
        // Check circuit breaker state
        updateState();
        
        if (state_ == State::Open) {
            return qtforge::Error("Circuit breaker is OPEN for service: " + serviceName_);
        }
        
        if (state_ == State::HalfOpen && halfOpenCalls_ >= config_.halfOpenMaxCalls) {
            return qtforge::Error("Circuit breaker is HALF-OPEN and max calls exceeded for service: " + serviceName_);
        }
        
        // Perform request
        auto result = performRequest(topic, request, timeout);
        
        // Update circuit breaker based on result
        if (result) {
            onSuccess();
        } else {
            if (config_.isFailure(result.error())) {
                onFailure();
            }
        }
        
        return result;
    }
    
    State getState() const {
        std::lock_guard<std::mutex> lock(stateMutex_);
        return state_;
    }
    
    int getFailureCount() const {
        std::lock_guard<std::mutex> lock(stateMutex_);
        return failureCount_;
    }

private:
    std::string serviceName_;
    CircuitBreakerConfig config_;
    State state_;
    int failureCount_;
    std::chrono::system_clock::time_point lastFailureTime_;
    int halfOpenCalls_;
    mutable std::mutex stateMutex_;
    
    void updateState() {
        auto now = std::chrono::system_clock::now();
        
        switch (state_) {
            case State::Closed:
                // Stay closed
                break;
                
            case State::Open:
                if (now - lastFailureTime_ >= config_.openTimeout) {
                    state_ = State::HalfOpen;
                    halfOpenCalls_ = 0;
                    qtforge::Logger::info("CircuitBreaker", 
                        "Circuit breaker transitioning to HALF-OPEN for service: " + serviceName_);
                }
                break;
                
            case State::HalfOpen:
                // State will be updated based on request results
                break;
        }
    }
    
    void onSuccess() {
        if (state_ == State::HalfOpen) {
            halfOpenCalls_++;
            
            // If we've had enough successful calls, close the circuit
            if (halfOpenCalls_ >= config_.halfOpenMaxCalls) {
                state_ = State::Closed;
                failureCount_ = 0;
                qtforge::Logger::info("CircuitBreaker", 
                    "Circuit breaker transitioning to CLOSED for service: " + serviceName_);
            }
        } else if (state_ == State::Closed) {
            // Reset failure count on success
            failureCount_ = 0;
        }
    }
    
    void onFailure() {
        failureCount_++;
        lastFailureTime_ = std::chrono::system_clock::now();
        
        if (state_ == State::Closed && failureCount_ >= config_.failureThreshold) {
            state_ = State::Open;
            qtforge::Logger::warning("CircuitBreaker", 
                "Circuit breaker transitioning to OPEN for service: " + serviceName_);
        } else if (state_ == State::HalfOpen) {
            state_ = State::Open;
            qtforge::Logger::warning("CircuitBreaker", 
                "Circuit breaker transitioning back to OPEN for service: " + serviceName_);
        }
    }
    
    qtforge::expected<TResponse, qtforge::Error> performRequest(
        const std::string& topic,
        const TRequest& request,
        std::chrono::milliseconds timeout) {
        
        auto& messageBus = qtforge::MessageBus::instance();
        return messageBus.request<TRequest, TResponse>(topic, request, timeout);
    }
};
```

## Resource Management Patterns

### 1. Resource Pool Pattern

Manage expensive resources efficiently:

```cpp
template<typename TResource>
class ResourcePool {
public:
    using ResourceFactory = std::function<std::unique_ptr<TResource>()>;
    using ResourceValidator = std::function<bool(const TResource&)>;
    using ResourceResetter = std::function<void(TResource&)>;
    
    struct PoolConfig {
        size_t minSize = 1;
        size_t maxSize = 10;
        std::chrono::seconds idleTimeout = std::chrono::seconds(300);
        std::chrono::seconds validationInterval = std::chrono::seconds(60);
    };
    
    ResourcePool(ResourceFactory factory, const PoolConfig& config = {})
        : factory_(factory), config_(config), shutdown_(false) {
        
        // Pre-populate pool with minimum resources
        for (size_t i = 0; i < config_.minSize; ++i) {
            auto resource = factory_();
            if (resource) {
                ResourceInfo info;
                info.resource = std::move(resource);
                info.lastUsed = std::chrono::system_clock::now();
                info.inUse = false;
                
                availableResources_.push(std::move(info));
            }
        }
        
        // Start maintenance thread
        maintenanceThread_ = std::thread([this] { maintenanceLoop(); });
    }
    
    ~ResourcePool() {
        shutdown();
    }
    
    class ResourceHandle {
    public:
        ResourceHandle(ResourcePool* pool, std::unique_ptr<TResource> resource)
            : pool_(pool), resource_(std::move(resource)) {}
        
        ~ResourceHandle() {
            if (pool_ && resource_) {
                pool_->returnResource(std::move(resource_));
            }
        }
        
        // Move-only semantics
        ResourceHandle(const ResourceHandle&) = delete;
        ResourceHandle& operator=(const ResourceHandle&) = delete;
        
        ResourceHandle(ResourceHandle&& other) noexcept
            : pool_(other.pool_), resource_(std::move(other.resource_)) {
            other.pool_ = nullptr;
        }
        
        ResourceHandle& operator=(ResourceHandle&& other) noexcept {
            if (this != &other) {
                if (pool_ && resource_) {
                    pool_->returnResource(std::move(resource_));
                }
                pool_ = other.pool_;
                resource_ = std::move(other.resource_);
                other.pool_ = nullptr;
            }
            return *this;
        }
        
        TResource* get() const { return resource_.get(); }
        TResource& operator*() const { return *resource_; }
        TResource* operator->() const { return resource_.get(); }
        
    private:
        ResourcePool* pool_;
        std::unique_ptr<TResource> resource_;
    };
    
    qtforge::expected<ResourceHandle, qtforge::Error> acquire(
        std::chrono::milliseconds timeout = std::chrono::seconds(30)) {
        
        std::unique_lock<std::mutex> lock(poolMutex_);
        
        // Wait for available resource
        bool acquired = resourceAvailable_.wait_for(lock, timeout, [this] {
            return !availableResources_.empty() || shutdown_;
        });
        
        if (shutdown_) {
            return qtforge::Error("Resource pool is shutting down");
        }
        
        if (!acquired) {
            return qtforge::Error("Timeout waiting for resource");
        }
        
        // Get resource from pool
        ResourceInfo info = std::move(availableResources_.front());
        availableResources_.pop();
        
        // Validate resource if validator is set
        if (validator_ && !validator_(*info.resource)) {
            // Resource is invalid, create a new one
            lock.unlock();
            auto newResource = factory_();
            if (!newResource) {
                return qtforge::Error("Failed to create new resource");
            }
            return ResourceHandle(this, std::move(newResource));
        }
        
        // Reset resource if resetter is set
        if (resetter_) {
            resetter_(*info.resource);
        }
        
        return ResourceHandle(this, std::move(info.resource));
    }
    
    void setValidator(ResourceValidator validator) {
        validator_ = validator;
    }
    
    void setResetter(ResourceResetter resetter) {
        resetter_ = resetter;
    }
    
    size_t getAvailableCount() const {
        std::lock_guard<std::mutex> lock(poolMutex_);
        return availableResources_.size();
    }
    
    size_t getTotalCount() const {
        std::lock_guard<std::mutex> lock(poolMutex_);
        return availableResources_.size() + inUseCount_;
    }

private:
    struct ResourceInfo {
        std::unique_ptr<TResource> resource;
        std::chrono::system_clock::time_point lastUsed;
        bool inUse;
    };
    
    ResourceFactory factory_;
    PoolConfig config_;
    ResourceValidator validator_;
    ResourceResetter resetter_;
    
    mutable std::mutex poolMutex_;
    std::queue<ResourceInfo> availableResources_;
    std::condition_variable resourceAvailable_;
    size_t inUseCount_ = 0;
    
    std::atomic<bool> shutdown_;
    std::thread maintenanceThread_;
    
    void returnResource(std::unique_ptr<TResource> resource) {
        std::lock_guard<std::mutex> lock(poolMutex_);
        
        if (shutdown_) {
            return;
        }
        
        ResourceInfo info;
        info.resource = std::move(resource);
        info.lastUsed = std::chrono::system_clock::now();
        info.inUse = false;
        
        availableResources_.push(std::move(info));
        inUseCount_--;
        
        resourceAvailable_.notify_one();
    }
    
    void maintenanceLoop() {
        while (!shutdown_) {
            std::this_thread::sleep_for(config_.validationInterval);
            
            if (shutdown_) break;
            
            performMaintenance();
        }
    }
    
    void performMaintenance() {
        std::lock_guard<std::mutex> lock(poolMutex_);
        
        auto now = std::chrono::system_clock::now();
        std::queue<ResourceInfo> validResources;
        
        // Remove idle and invalid resources
        while (!availableResources_.empty()) {
            ResourceInfo info = std::move(availableResources_.front());
            availableResources_.pop();
            
            // Check if resource has been idle too long
            if (now - info.lastUsed > config_.idleTimeout && 
                validResources.size() >= config_.minSize) {
                continue; // Remove idle resource
            }
            
            // Validate resource
            if (validator_ && !validator_(*info.resource)) {
                continue; // Remove invalid resource
            }
            
            validResources.push(std::move(info));
        }
        
        availableResources_ = std::move(validResources);
        
        // Ensure minimum pool size
        while (availableResources_.size() < config_.minSize) {
            auto resource = factory_();
            if (resource) {
                ResourceInfo info;
                info.resource = std::move(resource);
                info.lastUsed = now;
                info.inUse = false;
                
                availableResources_.push(std::move(info));
            } else {
                break; // Failed to create resource
            }
        }
    }
    
    void shutdown() {
        shutdown_ = true;
        resourceAvailable_.notify_all();
        
        if (maintenanceThread_.joinable()) {
            maintenanceThread_.join();
        }
    }
};
```

## Error Handling Patterns

### 1. Error Recovery Strategies

Implement sophisticated error recovery mechanisms:

```cpp
class ErrorRecoveryManager {
public:
    enum class RecoveryStrategy {
        Retry,
        Fallback,
        CircuitBreaker,
        Graceful Degradation,
        Fail Fast
    };
    
    struct RecoveryConfig {
        RecoveryStrategy strategy = RecoveryStrategy::Retry;
        int maxRetries = 3;
        std::chrono::milliseconds retryDelay = std::chrono::milliseconds(100);
        double backoffMultiplier = 2.0;
        std::function<qtforge::expected<void, qtforge::Error>()> fallbackAction;
        std::function<bool(const qtforge::Error&)> shouldRecover = [](const qtforge::Error&) { return true; };
    };
    
    template<typename F>
    auto executeWithRecovery(F&& operation, const RecoveryConfig& config = {}) 
        -> qtforge::expected<std::invoke_result_t<F>, qtforge::Error> {
        
        using ReturnType = std::invoke_result_t<F>;
        
        switch (config.strategy) {
            case RecoveryStrategy::Retry:
                return executeWithRetry(std::forward<F>(operation), config);
                
            case RecoveryStrategy::Fallback:
                return executeWithFallback(std::forward<F>(operation), config);
                
            case RecoveryStrategy::CircuitBreaker:
                return executeWithCircuitBreaker(std::forward<F>(operation), config);
                
            case RecoveryStrategy::GracefulDegradation:
                return executeWithGracefulDegradation(std::forward<F>(operation), config);
                
            case RecoveryStrategy::FailFast:
                return executeFailFast(std::forward<F>(operation));
                
            default:
                return qtforge::Error("Unknown recovery strategy");
        }
    }

private:
    template<typename F>
    auto executeWithRetry(F&& operation, const RecoveryConfig& config) 
        -> qtforge::expected<std::invoke_result_t<F>, qtforge::Error> {
        
        qtforge::Error lastError("No attempts made");
        
        for (int attempt = 0; attempt < config.maxRetries; ++attempt) {
            if (attempt > 0) {
                auto delay = std::chrono::milliseconds(
                    static_cast<long long>(config.retryDelay.count() * 
                                         std::pow(config.backoffMultiplier, attempt - 1)));
                std::this_thread::sleep_for(delay);
            }
            
            try {
                if constexpr (std::is_same_v<std::invoke_result_t<F>, void>) {
                    operation();
                    return {};
                } else {
                    return operation();
                }
            } catch (const std::exception& e) {
                lastError = qtforge::Error("Operation failed: " + std::string(e.what()));
                
                if (!config.shouldRecover(lastError)) {
                    break;
                }
            }
        }
        
        qtforge::Error finalError("Operation failed after " + std::to_string(config.maxRetries) + " attempts");
        finalError.setCause(lastError);
        return finalError;
    }
    
    template<typename F>
    auto executeWithFallback(F&& operation, const RecoveryConfig& config) 
        -> qtforge::expected<std::invoke_result_t<F>, qtforge::Error> {
        
        try {
            if constexpr (std::is_same_v<std::invoke_result_t<F>, void>) {
                operation();
                return {};
            } else {
                return operation();
            }
        } catch (const std::exception& e) {
            qtforge::Error error("Primary operation failed: " + std::string(e.what()));
            
            if (config.shouldRecover(error) && config.fallbackAction) {
                auto fallbackResult = config.fallbackAction();
                if (fallbackResult) {
                    qtforge::Logger::warning("ErrorRecovery", "Using fallback after primary failure");
                    if constexpr (std::is_same_v<std::invoke_result_t<F>, void>) {
                        return {};
                    } else {
                        // Return default value or handle fallback result
                        return std::invoke_result_t<F>{};
                    }
                } else {
                    error.setCause(fallbackResult.error());
                }
            }
            
            return error;
        }
    }
};
```

## See Also

- **[Plugin Development Guide](plugin-development.md)**: Basic plugin development
- **[Best Practices](best-practices.md)**: Development best practices
- **[System Design](../architecture/system-design.md)**: System architecture patterns
- **[Performance Optimization](../user-guide/performance-optimization.md)**: Performance patterns
