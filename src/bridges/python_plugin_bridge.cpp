/**
 * @file python_plugin_bridge.cpp
 * @brief Implementation of Python plugin bridge
 * @version 3.2.0
 */

#include "qtplugin/bridges/python_plugin_bridge.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <future>
#include <chrono>

Q_LOGGING_CATEGORY(pythonBridgeLog, "qtplugin.python");

namespace qtplugin {

// === PythonExecutionEnvironment Implementation ===

PythonExecutionEnvironment::PythonExecutionEnvironment(
    const QString& python_path)
    : m_python_path(python_path) {}

PythonExecutionEnvironment::~PythonExecutionEnvironment() { shutdown(); }

qtplugin::expected<void, PluginError> PythonExecutionEnvironment::initialize() {
    if (m_process && m_process->state() == QProcess::Running) {
        return make_success();
    }

    m_process = std::make_unique<QProcess>();
    setup_process();

    // Start Python interpreter with our bridge script
    QString bridge_script =
        QCoreApplication::applicationDirPath() + "/python_bridge.py";
    QStringList arguments;
    arguments << bridge_script;

    qCDebug(pythonBridgeLog)
        << "Starting Python process:" << m_python_path << arguments;

    m_process->start(m_python_path, arguments);

    if (!m_process->waitForStarted(5000)) {
        return make_error<void>(PluginErrorCode::InitializationFailed,
                                "Failed to start Python interpreter: " +
                                    m_process->errorString().toStdString());
    }

    // Send initialization request
    QJsonObject init_request;
    init_request["type"] = "initialize";
    init_request["id"] = ++m_request_id;

    auto response = send_request(init_request);
    if (!response) {
        return make_error<void>(PluginErrorCode::InitializationFailed,
                                "Failed to initialize Python environment: " +
                                    response.error().message);
    }

    if (!response.value()["success"].toBool()) {
        return make_error<void>(PluginErrorCode::InitializationFailed,
                                "Python environment initialization failed");
    }

    qCDebug(pythonBridgeLog) << "Python environment initialized successfully";
    return make_success();
}

void PythonExecutionEnvironment::shutdown() {
    if (!m_process) {
        return;
    }

    if (m_process->state() == QProcess::Running) {
        // Send shutdown request
        QJsonObject shutdown_request;
        shutdown_request["type"] = "shutdown";
        shutdown_request["id"] = ++m_request_id;

        send_request(shutdown_request);

        // Give the process time to shutdown gracefully
        if (!m_process->waitForFinished(3000)) {
            qCWarning(pythonBridgeLog)
                << "Python process did not shutdown gracefully, terminating";
            m_process->terminate();
            if (!m_process->waitForFinished(1000)) {
                m_process->kill();
            }
        }
    }

    m_process.reset();
    qCDebug(pythonBridgeLog) << "Python environment shutdown completed";
}

qtplugin::expected<QJsonObject, PluginError>
PythonExecutionEnvironment::execute_code(const QString& code,
                                         const QJsonObject& context) {
    if (!is_running()) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState,
                                       "Python environment is not running");
    }

    QJsonObject request;
    request["type"] = "execute_code";
    request["id"] = ++m_request_id;
    request["code"] = code;
    request["context"] = context;

    return send_request(request);
}

qtplugin::expected<QString, PluginError>
PythonExecutionEnvironment::load_plugin_module(const QString& plugin_path,
                                               const QString& plugin_class) {
    if (!is_running()) {
        return make_error<QString>(PluginErrorCode::InvalidState,
                                   "Python environment is not running");
    }

    QJsonObject request;
    request["type"] = "load_plugin";
    request["id"] = ++m_request_id;
    request["plugin_path"] = plugin_path;
    request["plugin_class"] = plugin_class;

    auto response = send_request(request);
    if (!response) {
        return make_error<QString>(response.error().code,
                                   response.error().message);
    }

    if (!response.value()["success"].toBool()) {
        return make_error<QString>(
            PluginErrorCode::LoadFailed,
            response.value()["error"].toString().toStdString());
    }

    return response.value()["plugin_id"].toString();
}

