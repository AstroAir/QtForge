# PluginOrchestrator API Reference

!!! info "Module Information"
**Header**: `qtplugin/orchestration/plugin_orchestrator.hpp`  
 **Namespace**: `qtplugin::orchestration`  
 **Since**: QtForge v3.1.0  
 **Status**: Beta

## Overview

The PluginOrchestrator provides a powerful workflow management system for coordinating complex multi-plugin operations. It enables the creation and execution of sophisticated workflows with dependency management, parallel execution, rollback capabilities, and transaction support.

### Key Features

- **Workflow Management**: Create, register, and execute complex workflows
- **Multiple Execution Modes**: Sequential, parallel, conditional, and pipeline execution
- **Dependency Resolution**: Automatic step dependency ordering and validation
- **Transaction Support**: Atomic workflow execution with rollback capabilities
- **Async Execution**: Non-blocking workflow execution with progress monitoring
- **Error Handling**: Comprehensive error handling with retry mechanisms

### Use Cases

- **Multi-Plugin Coordination**: Orchestrate operations across multiple plugins
- **Data Processing Pipelines**: Create complex data transformation workflows
- **System Integration**: Coordinate system-wide operations and configurations
- **Batch Processing**: Execute large-scale batch operations with error recovery

## Quick Start

```cpp
#include <qtplugin/orchestration/plugin_orchestrator.hpp>

using namespace qtplugin::orchestration;

// Create orchestrator
auto orchestrator = std::make_shared<PluginOrchestrator>();

// Create workflow
Workflow workflow("data_processing", "Data Processing Pipeline");
workflow.set_description("Process and transform data through multiple plugins")
        .set_execution_mode(ExecutionMode::Sequential);

// Add workflow steps
WorkflowStep step1("load_data", "data_loader", "load");
step1.parameters = QJsonObject{{"source", "input.csv"}};

WorkflowStep step2("transform", "data_transformer", "transform");
step2.dependencies = {"load_data"};
step2.parameters = QJsonObject{{"operation", "normalize"}};

workflow.add_step(step1).add_step(step2);

// Register and execute
auto result = orchestrator->register_workflow(workflow);
if (result) {
    auto execution_id = orchestrator->execute_workflow("data_processing");
    if (execution_id) {
        qDebug() << "Workflow started:" << execution_id.value();
    }
}
```

## Enumerations

### StepStatus

```cpp
enum class StepStatus {
    Pending,    ///< Step is waiting to be executed
    Running,    ///< Step is currently executing
    Completed,  ///< Step completed successfully
    Failed,     ///< Step failed with error
    Skipped,    ///< Step was skipped due to conditions
    Cancelled,  ///< Step was cancelled
    Retrying    ///< Step is being retried after failure
};
```

### ExecutionMode

```cpp
enum class ExecutionMode {
    Sequential,   ///< Execute steps one by one
    Parallel,     ///< Execute independent steps in parallel
    Conditional,  ///< Execute based on conditions
    Pipeline      ///< Execute as a pipeline with data flow
};
```

## Data Structures

### WorkflowStep

```cpp
struct WorkflowStep {
    QString id;                         ///< Unique step identifier
    QString name;                       ///< Human-readable name
    QString description;                ///< Step description
    QString plugin_id;                  ///< Plugin that executes this step
    QString service_name;               ///< Service to call (if applicable)
    QString method_name;                ///< Method to call
    QJsonObject parameters;             ///< Step parameters
    std::vector<QString> dependencies;  ///< Step dependencies (other step IDs)
    std::chrono::milliseconds timeout{60000};  ///< Step timeout
    int max_retries{0};                 ///< Maximum retry attempts
    std::chrono::milliseconds retry_delay{1000}; ///< Delay between retries
    bool critical{true};                ///< Whether failure should stop workflow
    QJsonObject metadata;               ///< Additional metadata

    WorkflowStep() = default;
    WorkflowStep(const QString& step_id, const QString& plugin, const QString& method);
};
```

### StepResult

```cpp
struct StepResult {
    QString step_id;                    ///< Step identifier
    StepStatus status;                  ///< Execution status
    QJsonObject result_data;            ///< Step result data
    QString error_message;              ///< Error message (if failed)
    int retry_count{0};                 ///< Number of retries attempted
    std::chrono::system_clock::time_point start_time;  ///< Execution start time
    std::chrono::system_clock::time_point end_time;    ///< Execution end time

    std::chrono::milliseconds execution_time() const;
};
```

