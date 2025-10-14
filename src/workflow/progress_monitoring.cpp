/**
 * @file progress_monitoring.cpp
 * @brief Implementation of comprehensive workflow progress monitoring APIs
 * @version 3.1.0
 */

#include "qtplugin/workflow/progress_monitoring.hpp"
#include <QLoggingCategory>
#include <QUuid>
#include <algorithm>

namespace {
Q_LOGGING_CATEGORY(workflow_progress_monitoring_log,
                   "qtplugin.workflow.progress.monitoring")
}  // namespace

namespace qtplugin::workflow::progress {

// === ProgressMonitoringFilter Implementation ===

QJsonObject ProgressMonitoringFilter::to_json() const {
    QJsonObject json;

    // Execution filtering
    if (!execution_ids.empty()) {
        QJsonArray execution_array;
        for (const QString& id : execution_ids) {
            execution_array.append(id);
        }
        json["execution_ids"] = execution_array;
    }

    if (!workflow_ids.empty()) {
        QJsonArray workflow_array;
        for (const QString& id : workflow_ids) {
            workflow_array.append(id);
        }
        json["workflow_ids"] = workflow_array;
    }

    if (!workflow_names.empty()) {
        QJsonArray names_array;
        for (const QString& name : workflow_names) {
            names_array.append(name);
        }
        json["workflow_names"] = names_array;
    }

    // Event type filtering
    if (!event_types.empty()) {
        QJsonArray event_types_array;
        for (WorkflowProgressEventType event_type : event_types) {
            event_types_array.append(static_cast<int>(event_type));
        }
        json["event_types"] = event_types_array;
    }

    // Progress range filtering
    if (min_progress.has_value()) {
        json["min_progress"] = min_progress.value();
    }
    if (max_progress.has_value()) {
        json["max_progress"] = max_progress.value();
    }

    // Time-based filtering
    if (start_time.has_value()) {
        json["start_time"] = start_time.value().toString(Qt::ISODate);
    }
    if (end_time.has_value()) {
        json["end_time"] = end_time.value().toString(Qt::ISODate);
    }

    // Step filtering
    if (!step_ids.empty()) {
        QJsonArray step_ids_array;
        for (const QString& step_id : step_ids) {
            step_ids_array.append(step_id);
        }
        json["step_ids"] = step_ids_array;
    }

    if (!step_states.empty()) {
        QJsonArray step_states_array;
        for (state::StepExecutionState step_state : step_states) {
            step_states_array.append(static_cast<int>(step_state));
        }
        json["step_states"] = step_states_array;
    }

    // Metadata filtering
    if (!required_metadata.isEmpty()) {
        json["required_metadata"] = required_metadata;
    }

    return json;
}

qtplugin::expected<ProgressMonitoringFilter, PluginError>
ProgressMonitoringFilter::from_json(const QJsonObject& json) {
    ProgressMonitoringFilter filter;

    // Execution filtering
    if (json.contains("execution_ids") && json["execution_ids"].isArray()) {
        QJsonArray execution_array = json["execution_ids"].toArray();
        for (const QJsonValue& value : execution_array) {
            if (value.isString()) {
                filter.execution_ids.push_back(value.toString());
            }
        }
    }

    if (json.contains("workflow_ids") && json["workflow_ids"].isArray()) {
        QJsonArray workflow_array = json["workflow_ids"].toArray();
        for (const QJsonValue& value : workflow_array) {
            if (value.isString()) {
                filter.workflow_ids.push_back(value.toString());
            }
        }
    }

    if (json.contains("workflow_names") && json["workflow_names"].isArray()) {
        QJsonArray names_array = json["workflow_names"].toArray();
        for (const QJsonValue& value : names_array) {
            if (value.isString()) {
                filter.workflow_names.push_back(value.toString());
            }
        }
    }

    // Event type filtering
    if (json.contains("event_types") && json["event_types"].isArray()) {
        QJsonArray event_types_array = json["event_types"].toArray();
        for (const QJsonValue& value : event_types_array) {
            if (value.isDouble()) {
                filter.event_types.push_back(
                    static_cast<WorkflowProgressEventType>(value.toInt()));
            }
        }
    }

    // Progress range filtering
    if (json.contains("min_progress") && json["min_progress"].isDouble()) {
        filter.min_progress = json["min_progress"].toDouble();
    }
    if (json.contains("max_progress") && json["max_progress"].isDouble()) {
        filter.max_progress = json["max_progress"].toDouble();
    }

    // Time-based filtering
    if (json.contains("start_time") && json["start_time"].isString()) {
        filter.start_time =
            QDateTime::fromString(json["start_time"].toString(), Qt::ISODate);
    }
    if (json.contains("end_time") && json["end_time"].isString()) {
        filter.end_time =
            QDateTime::fromString(json["end_time"].toString(), Qt::ISODate);
    }

    // Step filtering
    if (json.contains("step_ids") && json["step_ids"].isArray()) {
        QJsonArray step_ids_array = json["step_ids"].toArray();
        for (const QJsonValue& value : step_ids_array) {
            if (value.isString()) {
                filter.step_ids.push_back(value.toString());
            }
        }
    }

    if (json.contains("step_states") && json["step_states"].isArray()) {
        QJsonArray step_states_array = json["step_states"].toArray();
        for (const QJsonValue& value : step_states_array) {
            if (value.isDouble()) {
                filter.step_states.push_back(
                    static_cast<state::StepExecutionState>(value.toInt()));
            }
        }
    }

    // Metadata filtering
    if (json.contains("required_metadata") &&
        json["required_metadata"].isObject()) {
        filter.required_metadata = json["required_metadata"].toObject();
    }

    return filter;
}

bool ProgressMonitoringFilter::matches(
    const WorkflowProgressData& progress_data) const {
    // Check execution IDs
    if (!execution_ids.empty()) {
        bool found =
            std::find(execution_ids.begin(), execution_ids.end(),
                      progress_data.execution_id) != execution_ids.end();
        if (!found)
            return false;
    }

    // Check workflow IDs
    if (!workflow_ids.empty()) {
        bool found = std::find(workflow_ids.begin(), workflow_ids.end(),
                               progress_data.workflow_id) != workflow_ids.end();
        if (!found)
            return false;
    }

    // Check workflow names
    if (!workflow_names.empty()) {
        bool found =
            std::find(workflow_names.begin(), workflow_names.end(),
                      progress_data.workflow_name) != workflow_names.end();
        if (!found)
            return false;
    }

    // Check event types
    if (!event_types.empty()) {
        bool found = std::find(event_types.begin(), event_types.end(),
                               progress_data.event_type) != event_types.end();
        if (!found)
            return false;
    }

    // Check progress range
    if (min_progress.has_value() &&
        progress_data.overall_progress < min_progress.value()) {
        return false;
    }
    if (max_progress.has_value() &&
        progress_data.overall_progress > max_progress.value()) {
        return false;
    }

    // Check time range
    if (start_time.has_value() &&
        progress_data.current_time < start_time.value()) {
        return false;
    }
    if (end_time.has_value() && progress_data.current_time > end_time.value()) {
        return false;
    }

    // Check current step
    if (!step_ids.empty() && !progress_data.current_step_id.isEmpty()) {
        bool found = std::find(step_ids.begin(), step_ids.end(),
                               progress_data.current_step_id) != step_ids.end();
        if (!found)
            return false;
    }

    // Check required metadata
    if (!required_metadata.isEmpty()) {
        for (auto it = required_metadata.begin(); it != required_metadata.end();
             ++it) {
            const QString& key = it.key();
            const QJsonValue& required_value = it.value();

            if (!progress_data.metadata.contains(key) ||
                progress_data.metadata[key] != required_value) {
                return false;
            }
        }
    }

    return true;
}

bool ProgressMonitoringFilter::matches_step(
    const QString& execution_id, const QString& step_id,
    const state::WorkflowStepState& step_state) const {
    // Check execution IDs
    if (!execution_ids.empty()) {
        bool found = std::find(execution_ids.begin(), execution_ids.end(),
                               execution_id) != execution_ids.end();
        if (!found)
            return false;
    }

    // Check step IDs
    if (!step_ids.empty()) {
        bool found = std::find(step_ids.begin(), step_ids.end(), step_id) !=
                     step_ids.end();
        if (!found)
            return false;
    }

    // Check step states
    if (!step_states.empty()) {
        bool found = std::find(step_states.begin(), step_states.end(),
                               step_state.state) != step_states.end();
        if (!found)
            return false;
    }

    // Check time range
    if (start_time.has_value() && step_state.start_time < start_time.value()) {
        return false;
    }
    if (end_time.has_value() && step_state.end_time.isValid() &&
        step_state.end_time > end_time.value()) {
        return false;
    }

    // Check required metadata
    if (!required_metadata.isEmpty()) {
        for (auto it = required_metadata.begin(); it != required_metadata.end();
             ++it) {
            const QString& key = it.key();
            const QJsonValue& required_value = it.value();

            if (!step_state.metadata.contains(key) ||
                step_state.metadata[key] != required_value) {
                return false;
            }
        }
    }

    return true;
}

// === FunctionProgressMonitoringCallback Implementation ===

FunctionProgressMonitoringCallback::FunctionProgressMonitoringCallback(
    WorkflowEventHandler workflow_handler, StepEventHandler step_handler,
    AggregationEventHandler aggregation_handler, ErrorHandler error_handler)
    : m_workflow_handler(std::move(workflow_handler)),
      m_step_handler(std::move(step_handler)),
      m_aggregation_handler(std::move(aggregation_handler)),
      m_error_handler(std::move(error_handler)) {}

void FunctionProgressMonitoringCallback::on_workflow_event(
    const WorkflowProgressData& progress_data) {
    if (m_workflow_handler) {
        try {
            m_workflow_handler(progress_data);
        } catch (const std::exception& e) {
            if (m_error_handler) {
                PluginError error(
                    PluginErrorCode::ExecutionFailed,
                    std::string("Workflow event handler threw exception: ") +
                        e.what());
                m_error_handler(error);
            }
        } catch (...) {
            if (m_error_handler) {
                PluginError error(
                    PluginErrorCode::ExecutionFailed,
                    "Workflow event handler threw unknown exception");
                m_error_handler(error);
            }
        }
    }
}

void FunctionProgressMonitoringCallback::on_step_event(
    const QString& execution_id, const QString& step_id,
    const state::WorkflowStepState& step_state) {
    if (m_step_handler) {
        try {
            m_step_handler(execution_id, step_id, step_state);
        } catch (const std::exception& e) {
            if (m_error_handler) {
                PluginError error(
                    PluginErrorCode::ExecutionFailed,
                    std::string("Step event handler threw exception: ") +
                        e.what());
                m_error_handler(error);
            }
        } catch (...) {
            if (m_error_handler) {
                PluginError error(PluginErrorCode::ExecutionFailed,
                                  "Step event handler threw unknown exception");
                m_error_handler(error);
            }
        }
    }
}

void FunctionProgressMonitoringCallback::on_aggregation_event(
    const ProgressAggregationData& aggregation_data) {
    if (m_aggregation_handler) {
        try {
            m_aggregation_handler(aggregation_data);
        } catch (const std::exception& e) {
            if (m_error_handler) {
                PluginError error(
                    PluginErrorCode::ExecutionFailed,
                    std::string("Aggregation event handler threw exception: ") +
                        e.what());
                m_error_handler(error);
            }
        } catch (...) {
            if (m_error_handler) {
                PluginError error(
                    PluginErrorCode::ExecutionFailed,
                    "Aggregation event handler threw unknown exception");
                m_error_handler(error);
            }
        }
    }
}

void FunctionProgressMonitoringCallback::on_monitoring_error(
    const PluginError& error) {
    if (m_error_handler) {
        try {
            m_error_handler(error);
        } catch (...) {
            // Can't handle error in error handler - just log
            qCWarning(workflow_progress_monitoring_log)
                << "Error handler threw exception while handling error:"
                << QString::fromStdString(error.message);
        }
    }
}

// === ProgressMonitoringSubscription Implementation ===

ProgressMonitoringSubscription::ProgressMonitoringSubscription(
    const QString& subscription_id, const ProgressMonitoringFilter& filter,
    std::shared_ptr<IProgressMonitoringCallback> callback)
    : m_subscription_id(subscription_id),
      m_filter(filter),
      m_callback(std::move(callback)),
      m_created_time(QDateTime::currentDateTime()) {
    qCDebug(workflow_progress_monitoring_log)
        << "Created progress monitoring subscription:" << m_subscription_id;
}

ProgressMonitoringSubscription::~ProgressMonitoringSubscription() {
    qCDebug(workflow_progress_monitoring_log)
        << "Destroyed progress monitoring subscription:" << m_subscription_id;
}

void ProgressMonitoringSubscription::process_workflow_event(
    const WorkflowProgressData& progress_data) {
    if (!m_active || !m_callback) {
        return;
    }

    if (m_filter.matches(progress_data)) {
        try {
            m_callback->on_workflow_event(progress_data);
            m_last_event_time = QDateTime::currentDateTime();
            m_event_count++;
        } catch (const std::exception& e) {
            PluginError error(
                PluginErrorCode::ExecutionFailed,
                std::string("Workflow event callback threw exception: ") +
                    e.what());
            m_callback->on_monitoring_error(error);
        } catch (...) {
            PluginError error(
                PluginErrorCode::ExecutionFailed,
                "Workflow event callback threw unknown exception");
            m_callback->on_monitoring_error(error);
        }
    }
}

void ProgressMonitoringSubscription::process_step_event(
    const QString& execution_id, const QString& step_id,
    const state::WorkflowStepState& step_state) {
    if (!m_active || !m_callback) {
        return;
    }

    if (m_filter.matches_step(execution_id, step_id, step_state)) {
        try {
            m_callback->on_step_event(execution_id, step_id, step_state);
            m_last_event_time = QDateTime::currentDateTime();
            m_event_count++;
        } catch (const std::exception& e) {
            PluginError error(
                PluginErrorCode::ExecutionFailed,
                std::string("Step event callback threw exception: ") +
                    e.what());
            m_callback->on_monitoring_error(error);
        } catch (...) {
            PluginError error(PluginErrorCode::ExecutionFailed,
                              "Step event callback threw unknown exception");
            m_callback->on_monitoring_error(error);
        }
    }
}

void ProgressMonitoringSubscription::process_aggregation_event(
    const ProgressAggregationData& aggregation_data) {
    if (!m_active || !m_callback) {
        return;
    }

    // Aggregation events are not filtered by the standard filter
    try {
        m_callback->on_aggregation_event(aggregation_data);
        m_last_event_time = QDateTime::currentDateTime();
        m_event_count++;
    } catch (const std::exception& e) {
        PluginError error(
            PluginErrorCode::ExecutionFailed,
            std::string("Aggregation event callback threw exception: ") +
                e.what());
        m_callback->on_monitoring_error(error);
    } catch (...) {
        PluginError error(PluginErrorCode::ExecutionFailed,
                          "Aggregation event callback threw unknown exception");
        m_callback->on_monitoring_error(error);
    }
}

void ProgressMonitoringSubscription::update_filter(
    const ProgressMonitoringFilter& new_filter) {
    m_filter = new_filter;

    qCDebug(workflow_progress_monitoring_log)
        << "Updated filter for subscription:" << m_subscription_id;
}

// === ProgressMonitoringManager Implementation ===

ProgressMonitoringManager::ProgressMonitoringManager(QObject* parent)
    : QObject(parent) {
    qCDebug(workflow_progress_monitoring_log)
        << "Created progress monitoring manager";
}

ProgressMonitoringManager::~ProgressMonitoringManager() {
    if (m_initialized) {
        shutdown();
    }
}

qtplugin::expected<void, PluginError> ProgressMonitoringManager::initialize() {
    if (m_initialized) {
        return make_success();
    }

    m_initialized = true;

    qCDebug(workflow_progress_monitoring_log)
        << "Initialized progress monitoring manager";

    return make_success();
}

void ProgressMonitoringManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    unsubscribe_all();
    m_initialized = false;

