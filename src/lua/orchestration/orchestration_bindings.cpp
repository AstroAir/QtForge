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

#include "../../../include/qtplugin/orchestration/plugin_orchestrator.hpp"
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

void register_orchestration_bindings(sol::state& lua) {
    qCDebug(orchestrationBindingsLog) << "Registering orchestration bindings...";

    // Create qtforge.orchestration namespace
    sol::table qtforge = lua["qtforge"];
    sol::table orchestration = qtforge.get_or_create<sol::table>("orchestration");

    // Register enums and classes
    register_step_status_bindings(lua);
    register_execution_mode_bindings(lua);
    register_workflow_step_bindings(lua);

    // Factory functions
    orchestration["create_workflow_step"] = [](const std::string& id, const std::string& name, const std::string& plugin_id) {
        return qtplugin::orchestration::WorkflowStep{QString::fromStdString(id), QString::fromStdString(name), QString::fromStdString(plugin_id)};
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
