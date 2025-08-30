/**
 * @file orchestration_bindings.cpp
 * @brief Orchestration system Python bindings
 * @version 3.0.0
 * @author QtForge Development Team
 */

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <qtplugin/orchestration/plugin_orchestrator.hpp>

#include "../qt_conversions.hpp"

namespace py = pybind11;
using namespace qtplugin::orchestration;

namespace qtforge_python {

void bind_orchestration(py::module& m) {
    // Step status enum
    py::enum_<StepStatus>(m, "StepStatus")
        .value("Pending", StepStatus::Pending)
        .value("Running", StepStatus::Running)
        .value("Completed", StepStatus::Completed)
        .value("Failed", StepStatus::Failed)
        .value("Skipped", StepStatus::Skipped)
        .value("Cancelled", StepStatus::Cancelled)
        .value("Retrying", StepStatus::Retrying)
        .export_values();

    // Execution mode enum
    py::enum_<ExecutionMode>(m, "ExecutionMode")
        .value("Sequential", ExecutionMode::Sequential)
        .value("Parallel", ExecutionMode::Parallel)
        .value("Conditional", ExecutionMode::Conditional)
        .value("Pipeline", ExecutionMode::Pipeline)
        .export_values();

    // Workflow step
    py::class_<WorkflowStep>(m, "WorkflowStep")
        .def(py::init<>())
        .def(py::init<const QString&, const QString&, const QString&>())
        .def_readwrite("id", &WorkflowStep::id)
        .def_readwrite("name", &WorkflowStep::name)
        .def_readwrite("description", &WorkflowStep::description)
        .def_readwrite("plugin_id", &WorkflowStep::plugin_id)
        .def_readwrite("service_name", &WorkflowStep::service_name)
        .def_readwrite("method_name", &WorkflowStep::method_name)
        .def_readwrite("parameters", &WorkflowStep::parameters)
        .def_readwrite("dependencies", &WorkflowStep::dependencies)
        .def_readwrite("max_retries", &WorkflowStep::max_retries)
        .def_readwrite("critical", &WorkflowStep::critical)
        .def_readwrite("metadata", &WorkflowStep::metadata)
        .def("__repr__", [](const WorkflowStep& step) {
            return "<WorkflowStep id='" + step.id.toStdString() + "' plugin='" +
                   step.plugin_id.toStdString() + "'>";
        });

    // Step result
    py::class_<StepResult>(m, "StepResult")
        .def(py::init<>())
        .def_readwrite("step_id", &StepResult::step_id)
        .def_readwrite("status", &StepResult::status)
        .def_readwrite("result_data", &StepResult::result_data)
        .def_readwrite("error_message", &StepResult::error_message)
        .def_readwrite("retry_count", &StepResult::retry_count)
        .def("execution_time", &StepResult::execution_time)
        .def("__repr__", [](const StepResult& result) {
            return "<StepResult step='" + result.step_id.toStdString() +
                   "' status=" +
                   std::to_string(static_cast<int>(result.status)) + ">";
        });

    // Workflow
    py::class_<Workflow>(m, "Workflow")
        .def(py::init<>())
        .def(py::init<const QString&, const QString&>())
        .def("set_description", &Workflow::set_description,
             py::return_value_policy::reference)
        .def("set_execution_mode", &Workflow::set_execution_mode,
             py::return_value_policy::reference)
        .def("add_step", &Workflow::add_step,
             py::return_value_policy::reference)
        .def("add_rollback_step", &Workflow::add_rollback_step,
             py::return_value_policy::reference)
        .def("id", &Workflow::id)
        .def("name", &Workflow::name)
        .def("description", &Workflow::description)
        .def("execution_mode", &Workflow::execution_mode)
        .def("__repr__", [](const Workflow& workflow) {
            return "<Workflow id='" + workflow.id().toStdString() + "' name='" +
                   workflow.name().toStdString() + "'>";
        });

    // Plugin orchestrator
    py::class_<PluginOrchestrator, std::shared_ptr<PluginOrchestrator>>(
        m, "PluginOrchestrator")
        .def(py::init<>())
        .def_static("create", &PluginOrchestrator::create)
        .def("register_workflow", &PluginOrchestrator::register_workflow)
        .def("unregister_workflow", &PluginOrchestrator::unregister_workflow)
        .def("has_workflow", &PluginOrchestrator::has_workflow)
        .def("get_workflow", &PluginOrchestrator::get_workflow)
        .def("list_workflows", &PluginOrchestrator::list_workflows)
        .def("execute_workflow", &PluginOrchestrator::execute_workflow)
        .def("cancel_workflow", &PluginOrchestrator::cancel_workflow)
        .def("get_workflow_status", &PluginOrchestrator::get_workflow_status)
        .def("get_workflow_results", &PluginOrchestrator::get_workflow_results)
        .def("clear_workflows", &PluginOrchestrator::clear_workflows)
        .def("__repr__", [](const PluginOrchestrator& orchestrator) {
            return "<PluginOrchestrator workflows=" +
                   std::to_string(orchestrator.list_workflows().size()) + ">";
        });

    // Utility functions
    m.def(
        "create_orchestrator",
        []() -> std::shared_ptr<PluginOrchestrator> {
            return PluginOrchestrator::create();
        },
        "Create a new PluginOrchestrator instance");

    m.def(
        "create_workflow",
        [](const std::string& id, const std::string& name) -> Workflow {
            return Workflow(QString::fromStdString(id),
                            QString::fromStdString(name));
        },
        py::arg("id"), py::arg("name") = "", "Create a new Workflow instance");

    m.def(
        "create_workflow_step",
        [](const std::string& id, const std::string& plugin_id,
           const std::string& method) -> WorkflowStep {
            return WorkflowStep(QString::fromStdString(id),
                                QString::fromStdString(plugin_id),
                                QString::fromStdString(method));
        },
        py::arg("id"), py::arg("plugin_id"), py::arg("method"),
        "Create a new WorkflowStep instance");
}

}  // namespace qtforge_python