    qCDebug(workflow_progress_monitoring_log)
        << "Shutdown progress monitoring manager";
}

qtplugin::expected<QString, PluginError> ProgressMonitoringManager::subscribe(
    const ProgressMonitoringFilter& filter,
    std::shared_ptr<IProgressMonitoringCallback> callback) {
    if (!m_initialized) {
        return make_error<QString>(PluginErrorCode::InvalidState,
                                   "Manager not initialized");
    }

    if (!callback) {
        return make_error<QString>(PluginErrorCode::InvalidParameters,
                                   "Callback is null");
    }

    QString subscription_id = generate_subscription_id();

    auto subscription = std::make_unique<ProgressMonitoringSubscription>(
        subscription_id, filter, std::move(callback));
    m_subscriptions[subscription_id] = std::move(subscription);

    emit subscription_added(subscription_id);

    qCDebug(workflow_progress_monitoring_log)
        << "Added subscription:" << subscription_id;

    return subscription_id;
}

qtplugin::expected<QString, PluginError>
ProgressMonitoringManager::subscribe_with_functions(
    const ProgressMonitoringFilter& filter,
    FunctionProgressMonitoringCallback::WorkflowEventHandler workflow_handler,
    FunctionProgressMonitoringCallback::StepEventHandler step_handler,
    FunctionProgressMonitoringCallback::AggregationEventHandler
        aggregation_handler,
    FunctionProgressMonitoringCallback::ErrorHandler error_handler) {
    auto callback = std::make_shared<FunctionProgressMonitoringCallback>(
        std::move(workflow_handler), std::move(step_handler),
        std::move(aggregation_handler), std::move(error_handler));

    return subscribe(filter, callback);
}

