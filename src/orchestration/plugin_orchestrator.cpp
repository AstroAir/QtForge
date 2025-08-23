/**
 * @file plugin_orchestrator.cpp
 * @brief Implementation of plugin orchestration framework
 * @version 3.1.0
 */

#include "qtplugin/orchestration/plugin_orchestrator.hpp"
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QUuid>
#include <algorithm>
#include <queue>
#include <set>
#include "qtplugin/core/plugin_manager.hpp"

Q_LOGGING_CATEGORY(orchestratorLog, "qtplugin.orchestrator")

namespace qtplugin::orchestration {

// === Workflow Implementation ===

qtplugin::expected<void, PluginError> Workflow::validate() const {
    if (m_id.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration,
                                "Workflow ID cannot be empty");
    }

    if (m_steps.empty()) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration,
                                "Workflow must have at least one step");
    }

    // Validate step dependencies
    for (const auto& [step_id, step] : m_steps) {
        for (const QString& dep_id : step.dependencies) {
            if (m_steps.find(dep_id) == m_steps.end()) {
                return make_error<void>(
                    PluginErrorCode::DependencyMissing,
                    "Step dependency not found: " + dep_id.toStdString());
            }
        }
    }

    // Check for circular dependencies
    auto execution_order_result = get_execution_order();
    if (execution_order_result.empty() && !m_steps.empty()) {
        return make_error<void>(PluginErrorCode::CircularDependency,
                                "Circular dependency detected in workflow");
    }

    return make_success();
}

std::vector<QString> Workflow::get_execution_order() const {
    std::vector<QString> result;
    std::set<QString> visited;
    std::set<QString> visiting;

    std::function<bool(const QString&)> visit =
        [&](const QString& step_id) -> bool {
        if (visiting.find(step_id) != visiting.end()) {
            return false;  // Circular dependency
        }
        if (visited.find(step_id) != visited.end()) {
            return true;  // Already processed
        }

        visiting.insert(step_id);

        auto step_it = m_steps.find(step_id);
        if (step_it != m_steps.end()) {
            for (const QString& dep_id : step_it->second.dependencies) {
                if (!visit(dep_id)) {
                    return false;
                }
            }
        }

        visiting.erase(step_id);
        visited.insert(step_id);
        result.push_back(step_id);
        return true;
    };

    for (const auto& [step_id, _] : m_steps) {
        if (visited.find(step_id) == visited.end()) {
            if (!visit(step_id)) {
                return {};  // Circular dependency detected
            }
        }
    }

    return result;
}

QJsonObject Workflow::to_json() const {
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["description"] = m_description;
    json["execution_mode"] = static_cast<int>(m_execution_mode);
    json["timeout"] = static_cast<int>(m_timeout.count());

    // Serialize steps
    QJsonObject steps_json;
    for (const auto& [step_id, step] : m_steps) {
        QJsonObject step_json;
        step_json["id"] = step.id;
        step_json["name"] = step.name;
        step_json["description"] = step.description;
        step_json["plugin_id"] = step.plugin_id;
        step_json["service_name"] = step.service_name;
        step_json["method_name"] = step.method_name;
        step_json["parameters"] = step.parameters;
        step_json["timeout"] = static_cast<int>(step.timeout.count());
        step_json["max_retries"] = step.max_retries;
        step_json["retry_delay"] = static_cast<int>(step.retry_delay.count());
        step_json["critical"] = step.critical;
        step_json["metadata"] = step.metadata;

        QJsonArray deps_array;
        for (const QString& dep : step.dependencies) {
            deps_array.append(dep);
        }
        step_json["dependencies"] = deps_array;

        steps_json[step_id] = step_json;
    }
    json["steps"] = steps_json;

    // Serialize rollback steps
    QJsonObject rollback_json;
    for (const auto& [step_id, rollback_step] : m_rollback_steps) {
        rollback_json[step_id] =
            QJsonObject{{"plugin_id", rollback_step.plugin_id},
                        {"method_name", rollback_step.method_name},
                        {"parameters", rollback_step.parameters}};
    }
    json["rollback_steps"] = rollback_json;

    return json;
}

