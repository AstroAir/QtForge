#!/usr/bin/env python3
"""
QtForge Python Bindings Example 4: Orchestration and Workflows

This example demonstrates plugin orchestration and workflow management
using the QtForge orchestration system, including:
- Creating and managing workflows
- Defining workflow steps and dependencies
- Executing workflows in different modes
- Monitoring workflow progress and results
- Handling workflow errors and cancellation
"""

import sys
import time
import threading
from pathlib import Path

# Add the build directory to Python path
sys.path.insert(0, str(Path(__file__).parent.parent.parent / "build"))

try:
    import qtforge
    import qtforge.orchestration as orchestration
    print("✅ QtForge Orchestration bindings loaded successfully")
except ImportError as e:
    print(f"❌ Failed to import QtForge Orchestration: {e}")
    print("Make sure QtForge is built with Python bindings enabled")
    sys.exit(1)


def demonstrate_plugin_orchestrator():
    """Demonstrate plugin orchestrator creation and basic operations."""
    print("\n" + "="*50)
    print("🎭 Plugin Orchestrator Operations")
    print("="*50)
    
    try:
        # Create a plugin orchestrator
        orchestrator = orchestration.create_plugin_orchestrator()
        print("✅ Plugin orchestrator created successfully")
        
        # Check available methods
        methods = ['execute_workflow', 'register_plugin', 'get_registered_plugins']
        available_methods = []
        
        for method in methods:
            if hasattr(orchestrator, method):
                available_methods.append(method)
        
        print(f"📋 Available methods: {', '.join(available_methods)}")
        
        # Test plugin registration
        if hasattr(orchestrator, 'register_plugin'):
            print("\n📝 Testing plugin registration...")
            
            test_plugins = [
                ("data_processor", "/path/to/data_processor.so"),
                ("file_handler", "/path/to/file_handler.so"),
                ("network_client", "/path/to/network_client.so")
            ]
            
            for plugin_name, plugin_path in test_plugins:
                try:
                    orchestrator.register_plugin(plugin_name, plugin_path)
                    print(f"  ✅ Registered plugin: {plugin_name}")
                except Exception as e:
                    print(f"  ⚠️  Failed to register {plugin_name}: {e}")
        
        # Test getting registered plugins
        if hasattr(orchestrator, 'get_registered_plugins'):
            try:
                plugins = orchestrator.get_registered_plugins()
                print(f"\n📊 Total registered plugins: {len(plugins) if plugins else 0}")
                
                if plugins:
                    for plugin in plugins[:3]:  # Show first 3
                        print(f"  • {plugin}")
                        
            except Exception as e:
                print(f"⚠️  Failed to get registered plugins: {e}")
        
        return orchestrator
        
    except Exception as e:
        print(f"❌ Failed to create plugin orchestrator: {e}")
        return None


def demonstrate_workflow_creation():
    """Demonstrate workflow creation and configuration."""
    print("\n" + "="*50)
    print("📋 Workflow Creation and Configuration")
    print("="*50)
    
    if not hasattr(orchestration, 'Workflow'):
        print("❌ Workflow class not available")
        return None
    
    try:
        # Create a workflow
        workflow = orchestration.Workflow("data_processing_workflow")
        print("✅ Workflow created successfully")
        
        # Check workflow properties
        if hasattr(workflow, 'name'):
            print(f"📋 Workflow name: {workflow.name}")
        
        # Test execution mode setting
        if hasattr(workflow, 'set_execution_mode') and hasattr(orchestration, 'ExecutionMode'):
            print("\n⚙️  Testing execution mode configuration...")
            
            modes_to_test = ['Sequential', 'Parallel', 'Pipeline']
            
            for mode_name in modes_to_test:
                if hasattr(orchestration.ExecutionMode, mode_name):
                    mode = getattr(orchestration.ExecutionMode, mode_name)
                    
                    try:
                        workflow.set_execution_mode(mode)
                        print(f"  ✅ Set execution mode to {mode_name}")
                    except Exception as e:
                        print(f"  ⚠️  Failed to set mode {mode_name}: {e}")
        
        # Test workflow validation
        if hasattr(workflow, 'validate'):
            try:
                is_valid = workflow.validate()
                print(f"\n📊 Workflow validation result: {is_valid}")
            except Exception as e:
                print(f"⚠️  Workflow validation failed: {e}")
        
        return workflow
        
    except Exception as e:
        print(f"❌ Failed to create workflow: {e}")
        return None


