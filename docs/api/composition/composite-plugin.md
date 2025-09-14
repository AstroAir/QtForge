# CompositePlugin API Reference

!!! info "Module Information"
**Header**: `qtplugin/composition/plugin_composition.hpp`  
 **Namespace**: `qtplugin::composition`  
 **Since**: QtForge v3.1.0  
 **Status**: Stable

## Overview

The CompositePlugin provides a powerful plugin composition system that allows combining multiple plugins into a single unified interface. It supports various composition strategies including aggregation, pipeline processing, facade patterns, and more advanced architectural patterns.

### Key Features

- **Multiple Composition Strategies**: Aggregation, Pipeline, Facade, Decorator, Proxy, Adapter, Bridge
- **Plugin Role Management**: Primary and secondary plugin roles with different responsibilities
- **Data Binding System**: Flexible binding between plugin inputs and outputs
- **Unified Interface**: Single interface to access multiple underlying plugins
- **Dynamic Configuration**: Runtime composition configuration and modification
- **Strategy-based Execution**: Different execution patterns based on composition strategy

### Use Cases

- **Plugin Aggregation**: Combine related plugins into a single logical unit
- **Data Processing Pipelines**: Chain plugins for sequential data processing
- **Facade Pattern**: Provide simplified interface to complex plugin subsystems
- **Plugin Enhancement**: Add functionality to existing plugins through decoration
- **Interface Adaptation**: Adapt plugins with incompatible interfaces

## Quick Start

```cpp
#include <qtplugin/composition/plugin_composition.hpp>

using namespace qtplugin::composition;

// Create a pipeline composition
PluginComposition composition("data_pipeline", "Data Processing Pipeline");
composition.set_description("Process data through multiple stages")
           .set_strategy(CompositionStrategy::Pipeline)
           .add_plugin("data_loader", PluginRole::Primary)
           .add_plugin("data_transformer", PluginRole::Secondary)
           .add_plugin("data_validator", PluginRole::Secondary);

// Add data flow bindings
CompositionBinding binding1("data_loader", "output", "data_transformer", "input");
CompositionBinding binding2("data_transformer", "output", "data_validator", "input");
composition.add_binding(binding1).add_binding(binding2);

// Create composite plugin
auto composite = std::make_shared<CompositePlugin>(composition);

// Initialize and use
auto init_result = composite->initialize();
if (init_result) {
    auto result = composite->execute_command("process", QJsonObject{{"file", "data.csv"}});
    if (result) {
        qDebug() << "Processing result:" << result.value();
    }
}
```

## Enumerations

### CompositionStrategy

```cpp
enum class CompositionStrategy {
    Aggregation,  ///< Simple aggregation of plugins
    Pipeline,     ///< Pipeline processing through plugins
    Facade,       ///< Facade pattern - single interface to multiple plugins
    Decorator,    ///< Decorator pattern - enhance plugin functionality
    Proxy,        ///< Proxy pattern - control access to plugins
    Adapter,      ///< Adapter pattern - adapt plugin interfaces
    Bridge        ///< Bridge pattern - separate abstraction from implementation
};
```

### PluginRole

```cpp
enum class PluginRole {
    Primary,      ///< Primary plugin (main functionality)
    Secondary,    ///< Secondary plugin (supporting functionality)
    Auxiliary     ///< Auxiliary plugin (optional functionality)
};
```

### BindingType

```cpp
enum class BindingType {
    DataFlow,     ///< Data flows from source to target
    EventFlow,    ///< Events flow from source to target
    Control,      ///< Control flow binding
    Dependency    ///< Dependency relationship
};
```

## Data Structures

### CompositionBinding

```cpp
struct CompositionBinding {
    QString source_plugin;      ///< Source plugin ID
    QString source_port;        ///< Source port/output name
    QString target_plugin;      ///< Target plugin ID
    QString target_port;        ///< Target port/input name
    BindingType type;           ///< Type of binding
    QJsonObject transformation; ///< Data transformation rules

    CompositionBinding() = default;
    CompositionBinding(const QString& src_plugin, const QString& src_port,
                      const QString& tgt_plugin, const QString& tgt_port,
                      BindingType binding_type = BindingType::DataFlow);

    QJsonObject to_json() const;
    static CompositionBinding from_json(const QJsonObject& json);
};
```

## Class: PluginComposition

Configuration class for defining plugin compositions.

### Constructor

```cpp
PluginComposition(const QString& composition_id, const QString& name = "");
```

Creates a new plugin composition definition.