qtplugin::expected<Workflow, PluginError> Workflow::from_json(
    const QJsonObject& json) {
    if (!json.contains("id") || !json["id"].isString()) {
        return make_error<Workflow>(PluginErrorCode::InvalidConfiguration,
                                    "Missing workflow ID");
    }

    QString id = json["id"].toString();
    QString name = json.value("name").toString(id);

    Workflow workflow(id, name);
    workflow.set_description(json.value("description").toString());
    workflow.set_execution_mode(
        static_cast<ExecutionMode>(json.value("execution_mode").toInt()));
    workflow.set_timeout(
        std::chrono::milliseconds(json.value("timeout").toInt(300000)));

    // Parse steps
    if (json.contains("steps") && json["steps"].isObject()) {
        QJsonObject steps_json = json["steps"].toObject();
        for (auto it = steps_json.begin(); it != steps_json.end(); ++it) {
            const QString& step_id = it.key();
            const QJsonObject& step_json = it.value().toObject();

            WorkflowStep step;
            step.id = step_json.value("id").toString(step_id);
            step.name = step_json.value("name").toString();
            step.description = step_json.value("description").toString();
            step.plugin_id = step_json.value("plugin_id").toString();
            step.service_name = step_json.value("service_name").toString();
            step.method_name = step_json.value("method_name").toString();
            step.parameters = step_json.value("parameters").toObject();
            step.timeout = std::chrono::milliseconds(
                step_json.value("timeout").toInt(60000));
            step.max_retries = step_json.value("max_retries").toInt(0);
            step.retry_delay = std::chrono::milliseconds(
                step_json.value("retry_delay").toInt(1000));
            step.critical = step_json.value("critical").toBool(true);
            step.metadata = step_json.value("metadata").toObject();

            // Parse dependencies
            if (step_json.contains("dependencies") &&
                step_json["dependencies"].isArray()) {
                QJsonArray deps_array = step_json["dependencies"].toArray();
                for (const auto& dep_value : deps_array) {
                    step.dependencies.push_back(dep_value.toString());
                }
            }

            workflow.add_step(step);
        }
    }

    // Parse rollback steps
    if (json.contains("rollback_steps") && json["rollback_steps"].isObject()) {
        QJsonObject rollback_json = json["rollback_steps"].toObject();
        for (auto it = rollback_json.begin(); it != rollback_json.end(); ++it) {
            const QString& step_id = it.key();
            const QJsonObject& rollback_json_step = it.value().toObject();

            WorkflowStep rollback_step;
            rollback_step.plugin_id =
                rollback_json_step["plugin_id"].toString();
            rollback_step.method_name =
                rollback_json_step["method_name"].toString();
            rollback_step.parameters =
                rollback_json_step["parameters"].toObject();

            workflow.add_rollback_step(step_id, rollback_step);
        }
    }

    // Validate the workflow
    auto validation_result = workflow.validate();
    if (!validation_result) {
        return qtplugin::unexpected<PluginError>(validation_result.error());
    }

    return workflow;
}

// === PluginOrchestrator Implementation ===

PluginOrchestrator::PluginOrchestrator(QObject* parent) : QObject(parent) {
    qCDebug(orchestratorLog) << "Plugin orchestrator created";
}

PluginOrchestrator::~PluginOrchestrator() {
    // Cancel all active executions
    std::vector<std::future<void>> futures_to_wait;
    {
        std::unique_lock lock(m_executions_mutex);
        for (auto& [execution_id, state] : m_active_executions) {
            if (state->context) {
                state->context->cancelled = true;
            }
            if (state->execution_future.valid()) {
                futures_to_wait.push_back(std::move(state->execution_future));
            }
        }
    } // Release mutex before waiting

    // Wait for executions to complete without holding the mutex
    for (auto& future : futures_to_wait) {
        if (future.valid()) {
            future.wait_for(std::chrono::seconds(5));
        }
    }

    qCDebug(orchestratorLog) << "Plugin orchestrator destroyed";
}

qtplugin::expected<void, PluginError> PluginOrchestrator::register_workflow(
    const Workflow& workflow) {
    auto validation_result = workflow.validate();
    if (!validation_result) {
        return validation_result;
    }

    std::unique_lock lock(m_workflows_mutex);

    if (m_workflows.find(workflow.id()) != m_workflows.end()) {
        return make_error<void>(
            PluginErrorCode::DuplicatePlugin,
            "Workflow already registered: " + workflow.id().toStdString());
    }

    m_workflows[workflow.id()] = workflow;

    qCDebug(orchestratorLog) << "Registered workflow:" << workflow.id();

    return make_success();
}

