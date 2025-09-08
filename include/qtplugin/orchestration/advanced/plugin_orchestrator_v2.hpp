/**
 * @file plugin_orchestrator_v2.hpp
 * @brief Advanced plugin orchestration and composition system
 * @version 3.2.0
 * @author QtPlugin Development Team
 *
 * This file provides advanced plugin composition patterns including:
 * - Workflow orchestration
 * - Plugin dependency graphs
 * - Event-driven composition
 * - Visual composition editor support
 * - Performance optimization
 */

#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QJsonArray>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include "../../composition/plugin_composition.hpp"
#include "../../core/dynamic_plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Plugin workflow execution modes
 */
enum class WorkflowExecutionMode {
    Sequential,   ///< Execute plugins sequentially
    Parallel,     ///< Execute plugins in parallel
    Conditional,  ///< Execute based on conditions
    EventDriven,  ///< Execute based on events
    Streaming     ///< Execute in streaming mode
};

/**
 * @brief Plugin workflow node representing a plugin in the workflow
 */
struct WorkflowNode {
    QString node_id;                    ///< Unique node identifier
    QString plugin_id;                  ///< Associated plugin identifier
    QString display_name;               ///< Display name for UI
    QJsonObject configuration;          ///< Node configuration
    QJsonObject position;               ///< Position in visual editor
    std::vector<QString> input_ports;   ///< Input port names
    std::vector<QString> output_ports;  ///< Output port names
    bool enabled{true};                 ///< Whether node is enabled

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static qtplugin::expected<WorkflowNode, PluginError> from_json(
        const QJsonObject& json);
};

/**
 * @brief Connection between workflow nodes
 */
struct WorkflowConnection {
    QString connection_id;            ///< Unique connection identifier
    QString source_node_id;           ///< Source node identifier
    QString source_port;              ///< Source port name
    QString target_node_id;           ///< Target node identifier
    QString target_port;              ///< Target port name
    QJsonObject data_transformation;  ///< Data transformation rules
    bool enabled{true};               ///< Whether connection is enabled

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static qtplugin::expected<WorkflowConnection, PluginError> from_json(
        const QJsonObject& json);
};

/**
 * @brief Plugin workflow definition
 */
struct PluginWorkflow {
    QString workflow_id;  ///< Unique workflow identifier
    QString name;         ///< Workflow name
    QString description;  ///< Workflow description
    WorkflowExecutionMode execution_mode{WorkflowExecutionMode::Sequential};
    std::vector<WorkflowNode> nodes;              ///< Workflow nodes
    std::vector<WorkflowConnection> connections;  ///< Node connections
    QJsonObject global_configuration;  ///< Global workflow configuration
    QJsonObject metadata;              ///< Additional metadata

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static qtplugin::expected<PluginWorkflow, PluginError> from_json(
        const QJsonObject& json);

    /**
     * @brief Validate workflow structure
     */
    qtplugin::expected<void, PluginError> validate() const;

    /**
     * @brief Get execution order based on dependencies
     */
    std::vector<QString> get_execution_order() const;
};

/**
 * @brief Workflow execution context
 */
struct WorkflowExecutionContext {
    QString execution_id;  ///< Unique execution identifier
    std::chrono::steady_clock::time_point start_time;
    std::unordered_map<QString, QJsonObject>
        node_outputs;  ///< Node output data
    std::unordered_map<QString, QJsonObject>
        node_states;          ///< Node execution states
    QJsonObject global_data;  ///< Global workflow data
    bool cancelled{false};    ///< Whether execution is cancelled

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
};

/**
 * @brief Advanced plugin orchestrator with workflow support
 */
class AdvancedPluginOrchestrator : public QObject {
    Q_OBJECT

public:
    explicit AdvancedPluginOrchestrator(QObject* parent = nullptr);
    ~AdvancedPluginOrchestrator() override;

    /**
     * @brief Register a workflow
     * @param workflow Workflow definition
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> register_workflow(
        const PluginWorkflow& workflow);

    /**
     * @brief Unregister a workflow
     * @param workflow_id Workflow identifier
     */
    void unregister_workflow(const QString& workflow_id);

    /**
     * @brief Execute a workflow
     * @param workflow_id Workflow identifier
     * @param input_data Input data for the workflow
     * @return Execution context or error
     */
    qtplugin::expected<QString, PluginError> execute_workflow(
        const QString& workflow_id, const QJsonObject& input_data = {});

    /**
     * @brief Cancel workflow execution
     * @param execution_id Execution identifier
     */
    void cancel_execution(const QString& execution_id);

    /**
     * @brief Get workflow execution status
     * @param execution_id Execution identifier
     * @return Execution context or error
     */
    qtplugin::expected<WorkflowExecutionContext, PluginError>
    get_execution_status(const QString& execution_id);

    /**
     * @brief Get registered workflows
     */
    std::vector<QString> get_registered_workflows() const;

