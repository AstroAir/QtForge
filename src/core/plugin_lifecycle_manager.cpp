/**
 * @file plugin_lifecycle_manager.cpp
 * @brief Implementation of advanced plugin lifecycle management
 * @version 3.0.0
 */

#include "../../include/qtplugin/core/plugin_lifecycle_manager.hpp"
#include <QDateTime>
#include <QJsonArray>
#include <QLoggingCategory>
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>
#include <QTimer>
#include <QUuid>
#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <future>

Q_LOGGING_CATEGORY(pluginLifecycleLog, "qtplugin.plugin.lifecycle")

namespace qtplugin {

// === PluginLifecycleConfig Implementation ===

QJsonObject PluginLifecycleConfig::to_json() const {
    QJsonObject json;
    json["initialization_timeout"] =
        static_cast<qint64>(initialization_timeout.count());
    json["shutdown_timeout"] = static_cast<qint64>(shutdown_timeout.count());
    json["pause_timeout"] = static_cast<qint64>(pause_timeout.count());
    json["resume_timeout"] = static_cast<qint64>(resume_timeout.count());
    json["health_check_interval"] =
        static_cast<qint64>(health_check_interval.count());
    json["enable_graceful_shutdown"] = enable_graceful_shutdown;
    json["enable_health_monitoring"] = enable_health_monitoring;
    json["enable_resource_monitoring"] = enable_resource_monitoring;
    json["auto_restart_on_failure"] = auto_restart_on_failure;
    json["max_restart_attempts"] = max_restart_attempts;
    json["restart_delay"] = static_cast<qint64>(restart_delay.count());
    json["custom_config"] = custom_config;
    return json;
}

PluginLifecycleConfig PluginLifecycleConfig::from_json(
    const QJsonObject& json) {
    PluginLifecycleConfig config;
    config.initialization_timeout =
        std::chrono::milliseconds(json["initialization_timeout"].toInt(30000));
    config.shutdown_timeout =
        std::chrono::milliseconds(json["shutdown_timeout"].toInt(10000));
    config.pause_timeout =
        std::chrono::milliseconds(json["pause_timeout"].toInt(5000));
    config.resume_timeout =
        std::chrono::milliseconds(json["resume_timeout"].toInt(5000));
    config.health_check_interval =
        std::chrono::milliseconds(json["health_check_interval"].toInt(60000));
    config.enable_graceful_shutdown =
        json["enable_graceful_shutdown"].toBool(true);
    config.enable_health_monitoring =
        json["enable_health_monitoring"].toBool(true);
    config.enable_resource_monitoring =
        json["enable_resource_monitoring"].toBool(true);
    config.auto_restart_on_failure =
        json["auto_restart_on_failure"].toBool(false);
    config.max_restart_attempts = json["max_restart_attempts"].toInt(3);
    config.restart_delay =
        std::chrono::milliseconds(json["restart_delay"].toInt(5000));
    config.custom_config = json["custom_config"].toObject();
    return config;
}

// === PluginLifecycleEventData Implementation ===

QJsonObject PluginLifecycleEventData::to_json() const {
    QJsonObject json;
    json["plugin_id"] = plugin_id;
    json["event_type"] = static_cast<int>(event_type);
    json["old_state"] = static_cast<int>(old_state);
    json["new_state"] = static_cast<int>(new_state);
    json["timestamp"] = QDateTime::fromSecsSinceEpoch(
                            std::chrono::duration_cast<std::chrono::seconds>(
                                timestamp.time_since_epoch())
                                .count())
                            .toString(Qt::ISODate);
    json["message"] = message;
    json["metadata"] = metadata;

    if (error.has_value()) {
        QJsonObject error_json;
        error_json["code"] = static_cast<int>(error->code);
        error_json["message"] = QString::fromStdString(error->message);
        json["error"] = error_json;
    }

    return json;
}

PluginLifecycleEventData PluginLifecycleEventData::from_json(
    const QJsonObject& json) {
    PluginLifecycleEventData data;
    data.plugin_id = json["plugin_id"].toString();
    data.event_type =
        static_cast<PluginLifecycleEvent>(json["event_type"].toInt());
    data.old_state = static_cast<PluginState>(json["old_state"].toInt());
    data.new_state = static_cast<PluginState>(json["new_state"].toInt());

    auto timestamp_str = json["timestamp"].toString();
    auto datetime = QDateTime::fromString(timestamp_str, Qt::ISODate);
    data.timestamp =
        std::chrono::system_clock::from_time_t(datetime.toSecsSinceEpoch());

    data.message = json["message"].toString();
    data.metadata = json["metadata"].toObject();

    if (json.contains("error")) {
        auto error_json = json["error"].toObject();
        PluginError plugin_error;
        plugin_error.code =
            static_cast<PluginErrorCode>(error_json["code"].toInt());
        plugin_error.message = error_json["message"].toString().toStdString();
        data.error = plugin_error;
    }

    return data;
}

// === PluginHealthStatus Implementation ===

QJsonObject PluginHealthStatus::to_json() const {
    QJsonObject json;
    json["plugin_id"] = plugin_id;
    json["is_healthy"] = is_healthy;
    json["last_check"] = QDateTime::fromSecsSinceEpoch(
                             std::chrono::duration_cast<std::chrono::seconds>(
                                 last_check.time_since_epoch())
                                 .count())
                             .toString(Qt::ISODate);
    json["response_time"] = static_cast<qint64>(response_time.count());
    json["metrics"] = metrics;
    QStringList warnings_list(warnings.begin(), warnings.end());
    QStringList errors_list(errors.begin(), errors.end());
    json["warnings"] = QJsonArray::fromStringList(warnings_list);
    json["errors"] = QJsonArray::fromStringList(errors_list);
    return json;
}

// === PluginStateMachine Implementation ===

PluginStateMachine::PluginStateMachine(const QString& plugin_id)
    : m_plugin_id(plugin_id) {
    qCDebug(pluginLifecycleLog)
        << "Created state machine for plugin:" << plugin_id;
}

PluginState PluginStateMachine::current_state() const {
    return m_current_state.load();
}

qtplugin::expected<void, PluginError> PluginStateMachine::transition_to(
    PluginState new_state) {
    std::lock_guard<std::mutex> lock(m_state_mutex);

    PluginState old_state = m_current_state.load();

    // Check if transition is valid
    if (!is_valid_transition(old_state, new_state)) {
        QString error_msg =
            QString("Invalid state transition from %1 to %2 for plugin %3")
                .arg(static_cast<int>(old_state))
                .arg(static_cast<int>(new_state))
                .arg(m_plugin_id);
        return make_error<void>(PluginErrorCode::InvalidState,
                                error_msg.toStdString());
    }

    // Perform the transition
    m_current_state.store(new_state);

    qCDebug(pluginLifecycleLog)
        << "Plugin" << m_plugin_id << "transitioned from state"
        << static_cast<int>(old_state) << "to" << static_cast<int>(new_state);

    // Notify callback if set
    notify_transition(old_state, new_state);

    return {};
}

bool PluginStateMachine::is_valid_transition(PluginState from_state,
                                             PluginState to_state) {
    // Define valid state transitions based on plugin lifecycle
    switch (from_state) {
        case PluginState::Unloaded:
            return to_state == PluginState::Loading;

        case PluginState::Loading:
            return to_state == PluginState::Loaded ||
                   to_state == PluginState::Error;

        case PluginState::Loaded:
            return to_state == PluginState::Initializing ||
                   to_state == PluginState::Error;

        case PluginState::Initializing:
            return to_state == PluginState::Running ||
                   to_state == PluginState::Error;

        case PluginState::Running:
            return to_state == PluginState::Paused ||
                   to_state == PluginState::Stopping ||
                   to_state == PluginState::Error;

        case PluginState::Paused:
            return to_state == PluginState::Running ||
                   to_state == PluginState::Stopping ||
                   to_state == PluginState::Error;

        case PluginState::Stopping:
            return to_state == PluginState::Stopped;

        case PluginState::Stopped:
            return to_state == PluginState::Unloaded;

        case PluginState::Error:
            return to_state == PluginState::Reloading;

        case PluginState::Reloading:
            return to_state == PluginState::Loaded ||
                   to_state == PluginState::Error;
    }

    return false;
}

void PluginStateMachine::set_transition_callback(
    StateTransitionCallback callback) {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    m_transition_callback = std::move(callback);
}

void PluginStateMachine::reset() {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    PluginState old_state = m_current_state.load();
    m_current_state.store(PluginState::Unloaded);

    qCDebug(pluginLifecycleLog)
        << "Reset state machine for plugin:" << m_plugin_id;

    if (old_state != PluginState::Unloaded) {
        notify_transition(old_state, PluginState::Unloaded);
    }
}

void PluginStateMachine::notify_transition(PluginState old_state,
                                           PluginState new_state) {
    if (m_transition_callback) {
        try {
            m_transition_callback(old_state, new_state);
        } catch (const std::exception& e) {
            qCWarning(pluginLifecycleLog)
                << "Exception in state transition callback for plugin"
                << m_plugin_id << ":" << e.what();
        } catch (...) {
            qCWarning(pluginLifecycleLog)
                << "Unknown exception in state transition callback for plugin"
                << m_plugin_id;
        }
    }
}

// === PluginLifecycleManager Private Implementation ===

struct PluginLifecycleInfo {
    std::shared_ptr<IPlugin> plugin;
    PluginLifecycleConfig config;
    std::unique_ptr<PluginStateMachine> state_machine;
    std::vector<PluginLifecycleEventData> event_history;
    PluginHealthStatus health_status;
    PluginHealthCheckCallback health_check_callback;
    std::unique_ptr<QTimer> health_check_timer;
    std::unique_ptr<QTimer> operation_timeout_timer;
    int restart_attempts = 0;
    std::chrono::system_clock::time_point last_restart_time;
    bool health_monitoring_enabled = false;
};

struct LifecycleEventCallback {
    QString id;
    QString plugin_id_filter;
    PluginLifecycleEvent event_type;
    PluginLifecycleEventCallback callback;
};

class PluginLifecycleManager::Private {
public:
    explicit Private(PluginLifecycleManager* parent) : q_ptr(parent) {}

