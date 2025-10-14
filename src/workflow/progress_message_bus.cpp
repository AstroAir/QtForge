/**
 * @file progress_message_bus.cpp
 * @brief Implementation of message bus integration for workflow progress
 * tracking
 * @version 3.1.0
 */

#include "qtplugin/workflow/progress_message_bus.hpp"
#include <QJsonObject>
#include <QLoggingCategory>
#include <QObject>
#include <QString>
#include <QUuid>
#include <functional>
#include <memory>
#include <optional>
#include "qtplugin/communication/message_bus.hpp"
#include "qtplugin/utils/error_handling.hpp"

namespace {
Q_LOGGING_CATEGORY(workflow_progress_message_bus_log,
                   "qtplugin.workflow.progress.messagebus")
}

namespace qtplugin::workflow::progress {

// === WorkflowProgressMessageBusService Implementation ===

WorkflowProgressMessageBusService::WorkflowProgressMessageBusService(
    QObject* parent)
    : QObject(parent) {
    qCDebug(workflow_progress_message_bus_log)
        << "Created workflow progress message bus service";
}

WorkflowProgressMessageBusService::~WorkflowProgressMessageBusService() {
    if (m_initialized_) {
        shutdown();
    }
}

qtplugin::expected<void, PluginError>
WorkflowProgressMessageBusService::initialize() {
    if (m_initialized_) {
        return make_success();
    }

    // Verify message bus is available
    const IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Message bus is not available");
    }

    m_initialized_ = true;
    m_published_count_ = 0;
    m_subscription_count_ = 0;

    emit service_initialized();

    qCDebug(workflow_progress_message_bus_log)
        << "Initialized workflow progress message bus service";

    return make_success();
}

void WorkflowProgressMessageBusService::shutdown() {
    if (!m_initialized_) {
        return;
    }

    m_initialized_ = false;

    emit service_shutdown();

    qCDebug(workflow_progress_message_bus_log)
        << "Shutdown workflow progress message bus service";
}

qtplugin::expected<void, PluginError>
WorkflowProgressMessageBusService::publish_workflow_progress(
    const WorkflowProgressData& progress_data) {
    if (!m_initialized_) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Service not initialized");
    }

    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Message bus not available");
    }

    const WorkflowProgressMessage message =
        create_workflow_message(progress_data);

    auto result = message_bus->publish(message, DeliveryMode::Broadcast);
    if (result) {
        m_published_count_++;
        emit message_published(generate_workflow_topic(), "WorkflowProgress");

        qCDebug(workflow_progress_message_bus_log)
            << "Published workflow progress message for execution:"
            << progress_data.execution_id;
    }

    return result;
}

qtplugin::expected<void, PluginError>
WorkflowProgressMessageBusService::publish_step_progress(
    const QString& execution_id, const QString& step_id,
    const state::WorkflowStepState& step_state) {
    if (!m_initialized_) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Service not initialized");
    }

    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Message bus not available");
    }

    const WorkflowStepProgressMessage message =
        create_step_message(execution_id, step_id, step_state);

    auto result = message_bus->publish(message, DeliveryMode::Broadcast);
    if (result) {
        m_published_count_++;
        emit message_published(generate_step_topic(), "WorkflowStepProgress");

        qCDebug(workflow_progress_message_bus_log)
            << "Published step progress message for execution:" << execution_id
            << "step:" << step_id;
    }

    return result;
}

qtplugin::expected<void, PluginError>
WorkflowProgressMessageBusService::publish_aggregation_update(
    const ProgressAggregationData& aggregation_data) {
    if (!m_initialized_) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Service not initialized");
    }

    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Message bus not available");
    }

    const qtplugin::messages::CustomDataMessage message =
        create_aggregation_message(aggregation_data);

    auto result = message_bus->publish(message, DeliveryMode::Broadcast);
    if (result) {
        m_published_count_++;
        emit message_published(generate_aggregation_topic(),
                               "ProgressAggregation");

        qCDebug(workflow_progress_message_bus_log)
            << "Published aggregation update message";
    }

    return result;
}