qtplugin::expected<void, PluginError> ProgressMonitoringManager::unsubscribe(
    const QString& subscription_id) {
    auto it = m_subscriptions.find(subscription_id);
    if (it == m_subscriptions.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Subscription not found: " + subscription_id.toStdString());
    }

    m_subscriptions.erase(it);

    emit subscription_removed(subscription_id);

    qCDebug(workflow_progress_monitoring_log)
        << "Removed subscription:" << subscription_id;

    return make_success();
}

void ProgressMonitoringManager::unsubscribe_all() {
    for (const auto& [subscription_id, subscription] : m_subscriptions) {
        emit subscription_removed(subscription_id);
    }

    m_subscriptions.clear();

    qCDebug(workflow_progress_monitoring_log) << "Removed all subscriptions";
}

std::vector<QString> ProgressMonitoringManager::get_subscription_ids() const {
    std::vector<QString> ids;
    ids.reserve(m_subscriptions.size());

    for (const auto& [subscription_id, subscription] : m_subscriptions) {
        ids.push_back(subscription_id);
    }

    return ids;
}

std::optional<ProgressMonitoringSubscription*>
ProgressMonitoringManager::get_subscription(
    const QString& subscription_id) const {
    auto it = m_subscriptions.find(subscription_id);
    if (it != m_subscriptions.end()) {
        return it->second.get();
    }
    return std::nullopt;
}

