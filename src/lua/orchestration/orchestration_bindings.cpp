/**
 * @file orchestration_bindings.cpp
 * @brief Orchestration bindings for Lua
 * @version 3.2.0
 */

#include <QDebug>
#include <QLoggingCategory>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include <qtplugin/orchestration/plugin_orchestrator.hpp>
#include "../qt_conversions.cpp"

Q_LOGGING_CATEGORY(orchestrationBindingsLog, "qtforge.lua.orchestration");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register StepStatus enum with Lua
 */
void register_step_status_bindings(sol::state& lua) {
    lua.new_enum<qtplugin::orchestration::StepStatus>("StepStatus", {
        {"Pending", qtplugin::orchestration::StepStatus::Pending},
        {"Running", qtplugin::orchestration::StepStatus::Running},
        {"Completed", qtplugin::orchestration::StepStatus::Completed},
        {"Failed", qtplugin::orchestration::StepStatus::Failed},
        {"Skipped", qtplugin::orchestration::StepStatus::Skipped},
        {"Cancelled", qtplugin::orchestration::StepStatus::Cancelled},
        {"Retrying", qtplugin::orchestration::StepStatus::Retrying}
    });

    qCDebug(orchestrationBindingsLog) << "StepStatus bindings registered";
}

/**
 * @brief Register ExecutionMode enum with Lua
 */
void register_execution_mode_bindings(sol::state& lua) {
    lua.new_enum<qtplugin::orchestration::ExecutionMode>("ExecutionMode", {
        {"Sequential", qtplugin::orchestration::ExecutionMode::Sequential},
        {"Parallel", qtplugin::orchestration::ExecutionMode::Parallel},
        {"Conditional", qtplugin::orchestration::ExecutionMode::Conditional},
        {"Pipeline", qtplugin::orchestration::ExecutionMode::Pipeline}
    });

    qCDebug(orchestrationBindingsLog) << "ExecutionMode bindings registered";
}

/**
 * @brief Register WorkflowStep class with Lua
 */
void register_workflow_step_bindings(sol::state& lua) {
    auto step_type = lua.new_usertype<qtplugin::orchestration::WorkflowStep>("WorkflowStep",
        sol::constructors<
            qtplugin::orchestration::WorkflowStep(),
            qtplugin::orchestration::WorkflowStep(const QString&, const QString&, const QString&)
        >()
    );

    step_type["id"] = &qtplugin::orchestration::WorkflowStep::id;
    step_type["name"] = &qtplugin::orchestration::WorkflowStep::name;
    step_type["description"] = &qtplugin::orchestration::WorkflowStep::description;
    step_type["plugin_id"] = &qtplugin::orchestration::WorkflowStep::plugin_id;
    step_type["service_name"] = &qtplugin::orchestration::WorkflowStep::service_name;
    step_type["method_name"] = &qtplugin::orchestration::WorkflowStep::method_name;
    step_type["dependencies"] = &qtplugin::orchestration::WorkflowStep::dependencies;
    step_type["max_retries"] = &qtplugin::orchestration::WorkflowStep::max_retries;
    step_type["critical"] = &qtplugin::orchestration::WorkflowStep::critical;

    // Parameters and metadata (QJsonObject)
    step_type["parameters"] = sol::property(
        [&lua](const qtplugin::orchestration::WorkflowStep& step) -> sol::object {
            return qtforge_lua::qjson_to_lua(step.parameters, lua);
        },
        [](qtplugin::orchestration::WorkflowStep& step, const sol::object& params) {
            if (params.get_type() == sol::type::table) {
                QJsonValue json_value = qtforge_lua::lua_to_qjson(params);
                if (json_value.isObject()) {
                    step.parameters = json_value.toObject();
                }
            }
        }
    );

    step_type["metadata"] = sol::property(
        [&lua](const qtplugin::orchestration::WorkflowStep& step) -> sol::object {
            return qtforge_lua::qjson_to_lua(step.metadata, lua);
        },
        [](qtplugin::orchestration::WorkflowStep& step, const sol::object& metadata) {
            if (metadata.get_type() == sol::type::table) {
                QJsonValue json_value = qtforge_lua::lua_to_qjson(metadata);
                if (json_value.isObject()) {
                    step.metadata = json_value.toObject();
                }
            }
        }
    );

    qCDebug(orchestrationBindingsLog) << "WorkflowStep bindings registered";
}

/**
 * @brief Register WorkflowResult class with Lua
 */
void register_workflow_result_bindings(sol::state& lua) {
    auto result_type = lua.new_usertype<qtplugin::orchestration::WorkflowResult>("WorkflowResult");

    result_type["success"] = &qtplugin::orchestration::WorkflowResult::success;
    result_type["error_message"] = &qtplugin::orchestration::WorkflowResult::error_message;
    result_type["execution_time_ms"] = &qtplugin::orchestration::WorkflowResult::execution_time_ms;
    result_type["steps_executed"] = &qtplugin::orchestration::WorkflowResult::steps_executed;
    result_type["steps_failed"] = &qtplugin::orchestration::WorkflowResult::steps_failed;

    // Results (QJsonObject) - simplified for now
    result_type["has_results"] = [](const qtplugin::orchestration::WorkflowResult& result) -> bool {
        return !result.results.isEmpty();
    };

    qCDebug(orchestrationBindingsLog) << "WorkflowResult bindings registered";
}