    mutable QMutex mutex;
    PluginLifecycleConfig default_config;
    std::unordered_map<QString, std::unique_ptr<PluginLifecycleInfo>> plugins;
    std::unordered_map<QString, LifecycleEventCallback> event_callbacks;
    PluginLifecycleManager* q_ptr;

    void create_state_machine(PluginLifecycleInfo* info);
    void emit_lifecycle_event(const PluginLifecycleEventData& event_data);
    void perform_health_check(const QString& plugin_id);
    void handle_plugin_error(const QString& plugin_id,
                             const PluginError& error);
    bool should_auto_restart(const QString& plugin_id);
    void schedule_restart(const QString& plugin_id);
};

void PluginLifecycleManager::Private::create_state_machine(
    PluginLifecycleInfo* info) {
    if (!info || !info->plugin)
        return;

    // Create our custom state machine
    QString plugin_id = QString::fromStdString(info->plugin->id());
    info->state_machine = std::make_unique<PluginStateMachine>(plugin_id);

    // Set up state transition callback to emit lifecycle events
    info->state_machine->set_transition_callback(
        [this, plugin_id](PluginState old_state, PluginState new_state) {
            // Create lifecycle event data
            PluginLifecycleEventData event_data;
            event_data.plugin_id = plugin_id;
            event_data.event_type = PluginLifecycleEvent::StateChanged;
            event_data.old_state = old_state;
            event_data.new_state = new_state;
            event_data.timestamp = std::chrono::system_clock::now();
            event_data.message = QString("State changed from %1 to %2")
                                     .arg(static_cast<int>(old_state))
                                     .arg(static_cast<int>(new_state));

            // Emit the lifecycle event
            emit_lifecycle_event(event_data);

            // TODO: Re-enable when MOC issues are resolved
            // emit q_ptr->plugin_state_changed(plugin_id, old_state, new_state);
        });

    qCDebug(pluginLifecycleLog)
        << "Created custom state machine for plugin:" << info->plugin->id();
}

void PluginLifecycleManager::Private::emit_lifecycle_event(
    const PluginLifecycleEventData& event_data) {
    // Store event in history
    auto it = plugins.find(event_data.plugin_id);
    if (it != plugins.end()) {
        auto& history = it->second->event_history;
        history.push_back(event_data);

        // Limit history size
        const size_t max_history_size = 1000;
        if (history.size() > max_history_size) {
            history.erase(
                history.begin(),
                history.begin() + (history.size() - max_history_size));
        }
    }

    // Notify callbacks
    for (const auto& [callback_id, callback_info] : event_callbacks) {
        bool should_notify = false;

        // Check plugin filter
        if (callback_info.plugin_id_filter.isEmpty() ||
            callback_info.plugin_id_filter == event_data.plugin_id) {
            // Check event type filter
            if (callback_info.event_type == event_data.event_type) {
                should_notify = true;
            }
        }

        if (should_notify && callback_info.callback) {
            try {
                callback_info.callback(event_data);
            } catch (const std::exception& e) {
                qCWarning(pluginLifecycleLog)
                    << "Exception in lifecycle event callback:" << e.what();
            } catch (...) {
                qCWarning(pluginLifecycleLog)
                    << "Unknown exception in lifecycle event callback";
            }
        }
    }
}

void PluginLifecycleManager::Private::perform_health_check(
    const QString& plugin_id) {
    auto it = plugins.find(plugin_id);
    if (it == plugins.end() || !it->second->health_monitoring_enabled) {
        return;
    }

    auto& info = it->second;
    auto start_time = std::chrono::steady_clock::now();

    PluginHealthStatus health_status;
    health_status.plugin_id = plugin_id;
    health_status.last_check = std::chrono::system_clock::now();

    try {
        if (info->health_check_callback) {
            // Use custom health check
            health_status = info->health_check_callback(plugin_id);
        } else {
            // Default health check - just check if plugin is responsive
            health_status.is_healthy =
                (info->plugin->state() == PluginState::Running);
        }

        auto end_time = std::chrono::steady_clock::now();
        health_status.response_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                  start_time);

    } catch (const std::exception& e) {
        health_status.is_healthy = false;
        health_status.errors.push_back(
            QString("Health check exception: %1").arg(e.what()));
    } catch (...) {
        health_status.is_healthy = false;
        health_status.errors.push_back("Unknown health check exception");
    }

