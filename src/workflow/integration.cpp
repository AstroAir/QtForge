/**
 * @file integration.cpp
 * @brief Implementation of workflow integration functionality
 * @version 3.1.0
 */

#include "qtplugin/workflow/integration.hpp"
#include <QLoggingCategory>
#include <QUuid>

Q_LOGGING_CATEGORY(workflowIntegrationLog, "qtplugin.workflow.integration")

namespace qtplugin::workflow::integration {

// === UnifiedWorkflow Implementation ===

UnifiedWorkflow::UnifiedWorkflow(const QString& id, const QString& name)
    : m_id(id), m_name(name.isEmpty() ? id : name) {
    qCDebug(workflowIntegrationLog) << "Created unified workflow:" << m_id;
}

UnifiedWorkflow& UnifiedWorkflow::set_composition(const composition::PluginComposition& comp) {
    m_composition = comp;
    qCDebug(workflowIntegrationLog) << "Set composition for workflow:" << m_id
                                    << "with" << comp.plugins().size() << "plugins";
    return *this;
}

UnifiedWorkflow& UnifiedWorkflow::add_workflow_step(const orchestration::WorkflowStep& step) {
    m_workflow_steps[step.id] = step;
    qCDebug(workflowIntegrationLog) << "Added workflow step:" << step.id << "to workflow:" << m_id;
    return *this;
}

UnifiedWorkflow& UnifiedWorkflow::set_transactional(bool transactional) {
    m_transactional = transactional;
    qCDebug(workflowIntegrationLog) << "Set transactional mode:" << transactional << "for workflow:" << m_id;
    return *this;
}

UnifiedWorkflow& UnifiedWorkflow::set_isolation_level(transactions::IsolationLevel level) {
    m_isolation_level = level;
    qCDebug(workflowIntegrationLog) << "Set isolation level:" << static_cast<int>(level) << "for workflow:" << m_id;
    return *this;
}

qtplugin::expected<QJsonObject, PluginError> UnifiedWorkflow::execute(
    const QJsonObject& initial_data) {

    qCDebug(workflowIntegrationLog) << "Executing unified workflow:" << m_id;

    // Validate workflow before execution
    auto validation_result = validate();
    if (!validation_result) {
        return qtplugin::unexpected<PluginError>(validation_result.error());
    }

    try {
        // Create workflow for orchestration
        orchestration::Workflow workflow(m_id, m_name);

        // Add all workflow steps
        for (const auto& [step_id, step] : m_workflow_steps) {
            workflow.add_step(step);
        }

        // Register workflow with orchestrator
        auto& orchestrator = WorkflowManager::instance().orchestrator();
        auto register_result = orchestrator.register_workflow(workflow);
        if (!register_result) {
            return qtplugin::unexpected<PluginError>(register_result.error());
        }

        QJsonObject result;

        // Handle composition if present
        if (m_composition.has_value()) {
            if (m_transactional) {
                // Execute as composite transactional workflow
                auto execution_result = WorkflowManager::instance().execute_composite_workflow(
                    m_composition->id(), m_id, initial_data);

                if (!execution_result) {
                    return qtplugin::unexpected<PluginError>(execution_result.error());
                }

                result["execution_id"] = execution_result.value();
                result["composite"] = true;
                result["composition_id"] = m_composition->id();
            } else {
                // Execute as composite workflow
                auto execution_result = WorkflowManager::instance().execute_composite_workflow(
                    m_composition->id(), m_id, initial_data);

                if (!execution_result) {
                    return qtplugin::unexpected<PluginError>(execution_result.error());
                }

                result["execution_id"] = execution_result.value();
                result["composite"] = true;
                result["composition_id"] = m_composition->id();
            }
        } else {
            if (m_transactional) {
                // Execute as transactional workflow
                auto execution_result = WorkflowManager::instance().execute_transactional_workflow(
                    m_id, initial_data, m_isolation_level);

                if (!execution_result) {
                    return qtplugin::unexpected<PluginError>(execution_result.error());
                }

                result["execution_id"] = execution_result.value();
                result["transactional"] = true;
                result["isolation_level"] = static_cast<int>(m_isolation_level);
            } else {
                // Execute as regular workflow
                auto execution_result = orchestrator.execute_workflow(m_id, initial_data, false);

                if (!execution_result) {
                    return qtplugin::unexpected<PluginError>(execution_result.error());
                }

                result["execution_id"] = execution_result.value();
                result["transactional"] = false;
            }
        }

        result["workflow_id"] = m_id;
        result["initial_data"] = initial_data;

        qCDebug(workflowIntegrationLog) << "Unified workflow executed successfully:" << m_id;

        return result;

    } catch (const std::exception& e) {
        QString error_msg = QString("Unified workflow execution failed: %1").arg(e.what());
        qCWarning(workflowIntegrationLog) << error_msg;

        return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed, error_msg.toStdString());
    }
}

qtplugin::expected<void, PluginError> UnifiedWorkflow::validate() const {
    // Validate workflow ID
    if (m_id.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters, "Workflow ID cannot be empty");
    }

    // Validate that we have at least one step
    if (m_workflow_steps.empty()) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration, "Workflow must have at least one step");
    }

    // Validate composition if present
    if (m_composition.has_value()) {
        auto composition_validation = m_composition->validate();
        if (!composition_validation) {
            return composition_validation;
        }
    }

    // Validate workflow steps
    for (const auto& [step_id, step] : m_workflow_steps) {
        if (step.id.isEmpty()) {
            return make_error<void>(PluginErrorCode::InvalidParameters,
                                    "Workflow step ID cannot be empty");
        }

        if (step.plugin_id.isEmpty()) {
            return make_error<void>(PluginErrorCode::InvalidParameters,
                                    "Workflow step plugin ID cannot be empty");
        }

        if (step.method_name.isEmpty()) {
            return make_error<void>(PluginErrorCode::InvalidParameters,
                                    "Workflow step method name cannot be empty");
        }
    }

    qCDebug(workflowIntegrationLog) << "Unified workflow validation passed:" << m_id;
    return make_success();
}

