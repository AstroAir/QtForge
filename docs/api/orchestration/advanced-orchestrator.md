# AdvancedPluginOrchestrator API Reference

!!! info "Module Information"
**Header**: `qtplugin/orchestration/advanced/plugin_orchestrator_v2.hpp`  
 **Namespace**: `qtplugin`  
 **Since**: QtForge v3.2.0  
 **Status**: Beta

## Overview

The AdvancedPluginOrchestrator (v2) provides enhanced workflow management capabilities with visual workflow design, advanced execution modes, and plugin composition integration. It extends the basic orchestrator with graph-based workflow definitions and streaming execution support.

### Key Features

- **Visual Workflow Design**: Graph-based workflow definition with nodes and connections
- **Advanced Execution Modes**: Sequential, parallel, conditional, event-driven, and streaming
- **Plugin Composition Integration**: Seamless integration with composite plugins
- **Real-time Monitoring**: Live workflow execution monitoring and visualization
- **Dynamic Workflow Modification**: Runtime workflow updates and modifications
- **Event-Driven Execution**: Reactive workflow execution based on system events

### Use Cases

- **Complex System Integration**: Orchestrate sophisticated multi-system workflows
- **Real-time Data Processing**: Stream processing with dynamic workflow adaptation
- **Visual Workflow Design**: Create workflows through graphical interfaces
- **Event-Driven Automation**: Reactive system automation based on triggers

## Quick Start

```cpp
#include <qtplugin/orchestration/advanced/plugin_orchestrator_v2.hpp>

// Create advanced orchestrator
auto orchestrator = std::make_shared<AdvancedPluginOrchestrator>();

// Create workflow with nodes and connections
PluginWorkflow workflow;
workflow.workflow_id = "advanced_pipeline";
workflow.name = "Advanced Data Pipeline";
workflow.execution_mode = WorkflowExecutionMode::Streaming;

// Add workflow nodes
WorkflowNode input_node;
input_node.node_id = "input";
input_node.plugin_id = "data_source";
input_node.node_type = WorkflowNodeType::Source;

WorkflowNode process_node;
process_node.node_id = "process";
process_node.plugin_id = "data_processor";
process_node.node_type = WorkflowNodeType::Processor;

WorkflowNode output_node;
output_node.node_id = "output";
output_node.plugin_id = "data_sink";
output_node.node_type = WorkflowNodeType::Sink;

workflow.nodes = {input_node, process_node, output_node};

// Add connections
WorkflowConnection conn1;
conn1.source_node = "input";
conn1.target_node = "process";
conn1.connection_type = ConnectionType::DataFlow;

WorkflowConnection conn2;
conn2.source_node = "process";
conn2.target_node = "output";
conn2.connection_type = ConnectionType::DataFlow;

workflow.connections = {conn1, conn2};

// Register and execute
auto result = orchestrator->register_workflow(workflow);
if (result) {
    auto execution_id = orchestrator->execute_workflow("advanced_pipeline");
}
```

## Enumerations

### WorkflowExecutionMode

```cpp
enum class WorkflowExecutionMode {
    Sequential,     ///< Execute plugins sequentially
    Parallel,       ///< Execute plugins in parallel
    Conditional,    ///< Execute based on conditions
    EventDriven,    ///< Execute based on events
    Streaming       ///< Execute in streaming mode
};
```

### WorkflowNodeType

```cpp
enum class WorkflowNodeType {
    Source,         ///< Data source node
    Processor,      ///< Data processing node
    Sink,           ///< Data sink node
    Decision,       ///< Decision/branching node
    Aggregator,     ///< Data aggregation node
    Splitter,       ///< Data splitting node
    Custom          ///< Custom node type
};
```

### ConnectionType

```cpp
enum class ConnectionType {
    DataFlow,       ///< Data flow connection
    Control,        ///< Control flow connection
    Event,          ///< Event-based connection
    Dependency      ///< Dependency connection
};
```

### WorkflowExecutionStatus

```cpp
enum class WorkflowExecutionStatus {
    Pending,        ///< Workflow is pending execution
    Running,        ///< Workflow is currently running
    Paused,         ///< Workflow execution is paused
    Completed,      ///< Workflow completed successfully
    Failed,         ///< Workflow execution failed
    Cancelled       ///< Workflow was cancelled
};
```

## Data Structures

### WorkflowNode

```cpp
struct WorkflowNode {
    QString node_id;                    ///< Unique node identifier
    QString name;                       ///< Human-readable name
    QString description;                ///< Node description
    QString plugin_id;                  ///< Associated plugin ID
    WorkflowNodeType node_type;         ///< Type of node
    QJsonObject configuration;          ///< Node configuration
    QJsonObject ui_properties;          ///< UI positioning and properties
    std::vector<QString> input_ports;   ///< Input port names
    std::vector<QString> output_ports;  ///< Output port names

    QJsonObject to_json() const;
    static WorkflowNode from_json(const QJsonObject& json);
};
```