**Parameters:**

- `composition_id` - Unique composition identifier
- `name` - Human-readable name (defaults to composition_id)

### Configuration Methods

#### `set_description()`

```cpp
PluginComposition& set_description(const QString& desc);
```

Sets the composition description.

**Returns:**

- `PluginComposition&` - Reference for method chaining

#### `set_strategy()`

```cpp
PluginComposition& set_strategy(CompositionStrategy strategy);
```

Sets the composition strategy.

**Parameters:**

- `strategy` - Composition strategy to use

**Returns:**

- `PluginComposition&` - Reference for method chaining

#### `add_plugin()`

```cpp
PluginComposition& add_plugin(const QString& plugin_id, PluginRole role = PluginRole::Secondary);
```

Adds a plugin to the composition.

**Parameters:**

- `plugin_id` - Plugin identifier
- `role` - Plugin role in the composition

**Returns:**

- `PluginComposition&` - Reference for method chaining

#### `set_primary_plugin()`

```cpp
PluginComposition& set_primary_plugin(const QString& plugin_id);
```

Sets the primary plugin for the composition.

**Parameters:**

- `plugin_id` - Primary plugin identifier

**Returns:**

- `PluginComposition&` - Reference for method chaining

#### `add_binding()`

```cpp
PluginComposition& add_binding(const CompositionBinding& binding);
```

Adds a binding between plugins.

**Parameters:**

- `binding` - Binding definition

**Returns:**

- `PluginComposition&` - Reference for method chaining

#### `set_configuration()`

```cpp
PluginComposition& set_configuration(const QJsonObject& config);
```

Sets global configuration for the composition.

**Parameters:**

- `config` - Configuration object

**Returns:**

- `PluginComposition&` - Reference for method chaining

### Query Methods

#### `id()`

```cpp
QString id() const;
```

Returns the composition identifier.

#### `name()`

```cpp
QString name() const;
```

Returns the composition name.

#### `strategy()`

```cpp
CompositionStrategy strategy() const;
```

Returns the composition strategy.

#### `plugins()`

```cpp
std::unordered_map<QString, PluginRole> plugins() const;
```

Returns all plugins and their roles.

#### `bindings()`

```cpp
std::vector<CompositionBinding> bindings() const;
```

Returns all composition bindings.

#### `primary_plugin_id()`

```cpp
QString primary_plugin_id() const;
```

Returns the primary plugin identifier.

### Serialization

#### `to_json()`

```cpp
QJsonObject to_json() const;
```

Serializes the composition to JSON.

#### `from_json()`

```cpp
static PluginComposition from_json(const QJsonObject& json);
```

Creates a composition from JSON definition.

## Class: CompositePlugin

Main composite plugin implementation that aggregates multiple plugins.

### Constructor

```cpp
explicit CompositePlugin(const PluginComposition& composition, QObject* parent = nullptr);
```

Creates a composite plugin from a composition definition.

**Parameters:**

- `composition` - Composition definition
- `parent` - Qt parent object

### IPlugin Implementation

#### `initialize()`

```cpp
qtplugin::expected<void, PluginError> initialize() override;
```

Initializes the composite plugin and all component plugins.

**Returns:**

- `expected<void, PluginError>` - Success or error

**Process:**

1. Loads all component plugins
2. Initializes component plugins in dependency order
3. Sets up data bindings between plugins
4. Calculates combined capabilities

#### `shutdown()`

```cpp
void shutdown() noexcept override;
```

Shuts down all component plugins and cleans up resources.

#### `execute_command()`

```cpp
qtplugin::expected<QJsonObject, PluginError> execute_command(
    std::string_view command,
    const QJsonObject& params = {}) override;
```

Executes a command using the configured composition strategy.

**Parameters:**

- `command` - Command to execute
- `params` - Command parameters

**Returns:**

- `expected<QJsonObject, PluginError>` - Command result or error

**Strategy Behavior:**

- **Aggregation**: Executes command on all plugins that support it
- **Pipeline**: Passes data through plugins sequentially
- **Facade**: Routes command to appropriate plugin based on configuration

#### `available_commands()`

```cpp
std::vector<std::string> available_commands() const override;
```

Returns all commands available from component plugins.

#### `metadata()`

```cpp
PluginMetadata metadata() const override;
```

Returns combined metadata from all component plugins.

### IAdvancedPlugin Implementation

#### `get_service_contracts()`

```cpp
std::vector<contracts::ServiceContract> get_service_contracts() const override;
```