size_t ProgressMonitoringManager::subscription_count() const {
    return m_subscriptions.size();
}

qtplugin::expected<void, PluginError>
ProgressMonitoringManager::activate_subscription(
    const QString& subscription_id) {
    auto it = m_subscriptions.find(subscription_id);
    if (it == m_subscriptions.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Subscription not found: " + subscription_id.toStdString());
    }

    it->second->set_active(true);

    emit subscription_activated(subscription_id);

    qCDebug(workflow_progress_monitoring_log)
        << "Activated subscription:" << subscription_id;

    return make_success();
}

qtplugin::expected<void, PluginError>
ProgressMonitoringManager::deactivate_subscription(
    const QString& subscription_id) {
    auto it = m_subscriptions.find(subscription_id);
    if (it == m_subscriptions.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Subscription not found: " + subscription_id.toStdString());
    }

    it->second->set_active(false);

    emit subscription_deactivated(subscription_id);

    qCDebug(workflow_progress_monitoring_log)
        << "Deactivated subscription:" << subscription_id;

    return make_success();
}

qtplugin::expected<void, PluginError>
ProgressMonitoringManager::update_subscription_filter(
    const QString& subscription_id,
    const ProgressMonitoringFilter& new_filter) {
    auto it = m_subscriptions.find(subscription_id);
    if (it == m_subscriptions.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Subscription not found: " + subscription_id.toStdString());
    }

    it->second->update_filter(new_filter);

    qCDebug(workflow_progress_monitoring_log)
        << "Updated filter for subscription:" << subscription_id;

    return make_success();
}

