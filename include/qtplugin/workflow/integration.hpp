/**
 * @file integration.hpp
 * @brief Integration layer for unified workflow functionality
 * @version 3.1.0
 * @author QtPlugin Development Team
 *
 * This file defines the integration layer that provides unified access
 * to composition, orchestration, and transaction management capabilities
 * through a single, cohesive interface.
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <memory>
#include <vector>
#include <optional>
#include <unordered_map>

#include "../interfaces/core/plugin_interface.hpp"
#include "../utils/error_handling.hpp"
#include "workflow_types.hpp"
#include "composition.hpp"
#include "orchestration.hpp"
#include "transactions.hpp"

namespace qtplugin::workflow::integration {

/**
 * @brief Unified workflow manager that coordinates all workflow functionality
 *
 * This class provides thread-safe access to all workflow subsystems including
 * composition, orchestration, and transaction management. All public methods
 * are thread-safe and can be called from multiple threads concurrently.
 *
 * The singleton instance uses C++11 magic statics for thread-safe initialization.
 */
class WorkflowManager : public QObject {
    Q_OBJECT

public:
    static WorkflowManager& instance();

    // Access to individual managers
    composition::CompositionManager& composition_manager();
    orchestration::PluginOrchestrator& orchestrator();
    transactions::PluginTransactionManager& transaction_manager();

    // Unified workflow operations
    qtplugin::expected<QString, PluginError> execute_transactional_workflow(
        const QString& workflow_id,
        const QJsonObject& initial_data = {},
        transactions::IsolationLevel isolation = transactions::IsolationLevel::ReadCommitted
    );

    qtplugin::expected<QString, PluginError> execute_composite_workflow(
        const QString& composition_id,
        const QString& workflow_id,
        const QJsonObject& initial_data = {}
    );

signals:
    void unified_workflow_started(const QString& execution_id);
    void unified_workflow_completed(const QString& execution_id, const QJsonObject& result);
    void unified_workflow_failed(const QString& execution_id, const QString& error);

private:
    WorkflowManager() = default;
};

/**
 * @brief Unified workflow that combines composition, orchestration, and transaction capabilities
 */
class UnifiedWorkflow {
public:
    UnifiedWorkflow(const QString& id, const QString& name = "");

    // Configuration
    UnifiedWorkflow& set_composition(const composition::PluginComposition& comp);
    UnifiedWorkflow& add_workflow_step(const orchestration::WorkflowStep& step);
    UnifiedWorkflow& set_transactional(bool transactional = true);
    UnifiedWorkflow& set_isolation_level(transactions::IsolationLevel level);

    // Execution
    qtplugin::expected<QJsonObject, PluginError> execute(
        const QJsonObject& initial_data = {}
    );

    // Workflow information
    const QString& id() const { return m_id; }
    const QString& name() const { return m_name; }
    bool is_transactional() const { return m_transactional; }
    transactions::IsolationLevel isolation_level() const { return m_isolation_level; }
    size_t step_count() const { return m_workflow_steps.size(); }
    bool has_composition() const { return m_composition.has_value(); }

    // Validation
    qtplugin::expected<void, PluginError> validate() const;

private:
    QString m_id;
    QString m_name;
    bool m_transactional{false};
    transactions::IsolationLevel m_isolation_level{transactions::IsolationLevel::ReadCommitted};

    // Optional composition
    std::optional<composition::PluginComposition> m_composition;

    // Workflow steps
    std::unordered_map<QString, orchestration::WorkflowStep> m_workflow_steps;
};

/**
 * @brief Transactional composition that ensures atomic operations across component plugins
 */
class TransactionalComposition : public composition::CompositePlugin {
    Q_OBJECT

public:
    TransactionalComposition(const composition::PluginComposition& composition,
                           transactions::IsolationLevel isolation = transactions::IsolationLevel::ReadCommitted,
                           QObject* parent = nullptr);

    // Override execute_command to be transactional
    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override;

    // Transaction management
    transactions::IsolationLevel isolation_level() const { return m_isolation_level; }
    void set_isolation_level(transactions::IsolationLevel level) { m_isolation_level = level; }

    // Batch operations
    qtplugin::expected<QJsonObject, PluginError> execute_batch_commands(
        const std::vector<std::pair<std::string, QJsonObject>>& commands);

private:
    transactions::IsolationLevel m_isolation_level;
};

} // namespace qtplugin::workflow::integration