    // Update stored health status
    info->health_status = health_status;

    // Emit health change event if status changed
    static std::unordered_map<QString, bool> previous_health_status;
    bool previous_healthy = previous_health_status[plugin_id];

    if (previous_healthy != health_status.is_healthy) {
        previous_health_status[plugin_id] = health_status.is_healthy;

        PluginLifecycleEventData event_data;
        event_data.plugin_id = plugin_id;
        event_data.event_type = PluginLifecycleEvent::HealthCheck;
        event_data.old_state = info->plugin->state();
        event_data.new_state = info->plugin->state();
        event_data.timestamp = std::chrono::system_clock::now();
        event_data.message = health_status.is_healthy
                                 ? "Plugin is healthy"
                                 : "Plugin health check failed";
        event_data.metadata["health_status"] = health_status.to_json();

        emit_lifecycle_event(event_data);
    }
}

void PluginLifecycleManager::Private::handle_plugin_error(
    const QString& plugin_id, const PluginError& error) {
    auto it = plugins.find(plugin_id);
    if (it == plugins.end())
        return;

    auto& info = it->second;

    // Create error event
    PluginLifecycleEventData event_data;
    event_data.plugin_id = plugin_id;
    event_data.event_type = PluginLifecycleEvent::Error;
    event_data.old_state = info->plugin->state();
    event_data.new_state = PluginState::Error;
    event_data.timestamp = std::chrono::system_clock::now();
    event_data.message = QString::fromStdString(error.message);
    event_data.error = error;

    emit_lifecycle_event(event_data);

    // Check if auto-restart is enabled and should be attempted
    if (should_auto_restart(plugin_id)) {
        schedule_restart(plugin_id);
    }
}

bool PluginLifecycleManager::Private::should_auto_restart(
    const QString& plugin_id) {
    auto it = plugins.find(plugin_id);
    if (it == plugins.end())
        return false;

    auto& info = it->second;

    if (!info->config.auto_restart_on_failure) {
        return false;
    }

    if (info->restart_attempts >= info->config.max_restart_attempts) {
        return false;
    }

    // Check if enough time has passed since last restart
    auto now = std::chrono::system_clock::now();
    auto time_since_restart = now - info->last_restart_time;

    if (time_since_restart < info->config.restart_delay) {
        return false;
    }

    return true;
}

void PluginLifecycleManager::Private::schedule_restart(
    const QString& plugin_id) {
    auto it = plugins.find(plugin_id);
    if (it == plugins.end())
        return;

    auto& info = it->second;

    // Create restart timer
    auto* restart_timer = new QTimer();
    restart_timer->setSingleShot(true);
    restart_timer->setInterval(
        static_cast<int>(info->config.restart_delay.count()));

    QObject::connect(
        restart_timer, &QTimer::timeout, [this, plugin_id, restart_timer]() {
            // Attempt restart
            auto it = plugins.find(plugin_id);
            if (it != plugins.end()) {
                auto& info = it->second;
                info->restart_attempts++;
                info->last_restart_time = std::chrono::system_clock::now();

                qCInfo(pluginLifecycleLog)
                    << "Attempting auto-restart for plugin:" << plugin_id
                    << "attempt:" << info->restart_attempts;

                // Try to restart the plugin
                try {
                    info->plugin->shutdown();
                    auto init_result = info->plugin->initialize();

                    if (init_result) {
                        qCInfo(pluginLifecycleLog)
                            << "Auto-restart successful for plugin:"
                            << plugin_id;
                        info->restart_attempts = 0;  // Reset on success
                    } else {
                        qCWarning(pluginLifecycleLog)
                            << "Auto-restart failed for plugin:" << plugin_id
                            << "error:" << init_result.error().message.c_str();
                    }
                } catch (const std::exception& e) {
                    qCWarning(pluginLifecycleLog)
                        << "Exception during auto-restart for plugin:"
                        << plugin_id << "error:" << e.what();
                }
            }

            restart_timer->deleteLater();
        });

    restart_timer->start();
}

