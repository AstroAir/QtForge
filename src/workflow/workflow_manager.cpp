/**
 * @file workflow_manager.cpp
 * @brief Implementation of unified workflow manager
 * @version 3.1.0
 */

#include "qtplugin/workflow/workflow.hpp"
#include <QLoggingCategory>
#include <QUuid>
#include <mutex>
#include <shared_mutex>

Q_LOGGING_CATEGORY(workflowManagerLog, "qtplugin.workflow.manager")

namespace qtplugin::workflow::integration {

// === WorkflowManager Implementation ===

WorkflowManager& WorkflowManager::instance() {
    static WorkflowManager manager;
    return manager;
}

composition::CompositionManager& WorkflowManager::composition_manager() {
    return composition::CompositionManager::instance();
}

orchestration::PluginOrchestrator& WorkflowManager::orchestrator() {
    // Thread-safe singleton using C++11 magic statics
    static orchestration::PluginOrchestrator orchestrator_instance;
    return orchestrator_instance;
}

transactions::PluginTransactionManager& WorkflowManager::transaction_manager() {
    return transactions::PluginTransactionManager::instance();
}

qtplugin::expected<QString, PluginError> WorkflowManager::execute_transactional_workflow(
    const QString& workflow_id,
    const QJsonObject& initial_data,
    transactions::IsolationLevel isolation) {

    qCDebug(workflowManagerLog) << "Executing transactional workflow:" << workflow_id
                                << "isolation:" << static_cast<int>(isolation);

    // Begin transaction
    auto transaction_result = transaction_manager().begin_transaction(isolation);
    if (!transaction_result) {
        return qtplugin::unexpected<PluginError>(transaction_result.error());
    }

    QString transaction_id = transaction_result.value();

    try {
        // Execute workflow within transaction
        auto execution_result = orchestrator().execute_workflow(workflow_id, initial_data, false);
        if (!execution_result) {
            // Rollback transaction on workflow failure
            auto rollback_result = transaction_manager().rollback_transaction(transaction_id);
            if (!rollback_result) {
                qCWarning(workflowManagerLog) << "Failed to rollback transaction:" << transaction_id
                                              << "after workflow failure";
            }
            return qtplugin::unexpected<PluginError>(execution_result.error());
        }

        // Commit transaction on success
        auto commit_result = transaction_manager().commit_transaction(transaction_id);
        if (!commit_result) {
            qCWarning(workflowManagerLog) << "Failed to commit transaction:" << transaction_id;
            return qtplugin::unexpected<PluginError>(commit_result.error());
        }

        emit unified_workflow_completed(execution_result.value(), initial_data);

        qCDebug(workflowManagerLog) << "Transactional workflow completed successfully:"
                                    << workflow_id << "execution:" << execution_result.value();

        return execution_result.value();

    } catch (const std::exception& e) {
        // Rollback transaction on exception
        auto rollback_result = transaction_manager().rollback_transaction(transaction_id);
        if (!rollback_result) {
            qCWarning(workflowManagerLog) << "Failed to rollback transaction:" << transaction_id
                                          << "after exception";
        }

        QString error_msg = QString("Transactional workflow execution failed: %1").arg(e.what());
        emit unified_workflow_failed(workflow_id, error_msg);

        return make_error<QString>(PluginErrorCode::ExecutionFailed, error_msg.toStdString());
    }
}

qtplugin::expected<QString, PluginError> WorkflowManager::execute_composite_workflow(
    const QString& composition_id,
    const QString& workflow_id,
    const QJsonObject& initial_data) {

    qCDebug(workflowManagerLog) << "Executing composite workflow:" << workflow_id
                                << "composition:" << composition_id;

    // Get the composition
    auto composition_result = composition_manager().get_composition(composition_id);
    if (!composition_result) {
        return qtplugin::unexpected<PluginError>(composition_result.error());
    }

    // Create composite plugin instance
    auto composite_plugin = std::make_shared<composition::CompositePlugin>(
        composition_result.value());

    // Initialize composite plugin
    auto init_result = composite_plugin->initialize();
    if (!init_result) {
        return qtplugin::unexpected<PluginError>(init_result.error());
    }

    try {
        // Execute workflow using composite plugin context
        auto execution_result = orchestrator().execute_workflow(workflow_id, initial_data, false);
        if (!execution_result) {
            return qtplugin::unexpected<PluginError>(execution_result.error());
        }

        emit unified_workflow_completed(execution_result.value(), initial_data);

        qCDebug(workflowManagerLog) << "Composite workflow completed successfully:"
                                    << workflow_id << "execution:" << execution_result.value();

        return execution_result.value();

    } catch (const std::exception& e) {
        QString error_msg = QString("Composite workflow execution failed: %1").arg(e.what());
        emit unified_workflow_failed(workflow_id, error_msg);

        return make_error<QString>(PluginErrorCode::ExecutionFailed, error_msg.toStdString());
    }
}

} // namespace qtplugin::workflow::integration
