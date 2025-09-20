/**
 * @file progress_message_bus.cpp
 * @brief Implementation of message bus integration for workflow progress tracking
 * @version 3.1.0
 */

#include "qtplugin/workflow/progress_message_bus.hpp"
#include "qtplugin/communication/message_bus.hpp"
#include <QLoggingCategory>
#include <QUuid>

Q_LOGGING_CATEGORY(workflowProgressMessageBusLog, "qtplugin.workflow.progress.messagebus")

namespace qtplugin::workflow::progress {

// === WorkflowProgressMessageBusService Implementation ===

WorkflowProgressMessageBusService::WorkflowProgressMessageBusService(QObject* parent)
    : QObject(parent) {
    
    qCDebug(workflowProgressMessageBusLog) << "Created workflow progress message bus service";
}

WorkflowProgressMessageBusService::~WorkflowProgressMessageBusService() {
    if (m_initialized) {
        shutdown();
    }
}

qtplugin::expected<void, PluginError> WorkflowProgressMessageBusService::initialize() {
    if (m_initialized) {
        return make_success();
    }
    
    // Verify message bus is available
    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Message bus is not available");
    }
    
    m_initialized = true;
    m_published_count = 0;
    m_subscription_count = 0;
    
    emit service_initialized();
    
    qCDebug(workflowProgressMessageBusLog) << "Initialized workflow progress message bus service";
    
    return make_success();
}

void WorkflowProgressMessageBusService::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    m_initialized = false;
    
    emit service_shutdown();
    
    qCDebug(workflowProgressMessageBusLog) << "Shutdown workflow progress message bus service";
}

qtplugin::expected<void, PluginError> WorkflowProgressMessageBusService::publish_workflow_progress(const WorkflowProgressData& progress_data) {
    if (!m_initialized) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Service not initialized");
    }
    
    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Message bus not available");
    }
    
    WorkflowProgressMessage message = create_workflow_message(progress_data);
    
    auto result = message_bus->publish(message, DeliveryMode::Broadcast);
    if (result) {
        m_published_count++;
        emit message_published(generate_workflow_topic(), "WorkflowProgress");
        
        qCDebug(workflowProgressMessageBusLog) << "Published workflow progress message for execution:" 
                                               << progress_data.execution_id;
    }
    
    return result;
}

qtplugin::expected<void, PluginError> WorkflowProgressMessageBusService::publish_step_progress(
    const QString& execution_id, 
    const QString& step_id, 
    const state::WorkflowStepState& step_state) {
    
    if (!m_initialized) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Service not initialized");
    }
    
    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Message bus not available");
    }
    
    WorkflowStepProgressMessage message = create_step_message(execution_id, step_id, step_state);
    
    auto result = message_bus->publish(message, DeliveryMode::Broadcast);
    if (result) {
        m_published_count++;
        emit message_published(generate_step_topic(), "WorkflowStepProgress");
        
        qCDebug(workflowProgressMessageBusLog) << "Published step progress message for execution:" 
                                               << execution_id << "step:" << step_id;
    }
    
    return result;
}

qtplugin::expected<void, PluginError> WorkflowProgressMessageBusService::publish_aggregation_update(const ProgressAggregationData& aggregation_data) {
    if (!m_initialized) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Service not initialized");
    }
    
    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Message bus not available");
    }
    
    CustomDataMessage message = create_aggregation_message(aggregation_data);
    
    auto result = message_bus->publish(message, DeliveryMode::Broadcast);
    if (result) {
        m_published_count++;
        emit message_published(generate_aggregation_topic(), "ProgressAggregation");
        
        qCDebug(workflowProgressMessageBusLog) << "Published aggregation update message";
    }
    
    return result;
}

qtplugin::expected<void, PluginError> WorkflowProgressMessageBusService::subscribe_to_workflow_progress(
    const QString& subscriber_id,
    std::function<void(const WorkflowProgressMessage&)> handler) {
    
    if (!m_initialized) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Service not initialized");
    }
    
    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Message bus not available");
    }
    
    auto result = message_bus->subscribe<WorkflowProgressMessage>(
        subscriber_id.toStdString(),
        [handler](const WorkflowProgressMessage& msg) -> qtplugin::expected<void, PluginError> {
            handler(msg);
            return make_success();
        });
    
    if (result) {
        m_subscription_count++;
        emit subscription_added(subscriber_id, "WorkflowProgress");
        
        qCDebug(workflowProgressMessageBusLog) << "Added workflow progress subscription for:" << subscriber_id;
    }
    
    return result;
}