// === PluginLifecycleManager Implementation ===

PluginLifecycleManager::PluginLifecycleManager(QObject* parent)
    : QObject(parent), d(std::make_unique<Private>(this)) {
    // Set up default configuration
    d->default_config = PluginLifecycleConfig{};

    qCDebug(pluginLifecycleLog) << "Plugin lifecycle manager initialized";
}

PluginLifecycleManager::~PluginLifecycleManager() = default;

qtplugin::expected<void, PluginError> PluginLifecycleManager::set_plugin_config(
    const QString& plugin_id, const PluginLifecycleConfig& config) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    it->second->config = config;

    // Update health monitoring if needed
    if (it->second->health_check_timer) {
        it->second->health_check_timer->setInterval(
            static_cast<int>(config.health_check_interval.count()));
    }

    qCDebug(pluginLifecycleLog)
        << "Updated configuration for plugin:" << plugin_id;
    return make_success();
}

qtplugin::expected<PluginLifecycleConfig, PluginError>
PluginLifecycleManager::get_plugin_config(const QString& plugin_id) const {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<PluginLifecycleConfig>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    return it->second->config;
}

void PluginLifecycleManager::set_default_config(
    const PluginLifecycleConfig& config) {
    QMutexLocker locker(&d->mutex);
    d->default_config = config;
    qCDebug(pluginLifecycleLog) << "Updated default lifecycle configuration";
}

PluginLifecycleConfig PluginLifecycleManager::get_default_config() const {
    QMutexLocker locker(&d->mutex);
    return d->default_config;
}

qtplugin::expected<void, PluginError> PluginLifecycleManager::register_plugin(
    std::shared_ptr<IPlugin> plugin, const PluginLifecycleConfig& config) {
    if (!plugin) {
        return make_error<void>(PluginErrorCode::InvalidArgument,
                                "Plugin is null");
    }

    QString plugin_id = QString::fromStdString(plugin->id());

    QMutexLocker locker(&d->mutex);

    // Check if already registered
    if (d->plugins.find(plugin_id) != d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::AlreadyExists,
            "Plugin already registered: " + plugin_id.toStdString());
    }

    // Create plugin lifecycle info
    auto info = std::make_unique<PluginLifecycleInfo>();
    info->plugin = plugin;
    info->config = config;
    info->health_status.plugin_id = plugin_id;
    info->health_status.last_check = std::chrono::system_clock::now();

    // Create state machine
    d->create_state_machine(info.get());

    // Set up health monitoring if enabled
    if (config.enable_health_monitoring) {
        info->health_check_timer = std::make_unique<QTimer>();
        info->health_check_timer->setInterval(
            static_cast<int>(config.health_check_interval.count()));

        connect(info->health_check_timer.get(), &QTimer::timeout,
                [this, plugin_id]() { d->perform_health_check(plugin_id); });

        info->health_monitoring_enabled = true;
        info->health_check_timer->start();
    }

    // Store plugin info
    d->plugins[plugin_id] = std::move(info);

    qCDebug(pluginLifecycleLog)
        << "Registered plugin for lifecycle management:" << plugin_id;

    return make_success();
}

qtplugin::expected<void, PluginError> PluginLifecycleManager::unregister_plugin(
    const QString& plugin_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    // Stop health monitoring
    if (it->second->health_check_timer) {
        it->second->health_check_timer->stop();
    }

    // Reset state machine to unloaded state
    if (it->second->state_machine) {
        it->second->state_machine->reset();
    }

    // Remove plugin
    d->plugins.erase(it);

    qCDebug(pluginLifecycleLog)
        << "Unregistered plugin from lifecycle management:" << plugin_id;

    return make_success();
}

qtplugin::expected<void, PluginError> PluginLifecycleManager::initialize_plugin(
    const QString& plugin_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    auto plugin = info->plugin;

    // Check current state from our state machine
    PluginState current_state = info->state_machine->current_state();
    if (current_state == PluginState::Running) {
        return make_success();  // Already initialized
    }

    // Handle proper state transitions based on current state
    if (current_state == PluginState::Unloaded) {
        // First transition to Loading
        auto loading_result = info->state_machine->transition_to(PluginState::Loading);
        if (!loading_result) {
            return make_error<void>(PluginErrorCode::InvalidState,
                                    "Cannot transition to loading state: " +
                                        loading_result.error().message);
        }

        // Then transition to Loaded
        auto loaded_result = info->state_machine->transition_to(PluginState::Loaded);
        if (!loaded_result) {
            return make_error<void>(PluginErrorCode::InvalidState,
                                    "Cannot transition to loaded state: " +
                                        loaded_result.error().message);
        }
    } else if (current_state != PluginState::Loaded) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Plugin must be in Unloaded or Loaded state to initialize, current state: " +
                                    std::to_string(static_cast<int>(current_state)));
    }

    // Transition to initializing state
    auto transition_result =
        info->state_machine->transition_to(PluginState::Initializing);
    if (!transition_result) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Cannot transition to initializing state: " +
                                    transition_result.error().message);
    }

    // Emit before initialize event
    PluginLifecycleEventData before_event;
    before_event.plugin_id = plugin_id;
    before_event.event_type = PluginLifecycleEvent::BeforeInitialize;
    before_event.old_state = current_state;
    before_event.new_state = PluginState::Initializing;
    before_event.timestamp = std::chrono::system_clock::now();
    before_event.message = "Starting plugin initialization";

    d->emit_lifecycle_event(before_event);

    // Set up timeout timer
    info->operation_timeout_timer = std::make_unique<QTimer>();
    info->operation_timeout_timer->setSingleShot(true);
    info->operation_timeout_timer->setInterval(
        static_cast<int>(info->config.initialization_timeout.count()));

    bool timeout_occurred = false;
    connect(info->operation_timeout_timer.get(), &QTimer::timeout,
            [&timeout_occurred, plugin_id, this]() {
                timeout_occurred = true;

                PluginLifecycleEventData timeout_event;
                timeout_event.plugin_id = plugin_id;
                timeout_event.event_type = PluginLifecycleEvent::Timeout;
                timeout_event.old_state = PluginState::Initializing;
                timeout_event.new_state = PluginState::Error;
                timeout_event.timestamp = std::chrono::system_clock::now();
                timeout_event.message = "Plugin initialization timeout";

                d->emit_lifecycle_event(timeout_event);
            });

    info->operation_timeout_timer->start();

    // Attempt initialization
    auto init_result = plugin->initialize();

    // Stop timeout timer
    info->operation_timeout_timer->stop();
    info->operation_timeout_timer.reset();

    // Handle result
    PluginLifecycleEventData after_event;
    after_event.plugin_id = plugin_id;
    after_event.event_type = PluginLifecycleEvent::AfterInitialize;
    after_event.old_state = PluginState::Initializing;
    after_event.timestamp = std::chrono::system_clock::now();

    if (timeout_occurred) {
        // Transition to error state
        info->state_machine->transition_to(PluginState::Error);

        after_event.new_state = PluginState::Error;
        after_event.message = "Plugin initialization timed out";

        PluginError timeout_error;
        timeout_error.code = PluginErrorCode::OperationCancelled;
        timeout_error.message = "Initialization timeout";
        after_event.error = timeout_error;

        d->emit_lifecycle_event(after_event);

        return make_error<void>(PluginErrorCode::OperationCancelled,
                                "Plugin initialization timed out");
    }

    if (init_result) {
        // Transition to running state
        auto running_transition =
            info->state_machine->transition_to(PluginState::Running);
        if (!running_transition) {
            qCWarning(pluginLifecycleLog)
                << "Failed to transition to running state for plugin:"
                << plugin_id;
        }

        after_event.new_state = PluginState::Running;
        after_event.message = "Plugin initialization successful";

        d->emit_lifecycle_event(after_event);

        qCDebug(pluginLifecycleLog)
            << "Successfully initialized plugin:" << plugin_id;

        return make_success();
    } else {
        // Transition to error state
        auto error_transition =
            info->state_machine->transition_to(PluginState::Error);
        if (!error_transition) {
            qCWarning(pluginLifecycleLog)
                << "Failed to transition to error state for plugin:"
                << plugin_id;
        }

        after_event.new_state = PluginState::Error;
        after_event.message =
            QString::fromStdString(init_result.error().message);
        after_event.error = init_result.error();

        d->emit_lifecycle_event(after_event);

        d->handle_plugin_error(plugin_id, init_result.error());

        return init_result;
    }
}