qtplugin::expected<void, PluginError> PluginOrchestrator::unregister_workflow(
    const QString& workflow_id) {
    std::unique_lock lock(m_workflows_mutex);

    auto it = m_workflows.find(workflow_id);
    if (it == m_workflows.end()) {
        return make_error<void>(
            PluginErrorCode::PluginNotFound,
            "Workflow not found: " + workflow_id.toStdString());
    }

    m_workflows.erase(it);

    qCDebug(orchestratorLog) << "Unregistered workflow:" << workflow_id;

    return make_success();
}

qtplugin::expected<Workflow, PluginError> PluginOrchestrator::get_workflow(
    const QString& workflow_id) const {
    std::shared_lock lock(m_workflows_mutex);

    auto it = m_workflows.find(workflow_id);
    if (it == m_workflows.end()) {
        return make_error<Workflow>(
            PluginErrorCode::PluginNotFound,
            "Workflow not found: " + workflow_id.toStdString());
    }

    return it->second;
}

std::vector<QString> PluginOrchestrator::list_workflows() const {
    std::shared_lock lock(m_workflows_mutex);
    std::vector<QString> workflows;
    workflows.reserve(m_workflows.size());

    for (const auto& [workflow_id, _] : m_workflows) {
        workflows.push_back(workflow_id);
    }

    return workflows;
}

QString PluginOrchestrator::generate_execution_id() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

qtplugin::expected<QString, PluginError> PluginOrchestrator::execute_workflow(
    const QString& workflow_id, const QJsonObject& initial_data, bool async) {
    // Get the workflow
    auto workflow_result = get_workflow(workflow_id);
    if (!workflow_result) {
        return qtplugin::unexpected<PluginError>(workflow_result.error());
    }

    const Workflow& workflow = workflow_result.value();
    QString execution_id = generate_execution_id();

    // Create execution context
    auto context = std::make_unique<WorkflowContext>(workflow_id, execution_id);
    context->initial_data = initial_data;
    context->shared_data = initial_data;

    // Create execution state
    auto state = std::make_unique<ExecutionState>();
    state->context = std::move(context);
    state->workflow = workflow;
    state->execution_order = workflow.get_execution_order();

    // Setup timeout timer
    state->timeout_timer = std::make_unique<QTimer>(this);
    state->timeout_timer->setSingleShot(true);
    state->timeout_timer->setInterval(
        static_cast<int>(workflow.timeout().count()));
    connect(state->timeout_timer.get(), &QTimer::timeout, this,
            &PluginOrchestrator::on_execution_timeout);

    // Store execution state
    {
        std::unique_lock lock(m_executions_mutex);
        m_active_executions[execution_id] = std::move(state);
    }

    if (async) {
        // Execute asynchronously - simplified approach to avoid deadlocks
        auto& exec_state = m_active_executions[execution_id];
        exec_state->execution_future =
            std::async(std::launch::async, [this, execution_id]() {
                // Execute the workflow implementation directly
                qtplugin::expected<QJsonObject, PluginError> result = make_error<QJsonObject>(PluginErrorCode::ExecutionFailed, "Execution not found");

                // Get execution state with minimal lock time
                {
                    std::shared_lock lock(m_executions_mutex);
                    auto exec_it = m_active_executions.find(execution_id);
                    if (exec_it != m_active_executions.end()) {
                        // Execute directly with the original context - this is safe because
                        // the context is only accessed by this thread during execution
                        result = execute_workflow_impl(
                            exec_it->second->workflow, *exec_it->second->context);
                    }
                } // Release lock immediately after getting the result

                // Emit signals without holding any locks
                if (result) {
                    emit workflow_completed(execution_id, result.value());
                } else {
                    emit workflow_failed(
                        execution_id,
                        QString::fromStdString(result.error().message));
                }

                // Cleanup with minimal lock time
                {
                    std::unique_lock lock(m_executions_mutex);
                    m_active_executions.erase(execution_id);
                }
            });
    } else {
        // Execute synchronously
        auto& exec_state = m_active_executions[execution_id];
        auto result =
            execute_workflow_impl(exec_state->workflow, *exec_state->context);

        // Cleanup
        {
            std::unique_lock lock(m_executions_mutex);
            m_active_executions.erase(execution_id);
        }

        if (!result) {
            return qtplugin::unexpected<PluginError>(result.error());
        }
    }

    emit workflow_started(execution_id, workflow_id);

    return execution_id;
}

