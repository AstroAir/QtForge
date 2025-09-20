/**
 * @file progress_monitoring.hpp
 * @brief Comprehensive workflow progress monitoring APIs
 * @version 3.1.0
 */

#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "../utils/error_handling.hpp"
#include "progress_tracking.hpp"
#include "progress_message_bus.hpp"

namespace qtplugin::workflow::progress {

/**
 * @brief Progress monitoring filter criteria
 */
struct ProgressMonitoringFilter {
    // Execution filtering
    std::vector<QString> execution_ids;
    std::vector<QString> workflow_ids;
    std::vector<QString> workflow_names;
    
    // Event type filtering
    std::vector<WorkflowProgressEventType> event_types;
    
    // Progress range filtering
    std::optional<double> min_progress;
    std::optional<double> max_progress;
    
    // Time-based filtering
    std::optional<QDateTime> start_time;
    std::optional<QDateTime> end_time;
    
    // Step filtering
    std::vector<QString> step_ids;
    std::vector<state::StepExecutionState> step_states;
    
    // Metadata filtering
    QJsonObject required_metadata;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<ProgressMonitoringFilter, PluginError> from_json(const QJsonObject& json);
    
    // Filter evaluation
    bool matches(const WorkflowProgressData& progress_data) const;
    bool matches_step(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state) const;
};

/**
 * @brief Progress monitoring callback interface
 */
class IProgressMonitoringCallback {
public:
    virtual ~IProgressMonitoringCallback() = default;
    
    // Workflow events
    virtual void on_workflow_event(const WorkflowProgressData& progress_data) = 0;
    virtual void on_step_event(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state) = 0;
    virtual void on_aggregation_event(const ProgressAggregationData& aggregation_data) = 0;
    
    // Error handling
    virtual void on_monitoring_error(const PluginError& error) = 0;
};

/**
 * @brief Function-based progress monitoring callback
 */
class FunctionProgressMonitoringCallback : public IProgressMonitoringCallback {
public:
    using WorkflowEventHandler = std::function<void(const WorkflowProgressData&)>;
    using StepEventHandler = std::function<void(const QString&, const QString&, const state::WorkflowStepState&)>;
    using AggregationEventHandler = std::function<void(const ProgressAggregationData&)>;
    using ErrorHandler = std::function<void(const PluginError&)>;
    
    FunctionProgressMonitoringCallback(
        WorkflowEventHandler workflow_handler = nullptr,
        StepEventHandler step_handler = nullptr,
        AggregationEventHandler aggregation_handler = nullptr,
        ErrorHandler error_handler = nullptr);
    
    // IProgressMonitoringCallback implementation
    void on_workflow_event(const WorkflowProgressData& progress_data) override;
    void on_step_event(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state) override;
    void on_aggregation_event(const ProgressAggregationData& aggregation_data) override;
    void on_monitoring_error(const PluginError& error) override;
    
    // Handler management
    void set_workflow_handler(WorkflowEventHandler handler) { m_workflow_handler = std::move(handler); }
    void set_step_handler(StepEventHandler handler) { m_step_handler = std::move(handler); }
    void set_aggregation_handler(AggregationEventHandler handler) { m_aggregation_handler = std::move(handler); }
    void set_error_handler(ErrorHandler handler) { m_error_handler = std::move(handler); }

private:
    WorkflowEventHandler m_workflow_handler;
    StepEventHandler m_step_handler;
    AggregationEventHandler m_aggregation_handler;
    ErrorHandler m_error_handler;
};

/**
 * @brief Progress monitoring subscription
 */
class ProgressMonitoringSubscription {
public:
    ProgressMonitoringSubscription(
        const QString& subscription_id,
        const ProgressMonitoringFilter& filter,
        std::shared_ptr<IProgressMonitoringCallback> callback);
    
    ~ProgressMonitoringSubscription();
    
    // Subscription properties
    const QString& id() const { return m_subscription_id; }
    const ProgressMonitoringFilter& filter() const { return m_filter; }
    std::shared_ptr<IProgressMonitoringCallback> callback() const { return m_callback; }
    
    // State management
    bool is_active() const { return m_active; }
    void set_active(bool active) { m_active = active; }
    
    QDateTime created_time() const { return m_created_time; }
    QDateTime last_event_time() const { return m_last_event_time; }
    size_t event_count() const { return m_event_count; }
    