qtplugin::expected<QJsonObject, PluginError>
PythonExecutionEnvironment::call_plugin_method(const QString& plugin_id,
                                               const QString& method_name,
                                               const QJsonArray& parameters) {
    if (!is_running()) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState,
                                       "Python environment is not running");
    }

    QJsonObject request;
    request["type"] = "call_method";
    request["id"] = ++m_request_id;
    request["plugin_id"] = plugin_id;
    request["method_name"] = method_name;
    request["parameters"] = parameters;

    return send_request(request);
}

void PythonExecutionEnvironment::setup_process() {
    // Connect process signals
    QObject::connect(m_process.get(), &QProcess::readyReadStandardOutput,
                     [this]() { handle_process_output(); });

    QObject::connect(m_process.get(), &QProcess::readyReadStandardError,
                     [this]() { handle_process_error(); });

    QObject::connect(
        m_process.get(),
        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [](int exitCode, QProcess::ExitStatus exitStatus) {
            qCWarning(pythonBridgeLog)
                << "Python process finished with code:" << exitCode
                << "status:" << exitStatus;
        });
}

void PythonExecutionEnvironment::handle_process_output() {
    if (!m_process)
        return;

    QByteArray data = m_process->readAllStandardOutput();
    QStringList lines = QString::fromUtf8(data).split('\n', Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);

        if (error.error != QJsonParseError::NoError) {
            qCWarning(pythonBridgeLog)
                << "Invalid JSON from Python process:" << line;
            continue;
        }

        QJsonObject response = doc.object();
        int response_id = response["id"].toInt();

        QMutexLocker locker(&m_mutex);
        if (m_waiting_for_response && response_id == m_request_id) {
            m_last_response = response;
            m_waiting_for_response = false;
            m_response_condition.wakeAll();
        }
    }
}

void PythonExecutionEnvironment::handle_process_error() {
    if (!m_process)
        return;

    QByteArray data = m_process->readAllStandardError();
    QString error_text = QString::fromUtf8(data);
    qCWarning(pythonBridgeLog) << "Python process error:" << error_text;
}

qtplugin::expected<QJsonObject, PluginError>
PythonExecutionEnvironment::send_request(const QJsonObject& request) {
    if (!m_process || m_process->state() != QProcess::Running) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState,
                                       "Python process is not running");
    }

    QMutexLocker locker(&m_mutex);

    // Send request
    QJsonDocument doc(request);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
    m_process->write(data);

    // Wait for response
    m_waiting_for_response = true;
    if (!m_response_condition.wait(&m_mutex, 30000)) {  // 30 second timeout
        m_waiting_for_response = false;
        return make_error<QJsonObject>(PluginErrorCode::TimeoutError,
                                       "Timeout waiting for Python response");
    }

    return m_last_response;
}

// === PythonPluginBridge Implementation ===

PythonPluginBridge::PythonPluginBridge(const QString& plugin_path,
                                       QObject* parent)
    : QObject(parent),
      m_plugin_path(plugin_path),
      m_environment(std::make_unique<PythonExecutionEnvironment>()) {}

PythonPluginBridge::~PythonPluginBridge() = default;

std::string_view PythonPluginBridge::name() const noexcept {
    return "Python Plugin Bridge";
}

std::string_view PythonPluginBridge::description() const noexcept {
    return "Bridge for executing Python-based plugins";
}

qtplugin::Version PythonPluginBridge::version() const noexcept {
    return qtplugin::Version(1, 0, 0);
}

std::string_view PythonPluginBridge::author() const noexcept {
    return "QtForge Team";
}

std::string PythonPluginBridge::id() const noexcept { return "python-bridge"; }