    /**
     * @brief Get workflow definition
     * @param workflow_id Workflow identifier
     * @return Workflow definition or error
     */
    qtplugin::expected<PluginWorkflow, PluginError> get_workflow(
        const QString& workflow_id);

    /**
     * @brief Create workflow from composition
     * @param composition Plugin composition
     * @return Workflow definition or error
     */
    qtplugin::expected<PluginWorkflow, PluginError>
    create_workflow_from_composition(
        const qtplugin::composition::PluginComposition& composition);

    /**
     * @brief Optimize workflow for performance
     * @param workflow_id Workflow identifier
     * @return Optimized workflow or error
     */
    qtplugin::expected<PluginWorkflow, PluginError> optimize_workflow(
        const QString& workflow_id);

signals:
    /**
     * @brief Emitted when workflow execution starts
     */
    void workflow_execution_started(const QString& execution_id,
                                    const QString& workflow_id);

    /**
     * @brief Emitted when workflow execution completes
     */
    void workflow_execution_completed(const QString& execution_id,
                                      const QJsonObject& result);

    /**
     * @brief Emitted when workflow execution fails
     */
    void workflow_execution_failed(const QString& execution_id,
                                   const QString& error);

    /**
     * @brief Emitted when node execution completes
     */
    void node_execution_completed(const QString& execution_id,
                                  const QString& node_id,
                                  const QJsonObject& output);

    /**
     * @brief Emitted when workflow execution is cancelled
     */
    void workflow_execution_cancelled(const QString& execution_id);

private slots:
    void handle_node_execution_finished();
    void handle_execution_timeout();

private:
    std::unordered_map<QString, PluginWorkflow> m_workflows;
    std::unordered_map<QString, WorkflowExecutionContext> m_executions;
    std::unordered_map<QString, std::unique_ptr<QTimer>> m_execution_timers;
    mutable QMutex m_mutex;

    qtplugin::expected<void, PluginError> execute_node(
        const QString& execution_id, const QString& node_id);
    qtplugin::expected<void, PluginError> execute_sequential_workflow(
        const QString& execution_id, const PluginWorkflow& workflow);
    qtplugin::expected<void, PluginError> execute_parallel_workflow(
        const QString& execution_id, const PluginWorkflow& workflow);
    qtplugin::expected<void, PluginError> execute_conditional_workflow(
        const QString& execution_id, const PluginWorkflow& workflow);
    qtplugin::expected<void, PluginError> execute_event_driven_workflow(
        const QString& execution_id, const PluginWorkflow& workflow);
    qtplugin::expected<void, PluginError> execute_streaming_workflow(
        const QString& execution_id, const PluginWorkflow& workflow);

    QJsonObject transform_data(const QJsonObject& data,
                               const QJsonObject& transformation);
    bool evaluate_condition(const QJsonObject& condition,
                            const WorkflowExecutionContext& context);
    QString generate_execution_id();
};

/**
 * @brief Visual workflow editor for creating and editing plugin workflows
 */
class VisualWorkflowEditor : public QGraphicsView {
    Q_OBJECT

public:
    explicit VisualWorkflowEditor(QWidget* parent = nullptr);
    ~VisualWorkflowEditor() override;

    /**
     * @brief Load workflow into editor
     * @param workflow Workflow to load
     */
    void load_workflow(const PluginWorkflow& workflow);

    /**
     * @brief Get current workflow from editor
     * @return Current workflow
     */
    PluginWorkflow get_workflow() const;

    /**
     * @brief Clear the editor
     */
    void clear();

    /**
     * @brief Add a plugin node to the workflow
     * @param plugin_id Plugin identifier
     * @param position Position in the editor
     * @return Node identifier
     */
    QString add_plugin_node(const QString& plugin_id, const QPointF& position);

    /**
     * @brief Remove a node from the workflow
     * @param node_id Node identifier
     */
    void remove_node(const QString& node_id);

    /**
     * @brief Connect two nodes
     * @param source_node Source node identifier
     * @param source_port Source port name
     * @param target_node Target node identifier
     * @param target_port Target port name
     * @return Connection identifier
     */
    QString connect_nodes(const QString& source_node,
                          const QString& source_port,
                          const QString& target_node,
                          const QString& target_port);

    /**
     * @brief Remove a connection
     * @param connection_id Connection identifier
     */
    void remove_connection(const QString& connection_id);

signals:
    /**
     * @brief Emitted when workflow is modified
     */
    void workflow_modified();

    /**
     * @brief Emitted when node is selected
     */
    void node_selected(const QString& node_id);

    /**
     * @brief Emitted when connection is selected
     */
    void connection_selected(const QString& connection_id);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    std::unique_ptr<QGraphicsScene> m_scene;
    PluginWorkflow m_workflow;
    QString m_selected_node;
    QString m_selected_connection;
    bool m_dragging{false};
    QPointF m_drag_start_pos;

    void setup_scene();
    void update_visual_representation();
    QString generate_node_id();
    QString generate_connection_id();
};

}  // namespace qtplugin