// IWorkflowProgressMonitor implementation
void ProgressMonitoringManager::on_workflow_started(
    const WorkflowProgressData& progress_data) {
    process_workflow_event_internal(progress_data);
}

void ProgressMonitoringManager::on_workflow_completed(
    const WorkflowProgressData& progress_data) {
    process_workflow_event_internal(progress_data);
}

void ProgressMonitoringManager::on_workflow_failed(
    const WorkflowProgressData& progress_data) {
    process_workflow_event_internal(progress_data);
}

void ProgressMonitoringManager::on_workflow_cancelled(
    const WorkflowProgressData& progress_data) {
    process_workflow_event_internal(progress_data);
}

void ProgressMonitoringManager::on_workflow_suspended(
    const WorkflowProgressData& progress_data) {
    process_workflow_event_internal(progress_data);
}

void ProgressMonitoringManager::on_workflow_resumed(
    const WorkflowProgressData& progress_data) {
    process_workflow_event_internal(progress_data);
}

void ProgressMonitoringManager::on_step_started(
    const QString& execution_id, const QString& step_id,
    const state::WorkflowStepState& step_state) {
    process_step_event_internal(execution_id, step_id, step_state);
}

void ProgressMonitoringManager::on_step_completed(
    const QString& execution_id, const QString& step_id,
    const state::WorkflowStepState& step_state) {
    process_step_event_internal(execution_id, step_id, step_state);
}