### WorkflowConnection

```cpp
struct WorkflowConnection {
    QString connection_id;              ///< Unique connection identifier
    QString source_node;                ///< Source node ID
    QString source_port;                ///< Source port name
    QString target_node;                ///< Target node ID
    QString target_port;                ///< Target port name
    ConnectionType connection_type;     ///< Type of connection
    QJsonObject properties;             ///< Connection properties

    QJsonObject to_json() const;
    static WorkflowConnection from_json(const QJsonObject& json);
};
```

### PluginWorkflow

```cpp
struct PluginWorkflow {
    QString workflow_id;                        ///< Unique workflow identifier
    QString name;                               ///< Workflow name
    QString description;                        ///< Workflow description
    WorkflowExecutionMode execution_mode;       ///< Execution mode
    std::vector<WorkflowNode> nodes;            ///< Workflow nodes
    std::vector<WorkflowConnection> connections; ///< Node connections
    QJsonObject global_configuration;          ///< Global workflow configuration
    QJsonObject metadata;                       ///< Additional metadata

    QJsonObject to_json() const;
    static PluginWorkflow from_json(const QJsonObject& json);
};
```

### WorkflowExecutionContext

```cpp
struct WorkflowExecutionContext {
    QString execution_id;               ///< Execution identifier
    QString workflow_id;                ///< Workflow identifier
    WorkflowExecutionStatus status;     ///< Current execution status
    std::chrono::system_clock::time_point start_time;  ///< Execution start time
    std::chrono::system_clock::time_point end_time;    ///< Execution end time
    QJsonObject execution_data;         ///< Execution data and state
    std::unordered_map<QString, QJsonObject> node_states; ///< Individual node states
    double progress{0.0};               ///< Execution progress (0.0 to 1.0)
    QString current_node;               ///< Currently executing node
    std::vector<QString> error_messages; ///< Error messages
};
```

## Class Reference

### AdvancedPluginOrchestrator

Enhanced orchestrator with advanced workflow capabilities.

#### Constructor

```cpp
explicit AdvancedPluginOrchestrator(QObject* parent = nullptr);
```

#### Workflow Management

##### `register_workflow()`

```cpp
qtplugin::expected<void, PluginError> register_workflow(const PluginWorkflow& workflow);
```

Registers an advanced workflow for execution.

**Parameters:**

- `workflow` - Advanced workflow definition

**Returns:**

- `expected<void, PluginError>` - Success or error

**Example:**

```cpp
PluginWorkflow workflow;
workflow.workflow_id = "streaming_pipeline";
workflow.execution_mode = WorkflowExecutionMode::Streaming;

// Add nodes and connections...

auto result = orchestrator->register_workflow(workflow);
if (!result) {
    qDebug() << "Registration failed:" << result.error().message();
}
```

##### `unregister_workflow()`

```cpp
void unregister_workflow(const QString& workflow_id);
```

Unregisters a workflow.

##### `get_workflow()`

```cpp
qtplugin::expected<PluginWorkflow, PluginError> get_workflow(const QString& workflow_id);
```

Retrieves a registered workflow definition.

##### `get_registered_workflows()`

```cpp
std::vector<QString> get_registered_workflows() const;
```

Lists all registered workflow IDs.

#### Workflow Execution

##### `execute_workflow()`

```cpp
qtplugin::expected<QString, PluginError> execute_workflow(
    const QString& workflow_id,
    const QJsonObject& input_data = {});
```

Executes a registered workflow.

**Parameters:**

- `workflow_id` - Workflow to execute
- `input_data` - Initial input data

**Returns:**

- `expected<QString, PluginError>` - Execution ID or error

##### `cancel_execution()`

```cpp
void cancel_execution(const QString& execution_id);
```

Cancels a running workflow execution.

##### `pause_execution()`

```cpp
qtplugin::expected<void, PluginError> pause_execution(const QString& execution_id);
```

Pauses a running workflow execution.

##### `resume_execution()`

```cpp
qtplugin::expected<void, PluginError> resume_execution(const QString& execution_id);
```

Resumes a paused workflow execution.

#### Execution Monitoring

##### `get_execution_status()`

```cpp
qtplugin::expected<WorkflowExecutionContext, PluginError> get_execution_status(
    const QString& execution_id);
```

Gets detailed execution status and context.

**Parameters:**

- `execution_id` - Execution identifier

**Returns:**

- `expected<WorkflowExecutionContext, PluginError>` - Execution context or error

##### `get_active_executions()`

```cpp
std::vector<QString> get_active_executions() const;
```