std::future<qtplugin::expected<QJsonObject, PluginError>>
PluginOrchestrator::execute_workflow_async(const QString& workflow_id,
                                           const QJsonObject& initial_data) {
    return std::async(
        std::launch::async,
        [this, workflow_id,
         initial_data]() -> qtplugin::expected<QJsonObject, PluginError> {
            auto execution_result =
                execute_workflow(workflow_id, initial_data, false);
            if (!execution_result) {
                return qtplugin::unexpected<PluginError>(
                    execution_result.error());
            }

            // Since we executed synchronously, we need to get the result
            // In a real implementation, this would be handled differently
            return QJsonObject{{"execution_id", execution_result.value()}};
        });
}

qtplugin::expected<void, PluginError> PluginOrchestrator::cancel_workflow(
    const QString& execution_id) {
    {
        std::unique_lock lock(m_executions_mutex);

        auto it = m_active_executions.find(execution_id);
        if (it == m_active_executions.end()) {
            return make_error<void>(
                PluginErrorCode::PluginNotFound,
                "Execution not found: " + execution_id.toStdString());
        }

        it->second->context->cancelled = true;
    } // Release mutex before emitting signal

    emit workflow_cancelled(execution_id);

    qCDebug(orchestratorLog) << "Cancelled workflow execution:" << execution_id;

    return make_success();
}

qtplugin::expected<QJsonObject, PluginError>
PluginOrchestrator::get_execution_status(const QString& execution_id) const {
    std::shared_lock lock(m_executions_mutex);

    auto it = m_active_executions.find(execution_id);
    if (it == m_active_executions.end()) {
        return make_error<QJsonObject>(
            PluginErrorCode::PluginNotFound,
            "Execution not found: " + execution_id.toStdString());
    }

    const auto& state = it->second;
    const auto& context = state->context;

    QJsonObject status;
    status["execution_id"] = execution_id;
    status["workflow_id"] = context->workflow_id;
    status["running"] = state->running.load();
    status["cancelled"] = context->cancelled.load();
    status["current_step"] = static_cast<int>(state->current_step_index);
    status["total_steps"] = static_cast<int>(state->execution_order.size());

    // Calculate progress
    if (!state->execution_order.empty()) {
        double progress = static_cast<double>(state->current_step_index) /
                          state->execution_order.size() * 100.0;
        status["progress"] = progress;
    }

    // Add step results
    QJsonArray step_results;
    for (const auto& [step_id, result] : context->step_results) {
        QJsonObject step_status;
        step_status["step_id"] = step_id;
        step_status["status"] = static_cast<int>(result.status);
        step_status["execution_time"] =
            static_cast<int>(result.execution_time().count());
        step_status["retry_count"] = result.retry_count;
        if (!result.error_message.isEmpty()) {
            step_status["error"] = result.error_message;
        }
        step_results.append(step_status);
    }
    status["step_results"] = step_results;

    return status;
}

std::vector<QString> PluginOrchestrator::list_active_executions() const {
    std::shared_lock lock(m_executions_mutex);
    std::vector<QString> executions;
    executions.reserve(m_active_executions.size());

    for (const auto& [execution_id, _] : m_active_executions) {
        executions.push_back(execution_id);
    }

    return executions;
}