/**
 * @brief Register PluginOrchestrator class with Lua
 */
void register_plugin_orchestrator_bindings(sol::state& lua) {
    auto orchestrator_type = lua.new_usertype<qtplugin::orchestration::PluginOrchestrator>("PluginOrchestrator",
        sol::constructors<qtplugin::orchestration::PluginOrchestrator(QObject*)>()
    );

    // Workflow management
    orchestrator_type["create_workflow"] = &qtplugin::orchestration::PluginOrchestrator::create_workflow;
    orchestrator_type["delete_workflow"] = &qtplugin::orchestration::PluginOrchestrator::delete_workflow;
    orchestrator_type["get_workflow"] = &qtplugin::orchestration::PluginOrchestrator::get_workflow;
    orchestrator_type["list_workflows"] = &qtplugin::orchestration::PluginOrchestrator::list_workflows;

    // Step management
    orchestrator_type["add_step"] = &qtplugin::orchestration::PluginOrchestrator::add_step;
    orchestrator_type["remove_step"] = &qtplugin::orchestration::PluginOrchestrator::remove_step;
    orchestrator_type["update_step"] = &qtplugin::orchestration::PluginOrchestrator::update_step;
    orchestrator_type["get_step"] = &qtplugin::orchestration::PluginOrchestrator::get_step;

    // Execution
    orchestrator_type["execute_workflow"] = &qtplugin::orchestration::PluginOrchestrator::execute_workflow;
    orchestrator_type["execute_step"] = &qtplugin::orchestration::PluginOrchestrator::execute_step;
    orchestrator_type["cancel_workflow"] = &qtplugin::orchestration::PluginOrchestrator::cancel_workflow;
    orchestrator_type["pause_workflow"] = &qtplugin::orchestration::PluginOrchestrator::pause_workflow;
    orchestrator_type["resume_workflow"] = &qtplugin::orchestration::PluginOrchestrator::resume_workflow;

    // Status and monitoring
    orchestrator_type["get_workflow_status"] = &qtplugin::orchestration::PluginOrchestrator::get_workflow_status;
    orchestrator_type["get_step_status"] = &qtplugin::orchestration::PluginOrchestrator::get_step_status;
    orchestrator_type["is_workflow_running"] = &qtplugin::orchestration::PluginOrchestrator::is_workflow_running;

    qCDebug(orchestrationBindingsLog) << "PluginOrchestrator bindings registered";
}

void register_orchestration_bindings(sol::state& lua) {
    qCDebug(orchestrationBindingsLog) << "Registering orchestration bindings...";

    // Create qtforge.orchestration namespace
    sol::table qtforge = lua["qtforge"];
    sol::table orchestration = qtforge.get_or_create<sol::table>("orchestration");

    // Register enums and classes
    register_step_status_bindings(lua);
    register_execution_mode_bindings(lua);
    register_workflow_step_bindings(lua);
    register_workflow_result_bindings(lua);
    register_plugin_orchestrator_bindings(lua);

    // Factory functions
    orchestration["create_workflow_step"] = [](const std::string& id, const std::string& name, const std::string& plugin_id) {
        return qtplugin::orchestration::WorkflowStep{QString::fromStdString(id), QString::fromStdString(name), QString::fromStdString(plugin_id)};
    };

    orchestration["create_orchestrator"] = []() {
        return std::make_shared<qtplugin::orchestration::PluginOrchestrator>();
    };

    // Utility functions
    orchestration["status_to_string"] = [](qtplugin::orchestration::StepStatus status) -> std::string {
        switch (status) {
            case qtplugin::orchestration::StepStatus::Pending: return "Pending";
            case qtplugin::orchestration::StepStatus::Running: return "Running";
            case qtplugin::orchestration::StepStatus::Completed: return "Completed";
            case qtplugin::orchestration::StepStatus::Failed: return "Failed";
            case qtplugin::orchestration::StepStatus::Skipped: return "Skipped";
            case qtplugin::orchestration::StepStatus::Cancelled: return "Cancelled";
            case qtplugin::orchestration::StepStatus::Retrying: return "Retrying";
            default: return "Unknown";
        }
    };

    orchestration["mode_to_string"] = [](qtplugin::orchestration::ExecutionMode mode) -> std::string {
        switch (mode) {
            case qtplugin::orchestration::ExecutionMode::Sequential: return "Sequential";
            case qtplugin::orchestration::ExecutionMode::Parallel: return "Parallel";
            case qtplugin::orchestration::ExecutionMode::Conditional: return "Conditional";
            case qtplugin::orchestration::ExecutionMode::Pipeline: return "Pipeline";
            default: return "Unknown";
        }
    };

    qCDebug(orchestrationBindingsLog) << "Orchestration bindings registered successfully";
}

#else // QTFORGE_LUA_BINDINGS not defined

namespace sol { class state; }

void register_orchestration_bindings(sol::state& lua) {
    (void)lua;
    qCWarning(orchestrationBindingsLog) << "Orchestration bindings not available - Lua support not compiled";
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
