# PluginOrchestrator Python API

!!! info "Python Module"
    **Module**: `qtforge.orchestration`  
    **Class**: `PluginOrchestrator`  
    **C++ Equivalent**: `qtplugin::orchestration::PluginOrchestrator`  
    **Since**: QtForge v3.1.0

## Overview

The PluginOrchestrator provides workflow management capabilities in Python, allowing you to create, register, and execute complex multi-plugin workflows with dependency management and various execution modes.

### Key Features

- **Workflow Management**: Create and manage complex plugin workflows
- **Multiple Execution Modes**: Sequential, parallel, conditional execution
- **Dependency Resolution**: Automatic step dependency ordering
- **Async Execution**: Non-blocking workflow execution
- **Progress Monitoring**: Real-time workflow execution tracking
- **Error Handling**: Comprehensive error handling with rollback support

## Class Reference

### Constructor and Factory Methods

```python
import qtforge

# Create orchestrator instance
orchestrator = qtforge.orchestration.PluginOrchestrator()

# Alternative creation methods
orchestrator = qtforge.orchestration.create_orchestrator()
orchestrator = qtforge.create_orchestrator()
```

### Workflow Management

#### `register_workflow()`
```python
def register_workflow(self, workflow: qtforge.orchestration.Workflow) -> qtforge.Expected[None, qtforge.PluginError]:
    """Register a workflow for execution.
    
    Args:
        workflow: Workflow definition to register
        
    Returns:
        Expected indicating success or failure
        
    Example:
        >>> workflow = qtforge.orchestration.Workflow("data_pipeline", "Data Processing")
        >>> result = orchestrator.register_workflow(workflow)
        >>> if result:
        ...     print("Workflow registered successfully")
    """
```

#### `unregister_workflow()`
```python
def unregister_workflow(self, workflow_id: str) -> qtforge.Expected[None, qtforge.PluginError]:
    """Unregister a workflow.
    
    Args:
        workflow_id: Workflow identifier to unregister
        
    Returns:
        Expected indicating success or failure
    """
```

#### `has_workflow()`
```python
def has_workflow(self, workflow_id: str) -> bool:
    """Check if a workflow is registered.
    
    Args:
        workflow_id: Workflow identifier to check
        
    Returns:
        True if workflow is registered, False otherwise
    """
```

#### `get_workflow()`
```python
def get_workflow(self, workflow_id: str) -> qtforge.Expected[qtforge.orchestration.Workflow, qtforge.PluginError]:
    """Get a registered workflow.
    
    Args:
        workflow_id: Workflow identifier
        
    Returns:
        Expected containing workflow or error
    """
```

#### `list_workflows()`
```python
def list_workflows(self) -> List[str]:
    """Get list of all registered workflow IDs.
    
    Returns:
        List of workflow identifiers
        
    Example:
        >>> workflows = orchestrator.list_workflows()
        >>> print(f"Registered workflows: {workflows}")
    """
```

### Workflow Execution

#### `execute_workflow()`
```python
def execute_workflow(self, workflow_id: str, initial_data: Dict = None, async_execution: bool = False) -> qtforge.Expected[str, qtforge.PluginError]:
    """Execute a registered workflow.
    
    Args:
        workflow_id: Workflow to execute
        initial_data: Initial data for the workflow
        async_execution: Whether to execute asynchronously
        
    Returns:
        Expected containing execution ID or error
        
    Example:
        >>> result = orchestrator.execute_workflow("data_pipeline", {"input_file": "data.csv"})
        >>> if result:
        ...     execution_id = result.value()
        ...     print(f"Workflow started: {execution_id}")
    """
```

#### `cancel_workflow()`
```python
def cancel_workflow(self, execution_id: str) -> qtforge.Expected[None, qtforge.PluginError]:
    """Cancel a running workflow execution.
    
    Args:
        execution_id: Execution to cancel
        
    Returns:
        Expected indicating success or failure
    """
```

#### `get_workflow_status()`
```python
def get_workflow_status(self, execution_id: str) -> qtforge.Expected[Dict, qtforge.PluginError]:
    """Get status of a workflow execution.
    
    Args:
        execution_id: Execution identifier
        
    Returns:
        Expected containing status information or error
        
    Example:
        >>> status_result = orchestrator.get_workflow_status(execution_id)
        >>> if status_result:
        ...     status = status_result.value()
        ...     print(f"Progress: {status['progress']}%")
        ...     print(f"Current step: {status['current_step']}")
    """
```

#### `get_workflow_results()`
```python
def get_workflow_results(self, execution_id: str) -> qtforge.Expected[Dict, qtforge.PluginError]:
    """Get results of a completed workflow execution.
    
    Args:
        execution_id: Execution identifier
        
    Returns:
        Expected containing workflow results or error
    """
```

### Utility Methods

#### `clear_workflows()`
```python
def clear_workflows(self) -> None:
    """Clear all registered workflows."""
```

## Workflow Class

### Constructor

```python
# Create workflow
workflow = qtforge.orchestration.Workflow("workflow_id", "Workflow Name")

# Alternative creation
workflow = qtforge.orchestration.create_workflow("workflow_id", "Workflow Name")
```

