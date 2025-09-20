/**
 * @file plugin_lifecycle_manager.hpp
 * @brief Plugin lifecycle management with state transitions and event
 * notifications
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <QTimer>
#include "../utils/error_handling.hpp"
#include "../interfaces/core/plugin_interface.hpp"

// Custom state machine implementation - no QStateMachine dependency required
#include <QJsonObject>
#include <QMetaType>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace qtplugin {

/**
 * @brief Plugin lifecycle event types
 */
enum class PluginLifecycleEvent {
    BeforeInitialize,  ///< Before plugin initialization
    AfterInitialize,   ///< After plugin initialization
    BeforeShutdown,    ///< Before plugin shutdown
    AfterShutdown,     ///< After plugin shutdown
    BeforePause,       ///< Before plugin pause
    AfterPause,        ///< After plugin pause
    BeforeResume,      ///< Before plugin resume
    AfterResume,       ///< After plugin resume
    StateChanged,      ///< Plugin state changed
    Error,             ///< Plugin error occurred
    Timeout,           ///< Operation timeout
    HealthCheck,       ///< Health check event
    ResourceWarning,   ///< Resource usage warning
    DependencyChanged  ///< Plugin dependency changed
};

/**
 * @brief Plugin lifecycle configuration
 */
struct PluginLifecycleConfig {
    std::chrono::milliseconds initialization_timeout{
        30000};  ///< Initialization timeout
    std::chrono::milliseconds shutdown_timeout{10000};  ///< Shutdown timeout
    std::chrono::milliseconds pause_timeout{5000};      ///< Pause timeout
    std::chrono::milliseconds resume_timeout{5000};     ///< Resume timeout
    std::chrono::milliseconds health_check_interval{
        60000};                              ///< Health check interval
    bool enable_graceful_shutdown = true;    ///< Enable graceful shutdown
    bool enable_health_monitoring = true;    ///< Enable health monitoring
    bool enable_resource_monitoring = true;  ///< Enable resource monitoring
    bool auto_restart_on_failure = false;    ///< Auto-restart on failure
    int max_restart_attempts = 3;            ///< Maximum restart attempts
    std::chrono::milliseconds restart_delay{
        5000};                  ///< Delay between restart attempts
    QJsonObject custom_config;  ///< Custom configuration

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static PluginLifecycleConfig from_json(const QJsonObject& json);
};

/**
 * @brief Plugin lifecycle event data
 */
struct PluginLifecycleEventData {
    QString plugin_id;                                ///< Plugin identifier
    PluginLifecycleEvent event_type;                  ///< Event type
    PluginState old_state;                            ///< Previous state
    PluginState new_state;                            ///< New state
    std::chrono::system_clock::time_point timestamp;  ///< Event timestamp
    QString message;                                  ///< Event message
    QJsonObject metadata;                             ///< Additional metadata
    std::optional<PluginError> error;  ///< Error information if applicable

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static PluginLifecycleEventData from_json(const QJsonObject& json);
};

/**
 * @brief Plugin health status
 */
struct PluginHealthStatus {
    QString plugin_id;       ///< Plugin identifier
    bool is_healthy = true;  ///< Overall health status
    std::chrono::system_clock::time_point
        last_check;                              ///< Last health check time
    std::chrono::milliseconds response_time{0};  ///< Last response time
    QJsonObject metrics;                         ///< Health metrics
    std::vector<QString> warnings;               ///< Health warnings
    std::vector<QString> errors;                 ///< Health errors

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Plugin lifecycle event callback
 */
using PluginLifecycleEventCallback =
    std::function<void(const PluginLifecycleEventData&)>;

/**
 * @brief Plugin health check callback
 */
using PluginHealthCheckCallback =
    std::function<PluginHealthStatus(const QString& plugin_id)>;

/**
 * @brief Custom plugin state machine for lifecycle management
 *
 * This class provides thread-safe state management without QStateMachine
 * dependency. It manages state transitions, validates transitions, and provides
 * event notifications.
 */
class PluginStateMachine {
public:
    /**
     * @brief State transition callback type
     */
    using StateTransitionCallback =
        std::function<void(PluginState old_state, PluginState new_state)>;

    /**
     * @brief Constructor
     * @param plugin_id Plugin identifier for this state machine
     */
    explicit PluginStateMachine(const QString& plugin_id);

    /**
     * @brief Get current state (thread-safe)
     * @return Current plugin state
     */
    PluginState current_state() const;