Lists all active execution IDs.

##### `get_node_status()`

```cpp
qtplugin::expected<QJsonObject, PluginError> get_node_status(
    const QString& execution_id,
    const QString& node_id);
```

Gets the status of a specific node in an execution.

#### Dynamic Workflow Modification

##### `add_node_to_workflow()`

```cpp
qtplugin::expected<void, PluginError> add_node_to_workflow(
    const QString& workflow_id,
    const WorkflowNode& node);
```

Dynamically adds a node to an existing workflow.

##### `remove_node_from_workflow()`

```cpp
qtplugin::expected<void, PluginError> remove_node_from_workflow(
    const QString& workflow_id,
    const QString& node_id);
```

Removes a node from a workflow.

##### `add_connection_to_workflow()`

```cpp
qtplugin::expected<void, PluginError> add_connection_to_workflow(
    const QString& workflow_id,
    const WorkflowConnection& connection);
```

Adds a connection between nodes in a workflow.

## Signals

The AdvancedPluginOrchestrator emits the following Qt signals:

```cpp
signals:
    void workflow_started(const QString& execution_id, const QString& workflow_id);
    void workflow_completed(const QString& execution_id, const QJsonObject& results);
    void workflow_failed(const QString& execution_id, const QString& error_message);
    void workflow_paused(const QString& execution_id);
    void workflow_resumed(const QString& execution_id);
    void node_started(const QString& execution_id, const QString& node_id);
    void node_completed(const QString& execution_id, const QString& node_id, const QJsonObject& result);
    void node_failed(const QString& execution_id, const QString& node_id, const QString& error);
    void execution_progress_updated(const QString& execution_id, double progress);
```

## Error Handling

Common error codes specific to advanced orchestration:

| Error Code                  | Description                          | Resolution                        |
| --------------------------- | ------------------------------------ | --------------------------------- |
| `InvalidWorkflowStructure`  | Invalid node or connection structure | Verify workflow graph is valid    |
| `CircularDependency`        | Circular dependency in workflow      | Check node connections for cycles |
| `NodeNotFound`              | Referenced node not found            | Ensure all referenced nodes exist |
| `InvalidConnection`         | Invalid connection between nodes     | Check port compatibility          |
| `ExecutionModeNotSupported` | Execution mode not supported         | Use supported execution modes     |

## Thread Safety

- **Thread-safe methods**: All public methods are thread-safe
- **Signal emissions**: Signals are emitted from the main thread
- **Concurrent workflows**: Multiple workflows can execute concurrently
- **Node execution**: Individual nodes may execute in separate threads

## Integration Examples

### Streaming Data Pipeline

```cpp
#include <qtplugin/orchestration/advanced/plugin_orchestrator_v2.hpp>

class StreamingPipeline {
private:
    std::shared_ptr<AdvancedPluginOrchestrator> m_orchestrator;

public:
    bool setup_streaming_workflow() {
        m_orchestrator = std::make_shared<AdvancedPluginOrchestrator>();

        // Connect to orchestrator signals
        connect(m_orchestrator.get(), &AdvancedPluginOrchestrator::node_completed,
                this, &StreamingPipeline::on_node_completed);

        // Create streaming workflow
        PluginWorkflow workflow;
        workflow.workflow_id = "real_time_processing";
        workflow.name = "Real-time Data Processing";
        workflow.execution_mode = WorkflowExecutionMode::Streaming;

        // Data source node
        WorkflowNode source;
        source.node_id = "data_source";
        source.plugin_id = "kafka_consumer";
        source.node_type = WorkflowNodeType::Source;
        source.output_ports = {"raw_data"};

        // Processing nodes
        WorkflowNode filter;
        filter.node_id = "filter";
        filter.plugin_id = "data_filter";
        filter.node_type = WorkflowNodeType::Processor;
        filter.input_ports = {"input"};
        filter.output_ports = {"filtered_data"};

        WorkflowNode transform;
        transform.node_id = "transform";
        transform.plugin_id = "data_transformer";
        transform.node_type = WorkflowNodeType::Processor;
        transform.input_ports = {"input"};
        transform.output_ports = {"transformed_data"};

        // Sink node
        WorkflowNode sink;
        sink.node_id = "output";
        sink.plugin_id = "database_writer";
        sink.node_type = WorkflowNodeType::Sink;
        sink.input_ports = {"data"};

        workflow.nodes = {source, filter, transform, sink};

        // Create connections
        WorkflowConnection conn1;
        conn1.source_node = "data_source";
        conn1.source_port = "raw_data";
        conn1.target_node = "filter";
        conn1.target_port = "input";
        conn1.connection_type = ConnectionType::DataFlow;

        WorkflowConnection conn2;
        conn2.source_node = "filter";
        conn2.source_port = "filtered_data";
        conn2.target_node = "transform";
        conn2.target_port = "input";
        conn2.connection_type = ConnectionType::DataFlow;

        WorkflowConnection conn3;
        conn3.source_node = "transform";
        conn3.source_port = "transformed_data";
        conn3.target_node = "output";
        conn3.target_port = "data";
        conn3.connection_type = ConnectionType::DataFlow;

        workflow.connections = {conn1, conn2, conn3};

        // Register workflow
        auto result = m_orchestrator->register_workflow(workflow);
        return result.has_value();
    }

    void start_processing() {
        auto execution_id = m_orchestrator->execute_workflow("real_time_processing");
        if (execution_id) {
            qDebug() << "Started streaming pipeline:" << execution_id.value();
        }
    }

private slots:
    void on_node_completed(const QString& execution_id, const QString& node_id,
                          const QJsonObject& result) {
        qDebug() << "Node completed:" << node_id << "in execution:" << execution_id;
        // Handle node completion for streaming pipeline
    }
};
```

