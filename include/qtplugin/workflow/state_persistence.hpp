/**
 * @file state_persistence.hpp
 * @brief Workflow state persistence and serialization framework
 * @version 3.1.0
 */

#pragma once

#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QString>
#include <QObject>
#include <QTimer>
#include <memory>
#include <optional>
#include <unordered_map>
#include <chrono>

#include "../utils/error_handling.hpp"
#include "workflow_types.hpp"

namespace qtplugin::workflow::state {

/**
 * @brief Execution state of a workflow step
 */
enum class StepExecutionState {
    Pending = 0,
    Running = 1,
    Completed = 2,
    Failed = 3,
    Skipped = 4,
    Cancelled = 5
};

/**
 * @brief Execution state of a workflow
 */
enum class WorkflowExecutionState {
    Created = 0,
    Running = 1,
    Completed = 2,
    Failed = 3,
    Cancelled = 4,
    Suspended = 5
};

/**
 * @brief Serializable workflow step state
 */
struct WorkflowStepState {
    QString step_id;
    StepExecutionState state{StepExecutionState::Pending};
    QJsonObject input_data;
    QJsonObject output_data;
    QJsonObject error_data;
    QDateTime start_time;
    QDateTime end_time;
    int retry_count{0};
    QJsonObject metadata;

    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<WorkflowStepState, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Serializable workflow execution context
 */
struct WorkflowExecutionContext {
    QString execution_id;
    QString workflow_id;
    QString workflow_name;
    WorkflowExecutionState state{WorkflowExecutionState::Created};
    QJsonObject initial_data;
    QJsonObject final_result;
    QJsonObject error_data;
    QDateTime start_time;
    QDateTime end_time;
    QString current_step_id;
    std::unordered_map<QString, WorkflowStepState> step_states;
    QJsonObject execution_metadata;
    
    // Transaction information
    bool is_transactional{false};
    QString transaction_id;
    
    // Composition information
    bool is_composite{false};
    QString composition_id;

    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<WorkflowExecutionContext, PluginError> from_json(const QJsonObject& json);
    
    // State management
    void update_step_state(const QString& step_id, const WorkflowStepState& state);
    std::optional<WorkflowStepState> get_step_state(const QString& step_id) const;
    double calculate_progress() const;
};

/**
 * @brief Workflow checkpoint for state persistence
 */
struct WorkflowCheckpoint {
    QString checkpoint_id;
    QString execution_id;
    QDateTime timestamp;
    WorkflowExecutionContext context;
    QJsonObject checkpoint_metadata;

    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<WorkflowCheckpoint, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Configuration for state persistence
 */
struct StatePersistenceConfig {
    bool enabled{true};
    std::chrono::milliseconds checkpoint_interval{std::chrono::seconds(30)};
    size_t max_checkpoints_per_workflow{10};
    QString storage_directory{"./workflow_state"};
    bool compress_checkpoints{false};
    bool encrypt_checkpoints{false};
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<StatePersistenceConfig, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Interface for workflow state storage backends
 */
class IWorkflowStateStorage {
public:
    virtual ~IWorkflowStateStorage() = default;

    // Checkpoint management
    virtual qtplugin::expected<void, PluginError> save_checkpoint(
        const WorkflowCheckpoint& checkpoint) = 0;
    
    virtual qtplugin::expected<WorkflowCheckpoint, PluginError> load_checkpoint(
        const QString& checkpoint_id) = 0;
    
    virtual qtplugin::expected<std::vector<WorkflowCheckpoint>, PluginError> list_checkpoints(
        const QString& execution_id) = 0;
    
    virtual qtplugin::expected<void, PluginError> delete_checkpoint(
        const QString& checkpoint_id) = 0;

    // Execution context management
    virtual qtplugin::expected<void, PluginError> save_execution_context(
        const WorkflowExecutionContext& context) = 0;
    
    virtual qtplugin::expected<WorkflowExecutionContext, PluginError> load_execution_context(
        const QString& execution_id) = 0;
    
    virtual qtplugin::expected<void, PluginError> delete_execution_context(
        const QString& execution_id) = 0;

    // Cleanup operations
    virtual qtplugin::expected<void, PluginError> cleanup_old_checkpoints(
        std::chrono::hours max_age = std::chrono::hours(24)) = 0;
};

/**
 * @brief File-based workflow state storage implementation
 */
class FileWorkflowStateStorage : public IWorkflowStateStorage {
public:
    explicit FileWorkflowStateStorage(const QString& base_directory);
    
