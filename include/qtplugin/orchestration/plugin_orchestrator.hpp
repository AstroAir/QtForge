/**
 * @file plugin_orchestrator.hpp
 * @brief Plugin orchestration framework for coordinating complex workflows
 * @version 3.1.0
 * @author QtPlugin Development Team
 * 
 * This file defines the orchestration framework that allows coordinating
 * complex multi-plugin workflows with execution pipelines, dependency
 * ordering, rollback capabilities, and transaction management.
 */

#pragma once

#include "../core/plugin_interface.hpp"
#include "../core/enhanced_plugin_interface.hpp"
#include "../utils/error_handling.hpp"
#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QTimer>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <future>
#include <atomic>
#include <shared_mutex>
#include <mutex>

namespace qtplugin::orchestration {

/**
 * @brief Workflow step execution status
 */
enum class StepStatus {
    Pending,        // Step is waiting to be executed
    Running,        // Step is currently executing
    Completed,      // Step completed successfully
    Failed,         // Step failed with error
    Skipped,        // Step was skipped due to conditions
    Cancelled,      // Step was cancelled
    Retrying        // Step is being retried after failure
};

/**
 * @brief Workflow execution mode
 */
enum class ExecutionMode {
    Sequential,     // Execute steps one by one
    Parallel,       // Execute independent steps in parallel
    Conditional,    // Execute based on conditions
    Pipeline        // Execute as a pipeline with data flow
};

/**
 * @brief Workflow step definition
 */
struct WorkflowStep {
    QString id;                                    // Unique step identifier
    QString name;                                  // Human-readable name
    QString description;                           // Step description
    QString plugin_id;                             // Plugin that executes this step
    QString service_name;                          // Service to call (if applicable)
    QString method_name;                           // Method to call
    QJsonObject parameters;                        // Step parameters
    std::vector<QString> dependencies;             // Step dependencies (other step IDs)
    std::function<bool(const QJsonObject&)> condition; // Execution condition
    std::chrono::milliseconds timeout{std::chrono::milliseconds{60000}}; // Step timeout
    int max_retries{0};                           // Maximum retry attempts
    std::chrono::milliseconds retry_delay{std::chrono::milliseconds{1000}}; // Delay between retries
    bool critical{true};                          // Whether failure should stop workflow
    QJsonObject metadata;                         // Additional metadata
    
    WorkflowStep() = default;
    WorkflowStep(const QString& step_id, const QString& plugin, const QString& method)
        : id(step_id), plugin_id(plugin), method_name(method) {}
};

/**
 * @brief Workflow step execution result
 */
struct StepResult {
    QString step_id;
    StepStatus status;
    QJsonObject result_data;
    QString error_message;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    int retry_count{0};
    
    std::chrono::milliseconds execution_time() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }
};

/**
 * @brief Workflow definition
 */
class Workflow {
public:
    Workflow(const QString& workflow_id, const QString& name = "")
        : m_id(workflow_id), m_name(name.isEmpty() ? workflow_id : name) {}
    
    // === Workflow Configuration ===
    
    Workflow& set_description(const QString& desc) {
        m_description = desc;
        return *this;
    }
    
    Workflow& set_execution_mode(ExecutionMode mode) {
        m_execution_mode = mode;
        return *this;
    }
    
    Workflow& set_timeout(std::chrono::milliseconds timeout) {
        m_timeout = timeout;
        return *this;
    }
    
    Workflow& add_step(const WorkflowStep& step) {
        m_steps[step.id] = step;
        return *this;
    }
    
    Workflow& add_rollback_step(const QString& step_id, const WorkflowStep& rollback_step) {
        m_rollback_steps[step_id] = rollback_step;
        return *this;
    }
    
    Workflow& set_global_condition(std::function<bool(const QJsonObject&)> condition) {
        m_global_condition = std::move(condition);
        return *this;
    }
    
    // === Workflow Access ===
    
    const QString& id() const noexcept { return m_id; }
    const QString& name() const noexcept { return m_name; }
    const QString& description() const noexcept { return m_description; }
    ExecutionMode execution_mode() const noexcept { return m_execution_mode; }
    std::chrono::milliseconds timeout() const noexcept { return m_timeout; }
    
    const std::unordered_map<QString, WorkflowStep>& steps() const noexcept { return m_steps; }
    const std::unordered_map<QString, WorkflowStep>& rollback_steps() const noexcept { return m_rollback_steps; }
    
    bool has_step(const QString& step_id) const {
        return m_steps.find(step_id) != m_steps.end();
    }
    
    const WorkflowStep* get_step(const QString& step_id) const {
        auto it = m_steps.find(step_id);
        return it != m_steps.end() ? &it->second : nullptr;
    }
    
    // === Validation ===
    
    qtplugin::expected<void, PluginError> validate() const;
    std::vector<QString> get_execution_order() const;
    
    // === Serialization ===
    