qtplugin::expected<void, qtplugin::PluginError>
PythonPluginBridge::initialize() {
    // Initialize the Python environment
    auto env_result = m_environment->initialize();
    if (!env_result) {
        return env_result;
    }

    // If we have a plugin path, load the plugin
    if (!m_plugin_path.isEmpty()) {
        auto load_result = m_environment->load_plugin_module(m_plugin_path, "");
        if (!load_result) {
            return make_error<void>(load_result.error().code, load_result.error().message);
        }

        m_current_plugin_id = load_result.value();
        m_loaded_plugins[m_current_plugin_id] = m_plugin_path;

        // Get plugin information using execute_code to call our bridge functions
        QString info_code = QString(R"(
import json
bridge = globals().get('bridge')
if bridge and hasattr(bridge, 'handle_get_plugin_info'):
    request = {'type': 'get_plugin_info', 'id': 1, 'plugin_id': '%1'}
    response = bridge.handle_get_plugin_info(request)
    json.dumps(response)
else:
    json.dumps({'success': False, 'error': 'Bridge not available'})
)").arg(m_current_plugin_id);

        auto info_response = m_environment->execute_code(info_code);
        if (info_response) {
            // Parse the JSON response
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(info_response.value()["result"].toString().toUtf8(), &error);

            if (error.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject response_data = doc.object();

                if (response_data["success"].toBool()) {
                    // Extract metadata
                    if (response_data.contains("metadata")) {
                        m_metadata = response_data["metadata"].toObject();
                    }

                    // Extract available methods
                    if (response_data.contains("methods")) {
                        QJsonArray methods = response_data["methods"].toArray();
                        m_available_methods.clear();
                        for (const QJsonValue& method : methods) {
                            if (method.isObject()) {
                                QString method_name = method.toObject()["name"].toString();
                                if (!method_name.isEmpty()) {
                                    m_available_methods.push_back(method_name);
                                }
                            }
                        }
                    }

                    // Extract available properties
                    if (response_data.contains("properties")) {
                        QJsonArray properties = response_data["properties"].toArray();
                        m_available_properties.clear();
                        for (const QJsonValue& property : properties) {
                            if (property.isObject()) {
                                QString prop_name = property.toObject()["name"].toString();
                                if (!prop_name.isEmpty()) {
                                    m_available_properties.push_back(prop_name);
                                }
                            }
                        }
                    }
                }
            }
        }

        m_state = qtplugin::PluginState::Running;
        qCDebug(pythonBridgeLog) << "Python plugin initialized:" << m_plugin_path;
    }

    return make_success();
}

void PythonPluginBridge::shutdown() noexcept {
    if (m_environment) {
        m_environment->shutdown();
    }
    m_loaded_plugins.clear();
    qCDebug(pythonBridgeLog) << "Python plugin bridge shutdown completed";
}

qtplugin::PluginState PythonPluginBridge::state() const noexcept {
    return qtplugin::PluginState::Loaded;
}

qtplugin::PluginCapabilities PythonPluginBridge::capabilities() const noexcept {
    return static_cast<qtplugin::PluginCapabilities>(
        qtplugin::PluginCapability::Scripting |
        qtplugin::PluginCapability::HotReload);
}

qtplugin::expected<void, qtplugin::PluginError> PythonPluginBridge::configure(
    const QJsonObject& config) {
    Q_UNUSED(config)
    return {};
}

QJsonObject PythonPluginBridge::current_configuration() const {
    return QJsonObject{};
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
PythonPluginBridge::execute_command(std::string_view command,
                                    const QJsonObject& parameters) {
    if (!m_environment || !m_environment->is_running()) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState,
                                       "Python environment is not running");
    }

    // Check if we have a loaded plugin
    if (m_current_plugin_id.isEmpty()) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState,
                                       "No plugin loaded");
    }

    // Call the command method on the loaded plugin
    QJsonArray params;
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        params.append(it.value());
    }

    return m_environment->call_plugin_method(m_current_plugin_id,
                                             QString::fromUtf8(command.data(), command.size()),
                                             params);
}