qtplugin::expected<void, PluginError> WorkflowProgressMessageBusService::subscribe_to_step_progress(
    const QString& subscriber_id,
    std::function<void(const WorkflowStepProgressMessage&)> handler) {
    
    if (!m_initialized) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Service not initialized");
    }
    
    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Message bus not available");
    }
    
    auto result = message_bus->subscribe<WorkflowStepProgressMessage>(
        subscriber_id.toStdString(),
        [handler](const WorkflowStepProgressMessage& msg) -> qtplugin::expected<void, PluginError> {
            handler(msg);
            return make_success();
        });
    
    if (result) {
        m_subscription_count++;
        emit subscription_added(subscriber_id, "WorkflowStepProgress");
        
        qCDebug(workflowProgressMessageBusLog) << "Added step progress subscription for:" << subscriber_id;
    }
    
    return result;
}

qtplugin::expected<void, PluginError> WorkflowProgressMessageBusService::subscribe_to_aggregation_updates(
    const QString& subscriber_id,
    std::function<void(const CustomDataMessage&)> handler) {
    
    if (!m_initialized) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Service not initialized");
    }
    
    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Message bus not available");
    }
    
    auto result = message_bus->subscribe<CustomDataMessage>(
        subscriber_id.toStdString(),
        [handler](const CustomDataMessage& msg) -> qtplugin::expected<void, PluginError> {
            // Filter for aggregation messages
            if (msg.data_type() == "progress_aggregation") {
                handler(msg);
            }
            return make_success();
        });
    
    if (result) {
        m_subscription_count++;
        emit subscription_added(subscriber_id, "ProgressAggregation");
        
        qCDebug(workflowProgressMessageBusLog) << "Added aggregation subscription for:" << subscriber_id;
    }
    
    return result;
}

qtplugin::expected<void, PluginError> WorkflowProgressMessageBusService::unsubscribe(const QString& subscriber_id) {
    if (!m_initialized) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Service not initialized");
    }
    
    IMessageBus* message_bus = get_message_bus();
    if (!message_bus) {
        return make_error<void>(PluginErrorCode::ServiceUnavailable, "Message bus not available");
    }
    
    auto result = message_bus->unsubscribe(subscriber_id.toStdString());
    if (result) {
        if (m_subscription_count > 0) {
            m_subscription_count--;
        }
        emit subscription_removed(subscriber_id);
        
        qCDebug(workflowProgressMessageBusLog) << "Removed subscriptions for:" << subscriber_id;
    }
    
    return result;
}

WorkflowProgressMessageBusService& WorkflowProgressMessageBusService::instance() {
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
    return QString("%1.workflow").arg(m_topic_prefix);
}

QString WorkflowProgressMessageBusService::generate_step_topic() const {
    return QString("%1.step").arg(m_topic_prefix);
}

QString WorkflowProgressMessageBusService::generate_aggregation_topic() const {
    return QString("%1.aggregation").arg(m_topic_prefix);
}

WorkflowProgressMessage WorkflowProgressMessageBusService::create_workflow_message(const WorkflowProgressData& progress_data) const {
    return WorkflowProgressMessage("workflow_progress_service", progress_data);
}

WorkflowStepProgressMessage WorkflowProgressMessageBusService::create_step_message(
    const QString& execution_id, 
    const QString& step_id, 
    const state::WorkflowStepState& step_state) const {
    
    return WorkflowStepProgressMessage("workflow_progress_service", execution_id, step_id, step_state);
}

CustomDataMessage WorkflowProgressMessageBusService::create_aggregation_message(const ProgressAggregationData& aggregation_data) const {
    return CustomDataMessage("workflow_progress_service", "progress_aggregation", aggregation_data.to_json(), m_default_priority);
}

// === MessageBusWorkflowProgressTracker Implementation ===