void ProgressMonitoringManager::on_step_failed(
    const QString& execution_id, const QString& step_id,
    const state::WorkflowStepState& step_state) {
    process_step_event_internal(execution_id, step_id, step_state);
}

void ProgressMonitoringManager::on_step_skipped(
    const QString& execution_id, const QString& step_id,
    const state::WorkflowStepState& step_state) {
    process_step_event_internal(execution_id, step_id, step_state);
}

void ProgressMonitoringManager::on_progress_updated(
    const WorkflowProgressData& progress_data) {
    process_workflow_event_internal(progress_data);
}

void ProgressMonitoringManager::on_aggregation_updated(
    const ProgressAggregationData& aggregation_data) {
    process_aggregation_event_internal(aggregation_data);
}

ProgressMonitoringManager& ProgressMonitoringManager::instance() {
    static ProgressMonitoringManager manager;
    return manager;
}

// Helper methods
QString ProgressMonitoringManager::generate_subscription_id() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void ProgressMonitoringManager::process_workflow_event_internal(
    const WorkflowProgressData& progress_data) {
    if (!m_initialized) {
        return;
    }

    size_t processed_count = 0;

    for (const auto& [subscription_id, subscription] : m_subscriptions) {
        if (subscription && subscription->is_active()) {
            subscription->process_workflow_event(progress_data);
            processed_count++;
        }
    }

    m_total_events_processed++;
    m_workflow_events_processed++;

    emit event_processed("workflow", processed_count);

    qCDebug(workflow_progress_monitoring_log)
        << "Processed workflow event for" << processed_count << "subscriptions";
}

void ProgressMonitoringManager::process_step_event_internal(
    const QString& execution_id, const QString& step_id,
    const state::WorkflowStepState& step_state) {
    if (!m_initialized) {
        return;
    }

    size_t processed_count = 0;

    for (const auto& [subscription_id, subscription] : m_subscriptions) {
        if (subscription && subscription->is_active()) {
            subscription->process_step_event(execution_id, step_id, step_state);
            processed_count++;
        }
    }

    m_total_events_processed++;
    m_step_events_processed++;

    emit event_processed("step", processed_count);

    qCDebug(workflow_progress_monitoring_log)
        << "Processed step event for" << processed_count << "subscriptions";
}

void ProgressMonitoringManager::process_aggregation_event_internal(
    const ProgressAggregationData& aggregation_data) {
    if (!m_initialized) {
        return;
    }

    size_t processed_count = 0;

    for (const auto& [subscription_id, subscription] : m_subscriptions) {
        if (subscription && subscription->is_active()) {
            subscription->process_aggregation_event(aggregation_data);
            processed_count++;
        }
    }

    m_total_events_processed++;
    m_aggregation_events_processed++;

    emit event_processed("aggregation", processed_count);

    qCDebug(workflow_progress_monitoring_log)
        << "Processed aggregation event for" << processed_count
        << "subscriptions";
}

}  // namespace qtplugin::workflow::progress
