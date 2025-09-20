/**
 * @file progress_message_bus.hpp
 * @brief Message bus integration for workflow progress tracking
 * @version 3.1.0
 */

#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include <functional>

#include "../utils/error_handling.hpp"
#include "../communication/message_bus.hpp"
#include "progress_tracking.hpp"

namespace qtplugin::workflow::progress {

/**
 * @brief Message bus service for workflow progress tracking
 * 
 * Provides a centralized service for publishing and subscribing to workflow
 * progress messages through the QtForge message bus system.
 */
class WorkflowProgressMessageBusService : public QObject {
    Q_OBJECT

public:
    explicit WorkflowProgressMessageBusService(QObject* parent = nullptr);
    ~WorkflowProgressMessageBusService() override;

    // Service lifecycle
    qtplugin::expected<void, PluginError> initialize();
    void shutdown();
    bool is_initialized() const { return m_initialized; }

    // Publishing
    qtplugin::expected<void, PluginError> publish_workflow_progress(const WorkflowProgressData& progress_data);
    qtplugin::expected<void, PluginError> publish_step_progress(
        const QString& execution_id, 
        const QString& step_id, 
        const state::WorkflowStepState& step_state);
    qtplugin::expected<void, PluginError> publish_aggregation_update(const ProgressAggregationData& aggregation_data);

    // Subscription management
    qtplugin::expected<void, PluginError> subscribe_to_workflow_progress(
        const QString& subscriber_id,
        std::function<void(const WorkflowProgressMessage&)> handler);
    
    qtplugin::expected<void, PluginError> subscribe_to_step_progress(
        const QString& subscriber_id,
        std::function<void(const WorkflowStepProgressMessage&)> handler);
    
    qtplugin::expected<void, PluginError> subscribe_to_aggregation_updates(
        const QString& subscriber_id,
        std::function<void(const CustomDataMessage&)> handler);

    qtplugin::expected<void, PluginError> unsubscribe(const QString& subscriber_id);

    // Configuration
    void set_topic_prefix(const QString& prefix) { m_topic_prefix = prefix; }
    const QString& topic_prefix() const { return m_topic_prefix; }

    void set_default_priority(MessagePriority priority) { m_default_priority = priority; }
    MessagePriority default_priority() const { return m_default_priority; }

    // Statistics
    size_t published_message_count() const { return m_published_count; }
    size_t subscription_count() const { return m_subscription_count; }

    // Singleton access
    static WorkflowProgressMessageBusService& instance();

signals:
    void message_published(const QString& topic, const QString& message_type);
    void subscription_added(const QString& subscriber_id, const QString& message_type);
    void subscription_removed(const QString& subscriber_id);
    void service_initialized();
    void service_shutdown();

private:
    bool m_initialized{false};
    QString m_topic_prefix{"workflow.progress"};
    MessagePriority m_default_priority{MessagePriority::Normal};
    
    size_t m_published_count{0};
    size_t m_subscription_count{0};
    
    // Message bus access
    IMessageBus* get_message_bus();
    
    // Topic generation
    QString generate_workflow_topic() const;
    QString generate_step_topic() const;
    QString generate_aggregation_topic() const;
    
    // Message creation helpers
    WorkflowProgressMessage create_workflow_message(const WorkflowProgressData& progress_data) const;
    WorkflowStepProgressMessage create_step_message(
        const QString& execution_id, 
        const QString& step_id, 
        const state::WorkflowStepState& step_state) const;
    CustomDataMessage create_aggregation_message(const ProgressAggregationData& aggregation_data) const;
};

/**
 * @brief Enhanced workflow progress tracker with message bus integration
 */
class MessageBusWorkflowProgressTracker : public WorkflowProgressTracker {
    Q_OBJECT

public:
    explicit MessageBusWorkflowProgressTracker(
        const QString& execution_id,
        const QString& workflow_id,
        const QString& workflow_name = "",
        QObject* parent = nullptr);

    // Message bus service injection
    void set_message_bus_service(WorkflowProgressMessageBusService* service);
    WorkflowProgressMessageBusService* message_bus_service() const { return m_message_bus_service; }

