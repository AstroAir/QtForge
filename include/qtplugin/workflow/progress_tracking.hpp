/**
 * @file progress_tracking.hpp
 * @brief Workflow progress tracking and event system
 * @version 3.1.0
 */

#pragma once

#include <QJsonObject>
#include <QDateTime>
#include <QString>
#include <QObject>
#include <QTimer>
#include <memory>
#include <unordered_map>
#include <chrono>

#include "../utils/error_handling.hpp"
#include "../communication/message_types.hpp"
#include "../communication/message_bus.hpp"
#include "state_persistence.hpp"

namespace qtplugin::workflow::progress {

/**
 * @brief Workflow progress event types
 */
enum class WorkflowProgressEventType {
    WorkflowStarted = 0,
    WorkflowCompleted = 1,
    WorkflowFailed = 2,
    WorkflowCancelled = 3,
    WorkflowSuspended = 4,
    WorkflowResumed = 5,
    StepStarted = 10,
    StepCompleted = 11,
    StepFailed = 12,
    StepSkipped = 13,
    StepRetrying = 14,
    ProgressUpdate = 20,
    CheckpointCreated = 30,
    CheckpointRestored = 31
};

/**
 * @brief Workflow progress data structure
 */
struct WorkflowProgressData {
    QString execution_id;
    QString workflow_id;
    QString workflow_name;
    WorkflowProgressEventType event_type;
    
    // Progress information
    double overall_progress{0.0};  // 0.0 to 100.0
    int completed_steps{0};
    int total_steps{0};
    QString current_step_id;
    QString current_step_name;
    
    // Timing information
    QDateTime start_time;
    QDateTime current_time;
    QDateTime estimated_completion_time;
    std::chrono::milliseconds elapsed_time{0};
    std::chrono::milliseconds estimated_remaining_time{0};
    
    // Additional data
    QJsonObject metadata;
    QJsonObject step_data;
    QString error_message;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<WorkflowProgressData, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Workflow progress message for message bus communication
 */
class WorkflowProgressMessage : public Message<WorkflowProgressMessage> {
public:
    WorkflowProgressMessage(std::string_view sender, const WorkflowProgressData& progress_data)
        : Message(sender, MessagePriority::Normal), m_progress_data(progress_data) {}

    std::string_view type() const noexcept override { return "WorkflowProgress"; }
    
    const WorkflowProgressData& progress_data() const noexcept { return m_progress_data; }

    QJsonObject to_json() const override {
        auto json = messages::detail::create_base_json("workflow_progress", sender(), timestamp());
        json["progress_data"] = m_progress_data.to_json();
        return json;
    }

private:
    WorkflowProgressData m_progress_data;
};

/**
 * @brief Workflow step progress message for detailed step tracking
 */
class WorkflowStepProgressMessage : public Message<WorkflowStepProgressMessage> {
public:
    WorkflowStepProgressMessage(std::string_view sender, 
                               const QString& execution_id,
                               const QString& step_id,
                               const state::WorkflowStepState& step_state)
        : Message(sender, MessagePriority::Normal), 
          m_execution_id(execution_id), 
          m_step_id(step_id), 
          m_step_state(step_state) {}

    std::string_view type() const noexcept override { return "WorkflowStepProgress"; }
    
    const QString& execution_id() const noexcept { return m_execution_id; }
    const QString& step_id() const noexcept { return m_step_id; }
    const state::WorkflowStepState& step_state() const noexcept { return m_step_state; }

    QJsonObject to_json() const override {
        auto json = messages::detail::create_base_json("workflow_step_progress", sender(), timestamp());
        json["execution_id"] = m_execution_id;
        json["step_id"] = m_step_id;
        json["step_state"] = m_step_state.to_json();
        return json;
    }

private:
    QString m_execution_id;
    QString m_step_id;
    state::WorkflowStepState m_step_state;
};

/**
 * @brief Progress tracking configuration
 */
struct ProgressTrackingConfig {
    bool enabled{true};
    bool publish_workflow_events{true};
    bool publish_step_events{true};
    bool publish_progress_updates{true};
    std::chrono::milliseconds progress_update_interval{std::chrono::seconds(5)};
    bool include_metadata{true};
    bool include_step_data{false};
    
