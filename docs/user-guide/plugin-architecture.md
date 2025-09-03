# Plugin Architecture

This guide explores the architectural patterns and design principles that make QtForge plugins robust, scalable, and maintainable.

## Architectural Overview

QtForge follows a layered architecture that promotes separation of concerns and modularity:

```
┌─────────────────────────────────────────┐
│           Application Layer             │
├─────────────────────────────────────────┤
│           Plugin Layer                  │
├─────────────────────────────────────────┤
│           Framework Layer               │
├─────────────────────────────────────────┤
│           Qt Foundation Layer           │
└─────────────────────────────────────────┘
```

### Layer Responsibilities

**Application Layer**
- Application-specific logic
- User interface components
- Business rules and workflows

**Plugin Layer**
- Plugin implementations
- Domain-specific functionality
- Extension points

**Framework Layer**
- Plugin management
- Communication infrastructure
- Security and validation
- Resource management

**Qt Foundation Layer**
- Core Qt functionality
- Event system
- Threading support
- Platform abstraction

## Core Architectural Patterns

### Plugin Pattern

The fundamental pattern that enables runtime extensibility:

```cpp
// Plugin interface defines the contract
class IDataProcessor : public IPlugin {
public:
    virtual expected<Data, Error> process(const Data& input) = 0;
    virtual std::vector<std::string> supportedFormats() const = 0;
};

// Concrete implementations provide specific functionality
class JsonProcessor : public IDataProcessor {
public:
    expected<Data, Error> process(const Data& input) override {
        // JSON-specific processing
        return processJson(input);
    }
    
    std::vector<std::string> supportedFormats() const override {
        return {"json", "application/json"};
    }
};
```

### Factory Pattern

Plugin creation is managed through factory functions:

```cpp
// Plugin factory interface
class IPluginFactory {
public:
    virtual ~IPluginFactory() = default;
    virtual std::unique_ptr<IPlugin> createPlugin() = 0;
    virtual std::string pluginType() const = 0;
    virtual bool canCreate(const std::string& type) const = 0;
};

// Concrete factory implementation
class DataProcessorFactory : public IPluginFactory {
public:
    std::unique_ptr<IPlugin> createPlugin() override {
        return std::make_unique<JsonProcessor>();
    }
    
    std::string pluginType() const override {
        return "data-processor";
    }
    
    bool canCreate(const std::string& type) const override {
        return type == "json-processor";
    }
};
```

### Observer Pattern

Event-driven communication between plugins:

```cpp
class IEventObserver {
public:
    virtual ~IEventObserver() = default;
    virtual void onEvent(const Event& event) = 0;
    virtual std::vector<std::string> interestedEvents() const = 0;
};

class DataProcessorPlugin : public IPlugin, public IEventObserver {
public:
    void onEvent(const Event& event) override {
        if (event.type() == "data.received") {
            auto dataEvent = static_cast<const DataEvent&>(event);
            processData(dataEvent.data());
        }
    }
    
    std::vector<std::string> interestedEvents() const override {
        return {"data.received", "system.shutdown"};
    }
};
```

### Strategy Pattern

Pluggable algorithms and behaviors:

```cpp
class ICompressionStrategy {
public:
    virtual ~ICompressionStrategy() = default;
    virtual expected<std::vector<uint8_t>, Error> compress(const std::vector<uint8_t>& data) = 0;
    virtual expected<std::vector<uint8_t>, Error> decompress(const std::vector<uint8_t>& data) = 0;
    virtual std::string algorithmName() const = 0;
};

class CompressionPlugin : public IPlugin {
private:
    std::unique_ptr<ICompressionStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<ICompressionStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    expected<std::vector<uint8_t>, Error> compressData(const std::vector<uint8_t>& data) {
        if (!strategy_) {
            return Error("No compression strategy set");
        }
        return strategy_->compress(data);
    }
};
```

## Plugin Communication Patterns

### Message Bus Pattern

Decoupled communication through a central message bus:

