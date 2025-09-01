/**
 * @file basic_plugin.cpp
 * @brief Implementation of core QtForge plugin
 */

#include "basic_plugin.hpp"
#include <QDateTime>
#include <QDebug>
#include <QMutexLocker>
#include <string>
#include <string_view>

BasicPlugin::BasicPlugin(QObject* parent)
    : QObject(parent), m_timer(std::make_unique<QTimer>(this)) {
    // Connect timer signal
    connect(m_timer.get(), &QTimer::timeout, this,
            &BasicPlugin::on_timer_timeout);

    // Set default configuration
    auto default_config = default_configuration();
    if (default_config.has_value()) {
        m_configuration = default_config.value();
    }
}

BasicPlugin::~BasicPlugin() {
    if (m_state == qtplugin::PluginState::Loaded) {
        shutdown();
    }
}

// === IPlugin Interface Implementation ===

std::string_view BasicPlugin::name() const noexcept { return "BasicPlugin"; }

std::string_view BasicPlugin::description() const noexcept {
    return "Basic plugin demonstrating core IPlugin interface";
}

qtplugin::Version BasicPlugin::version() const noexcept {
    return qtplugin::Version{2, 0, 0};
}

std::string_view BasicPlugin::author() const noexcept { return "QtForge Team"; }

std::string BasicPlugin::id() const noexcept { return "qtplugin.BasicPlugin"; }

qtplugin::PluginCapabilities BasicPlugin::capabilities() const noexcept {
    return qtplugin::PluginCapabilities{};
}

qtplugin::expected<void, qtplugin::PluginError> BasicPlugin::initialize() {
    if (m_state == qtplugin::PluginState::Loaded) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin already initialized");
    }

    qDebug() << "BasicPlugin: Initializing...";

    // Apply configuration
    QMutexLocker locker(&m_config_mutex);
    int interval = m_configuration.value("timer_interval").toInt(5000);
    bool logging_enabled =
        m_configuration.value("logging_enabled").toBool(true);

    if (logging_enabled) {
        qDebug() << "BasicPlugin: Logging enabled, timer interval:" << interval
                 << "ms";
    }

    // Start timer if enabled
    if (m_configuration.value("timer_enabled").toBool(true)) {
        m_timer->start(interval);
    }

    m_state = qtplugin::PluginState::Loaded;
    qDebug() << "BasicPlugin: Initialized successfully!";

    return {};
}

void BasicPlugin::shutdown() noexcept {
    qDebug() << "BasicPlugin: Shutting down...";

    // Stop timer
    if (m_timer && m_timer->isActive()) {
        m_timer->stop();
    }

    m_state = qtplugin::PluginState::Unloaded;
    qDebug() << "BasicPlugin: Shutdown complete.";
}

qtplugin::expected<void, qtplugin::PluginError> BasicPlugin::configure(
    const QJsonObject& config) {
    if (!validate_configuration(config)) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError,
            "Invalid configuration provided");
    }

    QMutexLocker locker(&m_config_mutex);

    // Merge with existing configuration
    for (auto it = config.begin(); it != config.end(); ++it) {
        m_configuration[it.key()] = it.value();
    }

    // Apply timer interval change if plugin is running
    if (m_state == qtplugin::PluginState::Loaded &&
        config.contains("timer_interval")) {
        int new_interval = config.value("timer_interval").toInt();
        m_timer->setInterval(new_interval);
        qDebug() << "BasicPlugin: Timer interval updated to" << new_interval
                 << "ms";
    }

    qDebug() << "BasicPlugin: Configuration updated successfully";
    return {};
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
BasicPlugin::execute_command(std::string_view command,
                             const QJsonObject& params) {
    if (m_state != qtplugin::PluginState::Loaded) {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::InvalidState, "Plugin not initialized");
    }

    try {
        if (command == "status") {
            return execute_status_command(params);
        } else if (command == "echo") {
            return execute_echo_command(params);
        } else if (command == "config") {
            return execute_config_command(params);
        } else if (command == "timer") {
            return execute_timer_command(params);
        }

        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound,
            std::string("Unknown command: ") + std::string(command));

    } catch (const std::exception& e) {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::UnknownError,
            std::string("Command execution failed: ") + e.what());
    }
}

std::vector<std::string> BasicPlugin::available_commands() const {
    return {"status", "echo", "config", "timer"};
}

