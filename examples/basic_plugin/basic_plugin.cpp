/**
 * @file basic_plugin.cpp
 * @brief Implementation of enhanced basic example plugin
 * @version 3.0.0
 *
 * This implementation demonstrates ALL QtPlugin system capabilities
 * with comprehensive error handling, thread safety, and monitoring.
 */

#include "basic_plugin.hpp"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <chrono>
#include <thread>

BasicPlugin::BasicPlugin(QObject* parent)
    : QObject(parent), m_timer(std::make_unique<QTimer>(this)) {
    // Connect timer
    connect(m_timer.get(), &QTimer::timeout, this,
            &BasicPlugin::on_timer_timeout);

    // Initialize dependencies
    m_required_dependencies = {};  // No required dependencies for basic plugin
    m_optional_dependencies = {"qtplugin.MessageBus",
                               "qtplugin.ConfigurationManager"};

    log_info("Enhanced BasicPlugin constructed");
}

BasicPlugin::~BasicPlugin() {
    if (m_state != qtplugin::PluginState::Unloaded) {
        shutdown();
    }
}

qtplugin::expected<void, qtplugin::PluginError> BasicPlugin::initialize() {
    if (m_state != qtplugin::PluginState::Unloaded &&
        m_state != qtplugin::PluginState::Loaded) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::StateError,
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
        std::string error_msg =
            "Initialization failed: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed, error_msg);
    }
}

void BasicPlugin::shutdown() noexcept {
    try {
        std::unique_lock lock(m_state_mutex);
        m_state = qtplugin::PluginState::Stopping;
        lock.unlock();

        // Stop timer
        if (m_timer && m_timer->isActive()) {
            m_timer->stop();
        }

        lock.lock();
        m_state = qtplugin::PluginState::Stopped;
        lock.unlock();

        log_info("Enhanced BasicPlugin shutdown completed");
    } catch (...) {
        // Shutdown must not throw
        std::unique_lock lock(m_state_mutex);
        m_state = qtplugin::PluginState::Error;
    }
}

bool BasicPlugin::is_initialized() const noexcept {
    std::shared_lock lock(m_state_mutex);
    auto current_state = m_state.load();
    return current_state == qtplugin::PluginState::Running ||
           current_state == qtplugin::PluginState::Paused;
}

qtplugin::expected<void, qtplugin::PluginError> BasicPlugin::pause() {
    std::unique_lock lock(m_state_mutex);

    if (m_state != qtplugin::PluginState::Running) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be running to pause");
    }

    try {
        // Pause timer
        if (m_timer && m_timer->isActive()) {
            m_timer->stop();
        }

        m_state = qtplugin::PluginState::Paused;
        log_info("Plugin paused successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to pause plugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> BasicPlugin::resume() {
    std::unique_lock lock(m_state_mutex);

    if (m_state != qtplugin::PluginState::Paused) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be paused to resume");
    }

    try {
        // Resume timer
        if (m_timer) {
            m_timer->setInterval(m_timer_interval);
            m_timer->start();
        }

        m_state = qtplugin::PluginState::Running;
        log_info("Plugin resumed successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to resume plugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> BasicPlugin::restart() {
    log_info("Restarting plugin...");

    // Shutdown first
    shutdown();

    // Wait a brief moment for cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Initialize again
    return initialize();
}

std::optional<QJsonObject> BasicPlugin::default_configuration() const {
    return QJsonObject{{"timer_interval", 5000},
                       {"logging_enabled", true},
                       {"custom_message", "Hello from BasicPlugin!"}};
}

qtplugin::expected<void, qtplugin::PluginError> BasicPlugin::configure(
    const QJsonObject& config) {
    if (!validate_configuration(config)) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError,
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
BasicPlugin::execute_command(std::string_view command,
                             const QJsonObject& params) {
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
    } else if (command == "lifecycle") {
        return handle_lifecycle_command(params);
    } else if (command == "monitoring") {
        return handle_monitoring_command(params);
    } else if (command == "dependencies") {
        return handle_dependencies_command(params);
    } else if (command == "capabilities") {
        return handle_capabilities_command(params);
    } else {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound,
            "Unknown command: " + std::string(command));
    }
}

std::vector<std::string> BasicPlugin::available_commands() const {
    return {"status",    "echo",       "config",       "metrics",     "test",
            "lifecycle", "monitoring", "dependencies", "capabilities"};
}

void BasicPlugin::clear_errors() {
    std::lock_guard lock(m_error_mutex);
    m_error_log.clear();
    m_last_error.clear();
}

// === Dependency Management ===

std::vector<std::string> BasicPlugin::dependencies() const {
    return m_required_dependencies;
}

std::vector<std::string> BasicPlugin::optional_dependencies() const {
    return m_optional_dependencies;
}

bool BasicPlugin::dependencies_satisfied() const {
    return m_dependencies_satisfied.load();
}

std::chrono::milliseconds BasicPlugin::uptime() const {
    if (m_state == qtplugin::PluginState::Running) {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_initialization_time);
    }
    return std::chrono::milliseconds{0};
}

