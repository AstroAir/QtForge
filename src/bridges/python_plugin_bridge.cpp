/**
 * @file python_plugin_bridge.cpp
 * @brief Implementation of Python plugin bridge
 * @version 3.2.0
 */

#include "../../include/qtplugin/bridges/python_plugin_bridge.hpp"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QDebug>

Q_LOGGING_CATEGORY(pythonBridgeLog, "qtplugin.python");

namespace qtplugin {

// === PythonExecutionEnvironment Implementation ===

PythonExecutionEnvironment::PythonExecutionEnvironment(const QString& python_path)
    : m_python_path(python_path) {
}

PythonExecutionEnvironment::~PythonExecutionEnvironment() {
    shutdown();
}

qtplugin::expected<void, PluginError> PythonExecutionEnvironment::initialize() {
    if (m_process && m_process->state() == QProcess::Running) {
        return make_success();
    }

    m_process = std::make_unique<QProcess>();
    setup_process();

    // Start Python interpreter with our bridge script
    QString bridge_script = QCoreApplication::applicationDirPath() + "/python_bridge.py";
    QStringList arguments;
    arguments << bridge_script;

    qCDebug(pythonBridgeLog) << "Starting Python process:" << m_python_path << arguments;

    m_process->start(m_python_path, arguments);

    if (!m_process->waitForStarted(5000)) {
        return make_error<void>(PluginErrorCode::InitializationFailed,
                               "Failed to start Python interpreter: " + m_process->errorString().toStdString());
    }

    // Send initialization request
    QJsonObject init_request;
    init_request["type"] = "initialize";
    init_request["id"] = ++m_request_id;

    auto response = send_request(init_request);
    if (!response) {
        return make_error<void>(PluginErrorCode::InitializationFailed,
                               "Failed to initialize Python environment: " + response.error().message);
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
            qCWarning(pythonBridgeLog) << "Python process did not shutdown gracefully, terminating";
            m_process->terminate();
            if (!m_process->waitForFinished(1000)) {
                m_process->kill();
            }
        }
    }

    m_process.reset();
    qCDebug(pythonBridgeLog) << "Python environment shutdown completed";
}

qtplugin::expected<QJsonObject, PluginError> PythonExecutionEnvironment::execute_code(
    const QString& code, const QJsonObject& context) {

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

qtplugin::expected<QString, PluginError> PythonExecutionEnvironment::load_plugin_module(
    const QString& plugin_path, const QString& plugin_class) {

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
        return make_error<QString>(response.error().code, response.error().message);
    }

    if (!response.value()["success"].toBool()) {
        return make_error<QString>(PluginErrorCode::LoadFailed,
                                  response.value()["error"].toString().toStdString());
    }

    return response.value()["plugin_id"].toString();
}

qtplugin::expected<QJsonObject, PluginError> PythonExecutionEnvironment::call_plugin_method(
    const QString& plugin_id, const QString& method_name, const QJsonArray& parameters) {

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

    QObject::connect(m_process.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [this](int exitCode, QProcess::ExitStatus exitStatus) {
                         qCWarning(pythonBridgeLog) << "Python process finished with code:" << exitCode
                                                    << "status:" << exitStatus;
                     });
}