MessageBusWorkflowProgressTracker::MessageBusWorkflowProgressTracker(
    const QString& execution_id,
    const QString& workflow_id,
    const QString& workflow_name,
    QObject* parent)
    : WorkflowProgressTracker(execution_id, workflow_id, workflow_name, parent) {

    qCDebug(workflowProgressMessageBusLog) << "Created message bus workflow progress tracker for execution:" << execution_id;
}

void MessageBusWorkflowProgressTracker::set_message_bus_service(WorkflowProgressMessageBusService* service) {
    m_message_bus_service = service;

    qCDebug(workflowProgressMessageBusLog) << "Set message bus service for tracker:" << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_started() {
    WorkflowProgressTracker::report_workflow_started();

    qCDebug(workflowProgressMessageBusLog) << "Reported workflow started via message bus for execution:" << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_completed(const QJsonObject& result) {
    WorkflowProgressTracker::report_workflow_completed(result);

    qCDebug(workflowProgressMessageBusLog) << "Reported workflow completed via message bus for execution:" << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_failed(const QString& error_message) {
    WorkflowProgressTracker::report_workflow_failed(error_message);

    qCDebug(workflowProgressMessageBusLog) << "Reported workflow failed via message bus for execution:" << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_cancelled() {
    WorkflowProgressTracker::report_workflow_cancelled();

    qCDebug(workflowProgressMessageBusLog) << "Reported workflow cancelled via message bus for execution:" << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_suspended() {
    WorkflowProgressTracker::report_workflow_suspended();

    qCDebug(workflowProgressMessageBusLog) << "Reported workflow suspended via message bus for execution:" << execution_id();
}

void MessageBusWorkflowProgressTracker::report_workflow_resumed() {
    WorkflowProgressTracker::report_workflow_resumed();

    qCDebug(workflowProgressMessageBusLog) << "Reported workflow resumed via message bus for execution:" << execution_id();
}

void MessageBusWorkflowProgressTracker::publish_progress_message(const WorkflowProgressData& progress_data) {
    if (m_message_bus_service && m_message_bus_service->is_initialized()) {
        auto result = m_message_bus_service->publish_workflow_progress(progress_data);
        if (!result) {
            qCWarning(workflowProgressMessageBusLog) << "Failed to publish workflow progress message:"
                                                     << result.error().message().c_str();
        }
    } else {
        qCDebug(workflowProgressMessageBusLog) << "Message bus service not available, skipping progress message publication";
    }
}

void MessageBusWorkflowProgressTracker::publish_step_progress_message(const QString& step_id, const state::WorkflowStepState& step_state) {
    if (m_message_bus_service && m_message_bus_service->is_initialized()) {
        auto result = m_message_bus_service->publish_step_progress(execution_id(), step_id, step_state);
        if (!result) {
            qCWarning(workflowProgressMessageBusLog) << "Failed to publish step progress message:"
                                                     << result.error().message().c_str();
        }
    } else {
        qCDebug(workflowProgressMessageBusLog) << "Message bus service not available, skipping step progress message publication";
    }
}

// === MessageBusWorkflowProgressAggregator Implementation ===

MessageBusWorkflowProgressAggregator::MessageBusWorkflowProgressAggregator(QObject* parent)
    : WorkflowProgressAggregator(parent) {

    qCDebug(workflowProgressMessageBusLog) << "Created message bus workflow progress aggregator";
}

void MessageBusWorkflowProgressAggregator::set_message_bus_service(WorkflowProgressMessageBusService* service) {
    m_message_bus_service = service;

    qCDebug(workflowProgressMessageBusLog) << "Set message bus service for aggregator";
}

void MessageBusWorkflowProgressAggregator::update_aggregation() {
    WorkflowProgressAggregator::update_aggregation();

    qCDebug(workflowProgressMessageBusLog) << "Updated aggregation via message bus";
}

void MessageBusWorkflowProgressAggregator::publish_aggregation_message() {
    if (m_message_bus_service && m_message_bus_service->is_initialized()) {
        auto result = m_message_bus_service->publish_aggregation_update(get_aggregated_progress());
        if (!result) {
            qCWarning(workflowProgressMessageBusLog) << "Failed to publish aggregation message:"
                                                     << result.error().message().c_str();
        }
    } else {
        qCDebug(workflowProgressMessageBusLog) << "Message bus service not available, skipping aggregation message publication";
    }
}

// === WorkflowProgressFactory Implementation ===

bool WorkflowProgressFactory::s_services_initialized = false;

std::unique_ptr<MessageBusWorkflowProgressTracker> WorkflowProgressFactory::create_tracker(
    const QString& execution_id,
    const QString& workflow_id,
    const QString& workflow_name,
    QObject* parent) {

    auto tracker = std::make_unique<MessageBusWorkflowProgressTracker>(execution_id, workflow_id, workflow_name, parent);

    // Inject message bus service
    tracker->set_message_bus_service(&get_message_bus_service());

    return tracker;
}

std::unique_ptr<MessageBusWorkflowProgressAggregator> WorkflowProgressFactory::create_aggregator(QObject* parent) {
    auto aggregator = std::make_unique<MessageBusWorkflowProgressAggregator>(parent);

    // Inject message bus service
    aggregator->set_message_bus_service(&get_message_bus_service());

    return aggregator;
}

std::unique_ptr<WorkflowProgressMonitorManager> WorkflowProgressFactory::create_monitor_manager(QObject* parent) {
    return std::make_unique<WorkflowProgressMonitorManager>(parent);
}

WorkflowProgressMessageBusService& WorkflowProgressFactory::get_message_bus_service() {
    return WorkflowProgressMessageBusService::instance();
}

qtplugin::expected<void, PluginError> WorkflowProgressFactory::initialize_services() {
    if (s_services_initialized) {
        return make_success();
    }

    auto result = get_message_bus_service().initialize();
    if (result) {
        s_services_initialized = true;
        qCDebug(workflowProgressMessageBusLog) << "Initialized workflow progress services";
    }

    return result;
}

void WorkflowProgressFactory::shutdown_services() {
    if (s_services_initialized) {
        get_message_bus_service().shutdown();
        s_services_initialized = false;
        qCDebug(workflowProgressMessageBusLog) << "Shutdown workflow progress services";
    }
}

// === WorkflowProgressSession Implementation ===

WorkflowProgressSession::WorkflowProgressSession(
    const QString& execution_id,
    const QString& workflow_id,
    const QString& workflow_name,
    QObject* parent)
    : QObject(parent),
      m_execution_id(execution_id),
      m_workflow_id(workflow_id),
      m_workflow_name(workflow_name.isEmpty() ? workflow_id : workflow_name) {

    qCDebug(workflowProgressMessageBusLog) << "Created workflow progress session for execution:" << m_execution_id;
}

WorkflowProgressSession::~WorkflowProgressSession() {
    if (m_active) {
        stop();
    }
}

qtplugin::expected<void, PluginError> WorkflowProgressSession::start() {
    if (m_active) {
        return make_success();
    }

    // Initialize services if needed
    auto init_result = WorkflowProgressFactory::initialize_services();
    if (!init_result) {
        return init_result;
    }

    // Create tracker
    m_tracker = WorkflowProgressFactory::create_tracker(m_execution_id, m_workflow_id, m_workflow_name, this);
    if (!m_tracker) {
        return make_error<void>(PluginErrorCode::InitializationFailed, "Failed to create progress tracker");
    }

    // Get message bus service
    m_message_bus_service = &WorkflowProgressFactory::get_message_bus_service();

    // Start tracking
    m_tracker->start_tracking();

    m_active = true;

    emit session_started();

    qCDebug(workflowProgressMessageBusLog) << "Started workflow progress session for execution:" << m_execution_id;

    return make_success();
}

void WorkflowProgressSession::stop() {
    if (!m_active) {
        return;
    }

    if (m_tracker) {
        m_tracker->stop_tracking();
        m_tracker.reset();
    }

    m_message_bus_service = nullptr;
    m_active = false;

    emit session_stopped();

    qCDebug(workflowProgressMessageBusLog) << "Stopped workflow progress session for execution:" << m_execution_id;
}

} // namespace qtplugin::workflow::progress