qtplugin::PluginMetadata BasicPlugin::metadata() const {
    qtplugin::PluginMetadata meta;
    meta.name = "BasicPlugin";
    meta.description = "Basic plugin demonstrating core IPlugin interface";
    meta.version = qtplugin::Version{2, 0, 0};
    meta.author = "QtForge Examples";
    meta.category = "Example";
    meta.license = "MIT";
    meta.homepage = "https://github.com/qtforge/examples";
    return meta;
}

qtplugin::PluginState BasicPlugin::state() const noexcept {
    return m_state.load();
}

bool BasicPlugin::is_initialized() const noexcept {
    return m_state == qtplugin::PluginState::Loaded;
}

std::optional<QJsonObject> BasicPlugin::default_configuration() const {
    QJsonObject config;
    config["timer_interval"] = 5000;
    config["timer_enabled"] = true;
    config["logging_enabled"] = true;
    config["custom_message"] = "Hello from BasicPlugin!";
    return std::make_optional(config);
}

QJsonObject BasicPlugin::current_configuration() const {
    QMutexLocker locker(&m_config_mutex);
    return m_configuration;
}

void BasicPlugin::on_timer_timeout() {
    int count = ++m_timer_count;

    QMutexLocker locker(&m_config_mutex);
    bool logging_enabled =
        m_configuration.value("logging_enabled").toBool(true);
    QString message = m_configuration.value("custom_message").toString();

    if (logging_enabled) {
        qDebug() << "BasicPlugin: Timer event #" << count << "-" << message;
    }
}

bool BasicPlugin::validate_configuration(const QJsonObject& config) const {
    // Validate timer_interval
    if (config.contains("timer_interval")) {
        int interval = config.value("timer_interval").toInt(-1);
        if (interval < 1000 || interval > 60000) {
            qWarning()
                << "BasicPlugin: Invalid timer_interval, must be 1000-60000ms";
            return false;
        }
    }

    // Validate custom_message length
    if (config.contains("custom_message")) {
        QString message = config.value("custom_message").toString();
        if (message.length() > 200) {
            qWarning()
                << "BasicPlugin: custom_message too long, max 200 characters";
            return false;
        }
    }

    return true;
}

QJsonObject BasicPlugin::execute_status_command(const QJsonObject& params) {
    Q_UNUSED(params)

    QJsonObject result;
    result["plugin"] = PLUGIN_NAME;
    result["state"] = static_cast<int>(m_state.load());
    result["timer_count"] = m_timer_count.load();
    result["timer_active"] = m_timer->isActive();
    result["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    return result;
}

QJsonObject BasicPlugin::execute_echo_command(const QJsonObject& params) {
    QJsonObject result;
    result["echo"] = params;
    result["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    result["plugin"] = PLUGIN_NAME;

    return result;
}

QJsonObject BasicPlugin::execute_config_command(const QJsonObject& params) {
    if (params.contains("action")) {
        QString action = params.value("action").toString();

        if (action == "get") {
            QJsonObject result;
            result["configuration"] = current_configuration();
            auto default_config = default_configuration();
            if (default_config.has_value()) {
                result["default_configuration"] = default_config.value();
            }
            return result;
        } else if (action == "set" && params.contains("config")) {
            QJsonObject new_config = params.value("config").toObject();
            auto config_result = configure(new_config);

            QJsonObject result;
            result["success"] = config_result.has_value();
            if (!config_result) {
                result["error"] = config_result.error().message.c_str();
            }
            return result;
        }
    }

    // Default: return current configuration
    QJsonObject result;
    result["configuration"] = current_configuration();
    return result;
}

QJsonObject BasicPlugin::execute_timer_command(const QJsonObject& params) {
    QJsonObject result;

    if (params.contains("action")) {
        QString action = params.value("action").toString();

        if (action == "start") {
            if (!m_timer->isActive()) {
                m_timer->start();
                result["message"] = "Timer started";
            } else {
                result["message"] = "Timer already running";
            }
        } else if (action == "stop") {
            if (m_timer->isActive()) {
                m_timer->stop();
                result["message"] = "Timer stopped";
            } else {
                result["message"] = "Timer already stopped";
            }
        } else if (action == "reset") {
            m_timer_count = 0;
            result["message"] = "Timer count reset";
        }
    }

    result["timer_active"] = m_timer->isActive();
    result["timer_count"] = m_timer_count.load();
    result["timer_interval"] = m_timer->interval();

    return result;
}