## Class Reference

### Workflow

Workflow definition and configuration class.

#### Constructor

```cpp
Workflow() = default;
Workflow(const QString& workflow_id, const QString& name = "");
```

#### Configuration Methods

##### `set_description()`

```cpp
Workflow& set_description(const QString& desc);
```

Sets the workflow description.

**Parameters:**

- `desc` - Workflow description

**Returns:**

- `Workflow&` - Reference to this workflow for method chaining

##### `set_execution_mode()`

```cpp
Workflow& set_execution_mode(ExecutionMode mode);
```

Sets the workflow execution mode.

**Parameters:**

- `mode` - Execution mode (Sequential, Parallel, Conditional, Pipeline)

**Returns:**

- `Workflow&` - Reference to this workflow for method chaining

##### `add_step()`

```cpp
Workflow& add_step(const WorkflowStep& step);
```

Adds a step to the workflow.

**Parameters:**

- `step` - Workflow step to add

**Returns:**

- `Workflow&` - Reference to this workflow for method chaining

**Example:**

```cpp
WorkflowStep step("process_data", "data_processor", "process");
step.parameters = QJsonObject{{"mode", "batch"}};
workflow.add_step(step);
```

##### `add_rollback_step()`

```cpp
Workflow& add_rollback_step(const QString& step_id, const WorkflowStep& rollback_step);
```

Adds a rollback step for error recovery.

**Parameters:**

- `step_id` - ID of the step this rollback is for
- `rollback_step` - Rollback step definition

**Returns:**

- `Workflow&` - Reference to this workflow for method chaining

#### Query Methods

##### `id()`

```cpp
QString id() const;
```

Returns the workflow identifier.

##### `name()`

```cpp
QString name() const;
```

Returns the workflow name.

##### `execution_mode()`

```cpp
ExecutionMode execution_mode() const;
```

Returns the workflow execution mode.

#### Static Methods

##### `from_json()`

```cpp
static qtplugin::expected<Workflow, PluginError> from_json(const QJsonObject& json);
```

Creates a workflow from JSON definition.

**Parameters:**

- `json` - JSON workflow definition

**Returns:**

- `expected<Workflow, PluginError>` - Workflow instance or error

### PluginOrchestrator

Main orchestrator class for workflow management and execution.

#### Constructor

```cpp
explicit PluginOrchestrator(QObject* parent = nullptr);
```

#### Static Methods

##### `create()`

```cpp
static std::shared_ptr<PluginOrchestrator> create();
```

Creates a new orchestrator instance.

**Returns:**

- `std::shared_ptr<PluginOrchestrator>` - Shared pointer to new instance

#### Workflow Management

##### `register_workflow()`

```cpp
qtplugin::expected<void, PluginError> register_workflow(const Workflow& workflow);
```

Registers a workflow for execution.

**Parameters:**

- `workflow` - Workflow to register

**Returns:**

- `expected<void, PluginError>` - Success or error

**Errors:**

- `PluginErrorCode::InvalidConfiguration` - Invalid workflow definition
- `PluginErrorCode::AlreadyExists` - Workflow ID already registered

##### `unregister_workflow()`

```cpp
qtplugin::expected<void, PluginError> unregister_workflow(const QString& workflow_id);
```

Unregisters a workflow.

**Parameters:**

- `workflow_id` - Workflow identifier

**Returns:**

- `expected<void, PluginError>` - Success or error

##### `get_workflow()`

```cpp
qtplugin::expected<Workflow, PluginError> get_workflow(const QString& workflow_id) const;
```

Retrieves a registered workflow.

**Parameters:**

- `workflow_id` - Workflow identifier

**Returns:**

- `expected<Workflow, PluginError>` - Workflow instance or error

##### `list_workflows()`

```cpp
std::vector<QString> list_workflows() const;
```

Lists all registered workflow IDs.

**Returns:**

- `std::vector<QString>` - List of workflow IDs

#### Workflow Execution

##### `execute_workflow()`

```cpp
qtplugin::expected<QString, PluginError> execute_workflow(
    const QString& workflow_id,
    const QJsonObject& initial_data = {},
    bool async = false);
```

Executes a registered workflow.

**Parameters:**