    // IWorkflowStateStorage implementation
    qtplugin::expected<void, PluginError> save_checkpoint(
        const WorkflowCheckpoint& checkpoint) override;
    
    qtplugin::expected<WorkflowCheckpoint, PluginError> load_checkpoint(
        const QString& checkpoint_id) override;
    
    qtplugin::expected<std::vector<WorkflowCheckpoint>, PluginError> list_checkpoints(
        const QString& execution_id) override;
    
    qtplugin::expected<void, PluginError> delete_checkpoint(
        const QString& checkpoint_id) override;

    qtplugin::expected<void, PluginError> save_execution_context(
        const WorkflowExecutionContext& context) override;
    
    qtplugin::expected<WorkflowExecutionContext, PluginError> load_execution_context(
        const QString& execution_id) override;
    
    qtplugin::expected<void, PluginError> delete_execution_context(
        const QString& execution_id) override;

    qtplugin::expected<void, PluginError> cleanup_old_checkpoints(
        std::chrono::hours max_age = std::chrono::hours(24)) override;

private:
    QString m_base_directory;
    
    QString get_checkpoint_path(const QString& checkpoint_id) const;
    QString get_execution_context_path(const QString& execution_id) const;
    QString get_execution_directory(const QString& execution_id) const;
    
    qtplugin::expected<void, PluginError> ensure_directory_exists(const QString& path) const;
    qtplugin::expected<QJsonObject, PluginError> load_json_file(const QString& file_path) const;
    qtplugin::expected<void, PluginError> save_json_file(const QString& file_path, const QJsonObject& json) const;
};

/**
 * @brief Workflow checkpoint manager for automated state persistence
 */
class WorkflowCheckpointManager : public QObject {
    Q_OBJECT

public:
    explicit WorkflowCheckpointManager(
        std::unique_ptr<IWorkflowStateStorage> storage,
        const StatePersistenceConfig& config = {},
        QObject* parent = nullptr);

    // Configuration
    void set_config(const StatePersistenceConfig& config);
    const StatePersistenceConfig& config() const { return m_config; }

    // Checkpoint operations
    qtplugin::expected<QString, PluginError> create_checkpoint(
        const WorkflowExecutionContext& context,
        const QJsonObject& metadata = {});

    qtplugin::expected<WorkflowCheckpoint, PluginError> load_checkpoint(
        const QString& checkpoint_id);

    qtplugin::expected<std::vector<WorkflowCheckpoint>, PluginError> list_checkpoints(
        const QString& execution_id);

    qtplugin::expected<void, PluginError> delete_checkpoint(
        const QString& checkpoint_id);

    // Execution context management
    qtplugin::expected<void, PluginError> save_execution_context(
        const WorkflowExecutionContext& context);

    qtplugin::expected<WorkflowExecutionContext, PluginError> load_execution_context(
        const QString& execution_id);

    qtplugin::expected<void, PluginError> delete_execution_context(
        const QString& execution_id);

    // Automatic checkpoint management
    void start_automatic_checkpointing(const QString& execution_id);
    void stop_automatic_checkpointing(const QString& execution_id);
    void update_execution_context(const WorkflowExecutionContext& context);

    // Cleanup operations
    qtplugin::expected<void, PluginError> cleanup_old_checkpoints(
        std::chrono::hours max_age = std::chrono::hours(24));

signals:
    void checkpoint_created(const QString& checkpoint_id, const QString& execution_id);
    void checkpoint_failed(const QString& execution_id, const QString& error);
    void execution_context_saved(const QString& execution_id);
    void execution_context_failed(const QString& execution_id, const QString& error);

private slots:
    void on_checkpoint_timer();

private:
    std::unique_ptr<IWorkflowStateStorage> m_storage;
    StatePersistenceConfig m_config;

    // Automatic checkpointing
    std::unordered_map<QString, std::unique_ptr<QTimer>> m_checkpoint_timers;
    std::unordered_map<QString, WorkflowExecutionContext> m_active_contexts;