Returns combined service contracts from all component plugins.

#### `call_service()`

```cpp
qtplugin::expected<QJsonObject, PluginError> call_service(
    const QString& service_name,
    const QString& method_name,
    const QJsonObject& parameters = {},
    std::chrono::milliseconds timeout = std::chrono::milliseconds{30000}) override;
```

Calls a service on the appropriate component plugin.

### Composition-Specific Methods

#### `get_component_plugin()`

```cpp
std::shared_ptr<IPlugin> get_component_plugin(const QString& plugin_id) const;
```

Gets a specific component plugin by ID.

#### `get_component_plugins()`

```cpp
std::unordered_map<QString, std::shared_ptr<IPlugin>> get_component_plugins() const;
```

Gets all component plugins.

#### `get_composition()`

```cpp
const PluginComposition& get_composition() const;
```

Gets the composition definition.

#### `update_binding()`

```cpp
qtplugin::expected<void, PluginError> update_binding(const CompositionBinding& binding);
```

Updates or adds a binding at runtime.

#### `remove_binding()`

```cpp
qtplugin::expected<void, PluginError> remove_binding(const QString& source_plugin,
                                                     const QString& target_plugin);
```

Removes a binding between plugins.

## Error Handling

Common error codes and their meanings:

| Error Code              | Description                        | Resolution                                     |
| ----------------------- | ---------------------------------- | ---------------------------------------------- |
| `PluginNotFound`        | Component plugin not found         | Verify plugin ID and ensure plugin is loaded   |
| `InvalidConfiguration`  | Invalid composition configuration  | Check composition definition and bindings      |
| `BindingFailed`         | Failed to establish plugin binding | Verify binding compatibility and plugin states |
| `CircularDependency`    | Circular dependency in composition | Review plugin dependencies and bindings        |
| `IncompatibleInterface` | Plugin interfaces incompatible     | Check plugin interface compatibility           |

## Thread Safety

- **Thread-safe methods**: All public methods are thread-safe
- **Component plugins**: Thread safety depends on individual component plugins
- **Binding execution**: Bindings are executed sequentially to avoid race conditions
- **State management**: Composite plugin state is synchronized

## Performance Considerations

- **Memory usage**: Overhead of ~1-2KB plus component plugin memory
- **CPU usage**: Minimal overhead for composition logic
- **Strategy impact**: Pipeline strategy may have higher latency than aggregation
- **Binding overhead**: Each binding adds small processing overhead

## Integration Examples

### Pipeline Composition

```cpp
#include <qtplugin/composition/plugin_composition.hpp>

class DataProcessingPipeline {
private:
    std::shared_ptr<CompositePlugin> m_pipeline;

public:
    bool create_pipeline() {
        // Create pipeline composition
        PluginComposition composition("data_pipeline", "Data Processing Pipeline");
        composition.set_description("Multi-stage data processing pipeline")
                   .set_strategy(CompositionStrategy::Pipeline);

        // Add plugins in processing order
        composition.add_plugin("csv_reader", PluginRole::Primary)
                   .add_plugin("data_cleaner", PluginRole::Secondary)
                   .add_plugin("data_transformer", PluginRole::Secondary)
                   .add_plugin("data_validator", PluginRole::Secondary)
                   .add_plugin("json_writer", PluginRole::Secondary);

        // Create pipeline bindings
        composition.add_binding(CompositionBinding("csv_reader", "data", "data_cleaner", "input"))
                   .add_binding(CompositionBinding("data_cleaner", "cleaned_data", "data_transformer", "input"))
                   .add_binding(CompositionBinding("data_transformer", "transformed_data", "data_validator", "input"))
                   .add_binding(CompositionBinding("data_validator", "validated_data", "json_writer", "input"));

        // Create composite plugin
        m_pipeline = std::make_shared<CompositePlugin>(composition);

        // Initialize pipeline
        auto init_result = m_pipeline->initialize();
        if (!init_result) {
            qWarning() << "Failed to initialize pipeline:" << init_result.error().message();
            return false;
        }

        qDebug() << "Data processing pipeline created successfully";
        return true;
    }

    QJsonObject process_file(const QString& input_file, const QString& output_file) {
        if (!m_pipeline || m_pipeline->state() != PluginState::Running) {
            return QJsonObject{{"error", "Pipeline not ready"}};
        }

        QJsonObject params{
            {"input_file", input_file},
            {"output_file", output_file}
        };

        auto result = m_pipeline->execute_command("process", params);
        if (result) {
            return result.value();
        } else {
            return QJsonObject{{"error", result.error().message()}};
        }
    }
};
```