std::vector<std::string> PythonPluginBridge::available_commands() const {
    std::vector<std::string> commands;
    for (const QString& method : m_available_methods) {
        commands.push_back(method.toStdString());
    }
    return commands;
}

bool PythonPluginBridge::validate_configuration(
    const QJsonObject& config) const {
    Q_UNUSED(config)
    return true;
}

QJsonObject PythonPluginBridge::get_configuration_schema() const {
    return QJsonObject{};
}

qtplugin::expected<void, qtplugin::PluginError>
PythonPluginBridge::handle_dependency_change(const QString& dependency_id,
                                              qtplugin::PluginState new_state) {
    qCDebug(pythonBridgeLog) << "Handling dependency change:" << dependency_id
                             << "new state:" << static_cast<int>(new_state);

    if (!m_environment || !m_environment->is_running()) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Python environment is not running");
    }

    if (m_current_plugin_id.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "No plugin loaded");
    }

    // Try to call handle_dependency_change method on the plugin if it exists
    QJsonArray params;
    params.append(dependency_id);
    params.append(static_cast<int>(new_state));

    auto result = m_environment->call_plugin_method(
        m_current_plugin_id,
        "handle_dependency_change",
        params
    );

    if (result) {
        qCDebug(pythonBridgeLog) << "Plugin handled dependency change successfully";
        return make_success();
    } else {
        // It's okay if the plugin doesn't have this method
        qCDebug(pythonBridgeLog) << "Plugin doesn't have handle_dependency_change method, ignoring";
        return make_success();
    }
}

qtplugin::expected<void, qtplugin::PluginError>
PythonPluginBridge::hot_reload() {
    if (!m_environment || !m_environment->is_running()) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Python environment is not running");
    }

    if (m_current_plugin_id.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "No plugin loaded");
    }

    // Save current state
    QString old_plugin_id = m_current_plugin_id;
    QString plugin_path = m_loaded_plugins.value(old_plugin_id);

    // Reload the plugin
    auto load_result = m_environment->load_plugin_module(plugin_path, "");
    if (!load_result) {
        return make_error<void>(load_result.error().code, load_result.error().message);
    }

    // Update plugin ID
    m_current_plugin_id = load_result.value();
    m_loaded_plugins.remove(old_plugin_id);
    m_loaded_plugins[m_current_plugin_id] = plugin_path;

    qCDebug(pythonBridgeLog) << "Hot reload completed for plugin:" << plugin_path;
    return make_success();
}



std::vector<qtplugin::InterfaceDescriptor>
PythonPluginBridge::get_interface_descriptors() const {
    return {};
}

bool PythonPluginBridge::supports_interface(
    const QString& interface_id, const qtplugin::Version& version) const {
    Q_UNUSED(interface_id)
    Q_UNUSED(version)
    return false;
}

std::optional<qtplugin::InterfaceDescriptor>
PythonPluginBridge::get_interface_descriptor(
    const QString& interface_id) const {
    Q_UNUSED(interface_id)
    return std::nullopt;
}