    // Enhanced reporting with automatic message bus publishing
    void report_workflow_started();
    void report_workflow_completed(const QJsonObject& result = {});
    void report_workflow_failed(const QString& error_message);
    void report_workflow_cancelled();
    void report_workflow_suspended();
    void report_workflow_resumed();

protected:
    void publish_progress_message(const WorkflowProgressData& progress_data);
    void publish_step_progress_message(const QString& step_id, const state::WorkflowStepState& step_state);

private:
    WorkflowProgressMessageBusService* m_message_bus_service{nullptr};
};

/**
 * @brief Enhanced workflow progress aggregator with message bus integration
 */
class MessageBusWorkflowProgressAggregator : public WorkflowProgressAggregator {
    Q_OBJECT

public:
    explicit MessageBusWorkflowProgressAggregator(QObject* parent = nullptr);

    // Message bus service injection
    void set_message_bus_service(WorkflowProgressMessageBusService* service);
    WorkflowProgressMessageBusService* message_bus_service() const { return m_message_bus_service; }

    // Enhanced aggregation with automatic message bus publishing
    void update_aggregation();

protected:
    void publish_aggregation_message();

private:
    WorkflowProgressMessageBusService* m_message_bus_service{nullptr};
};

/**
 * @brief Factory for creating message bus integrated progress tracking components
 */
class WorkflowProgressFactory {
public:
    // Factory methods
    static std::unique_ptr<MessageBusWorkflowProgressTracker> create_tracker(
        const QString& execution_id,
        const QString& workflow_id,
        const QString& workflow_name = "",
        QObject* parent = nullptr);

    static std::unique_ptr<MessageBusWorkflowProgressAggregator> create_aggregator(
        QObject* parent = nullptr);

    static std::unique_ptr<WorkflowProgressMonitorManager> create_monitor_manager(
        QObject* parent = nullptr);

    // Service management
    static WorkflowProgressMessageBusService& get_message_bus_service();
    static qtplugin::expected<void, PluginError> initialize_services();
    static void shutdown_services();

private:
    static bool s_services_initialized;
};

/**
 * @brief Convenience class for easy workflow progress tracking setup
 */
class WorkflowProgressSession : public QObject {
    Q_OBJECT

public:
    explicit WorkflowProgressSession(
        const QString& execution_id,
        const QString& workflow_id,
        const QString& workflow_name = "",
        QObject* parent = nullptr);

    ~WorkflowProgressSession() override;

    // Session management
    qtplugin::expected<void, PluginError> start();
    void stop();
    bool is_active() const { return m_active; }

    // Component access
    MessageBusWorkflowProgressTracker* tracker() const { return m_tracker.get(); }
    WorkflowProgressMessageBusService* message_bus_service() const { return m_message_bus_service; }

    // Convenience methods
    void report_workflow_started() { if (m_tracker) m_tracker->report_workflow_started(); }
    void report_workflow_completed(const QJsonObject& result = {}) { 
        if (m_tracker) m_tracker->report_workflow_completed(result); 
    }
    void report_workflow_failed(const QString& error_message) { 
        if (m_tracker) m_tracker->report_workflow_failed(error_message); 
    }
    void report_step_started(const QString& step_id, const QString& step_name = "") {
        if (m_tracker) m_tracker->report_step_started(step_id, step_name);
    }
    void report_step_completed(const QString& step_id, const QJsonObject& result = {}) {
        if (m_tracker) m_tracker->report_step_completed(step_id, result);
    }
    void report_step_failed(const QString& step_id, const QString& error_message) {
        if (m_tracker) m_tracker->report_step_failed(step_id, error_message);
    }

signals:
    void session_started();
    void session_stopped();

private:
    QString m_execution_id;
    QString m_workflow_id;
    QString m_workflow_name;
    bool m_active{false};
    
    std::unique_ptr<MessageBusWorkflowProgressTracker> m_tracker;
    WorkflowProgressMessageBusService* m_message_bus_service{nullptr};
};

} // namespace qtplugin::workflow::progress