    QJsonObject to_json() const;
    static qtplugin::expected<Workflow, PluginError> from_json(const QJsonObject& json);
    
private:
    QString m_id;
    QString m_name;
    QString m_description;
    ExecutionMode m_execution_mode{ExecutionMode::Sequential};
    std::chrono::milliseconds m_timeout{std::chrono::milliseconds{300000}}; // 5 minutes default
    std::unordered_map<QString, WorkflowStep> m_steps;
    std::unordered_map<QString, WorkflowStep> m_rollback_steps;
    std::function<bool(const QJsonObject&)> m_global_condition;
};

/**
 * @brief Workflow execution context
 */
struct WorkflowContext {
    QString workflow_id;
    QString execution_id;
    QJsonObject initial_data;
    QJsonObject shared_data;                      // Data shared between steps
    std::unordered_map<QString, StepResult> step_results;
    std::chrono::system_clock::time_point start_time;
    std::atomic<bool> cancelled{false};
    QString transaction_id;                       // For transactional workflows
    
    WorkflowContext(const QString& wf_id, const QString& exec_id)
        : workflow_id(wf_id), execution_id(exec_id) {
        start_time = std::chrono::system_clock::now();
    }
};

/**
 * @brief Plugin orchestrator for managing workflow execution
 */
class PluginOrchestrator : public QObject {
    Q_OBJECT
    
public:
    explicit PluginOrchestrator(QObject* parent = nullptr);
    ~PluginOrchestrator() override;
    
    // === Workflow Management ===
    
    qtplugin::expected<void, PluginError> register_workflow(const Workflow& workflow);
    qtplugin::expected<void, PluginError> unregister_workflow(const QString& workflow_id);
    
    qtplugin::expected<Workflow, PluginError> get_workflow(const QString& workflow_id) const;
    std::vector<QString> list_workflows() const;
    
    // === Workflow Execution ===
    
    qtplugin::expected<QString, PluginError> execute_workflow(
        const QString& workflow_id,
        const QJsonObject& initial_data = {},
        bool async = false);
    
    std::future<qtplugin::expected<QJsonObject, PluginError>> execute_workflow_async(
        const QString& workflow_id,
        const QJsonObject& initial_data = {});
    
    qtplugin::expected<void, PluginError> cancel_workflow(const QString& execution_id);
    
    // === Execution Monitoring ===
    
    qtplugin::expected<QJsonObject, PluginError> get_execution_status(const QString& execution_id) const;
    std::vector<QString> list_active_executions() const;
    
    qtplugin::expected<std::vector<StepResult>, PluginError> get_step_results(
        const QString& execution_id) const;
    
    // === Transaction Support ===
    
    qtplugin::expected<void, PluginError> begin_transaction(const QString& transaction_id);
    qtplugin::expected<void, PluginError> commit_transaction(const QString& transaction_id);
    qtplugin::expected<void, PluginError> rollback_transaction(const QString& transaction_id);
    
signals:
    void workflow_started(const QString& execution_id, const QString& workflow_id);
    void workflow_completed(const QString& execution_id, const QJsonObject& result);
    void workflow_failed(const QString& execution_id, const QString& error);
    void workflow_cancelled(const QString& execution_id);
    void step_started(const QString& execution_id, const QString& step_id);
    void step_completed(const QString& execution_id, const QString& step_id, const QJsonObject& result);
    void step_failed(const QString& execution_id, const QString& step_id, const QString& error);
    
private slots:
    void on_execution_timeout();
    
private:
    struct ExecutionState {
        std::unique_ptr<WorkflowContext> context;
        Workflow workflow;
        std::vector<QString> execution_order;
        size_t current_step_index{0};
        std::atomic<bool> running{false};
        std::unique_ptr<QTimer> timeout_timer;
        std::future<void> execution_future;

        ExecutionState() = default;
        ExecutionState(const Workflow& wf) : workflow(wf) {}
    };
    
    mutable std::shared_mutex m_workflows_mutex;
    mutable std::shared_mutex m_executions_mutex;
    std::unordered_map<QString, Workflow> m_workflows;
    std::unordered_map<QString, std::unique_ptr<ExecutionState>> m_active_executions;
    std::unordered_map<QString, std::vector<QString>> m_transactions; // transaction_id -> execution_ids
    
    // Execution methods
    qtplugin::expected<QJsonObject, PluginError> execute_workflow_impl(
        const Workflow& workflow,
        WorkflowContext& context);
    
    qtplugin::expected<StepResult, PluginError> execute_step(
        const WorkflowStep& step,
        WorkflowContext& context);
    
    qtplugin::expected<void, PluginError> rollback_workflow(
        const Workflow& workflow,
        WorkflowContext& context);
    
    QString generate_execution_id() const;
    bool check_step_dependencies(const WorkflowStep& step, const WorkflowContext& context) const;
    QJsonObject merge_step_data(const QJsonObject& shared_data, const QJsonObject& step_params) const;
};

} // namespace qtplugin::orchestration

Q_DECLARE_METATYPE(qtplugin::orchestration::StepStatus)
Q_DECLARE_METATYPE(qtplugin::orchestration::ExecutionMode)
Q_DECLARE_METATYPE(qtplugin::orchestration::WorkflowStep)
Q_DECLARE_METATYPE(qtplugin::orchestration::StepResult)