qtplugin::expected<void, PluginError> PluginLifecycleManager::shutdown_plugin(
    const QString& plugin_id, bool force) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    auto plugin = info->plugin;

    // Get current state from our state machine
    PluginState current_state = info->state_machine->current_state();

    // Transition to stopping state
    auto transition_result =
        info->state_machine->transition_to(PluginState::Stopping);
    if (!transition_result) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Cannot transition to stopping state: " +
                                    transition_result.error().message);
    }

    // Emit before shutdown event
    PluginLifecycleEventData before_event;
    before_event.plugin_id = plugin_id;
    before_event.event_type = PluginLifecycleEvent::BeforeShutdown;
    before_event.old_state = current_state;
    before_event.new_state = PluginState::Stopping;
    before_event.timestamp = std::chrono::system_clock::now();
    before_event.message = force ? "Starting forced plugin shutdown"
                                 : "Starting graceful plugin shutdown";

    d->emit_lifecycle_event(before_event);

    // Perform shutdown
    try {
        if (!force && info->config.enable_graceful_shutdown) {
            // Set up timeout for graceful shutdown
            info->operation_timeout_timer = std::make_unique<QTimer>();
            info->operation_timeout_timer->setSingleShot(true);
            info->operation_timeout_timer->setInterval(
                static_cast<int>(info->config.shutdown_timeout.count()));

            bool timeout_occurred = false;
            connect(info->operation_timeout_timer.get(), &QTimer::timeout,
                    [&timeout_occurred]() { timeout_occurred = true; });

            info->operation_timeout_timer->start();

            // Attempt graceful shutdown
            plugin->shutdown();

            info->operation_timeout_timer->stop();
            info->operation_timeout_timer.reset();

            if (timeout_occurred) {
                qCWarning(pluginLifecycleLog)
                    << "Graceful shutdown timed out for plugin:" << plugin_id
                    << "forcing shutdown";
                // Force shutdown after timeout
                plugin->shutdown();
            }
        } else {
            // Force shutdown
            plugin->shutdown();
        }

        // Transition to stopped state
        auto stopped_transition =
            info->state_machine->transition_to(PluginState::Stopped);
        if (!stopped_transition) {
            qCWarning(pluginLifecycleLog)
                << "Failed to transition to stopped state for plugin:"
                << plugin_id;
        }

        // Emit after shutdown event
        PluginLifecycleEventData after_event;
        after_event.plugin_id = plugin_id;
        after_event.event_type = PluginLifecycleEvent::AfterShutdown;
        after_event.old_state = PluginState::Stopping;
        after_event.new_state = PluginState::Stopped;
        after_event.timestamp = std::chrono::system_clock::now();
        after_event.message = "Plugin shutdown completed";

        d->emit_lifecycle_event(after_event);

        qCDebug(pluginLifecycleLog)
            << "Successfully shutdown plugin:" << plugin_id;

        return make_success();

    } catch (const std::exception& e) {
        PluginError error;
        error.code = PluginErrorCode::ExecutionFailed;
        error.message = std::string("Shutdown exception: ") + e.what();

        d->handle_plugin_error(plugin_id, error);

        return make_error<void>(error.code, error.message);
    } catch (...) {
        PluginError error;
        error.code = PluginErrorCode::ExecutionFailed;
        error.message = "Unknown shutdown exception";

        d->handle_plugin_error(plugin_id, error);

        return make_error<void>(error.code, error.message);
    }
}

bool PluginLifecycleManager::is_plugin_registered(
    const QString& plugin_id) const {
    QMutexLocker locker(&d->mutex);
    return d->plugins.find(plugin_id) != d->plugins.end();
}

std::vector<QString> PluginLifecycleManager::get_registered_plugins() const {
    QMutexLocker locker(&d->mutex);
    std::vector<QString> plugin_ids;
    plugin_ids.reserve(d->plugins.size());

    for (const auto& [plugin_id, info] : d->plugins) {
        plugin_ids.push_back(plugin_id);
    }

    return plugin_ids;
}