    // Message bus configuration
    QString message_bus_topic_prefix{"workflow.progress"};
    MessagePriority default_message_priority{MessagePriority::Normal};
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<ProgressTrackingConfig, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Progress aggregation data for multiple workflows
 */
struct ProgressAggregationData {
    int active_workflows{0};
    int completed_workflows{0};
    int failed_workflows{0};
    int cancelled_workflows{0};
    double average_progress{0.0};
    std::chrono::milliseconds total_execution_time{0};
    std::chrono::milliseconds average_execution_time{0};
    QDateTime last_update_time;
    
    // Per-workflow data
    std::unordered_map<QString, WorkflowProgressData> workflow_progress;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<ProgressAggregationData, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Workflow progress tracker for individual workflow execution
 */
class WorkflowProgressTracker : public QObject {
    Q_OBJECT

public:
    explicit WorkflowProgressTracker(
        const QString& execution_id,
        const QString& workflow_id,
        const QString& workflow_name = "",
        QObject* parent = nullptr);

    // Progress tracking
    void start_tracking();
    void stop_tracking();
    void update_progress(const WorkflowProgressData& progress_data);
    void update_step_progress(const QString& step_id, const state::WorkflowStepState& step_state);

    // Event reporting
    void report_workflow_started();
    void report_workflow_completed(const QJsonObject& result = {});
    void report_workflow_failed(const QString& error_message);
    void report_workflow_cancelled();
    void report_workflow_suspended();
    void report_workflow_resumed();
    
    void report_step_started(const QString& step_id, const QString& step_name = "");
    void report_step_completed(const QString& step_id, const QJsonObject& result = {});
    void report_step_failed(const QString& step_id, const QString& error_message);
    void report_step_skipped(const QString& step_id, const QString& reason = "");
    void report_step_retrying(const QString& step_id, int retry_count);

    void report_checkpoint_created(const QString& checkpoint_id);
    void report_checkpoint_restored(const QString& checkpoint_id);

    // Progress calculation
    void calculate_and_update_progress();
    double calculate_overall_progress() const;
    std::chrono::milliseconds estimate_remaining_time() const;

    // Configuration
    void set_config(const ProgressTrackingConfig& config);
    const ProgressTrackingConfig& config() const { return m_config; }

    // Current state access
    const WorkflowProgressData& current_progress() const { return m_current_progress; }
    const QString& execution_id() const { return m_execution_id; }

signals:
    void progress_updated(const WorkflowProgressData& progress_data);
    void step_progress_updated(const QString& step_id, const state::WorkflowStepState& step_state);

private slots:
    void on_progress_update_timer();

private:
    QString m_execution_id;
    QString m_workflow_id;
    QString m_workflow_name;
    ProgressTrackingConfig m_config;
    
    WorkflowProgressData m_current_progress;
    std::unordered_map<QString, state::WorkflowStepState> m_step_states;
    
    std::unique_ptr<QTimer> m_progress_timer;
    bool m_tracking_active{false};
    
    void publish_progress_message(const WorkflowProgressData& progress_data);
    void publish_step_progress_message(const QString& step_id, const state::WorkflowStepState& step_state);
    void update_timing_information();
    QString generate_message_topic(const QString& event_type) const;
};

/**
 * @brief Workflow progress aggregator for collecting and consolidating progress from multiple workflows
 */
class WorkflowProgressAggregator : public QObject {
    Q_OBJECT

public:
    explicit WorkflowProgressAggregator(QObject* parent = nullptr);

    // Aggregation management
    void add_workflow_tracker(const QString& execution_id, WorkflowProgressTracker* tracker);
    void remove_workflow_tracker(const QString& execution_id);
    void clear_all_trackers();

    // Progress aggregation
    ProgressAggregationData get_aggregated_progress() const;
    void update_aggregation();

    // Configuration
    void set_config(const ProgressTrackingConfig& config);
    const ProgressTrackingConfig& config() const { return m_config; }

    // Statistics
    int active_workflow_count() const;
    int total_workflow_count() const;
    double average_progress() const;
    std::chrono::milliseconds total_execution_time() const;
    std::chrono::milliseconds average_execution_time() const;