### Configuration Methods

#### `set_description()`
```python
def set_description(self, description: str) -> qtforge.orchestration.Workflow:
    """Set workflow description.
    
    Args:
        description: Workflow description
        
    Returns:
        Self for method chaining
    """
```

#### `set_execution_mode()`
```python
def set_execution_mode(self, mode: qtforge.orchestration.ExecutionMode) -> qtforge.orchestration.Workflow:
    """Set workflow execution mode.
    
    Args:
        mode: Execution mode (Sequential, Parallel, Conditional, Pipeline)
        
    Returns:
        Self for method chaining
        
    Example:
        >>> workflow.set_execution_mode(qtforge.orchestration.ExecutionMode.Sequential)
    """
```

#### `add_step()`
```python
def add_step(self, step: qtforge.orchestration.WorkflowStep) -> qtforge.orchestration.Workflow:
    """Add a step to the workflow.
    
    Args:
        step: Workflow step to add
        
    Returns:
        Self for method chaining
        
    Example:
        >>> step = qtforge.orchestration.WorkflowStep("load_data", "data_loader", "load")
        >>> workflow.add_step(step)
    """
```

## WorkflowStep Class

### Constructor

```python
# Create workflow step
step = qtforge.orchestration.WorkflowStep("step_id", "plugin_id", "method_name")

# Alternative creation
step = qtforge.orchestration.create_workflow_step("step_id", "plugin_id", "method_name")
```

### Properties

```python
# Step configuration
step.parameters = {"input_file": "data.csv", "format": "csv"}
step.dependencies = ["previous_step_id"]
step.timeout = 60000  # milliseconds
step.max_retries = 3
step.critical = True
```

## Enumerations

### ExecutionMode

```python
import qtforge

# Available execution modes
qtforge.orchestration.ExecutionMode.Sequential   # Execute steps one by one
qtforge.orchestration.ExecutionMode.Parallel    # Execute independent steps in parallel
qtforge.orchestration.ExecutionMode.Conditional # Execute based on conditions
qtforge.orchestration.ExecutionMode.Pipeline    # Execute as pipeline with data flow
```

### StepStatus

```python
import qtforge

# Step execution states
qtforge.orchestration.StepStatus.Pending    # Step is waiting to be executed
qtforge.orchestration.StepStatus.Running    # Step is currently executing
qtforge.orchestration.StepStatus.Completed  # Step completed successfully
qtforge.orchestration.StepStatus.Failed     # Step failed with error
qtforge.orchestration.StepStatus.Skipped    # Step was skipped due to conditions
qtforge.orchestration.StepStatus.Cancelled  # Step was cancelled
```

## Usage Examples

### Basic Workflow Creation and Execution

```python
import qtforge

def create_data_processing_workflow():
    # Create orchestrator
    orchestrator = qtforge.orchestration.PluginOrchestrator()
    
    # Create workflow
    workflow = qtforge.orchestration.Workflow("data_processing", "Data Processing Pipeline")
    workflow.set_description("Process CSV data through multiple stages")
    workflow.set_execution_mode(qtforge.orchestration.ExecutionMode.Sequential)
    
    # Create workflow steps
    load_step = qtforge.orchestration.WorkflowStep("load", "csv_loader", "load_file")
    load_step.parameters = {"file_path": "input.csv"}
    
    validate_step = qtforge.orchestration.WorkflowStep("validate", "data_validator", "validate")
    validate_step.dependencies = ["load"]
    
    transform_step = qtforge.orchestration.WorkflowStep("transform", "data_transformer", "transform")
    transform_step.dependencies = ["validate"]
    transform_step.parameters = {"operation": "normalize"}
    
    save_step = qtforge.orchestration.WorkflowStep("save", "csv_writer", "save_file")
    save_step.dependencies = ["transform"]
    save_step.parameters = {"output_path": "output.csv"}
    
    # Add steps to workflow
    workflow.add_step(load_step)
    workflow.add_step(validate_step)
    workflow.add_step(transform_step)
    workflow.add_step(save_step)
    
    # Register workflow
    result = orchestrator.register_workflow(workflow)
    if result:
        print("Workflow registered successfully")
        return orchestrator
    else:
        print(f"Failed to register workflow: {result.error().message}")
        return None

def execute_workflow(orchestrator):
    # Execute workflow
    execution_result = orchestrator.execute_workflow("data_processing")
    if not execution_result:
        print(f"Failed to start workflow: {execution_result.error().message}")
        return
    
    execution_id = execution_result.value()
    print(f"Workflow execution started: {execution_id}")
    
    # Monitor execution
    import time
    while True:
        status_result = orchestrator.get_workflow_status(execution_id)
        if status_result:
            status = status_result.value()
            print(f"Progress: {status.get('progress', 0)}% - {status.get('current_step', 'Unknown')}")
            
            if status.get('status') == 'completed':
                print("Workflow completed successfully!")
                
                # Get results
                results_result = orchestrator.get_workflow_results(execution_id)
                if results_result:
                    results = results_result.value()
                    print(f"Results: {results}")
                break
            elif status.get('status') == 'failed':
                print("Workflow execution failed!")
                break
        
        time.sleep(1)

# Usage
orchestrator = create_data_processing_workflow()
if orchestrator:
    execute_workflow(orchestrator)
```