// === TransactionalComposition Implementation ===

TransactionalComposition::TransactionalComposition(
    const composition::PluginComposition& composition,
    transactions::IsolationLevel isolation,
    QObject* parent)
    : CompositePlugin(composition, parent), m_isolation_level(isolation) {

    qCDebug(workflowIntegrationLog) << "Created transactional composition with isolation level:"
                                    << static_cast<int>(isolation);
}

qtplugin::expected<QJsonObject, PluginError> TransactionalComposition::execute_command(
    std::string_view command, const QJsonObject& params) {

    qCDebug(workflowIntegrationLog) << "Executing transactional command:" << command.data();

    // Begin transaction
    auto& transaction_manager = transactions::PluginTransactionManager::instance();
    auto transaction_result = transaction_manager.begin_transaction(m_isolation_level);
    if (!transaction_result) {
        return qtplugin::unexpected<PluginError>(transaction_result.error());
    }

    QString transaction_id = transaction_result.value();

    try {
        // Execute command using parent implementation
        auto result = CompositePlugin::execute_command(command, params);

        if (result) {
            // Commit transaction on success
            auto commit_result = transaction_manager.commit_transaction(transaction_id);
            if (!commit_result) {
                qCWarning(workflowIntegrationLog) << "Failed to commit transaction:" << transaction_id;
                return qtplugin::unexpected<PluginError>(commit_result.error());
            }

            qCDebug(workflowIntegrationLog) << "Transactional command completed successfully:" << command.data();
            return result;
        } else {
            // Rollback transaction on failure
            auto rollback_result = transaction_manager.rollback_transaction(transaction_id);
            if (!rollback_result) {
                qCWarning(workflowIntegrationLog) << "Failed to rollback transaction:" << transaction_id;
            }

            return result;
        }

    } catch (const std::exception& e) {
        // Rollback transaction on exception
        auto rollback_result = transaction_manager.rollback_transaction(transaction_id);
        if (!rollback_result) {
            qCWarning(workflowIntegrationLog) << "Failed to rollback transaction:" << transaction_id
                                              << "after exception";
        }

        QString error_msg = QString("Transactional command execution failed: %1").arg(e.what());
        qCWarning(workflowIntegrationLog) << error_msg;

        return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed, error_msg.toStdString());
    }
}

qtplugin::expected<QJsonObject, PluginError> TransactionalComposition::execute_batch_commands(
    const std::vector<std::pair<std::string, QJsonObject>>& commands) {

    qCDebug(workflowIntegrationLog) << "Executing batch commands in transaction, count:" << commands.size();

    if (commands.empty()) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidParameters, "No commands to execute");
    }

    // Begin transaction
    auto& transaction_manager = transactions::PluginTransactionManager::instance();
    auto transaction_result = transaction_manager.begin_transaction(m_isolation_level);
    if (!transaction_result) {
        return qtplugin::unexpected<PluginError>(transaction_result.error());
    }

    QString transaction_id = transaction_result.value();
    QJsonObject batch_result;
    QJsonArray command_results;

    try {
        // Execute all commands in sequence
        for (size_t i = 0; i < commands.size(); ++i) {
            const auto& [command, params] = commands[i];

            qCDebug(workflowIntegrationLog) << "Executing batch command" << i + 1 << "of" << commands.size()
                                            << ":" << command.c_str();

            // Execute command using parent implementation (without transaction)
            auto result = CompositePlugin::execute_command(command, params);

            if (!result) {
                // Rollback transaction on any failure
                auto rollback_result = transaction_manager.rollback_transaction(transaction_id);
                if (!rollback_result) {
                    qCWarning(workflowIntegrationLog) << "Failed to rollback transaction:" << transaction_id;
                }

                qCWarning(workflowIntegrationLog) << "Batch command failed at index" << i << ":" << command.c_str();
                return qtplugin::unexpected<PluginError>(result.error());
            }

            // Add result to batch results
            QJsonObject command_result;
            command_result["command"] = QString::fromStdString(command);
            command_result["index"] = static_cast<int>(i);
            command_result["result"] = result.value();
            command_results.append(command_result);
        }

        // Commit transaction on success
        auto commit_result = transaction_manager.commit_transaction(transaction_id);
        if (!commit_result) {
            qCWarning(workflowIntegrationLog) << "Failed to commit batch transaction:" << transaction_id;
            return qtplugin::unexpected<PluginError>(commit_result.error());
        }

        batch_result["transaction_id"] = transaction_id;
        batch_result["commands_executed"] = static_cast<int>(commands.size());
        batch_result["results"] = command_results;
        batch_result["success"] = true;

        qCDebug(workflowIntegrationLog) << "Batch commands completed successfully, count:" << commands.size();
        return batch_result;

    } catch (const std::exception& e) {
        // Rollback transaction on exception
        auto rollback_result = transaction_manager.rollback_transaction(transaction_id);
        if (!rollback_result) {
            qCWarning(workflowIntegrationLog) << "Failed to rollback transaction:" << transaction_id
                                              << "after exception";
        }

        QString error_msg = QString("Batch command execution failed: %1").arg(e.what());
        qCWarning(workflowIntegrationLog) << error_msg;

        return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed, error_msg.toStdString());
    }
}

} // namespace qtplugin::workflow::integration
