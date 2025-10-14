/**
 * @file progress_message_bus.hpp
 * @brief Message bus integration for workflow progress tracking
 * @version 3.1.0
 */

#pragma once

#include <QObject>
#include <QString>
#include <functional>
#include <memory>

#include "../communication/message_bus.hpp"
#include "../communication/message_types.hpp"
#include "../utils/error_handling.hpp"
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
    [[nodiscard]] qtplugin::expected<void, PluginError> initialize();
    void shutdown();
    [[nodiscard]] bool is_initialized() const { return m_initialized_; }

    // Publishing
    [[nodiscard]] qtplugin::expected<void, PluginError>
    publish_workflow_progress(const WorkflowProgressData& progress_data);
    [[nodiscard]] qtplugin::expected<void, PluginError> publish_step_progress(
        const QString& execution_id, const QString& step_id,
        const state::WorkflowStepState& step_state);
    [[nodiscard]] qtplugin::expected<void, PluginError>
    publish_aggregation_update(const ProgressAggregationData& aggregation_data);

    // Subscription management
    [[nodiscard]] qtplugin::expected<void, PluginError>
    subscribe_to_workflow_progress(
        const QString& subscriber_id,
        const std::function<void(const WorkflowProgressMessage&)>& handler);

    [[nodiscard]] qtplugin::expected<void, PluginError>
    subscribe_to_step_progress(
        const QString& subscriber_id,
        const std::function<void(const WorkflowStepProgressMessage&)>& handler);

    [[nodiscard]] qtplugin::expected<void, PluginError>
    subscribe_to_aggregation_updates(
        const QString& subscriber_id,
        const std::function<void(const qtplugin::messages::CustomDataMessage&)>&
            handler);

    [[nodiscard]] qtplugin::expected<void, PluginError> unsubscribe(
        const QString& subscriber_id);

    // Configuration
    void set_topic_prefix(const QString& prefix) { m_topic_prefix_ = prefix; }
    [[nodiscard]] const QString& topic_prefix() const {
        return m_topic_prefix_;
    }

    void set_default_priority(MessagePriority priority) {
        m_default_priority_ = priority;
    }
    [[nodiscard]] MessagePriority default_priority() const {
        return m_default_priority_;
    }

    // Statistics
    [[nodiscard]] size_t published_message_count() const {
        return m_published_count_;
    }
    [[nodiscard]] size_t subscription_count() const {
        return m_subscription_count_;
    }

    // Singleton access
    [[nodiscard]] static WorkflowProgressMessageBusService& instance();

signals:
    void message_published(const QString& topic, const QString& message_type);
    void subscription_added(const QString& subscriber_id,
                            const QString& message_type);
    void subscription_removed(const QString& subscriber_id);
    void service_initialized();
    void service_shutdown();

private:
    bool m_initialized_{false};
    QString m_topic_prefix_{"workflow.progress"};
    MessagePriority m_default_priority_{MessagePriority::Normal};

    size_t m_published_count_{0};
    size_t m_subscription_count_{0};

    // Message bus access
    IMessageBus* get_message_bus();

    // Topic generation
    QString generate_workflow_topic() const;
    QString generate_step_topic() const;
    QString generate_aggregation_topic() const;

    // Message creation helpers
    WorkflowProgressMessage create_workflow_message(
        const WorkflowProgressData& progress_data) const;
    WorkflowStepProgressMessage create_step_message(
        const QString& execution_id, const QString& step_id,
        const state::WorkflowStepState& step_state) const;
    qtplugin::messages::CustomDataMessage create_aggregation_message(
        const ProgressAggregationData& aggregation_data) const;
};

/**
 * @brief Enhanced workflow progress tracker with message bus integration
 */
class MessageBusWorkflowProgressTracker : public WorkflowProgressTracker {
    Q_OBJECT

public:
    explicit MessageBusWorkflowProgressTracker(
        const QString& execution_id, const QString& workflow_id,
        const QString& workflow_name = "", QObject* parent = nullptr);