### Parallel Workflow Execution

```python
import qtforge

def create_parallel_workflow():
    orchestrator = qtforge.orchestration.PluginOrchestrator()
    
    # Create parallel workflow
    workflow = qtforge.orchestration.Workflow("parallel_processing", "Parallel Processing")
    workflow.set_execution_mode(qtforge.orchestration.ExecutionMode.Parallel)
    
    # Independent parallel steps
    process_images = qtforge.orchestration.WorkflowStep("images", "image_processor", "process_batch")
    process_images.parameters = {"input_dir": "images/", "format": "jpg"}
    
    process_videos = qtforge.orchestration.WorkflowStep("videos", "video_processor", "process_batch")
    process_videos.parameters = {"input_dir": "videos/", "format": "mp4"}
    
    process_audio = qtforge.orchestration.WorkflowStep("audio", "audio_processor", "process_batch")
    process_audio.parameters = {"input_dir": "audio/", "format": "wav"}
    
    # Final aggregation step (depends on all parallel steps)
    aggregate = qtforge.orchestration.WorkflowStep("aggregate", "aggregator", "combine_results")
    aggregate.dependencies = ["images", "videos", "audio"]
    
    # Add steps
    workflow.add_step(process_images)
    workflow.add_step(process_videos)
    workflow.add_step(process_audio)
    workflow.add_step(aggregate)
    
    # Register and execute
    orchestrator.register_workflow(workflow)
    return orchestrator.execute_workflow("parallel_processing")

# Execute parallel workflow
result = create_parallel_workflow()
if result:
    print(f"Parallel workflow started: {result.value()}")
```

### Error Handling and Retry Logic

```python
import qtforge

def create_robust_workflow():
    orchestrator = qtforge.orchestration.PluginOrchestrator()
    
    workflow = qtforge.orchestration.Workflow("robust_processing", "Robust Processing")
    
    # Step with retry logic
    unreliable_step = qtforge.orchestration.WorkflowStep("fetch_data", "http_client", "fetch")
    unreliable_step.parameters = {"url": "https://api.example.com/data"}
    unreliable_step.max_retries = 3
    unreliable_step.timeout = 30000  # 30 seconds
    unreliable_step.critical = True
    
    # Fallback step (non-critical)
    fallback_step = qtforge.orchestration.WorkflowStep("fallback", "cache_reader", "read_cached")
    fallback_step.dependencies = ["fetch_data"]
    fallback_step.critical = False  # Won't fail entire workflow
    
    workflow.add_step(unreliable_step)
    workflow.add_step(fallback_step)
    
    orchestrator.register_workflow(workflow)
    
    # Execute with error handling
    try:
        result = orchestrator.execute_workflow("robust_processing")
        if result:
            execution_id = result.value()
            print(f"Robust workflow started: {execution_id}")
            return execution_id
        else:
            print(f"Failed to start workflow: {result.error().message}")
    except qtforge.PluginError as e:
        print(f"Plugin error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")
    
    return None
```

### Workflow Status Monitoring

```python
import qtforge
import asyncio

async def monitor_workflow_async(orchestrator, execution_id):
    """Asynchronously monitor workflow execution."""
    while True:
        status_result = orchestrator.get_workflow_status(execution_id)
        if status_result:
            status = status_result.value()
            
            print(f"Workflow {execution_id}:")
            print(f"  Status: {status.get('status', 'unknown')}")
            print(f"  Progress: {status.get('progress', 0):.1%}")
            print(f"  Current Step: {status.get('current_step', 'none')}")
            print(f"  Completed Steps: {status.get('completed_steps', 0)}/{status.get('total_steps', 0)}")
            
            if status.get('status') in ['completed', 'failed', 'cancelled']:
                break
        
        await asyncio.sleep(1)

def monitor_multiple_workflows(orchestrator, execution_ids):
    """Monitor multiple workflow executions."""
    import concurrent.futures
    
    def monitor_single(execution_id):
        while True:
            status_result = orchestrator.get_workflow_status(execution_id)
            if status_result:
                status = status_result.value()
                if status.get('status') in ['completed', 'failed', 'cancelled']:
                    return execution_id, status.get('status')
            time.sleep(0.5)
    
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = {executor.submit(monitor_single, eid): eid for eid in execution_ids}
        
        for future in concurrent.futures.as_completed(futures):
            execution_id, final_status = future.result()
            print(f"Workflow {execution_id} finished with status: {final_status}")
```

## See Also

- [Workflow Python API](workflow.md)
- [WorkflowStep Python API](workflow-step.md)
- [Advanced Orchestrator Python API](advanced-orchestrator.md)
- [Orchestration User Guide](../../../user-guide/workflow-orchestration.md)

---

*Last updated: December 2024 | QtForge v3.1.0*
