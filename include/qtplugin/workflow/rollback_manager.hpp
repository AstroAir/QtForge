/**
 * @file rollback_manager.hpp
 * @brief Comprehensive workflow rollback mechanisms with error recovery
 * @version 3.1.0
 */

#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>

#include "../utils/error_handling.hpp"
#include "state_persistence.hpp"
#include "transactions.hpp"

namespace qtplugin::workflow::rollback {

/**
 * @brief Rollback strategy types
 */
enum class RollbackStrategy {
    FullRollback = 0,           // Rollback entire workflow
    PartialRollback = 1,        // Rollback from specific point
    SelectiveRollback = 2,      // Rollback specific operations only
    CompensatingRollback = 3,   // Use compensating actions
    SnapshotRollback = 4        // Rollback to specific snapshot
};

/**
 * @brief Rollback validation level
 */
enum class RollbackValidationLevel {
    None = 0,           // No validation
    Basic = 1,          // Basic consistency checks
    Comprehensive = 2,  // Full validation including dependencies
    Strict = 3          // Strict validation with external verification
};

/**
 * @brief Rollback operation result
 */
enum class RollbackOperationResult {
    Success = 0,
    PartialSuccess = 1,
    Failed = 2,
    Skipped = 3,
    CompensationApplied = 4
};

/**
 * @brief Individual rollback operation
 */
struct RollbackOperation {
    QString operation_id;
    QString step_id;
    QString plugin_id;
    QString method_name;
    QJsonObject rollback_data;
    QJsonObject original_data;
    
    // Rollback function
    std::function<qtplugin::expected<void, PluginError>()> rollback_func;
    
    // Compensation function (alternative to rollback)
    std::function<qtplugin::expected<void, PluginError>()> compensation_func;
    
    // Validation function
    std::function<qtplugin::expected<bool, PluginError>()> validation_func;
    
    // Metadata
    QDateTime created_time;
    QDateTime executed_time;
    int priority{0};
    bool critical{false};
    bool compensatable{true};
    
    // Dependencies
    std::vector<QString> depends_on;
    std::vector<QString> dependents;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<RollbackOperation, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Rollback plan configuration
 */
struct RollbackPlanConfig {
    RollbackStrategy strategy{RollbackStrategy::FullRollback};
    RollbackValidationLevel validation_level{RollbackValidationLevel::Basic};
    
    // Rollback scope
    QString execution_id;
    QString workflow_id;
    QString from_step_id;           // Start rollback from this step
    QString to_step_id;             // Rollback until this step
    std::vector<QString> include_operations;  // Specific operations to include
    std::vector<QString> exclude_operations;  // Operations to exclude
    
    // Timing and retry configuration
    std::chrono::milliseconds operation_timeout{std::chrono::seconds(30)};
    int max_retries{3};
    std::chrono::milliseconds retry_delay{std::chrono::seconds(1)};
    double backoff_multiplier{2.0};
    
    // Validation configuration
    bool validate_before_rollback{true};
    bool validate_after_rollback{true};
    bool continue_on_validation_failure{false};
    
    // Recovery configuration
    bool use_compensation_on_failure{true};
    bool create_rollback_checkpoint{true};
    bool preserve_partial_results{true};
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<RollbackPlanConfig, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Rollback execution result
 */
struct RollbackExecutionResult {
    QString rollback_id;
    QString execution_id;
    RollbackStrategy strategy;
    RollbackOperationResult overall_result;
    
    // Operation results
    std::vector<std::pair<QString, RollbackOperationResult>> operation_results;
    
    // Timing information
    QDateTime start_time;
    QDateTime end_time;
    std::chrono::milliseconds total_duration{0};
    
    // Statistics
    int total_operations{0};
    int successful_operations{0};
    int failed_operations{0};
    int skipped_operations{0};
    int compensated_operations{0};
    
    // Error information
    std::vector<PluginError> errors;
    QString error_summary;
    
    // Validation results
    bool pre_validation_passed{false};
    bool post_validation_passed{false};
    std::vector<QString> validation_warnings;
    
    // Recovery information
    QString recovery_checkpoint_id;
    QJsonObject recovery_metadata;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<RollbackExecutionResult, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Rollback plan that defines how to rollback a workflow
 */
class WorkflowRollbackPlan : public QObject {
    Q_OBJECT

public:
    explicit WorkflowRollbackPlan(const QString& plan_id, const RollbackPlanConfig& config, QObject* parent = nullptr);
    
    // Plan properties
    const QString& plan_id() const { return m_plan_id; }
    const RollbackPlanConfig& config() const { return m_config; }
    
    // Operation management
    void add_operation(const RollbackOperation& operation);
    void remove_operation(const QString& operation_id);
    void clear_operations();
    