### Event-Driven Workflow

```cpp
// Create event-driven workflow
PluginWorkflow event_workflow;
event_workflow.workflow_id = "event_driven_automation";
event_workflow.execution_mode = WorkflowExecutionMode::EventDriven;

// Event trigger node
WorkflowNode trigger;
trigger.node_id = "file_watcher";
trigger.plugin_id = "file_system_monitor";
trigger.node_type = WorkflowNodeType::Source;
trigger.configuration = QJsonObject{
    {"watch_path", "/data/incoming"},
    {"file_pattern", "*.csv"}
};

// Decision node
WorkflowNode decision;
decision.node_id = "file_validator";
decision.plugin_id = "file_validator";
decision.node_type = WorkflowNodeType::Decision;

// Processing branches
WorkflowNode process_valid;
process_valid.node_id = "process_valid_file";
process_valid.plugin_id = "csv_processor";
process_valid.node_type = WorkflowNodeType::Processor;

WorkflowNode handle_invalid;
handle_invalid.node_id = "handle_invalid_file";
handle_invalid.plugin_id = "error_handler";
handle_invalid.node_type = WorkflowNodeType::Processor;

event_workflow.nodes = {trigger, decision, process_valid, handle_invalid};

// Add conditional connections
WorkflowConnection valid_conn;
valid_conn.source_node = "file_validator";
valid_conn.target_node = "process_valid_file";
valid_conn.connection_type = ConnectionType::Control;
valid_conn.properties = QJsonObject{{"condition", "file_valid == true"}};

WorkflowConnection invalid_conn;
invalid_conn.source_node = "file_validator";
invalid_conn.target_node = "handle_invalid_file";
invalid_conn.connection_type = ConnectionType::Control;
invalid_conn.properties = QJsonObject{{"condition", "file_valid == false"}};

event_workflow.connections = {valid_conn, invalid_conn};
```

## Python Bindings

!!! note "Python Support"
Advanced orchestration features are available through the `qtforge.orchestration` module.

```python
import qtforge

# Create advanced orchestrator
orchestrator = qtforge.orchestration.AdvancedPluginOrchestrator()

# Create workflow with visual nodes
workflow = qtforge.orchestration.PluginWorkflow()
workflow.workflow_id = "python_pipeline"
workflow.execution_mode = qtforge.orchestration.WorkflowExecutionMode.Streaming

# Add nodes
source_node = qtforge.orchestration.WorkflowNode()
source_node.node_id = "source"
source_node.plugin_id = "data_source"
source_node.node_type = qtforge.orchestration.WorkflowNodeType.Source

workflow.nodes.append(source_node)

# Register and execute
orchestrator.register_workflow(workflow)
execution_id = orchestrator.execute_workflow("python_pipeline")
```

## Related Components

- **[PluginOrchestrator](plugin-orchestrator.md)**: Basic orchestration functionality
- **[CompositePlugin](../composition/composite-plugin.md)**: Plugin composition integration
- **[MessageBus](../communication/message-bus.md)**: Inter-node communication
- **[TransactionManager](../transactions/transaction-manager.md)**: Transaction support

## Migration Notes

### From Basic to Advanced Orchestrator

- **Workflow Definition**: Convert step-based workflows to node-based workflows
- **Execution Modes**: New streaming and event-driven modes available
- **Visual Design**: Enhanced support for graphical workflow design
- **Monitoring**: More detailed execution monitoring and visualization

## See Also

- [Advanced Orchestration User Guide](../../user-guide/advanced-orchestration.md)
- [Workflow Design Examples](../../examples/advanced-workflows.md)
- [Architecture Overview](../../architecture/system-design.md)

---

_Last updated: December 2024 | QtForge v3.2.0_