void PythonExecutionEnvironment::handle_process_output() {
    if (!m_process) return;

    QByteArray data = m_process->readAllStandardOutput();
    QStringList lines = QString::fromUtf8(data).split('\n', Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);

        if (error.error != QJsonParseError::NoError) {
            qCWarning(pythonBridgeLog) << "Invalid JSON from Python process:" << line;
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
    if (!m_process) return;

    QByteArray data = m_process->readAllStandardError();
    QString error_text = QString::fromUtf8(data);
    qCWarning(pythonBridgeLog) << "Python process error:" << error_text;
}

qtplugin::expected<QJsonObject, PluginError> PythonExecutionEnvironment::send_request(
    const QJsonObject& request) {

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
    if (!m_response_condition.wait(&m_mutex, 30000)) { // 30 second timeout
        m_waiting_for_response = false;
        return make_error<QJsonObject>(PluginErrorCode::TimeoutError,
                                      "Timeout waiting for Python response");
    }

    return m_last_response;
}

// === PythonPluginBridge Implementation ===

PythonPluginBridge::PythonPluginBridge(const QString& plugin_path, QObject* parent)
    : QObject(parent), m_plugin_path(plugin_path), m_environment(std::make_unique<PythonExecutionEnvironment>()) {
}

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

std::string PythonPluginBridge::id() const noexcept {
    return "python-bridge";
}

qtplugin::expected<void, qtplugin::PluginError> PythonPluginBridge::initialize() {
    return m_environment->initialize();
}

void PythonPluginBridge::shutdown() noexcept {
    // TODO: Implement shutdown
}

qtplugin::PluginState PythonPluginBridge::state() const noexcept {
    return qtplugin::PluginState::Loaded;
}

qtplugin::PluginCapabilities PythonPluginBridge::capabilities() const noexcept {
    return static_cast<qtplugin::PluginCapabilities>(qtplugin::PluginCapability::Scripting | qtplugin::PluginCapability::HotReload);
}

qtplugin::expected<void, qtplugin::PluginError> PythonPluginBridge::configure(const QJsonObject& config) {
    Q_UNUSED(config)
    return {};
}

QJsonObject PythonPluginBridge::current_configuration() const {
    return QJsonObject{};
}

qtplugin::expected<QJsonObject, qtplugin::PluginError> PythonPluginBridge::execute_command(
    std::string_view command, const QJsonObject& parameters) {
    Q_UNUSED(command)
    Q_UNUSED(parameters)
    return qtplugin::unexpected(qtplugin::PluginError{qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

std::vector<std::string> PythonPluginBridge::available_commands() const {
    return {};
}

bool PythonPluginBridge::validate_configuration(const QJsonObject& config) const {
    Q_UNUSED(config)
    return true;
}

QJsonObject PythonPluginBridge::get_configuration_schema() const {
    return QJsonObject{};
}

qtplugin::expected<void, qtplugin::PluginError> PythonPluginBridge::hot_reload() {
    return qtplugin::unexpected(qtplugin::PluginError{qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

qtplugin::expected<void, qtplugin::PluginError> PythonPluginBridge::handle_dependency_change(
    const QString& dependency_id, qtplugin::PluginState new_state) {
    Q_UNUSED(dependency_id)
    Q_UNUSED(new_state)
    return qtplugin::unexpected(qtplugin::PluginError{qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

std::vector<qtplugin::InterfaceDescriptor> PythonPluginBridge::get_interface_descriptors() const {
    return {};
}

bool PythonPluginBridge::supports_interface(const QString& interface_id, const qtplugin::Version& version) const {
    Q_UNUSED(interface_id)
    Q_UNUSED(version)
    return false;
}

std::optional<qtplugin::InterfaceDescriptor> PythonPluginBridge::get_interface_descriptor(
    const QString& interface_id) const {
    Q_UNUSED(interface_id)
    return std::nullopt;
}

qtplugin::expected<void, qtplugin::PluginError> PythonPluginBridge::adapt_to_interface(
    const QString& interface_id, const qtplugin::Version& version) {
    Q_UNUSED(interface_id)
    Q_UNUSED(version)
    return qtplugin::unexpected(qtplugin::PluginError{qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

qtplugin::expected<std::vector<qtplugin::InterfaceCapability>, qtplugin::PluginError> PythonPluginBridge::negotiate_capabilities(
    const QString& interface_id, const std::vector<qtplugin::InterfaceCapability>& requested_capabilities) {
    Q_UNUSED(interface_id)
    Q_UNUSED(requested_capabilities)
    return qtplugin::unexpected(qtplugin::PluginError{qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

qtplugin::PluginType PythonPluginBridge::get_plugin_type() const {
    return qtplugin::PluginType::Python;
}

qtplugin::PluginExecutionContext PythonPluginBridge::get_execution_context() const {
    qtplugin::PluginExecutionContext context;
    context.type = qtplugin::PluginType::Python;
    context.interpreter_path = "python";
    return context;
}

qtplugin::expected<QVariant, qtplugin::PluginError> PythonPluginBridge::execute_code(
    const QString& code, const QJsonObject& context) {
    auto result = m_environment->execute_code(code, context);
    if (result) {
        return QVariant::fromValue(result.value());
    }
    return qtplugin::unexpected(result.error());
}

qtplugin::expected<QVariant, qtplugin::PluginError> PythonPluginBridge::invoke_method(
    const QString& object_name, const QList<QVariant>& arguments, const QString& method_name) {
    Q_UNUSED(object_name)
    Q_UNUSED(arguments)
    Q_UNUSED(method_name)
    return qtplugin::unexpected(qtplugin::PluginError{qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

std::vector<QString> PythonPluginBridge::get_available_methods(const QString& object_name) const {
    Q_UNUSED(object_name)
    return {};
}

std::optional<QJsonObject> PythonPluginBridge::get_method_signature(
    const QString& method_name, const QString& interface_id) const {
    Q_UNUSED(method_name)
    Q_UNUSED(interface_id)
    return std::nullopt;
}

qtplugin::expected<QVariant, qtplugin::PluginError> PythonPluginBridge::get_property(
    const QString& object_name, const QString& property_name) {
    Q_UNUSED(object_name)
    Q_UNUSED(property_name)
    return qtplugin::unexpected(qtplugin::PluginError{qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

qtplugin::expected<void, qtplugin::PluginError> PythonPluginBridge::set_property(
    const QString& object_name, const QVariant& value, const QString& property_name) {
    Q_UNUSED(object_name)
    Q_UNUSED(value)
    Q_UNUSED(property_name)
    return qtplugin::unexpected(qtplugin::PluginError{qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

std::vector<QString> PythonPluginBridge::get_available_properties(const QString& object_name) const {
    Q_UNUSED(object_name)
    return {};
}

qtplugin::expected<void, qtplugin::PluginError> PythonPluginBridge::subscribe_to_events(
    const QString& object_name, const std::vector<QString>& event_names,
    std::function<void(const QString&, const QJsonObject&)> callback) {
    Q_UNUSED(object_name)
    Q_UNUSED(event_names)
    Q_UNUSED(callback)
    return qtplugin::unexpected(qtplugin::PluginError{qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

qtplugin::expected<void, qtplugin::PluginError> PythonPluginBridge::unsubscribe_from_events(
    const QString& object_name, const std::vector<QString>& event_names) {
    Q_UNUSED(object_name)
    Q_UNUSED(event_names)
    return qtplugin::unexpected(qtplugin::PluginError{qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

qtplugin::expected<void, qtplugin::PluginError> PythonPluginBridge::emit_event(
    const QString& event_name, const QJsonObject& event_data) {
    Q_UNUSED(event_name)
    Q_UNUSED(event_data)
    return qtplugin::unexpected(qtplugin::PluginError{qtplugin::PluginErrorCode::NotImplemented, "Not implemented"});
}

void PythonPluginBridge::handle_environment_error() {
    // TODO: Implement error handling
}

} // namespace qtplugin