qtplugin::expected<void, PluginError>
WorkflowProgressMessageBusService::subscribe_to_workflow_progress(
    const QString& subscriber_id,
    const std::function<void(const WorkflowProgressMessage&)>& handler) {
    if (!m_initialized_) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Service not initialized");
    }

    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Message bus not available");
    }

    auto result = message_bus->subscribe<WorkflowProgressMessage>(
        subscriber_id.toStdString(),
        [handler](const WorkflowProgressMessage& msg)
            -> qtplugin::expected<void, PluginError> {
            handler(msg);
            return make_success();
        });

    if (result) {
        m_subscription_count_++;
        emit subscription_added(subscriber_id, "WorkflowProgress");

        qCDebug(workflow_progress_message_bus_log)
            << "Added workflow progress subscription for:" << subscriber_id;
    }

    return result;
}

qtplugin::expected<void, PluginError>
WorkflowProgressMessageBusService::subscribe_to_step_progress(
    const QString& subscriber_id,
    const std::function<void(const WorkflowStepProgressMessage&)>& handler) {
    if (!m_initialized_) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Service not initialized");
    }

    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Message bus not available");
    }

    auto result = message_bus->subscribe<WorkflowStepProgressMessage>(
        subscriber_id.toStdString(),
        [handler](const WorkflowStepProgressMessage& msg)
            -> qtplugin::expected<void, PluginError> {
            handler(msg);
            return make_success();
        });

    if (result) {
        m_subscription_count_++;
        emit subscription_added(subscriber_id, "WorkflowStepProgress");

        qCDebug(workflow_progress_message_bus_log)
            << "Added step progress subscription for:" << subscriber_id;
    }

    return result;
}

qtplugin::expected<void, PluginError>
WorkflowProgressMessageBusService::subscribe_to_aggregation_updates(
    const QString& subscriber_id,
    const std::function<void(const qtplugin::messages::CustomDataMessage&)>&
        handler) {
    if (!m_initialized_) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Service not initialized");
    }

    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Message bus not available");
    }

    auto result = message_bus->subscribe<qtplugin::messages::CustomDataMessage>(
        subscriber_id.toStdString(),
        [handler](const qtplugin::messages::CustomDataMessage& msg)
            -> qtplugin::expected<void, PluginError> {
            // Filter for aggregation messages
            if (msg.data_type() == "progress_aggregation") {
                handler(msg);
            }
            return make_success();
        });

    if (result) {
        m_subscription_count_++;
        emit subscription_added(subscriber_id, "ProgressAggregation");

        qCDebug(workflow_progress_message_bus_log)
            << "Added aggregation subscription for:" << subscriber_id;
    }

    return result;
}

qtplugin::expected<void, PluginError>
WorkflowProgressMessageBusService::unsubscribe(const QString& subscriber_id) {
    if (!m_initialized_) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Service not initialized");
    }

    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Message bus not available");
    }

    auto result =
        message_bus->unsubscribe(subscriber_id.toStdString(), std::nullopt);
    if (result) {
        if (m_subscription_count_ > 0) {
            m_subscription_count_--;
        }
        emit subscription_removed(subscriber_id);

        qCDebug(workflow_progress_message_bus_log)
            << "Removed subscriptions for:" << subscriber_id;
    }

    return result;
}

WorkflowProgressMessageBusService&
WorkflowProgressMessageBusService::instance() {
    static WorkflowProgressMessageBusService service;
    return service;
}

IMessageBus* WorkflowProgressMessageBusService::get_message_bus() {
    // Access the QtForge message bus singleton
    // In the actual implementation, this would be:
    // return &MessageBus::instance();

    // For now, return nullptr to indicate the message bus is not available
    // This will be replaced with actual message bus access
    return nullptr;
}

QString WorkflowProgressMessageBusService::generate_workflow_topic() const {
    return QString("%1.workflow").arg(m_topic_prefix_);
}

QString WorkflowProgressMessageBusService::generate_step_topic() const {
    return QString("%1.step").arg(m_topic_prefix_);
}

QString WorkflowProgressMessageBusService::generate_aggregation_topic() const {
    return QString("%1.aggregation").arg(m_topic_prefix_);
}

WorkflowProgressMessage
WorkflowProgressMessageBusService::create_workflow_message(
    const WorkflowProgressData& progress_data) const {
    return {"workflow_progress_service", progress_data};
}

WorkflowStepProgressMessage
WorkflowProgressMessageBusService::create_step_message(
    const QString& execution_id, const QString& step_id,
    const state::WorkflowStepState& step_state) const {
    return {"workflow_progress_service", execution_id, step_id, step_state};
}