QJsonObject BasicPlugin::performance_metrics() const {
    std::lock_guard lock(m_metrics_mutex);

    auto current_uptime = uptime();
    auto commands_per_second =
        current_uptime.count() > 0
            ? (m_command_count.load() * 1000.0) / current_uptime.count()
            : 0.0;

    return QJsonObject{
        {"uptime_ms", static_cast<qint64>(current_uptime.count())},
        {"command_count", static_cast<qint64>(m_command_count.load())},
        {"message_count", static_cast<qint64>(m_message_count.load())},
        {"error_count", static_cast<qint64>(m_error_count.load())},
        {"commands_per_second", commands_per_second},
        {"state", static_cast<int>(m_state.load())},
        {"state_name",
         [this]() {
             switch (m_state.load()) {
                 case qtplugin::PluginState::Unloaded:
                     return "Unloaded";
                 case qtplugin::PluginState::Loading:
                     return "Loading";
                 case qtplugin::PluginState::Loaded:
                     return "Loaded";
                 case qtplugin::PluginState::Initializing:
                     return "Initializing";
                 case qtplugin::PluginState::Running:
                     return "Running";
                 case qtplugin::PluginState::Paused:
                     return "Paused";
                 case qtplugin::PluginState::Stopping:
                     return "Stopping";
                 case qtplugin::PluginState::Stopped:
                     return "Stopped";
                 case qtplugin::PluginState::Error:
                     return "Error";
                 case qtplugin::PluginState::Reloading:
                     return "Reloading";
                 default:
                     return "Unknown";
             }
         }()},
        {"timer_interval", m_timer_interval},
        {"logging_enabled", m_logging_enabled},
        {"is_thread_safe", is_thread_safe()},
        {"thread_model", QString::fromStdString(std::string(thread_model()))}};
}

QJsonObject BasicPlugin::resource_usage() const {
    std::lock_guard lock(m_metrics_mutex);

    // Enhanced resource usage tracking
    // In a real plugin, you might collect actual resource usage data
    auto memory_estimate = 512 + (m_error_log.size() * 50);  // Base + error log
    auto cpu_estimate = m_timer && m_timer->isActive() ? 0.5 : 0.1;

    return QJsonObject{
        {"estimated_memory_kb", static_cast<qint64>(memory_estimate)},
        {"estimated_cpu_percent", cpu_estimate},
        {"thread_count", 1},
        {"timer_active", m_timer && m_timer->isActive()},
        {"error_log_size", static_cast<qint64>(m_error_log.size())},
        {"dependencies_satisfied", dependencies_satisfied()},
        {"capabilities_count",
         static_cast<int>(__builtin_popcountll(capabilities()))}};
}

void BasicPlugin::on_timer_timeout() {
    m_message_count.fetch_add(1);

    if (m_logging_enabled) {
        log_info("Timer tick: " + m_custom_message);
    }

    // Update last activity time
    // Note: In a real implementation, you might want to make this thread-safe
}

void BasicPlugin::on_message_received() { m_message_count.fetch_add(1); }