qtplugin::expected<void, qtplugin::PluginError>
PythonPluginBridge::adapt_to_interface(const QString& interface_id,
                                       const qtplugin::Version& version) {
    Q_UNUSED(interface_id)
    Q_UNUSED(version)
    return qtplugin::unexpected(qtplugin::PluginError{
        qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

qtplugin::expected<std::vector<qtplugin::InterfaceCapability>,
                   qtplugin::PluginError>
PythonPluginBridge::negotiate_capabilities(
    const QString& interface_id,
    const std::vector<qtplugin::InterfaceCapability>& requested_capabilities) {
    Q_UNUSED(interface_id)
    Q_UNUSED(requested_capabilities)
    return qtplugin::unexpected(qtplugin::PluginError{
        qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

qtplugin::PluginType PythonPluginBridge::get_plugin_type() const {
    return qtplugin::PluginType::Python;
}

qtplugin::PluginExecutionContext PythonPluginBridge::get_execution_context()
    const {
    qtplugin::PluginExecutionContext context;
    context.type = qtplugin::PluginType::Python;
    context.interpreter_path = "python";
    return context;
}

qtplugin::expected<QVariant, qtplugin::PluginError>
PythonPluginBridge::execute_code(const QString& code,
                                 const QJsonObject& context) {
    auto result = m_environment->execute_code(code, context);
    if (result) {
        return QVariant::fromValue(result.value());
    }
    return qtplugin::unexpected(result.error());
}

qtplugin::expected<QVariant, qtplugin::PluginError>
PythonPluginBridge::invoke_method(const QString& method_name,
                                  const QVariantList& parameters,
                                  const QString& interface_id) {
    Q_UNUSED(interface_id)  // For now, we ignore interface_id

    if (!m_environment || !m_environment->is_running()) {
        return make_error<QVariant>(PluginErrorCode::InvalidState,
                                    "Python environment is not running");
    }

    if (m_current_plugin_id.isEmpty()) {
        return make_error<QVariant>(PluginErrorCode::InvalidState,
                                    "No plugin loaded");
    }

    // Convert QVariantList to QJsonArray
    QJsonArray params;
    for (const QVariant& param : parameters) {
        params.append(QJsonValue::fromVariant(param));
    }

    auto result = m_environment->call_plugin_method(m_current_plugin_id, method_name, params);
    if (result) {
        return QVariant::fromValue(result.value());
    }
    return qtplugin::unexpected(result.error());
}

std::vector<QString> PythonPluginBridge::get_available_methods(
    const QString& interface_id) const {
    Q_UNUSED(interface_id)  // For now, we return all available methods regardless of interface
    return m_available_methods;
}



qtplugin::expected<QVariant, qtplugin::PluginError>
PythonPluginBridge::get_property(const QString& property_name,
                                 const QString& interface_id) {
    Q_UNUSED(interface_id)  // For now, we ignore interface_id

    if (!m_environment || !m_environment->is_running()) {
        return make_error<QVariant>(PluginErrorCode::InvalidState,
                                    "Python environment is not running");
    }

    if (m_current_plugin_id.isEmpty()) {
        return make_error<QVariant>(PluginErrorCode::InvalidState,
                                    "No plugin loaded");
    }

    // Create a Python code snippet to get the property
    QString code = QString("getattr(plugin, '%1', None)").arg(property_name);

    auto result = m_environment->execute_code(code, QJsonObject());
    if (result) {
        return QVariant::fromValue(result.value());
    }
    return qtplugin::unexpected(result.error());
}

qtplugin::expected<void, qtplugin::PluginError>
PythonPluginBridge::set_property(const QString& property_name,
                                 const QVariant& value,
                                 const QString& interface_id) {
    Q_UNUSED(interface_id)  // For now, we ignore interface_id

    if (!m_environment || !m_environment->is_running()) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Python environment is not running");
    }

    if (m_current_plugin_id.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "No plugin loaded");
    }

    // Create a Python code snippet to set the property
    QString valueStr;
    if (value.typeId() == QMetaType::QString) {
        valueStr = QString("'%1'").arg(value.toString());
    } else {
        valueStr = value.toString();
    }

    QString code = QString("setattr(plugin, '%1', %2)").arg(property_name, valueStr);

    auto result = m_environment->execute_code(code, QJsonObject());
    if (result) {
        return make_success();
    }
    return make_error<void>(result.error().code, result.error().message);
}

std::vector<QString> PythonPluginBridge::get_available_properties(
    const QString& interface_id) const {
    Q_UNUSED(interface_id)  // For now, we return all available properties regardless of interface
    return m_available_properties;
}

qtplugin::expected<void, qtplugin::PluginError>
PythonPluginBridge::subscribe_to_events(
    const QString& source_plugin_id, const std::vector<QString>& event_types,
    std::function<void(const QString&, const QJsonObject&)> callback) {
    if (!m_environment || !m_environment->is_running()) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Python environment is not running");
    }

    // Store the callback for each event
    for (const QString& event_type : event_types) {
        QString full_event_name = source_plugin_id.isEmpty() ? event_type : source_plugin_id + "." + event_type;
        m_event_callbacks[full_event_name] = callback;
    }

    // Notify Python plugin about event subscription
    if (!m_current_plugin_id.isEmpty()) {
        QJsonArray event_array;
        for (const QString& event_type : event_types) {
            event_array.append(event_type);
        }

        auto result = m_environment->call_plugin_method(
            m_current_plugin_id,
            "subscribe_events",
            event_array
        );

        if (!result) {
            qCWarning(pythonBridgeLog) << "Failed to notify Python plugin about event subscription:"
                                       << result.error().message.c_str();
        }
    }

    return make_success();
}

qtplugin::expected<void, qtplugin::PluginError>
PythonPluginBridge::unsubscribe_from_events(
    const QString& source_plugin_id, const std::vector<QString>& event_types) {
    // Remove callbacks for each event
    for (const QString& event_type : event_types) {
        QString full_event_name = source_plugin_id.isEmpty() ? event_type : source_plugin_id + "." + event_type;
        m_event_callbacks.remove(full_event_name);
    }

    // Notify Python plugin about event unsubscription
    if (!m_current_plugin_id.isEmpty()) {
        QJsonArray event_array;
        for (const QString& event_type : event_types) {
            event_array.append(event_type);
        }

        auto result = m_environment->call_plugin_method(
            m_current_plugin_id,
            "unsubscribe_events",
            event_array
        );

        if (!result) {
            qCWarning(pythonBridgeLog) << "Failed to notify Python plugin about event unsubscription:"
                                       << result.error().message.c_str();
        }
    }

    return make_success();
}

qtplugin::expected<void, qtplugin::PluginError> PythonPluginBridge::emit_event(
    const QString& event_name, const QJsonObject& event_data) {
    // Call registered callbacks for this event
    auto it = m_event_callbacks.find(event_name);
    if (it != m_event_callbacks.end()) {
        try {
            it.value()(event_name, event_data);
        } catch (const std::exception& e) {
            return make_error<void>(PluginErrorCode::ExecutionFailed,
                                    "Error in event callback: " + std::string(e.what()));
        }
    }

    // Also notify Python plugin about the event
    if (!m_current_plugin_id.isEmpty()) {
        QJsonArray params;
        params.append(event_name);
        params.append(event_data);

        auto result = m_environment->call_plugin_method(
            m_current_plugin_id,
            "emit_event",
            params
        );

        if (!result) {
            qCWarning(pythonBridgeLog) << "Failed to notify Python plugin about event emission:"
                                       << result.error().message.c_str();
        }
    }

    return make_success();
}

void PythonPluginBridge::handle_environment_error() {
    qCWarning(pythonBridgeLog) << "Python environment error detected";

    // Update plugin state to error
    m_state = qtplugin::PluginState::Error;

    // Clear current plugin information
    m_current_plugin_id.clear();
    m_available_methods.clear();
    m_available_properties.clear();
    m_event_callbacks.clear();

    // Try to restart the environment if it's not running
    if (m_environment && !m_environment->is_running()) {
        qCDebug(pythonBridgeLog) << "Attempting to restart Python environment";

        auto restart_result = m_environment->initialize();
        if (restart_result) {
            qCDebug(pythonBridgeLog) << "Python environment restarted successfully";
            m_state = qtplugin::PluginState::Loaded;

            // Try to reload the plugin if we have a path
            if (!m_plugin_path.isEmpty()) {
                auto reload_result = hot_reload();
                if (reload_result) {
                    qCDebug(pythonBridgeLog) << "Plugin reloaded successfully after environment restart";
                } else {
                    qCWarning(pythonBridgeLog) << "Failed to reload plugin after environment restart:"
                                               << reload_result.error().message.c_str();
                }
            }
        } else {
            qCCritical(pythonBridgeLog) << "Failed to restart Python environment:"
                                        << restart_result.error().message.c_str();
        }
    }
}

qtplugin::expected<void, qtplugin::PluginError>
PythonPluginBridge::discover_methods_and_properties() {
    if (!m_environment || !m_environment->is_running()) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Python environment is not running");
    }

    if (m_current_plugin_id.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "No plugin loaded");
    }

    // Get plugin information using the bridge
    QString info_code = QString(R"(
import json
bridge = globals().get('bridge')
if bridge and hasattr(bridge, 'handle_get_plugin_info'):
    request = {'type': 'get_plugin_info', 'id': 1, 'plugin_id': '%1'}
    response = bridge.handle_get_plugin_info(request)
    json.dumps(response)
else:
    json.dumps({'success': False, 'error': 'Bridge not available'})
)").arg(m_current_plugin_id);

    auto info_response = m_environment->execute_code(info_code);
    if (!info_response) {
        return make_error<void>(info_response.error().code, info_response.error().message);
    }

    // Parse the JSON response
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(info_response.value()["result"].toString().toUtf8(), &error);

    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return make_error<void>(PluginErrorCode::ExecutionFailed,
                                "Failed to parse plugin information response");
    }

    QJsonObject response_data = doc.object();

    if (!response_data["success"].toBool()) {
        return make_error<void>(PluginErrorCode::ExecutionFailed,
                                response_data["error"].toString().toStdString());
    }

    // Extract methods
    m_available_methods.clear();
    if (response_data.contains("methods")) {
        QJsonArray methods = response_data["methods"].toArray();
        for (const QJsonValue& method : methods) {
            if (method.isObject()) {
                QString method_name = method.toObject()["name"].toString();
                if (!method_name.isEmpty()) {
                    m_available_methods.push_back(method_name);
                }
            }
        }
    }

    // Extract properties
    m_available_properties.clear();
    if (response_data.contains("properties")) {
        QJsonArray properties = response_data["properties"].toArray();
        for (const QJsonValue& property : properties) {
            if (property.isObject()) {
                QString prop_name = property.toObject()["name"].toString();
                if (!prop_name.isEmpty()) {
                    m_available_properties.push_back(prop_name);
                }
            }
        }
    }

    qCDebug(pythonBridgeLog) << "Discovered" << m_available_methods.size() << "methods and"
                             << m_available_properties.size() << "properties";

    return make_success();
}