def demonstrate_workflow_steps():
    """Demonstrate workflow step creation and management."""
    print("\n" + "="*50)
    print("🔧 Workflow Step Management")
    print("="*50)
    
    workflow = None
    if hasattr(orchestration, 'Workflow'):
        workflow = orchestration.Workflow("step_demo_workflow")
        print("✅ Created workflow for step demonstration")
    
    # Create workflow steps
    steps = []
    
    if hasattr(orchestration, 'WorkflowStep'):
        print("\n🔧 Creating workflow steps...")
        
        step_definitions = [
            ("load_data", "Load Data", "data_loader"),
            ("validate_data", "Validate Data", "data_validator"),
            ("process_data", "Process Data", "data_processor"),
            ("save_results", "Save Results", "result_saver")
        ]
        
        for step_id, step_name, plugin_id in step_definitions:
            try:
                step = orchestration.WorkflowStep(step_id, step_name, plugin_id)
                steps.append(step)
                print(f"  ✅ Created step: {step_name} ({step_id})")
                
                # Test step properties
                if hasattr(step, 'id'):
                    print(f"    📋 Step ID: {step.id}")
                if hasattr(step, 'name'):
                    print(f"    📋 Step name: {step.name}")
                if hasattr(step, 'plugin_id'):
                    print(f"    📋 Plugin ID: {step.plugin_id}")
                    
            except Exception as e:
                print(f"  ❌ Failed to create step {step_name}: {e}")
    
    # Test step parameters
    if steps and hasattr(steps[0], 'set_parameter'):
        print("\n⚙️  Testing step parameters...")
        
        step = steps[0]
        parameters = [
            ("input_file", "/data/input.csv"),
            ("output_format", "json"),
            ("batch_size", "1000"),
            ("timeout", "30")
        ]
        
        for param_name, param_value in parameters:
            try:
                step.set_parameter(param_name, param_value)
                print(f"  ✅ Set parameter {param_name}: {param_value}")
                
                if hasattr(step, 'get_parameter'):
                    retrieved_value = step.get_parameter(param_name)
                    print(f"    📊 Retrieved value: {retrieved_value}")
                    
            except Exception as e:
                print(f"  ⚠️  Failed to set parameter {param_name}: {e}")
    
    # Test step dependencies
    if len(steps) >= 2 and hasattr(steps[1], 'add_dependency'):
        print("\n🔗 Testing step dependencies...")
        
        # Create a dependency chain
        for i in range(1, len(steps)):
            try:
                steps[i].add_dependency(steps[i-1].id if hasattr(steps[i-1], 'id') else f"step_{i-1}")
                print(f"  ✅ Added dependency: {steps[i].name if hasattr(steps[i], 'name') else f'step_{i}'} depends on {steps[i-1].name if hasattr(steps[i-1], 'name') else f'step_{i-1}'}")
                
                if hasattr(steps[i], 'get_dependencies'):
                    deps = steps[i].get_dependencies()
                    print(f"    📊 Dependencies: {deps}")
                    
            except Exception as e:
                print(f"  ⚠️  Failed to add dependency: {e}")
    
    # Add steps to workflow
    if workflow and steps and hasattr(workflow, 'add_step'):
        print("\n📋 Adding steps to workflow...")
        
        for step in steps:
            try:
                workflow.add_step(step)
                print(f"  ✅ Added step to workflow: {step.name if hasattr(step, 'name') else 'unnamed'}")
            except Exception as e:
                print(f"  ⚠️  Failed to add step to workflow: {e}")
        
        # Get workflow steps
        if hasattr(workflow, 'get_steps'):
            try:
                workflow_steps = workflow.get_steps()
                print(f"  📊 Total steps in workflow: {len(workflow_steps) if workflow_steps else 0}")
            except Exception as e:
                print(f"  ⚠️  Failed to get workflow steps: {e}")
    
    return workflow, steps