qtplugin::expected<PluginState, PluginError>
PluginLifecycleManager::get_plugin_state(const QString& plugin_id) const {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<PluginState>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    return it->second->state_machine->current_state();
}

bool PluginLifecycleManager::can_transition_to_state(
    const QString& plugin_id, PluginState target_state) const {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return false;
    }

    PluginState current_state = it->second->state_machine->current_state();
    return PluginStateMachine::is_valid_transition(current_state, target_state);
}

std::vector<PluginLifecycleEventData>
PluginLifecycleManager::get_plugin_state_history(const QString& plugin_id,
                                                 int max_entries) const {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return {};
    }

    const auto& history = it->second->event_history;
    if (max_entries <= 0 ||
        static_cast<size_t>(max_entries) >= history.size()) {
        return history;
    }

    // Return the most recent entries
    auto start_it = history.end() - max_entries;
    return std::vector<PluginLifecycleEventData>(start_it, history.end());
}

qtplugin::expected<void, PluginError> PluginLifecycleManager::pause_plugin(
    const QString& plugin_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    PluginState current_state = info->state_machine->current_state();

    if (current_state != PluginState::Running) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Plugin must be running to pause");
    }

    // Transition to paused state
    auto transition_result =
        info->state_machine->transition_to(PluginState::Paused);
    if (!transition_result) {
        return transition_result;
    }

    qCDebug(pluginLifecycleLog) << "Paused plugin:" << plugin_id;
    return make_success();
}

qtplugin::expected<void, PluginError> PluginLifecycleManager::resume_plugin(
    const QString& plugin_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    PluginState current_state = info->state_machine->current_state();

    if (current_state != PluginState::Paused) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Plugin must be paused to resume");
    }

    // Transition to running state
    auto transition_result =
        info->state_machine->transition_to(PluginState::Running);
    if (!transition_result) {
        return transition_result;
    }

    qCDebug(pluginLifecycleLog) << "Resumed plugin:" << plugin_id;
    return make_success();
}

qtplugin::expected<void, PluginError> PluginLifecycleManager::restart_plugin(
    const QString& plugin_id) {
    // First shutdown the plugin
    auto shutdown_result = shutdown_plugin(plugin_id, false);
    if (!shutdown_result) {
        return shutdown_result;
    }

    // Then initialize it again
    return initialize_plugin(plugin_id);
}

qtplugin::expected<void, PluginError>
PluginLifecycleManager::enable_health_monitoring(
    const QString& plugin_id, PluginHealthCheckCallback health_check_callback) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    info->health_check_callback = health_check_callback;
    info->health_monitoring_enabled = true;

    if (!info->health_check_timer) {
        info->health_check_timer = std::make_unique<QTimer>();
        info->health_check_timer->setInterval(
            static_cast<int>(info->config.health_check_interval.count()));

        connect(info->health_check_timer.get(), &QTimer::timeout,
                [this, plugin_id]() { d->perform_health_check(plugin_id); });
    }

    info->health_check_timer->start();

    qCDebug(pluginLifecycleLog)
        << "Enabled health monitoring for plugin:" << plugin_id;
    return make_success();
}

qtplugin::expected<void, PluginError>
PluginLifecycleManager::disable_health_monitoring(const QString& plugin_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    info->health_monitoring_enabled = false;

    if (info->health_check_timer) {
        info->health_check_timer->stop();
    }

    qCDebug(pluginLifecycleLog)
        << "Disabled health monitoring for plugin:" << plugin_id;
    return make_success();
}

qtplugin::expected<PluginHealthStatus, PluginError>
PluginLifecycleManager::check_plugin_health(const QString& plugin_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<PluginHealthStatus>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    d->perform_health_check(plugin_id);
    return it->second->health_status;
}

qtplugin::expected<PluginHealthStatus, PluginError>
PluginLifecycleManager::get_plugin_health_status(
    const QString& plugin_id) const {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<PluginHealthStatus>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    return it->second->health_status;
}

QString PluginLifecycleManager::register_event_callback(
    const QString& plugin_id, PluginLifecycleEvent event_type,
    PluginLifecycleEventCallback callback) {
    QMutexLocker locker(&d->mutex);

    QString callback_id = QUuid::createUuid().toString(QUuid::WithoutBraces);

    LifecycleEventCallback callback_info;
    callback_info.id = callback_id;
    callback_info.plugin_id_filter = plugin_id;
    callback_info.event_type = event_type;
    callback_info.callback = std::move(callback);

    d->event_callbacks[callback_id] = std::move(callback_info);

    qCDebug(pluginLifecycleLog) << "Registered event callback:" << callback_id
                                << "for plugin:" << plugin_id;

    return callback_id;
}

qtplugin::expected<void, PluginError>
PluginLifecycleManager::unregister_event_callback(const QString& callback_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->event_callbacks.find(callback_id);
    if (it == d->event_callbacks.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Event callback not found: " + callback_id.toStdString());
    }

    d->event_callbacks.erase(it);

    qCDebug(pluginLifecycleLog)
        << "Unregistered event callback:" << callback_id;
    return make_success();
}

std::vector<qtplugin::expected<void, PluginError>>
PluginLifecycleManager::initialize_plugins(
    const std::vector<QString>& plugin_ids) {
    std::vector<qtplugin::expected<void, PluginError>> results;
    results.reserve(plugin_ids.size());

    for (const auto& plugin_id : plugin_ids) {
        results.push_back(initialize_plugin(plugin_id));
    }

    return results;
}

std::vector<qtplugin::expected<void, PluginError>>
PluginLifecycleManager::shutdown_plugins(const std::vector<QString>& plugin_ids,
                                         bool force) {
    std::vector<qtplugin::expected<void, PluginError>> results;
    results.reserve(plugin_ids.size());

    for (const auto& plugin_id : plugin_ids) {
        results.push_back(shutdown_plugin(plugin_id, force));
    }

    return results;
}

void PluginLifecycleManager::on_health_check_timer() {
    // This slot is connected to individual plugin health check timers
    // The actual health check is performed in the lambda connected to each
    // timer This method is here for completeness but may not be used directly
}