- `workflow_id` - Workflow to execute
- `initial_data` - Initial data for the workflow
- `async` - Whether to execute asynchronously

**Returns:**

- `expected<QString, PluginError>` - Execution ID or error

**Example:**

```cpp
QJsonObject input_data{{"source_file", "data.csv"}};
auto execution_id = orchestrator->execute_workflow("data_pipeline", input_data);
if (execution_id) {
    qDebug() << "Execution started:" << execution_id.value();
}
```

##### `execute_workflow_async()`

```cpp
std::future<qtplugin::expected<QJsonObject, PluginError>> execute_workflow_async(
    const QString& workflow_id,
    const QJsonObject& initial_data = {});
```

Executes a workflow asynchronously with future result.

**Parameters:**

- `workflow_id` - Workflow to execute
- `initial_data` - Initial data for the workflow

**Returns:**

- `std::future<expected<QJsonObject, PluginError>>` - Future result

##### `cancel_workflow()`

```cpp
qtplugin::expected<void, PluginError> cancel_workflow(const QString& execution_id);
```

Cancels a running workflow execution.

**Parameters:**

- `execution_id` - Execution to cancel

**Returns:**

- `expected<void, PluginError>` - Success or error

#### Execution Monitoring

##### `get_execution_status()`

```cpp
qtplugin::expected<QJsonObject, PluginError> get_execution_status(const QString& execution_id) const;
```

Gets the current status of a workflow execution.

**Parameters:**

- `execution_id` - Execution identifier

**Returns:**

- `expected<QJsonObject, PluginError>` - Status information or error

**Status Object Fields:**

- `execution_id` - Execution identifier
- `workflow_id` - Workflow identifier
- `status` - Overall execution status
- `progress` - Execution progress (0.0 to 1.0)
- `current_step` - Currently executing step
- `completed_steps` - Number of completed steps
- `total_steps` - Total number of steps

##### `list_active_executions()`

```cpp
std::vector<QString> list_active_executions() const;
```

Lists all active execution IDs.

**Returns:**

- `std::vector<QString>` - List of active execution IDs

##### `get_step_results()`

```cpp
qtplugin::expected<std::vector<StepResult>, PluginError> get_step_results(const QString& execution_id) const;
```

Gets results for all steps in an execution.

**Parameters:**

- `execution_id` - Execution identifier

**Returns:**

- `expected<std::vector<StepResult>, PluginError>` - Step results or error

#### Transaction Support

##### `begin_transaction()`

```cpp
qtplugin::expected<void, PluginError> begin_transaction(const QString& transaction_id);
```

Begins a transaction for atomic workflow execution.

##### `commit_transaction()`

```cpp
qtplugin::expected<void, PluginError> commit_transaction(const QString& transaction_id);
```

Commits a transaction, making all changes permanent.

##### `rollback_transaction()`

```cpp
qtplugin::expected<void, PluginError> rollback_transaction(const QString& transaction_id);
```

Rolls back a transaction, undoing all changes.

## Error Handling

Common error codes and their meanings:

| Error Code               | Description                          | Resolution                                              |
| ------------------------ | ------------------------------------ | ------------------------------------------------------- |
| `InvalidConfiguration`   | Invalid workflow definition          | Check workflow structure and step definitions           |
| `NotFound`               | Workflow or execution not found      | Verify workflow is registered and execution ID is valid |
| `InvalidState`           | Operation not valid in current state | Check execution status before operation                 |
| `PluginNotFound`         | Required plugin not available        | Ensure all referenced plugins are loaded                |
| `DependencyNotSatisfied` | Step dependencies not met            | Check step dependency configuration                     |
| `Timeout`                | Step or workflow execution timeout   | Increase timeout values or optimize step execution      |

## Thread Safety

- **Thread-safe methods**: All public methods are thread-safe
- **Concurrent executions**: Multiple workflows can execute concurrently
- **Synchronization**: Internal synchronization ensures data consistency

## Performance Considerations

- **Memory usage**: Approximately 1-2KB per workflow step
- **CPU usage**: Minimal overhead for orchestration logic
- **Scalability**: Supports hundreds of concurrent workflow executions
- **Best practices**: Use appropriate execution modes for optimal performance

## Integration Examples

### Basic Workflow Creation