def demonstrate_workflow_execution():
    """Demonstrate workflow execution in different modes."""
    print("\n" + "="*50)
    print("▶️  Workflow Execution")
    print("="*50)
    
    # Create orchestrator and workflow
    orchestrator = None
    if hasattr(orchestration, 'create_plugin_orchestrator'):
        orchestrator = orchestration.create_plugin_orchestrator()
    
    workflow = None
    if hasattr(orchestration, 'Workflow'):
        workflow = orchestration.Workflow("execution_demo")
        
        # Add some simple steps
        if hasattr(orchestration, 'WorkflowStep'):
            steps = [
                orchestration.WorkflowStep("step1", "First Step", "plugin1"),
                orchestration.WorkflowStep("step2", "Second Step", "plugin2"),
                orchestration.WorkflowStep("step3", "Third Step", "plugin3")
            ]
            
            if hasattr(workflow, 'add_step'):
                for step in steps:
                    try:
                        workflow.add_step(step)
                    except Exception as e:
                        print(f"⚠️  Failed to add step: {e}")
    
    if not orchestrator or not workflow:
        print("❌ Cannot demonstrate execution without orchestrator and workflow")
        return
    
    # Test different execution modes
    if hasattr(orchestration, 'ExecutionMode'):
        execution_modes = ['Sequential', 'Parallel']
        
        for mode_name in execution_modes:
            if hasattr(orchestration.ExecutionMode, mode_name):
                mode = getattr(orchestration.ExecutionMode, mode_name)
                
                print(f"\n▶️  Testing {mode_name} execution...")
                
                if hasattr(workflow, 'set_execution_mode'):
                    try:
                        workflow.set_execution_mode(mode)
                        print(f"  ✅ Set execution mode to {mode_name}")
                    except Exception as e:
                        print(f"  ⚠️  Failed to set execution mode: {e}")
                        continue
                
                # Execute workflow
                if hasattr(orchestrator, 'execute_workflow'):
                    try:
                        print(f"  🚀 Executing workflow in {mode_name} mode...")
                        result = orchestrator.execute_workflow(workflow)
                        
                        if result:
                            print(f"  ✅ Workflow execution completed: {result}")
                        else:
                            print(f"  ⚠️  Workflow execution returned: {result}")
                            
                    except Exception as e:
                        print(f"  ⚠️  Workflow execution failed: {e} (expected if plugins not available)")
    
    # Test asynchronous execution
    if hasattr(orchestrator, 'execute_workflow_async'):
        print("\n🔀 Testing asynchronous workflow execution...")
        
        try:
            future = orchestrator.execute_workflow_async(workflow)
            
            if future:
                print("  ✅ Async workflow execution started")
                
                # Test waiting for result
                if hasattr(future, 'wait'):
                    try:
                        result = future.wait(timeout=1.0)
                        print(f"  📊 Async execution result: {result}")
                    except Exception as e:
                        print(f"  ⚠️  Async execution wait failed: {e}")
                        
        except Exception as e:
            print(f"  ⚠️  Async workflow execution failed: {e}")