    /**
     * @brief Attempt state transition (thread-safe)
     * @param new_state Target state
     * @return Success or error if transition is invalid
     */
    qtplugin::expected<void, PluginError> transition_to(PluginState new_state);

    /**
     * @brief Check if transition is valid
     * @param from_state Source state
     * @param to_state Target state
     * @return true if transition is allowed
     */
    static bool is_valid_transition(PluginState from_state,
                                    PluginState to_state);

    /**
     * @brief Set state transition callback
     * @param callback Callback to invoke on state changes
     */
    void set_transition_callback(StateTransitionCallback callback);

    /**
     * @brief Reset to initial state
     */
    void reset();

private:
    QString m_plugin_id;
    mutable std::mutex m_state_mutex;
    std::atomic<PluginState> m_current_state{PluginState::Unloaded};
    StateTransitionCallback m_transition_callback;

    void notify_transition(PluginState old_state, PluginState new_state);
};

/**
 * @brief Plugin lifecycle manager
 *
 * This class manages the complete lifecycle of plugins including state
 * transitions, event notifications, health monitoring, and graceful shutdown
 * procedures.
 */
class PluginLifecycleManager : public QObject {
    Q_OBJECT

public:
    explicit PluginLifecycleManager(QObject* parent = nullptr);
    ~PluginLifecycleManager() override;

    // === Configuration ===

    /**
     * @brief Set lifecycle configuration for a plugin
     * @param plugin_id Plugin identifier
     * @param config Lifecycle configuration
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_plugin_config(
        const QString& plugin_id, const PluginLifecycleConfig& config);

    /**
     * @brief Get lifecycle configuration for a plugin
     * @param plugin_id Plugin identifier
     * @return Lifecycle configuration or error
     */
    qtplugin::expected<PluginLifecycleConfig, PluginError> get_plugin_config(
        const QString& plugin_id) const;

    /**
     * @brief Set default lifecycle configuration
     * @param config Default configuration
     */
    void set_default_config(const PluginLifecycleConfig& config);

    /**
     * @brief Get default lifecycle configuration
     * @return Default configuration
     */
    PluginLifecycleConfig get_default_config() const;

    // === Plugin Registration ===

    /**
     * @brief Register plugin for lifecycle management
     * @param plugin Plugin to register
     * @param config Optional lifecycle configuration
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> register_plugin(
        std::shared_ptr<IPlugin> plugin,
        const PluginLifecycleConfig& config = {});

    /**
     * @brief Unregister plugin from lifecycle management
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unregister_plugin(
        const QString& plugin_id);

    /**
     * @brief Check if plugin is registered
     * @param plugin_id Plugin identifier
     * @return true if plugin is registered
     */
    bool is_plugin_registered(const QString& plugin_id) const;

    /**
     * @brief Get registered plugins
     * @return Vector of registered plugin IDs
     */
    std::vector<QString> get_registered_plugins() const;

    // === Lifecycle Operations ===

    /**
     * @brief Initialize plugin with lifecycle management
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> initialize_plugin(
        const QString& plugin_id);

    /**
     * @brief Shutdown plugin with graceful handling
     * @param plugin_id Plugin identifier
     * @param force Force shutdown if graceful fails
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> shutdown_plugin(
        const QString& plugin_id, bool force = false);

    /**
     * @brief Pause plugin execution
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> pause_plugin(
        const QString& plugin_id);

    /**
     * @brief Resume plugin execution
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> resume_plugin(
        const QString& plugin_id);

    /**
     * @brief Restart plugin
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> restart_plugin(
        const QString& plugin_id);

    // === State Management ===

    /**
     * @brief Get plugin state
     * @param plugin_id Plugin identifier
     * @return Plugin state or error
     */
    qtplugin::expected<PluginState, PluginError> get_plugin_state(
        const QString& plugin_id) const;

    /**
     * @brief Check if plugin can transition to state
     * @param plugin_id Plugin identifier
     * @param target_state Target state
     * @return true if transition is allowed
     */
    bool can_transition_to_state(const QString& plugin_id,
                                 PluginState target_state) const;

    /**
     * @brief Get plugin state history
     * @param plugin_id Plugin identifier
     * @param max_entries Maximum number of history entries
     * @return Vector of lifecycle events
     */
    std::vector<PluginLifecycleEventData> get_plugin_state_history(
        const QString& plugin_id, int max_entries = 100) const;