    // Message bus service injection
    void set_message_bus_service(WorkflowProgressMessageBusService* service);
    [[nodiscard]] WorkflowProgressMessageBusService* message_bus_service()
        const {
        return m_message_bus_service_;
    }

    // Enhanced reporting with automatic message bus publishing
    void report_workflow_started();
    void report_workflow_completed(const QJsonObject& result = {});
    void report_workflow_failed(const QString& error_message);
    void report_workflow_cancelled();
    void report_workflow_suspended();
    void report_workflow_resumed();

protected:
    void publish_progress_message(const WorkflowProgressData& progress_data);
    void publish_step_progress_message(
        const QString& step_id, const state::WorkflowStepState& step_state);

private:
    WorkflowProgressMessageBusService* m_message_bus_service_{nullptr};
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
    [[nodiscard]] WorkflowProgressMessageBusService* message_bus_service()
        const {
        return m_message_bus_service_;
    }

    // Enhanced aggregation with automatic message bus publishing
    void update_aggregation();

protected:
    void publish_aggregation_message();

private:
    WorkflowProgressMessageBusService* m_message_bus_service_{nullptr};
};

/**
 * @brief Factory for creating message bus integrated progress tracking
 * components
 */
class WorkflowProgressFactory {
public:
    // Factory methods
    static std::unique_ptr<MessageBusWorkflowProgressTracker> create_tracker(
        const QString& execution_id, const QString& workflow_id,
        const QString& workflow_name = "", QObject* parent = nullptr);

    static std::unique_ptr<MessageBusWorkflowProgressAggregator>
    create_aggregator(QObject* parent = nullptr);

    static std::unique_ptr<WorkflowProgressMonitorManager>
    create_monitor_manager(QObject* parent = nullptr);

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
    explicit WorkflowProgressSession(const QString& execution_id,
                                     const QString& workflow_id,
                                     const QString& workflow_name = "",
                                     QObject* parent = nullptr);

    ~WorkflowProgressSession() override;

    // Session management
    [[nodiscard]] qtplugin::expected<void, PluginError> start();
    void stop();
    [[nodiscard]] bool is_active() const { return m_active_; }

    // Component access
    [[nodiscard]] MessageBusWorkflowProgressTracker* tracker() const {
        return m_tracker_.get();
    }
    [[nodiscard]] WorkflowProgressMessageBusService* message_bus_service()
        const {
        return m_message_bus_service_;
    }

    // Convenience methods
    void report_workflow_started() {
        if (m_tracker_)
            m_tracker_->report_workflow_started();
    }
    void report_workflow_completed(const QJsonObject& result = {}) {
        if (m_tracker_)
            m_tracker_->report_workflow_completed(result);
    }
    void report_workflow_failed(const QString& error_message) {
        if (m_tracker_)
            m_tracker_->report_workflow_failed(error_message);
    }
    void report_step_started(const QString& step_id,
                             const QString& step_name = "") {
        if (m_tracker_)
            m_tracker_->report_step_started(step_id, step_name);
    }
    void report_step_completed(const QString& step_id,
                               const QJsonObject& result = {}) {
        if (m_tracker_)
            m_tracker_->report_step_completed(step_id, result);
    }
    void report_step_failed(const QString& step_id,
                            const QString& error_message) {
        if (m_tracker_)
            m_tracker_->report_step_failed(step_id, error_message);
    }

signals:
    void session_started();
    void session_stopped();

private:
    QString m_execution_id_;
    QString m_workflow_id_;
    QString m_workflow_name_;
    bool m_active_{false};

    std::unique_ptr<MessageBusWorkflowProgressTracker> m_tracker_;
    WorkflowProgressMessageBusService* m_message_bus_service_{nullptr};
};

}  // namespace qtplugin::workflow::progress