void BasicPlugin::log_error(const std::string& error) {
    {
        std::lock_guard lock(m_error_mutex);
        m_last_error = error;
        m_error_log.push_back(error);

        // Maintain error log size
        if (m_error_log.size() > MAX_ERROR_LOG_SIZE) {
            m_error_log.erase(m_error_log.begin());
        }
    }

    m_error_count.fetch_add(1);

    if (m_logging_enabled) {
        qWarning() << "Enhanced BasicPlugin Error:"
                   << QString::fromStdString(error);
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
        case qtplugin::PluginState::Unloaded:
            state_str = "Unloaded";
            break;
        case qtplugin::PluginState::Loading:
            state_str = "Loading";
            break;
        case qtplugin::PluginState::Loaded:
            state_str = "Loaded";
            break;
        case qtplugin::PluginState::Initializing:
            state_str = "Initializing";
            break;
        case qtplugin::PluginState::Running:
            state_str = "Running";
            break;
        case qtplugin::PluginState::Paused:
            state_str = "Paused";
            break;
        case qtplugin::PluginState::Stopping:
            state_str = "Stopping";
            break;
        case qtplugin::PluginState::Stopped:
            state_str = "Stopped";
            break;
        case qtplugin::PluginState::Error:
            state_str = "Error";
            break;
        case qtplugin::PluginState::Reloading:
            state_str = "Reloading";
            break;
    }

    return QJsonObject{
        {"state", state_str},
        {"uptime_ms", static_cast<qint64>(uptime().count())},
        {"message_count", static_cast<qint64>(m_message_count.load())},
        {"command_count", static_cast<qint64>(m_command_count.load())},
        {"error_count", static_cast<qint64>(m_error_count.load())},
        {"custom_message", QString::fromStdString(m_custom_message)},
        {"timer_active", m_timer && m_timer->isActive()}};
}

QJsonObject BasicPlugin::handle_echo_command(const QJsonObject& params) {
    if (!params.contains("message") || !params["message"].isString()) {
        return QJsonObject{{"error", "Missing or invalid 'message' parameter"}};
    }

    QString message = params["message"].toString();
    QString timestamp =
        QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count());

    return QJsonObject{{"echoed_message", message},
                       {"timestamp", timestamp},
                       {"success", true}};
}

QJsonObject BasicPlugin::handle_config_command(const QJsonObject& params) {
    QString action = params.value("action").toString("get");

    if (action == "get") {
        return QJsonObject{{"current_config", m_configuration},
                           {"success", true}};
    } else if (action == "set") {
        if (!params.contains("config") || !params["config"].isObject()) {
            return QJsonObject{
                {"error", "Missing or invalid 'config' parameter"},
                {"success", false}};
        }

        QJsonObject new_config = params["config"].toObject();
        auto result = configure(new_config);

        return QJsonObject{
            {"current_config", m_configuration},
            {"success", result.has_value()},
            {"error",
             result ? "" : QString::fromStdString(result.error().message)}};
    } else {
        return QJsonObject{{"error", "Invalid action. Use 'get' or 'set'"},
                           {"success", false}};
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
            {"details",
             QJsonObject{{"plugin_responsive", true},
                         {"configuration_valid",
                          validate_configuration(m_configuration)},
                         {"timer_working", m_timer && m_timer->isActive()}}}};
    } else if (test_type == "performance") {
        return QJsonObject{{"test_result", "Performance test completed"},
                           {"success", true},
                           {"details", performance_metrics()}};
    } else if (test_type == "stress") {
        // Simulate some work
        for (int i = 0; i < 1000; ++i) {
            m_command_count.fetch_add(1);
        }

        return QJsonObject{
            {"test_result", "Stress test completed"},
            {"success", true},
            {"details",
             QJsonObject{{"iterations", 1000},
                         {"final_command_count",
                          static_cast<qint64>(m_command_count.load())}}}};
    } else {
        return QJsonObject{
            {"test_result", "Unknown test type"},
            {"success", false},
            {"error", "Supported test types: basic, performance, stress"}};
    }
}

// === Enhanced Command Handlers ===

QJsonObject BasicPlugin::handle_lifecycle_command(const QJsonObject& params) {
    QString action = params.value("action").toString();

    if (action == "pause") {
        auto result = pause();
        return QJsonObject{
            {"action", "pause"},
            {"success", result.has_value()},
            {"error",
             result ? "" : QString::fromStdString(result.error().message)},
            {"current_state", static_cast<int>(m_state.load())}};
    } else if (action == "resume") {
        auto result = resume();
        return QJsonObject{
            {"action", "resume"},
            {"success", result.has_value()},
            {"error",
             result ? "" : QString::fromStdString(result.error().message)},
            {"current_state", static_cast<int>(m_state.load())}};
    } else if (action == "restart") {
        auto result = restart();
        return QJsonObject{
            {"action", "restart"},
            {"success", result.has_value()},
            {"error",
             result ? "" : QString::fromStdString(result.error().message)},
            {"current_state", static_cast<int>(m_state.load())}};
    } else if (action == "status") {
        return QJsonObject{
            {"action", "status"},
            {"success", true},
            {"current_state", static_cast<int>(m_state.load())},
            {"is_initialized", is_initialized()},
            {"uptime_ms", static_cast<qint64>(uptime().count())}};
    } else {
        return QJsonObject{
            {"error",
             "Invalid action. Supported: pause, resume, restart, status"},
            {"success", false}};
    }
}

