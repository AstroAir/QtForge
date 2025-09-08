/**
 * @file orchestration_bindings.cpp
 * @brief Orchestration system Python bindings (simplified version)
 * @version 3.2.0
 * @author QtForge Development Team
 */

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <qtplugin/orchestration/plugin_orchestrator.hpp>

namespace py = pybind11;
using namespace qtplugin::orchestration;

namespace qtforge_python {

void bind_orchestration(py::module& m) {
    // === Step Status Enum ===
    py::enum_<StepStatus>(m, "StepStatus", "Workflow step status")
        .value("Pending", StepStatus::Pending, "Step is pending")
        .value("Running", StepStatus::Running, "Step is running")
        .value("Completed", StepStatus::Completed, "Step completed successfully")
        .value("Failed", StepStatus::Failed, "Step failed")
        .value("Skipped", StepStatus::Skipped, "Step was skipped")
        .value("Cancelled", StepStatus::Cancelled, "Step was cancelled")
        .value("Retrying", StepStatus::Retrying, "Step is retrying")
        .export_values();

    // === Execution Mode Enum ===
    py::enum_<ExecutionMode>(m, "ExecutionMode", "Workflow execution modes")
        .value("Sequential", ExecutionMode::Sequential, "Execute steps sequentially")
        .value("Parallel", ExecutionMode::Parallel, "Execute steps in parallel")
        .value("Conditional", ExecutionMode::Conditional, "Execute steps conditionally")
        .value("Pipeline", ExecutionMode::Pipeline, "Execute steps as pipeline")
        .export_values();

    // Note: WorkflowPriority and WorkflowState are not yet implemented in the C++ headers
    // These bindings are placeholders for future implementation

    // === Utility Functions ===
    m.def("test_orchestration", []() -> std::string {
        return "Orchestration module working!";
    }, "Test function for orchestration module");

    m.def("get_available_orchestration_features", []() -> py::list {
        py::list features;
        features.append("step_status");
        features.append("execution_modes");
        features.append("workflow_priority");
        features.append("workflow_state");
        return features;
    }, "Get list of available orchestration features");

    m.def("validate_step_status", [](int status) -> bool {
        return status >= static_cast<int>(StepStatus::Pending) &&
               status <= static_cast<int>(StepStatus::Retrying);
    }, "Validate step status value", py::arg("status"));

    m.def("validate_execution_mode", [](int mode) -> bool {
        return mode >= static_cast<int>(ExecutionMode::Sequential) &&
               mode <= static_cast<int>(ExecutionMode::Pipeline);
    }, "Validate execution mode value", py::arg("mode"));

    // Note: validate_workflow_priority function removed as WorkflowPriority is not yet implemented
}

}  // namespace qtforge_python
