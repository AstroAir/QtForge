# QtForge Communication Architecture

## Overview

The QtForge communication subsystem provides inter-plugin communication through multiple patterns:

- **Message Bus**: Publish/subscribe messaging
- **Event System**: Typed event distribution
- **Request/Response**: Synchronous and asynchronous service calls
- **Service Contracts**: Formal service definitions
- **Service Discovery**: Network-based service location

## Architecture Patterns

### 1. Factory Pattern (Dependency Injection)

The communication subsystem can be instantiated through a factory pattern for dependency injection:

```cpp
#include <qtplugin/communication/factory.hpp>

// Create with default configuration
auto system = qtplugin::communication::create_default_communication_system();

// Or use builder for custom configuration
auto factory = std::make_shared<DefaultCommunicationFactory>();
auto system = CommunicationSystemBuilder(factory)
    .with_config(custom_config)
    .build();
```

**When to use**:

- Building testable systems with dependency injection
- Need to swap implementations for testing
- Require centralized configuration

**Current Limitations**:

- EventSystemImpl is a non-functional stub
- MessageRouter cannot deliver messages (interface limitation)
- Shutdown logic not implemented

### 2. Direct Instantiation (Recommended)

For most use cases, direct instantiation of specific components is recommended:

```cpp
#include <qtplugin/communication/message_bus.hpp>
#include <qtplugin/communication/typed_event_system.hpp>
#include <qtplugin/communication/request_response_system.hpp>

// Message Bus
auto message_bus = std::make_unique<qtplugin::MessageBus>();

// Typed Event System (RECOMMENDED for events)
auto event_system = std::make_unique<qtplugin::TypedEventSystem>();

// Request/Response System
auto rr_system = std::make_unique<qtplugin::RequestResponseSystem>();
```

**When to use**:

- Production code
- Need full functionality
- Simpler, more direct approach

## Component Relationships

```
┌─────────────────────────────────────────────────────────────┐
│                    Communication Layer                       │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ MessageBus   │  │TypedEvent    │  │Request/      │      │
│  │              │  │System        │  │Response      │      │
│  │ (Pub/Sub)    │  │ (Events)     │  │System        │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│         │                  │                  │              │
│         └──────────────────┴──────────────────┘              │
│                            │                                 │
│                  ┌─────────▼─────────┐                      │
│                  │  Service          │                      │
│                  │  Contracts        │                      │
│                  └─────────┬─────────┘                      │
│                            │                                 │
│                  ┌─────────▼─────────┐                      │
│                  │  Service          │                      │
│                  │  Discovery        │                      │
│                  └───────────────────┘                      │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

## Event Systems: Which to Use?

### TypedEventSystem (RECOMMENDED)

**Use for**: All event-based communication

```cpp
#include <qtplugin/communication/typed_event_system.hpp>

auto event_system = std::make_unique<qtplugin::TypedEventSystem>();

// Publish typed event
struct MyEvent {
    QString data;
    int value;
};

event_system->publish("my_plugin", MyEvent{"test", 42});

// Subscribe to typed events
event_system->subscribe<MyEvent>(
    "subscriber_id",
    [](const qtplugin::TypedEvent<MyEvent>& event) {
        qDebug() << "Received:" << event.data().data;
    }
);
```

**Features**:

- ✅ Type-safe event handling
- ✅ Qt meta-object integration
- ✅ Event filtering and routing
- ✅ Event history and replay
- ✅ Multiple delivery modes (immediate, queued, deferred, batched)
- ✅ Full implementation

### IEventSystem (Factory Interface)

**Status**: ⚠️ **DO NOT USE** - Current implementation is a non-functional stub

The factory's `IEventSystem` interface is currently implemented by `EventSystemImpl`, which is a placeholder that doesn't actually route events or manage subscriptions.

**Future**: This will either be:

- Removed entirely
- Replaced with an adapter to TypedEventSystem
- Properly implemented

## Message Bus vs Event System

### Use MessageBus when:

- Need publish/subscribe messaging
- Working with heterogeneous message types
- Require message filtering and routing
- Need delivery mode control (broadcast, unicast, multicast)

### Use TypedEventSystem when:

- Need type-safe event handling
- Want Qt signal/slot integration
- Require event history/replay
- Need batched or deferred delivery

## Service Contracts

Service contracts provide formal service definitions with type safety and validation:

```cpp
#include <qtplugin/communication/plugin_service_contracts.hpp>