def demonstrate_step_results():
    """Demonstrate step result handling."""
    print("\n" + "="*50)
    print("📊 Step Result Handling")
    print("="*50)
    
    if not hasattr(orchestration, 'StepResult'):
        print("❌ StepResult class not available")
        return
    
    try:
        # Create successful step result
        success_result = orchestration.StepResult(True, "Step completed successfully")
        print("✅ Created successful step result")
        
        if hasattr(success_result, 'success'):
            print(f"📊 Success status: {success_result.success}")
        if hasattr(success_result, 'message'):
            print(f"📋 Result message: {success_result.message}")
        
        # Test setting result data
        if hasattr(success_result, 'set_data'):
            result_data = {
                "processed_items": 150,
                "execution_time": 2.5,
                "output_file": "/tmp/results.json"
            }
            
            success_result.set_data(result_data)
            print("✅ Set result data")
            
            if hasattr(success_result, 'get_data'):
                retrieved_data = success_result.get_data()
                print(f"📊 Retrieved data: {retrieved_data}")
        
        # Create failure step result
        failure_result = orchestration.StepResult(False, "Step failed due to invalid input")
        print("\n❌ Created failure step result")
        
        if hasattr(failure_result, 'success'):
            print(f"📊 Success status: {failure_result.success}")
        if hasattr(failure_result, 'message'):
            print(f"📋 Error message: {failure_result.message}")
        
        return [success_result, failure_result]
        
    except Exception as e:
        print(f"❌ Failed to create step results: {e}")
        return []


def demonstrate_orchestration_enums():
    """Demonstrate orchestration-related enumerations."""
    print("\n" + "="*50)
    print("📊 Orchestration Enumerations")
    print("="*50)
    
    # StepStatus enum
    if hasattr(orchestration, 'StepStatus'):
        print("🔄 Step Statuses:")
        statuses = ['Pending', 'Running', 'Completed', 'Failed', 'Skipped']
        
        for status in statuses:
            if hasattr(orchestration.StepStatus, status):
                value = getattr(orchestration.StepStatus, status)
                print(f"  • {status}: {value}")
    
    # ExecutionMode enum
    if hasattr(orchestration, 'ExecutionMode'):
        print("\n⚙️  Execution Modes:")
        modes = ['Sequential', 'Parallel', 'Pipeline']
        
        for mode in modes:
            if hasattr(orchestration.ExecutionMode, mode):
                value = getattr(orchestration.ExecutionMode, mode)
                print(f"  • {mode}: {value}")
    
    # WorkflowState enum
    if hasattr(orchestration, 'WorkflowState'):
        print("\n📋 Workflow States:")
        states = ['Created', 'Running', 'Completed', 'Failed', 'Cancelled']
        
        for state in states:
            if hasattr(orchestration.WorkflowState, state):
                value = getattr(orchestration.WorkflowState, state)
                print(f"  • {state}: {value}")


def demonstrate_workflow_monitoring():
    """Demonstrate workflow monitoring and progress tracking."""
    print("\n" + "="*50)
    print("📈 Workflow Monitoring and Progress Tracking")
    print("="*50)
    
    # Create a workflow with steps for monitoring
    if hasattr(orchestration, 'Workflow') and hasattr(orchestration, 'WorkflowStep'):
        workflow = orchestration.Workflow("monitoring_demo")
        
        # Create steps with status tracking
        steps = []
        for i in range(3):
            step = orchestration.WorkflowStep(f"step_{i}", f"Step {i+1}", f"plugin_{i}")
            steps.append(step)
            
            if hasattr(workflow, 'add_step'):
                try:
                    workflow.add_step(step)
                except Exception as e:
                    print(f"⚠️  Failed to add step {i}: {e}")
        
        print(f"✅ Created workflow with {len(steps)} steps for monitoring")
        
        # Test step status tracking
        if steps and hasattr(orchestration, 'StepStatus'):
            print("\n🔄 Testing step status tracking...")
            
            status_progression = ['Pending', 'Running', 'Completed']
            
            for step in steps[:2]:  # Test first 2 steps
                for status_name in status_progression:
                    if hasattr(orchestration.StepStatus, status_name):
                        status = getattr(orchestration.StepStatus, status_name)
                        
                        if hasattr(step, 'set_status'):
                            try:
                                step.set_status(status)
                                print(f"  ✅ Set {step.name if hasattr(step, 'name') else 'step'} status to {status_name}")
                                
                                if hasattr(step, 'get_status'):
                                    current_status = step.get_status()
                                    print(f"    📊 Current status: {current_status}")
                                    
                            except Exception as e:
                                print(f"  ⚠️  Failed to set status: {e}")
                        
                        time.sleep(0.1)  # Simulate processing time
        
        # Test workflow state tracking
        if hasattr(workflow, 'get_state') and hasattr(orchestration, 'WorkflowState'):
            try:
                state = workflow.get_state()
                print(f"\n📊 Current workflow state: {state}")
            except Exception as e:
                print(f"⚠️  Failed to get workflow state: {e}")