    // === Health Monitoring ===

    /**
     * @brief Enable health monitoring for plugin
     * @param plugin_id Plugin identifier
     * @param health_check_callback Custom health check callback
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> enable_health_monitoring(
        const QString& plugin_id,
        PluginHealthCheckCallback health_check_callback = nullptr);

    /**
     * @brief Disable health monitoring for plugin
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> disable_health_monitoring(
        const QString& plugin_id);

    /**
     * @brief Perform health check on plugin
     * @param plugin_id Plugin identifier
     * @return Health status or error
     */
    qtplugin::expected<PluginHealthStatus, PluginError> check_plugin_health(
        const QString& plugin_id);

    /**
     * @brief Get plugin health status
     * @param plugin_id Plugin identifier
     * @return Health status or error
     */
    qtplugin::expected<PluginHealthStatus, PluginError>
    get_plugin_health_status(const QString& plugin_id) const;

    // === Event Management ===

    /**
     * @brief Register lifecycle event callback
     * @param plugin_id Plugin identifier (empty for all plugins)
     * @param event_type Event type filter
     * @param callback Event callback
     * @return Callback ID for unregistration
     */
    QString register_event_callback(const QString& plugin_id,
                                    PluginLifecycleEvent event_type,
                                    PluginLifecycleEventCallback callback);

    /**
     * @brief Unregister lifecycle event callback
     * @param callback_id Callback identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unregister_event_callback(
        const QString& callback_id);

    // === Batch Operations ===

    /**
     * @brief Initialize multiple plugins
     * @param plugin_ids Vector of plugin identifiers
     * @return Vector of results
     */
    std::vector<qtplugin::expected<void, PluginError>> initialize_plugins(
        const std::vector<QString>& plugin_ids);

    /**
     * @brief Shutdown multiple plugins
     * @param plugin_ids Vector of plugin identifiers
     * @param force Force shutdown if graceful fails
     * @return Vector of results
     */
    std::vector<qtplugin::expected<void, PluginError>> shutdown_plugins(
        const std::vector<QString>& plugin_ids, bool force = false);

    // === Enhanced Resource Management and State Migration ===

    /**
     * @brief Migrate plugin state with new data
     * @param plugin_id Plugin identifier
     * @param state_data State data to migrate
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> migrate_plugin_state(
        const QString& plugin_id, const QJsonObject& state_data);

    /**
     * @brief Backup plugin state to JSON
     * @param plugin_id Plugin identifier
     * @return Backup data or error
     */
    qtplugin::expected<QJsonObject, PluginError> backup_plugin_state(
        const QString& plugin_id);

    /**
     * @brief Restore plugin state from backup
     * @param plugin_id Plugin identifier
     * @param backup_data Backup data to restore
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> restore_plugin_state(
        const QString& plugin_id, const QJsonObject& backup_data);

    /**
     * @brief Clean up plugin resources and memory
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> cleanup_plugin_resources(
        const QString& plugin_id);

    /**
     * @brief Gracefully shutdown all registered plugins
     * @param timeout Maximum time to wait for graceful shutdown
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> shutdown_all_plugins_gracefully(
        std::chrono::milliseconds timeout = std::chrono::seconds(30));

    /**
     * @brief Get lifecycle management statistics
     * @return Statistics as JSON object
     */
    QJsonObject get_lifecycle_statistics() const;

signals:
    /**
     * @brief Emitted when plugin lifecycle event occurs
     * @param event_data Event data
     */
    void lifecycle_event(const PluginLifecycleEventData& event_data);

    /**
     * @brief Emitted when plugin state changes
     * @param plugin_id Plugin identifier
     * @param old_state Previous state
     * @param new_state New state
     */
    void plugin_state_changed(const QString& plugin_id, PluginState old_state,
                              PluginState new_state);

    /**
     * @brief Emitted when plugin health status changes
     * @param plugin_id Plugin identifier
     * @param health_status Health status
     */
    void plugin_health_changed(const QString& plugin_id,
                               const PluginHealthStatus& health_status);

private slots:
    void on_health_check_timer();
    void on_operation_timeout();

private:
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::PluginLifecycleEvent)
Q_DECLARE_METATYPE(qtplugin::PluginLifecycleConfig)
Q_DECLARE_METATYPE(qtplugin::PluginLifecycleEventData)
Q_DECLARE_METATYPE(qtplugin::PluginHealthStatus)