qtplugin::messages::CustomDataMessage
WorkflowProgressMessageBusService::create_aggregation_message(
    const ProgressAggregationData& aggregation_data) const {
    return {"workflow_progress_service", "progress_aggregation",
            aggregation_data.to_json(), m_default_priority_};
}

// === MessageBusWorkflowProgressTracker Implementation ===

MessageBusWorkflowProgressTracker::MessageBusWorkflowProgressTracker(
    const QString& execution_id, const QString& workflow_id,
    const QString& workflow_name, QObject* parent)
    : WorkflowProgressTracker(execution_id, workflow_id, workflow_name,
                              parent) {
    qCDebug(workflow_progress_message_bus_log)
        << "Created message bus workflow progress tracker for execution:"
        << execution_id;
}

void MessageBusWorkflowProgressTracker::set_message_bus_service(
    WorkflowProgressMessageBusService* service) {
    m_message_bus_service_ = service;

    qCDebug(workflow_progress_message_bus_log)
        << "Set message bus service for tracker:" << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_started() {
    WorkflowProgressTracker::report_workflow_started();

    qCDebug(workflow_progress_message_bus_log)
        << "Reported workflow started via message bus for execution:"
        << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_completed(
    const QJsonObject& result) {
    WorkflowProgressTracker::report_workflow_completed(result);

    qCDebug(workflow_progress_message_bus_log)
        << "Reported workflow completed via message bus for execution:"
        << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_failed(
    const QString& error_message) {
    WorkflowProgressTracker::report_workflow_failed(error_message);

    qCDebug(workflow_progress_message_bus_log)
        << "Reported workflow failed via message bus for execution:"
        << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_cancelled() {
    WorkflowProgressTracker::report_workflow_cancelled();

    qCDebug(workflow_progress_message_bus_log)
        << "Reported workflow cancelled via message bus for execution:"
        << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_suspended() {
    WorkflowProgressTracker::report_workflow_suspended();

    qCDebug(workflow_progress_message_bus_log)
        << "Reported workflow suspended via message bus for execution:"
        << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_resumed() {
    WorkflowProgressTracker::report_workflow_resumed();

    qCDebug(workflow_progress_message_bus_log)
        << "Reported workflow resumed via message bus for execution:"
        << execution_id();
}

void MessageBusWorkflowProgressTracker::publish_progress_message(
    const WorkflowProgressData& progress_data) {
    if (m_message_bus_service_ && m_message_bus_service_->is_initialized()) {
        auto result =
            m_message_bus_service_->publish_workflow_progress(progress_data);
        if (!result) {
            qCWarning(workflow_progress_message_bus_log)
                << "Failed to publish workflow progress message:"
                << result.error().message.c_str();
        }
    } else {
        qCDebug(workflow_progress_message_bus_log)
            << "Message bus service not available, skipping progress message "
               "publication";
    }
}

void MessageBusWorkflowProgressTracker::publish_step_progress_message(
    const QString& step_id, const state::WorkflowStepState& step_state) {
    if (m_message_bus_service_ && m_message_bus_service_->is_initialized()) {
        auto result = m_message_bus_service_->publish_step_progress(
            execution_id(), step_id, step_state);
        if (!result) {
            qCWarning(workflow_progress_message_bus_log)
                << "Failed to publish step progress message:"
                << result.error().message.c_str();
        }
    } else {
        qCDebug(workflow_progress_message_bus_log)
            << "Message bus service not available, skipping step progress "
               "message publication";
    }
}

// === MessageBusWorkflowProgressAggregator Implementation ===

MessageBusWorkflowProgressAggregator::MessageBusWorkflowProgressAggregator(
    QObject* parent)
    : WorkflowProgressAggregator(parent) {
    qCDebug(workflow_progress_message_bus_log)
        << "Created message bus workflow progress aggregator";
}

void MessageBusWorkflowProgressAggregator::set_message_bus_service(
    WorkflowProgressMessageBusService* service) {
    m_message_bus_service_ = service;

    qCDebug(workflow_progress_message_bus_log)
        << "Set message bus service for aggregator";
}

void MessageBusWorkflowProgressAggregator::update_aggregation() {
    WorkflowProgressAggregator::update_aggregation();

    qCDebug(workflow_progress_message_bus_log)
        << "Updated aggregation via message bus";
}

void MessageBusWorkflowProgressAggregator::publish_aggregation_message() {
    if (m_message_bus_service_ && m_message_bus_service_->is_initialized()) {
        auto result = m_message_bus_service_->publish_aggregation_update(
            get_aggregated_progress());
        if (!result) {
            qCWarning(workflow_progress_message_bus_log)
                << "Failed to publish aggregation message:"
                << result.error().message.c_str();
        }
    } else {
        qCDebug(workflow_progress_message_bus_log)
            << "Message bus service not available, skipping aggregation "
               "message publication";
    }
}

// === WorkflowProgressFactory Implementation ===

bool WorkflowProgressFactory::s_services_initialized = false;

std::unique_ptr<MessageBusWorkflowProgressTracker>
WorkflowProgressFactory::create_tracker(const QString& execution_id,
                                        const QString& workflow_id,
                                        const QString& workflow_name,
                                        QObject* parent) {
    auto tracker = std::make_unique<MessageBusWorkflowProgressTracker>(
        execution_id, workflow_id, workflow_name, parent);

    // Inject message bus service
    tracker->set_message_bus_service(&get_message_bus_service());

    return tracker;
}

std::unique_ptr<MessageBusWorkflowProgressAggregator>
WorkflowProgressFactory::create_aggregator(QObject* parent) {
    auto aggregator =
        std::make_unique<MessageBusWorkflowProgressAggregator>(parent);

    // Inject message bus service
    aggregator->set_message_bus_service(&get_message_bus_service());

    return aggregator;
}

std::unique_ptr<WorkflowProgressMonitorManager>
WorkflowProgressFactory::create_monitor_manager(QObject* parent) {
    return std::make_unique<WorkflowProgressMonitorManager>(parent);
}

WorkflowProgressMessageBusService&
WorkflowProgressFactory::get_message_bus_service() {
    return WorkflowProgressMessageBusService::instance();
}

qtplugin::expected<void, PluginError>
WorkflowProgressFactory::initialize_services() {
    if (s_services_initialized) {
        return make_success();
    }

    auto result = get_message_bus_service().initialize();
    if (result) {
        s_services_initialized = true;
        qCDebug(workflow_progress_message_bus_log)
            << "Initialized workflow progress services";
    }

    return result;
}

void WorkflowProgressFactory::shutdown_services() {
    if (s_services_initialized) {
        get_message_bus_service().shutdown();
        s_services_initialized = false;
        qCDebug(workflow_progress_message_bus_log)
            << "Shutdown workflow progress services";
    }
}

// === WorkflowProgressSession Implementation ===

WorkflowProgressSession::WorkflowProgressSession(const QString& execution_id,
                                                 const QString& workflow_id,
                                                 const QString& workflow_name,
                                                 QObject* parent)
    : QObject(parent),
      m_execution_id_(execution_id),
      m_workflow_id_(workflow_id),
      m_workflow_name_(workflow_name.isEmpty() ? workflow_id : workflow_name) {
    qCDebug(workflow_progress_message_bus_log)
        << "Created workflow progress session for execution:"
        << m_execution_id_;
}

WorkflowProgressSession::~WorkflowProgressSession() {
    if (m_active_) {
        stop();
    }
}

qtplugin::expected<void, PluginError> WorkflowProgressSession::start() {
    if (m_active_) {
        return make_success();
    }

    // Initialize services if needed
    auto init_result = WorkflowProgressFactory::initialize_services();
    if (!init_result) {
        return init_result;
    }

    // Create tracker
    m_tracker_ = WorkflowProgressFactory::create_tracker(
        m_execution_id_, m_workflow_id_, m_workflow_name_, this);
    if (!m_tracker_) {
        return make_error<void>(PluginErrorCode::InitializationFailed,
                                "Failed to create progress tracker");
    }

    // Get message bus service
    m_message_bus_service_ =
        &WorkflowProgressFactory::get_message_bus_service();

    // Start tracking
    m_tracker_->start_tracking();

    m_active_ = true;

    emit session_started();

    qCDebug(workflow_progress_message_bus_log)
        << "Started workflow progress session for execution:"
        << m_execution_id_;

    return make_success();
}

void WorkflowProgressSession::stop() {
    if (!m_active_) {
        return;
    }

    if (m_tracker_) {
        m_tracker_->stop_tracking();
        m_tracker_.reset();
    }

    m_message_bus_service_ = nullptr;
    m_active_ = false;

    emit session_stopped();

    qCDebug(workflow_progress_message_bus_log)
        << "Stopped workflow progress session for execution:"
        << m_execution_id_;
}

}  // namespace qtplugin::workflow::progress