void PluginLifecycleManager::on_operation_timeout() {
    // This slot is connected to operation timeout timers
    // The actual timeout handling is performed in the lambda connected to each
    // timer This method is here for completeness but may not be used directly
}

// === Enhanced Resource Management and State Migration ===

qtplugin::expected<void, PluginError> PluginLifecycleManager::migrate_plugin_state(
    const QString& plugin_id, const QJsonObject& state_data) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    auto plugin = info->plugin;

    try {
        // Check if plugin supports state migration
        auto available_commands = plugin->available_commands();
        bool supports_migration = std::find(available_commands.begin(),
                                           available_commands.end(),
                                           "migrate_state") != available_commands.end();

        if (!supports_migration) {
            return make_error<void>(
                PluginErrorCode::OperationNotSupported,
                "Plugin does not support state migration");
        }

        // Create migration event
        PluginLifecycleEventData migration_event;
        migration_event.plugin_id = plugin_id;
        migration_event.event_type = PluginLifecycleEvent::StateChanged;
        migration_event.old_state = plugin->state();
        migration_event.new_state = plugin->state();
        migration_event.timestamp = std::chrono::system_clock::now();
        migration_event.message = "State migration started";
        migration_event.metadata = state_data;

        d->emit_lifecycle_event(migration_event);

        // Perform state migration
        auto result = plugin->execute_command("migrate_state", state_data);
        if (!result) {
            return make_error<void>(
                PluginErrorCode::ExecutionFailed,
                "State migration failed: " + result.error().message);
        }

        qCDebug(pluginLifecycleLog)
            << "Successfully migrated state for plugin:" << plugin_id;

        return make_success();

    } catch (const std::exception& e) {
        return make_error<void>(
            PluginErrorCode::ExecutionFailed,
            "State migration exception: " + std::string(e.what()));
    }
}

qtplugin::expected<QJsonObject, PluginError> PluginLifecycleManager::backup_plugin_state(
    const QString& plugin_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<QJsonObject>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    auto plugin = info->plugin;

    try {
        QJsonObject backup_data;

        // Get basic plugin information
        backup_data["plugin_id"] = plugin_id;
        backup_data["plugin_name"] = QString::fromStdString(std::string(plugin->name()));
        backup_data["plugin_version"] = QString::fromStdString(plugin->version().to_string());
        backup_data["current_state"] = static_cast<int>(plugin->state());
        backup_data["backup_timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

        // Try to get plugin-specific state if supported
        auto available_commands = plugin->available_commands();
        bool supports_backup = std::find(available_commands.begin(),
                                        available_commands.end(),
                                        "backup_state") != available_commands.end();

        if (supports_backup) {
            auto result = plugin->execute_command("backup_state");
            if (result) {
                backup_data["plugin_state"] = result.value();
            } else {
                backup_data["backup_error"] = QString::fromStdString(result.error().message);
            }
        }

        // Include lifecycle configuration
        backup_data["lifecycle_config"] = info->config.to_json();

        // Include recent event history (last 10 events)
        QJsonArray history_array;
        const auto& history = info->event_history;
        size_t start_index = history.size() > 10 ? history.size() - 10 : 0;

        for (size_t i = start_index; i < history.size(); ++i) {
            history_array.append(history[i].to_json());
        }
        backup_data["event_history"] = history_array;

        // Include health status
        backup_data["health_status"] = info->health_status.to_json();

        qCDebug(pluginLifecycleLog)
            << "Created backup for plugin:" << plugin_id;

        return backup_data;

    } catch (const std::exception& e) {
        return make_error<QJsonObject>(
            PluginErrorCode::ExecutionFailed,
            "State backup exception: " + std::string(e.what()));
    }
}

qtplugin::expected<void, PluginError> PluginLifecycleManager::restore_plugin_state(
    const QString& plugin_id, const QJsonObject& backup_data) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    auto plugin = info->plugin;

    try {
        // Validate backup data
        if (!backup_data.contains("plugin_id") ||
            backup_data["plugin_id"].toString() != plugin_id) {
            return make_error<void>(
                PluginErrorCode::InvalidParameters,
                "Backup data does not match plugin ID");
        }

        // Restore lifecycle configuration if present
        if (backup_data.contains("lifecycle_config")) {
            auto config_data = backup_data["lifecycle_config"].toObject();
            info->config = PluginLifecycleConfig::from_json(config_data);
        }

        // Try to restore plugin-specific state if supported and present
        auto available_commands = plugin->available_commands();
        bool supports_restore = std::find(available_commands.begin(),
                                         available_commands.end(),
                                         "restore_state") != available_commands.end();

        if (supports_restore && backup_data.contains("plugin_state")) {
            auto plugin_state = backup_data["plugin_state"].toObject();
            auto result = plugin->execute_command("restore_state", plugin_state);

            if (!result) {
                return make_error<void>(
                    PluginErrorCode::ExecutionFailed,
                    "State restore failed: " + result.error().message);
            }
        }

        // Create restore event
        PluginLifecycleEventData restore_event;
        restore_event.plugin_id = plugin_id;
        restore_event.event_type = PluginLifecycleEvent::StateChanged;
        restore_event.old_state = plugin->state();
        restore_event.new_state = plugin->state();
        restore_event.timestamp = std::chrono::system_clock::now();
        restore_event.message = "State restored from backup";
        restore_event.metadata = backup_data;

        d->emit_lifecycle_event(restore_event);

        qCDebug(pluginLifecycleLog)
            << "Successfully restored state for plugin:" << plugin_id;

        return make_success();

    } catch (const std::exception& e) {
        return make_error<void>(
            PluginErrorCode::ExecutionFailed,
            "State restore exception: " + std::string(e.what()));
    }
}