QJsonObject BasicPlugin::handle_monitoring_command(const QJsonObject& params) {
    QString type = params.value("type").toString("all");

    if (type == "performance") {
        return QJsonObject{{"type", "performance"},
                           {"data", performance_metrics()}};
    } else if (type == "resources") {
        return QJsonObject{{"type", "resources"}, {"data", resource_usage()}};
    } else if (type == "errors") {
        std::lock_guard lock(m_error_mutex);
        QJsonArray error_array;
        for (const auto& error : m_error_log) {
            error_array.append(QString::fromStdString(error));
        }
        return QJsonObject{
            {"type", "errors"},
            {"data",
             QJsonObject{
                 {"last_error", QString::fromStdString(m_last_error)},
                 {"error_count", static_cast<qint64>(m_error_count.load())},
                 {"error_log", error_array}}}};
    } else if (type == "all") {
        return QJsonObject{
            {"type", "all"},
            {"performance", performance_metrics()},
            {"resources", resource_usage()},
            {"error_count", static_cast<qint64>(m_error_count.load())}};
    } else {
        return QJsonObject{
            {"error",
             "Invalid type. Supported: performance, resources, errors, all"},
            {"success", false}};
    }
}

QJsonObject BasicPlugin::handle_dependencies_command(
    const QJsonObject& params) {
    Q_UNUSED(params)

    QJsonArray required_array;
    for (const auto& dep : m_required_dependencies) {
        required_array.append(QString::fromStdString(dep));
    }

    QJsonArray optional_array;
    for (const auto& dep : m_optional_dependencies) {
        optional_array.append(QString::fromStdString(dep));
    }

    return QJsonObject{
        {"required_dependencies", required_array},
        {"optional_dependencies", optional_array},
        {"dependencies_satisfied", dependencies_satisfied()},
        {"dependency_count", static_cast<int>(m_required_dependencies.size() +
                                              m_optional_dependencies.size())}};
}

QJsonObject BasicPlugin::handle_capabilities_command(
    const QJsonObject& params) {
    Q_UNUSED(params)

    auto caps = capabilities();
    QJsonArray capabilities_array;

    if (caps & qtplugin::PluginCapability::UI)
        capabilities_array.append("UI");
    if (caps & qtplugin::PluginCapability::Service)
        capabilities_array.append("Service");
    if (caps & qtplugin::PluginCapability::Network)
        capabilities_array.append("Network");
    if (caps & qtplugin::PluginCapability::DataProcessing)
        capabilities_array.append("DataProcessing");
    if (caps & qtplugin::PluginCapability::Scripting)
        capabilities_array.append("Scripting");
    if (caps & qtplugin::PluginCapability::FileSystem)
        capabilities_array.append("FileSystem");
    if (caps & qtplugin::PluginCapability::Database)
        capabilities_array.append("Database");
    if (caps & qtplugin::PluginCapability::AsyncInit)
        capabilities_array.append("AsyncInit");
    if (caps & qtplugin::PluginCapability::HotReload)
        capabilities_array.append("HotReload");
    if (caps & qtplugin::PluginCapability::Configuration)
        capabilities_array.append("Configuration");
    if (caps & qtplugin::PluginCapability::Logging)
        capabilities_array.append("Logging");
    if (caps & qtplugin::PluginCapability::Security)
        capabilities_array.append("Security");
    if (caps & qtplugin::PluginCapability::Threading)
        capabilities_array.append("Threading");
    if (caps & qtplugin::PluginCapability::Monitoring)
        capabilities_array.append("Monitoring");

    return QJsonObject{
        {"capabilities", capabilities_array},
        {"capabilities_bitfield", static_cast<qint64>(caps)},
        {"priority", static_cast<int>(priority())},
        {"is_thread_safe", is_thread_safe()},
        {"thread_model", QString::fromStdString(std::string(thread_model()))}};
}

// Plugin factory implementation is in the header file as static methods
