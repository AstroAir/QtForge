/**
 * @file progress_tracking.cpp
 * @brief Implementation of workflow progress tracking and event system
 * @version 3.1.0
 */

#include "qtplugin/workflow/progress_tracking.hpp"
#include <QLoggingCategory>
#include <QUuid>
#include <algorithm>
#include "qtplugin/communication/message_bus.hpp"

namespace {
Q_LOGGING_CATEGORY(workflowProgressLog, "qtplugin.workflow.progress")
}  // namespace

namespace qtplugin::workflow::progress {

// === WorkflowProgressData Implementation ===

QJsonObject WorkflowProgressData::to_json() const {
    QJsonObject json;
    json["execution_id"] = execution_id;
    json["workflow_id"] = workflow_id;
    json["workflow_name"] = workflow_name;
    json["event_type"] = static_cast<int>(event_type);
    json["overall_progress"] = overall_progress;
    json["completed_steps"] = completed_steps;
    json["total_steps"] = total_steps;
    json["current_step_id"] = current_step_id;
    json["current_step_name"] = current_step_name;
    json["start_time"] = start_time.toString(Qt::ISODate);
    json["current_time"] = current_time.toString(Qt::ISODate);
    json["estimated_completion_time"] =
        estimated_completion_time.toString(Qt::ISODate);
    json["elapsed_time_ms"] = static_cast<int>(elapsed_time.count());
    json["estimated_remaining_time_ms"] =
        static_cast<int>(estimated_remaining_time.count());
    json["metadata"] = metadata;
    json["step_data"] = step_data;
    json["error_message"] = error_message;
    return json;
}

qtplugin::expected<WorkflowProgressData, PluginError>
WorkflowProgressData::from_json(const QJsonObject& json) {
    WorkflowProgressData data;

    if (!json.contains("execution_id") || !json["execution_id"].isString()) {
        return make_error<WorkflowProgressData>(
            PluginErrorCode::InvalidFormat, "Missing or invalid execution_id");
    }
    data.execution_id = json["execution_id"].toString();

    if (!json.contains("workflow_id") || !json["workflow_id"].isString()) {
        return make_error<WorkflowProgressData>(
            PluginErrorCode::InvalidFormat, "Missing or invalid workflow_id");
    }
    data.workflow_id = json["workflow_id"].toString();

    if (json.contains("workflow_name") && json["workflow_name"].isString()) {
        data.workflow_name = json["workflow_name"].toString();
    }

    if (json.contains("event_type") && json["event_type"].isDouble()) {
        data.event_type =
            static_cast<WorkflowProgressEventType>(json["event_type"].toInt());
    }

    if (json.contains("overall_progress") &&
        json["overall_progress"].isDouble()) {
        data.overall_progress = json["overall_progress"].toDouble();
    }

    if (json.contains("completed_steps") &&
        json["completed_steps"].isDouble()) {
        data.completed_steps = json["completed_steps"].toInt();
    }

    if (json.contains("total_steps") && json["total_steps"].isDouble()) {
        data.total_steps = json["total_steps"].toInt();
    }

    if (json.contains("current_step_id") &&
        json["current_step_id"].isString()) {
        data.current_step_id = json["current_step_id"].toString();
    }

    if (json.contains("current_step_name") &&
        json["current_step_name"].isString()) {
        data.current_step_name = json["current_step_name"].toString();
    }

    if (json.contains("start_time") && json["start_time"].isString()) {
        data.start_time =
            QDateTime::fromString(json["start_time"].toString(), Qt::ISODate);
    }

    if (json.contains("current_time") && json["current_time"].isString()) {
        data.current_time =
            QDateTime::fromString(json["current_time"].toString(), Qt::ISODate);
    }

    if (json.contains("estimated_completion_time") &&
        json["estimated_completion_time"].isString()) {
        data.estimated_completion_time = QDateTime::fromString(
            json["estimated_completion_time"].toString(), Qt::ISODate);
    }

    if (json.contains("elapsed_time_ms") &&
        json["elapsed_time_ms"].isDouble()) {
        data.elapsed_time =
            std::chrono::milliseconds(json["elapsed_time_ms"].toInt());
    }

    if (json.contains("estimated_remaining_time_ms") &&
        json["estimated_remaining_time_ms"].isDouble()) {
        data.estimated_remaining_time = std::chrono::milliseconds(
            json["estimated_remaining_time_ms"].toInt());
    }

    if (json.contains("metadata") && json["metadata"].isObject()) {
        data.metadata = json["metadata"].toObject();
    }

    if (json.contains("step_data") && json["step_data"].isObject()) {
        data.step_data = json["step_data"].toObject();
    }

    if (json.contains("error_message") && json["error_message"].isString()) {
        data.error_message = json["error_message"].toString();
    }

    return data;
}

// === ProgressTrackingConfig Implementation ===

QJsonObject ProgressTrackingConfig::to_json() const {
    QJsonObject json;
    json["enabled"] = enabled;
    json["publish_workflow_events"] = publish_workflow_events;
    json["publish_step_events"] = publish_step_events;
    json["publish_progress_updates"] = publish_progress_updates;
    json["progress_update_interval_ms"] =
        static_cast<int>(progress_update_interval.count());
    json["include_metadata"] = include_metadata;
    json["include_step_data"] = include_step_data;
    json["message_bus_topic_prefix"] = message_bus_topic_prefix;
    json["default_message_priority"] =
        static_cast<int>(default_message_priority);
    return json;
}

qtplugin::expected<ProgressTrackingConfig, PluginError>
ProgressTrackingConfig::from_json(const QJsonObject& json) {
    ProgressTrackingConfig config;

    if (json.contains("enabled") && json["enabled"].isBool()) {
        config.enabled = json["enabled"].toBool();
    }

    if (json.contains("publish_workflow_events") &&
        json["publish_workflow_events"].isBool()) {
        config.publish_workflow_events =
            json["publish_workflow_events"].toBool();
    }

    if (json.contains("publish_step_events") &&
        json["publish_step_events"].isBool()) {
        config.publish_step_events = json["publish_step_events"].toBool();
    }

    if (json.contains("publish_progress_updates") &&
        json["publish_progress_updates"].isBool()) {
        config.publish_progress_updates =
            json["publish_progress_updates"].toBool();
    }

    if (json.contains("progress_update_interval_ms") &&
        json["progress_update_interval_ms"].isDouble()) {
        config.progress_update_interval = std::chrono::milliseconds(
            json["progress_update_interval_ms"].toInt());
    }

    if (json.contains("include_metadata") &&
        json["include_metadata"].isBool()) {
        config.include_metadata = json["include_metadata"].toBool();
    }

    if (json.contains("include_step_data") &&
        json["include_step_data"].isBool()) {
        config.include_step_data = json["include_step_data"].toBool();
    }

    if (json.contains("message_bus_topic_prefix") &&
        json["message_bus_topic_prefix"].isString()) {
        config.message_bus_topic_prefix =
            json["message_bus_topic_prefix"].toString();
    }

    if (json.contains("default_message_priority") &&
        json["default_message_priority"].isDouble()) {
        config.default_message_priority = static_cast<MessagePriority>(
            json["default_message_priority"].toInt());
    }

    return config;
}

// === ProgressAggregationData Implementation ===

QJsonObject ProgressAggregationData::to_json() const {
    QJsonObject json;
    json["active_workflows"] = active_workflows;
    json["completed_workflows"] = completed_workflows;
    json["failed_workflows"] = failed_workflows;
    json["cancelled_workflows"] = cancelled_workflows;
    json["average_progress"] = average_progress;
    json["total_execution_time_ms"] =
        static_cast<int>(total_execution_time.count());
    json["average_execution_time_ms"] =
        static_cast<int>(average_execution_time.count());
    json["last_update_time"] = last_update_time.toString(Qt::ISODate);

    QJsonObject workflow_progress_json;
    for (const auto& [execution_id, progress_data] : workflow_progress) {
        workflow_progress_json[execution_id] = progress_data.to_json();
    }
    json["workflow_progress"] = workflow_progress_json;

    return json;
}

qtplugin::expected<ProgressAggregationData, PluginError>
ProgressAggregationData::from_json(const QJsonObject& json) {
    ProgressAggregationData data;

    if (json.contains("active_workflows") &&
        json["active_workflows"].isDouble()) {
        data.active_workflows = json["active_workflows"].toInt();
    }

    if (json.contains("completed_workflows") &&
        json["completed_workflows"].isDouble()) {
        data.completed_workflows = json["completed_workflows"].toInt();
    }

    if (json.contains("failed_workflows") &&
        json["failed_workflows"].isDouble()) {
        data.failed_workflows = json["failed_workflows"].toInt();
    }

    if (json.contains("cancelled_workflows") &&
        json["cancelled_workflows"].isDouble()) {
        data.cancelled_workflows = json["cancelled_workflows"].toInt();
    }

    if (json.contains("average_progress") &&
        json["average_progress"].isDouble()) {
        data.average_progress = json["average_progress"].toDouble();
    }

    if (json.contains("total_execution_time_ms") &&
        json["total_execution_time_ms"].isDouble()) {
        data.total_execution_time =
            std::chrono::milliseconds(json["total_execution_time_ms"].toInt());
    }

    if (json.contains("average_execution_time_ms") &&
        json["average_execution_time_ms"].isDouble()) {
        data.average_execution_time = std::chrono::milliseconds(
            json["average_execution_time_ms"].toInt());
    }

    if (json.contains("last_update_time") &&
        json["last_update_time"].isString()) {
        data.last_update_time = QDateTime::fromString(
            json["last_update_time"].toString(), Qt::ISODate);
    }

    if (json.contains("workflow_progress") &&
        json["workflow_progress"].isObject()) {
        QJsonObject workflow_progress_json =
            json["workflow_progress"].toObject();
        for (auto it = workflow_progress_json.begin();
             it != workflow_progress_json.end(); ++it) {
            const QString& execution_id = it.key();
            const QJsonObject& progress_json = it.value().toObject();

            auto progress_result =
                WorkflowProgressData::from_json(progress_json);
            if (progress_result) {
                data.workflow_progress[execution_id] = progress_result.value();
            }
        }
    }

    return data;
}

// === WorkflowProgressTracker Implementation ===

WorkflowProgressTracker::WorkflowProgressTracker(const QString& execution_id,
                                                 const QString& workflow_id,
                                                 const QString& workflow_name,
                                                 QObject* parent)
    : QObject(parent),
      m_execution_id(execution_id),
      m_workflow_id(workflow_id),
      m_workflow_name(workflow_name.isEmpty() ? workflow_id : workflow_name) {
    // Initialize progress data
    m_current_progress.execution_id = m_execution_id;
    m_current_progress.workflow_id = m_workflow_id;
    m_current_progress.workflow_name = m_workflow_name;
    m_current_progress.event_type = WorkflowProgressEventType::WorkflowStarted;
    m_current_progress.start_time = QDateTime::currentDateTime();
    m_current_progress.current_time = m_current_progress.start_time;

    // Create progress update timer
    m_progress_timer = std::make_unique<QTimer>(this);
    m_progress_timer->setSingleShot(false);
    connect(m_progress_timer.get(), &QTimer::timeout, this,
            &WorkflowProgressTracker::on_progress_update_timer);

    qCDebug(workflowProgressLog)
        << "Created progress tracker for execution:" << m_execution_id
        << "workflow:" << m_workflow_id;
}

void WorkflowProgressTracker::start_tracking() {
    if (m_tracking_active) {
        return;
    }

    m_tracking_active = true;

    if (m_config.enabled && m_config.publish_progress_updates) {
        m_progress_timer->setInterval(
            static_cast<int>(m_config.progress_update_interval.count()));
        m_progress_timer->start();
    }

    qCDebug(workflowProgressLog)
        << "Started progress tracking for execution:" << m_execution_id;
}

void WorkflowProgressTracker::stop_tracking() {
    if (!m_tracking_active) {
        return;
    }

    m_tracking_active = false;
    m_progress_timer->stop();

    qCDebug(workflowProgressLog)
        << "Stopped progress tracking for execution:" << m_execution_id;
}

void WorkflowProgressTracker::update_progress(
    const WorkflowProgressData& progress_data) {
    m_current_progress = progress_data;
    m_current_progress.execution_id = m_execution_id;  // Ensure consistency
    m_current_progress.workflow_id = m_workflow_id;
    m_current_progress.workflow_name = m_workflow_name;

    update_timing_information();

    if (m_config.enabled && m_config.publish_progress_updates) {
        publish_progress_message(m_current_progress);
    }

    emit progress_updated(m_current_progress);

    qCDebug(workflowProgressLog)
        << "Updated progress for execution:" << m_execution_id
        << "progress:" << m_current_progress.overall_progress << "%";
}

void WorkflowProgressTracker::update_step_progress(
    const QString& step_id, const state::WorkflowStepState& step_state) {
    m_step_states[step_id] = step_state;

    // Update current step information
    if (step_state.state == state::StepExecutionState::Running) {
        m_current_progress.current_step_id = step_id;
        // Try to get step name from metadata if available
        if (step_state.metadata.contains("step_name")) {
            m_current_progress.current_step_name =
                step_state.metadata["step_name"].toString();
        }
    }

    // Recalculate overall progress
    calculate_and_update_progress();

    if (m_config.enabled && m_config.publish_step_events) {
        publish_step_progress_message(step_id, step_state);
    }

    emit step_progress_updated(step_id, step_state);

    qCDebug(workflowProgressLog)
        << "Updated step progress for execution:" << m_execution_id
        << "step:" << step_id << "state:" << static_cast<int>(step_state.state);
}

void WorkflowProgressTracker::report_workflow_started() {
    m_current_progress.event_type = WorkflowProgressEventType::WorkflowStarted;
    m_current_progress.start_time = QDateTime::currentDateTime();
    m_current_progress.current_time = m_current_progress.start_time;
    m_current_progress.overall_progress = 0.0;

    update_timing_information();

    if (m_config.enabled && m_config.publish_workflow_events) {
        publish_progress_message(m_current_progress);
    }

    emit progress_updated(m_current_progress);

    qCDebug(workflowProgressLog)
        << "Reported workflow started for execution:" << m_execution_id;
}

void WorkflowProgressTracker::report_workflow_completed(
    const QJsonObject& result) {
    m_current_progress.event_type =
        WorkflowProgressEventType::WorkflowCompleted;
    m_current_progress.overall_progress = 100.0;
    m_current_progress.metadata["completion_result"] = result;

    update_timing_information();

    if (m_config.enabled && m_config.publish_workflow_events) {
        publish_progress_message(m_current_progress);
    }

    emit progress_updated(m_current_progress);

    // Stop tracking
    stop_tracking();

    qCDebug(workflowProgressLog)
        << "Reported workflow completed for execution:" << m_execution_id;
}

void WorkflowProgressTracker::report_workflow_failed(
    const QString& error_message) {
    m_current_progress.event_type = WorkflowProgressEventType::WorkflowFailed;
    m_current_progress.error_message = error_message;

    update_timing_information();

    if (m_config.enabled && m_config.publish_workflow_events) {
        publish_progress_message(m_current_progress);
    }

    emit progress_updated(m_current_progress);

    // Stop tracking
    stop_tracking();

    qCDebug(workflowProgressLog)
        << "Reported workflow failed for execution:" << m_execution_id
        << "error:" << error_message;
}

void WorkflowProgressTracker::report_workflow_cancelled() {
    m_current_progress.event_type =
        WorkflowProgressEventType::WorkflowCancelled;

    update_timing_information();

    if (m_config.enabled && m_config.publish_workflow_events) {
        publish_progress_message(m_current_progress);
    }

    emit progress_updated(m_current_progress);

    // Stop tracking
    stop_tracking();

    qCDebug(workflowProgressLog)
        << "Reported workflow cancelled for execution:" << m_execution_id;
}

void WorkflowProgressTracker::report_workflow_suspended() {
    m_current_progress.event_type =
        WorkflowProgressEventType::WorkflowSuspended;

    update_timing_information();

    if (m_config.enabled && m_config.publish_workflow_events) {
        publish_progress_message(m_current_progress);
    }

    emit progress_updated(m_current_progress);

    qCDebug(workflowProgressLog)
        << "Reported workflow suspended for execution:" << m_execution_id;
}

void WorkflowProgressTracker::report_workflow_resumed() {
    m_current_progress.event_type = WorkflowProgressEventType::WorkflowResumed;

    update_timing_information();

    if (m_config.enabled && m_config.publish_workflow_events) {
        publish_progress_message(m_current_progress);
    }

    emit progress_updated(m_current_progress);

    qCDebug(workflowProgressLog)
        << "Reported workflow resumed for execution:" << m_execution_id;
}

void WorkflowProgressTracker::report_step_started(const QString& step_id,
                                                  const QString& step_name) {
    state::WorkflowStepState step_state;
    step_state.step_id = step_id;
    step_state.state = state::StepExecutionState::Running;
    step_state.start_time = QDateTime::currentDateTime();

    if (!step_name.isEmpty()) {
        step_state.metadata["step_name"] = step_name;
    }

    update_step_progress(step_id, step_state);

    qCDebug(workflowProgressLog)
        << "Reported step started for execution:" << m_execution_id
        << "step:" << step_id;
}

void WorkflowProgressTracker::report_step_completed(const QString& step_id,
                                                    const QJsonObject& result) {
    auto it = m_step_states.find(step_id);
    if (it != m_step_states.end()) {
        it->second.state = state::StepExecutionState::Completed;
        it->second.end_time = QDateTime::currentDateTime();
        it->second.output_data = result;

        update_step_progress(step_id, it->second);
    }

    qCDebug(workflowProgressLog)
        << "Reported step completed for execution:" << m_execution_id
        << "step:" << step_id;
}

void WorkflowProgressTracker::report_step_failed(const QString& step_id,
                                                 const QString& error_message) {
    auto it = m_step_states.find(step_id);
    if (it != m_step_states.end()) {
        it->second.state = state::StepExecutionState::Failed;
        it->second.end_time = QDateTime::currentDateTime();
        it->second.error_data["error_message"] = error_message;

        update_step_progress(step_id, it->second);
    }

    qCDebug(workflowProgressLog)
        << "Reported step failed for execution:" << m_execution_id
        << "step:" << step_id << "error:" << error_message;
}

void WorkflowProgressTracker::report_step_skipped(const QString& step_id,
                                                  const QString& reason) {
    auto it = m_step_states.find(step_id);
    if (it != m_step_states.end()) {
        it->second.state = state::StepExecutionState::Skipped;
        it->second.end_time = QDateTime::currentDateTime();

        if (!reason.isEmpty()) {
            it->second.metadata["skip_reason"] = reason;
        }

        update_step_progress(step_id, it->second);
    }

    qCDebug(workflowProgressLog)
        << "Reported step skipped for execution:" << m_execution_id
        << "step:" << step_id << "reason:" << reason;
}

void WorkflowProgressTracker::report_step_retrying(const QString& step_id,
                                                   int retry_count) {
    auto it = m_step_states.find(step_id);
    if (it != m_step_states.end()) {
        it->second.state =
            state::StepExecutionState::Running;  // Reset to running for retry
        it->second.retry_count = retry_count;
        it->second.metadata["retrying"] = true;

        update_step_progress(step_id, it->second);
    }

    qCDebug(workflowProgressLog)
        << "Reported step retrying for execution:" << m_execution_id
        << "step:" << step_id << "retry:" << retry_count;
}

void WorkflowProgressTracker::report_checkpoint_created(
    const QString& checkpoint_id) {
    m_current_progress.event_type =
        WorkflowProgressEventType::CheckpointCreated;
    m_current_progress.metadata["checkpoint_id"] = checkpoint_id;
    m_current_progress.metadata["checkpoint_time"] =
        QDateTime::currentDateTime().toString(Qt::ISODate);

    update_timing_information();

    if (m_config.enabled && m_config.publish_workflow_events) {
        publish_progress_message(m_current_progress);
    }

    emit progress_updated(m_current_progress);

    qCDebug(workflowProgressLog)
        << "Reported checkpoint created for execution:" << m_execution_id
        << "checkpoint:" << checkpoint_id;
}

void WorkflowProgressTracker::report_checkpoint_restored(
    const QString& checkpoint_id) {
    m_current_progress.event_type =
        WorkflowProgressEventType::CheckpointRestored;
    m_current_progress.metadata["restored_checkpoint_id"] = checkpoint_id;
    m_current_progress.metadata["restore_time"] =
        QDateTime::currentDateTime().toString(Qt::ISODate);

    update_timing_information();

    if (m_config.enabled && m_config.publish_workflow_events) {
        publish_progress_message(m_current_progress);
    }

    emit progress_updated(m_current_progress);

    qCDebug(workflowProgressLog)
        << "Reported checkpoint restored for execution:" << m_execution_id
        << "checkpoint:" << checkpoint_id;
}

void WorkflowProgressTracker::calculate_and_update_progress() {
    double new_progress = calculate_overall_progress();

    if (std::abs(new_progress - m_current_progress.overall_progress) >
        0.01) {  // Only update if significant change
        m_current_progress.overall_progress = new_progress;
        m_current_progress.completed_steps = 0;
        m_current_progress.total_steps = static_cast<int>(m_step_states.size());

        // Count completed and skipped steps
        for (const auto& [step_id, step_state] : m_step_states) {
            if (step_state.state == state::StepExecutionState::Completed ||
                step_state.state == state::StepExecutionState::Skipped) {
                m_current_progress.completed_steps++;
            }
        }

        m_current_progress.estimated_remaining_time = estimate_remaining_time();

        update_timing_information();

        if (m_config.enabled && m_config.publish_progress_updates) {
            publish_progress_message(m_current_progress);
        }

        emit progress_updated(m_current_progress);
    }
}

double WorkflowProgressTracker::calculate_overall_progress() const {
    if (m_step_states.empty()) {
        return 0.0;
    }

    size_t completed_steps = 0;
    for (const auto& [step_id, step_state] : m_step_states) {
        if (step_state.state == state::StepExecutionState::Completed ||
            step_state.state == state::StepExecutionState::Skipped) {
            completed_steps++;
        }
    }

    return (static_cast<double>(completed_steps) / m_step_states.size()) *
           100.0;
}

std::chrono::milliseconds WorkflowProgressTracker::estimate_remaining_time()
    const {
    if (m_current_progress.overall_progress <= 0.0 ||
        m_current_progress.overall_progress >= 100.0) {
        return std::chrono::milliseconds(0);
    }

    auto elapsed = m_current_progress.elapsed_time;
    if (elapsed.count() <= 0) {
        return std::chrono::milliseconds(0);
    }

    // Simple linear estimation based on current progress
    double remaining_progress = 100.0 - m_current_progress.overall_progress;
    double time_per_percent = static_cast<double>(elapsed.count()) /
                              m_current_progress.overall_progress;

    return std::chrono::milliseconds(
        static_cast<long long>(remaining_progress * time_per_percent));
}

void WorkflowProgressTracker::set_config(const ProgressTrackingConfig& config) {
    bool interval_changed =
        (m_config.progress_update_interval != config.progress_update_interval);

    m_config = config;

    if (interval_changed && m_progress_timer && m_progress_timer->isActive()) {
        m_progress_timer->setInterval(
            static_cast<int>(m_config.progress_update_interval.count()));
    }

    qCDebug(workflowProgressLog)
        << "Updated progress tracking config for execution:" << m_execution_id;
}

void WorkflowProgressTracker::on_progress_update_timer() {
    if (m_tracking_active && m_config.enabled &&
        m_config.publish_progress_updates) {
        update_timing_information();
        publish_progress_message(m_current_progress);
        emit progress_updated(m_current_progress);
    }
}

void WorkflowProgressTracker::publish_progress_message(
    const WorkflowProgressData& progress_data) {
    // Get message bus instance (this would need to be injected or accessed via
    // singleton) For now, we'll just log the intent
    qCDebug(workflowProgressLog)
        << "Publishing progress message for execution:"
        << progress_data.execution_id
        << "event:" << static_cast<int>(progress_data.event_type)
        << "progress:" << progress_data.overall_progress << "%";

    // In a real implementation, this would publish to the message bus:
    // auto message_bus = MessageBus::instance();
    // WorkflowProgressMessage message("workflow_progress_tracker",
    // progress_data); message_bus->publish(message);
}

void WorkflowProgressTracker::publish_step_progress_message(
    const QString& step_id, const state::WorkflowStepState& step_state) {
    qCDebug(workflowProgressLog)
        << "Publishing step progress message for execution:" << m_execution_id
        << "step:" << step_id << "state:" << static_cast<int>(step_state.state);

    // In a real implementation, this would publish to the message bus:
    // auto message_bus = MessageBus::instance();
    // WorkflowStepProgressMessage message("workflow_progress_tracker",
    // m_execution_id, step_id, step_state); message_bus->publish(message);
}

void WorkflowProgressTracker::update_timing_information() {
    m_current_progress.current_time = QDateTime::currentDateTime();
    m_current_progress.elapsed_time = std::chrono::milliseconds(
        m_current_progress.start_time.msecsTo(m_current_progress.current_time));

    // Update estimated completion time if we have progress
    if (m_current_progress.overall_progress > 0.0 &&
        m_current_progress.overall_progress < 100.0) {
        auto estimated_remaining = estimate_remaining_time();
        m_current_progress.estimated_completion_time =
            m_current_progress.current_time.addMSecs(
                static_cast<qint64>(estimated_remaining.count()));
        m_current_progress.estimated_remaining_time = estimated_remaining;
    }
}

QString WorkflowProgressTracker::generate_message_topic(
    const QString& event_type) const {
    return QString("%1.%2.%3")
        .arg(m_config.message_bus_topic_prefix, m_workflow_id, event_type);
}

// === WorkflowProgressAggregator Implementation ===

WorkflowProgressAggregator::WorkflowProgressAggregator(QObject* parent)
    : QObject(parent) {
    // Initialize aggregation data
    m_aggregation_data.last_update_time = QDateTime::currentDateTime();

    // Create aggregation update timer
    m_aggregation_timer = std::make_unique<QTimer>(this);
    m_aggregation_timer->setSingleShot(false);
    m_aggregation_timer->setInterval(1000);  // Update every second by default
    connect(m_aggregation_timer.get(), &QTimer::timeout, this,
            &WorkflowProgressAggregator::on_aggregation_update_timer);

    qCDebug(workflowProgressLog) << "Created workflow progress aggregator";
}

void WorkflowProgressAggregator::add_workflow_tracker(
    const QString& execution_id, WorkflowProgressTracker* tracker) {
    if (!tracker) {
        qCWarning(workflowProgressLog)
            << "Cannot add null workflow tracker for execution:"
            << execution_id;
        return;
    }

    // Remove existing tracker if present
    remove_workflow_tracker(execution_id);

    m_workflow_trackers[execution_id] = tracker;

    // Connect to tracker signals
    connect(tracker, &WorkflowProgressTracker::progress_updated, this,
            &WorkflowProgressAggregator::on_workflow_progress_updated);

    // Initialize with current progress
    m_last_progress_data[execution_id] = tracker->current_progress();
    m_aggregation_data.workflow_progress[execution_id] =
        tracker->current_progress();

    update_aggregation();

    emit workflow_added(execution_id);

    qCDebug(workflowProgressLog)
        << "Added workflow tracker for execution:" << execution_id;
}

void WorkflowProgressAggregator::remove_workflow_tracker(
    const QString& execution_id) {
    auto tracker_it = m_workflow_trackers.find(execution_id);
    if (tracker_it != m_workflow_trackers.end()) {
        // Disconnect signals
        disconnect(tracker_it->second, nullptr, this, nullptr);

        m_workflow_trackers.erase(tracker_it);
        m_last_progress_data.erase(execution_id);
        m_aggregation_data.workflow_progress.erase(execution_id);

        update_aggregation();

        emit workflow_removed(execution_id);

        qCDebug(workflowProgressLog)
            << "Removed workflow tracker for execution:" << execution_id;
    }
}

void WorkflowProgressAggregator::clear_all_trackers() {
    // Disconnect all signals
    for (const auto& [execution_id, tracker] : m_workflow_trackers) {
        disconnect(tracker, nullptr, this, nullptr);
    }

    m_workflow_trackers.clear();
    m_last_progress_data.clear();
    m_aggregation_data.workflow_progress.clear();

    update_aggregation();

    qCDebug(workflowProgressLog) << "Cleared all workflow trackers";
}

ProgressAggregationData WorkflowProgressAggregator::get_aggregated_progress()
    const {
    return m_aggregation_data;
}

void WorkflowProgressAggregator::update_aggregation() {
    calculate_aggregation_statistics();
    update_workflow_counts();

    m_aggregation_data.last_update_time = QDateTime::currentDateTime();

    if (m_config.enabled) {
        publish_aggregation_message();
    }

    emit aggregation_updated(m_aggregation_data);
}

void WorkflowProgressAggregator::set_config(
    const ProgressTrackingConfig& config) {
    m_config = config;

    if (m_config.enabled && m_config.publish_progress_updates) {
        if (!m_aggregation_timer->isActive()) {
            m_aggregation_timer->start();
        }
    } else {
        m_aggregation_timer->stop();
    }

    qCDebug(workflowProgressLog) << "Updated progress aggregator config";
}

int WorkflowProgressAggregator::active_workflow_count() const {
    return m_aggregation_data.active_workflows;
}

int WorkflowProgressAggregator::total_workflow_count() const {
    return static_cast<int>(m_workflow_trackers.size());
}

double WorkflowProgressAggregator::average_progress() const {
    return m_aggregation_data.average_progress;
}

std::chrono::milliseconds WorkflowProgressAggregator::total_execution_time()
    const {
    return m_aggregation_data.total_execution_time;
}

std::chrono::milliseconds WorkflowProgressAggregator::average_execution_time()
    const {
    return m_aggregation_data.average_execution_time;
}

std::vector<QString> WorkflowProgressAggregator::get_active_executions() const {
    std::vector<QString> active_executions;

    for (const auto& [execution_id, progress_data] :
         m_aggregation_data.workflow_progress) {
        if (progress_data.event_type ==
                WorkflowProgressEventType::WorkflowStarted ||
            progress_data.event_type ==
                WorkflowProgressEventType::WorkflowResumed ||
            progress_data.event_type ==
                WorkflowProgressEventType::ProgressUpdate) {
            active_executions.push_back(execution_id);
        }
    }

    return active_executions;
}

std::vector<QString> WorkflowProgressAggregator::get_completed_executions()
    const {
    std::vector<QString> completed_executions;

    for (const auto& [execution_id, progress_data] :
         m_aggregation_data.workflow_progress) {
        if (progress_data.event_type ==
            WorkflowProgressEventType::WorkflowCompleted) {
            completed_executions.push_back(execution_id);
        }
    }

    return completed_executions;
}

std::vector<QString> WorkflowProgressAggregator::get_failed_executions() const {
    std::vector<QString> failed_executions;

    for (const auto& [execution_id, progress_data] :
         m_aggregation_data.workflow_progress) {
        if (progress_data.event_type ==
            WorkflowProgressEventType::WorkflowFailed) {
            failed_executions.push_back(execution_id);
        }
    }

    return failed_executions;
}

std::optional<WorkflowProgressData>
WorkflowProgressAggregator::get_workflow_progress(
    const QString& execution_id) const {
    auto it = m_aggregation_data.workflow_progress.find(execution_id);
    if (it != m_aggregation_data.workflow_progress.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<WorkflowProgressData>
WorkflowProgressAggregator::get_workflows_by_status(
    WorkflowProgressEventType status) const {
    std::vector<WorkflowProgressData> workflows;

    for (const auto& [execution_id, progress_data] :
         m_aggregation_data.workflow_progress) {
        if (progress_data.event_type == status) {
            workflows.push_back(progress_data);
        }
    }

    return workflows;
}

void WorkflowProgressAggregator::on_workflow_progress_updated(
    const WorkflowProgressData& progress_data) {
    const QString& execution_id = progress_data.execution_id;

    // Check for status change
    WorkflowProgressEventType old_status =
        WorkflowProgressEventType::WorkflowStarted;
    auto last_it = m_last_progress_data.find(execution_id);
    if (last_it != m_last_progress_data.end()) {
        old_status = last_it->second.event_type;
    }

    // Update stored data
    m_last_progress_data[execution_id] = progress_data;
    m_aggregation_data.workflow_progress[execution_id] = progress_data;

    // Emit status change signal if needed
    if (old_status != progress_data.event_type) {
        emit workflow_status_changed(execution_id, old_status,
                                     progress_data.event_type);
    }

    // Update aggregation
    update_aggregation();
}

void WorkflowProgressAggregator::on_aggregation_update_timer() {
    update_aggregation();
}

void WorkflowProgressAggregator::calculate_aggregation_statistics() {
    if (m_aggregation_data.workflow_progress.empty()) {
        m_aggregation_data.average_progress = 0.0;
        m_aggregation_data.total_execution_time = std::chrono::milliseconds(0);
        m_aggregation_data.average_execution_time =
            std::chrono::milliseconds(0);
        return;
    }

    double total_progress = 0.0;
    std::chrono::milliseconds total_time(0);

    for (const auto& [execution_id, progress_data] :
         m_aggregation_data.workflow_progress) {
        total_progress += progress_data.overall_progress;
        total_time += progress_data.elapsed_time;
    }

    m_aggregation_data.average_progress =
        total_progress / m_aggregation_data.workflow_progress.size();
    m_aggregation_data.total_execution_time = total_time;
    m_aggregation_data.average_execution_time = std::chrono::milliseconds(
        total_time.count() / m_aggregation_data.workflow_progress.size());
}

void WorkflowProgressAggregator::update_workflow_counts() {
    m_aggregation_data.active_workflows = 0;
    m_aggregation_data.completed_workflows = 0;
    m_aggregation_data.failed_workflows = 0;
    m_aggregation_data.cancelled_workflows = 0;

    for (const auto& [execution_id, progress_data] :
         m_aggregation_data.workflow_progress) {
        switch (progress_data.event_type) {
            case WorkflowProgressEventType::WorkflowStarted:
            case WorkflowProgressEventType::WorkflowResumed:
            case WorkflowProgressEventType::ProgressUpdate:
            case WorkflowProgressEventType::WorkflowSuspended:
                m_aggregation_data.active_workflows++;
                break;
            case WorkflowProgressEventType::WorkflowCompleted:
                m_aggregation_data.completed_workflows++;
                break;
            case WorkflowProgressEventType::WorkflowFailed:
                m_aggregation_data.failed_workflows++;
                break;
            case WorkflowProgressEventType::WorkflowCancelled:
                m_aggregation_data.cancelled_workflows++;
                break;
            default:
                // Other event types don't affect counts
                break;
        }
    }
}

void WorkflowProgressAggregator::publish_aggregation_message() {
    qCDebug(workflowProgressLog)
        << "Publishing aggregation message - Active:"
        << m_aggregation_data.active_workflows
        << "Completed:" << m_aggregation_data.completed_workflows
        << "Failed:" << m_aggregation_data.failed_workflows
        << "Average progress:" << m_aggregation_data.average_progress << "%";

    // In a real implementation, this would publish to the message bus:
    // auto message_bus = MessageBus::instance();
    // CustomDataMessage message("workflow_progress_aggregator",
    // "aggregation_update", m_aggregation_data.to_json());
    // message_bus->publish(message);
}

// === WorkflowProgressMonitorManager Implementation ===

WorkflowProgressMonitorManager::WorkflowProgressMonitorManager(QObject* parent)
    : QObject(parent) {
    qCDebug(workflowProgressLog) << "Created workflow progress monitor manager";
}

void WorkflowProgressMonitorManager::add_monitor(
    const QString& monitor_id, IWorkflowProgressMonitor* monitor) {
    if (!monitor) {
        qCWarning(workflowProgressLog)
            << "Cannot add null progress monitor:" << monitor_id;
        return;
    }

    m_monitors[monitor_id] = monitor;

    emit monitor_added(monitor_id);

    qCDebug(workflowProgressLog) << "Added progress monitor:" << monitor_id;
}

void WorkflowProgressMonitorManager::remove_monitor(const QString& monitor_id) {
    auto it = m_monitors.find(monitor_id);
    if (it != m_monitors.end()) {
        m_monitors.erase(it);

        emit monitor_removed(monitor_id);

        qCDebug(workflowProgressLog)
            << "Removed progress monitor:" << monitor_id;
    }
}

void WorkflowProgressMonitorManager::clear_monitors() {
    m_monitors.clear();

    qCDebug(workflowProgressLog) << "Cleared all progress monitors";
}

void WorkflowProgressMonitorManager::subscribe_to_progress_messages() {
    if (m_subscribed) {
        return;
    }

    // In a real implementation, this would subscribe to message bus:
    // auto message_bus = MessageBus::instance();
    // message_bus->subscribe<WorkflowProgressMessage>("workflow_progress_monitor",
    //     [this](const WorkflowProgressMessage& msg) {
    //         on_progress_message_received(msg);
    //     });
    // message_bus->subscribe<WorkflowStepProgressMessage>("workflow_progress_monitor",
    //     [this](const WorkflowStepProgressMessage& msg) {
    //         on_step_progress_message_received(msg);
    //     });

    m_subscribed = true;

    qCDebug(workflowProgressLog) << "Subscribed to progress messages";
}

void WorkflowProgressMonitorManager::unsubscribe_from_progress_messages() {
    if (!m_subscribed) {
        return;
    }

    // In a real implementation, this would unsubscribe from message bus

    m_subscribed = false;

    qCDebug(workflowProgressLog) << "Unsubscribed from progress messages";
}

void WorkflowProgressMonitorManager::set_execution_filter(
    const std::vector<QString>& execution_ids) {
    m_execution_filter = execution_ids;

    qCDebug(workflowProgressLog)
        << "Set execution filter with" << execution_ids.size() << "entries";
}

void WorkflowProgressMonitorManager::set_workflow_filter(
    const std::vector<QString>& workflow_ids) {
    m_workflow_filter = workflow_ids;

    qCDebug(workflowProgressLog)
        << "Set workflow filter with" << workflow_ids.size() << "entries";
}

void WorkflowProgressMonitorManager::set_event_type_filter(
    const std::vector<WorkflowProgressEventType>& event_types) {
    m_event_type_filter = event_types;

    qCDebug(workflowProgressLog)
        << "Set event type filter with" << event_types.size() << "entries";
}

void WorkflowProgressMonitorManager::clear_filters() {
    m_execution_filter.clear();
    m_workflow_filter.clear();
    m_event_type_filter.clear();

    qCDebug(workflowProgressLog) << "Cleared all filters";
}

void WorkflowProgressMonitorManager::on_progress_message_received(
    const WorkflowProgressMessage& message) {
    const WorkflowProgressData& progress_data = message.progress_data();

    if (!passes_filters(progress_data)) {
        return;
    }

    notify_monitors_workflow_event(progress_data);
}

void WorkflowProgressMonitorManager::on_step_progress_message_received(
    const WorkflowStepProgressMessage& message) {
    // Create a dummy progress data for filtering
    WorkflowProgressData filter_data;
    filter_data.execution_id = message.execution_id();

    if (!passes_filters(filter_data)) {
        return;
    }

    notify_monitors_step_event(message.execution_id(), message.step_id(),
                               message.step_state());
}

bool WorkflowProgressMonitorManager::passes_filters(
    const WorkflowProgressData& progress_data) const {
    // Check execution filter
    if (!m_execution_filter.empty()) {
        bool found = false;
        for (const QString& execution_id : m_execution_filter) {
            if (progress_data.execution_id == execution_id) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    // Check workflow filter
    if (!m_workflow_filter.empty()) {
        bool found = false;
        for (const QString& workflow_id : m_workflow_filter) {
            if (progress_data.workflow_id == workflow_id) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    // Check event type filter
    if (!m_event_type_filter.empty()) {
        bool found = false;
        for (WorkflowProgressEventType event_type : m_event_type_filter) {
            if (progress_data.event_type == event_type) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    return true;
}

void WorkflowProgressMonitorManager::notify_monitors_workflow_event(
    const WorkflowProgressData& progress_data) {
    for (const auto& [monitor_id, monitor] : m_monitors) {
        try {
            switch (progress_data.event_type) {
                case WorkflowProgressEventType::WorkflowStarted:
                    monitor->on_workflow_started(progress_data);
                    break;
                case WorkflowProgressEventType::WorkflowCompleted:
                    monitor->on_workflow_completed(progress_data);
                    break;
                case WorkflowProgressEventType::WorkflowFailed:
                    monitor->on_workflow_failed(progress_data);
                    break;
                case WorkflowProgressEventType::WorkflowCancelled:
                    monitor->on_workflow_cancelled(progress_data);
                    break;
                case WorkflowProgressEventType::WorkflowSuspended:
                    monitor->on_workflow_suspended(progress_data);
                    break;
                case WorkflowProgressEventType::WorkflowResumed:
                    monitor->on_workflow_resumed(progress_data);
                    break;
                case WorkflowProgressEventType::ProgressUpdate:
                    monitor->on_progress_updated(progress_data);
                    break;
                default:
                    // Handle other event types as progress updates
                    monitor->on_progress_updated(progress_data);
                    break;
            }
        } catch (const std::exception& e) {
            qCWarning(workflowProgressLog)
                << "Monitor" << monitor_id << "threw exception:" << e.what();
        } catch (...) {
            qCWarning(workflowProgressLog)
                << "Monitor" << monitor_id << "threw unknown exception";
        }
    }
}

void WorkflowProgressMonitorManager::notify_monitors_step_event(
    const QString& execution_id, const QString& step_id,
    const state::WorkflowStepState& step_state) {
    for (const auto& [monitor_id, monitor] : m_monitors) {
        try {
            switch (step_state.state) {
                case state::StepExecutionState::Running:
                    monitor->on_step_started(execution_id, step_id, step_state);
                    break;
                case state::StepExecutionState::Completed:
                    monitor->on_step_completed(execution_id, step_id,
                                               step_state);
                    break;
                case state::StepExecutionState::Failed:
                    monitor->on_step_failed(execution_id, step_id, step_state);
                    break;
                case state::StepExecutionState::Skipped:
                    monitor->on_step_skipped(execution_id, step_id, step_state);
                    break;
                default:
                    // Handle other states as generic updates
                    break;
            }
        } catch (const std::exception& e) {
            qCWarning(workflowProgressLog)
                << "Monitor" << monitor_id << "threw exception:" << e.what();
        } catch (...) {
            qCWarning(workflowProgressLog)
                << "Monitor" << monitor_id << "threw unknown exception";
        }
    }
}

}  // namespace qtplugin::workflow::progress