```cpp
class MessageBus {
public:
    template<typename T>
    void publish(const std::string& topic, const T& message) {
        auto subscribers = getSubscribers(topic);
        for (auto& subscriber : subscribers) {
            subscriber->handleMessage(topic, message);
        }
    }
    
    void subscribe(const std::string& topic, IMessageHandler* handler) {
        subscribers_[topic].push_back(handler);
    }
    
private:
    std::unordered_map<std::string, std::vector<IMessageHandler*>> subscribers_;
};

// Usage in plugins
class PublisherPlugin : public IPlugin {
public:
    void publishStatus(const std::string& status) {
        StatusMessage msg{name(), status, std::chrono::system_clock::now()};
        messageBus().publish("plugin.status", msg);
    }
};

class MonitorPlugin : public IPlugin, public IMessageHandler {
public:
    expected<void, Error> initialize() override {
        messageBus().subscribe("plugin.status", this);
        return {};
    }
    
    void handleMessage(const std::string& topic, const Message& message) override {
        if (topic == "plugin.status") {
            auto statusMsg = static_cast<const StatusMessage&>(message);
            logStatus(statusMsg);
        }
    }
};
```

### Service Locator Pattern

Plugin discovery and dependency injection:

```cpp
class ServiceLocator {
public:
    template<typename T>
    void registerService(const std::string& name, std::shared_ptr<T> service) {
        services_[name] = service;
    }
    
    template<typename T>
    std::shared_ptr<T> getService(const std::string& name) {
        auto it = services_.find(name);
        if (it != services_.end()) {
            return std::dynamic_pointer_cast<T>(it->second);
        }
        return nullptr;
    }
    
private:
    std::unordered_map<std::string, std::shared_ptr<void>> services_;
};

// Plugin using services
class ConsumerPlugin : public IPlugin {
public:
    expected<void, Error> initialize() override {
        databaseService_ = serviceLocator().getService<IDatabaseService>("database");
        if (!databaseService_) {
            return Error("Database service not available");
        }
        return {};
    }
    
private:
    std::shared_ptr<IDatabaseService> databaseService_;
};
```

## Plugin Lifecycle Management

### State Machine Pattern

Plugin state management through a well-defined state machine:

```cpp
enum class PluginState {
    Unloaded,
    Loaded,
    Initialized,
    Active,
    Inactive,
    Error,
    Unloading
};

class PluginStateMachine {
public:
    expected<void, Error> transition(PluginState newState) {
        if (!isValidTransition(currentState_, newState)) {
            return Error("Invalid state transition");
        }
        
        auto result = executeTransition(currentState_, newState);
        if (result) {
            currentState_ = newState;
        }
        return result;
    }
    
private:
    PluginState currentState_ = PluginState::Unloaded;
    
    bool isValidTransition(PluginState from, PluginState to) {
        // Define valid state transitions
        static const std::map<PluginState, std::vector<PluginState>> validTransitions = {
            {PluginState::Unloaded, {PluginState::Loaded, PluginState::Error}},
            {PluginState::Loaded, {PluginState::Initialized, PluginState::Error, PluginState::Unloading}},
            {PluginState::Initialized, {PluginState::Active, PluginState::Error, PluginState::Unloading}},
            {PluginState::Active, {PluginState::Inactive, PluginState::Error, PluginState::Unloading}},
            {PluginState::Inactive, {PluginState::Active, PluginState::Error, PluginState::Unloading}},
            {PluginState::Error, {PluginState::Unloading}},
            {PluginState::Unloading, {PluginState::Unloaded}}
        };
        
        auto it = validTransitions.find(from);
        if (it != validTransitions.end()) {
            return std::find(it->second.begin(), it->second.end(), to) != it->second.end();
        }
        return false;
    }
};
```

### Dependency Injection Pattern

Automatic dependency resolution and injection:

```cpp
class DependencyInjector {
public:
    template<typename T>
    void registerDependency(const std::string& name, std::shared_ptr<T> dependency) {
        dependencies_[name] = dependency;
    }
    
    expected<void, Error> injectDependencies(IPlugin* plugin) {
        auto dependencies = plugin->dependencies();
        for (const auto& dep : dependencies) {
            auto dependency = findDependency(dep);
            if (!dependency) {
                return Error("Dependency not found: " + dep);
            }
            plugin->injectDependency(dep, dependency);
        }
        return {};
    }
    
private:
    std::unordered_map<std::string, std::shared_ptr<void>> dependencies_;
};
```