def demonstrate_error_handling():
    """Demonstrate error handling in orchestration operations."""
    print("\n" + "="*50)
    print("⚠️  Orchestration Error Handling")
    print("="*50)
    
    # Test various error conditions
    try:
        orchestrator = orchestration.create_plugin_orchestrator()
        
        # Test with invalid workflow
        print("🧪 Testing error handling with invalid parameters...")
        
        try:
            result = orchestrator.execute_workflow(None)
            print(f"  ⚠️  Unexpected success with None workflow: {result}")
        except Exception as e:
            print(f"  ✅ Correctly caught error for None workflow: {type(e).__name__}: {e}")
        
        # Test invalid step creation
        if hasattr(orchestration, 'WorkflowStep'):
            try:
                step = orchestration.WorkflowStep(None, "Test", "plugin")
                print(f"  ⚠️  Unexpected success with None step ID: {step}")
            except Exception as e:
                print(f"  ✅ Correctly caught error for None step ID: {type(e).__name__}: {e}")
            
            try:
                step = orchestration.WorkflowStep("test", None, "plugin")
                print(f"  ⚠️  Unexpected success with None step name: {step}")
            except Exception as e:
                print(f"  ✅ Correctly caught error for None step name: {type(e).__name__}: {e}")
        
        # Test circular dependency detection
        if hasattr(orchestration, 'Workflow') and hasattr(orchestration, 'WorkflowStep'):
            print("\n🔄 Testing circular dependency detection...")
            
            workflow = orchestration.Workflow("circular_test")
            step1 = orchestration.WorkflowStep("step1", "Step 1", "plugin1")
            step2 = orchestration.WorkflowStep("step2", "Step 2", "plugin2")
            
            if hasattr(step1, 'add_dependency') and hasattr(step2, 'add_dependency'):
                try:
                    # Create circular dependency
                    step1.add_dependency("step2")
                    step2.add_dependency("step1")
                    
                    if hasattr(workflow, 'add_step'):
                        workflow.add_step(step1)
                        workflow.add_step(step2)
                    
                    # Validation should detect circular dependency
                    if hasattr(workflow, 'validate'):
                        is_valid = workflow.validate()
                        if not is_valid:
                            print("  ✅ Correctly detected circular dependency")
                        else:
                            print("  ⚠️  Failed to detect circular dependency")
                            
                except Exception as e:
                    print(f"  ✅ Correctly caught circular dependency error: {e}")
        
    except Exception as e:
        print(f"❌ Error handling demonstration failed: {e}")


def main():
    """Main demonstration function."""
    print("QtForge Python Bindings - Orchestration and Workflows Example")
    print("=" * 65)
    
    # Demonstrate each aspect of orchestration
    demonstrate_plugin_orchestrator()
    demonstrate_workflow_creation()
    demonstrate_workflow_steps()
    demonstrate_workflow_execution()
    demonstrate_step_results()
    demonstrate_orchestration_enums()
    demonstrate_workflow_monitoring()
    demonstrate_error_handling()
    
    print("\n" + "="*65)
    print("🎉 Orchestration and Workflows Example Complete!")
    print("="*65)
    
    print("\n📚 Key Takeaways:")
    print("• Orchestrators coordinate complex multi-plugin workflows")
    print("• Workflows define step sequences with dependencies and parameters")
    print("• Different execution modes support various processing patterns")
    print("• Step results provide detailed feedback and data flow")
    print("• Monitoring and error handling ensure robust workflow execution")
    
    print("\n🔗 Next Steps:")
    print("• Create custom workflow templates and reusable patterns")
    print("• Implement workflow persistence and recovery mechanisms")
    print("• Add advanced scheduling and conditional execution")
    print("• Develop workflow visualization and debugging tools")


if __name__ == "__main__":
    main()