qtplugin::expected<QJsonObject, PluginError>
PluginOrchestrator::execute_workflow_impl(const Workflow& workflow,
                                          WorkflowContext& context) {
    qCDebug(orchestratorLog)
        << "Starting workflow execution:" << context.execution_id;

    auto execution_order = workflow.get_execution_order();
    if (execution_order.empty()) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidConfiguration,
                                       "No executable steps in workflow");
    }

    // Execute steps in order
    for (const QString& step_id : execution_order) {
        if (context.cancelled) {
            qCDebug(orchestratorLog)
                << "Workflow execution cancelled:" << context.execution_id;
            return make_error<QJsonObject>(PluginErrorCode::OperationCancelled,
                                           "Workflow cancelled");
        }

        const WorkflowStep* step = workflow.get_step(step_id);
        if (!step) {
            continue;  // Skip missing steps
        }

        // Check step dependencies
        if (!check_step_dependencies(*step, context)) {
            qCWarning(orchestratorLog)
                << "Step dependencies not satisfied:" << step_id;
            continue;
        }

        // Check step condition
        if (step->condition && !step->condition(context.shared_data)) {
            qCDebug(orchestratorLog)
                << "Step condition not met, skipping:" << step_id;

            StepResult result;
            result.step_id = step_id;
            result.status = StepStatus::Skipped;
            result.start_time = std::chrono::system_clock::now();
            result.end_time = result.start_time;
            context.step_results[step_id] = result;

            continue;
        }

        emit step_started(context.execution_id, step_id);

        // Execute the step
        auto step_result = execute_step(*step, context);

        if (step_result) {
            context.step_results[step_id] = step_result.value();

            // Merge step result into shared data
            if (step_result.value().status == StepStatus::Completed) {
                QJsonObject merged_data = context.shared_data;
                QJsonObject step_data = step_result.value().result_data;

                for (auto it = step_data.begin(); it != step_data.end(); ++it) {
                    merged_data[it.key()] = it.value();
                }

                context.shared_data = merged_data;

                emit step_completed(context.execution_id, step_id,
                                    step_result.value().result_data);
            } else if (step_result.value().status == StepStatus::Failed) {
                emit step_failed(context.execution_id, step_id,
                                 step_result.value().error_message);

                if (step->critical) {
                    qCWarning(orchestratorLog)
                        << "Critical step failed, stopping workflow:"
                        << step_id;
                    return make_error<QJsonObject>(
                        PluginErrorCode::ExecutionFailed,
                        "Critical step failed: " + step_id.toStdString());
                }
            }
        } else {
            emit step_failed(
                context.execution_id, step_id,
                QString::fromStdString(step_result.error().message));

            if (step->critical) {
                qCWarning(orchestratorLog)
                    << "Critical step failed, stopping workflow:" << step_id;
                return qtplugin::unexpected<PluginError>(step_result.error());
            }
        }
    }

    qCDebug(orchestratorLog)
        << "Workflow execution completed:" << context.execution_id;

    // Return final shared data
    return context.shared_data;
}

qtplugin::expected<StepResult, PluginError> PluginOrchestrator::execute_step(
    const WorkflowStep& step, WorkflowContext& context) {
    StepResult result;
    result.step_id = step.id;
    result.status = StepStatus::Running;
    result.start_time = std::chrono::system_clock::now();

    qCDebug(orchestratorLog)
        << "Executing step:" << step.id << "in plugin:" << step.plugin_id;

    // Note: In a real implementation, PluginManager would be injected
    // For now, we'll return an error indicating the plugin manager is not
    // available
    result.status = StepStatus::Failed;
    result.error_message =
        "Plugin manager not available - dependency injection needed";
    result.end_time = std::chrono::system_clock::now();
    return result;
}

bool PluginOrchestrator::check_step_dependencies(
    const WorkflowStep& step, const WorkflowContext& context) const {
    for (const QString& dep_id : step.dependencies) {
        auto it = context.step_results.find(dep_id);
        if (it == context.step_results.end() ||
            it->second.status != StepStatus::Completed) {
            return false;
        }
    }
    return true;
}

QJsonObject PluginOrchestrator::merge_step_data(
    const QJsonObject& shared_data, const QJsonObject& step_params) const {
    QJsonObject merged = shared_data;

    for (auto it = step_params.begin(); it != step_params.end(); ++it) {
        merged[it.key()] = it.value();
    }

    return merged;
}

void PluginOrchestrator::on_execution_timeout() {
    QTimer* timer = qobject_cast<QTimer*>(sender());
    if (!timer)
        return;

    // Find the execution associated with this timer
    std::unique_lock lock(m_executions_mutex);
    for (auto& [execution_id, state] : m_active_executions) {
        if (state->timeout_timer.get() == timer) {
            state->context->cancelled = true;
            emit workflow_failed(execution_id, "Workflow execution timeout");
            qCWarning(orchestratorLog)
                << "Workflow execution timeout:" << execution_id;
            break;
        }
    }
}

}  // namespace qtplugin::orchestration

#include "plugin_orchestrator.moc"
