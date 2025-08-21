/**
 * @file basic_plugin.cpp
 * @brief Implementation of basic example plugin
 * @version 3.0.0
 */

#include "basic_plugin.hpp"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <chrono>

BasicPlugin::BasicPlugin(QObject* parent)
    : QObject(parent)
    , m_timer(std::make_unique<QTimer>(this))
{
    // Connect timer
    connect(m_timer.get(), &QTimer::timeout, this, &BasicPlugin::on_timer_timeout);
}

BasicPlugin::~BasicPlugin() {
    if (m_state != qtplugin::PluginState::Unloaded) {
        shutdown();
    }
}

qtplugin::expected<void, qtplugin::PluginError> BasicPlugin::initialize() {
    if (m_state != qtplugin::PluginState::Unloaded && m_state != qtplugin::PluginState::Loaded) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError, 
                                         "Plugin is not in a state that allows initialization");
    }
    
    m_state = qtplugin::PluginState::Initializing;
    m_initialization_time = std::chrono::system_clock::now();
    
    try {
        // Initialize timer with configured interval
        m_timer->setInterval(m_timer_interval);
        m_timer->start();
        
        m_state = qtplugin::PluginState::Running;
        
        log_info("BasicPlugin initialized successfully");
        
        return qtplugin::make_success();
    } catch (const std::exception& e) {
        m_state = qtplugin::PluginState::Error;
        std::string error_msg = "Initialization failed: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::InitializationFailed, error_msg);
    }
}

void BasicPlugin::shutdown() noexcept {
    try {
        m_state = qtplugin::PluginState::Stopping;
        
        // Stop timer
        if (m_timer && m_timer->isActive()) {
            m_timer->stop();
        }
        
        m_state = qtplugin::PluginState::Stopped;
        
        log_info("BasicPlugin shutdown completed");
    } catch (...) {
        // Shutdown must not throw
        m_state = qtplugin::PluginState::Error;
    }
}

std::optional<QJsonObject> BasicPlugin::default_configuration() const {
    return QJsonObject{
        {"timer_interval", 5000},
        {"logging_enabled", true},
        {"custom_message", "Hello from BasicPlugin!"}
    };
}

qtplugin::expected<void, qtplugin::PluginError> BasicPlugin::configure(const QJsonObject& config) {
    if (!validate_configuration(config)) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::ConfigurationError, 
                                         "Invalid configuration provided");
    }
    
    // Store old configuration for comparison
    QJsonObject old_config = m_configuration;
    
    // Update configuration
    m_configuration = config;
    
    // Apply configuration changes
    if (config.contains("timer_interval")) {
        m_timer_interval = config["timer_interval"].toInt();
        if (m_timer && m_timer->isActive()) {
            m_timer->setInterval(m_timer_interval);
        }
    }
    
    if (config.contains("logging_enabled")) {
        m_logging_enabled = config["logging_enabled"].toBool();
    }
    
    if (config.contains("custom_message")) {
        m_custom_message = config["custom_message"].toString().toStdString();
    }
    
    log_info("Configuration updated successfully");
    
    return qtplugin::make_success();
}

QJsonObject BasicPlugin::current_configuration() const {
    return m_configuration;
}

bool BasicPlugin::validate_configuration(const QJsonObject& config) const {
    // Validate timer_interval
    if (config.contains("timer_interval")) {
        if (!config["timer_interval"].isDouble()) {
            return false;
        }
        int interval = config["timer_interval"].toInt();
        if (interval < 1000 || interval > 60000) {
            return false;
        }
    }
    
    // Validate logging_enabled
    if (config.contains("logging_enabled")) {
        if (!config["logging_enabled"].isBool()) {
            return false;
        }
    }
    
    // Validate custom_message
    if (config.contains("custom_message")) {
        if (!config["custom_message"].isString()) {
            return false;
        }
    }
    
    return true;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
BasicPlugin::execute_command(std::string_view command, const QJsonObject& params) {
    m_command_count.fetch_add(1);
    
    if (command == "status") {
        return handle_status_command(params);
    } else if (command == "echo") {
        return handle_echo_command(params);
    } else if (command == "config") {
        return handle_config_command(params);
    } else if (command == "metrics") {
        return handle_metrics_command(params);
    } else if (command == "test") {
        return handle_test_command(params);
    } else {
        return qtplugin::make_error<QJsonObject>(qtplugin::PluginErrorCode::CommandNotFound, 
                                                "Unknown command: " + std::string(command));
    }
}

std::vector<std::string> BasicPlugin::available_commands() const {
    return {"status", "echo", "config", "metrics", "test"};
}

void BasicPlugin::clear_errors() {
    m_error_log.clear();
    m_last_error.clear();
}

std::chrono::milliseconds BasicPlugin::uptime() const {
    if (m_state == qtplugin::PluginState::Running) {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_initialization_time);
    }
    return std::chrono::milliseconds{0};
}

QJsonObject BasicPlugin::performance_metrics() const {
    return QJsonObject{
        {"uptime_ms", static_cast<qint64>(uptime().count())},
        {"command_count", static_cast<qint64>(m_command_count.load())},
        {"message_count", static_cast<qint64>(m_message_count.load())},
        {"error_count", static_cast<qint64>(m_error_count.load())},
        {"state", static_cast<int>(m_state.load())},
        {"timer_interval", m_timer_interval},
        {"logging_enabled", m_logging_enabled}
    };
}