    QString generate_checkpoint_id(const QString& execution_id) const;
    void cleanup_checkpoint_timer(const QString& execution_id);
};

/**
 * @brief Recovery strategy for workflow restoration
 */
enum class RecoveryStrategy {
    RestoreFromLatest = 0,      // Restore from latest checkpoint
    RestoreFromSpecific = 1,    // Restore from specific checkpoint
    RestoreFromBest = 2,        // Restore from best available checkpoint (latest successful)
    RestartFromBeginning = 3    // Restart workflow from beginning
};

/**
 * @brief Recovery options for workflow restoration
 */
struct WorkflowRecoveryOptions {
    RecoveryStrategy strategy{RecoveryStrategy::RestoreFromLatest};
    QString specific_checkpoint_id;
    bool validate_checkpoint{true};
    bool resume_execution{true};
    QJsonObject recovery_metadata;

    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<WorkflowRecoveryOptions, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Result of workflow recovery operation
 */
struct WorkflowRecoveryResult {
    bool success{false};
    QString execution_id;
    QString checkpoint_id;
    WorkflowExecutionContext restored_context;
    QJsonObject recovery_metadata;
    QString error_message;

    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<WorkflowRecoveryResult, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Workflow recovery manager for restoring and resuming workflows
 */
class WorkflowRecoveryManager : public QObject {
    Q_OBJECT

public:
    explicit WorkflowRecoveryManager(
        WorkflowCheckpointManager* checkpoint_manager,
        QObject* parent = nullptr);

    // Recovery operations
    qtplugin::expected<WorkflowRecoveryResult, PluginError> recover_workflow(
        const QString& execution_id,
        const WorkflowRecoveryOptions& options = {});

    qtplugin::expected<WorkflowRecoveryResult, PluginError> recover_from_checkpoint(
        const QString& checkpoint_id,
        const WorkflowRecoveryOptions& options = {});

    // Recovery validation
    qtplugin::expected<bool, PluginError> validate_checkpoint_for_recovery(
        const WorkflowCheckpoint& checkpoint);

    qtplugin::expected<std::vector<WorkflowCheckpoint>, PluginError> find_recoverable_checkpoints(
        const QString& execution_id);

    // Recovery utilities
    qtplugin::expected<WorkflowCheckpoint, PluginError> find_best_checkpoint(
        const QString& execution_id);

    qtplugin::expected<WorkflowExecutionContext, PluginError> prepare_recovery_context(
        const WorkflowCheckpoint& checkpoint,
        const WorkflowRecoveryOptions& options);

signals:
    void recovery_started(const QString& execution_id, const QString& checkpoint_id);
    void recovery_completed(const WorkflowRecoveryResult& result);
    void recovery_failed(const QString& execution_id, const QString& error);

private:
    WorkflowCheckpointManager* m_checkpoint_manager;

    qtplugin::expected<WorkflowCheckpoint, PluginError> select_checkpoint_by_strategy(
        const QString& execution_id,
        const WorkflowRecoveryOptions& options);

    bool is_checkpoint_valid_for_recovery(const WorkflowCheckpoint& checkpoint);
    WorkflowExecutionContext create_recovery_context(
        const WorkflowCheckpoint& checkpoint,
        const WorkflowRecoveryOptions& options);
};

/**
 * @brief Workflow state persistence configuration manager
 *
 * Integrates with QtForge's configuration system to manage workflow
 * state persistence settings across different scopes.
 */
class WorkflowStatePersistenceConfigManager : public QObject {
    Q_OBJECT

public:
    explicit WorkflowStatePersistenceConfigManager(QObject* parent = nullptr);

    // Configuration management
    qtplugin::expected<void, PluginError> load_config();
    qtplugin::expected<void, PluginError> save_config();
    qtplugin::expected<void, PluginError> reset_to_defaults();

    // Configuration access
    const StatePersistenceConfig& get_config() const { return m_config; }
    void set_config(const StatePersistenceConfig& config);

    // Specific setting access
    bool is_enabled() const { return m_config.enabled; }
    void set_enabled(bool enabled);

    std::chrono::milliseconds checkpoint_interval() const { return m_config.checkpoint_interval; }
    void set_checkpoint_interval(std::chrono::milliseconds interval);

    size_t max_checkpoints_per_workflow() const { return m_config.max_checkpoints_per_workflow; }
    void set_max_checkpoints_per_workflow(size_t max_checkpoints);

    QString storage_directory() const { return m_config.storage_directory; }
    void set_storage_directory(const QString& directory);

    bool compress_checkpoints() const { return m_config.compress_checkpoints; }
    void set_compress_checkpoints(bool compress);

    bool encrypt_checkpoints() const { return m_config.encrypt_checkpoints; }
    void set_encrypt_checkpoints(bool encrypt);

    // Configuration validation
    qtplugin::expected<void, PluginError> validate_config() const;

signals:
    void config_changed(const StatePersistenceConfig& new_config);
    void config_loaded();
    void config_saved();

private:
    StatePersistenceConfig m_config;

    static StatePersistenceConfig create_default_config();
    qtplugin::expected<void, PluginError> ensure_storage_directory() const;
};

} // namespace qtplugin::workflow::state