## Security Architecture

### Sandbox Pattern

Plugin isolation and security:

```cpp
class PluginSandbox {
public:
    expected<void, Error> executeInSandbox(IPlugin* plugin, std::function<void()> operation) {
        // Set up security context
        SecurityContext context;
        context.setPermissions(plugin->requiredPermissions());
        context.setResourceLimits(plugin->resourceLimits());
        
        // Execute operation in sandbox
        try {
            SandboxGuard guard(context);
            operation();
            return {};
        } catch (const SecurityException& e) {
            return Error("Security violation: " + std::string(e.what()));
        }
    }
    
private:
    class SandboxGuard {
    public:
        SandboxGuard(const SecurityContext& context) : context_(context) {
            // Apply security restrictions
            applySecurityContext(context_);
        }
        
        ~SandboxGuard() {
            // Restore previous security context
            restoreSecurityContext();
        }
        
    private:
        SecurityContext context_;
    };
};
```

### Permission System

Fine-grained permission control:

```cpp
enum class Permission {
    FileRead,
    FileWrite,
    NetworkAccess,
    SystemCall,
    PluginCommunication
};

class PermissionManager {
public:
    bool hasPermission(const IPlugin* plugin, Permission permission) {
        auto permissions = getPluginPermissions(plugin);
        return permissions.find(permission) != permissions.end();
    }
    
    expected<void, Error> checkPermission(const IPlugin* plugin, Permission permission) {
        if (!hasPermission(plugin, permission)) {
            return Error("Permission denied: " + permissionToString(permission));
        }
        return {};
    }
    
private:
    std::unordered_map<const IPlugin*, std::set<Permission>> pluginPermissions_;
};
```

## Performance Patterns

### Lazy Loading Pattern

On-demand resource loading:

```cpp
class LazyResource {
public:
    const Resource& get() {
        if (!resource_) {
            resource_ = loadResource();
        }
        return *resource_;
    }
    
private:
    mutable std::optional<Resource> resource_;
    
    Resource loadResource() const {
        // Expensive resource loading
        return Resource{};
    }
};
```

### Object Pool Pattern

Reusable object management:

```cpp
template<typename T>
class ObjectPool {
public:
    std::unique_ptr<T, std::function<void(T*)>> acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (pool_.empty()) {
            return std::unique_ptr<T, std::function<void(T*)>>(
                new T(), [this](T* obj) { release(obj); });
        }
        
        auto obj = std::move(pool_.back());
        pool_.pop_back();
        return std::unique_ptr<T, std::function<void(T*)>>(
            obj.release(), [this](T* obj) { release(obj); });
    }
    
private:
    std::vector<std::unique_ptr<T>> pool_;
    std::mutex mutex_;
    
    void release(T* obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.emplace_back(obj);
    }
};
```

## Best Practices

### Architectural Guidelines

1. **Separation of Concerns**: Each plugin should have a single, well-defined responsibility
2. **Loose Coupling**: Minimize dependencies between plugins
3. **High Cohesion**: Related functionality should be grouped together
4. **Interface Stability**: Keep plugin interfaces stable across versions
5. **Error Handling**: Design for failure and recovery

### Design Principles

1. **SOLID Principles**: Follow SOLID design principles
2. **DRY (Don't Repeat Yourself)**: Avoid code duplication
3. **KISS (Keep It Simple, Stupid)**: Prefer simple solutions
4. **YAGNI (You Aren't Gonna Need It)**: Don't over-engineer
5. **Composition over Inheritance**: Prefer composition to inheritance

### Performance Considerations

1. **Lazy Initialization**: Load resources only when needed
2. **Efficient Communication**: Use appropriate message types and patterns
3. **Memory Management**: Use smart pointers and RAII
4. **Threading**: Design for concurrent execution
5. **Caching**: Cache expensive computations and resources

## Next Steps

- **[Plugin Development Guide](plugin-development.md)**: Learn to implement plugins
- **[Advanced Plugin Development](advanced-plugin-development.md)**: Advanced techniques
- **[API Reference](../api/index.md)**: Detailed API documentation
- **[Examples](../examples/index.md)**: Study architectural examples