    std::vector<RollbackOperation> get_operations() const;
    std::optional<RollbackOperation> get_operation(const QString& operation_id) const;
    size_t operation_count() const;
    
    // Plan validation
    qtplugin::expected<void, PluginError> validate_plan() const;
    qtplugin::expected<std::vector<QString>, PluginError> get_execution_order() const;
    
    // Plan optimization
    void optimize_plan();
    void sort_by_dependencies();
    void sort_by_priority();
    
    // Configuration updates
    void update_config(const RollbackPlanConfig& new_config);

signals:
    void operation_added(const QString& operation_id);
    void operation_removed(const QString& operation_id);
    void plan_validated();
    void plan_optimized();

private:
    QString m_plan_id;
    RollbackPlanConfig m_config;
    std::unordered_map<QString, RollbackOperation> m_operations;
    
    // Helper methods
    bool has_circular_dependencies() const;
    std::vector<QString> topological_sort() const;
};

/**
 * @brief Comprehensive workflow rollback manager
 */
class WorkflowRollbackManager : public QObject {
    Q_OBJECT

public:
    explicit WorkflowRollbackManager(QObject* parent = nullptr);
    ~WorkflowRollbackManager() override;
    
    // Service lifecycle
    qtplugin::expected<void, PluginError> initialize();
    void shutdown();
    bool is_initialized() const { return m_initialized; }
    
    // Plan management
    qtplugin::expected<QString, PluginError> create_rollback_plan(const RollbackPlanConfig& config);
    qtplugin::expected<void, PluginError> update_rollback_plan(const QString& plan_id, const RollbackPlanConfig& config);
    qtplugin::expected<void, PluginError> delete_rollback_plan(const QString& plan_id);
    
    std::optional<WorkflowRollbackPlan*> get_rollback_plan(const QString& plan_id) const;
    std::vector<QString> get_plan_ids() const;
    
    // Rollback execution
    qtplugin::expected<QString, PluginError> execute_rollback(const QString& plan_id);
    qtplugin::expected<QString, PluginError> execute_immediate_rollback(const RollbackPlanConfig& config);
    
    // Rollback monitoring
    qtplugin::expected<RollbackExecutionResult, PluginError> get_rollback_result(const QString& rollback_id) const;
    std::vector<QString> get_active_rollbacks() const;
    qtplugin::expected<void, PluginError> cancel_rollback(const QString& rollback_id);
    
    // Recovery operations
    qtplugin::expected<QString, PluginError> create_recovery_checkpoint(const QString& execution_id);
    qtplugin::expected<void, PluginError> restore_from_recovery_checkpoint(const QString& checkpoint_id);
    
    // Validation
    qtplugin::expected<void, PluginError> validate_rollback_feasibility(const QString& execution_id, const RollbackPlanConfig& config);
    qtplugin::expected<std::vector<QString>, PluginError> get_rollback_dependencies(const QString& execution_id);
    
    // Statistics
    size_t total_rollbacks_executed() const { return m_total_rollbacks; }
    size_t successful_rollbacks() const { return m_successful_rollbacks; }
    size_t failed_rollbacks() const { return m_failed_rollbacks; }
    
    // Singleton access
    static WorkflowRollbackManager& instance();

signals:
    void rollback_plan_created(const QString& plan_id);
    void rollback_plan_deleted(const QString& plan_id);
    void rollback_started(const QString& rollback_id, const QString& plan_id);
    void rollback_completed(const QString& rollback_id, RollbackOperationResult result);
    void rollback_operation_completed(const QString& rollback_id, const QString& operation_id, RollbackOperationResult result);
    void recovery_checkpoint_created(const QString& checkpoint_id);

private:
    bool m_initialized{false};
    std::unordered_map<QString, std::unique_ptr<WorkflowRollbackPlan>> m_rollback_plans;
    std::unordered_map<QString, RollbackExecutionResult> m_rollback_results;
    std::unordered_map<QString, QString> m_active_rollbacks; // rollback_id -> plan_id
    
    // Statistics
    size_t m_total_rollbacks{0};
    size_t m_successful_rollbacks{0};
    size_t m_failed_rollbacks{0};
    
    // Helper methods
    QString generate_plan_id() const;
    QString generate_rollback_id() const;
    
    qtplugin::expected<RollbackExecutionResult, PluginError> execute_rollback_plan(WorkflowRollbackPlan* plan);
    qtplugin::expected<void, PluginError> execute_rollback_operation(const RollbackOperation& operation, RollbackExecutionResult& result);
    qtplugin::expected<void, PluginError> validate_rollback_operation(const RollbackOperation& operation);
    
    // Integration with existing systems
    transactions::PluginTransactionManager* get_transaction_manager();
    state::WorkflowCheckpointManager* get_checkpoint_manager();
};

} // namespace qtplugin::workflow::rollback