    // Filtering and querying
    std::vector<QString> get_active_executions() const;
    std::vector<QString> get_completed_executions() const;
    std::vector<QString> get_failed_executions() const;

    std::optional<WorkflowProgressData> get_workflow_progress(const QString& execution_id) const;
    std::vector<WorkflowProgressData> get_workflows_by_status(WorkflowProgressEventType status) const;

signals:
    void aggregation_updated(const ProgressAggregationData& aggregation_data);
    void workflow_added(const QString& execution_id);
    void workflow_removed(const QString& execution_id);
    void workflow_status_changed(const QString& execution_id, WorkflowProgressEventType old_status, WorkflowProgressEventType new_status);

private slots:
    void on_workflow_progress_updated(const WorkflowProgressData& progress_data);
    void on_aggregation_update_timer();

private:
    ProgressTrackingConfig m_config;
    ProgressAggregationData m_aggregation_data;

    std::unordered_map<QString, WorkflowProgressTracker*> m_workflow_trackers;
    std::unordered_map<QString, WorkflowProgressData> m_last_progress_data;

    std::unique_ptr<QTimer> m_aggregation_timer;

    void calculate_aggregation_statistics();
    void update_workflow_counts();
    void publish_aggregation_message();
};

/**
 * @brief Progress monitoring interface for subscribing to workflow progress events
 */
class IWorkflowProgressMonitor {
public:
    virtual ~IWorkflowProgressMonitor() = default;

    // Progress event callbacks
    virtual void on_workflow_started(const WorkflowProgressData& progress_data) = 0;
    virtual void on_workflow_completed(const WorkflowProgressData& progress_data) = 0;
    virtual void on_workflow_failed(const WorkflowProgressData& progress_data) = 0;
    virtual void on_workflow_cancelled(const WorkflowProgressData& progress_data) = 0;
    virtual void on_workflow_suspended(const WorkflowProgressData& progress_data) = 0;
    virtual void on_workflow_resumed(const WorkflowProgressData& progress_data) = 0;

    virtual void on_step_started(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state) = 0;
    virtual void on_step_completed(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state) = 0;
    virtual void on_step_failed(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state) = 0;
    virtual void on_step_skipped(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state) = 0;

    virtual void on_progress_updated(const WorkflowProgressData& progress_data) = 0;
    virtual void on_aggregation_updated(const ProgressAggregationData& aggregation_data) = 0;
};

/**
 * @brief Progress monitoring manager for managing progress monitors and message bus subscriptions
 */
class WorkflowProgressMonitorManager : public QObject {
    Q_OBJECT

public:
    explicit WorkflowProgressMonitorManager(QObject* parent = nullptr);

    // Monitor management
    void add_monitor(const QString& monitor_id, IWorkflowProgressMonitor* monitor);
    void remove_monitor(const QString& monitor_id);
    void clear_monitors();

    // Message bus integration
    void subscribe_to_progress_messages();
    void unsubscribe_from_progress_messages();

    // Filtering
    void set_execution_filter(const std::vector<QString>& execution_ids);
    void set_workflow_filter(const std::vector<QString>& workflow_ids);
    void set_event_type_filter(const std::vector<WorkflowProgressEventType>& event_types);
    void clear_filters();

signals:
    void monitor_added(const QString& monitor_id);
    void monitor_removed(const QString& monitor_id);

private slots:
    void on_progress_message_received(const WorkflowProgressMessage& message);
    void on_step_progress_message_received(const WorkflowStepProgressMessage& message);

private:
    std::unordered_map<QString, IWorkflowProgressMonitor*> m_monitors;

    // Filters
    std::vector<QString> m_execution_filter;
    std::vector<QString> m_workflow_filter;
    std::vector<WorkflowProgressEventType> m_event_type_filter;

    bool m_subscribed{false};

    bool passes_filters(const WorkflowProgressData& progress_data) const;
    void notify_monitors_workflow_event(const WorkflowProgressData& progress_data);
    void notify_monitors_step_event(const QString& execution_id, const QString& step_id, const state::WorkflowStepState& step_state);
};

} // namespace qtplugin::workflow::progress