QJsonArray PythonPluginBridge::convert_variant_list_to_json(const QVariantList& list) const {
    QJsonArray json_array;
    for (const QVariant& variant : list) {
        json_array.append(QJsonValue::fromVariant(variant));
    }
    return json_array;
}

QVariant PythonPluginBridge::convert_json_to_variant(const QJsonValue& value) const {
    return value.toVariant();
}

std::optional<QJsonObject> PythonPluginBridge::get_method_signature(
    const QString& method_name, const QString& interface_id) const {
    Q_UNUSED(interface_id)  // For now, we ignore interface_id

    if (!m_environment || !m_environment->is_running()) {
        return std::nullopt;
    }

    if (m_current_plugin_id.isEmpty()) {
        return std::nullopt;
    }

    // Get method signature from Python using inspect module
    QString code = QString(R"(
import json
import inspect
try:
    method = getattr(plugin, '%1', None)
    if method and callable(method):
        sig = inspect.signature(method)
        result = {
            'name': '%1',
            'signature': str(sig),
            'parameters': []
        }

        for param_name, param in sig.parameters.items():
            param_info = {
                'name': param_name,
                'kind': str(param.kind),
                'default': str(param.default) if param.default != inspect.Parameter.empty else None,
                'annotation': str(param.annotation) if param.annotation != inspect.Parameter.empty else None
            }
            result['parameters'].append(param_info)

        if sig.return_annotation != inspect.Parameter.empty:
            result['return_type'] = str(sig.return_annotation)

        if hasattr(method, '__doc__') and method.__doc__:
            result['docstring'] = method.__doc__.strip()

        json.dumps(result)
    else:
        json.dumps(None)
except Exception as e:
    json.dumps({'error': str(e)})
)").arg(method_name);

    // This is a const method, so we can't call execute_code directly
    // We'll return a basic signature for now
    QJsonObject signature;
    signature["name"] = method_name;
    signature["signature"] = QString("(%1(...))").arg(method_name);
    signature["parameters"] = QJsonArray();

    return signature;
}