    // Event processing
    void process_workflow_event(const WorkflowProgressData& progress_data);
    void process_step_event(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state);
    void process_aggregation_event(const ProgressAggregationData& aggregation_data);
    
    // Filter management
    void update_filter(const ProgressMonitoringFilter& new_filter);

private:
    QString m_subscription_id;
    ProgressMonitoringFilter m_filter;
    std::shared_ptr<IProgressMonitoringCallback> m_callback;
    
    bool m_active{true};
    QDateTime m_created_time;
    QDateTime m_last_event_time;
    size_t m_event_count{0};
};

/**
 * @brief Comprehensive progress monitoring manager
 */
class ProgressMonitoringManager : public QObject, public IWorkflowProgressMonitor {
    Q_OBJECT

public:
    explicit ProgressMonitoringManager(QObject* parent = nullptr);
    ~ProgressMonitoringManager() override;
    
    // Service lifecycle
    qtplugin::expected<void, PluginError> initialize();
    void shutdown();
    bool is_initialized() const { return m_initialized; }
    
    // Subscription management
    qtplugin::expected<QString, PluginError> subscribe(
        const ProgressMonitoringFilter& filter,
        std::shared_ptr<IProgressMonitoringCallback> callback);
    
    qtplugin::expected<QString, PluginError> subscribe_with_functions(
        const ProgressMonitoringFilter& filter,
        FunctionProgressMonitoringCallback::WorkflowEventHandler workflow_handler = nullptr,
        FunctionProgressMonitoringCallback::StepEventHandler step_handler = nullptr,
        FunctionProgressMonitoringCallback::AggregationEventHandler aggregation_handler = nullptr,
        FunctionProgressMonitoringCallback::ErrorHandler error_handler = nullptr);
    
    qtplugin::expected<void, PluginError> unsubscribe(const QString& subscription_id);
    void unsubscribe_all();
    
    // Subscription management
    std::vector<QString> get_subscription_ids() const;
    std::optional<ProgressMonitoringSubscription*> get_subscription(const QString& subscription_id) const;
    size_t subscription_count() const;
    
    // Subscription control
    qtplugin::expected<void, PluginError> activate_subscription(const QString& subscription_id);
    qtplugin::expected<void, PluginError> deactivate_subscription(const QString& subscription_id);
    qtplugin::expected<void, PluginError> update_subscription_filter(const QString& subscription_id, const ProgressMonitoringFilter& new_filter);
    
    // Statistics
    size_t total_events_processed() const { return m_total_events_processed; }
    size_t workflow_events_processed() const { return m_workflow_events_processed; }
    size_t step_events_processed() const { return m_step_events_processed; }
    size_t aggregation_events_processed() const { return m_aggregation_events_processed; }
    
    // IWorkflowProgressMonitor implementation
    void on_workflow_started(const WorkflowProgressData& progress_data) override;
    void on_workflow_completed(const WorkflowProgressData& progress_data) override;
    void on_workflow_failed(const WorkflowProgressData& progress_data) override;
    void on_workflow_cancelled(const WorkflowProgressData& progress_data) override;
    void on_workflow_suspended(const WorkflowProgressData& progress_data) override;
    void on_workflow_resumed(const WorkflowProgressData& progress_data) override;
    void on_step_started(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state) override;
    void on_step_completed(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state) override;
    void on_step_failed(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state) override;
    void on_step_skipped(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state) override;
    void on_progress_updated(const WorkflowProgressData& progress_data) override;
    void on_aggregation_updated(const ProgressAggregationData& aggregation_data) override;
    
    // Singleton access
    static ProgressMonitoringManager& instance();

signals:
    void subscription_added(const QString& subscription_id);
    void subscription_removed(const QString& subscription_id);
    void subscription_activated(const QString& subscription_id);
    void subscription_deactivated(const QString& subscription_id);
    void event_processed(const QString& event_type, size_t subscription_count);

private:
    bool m_initialized{false};
    std::unordered_map<QString, std::unique_ptr<ProgressMonitoringSubscription>> m_subscriptions;
    
    // Statistics
    size_t m_total_events_processed{0};
    size_t m_workflow_events_processed{0};
    size_t m_step_events_processed{0};
    size_t m_aggregation_events_processed{0};
    
    // Helper methods
    QString generate_subscription_id() const;
    void process_workflow_event_internal(const WorkflowProgressData& progress_data);
    void process_step_event_internal(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state);
    void process_aggregation_event_internal(const ProgressAggregationData& aggregation_data);
};

} // namespace qtplugin::workflow::progress