QJsonObject BasicPlugin::resource_usage() const {
    // This is a simplified implementation
    // In a real plugin, you might collect actual resource usage data
    return QJsonObject{
        {"estimated_memory_kb", 512},
        {"estimated_cpu_percent", 0.1},
        {"thread_count", 1}
    };
}

void BasicPlugin::on_timer_timeout() {
    m_message_count.fetch_add(1);
    
    if (m_logging_enabled) {
        log_info("Timer tick: " + m_custom_message);
    }
    
    // Update last activity time
    // Note: In a real implementation, you might want to make this thread-safe
}

void BasicPlugin::on_message_received() {
    m_message_count.fetch_add(1);
}

void BasicPlugin::log_error(const std::string& error) {
    m_last_error = error;
    m_error_log.push_back(error);
    m_error_count.fetch_add(1);
    
    // Maintain error log size
    if (m_error_log.size() > MAX_ERROR_LOG_SIZE) {
        m_error_log.erase(m_error_log.begin());
    }
    
    if (m_logging_enabled) {
        qWarning() << "BasicPlugin Error:" << QString::fromStdString(error);
    }
}

void BasicPlugin::log_info(const std::string& message) {
    if (m_logging_enabled) {
        qInfo() << "BasicPlugin:" << QString::fromStdString(message);
    }
}

void BasicPlugin::update_metrics() {
    // Update internal metrics
    // This could be called periodically by the plugin manager
}

QJsonObject BasicPlugin::handle_status_command(const QJsonObject& params) {
    Q_UNUSED(params)
    
    const char* state_str = "Unknown";
    switch (m_state.load()) {
        case qtplugin::PluginState::Unloaded: state_str = "Unloaded"; break;
        case qtplugin::PluginState::Loading: state_str = "Loading"; break;
        case qtplugin::PluginState::Loaded: state_str = "Loaded"; break;
        case qtplugin::PluginState::Initializing: state_str = "Initializing"; break;
        case qtplugin::PluginState::Running: state_str = "Running"; break;
        case qtplugin::PluginState::Paused: state_str = "Paused"; break;
        case qtplugin::PluginState::Stopping: state_str = "Stopping"; break;
        case qtplugin::PluginState::Stopped: state_str = "Stopped"; break;
        case qtplugin::PluginState::Error: state_str = "Error"; break;
        case qtplugin::PluginState::Reloading: state_str = "Reloading"; break;
    }
    
    return QJsonObject{
        {"state", state_str},
        {"uptime_ms", static_cast<qint64>(uptime().count())},
        {"message_count", static_cast<qint64>(m_message_count.load())},
        {"command_count", static_cast<qint64>(m_command_count.load())},
        {"error_count", static_cast<qint64>(m_error_count.load())},
        {"custom_message", QString::fromStdString(m_custom_message)},
        {"timer_active", m_timer && m_timer->isActive()}
    };
}

QJsonObject BasicPlugin::handle_echo_command(const QJsonObject& params) {
    if (!params.contains("message") || !params["message"].isString()) {
        return QJsonObject{
            {"error", "Missing or invalid 'message' parameter"}
        };
    }
    
    QString message = params["message"].toString();
    QString timestamp = QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    
    return QJsonObject{
        {"echoed_message", message},
        {"timestamp", timestamp},
        {"success", true}
    };
}

QJsonObject BasicPlugin::handle_config_command(const QJsonObject& params) {
    QString action = params.value("action").toString("get");
    
    if (action == "get") {
        return QJsonObject{
            {"current_config", m_configuration},
            {"success", true}
        };
    } else if (action == "set") {
        if (!params.contains("config") || !params["config"].isObject()) {
            return QJsonObject{
                {"error", "Missing or invalid 'config' parameter"},
                {"success", false}
            };
        }
        
        QJsonObject new_config = params["config"].toObject();
        auto result = configure(new_config);
        
        return QJsonObject{
            {"current_config", m_configuration},
            {"success", result.has_value()},
            {"error", result ? "" : QString::fromStdString(result.error().message)}
        };
    } else {
        return QJsonObject{
            {"error", "Invalid action. Use 'get' or 'set'"},
            {"success", false}
        };
    }
}

QJsonObject BasicPlugin::handle_metrics_command(const QJsonObject& params) {
    Q_UNUSED(params)
    return performance_metrics();
}

QJsonObject BasicPlugin::handle_test_command(const QJsonObject& params) {
    QString test_type = params.value("test_type").toString("basic");
    
    if (test_type == "basic") {
        return QJsonObject{
            {"test_result", "Basic test passed"},
            {"success", true},
            {"details", QJsonObject{
                {"plugin_responsive", true},
                {"configuration_valid", validate_configuration(m_configuration)},
                {"timer_working", m_timer && m_timer->isActive()}
            }}
        };
    } else if (test_type == "performance") {
        return QJsonObject{
            {"test_result", "Performance test completed"},
            {"success", true},
            {"details", performance_metrics()}
        };
    } else if (test_type == "stress") {
        // Simulate some work
        for (int i = 0; i < 1000; ++i) {
            m_command_count.fetch_add(1);
        }
        
        return QJsonObject{
            {"test_result", "Stress test completed"},
            {"success", true},
            {"details", QJsonObject{
                {"iterations", 1000},
                {"final_command_count", static_cast<qint64>(m_command_count.load())}
            }}
        };
    } else {
        return QJsonObject{
            {"test_result", "Unknown test type"},
            {"success", false},
            {"error", "Supported test types: basic, performance, stress"}
        };
    }
}

// Plugin factory implementation is in the header file as static methods