```cpp
#include <qtplugin/orchestration/plugin_orchestrator.hpp>

class DataProcessingWorkflow {
private:
    std::shared_ptr<PluginOrchestrator> m_orchestrator;

public:
    bool initialize() {
        m_orchestrator = PluginOrchestrator::create();

        // Create data processing workflow
        Workflow workflow("data_processing", "Data Processing Pipeline");
        workflow.set_description("Complete data processing pipeline")
                .set_execution_mode(ExecutionMode::Sequential);

        // Add steps
        WorkflowStep load_step("load", "data_loader", "load_csv");
        load_step.parameters = QJsonObject{{"file_path", "/data/input.csv"}};

        WorkflowStep validate_step("validate", "data_validator", "validate");
        validate_step.dependencies = {"load"};

        WorkflowStep transform_step("transform", "data_transformer", "normalize");
        transform_step.dependencies = {"validate"};

        WorkflowStep save_step("save", "data_saver", "save_csv");
        save_step.dependencies = {"transform"};
        save_step.parameters = QJsonObject{{"output_path", "/data/output.csv"}};

        workflow.add_step(load_step)
                .add_step(validate_step)
                .add_step(transform_step)
                .add_step(save_step);

        return m_orchestrator->register_workflow(workflow).has_value();
    }

    void process_data() {
        auto execution_id = m_orchestrator->execute_workflow("data_processing");
        if (execution_id) {
            monitor_execution(execution_id.value());
        }
    }

private:
    void monitor_execution(const QString& execution_id) {
        // Monitor execution progress
        while (true) {
            auto status = m_orchestrator->get_execution_status(execution_id);
            if (status) {
                auto status_obj = status.value();
                qDebug() << "Progress:" << status_obj["progress"].toDouble();

                if (status_obj["status"].toString() == "completed") {
                    qDebug() << "Workflow completed successfully";
                    break;
                } else if (status_obj["status"].toString() == "failed") {
                    qDebug() << "Workflow failed";
                    break;
                }
            }

            QThread::msleep(1000); // Check every second
        }
    }
};
```

### Parallel Execution Workflow

```cpp
// Create parallel workflow for independent operations
Workflow parallel_workflow("parallel_processing", "Parallel Data Processing");
parallel_workflow.set_execution_mode(ExecutionMode::Parallel);

// These steps can run in parallel
WorkflowStep process_images("process_images", "image_processor", "batch_process");
WorkflowStep process_text("process_text", "text_processor", "analyze");
WorkflowStep process_audio("process_audio", "audio_processor", "extract_features");

// Final step depends on all parallel steps
WorkflowStep combine("combine", "data_combiner", "merge_results");
combine.dependencies = {"process_images", "process_text", "process_audio"};

parallel_workflow.add_step(process_images)
                 .add_step(process_text)
                 .add_step(process_audio)
                 .add_step(combine);
```

## Python Bindings

!!! note "Python Support"
This component is available in Python through the `qtforge.orchestration` module.

```python
import qtforge

# Create orchestrator
orchestrator = qtforge.orchestration.PluginOrchestrator()

# Create workflow
workflow = qtforge.orchestration.Workflow("data_pipeline", "Data Pipeline")
workflow.set_execution_mode(qtforge.orchestration.ExecutionMode.Sequential)

# Add steps
step1 = qtforge.orchestration.WorkflowStep("load", "data_loader", "load")
step1.parameters = {"source": "input.csv"}

step2 = qtforge.orchestration.WorkflowStep("process", "processor", "transform")
step2.dependencies = ["load"]

workflow.add_step(step1).add_step(step2)

# Register and execute
orchestrator.register_workflow(workflow)
execution_id = orchestrator.execute_workflow("data_pipeline")
```

## Related Components

- **[PluginManager](../core/plugin-manager.md)**: Core plugin management
- **[MessageBus](../communication/message-bus.md)**: Inter-plugin communication
- **[TransactionManager](../transactions/transaction-manager.md)**: Transaction support
- **[AdvancedOrchestrator](advanced-orchestrator.md)**: Enhanced orchestration features

## Migration Notes

### From v3.0 to v3.1

- **New Features**: Transaction support, async execution
- **API Changes**: None (backward compatible)
- **Deprecated**: None

## See Also

- [Orchestration User Guide](../../user-guide/workflow-orchestration.md)
- [Orchestration Examples](../../examples/orchestration-examples.md)
- [Architecture Overview](../../architecture/system-design.md)

---

_Last updated: December 2024 | QtForge v3.1.0_