// === PythonPluginFactory Implementation ===

QStringList PythonPluginFactory::required_python_modules() {
    return QStringList{
        "json",
        "sys",
        "os",
        "importlib",
        "importlib.util",
        "traceback",
        "logging",
        "inspect"
    };
}

QStringList PythonPluginFactory::check_required_modules(const QString& python_path) {
    QStringList missing_modules;
    QStringList required = required_python_modules();

    // Create a temporary process to check module availability
    QProcess process;

    for (const QString& module : required) {
        QStringList arguments;
        arguments << "-c" << QString("import %1").arg(module);

        process.start(python_path, arguments);
        if (!process.waitForFinished(5000)) {
            missing_modules << module + " (timeout)";
            process.kill();
            continue;
        }

        if (process.exitCode() != 0) {
            QString error = process.readAllStandardError();
            missing_modules << module + " (" + error.trimmed() + ")";
        }
    }

    return missing_modules;
}

// === IAdvancedPlugin Implementation ===

std::vector<contracts::ServiceContract> PythonPluginBridge::get_service_contracts() const {
    // Return empty vector for now - Python plugins don't expose services by default
    return {};
}

qtplugin::expected<QJsonObject, qtplugin::PluginError> PythonPluginBridge::call_service(
    const QString& service_name, const QString& method_name,
    const QJsonObject& parameters, std::chrono::milliseconds timeout) {

    Q_UNUSED(timeout)

    // Delegate to Python plugin's service handling
    QVariantList params;
    params << service_name << method_name << parameters;

    auto result = invoke_method("handle_service_call", params);
    if (!result.has_value()) {
        return qtplugin::unexpected(result.error());
    }

    // Convert result to QJsonObject
    QVariantMap resultMap = result.value().toMap();
    return QJsonObject::fromVariantMap(resultMap);
}

std::future<qtplugin::expected<QJsonObject, qtplugin::PluginError>>
PythonPluginBridge::call_service_async(const QString& service_name, const QString& method_name,
                                      const QJsonObject& parameters, std::chrono::milliseconds timeout) {

    // For now, just run synchronously in a future
    return std::async(std::launch::async, [this, service_name, method_name, parameters, timeout]() {
        return call_service(service_name, method_name, parameters, timeout);
    });
}

qtplugin::expected<QJsonObject, qtplugin::PluginError> PythonPluginBridge::handle_service_call(
    const QString& service_name, const QString& method_name,
    const QJsonObject& parameters) {

    // Delegate to Python plugin's service handling
    QVariantList params;
    params << service_name << method_name << parameters;

    auto result = invoke_method("handle_service_call", params);
    if (!result.has_value()) {
        return qtplugin::unexpected(result.error());
    }

    // Convert result to QJsonObject
    QVariantMap resultMap = result.value().toMap();
    return QJsonObject::fromVariantMap(resultMap);
}

}  // namespace qtplugin