qtplugin::expected<void, PluginError> PluginLifecycleManager::cleanup_plugin_resources(
    const QString& plugin_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    auto plugin = info->plugin;

    try {
        // Create cleanup event
        PluginLifecycleEventData cleanup_event;
        cleanup_event.plugin_id = plugin_id;
        cleanup_event.event_type = PluginLifecycleEvent::ResourceWarning;
        cleanup_event.old_state = plugin->state();
        cleanup_event.new_state = plugin->state();
        cleanup_event.timestamp = std::chrono::system_clock::now();
        cleanup_event.message = "Resource cleanup started";

        d->emit_lifecycle_event(cleanup_event);

        // Stop health monitoring
        if (info->health_check_timer) {
            info->health_check_timer->stop();
        }

        // Stop any operation timers
        if (info->operation_timeout_timer) {
            info->operation_timeout_timer->stop();
            info->operation_timeout_timer.reset();
        }

        // Try plugin-specific cleanup if supported
        auto available_commands = plugin->available_commands();
        bool supports_cleanup = std::find(available_commands.begin(),
                                         available_commands.end(),
                                         "cleanup_resources") != available_commands.end();

        if (supports_cleanup) {
            auto result = plugin->execute_command("cleanup_resources");
            if (!result) {
                qCWarning(pluginLifecycleLog)
                    << "Plugin-specific cleanup failed for" << plugin_id
                    << ":" << result.error().message.c_str();
            }
        }

        // Clear event history to free memory
        info->event_history.clear();
        info->event_history.shrink_to_fit();

        // Reset restart attempts
        info->restart_attempts = 0;

        qCDebug(pluginLifecycleLog)
            << "Completed resource cleanup for plugin:" << plugin_id;

        return make_success();

    } catch (const std::exception& e) {
        return make_error<void>(
            PluginErrorCode::ExecutionFailed,
            "Resource cleanup exception: " + std::string(e.what()));
    }
}

qtplugin::expected<void, PluginError> PluginLifecycleManager::shutdown_all_plugins_gracefully(
    std::chrono::milliseconds timeout) {

    auto registered_plugins = get_registered_plugins();
    if (registered_plugins.empty()) {
        return make_success();
    }

    qCInfo(pluginLifecycleLog)
        << "Starting graceful shutdown of" << registered_plugins.size() << "plugins";

    // Create shutdown coordination
    std::vector<std::future<qtplugin::expected<void, PluginError>>> shutdown_futures;
    shutdown_futures.reserve(registered_plugins.size());

    auto start_time = std::chrono::steady_clock::now();

    // Start shutdown for all plugins in parallel
    for (const auto& plugin_id : registered_plugins) {
        auto future = std::async(std::launch::async, [this, plugin_id]() {
            return shutdown_plugin(plugin_id, false); // Graceful shutdown
        });
        shutdown_futures.push_back(std::move(future));
    }

    // Wait for all shutdowns to complete or timeout
    std::vector<QString> failed_plugins;
    for (size_t i = 0; i < shutdown_futures.size(); ++i) {
        auto& future = shutdown_futures[i];
        const auto& plugin_id = registered_plugins[i];

        // Check if we've exceeded the total timeout
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed >= timeout) {
            qCWarning(pluginLifecycleLog)
                << "Global shutdown timeout exceeded, forcing remaining plugins";

            // Force shutdown remaining plugins
            for (size_t j = i; j < registered_plugins.size(); ++j) {
                shutdown_plugin(registered_plugins[j], true);
            }
            break;
        }

        auto remaining_time = timeout - elapsed;
        auto status = future.wait_for(remaining_time);

        if (status == std::future_status::ready) {
            auto result = future.get();
            if (!result) {
                failed_plugins.push_back(plugin_id);
                qCWarning(pluginLifecycleLog)
                    << "Failed to shutdown plugin gracefully:" << plugin_id
                    << "-" << result.error().message.c_str();
            }
        } else {
            // Timeout for this plugin, force shutdown
            failed_plugins.push_back(plugin_id);
            qCWarning(pluginLifecycleLog)
                << "Plugin shutdown timed out, forcing:" << plugin_id;
            shutdown_plugin(plugin_id, true);
        }
    }

    if (!failed_plugins.empty()) {
        QStringList failed_list;
        for (const auto& plugin : failed_plugins) {
            failed_list << plugin;
        }
        QString error_msg = QString("Failed to gracefully shutdown %1 plugins: %2")
                           .arg(failed_plugins.size())
                           .arg(failed_list.join(", "));
        return make_error<void>(PluginErrorCode::ExecutionFailed,
                               error_msg.toStdString());
    }

    qCInfo(pluginLifecycleLog)
        << "Successfully completed graceful shutdown of all plugins";

    return make_success();
}

QJsonObject PluginLifecycleManager::get_lifecycle_statistics() const {
    QMutexLocker locker(&d->mutex);

    QJsonObject stats;

    // Basic counts
    stats["total_registered_plugins"] = static_cast<int>(d->plugins.size());

    // Count plugins by state
    std::unordered_map<PluginState, int> state_counts;
    for (const auto& [plugin_id, info] : d->plugins) {
        if (info->state_machine) {
            auto state = info->state_machine->current_state();
            state_counts[state]++;
        }
    }

    QJsonObject state_stats;
    for (const auto& [state, count] : state_counts) {
        QString state_name = QString("state_%1").arg(static_cast<int>(state));
        state_stats[state_name] = count;
    }
    stats["plugins_by_state"] = state_stats;

    // Health monitoring statistics
    int healthy_plugins = 0;
    int unhealthy_plugins = 0;
    int monitoring_enabled = 0;

    for (const auto& [plugin_id, info] : d->plugins) {
        if (info->health_monitoring_enabled) {
            monitoring_enabled++;
            if (info->health_status.is_healthy) {
                healthy_plugins++;
            } else {
                unhealthy_plugins++;
            }
        }
    }

    stats["health_monitoring_enabled"] = monitoring_enabled;
    stats["healthy_plugins"] = healthy_plugins;
    stats["unhealthy_plugins"] = unhealthy_plugins;

    // Event callback statistics
    stats["registered_event_callbacks"] = static_cast<int>(d->event_callbacks.size());

    // Auto-restart statistics
    int auto_restart_enabled = 0;
    int total_restart_attempts = 0;

    for (const auto& [plugin_id, info] : d->plugins) {
        if (info->config.auto_restart_on_failure) {
            auto_restart_enabled++;
        }
        total_restart_attempts += info->restart_attempts;
    }

    stats["auto_restart_enabled_plugins"] = auto_restart_enabled;
    stats["total_restart_attempts"] = total_restart_attempts;

    return stats;
}

}  // namespace qtplugin