### Facade Composition

```cpp
class MediaProcessingFacade {
private:
    std::shared_ptr<CompositePlugin> m_facade;

public:
    bool create_facade() {
        // Create facade composition
        PluginComposition composition("media_facade", "Media Processing Facade");
        composition.set_description("Unified interface for media processing")
                   .set_strategy(CompositionStrategy::Facade);

        // Add specialized plugins
        composition.add_plugin("image_processor", PluginRole::Secondary)
                   .add_plugin("video_processor", PluginRole::Secondary)
                   .add_plugin("audio_processor", PluginRole::Secondary)
                   .add_plugin("metadata_extractor", PluginRole::Auxiliary);

        // Configure routing rules
        QJsonObject routing_config{
            {"image_extensions", QJsonArray{"jpg", "png", "gif", "bmp"}},
            {"video_extensions", QJsonArray{"mp4", "avi", "mov", "mkv"}},
            {"audio_extensions", QJsonArray{"mp3", "wav", "flac", "aac"}}
        };
        composition.set_configuration(routing_config);

        m_facade = std::make_shared<CompositePlugin>(composition);
        return m_facade->initialize().has_value();
    }

    QJsonObject process_media(const QString& file_path) {
        // Facade automatically routes to appropriate processor based on file type
        return m_facade->execute_command("process", QJsonObject{{"file", file_path}}).value_or(QJsonObject{});
    }
};
```

### Aggregation Composition

```cpp
class SystemMonitoringComposite {
private:
    std::shared_ptr<CompositePlugin> m_monitor;

public:
    bool create_monitor() {
        PluginComposition composition("system_monitor", "System Monitoring");
        composition.set_description("Aggregate system monitoring capabilities")
                   .set_strategy(CompositionStrategy::Aggregation);

        // Add monitoring plugins
        composition.add_plugin("cpu_monitor", PluginRole::Secondary)
                   .add_plugin("memory_monitor", PluginRole::Secondary)
                   .add_plugin("disk_monitor", PluginRole::Secondary)
                   .add_plugin("network_monitor", PluginRole::Secondary);

        m_monitor = std::make_shared<CompositePlugin>(composition);
        return m_monitor->initialize().has_value();
    }

    QJsonObject get_system_status() {
        // Aggregation strategy collects results from all monitoring plugins
        auto result = m_monitor->execute_command("get_status");
        return result.value_or(QJsonObject{});
    }
};
```

## Python Bindings

!!! note "Python Support"
This component is available in Python through the `qtforge.composition` module.

```python
import qtforge

# Create composition
composition = qtforge.composition.PluginComposition("my_pipeline", "My Pipeline")
composition.set_strategy(qtforge.composition.CompositionStrategy.Pipeline)

# Add plugins
composition.add_plugin("plugin1", qtforge.composition.PluginRole.Primary)
composition.add_plugin("plugin2", qtforge.composition.PluginRole.Secondary)

# Add binding
binding = qtforge.composition.CompositionBinding("plugin1", "output", "plugin2", "input")
composition.add_binding(binding)

# Create composite plugin
composite = qtforge.composition.CompositePlugin(composition)
result = composite.initialize()

if result:
    # Execute command
    output = composite.execute_command("process", {"data": "input"})
    print(f"Result: {output}")

# Helper function for pipeline creation
pipeline = qtforge.composition.create_pipeline_composition("data_pipeline", ["reader", "processor", "writer"])
```

## Related Components

- **[PluginManager](../core/plugin-manager.md)**: Core plugin management for component loading
- **[PluginOrchestrator](../orchestration/plugin-orchestrator.md)**: Workflow integration with compositions
- **[MessageBus](../communication/message-bus.md)**: Inter-plugin communication within compositions
- **[ServiceContracts](../contracts/service-contracts.md)**: Service contract aggregation

## Migration Notes

### From v3.0 to v3.1

- **New Features**: Bridge and Adapter composition strategies, runtime binding updates
- **API Changes**: None (backward compatible)
- **Performance**: Improved binding execution efficiency

## See Also

- [Plugin Composition User Guide](../../user-guide/plugin-composition.md)
- [Composition Patterns Examples](../../examples/composition-examples.md)
- [Architecture Patterns](../../architecture/composition-patterns.md)
- [Best Practices](../../developer-guide/composition-best-practices.md)

---

_Last updated: December 2024 | QtForge v3.1.0_