using namespace qtplugin::contracts;

// Define service contract
ServiceContract contract("com.example.myservice", ServiceVersion(1, 0, 0));
contract.set_description("My service")
    .set_provider("my_plugin")
    .set_capabilities(ServiceCapability::Synchronous | ServiceCapability::ThreadSafe);

// Add method
ServiceMethod method("process", "Process data");
method.add_parameter(ServiceParameter("input", "string", "Input data", true))
      .set_return_type(ServiceParameter("result", "object", "Result"));

contract.add_method(method);

// Register with registry
auto& registry = ServiceContractRegistry::instance();
registry.register_contract("my_plugin", contract);
```

## Service Discovery

Network-based service discovery (requires `QTFORGE_HAS_NETWORK`):

```cpp
#include <qtplugin/communication/plugin_service_discovery.hpp>

auto discovery = std::make_unique<qtplugin::PluginServiceDiscovery>();

// Register service
ServiceRegistration registration;
registration.service_id = "my_service";
registration.service_name = "My Service";
registration.plugin_id = "my_plugin";
registration.availability = ServiceAvailability::Available;

discovery->register_service(registration);

// Discover services
ServiceDiscoveryQuery query;
query.service_name = "My Service";

auto result = discovery->discover_services(query);
```

## Thread Safety

All communication components are thread-safe:

- **MessageBus**: Uses `std::shared_mutex` for subscriptions
- **TypedEventSystem**: Uses `QMutex` for event queues and subscriptions
- **RequestResponseSystem**: Uses `QMutex` for service registry and pending requests
- **SubscriptionManager**: Uses `QReadWriteLock` for subscription indices

## Error Handling

All components use the `expected<T, E>` pattern for error handling:

```cpp
auto result = message_bus->publish(message);
if (!result.has_value()) {
    auto error = result.error();
    qWarning() << "Publish failed:" << error.message;
}
```

Error types:

- `CommunicationError` - For communication operations
- `PluginError` - For plugin-level errors

## Performance Considerations

### Message Bus

- Asynchronous delivery for >5 subscribers
- Thread pool with configurable max threads (default: 10)
- Message queue with configurable size (default: 10,000)

### TypedEventSystem

- Separate queues for different delivery modes
- Configurable processing intervals
- Event history with size limits

### Request/Response

- Configurable default timeout (default: 30s)
- Automatic request cleanup
- Concurrent request limits

## Migration Guide

### From request_response.hpp (Deprecated)

**Old**:

```cpp
#include <qtplugin/communication/request_response.hpp>
```

**New**:

```cpp
#include <qtplugin/communication/request_response_system.hpp>
```

The functionality is identical; only the header name changed.

## Best Practices

1. **Use TypedEventSystem for events** - Don't use the factory's IEventSystem
2. **Direct instantiation** - Prefer direct instantiation over factory pattern
3. **Error handling** - Always check `expected<>` results
4. **Thread safety** - All components are thread-safe, but handlers should be too
5. **Resource cleanup** - Use RAII and smart pointers
6. **Service contracts** - Define contracts for formal service interfaces
7. **Testing** - Mock interfaces for unit testing

## Known Issues

1. **MessageRouter** - Cannot deliver messages (interface limitation)
2. **EventSystemImpl** - Non-functional stub implementation
3. **CommunicationSystem::shutdown()** - Not implemented
4. **Factory pattern** - Limited functionality due to above issues

**Recommendation**: Use direct instantiation until factory issues are resolved.

## Future Roadmap

- Fix MessageRouter delivery mechanism
- Replace EventSystemImpl with TypedEventSystem adapter
- Implement proper shutdown logic
- Add comprehensive integration tests
- Performance benchmarks
- Network service discovery tests
