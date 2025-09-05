/**
 * @file lua_plugin_bridge.cpp
 * @brief Implementation of Lua plugin bridge
 * @version 3.2.0
 */

#include "qtplugin/bridges/lua_plugin_bridge.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QUuid>

// Include sol2 only in implementation
#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

Q_LOGGING_CATEGORY(luaBridgeLog, "qtplugin.lua");

namespace qtplugin {

// === LuaExecutionEnvironment Implementation ===

LuaExecutionEnvironment::LuaExecutionEnvironment() = default;

LuaExecutionEnvironment::~LuaExecutionEnvironment() {
    shutdown();
}

qtplugin::expected<void, PluginError> LuaExecutionEnvironment::initialize() {
#ifndef QTFORGE_LUA_BINDINGS
    return make_error<void>(PluginErrorCode::NotSupported,
                           "Lua bindings not compiled in this build");
#else
    QMutexLocker locker(&m_mutex);

    if (m_initialized) {
        return make_success();
    }

    try {
        m_lua_state = std::make_unique<sol::state>();
        setup_lua_environment();

        if (m_sandbox_enabled) {
            setup_sandbox();
        }

        register_qt_bindings();

        m_initialized = true;
        qCDebug(luaBridgeLog) << "Lua execution environment initialized";
        return make_success();

    } catch (const sol::error& e) {
        qCWarning(luaBridgeLog) << "Failed to initialize Lua environment:" << e.what();
        return make_error<void>(PluginErrorCode::InitializationFailed,
                               std::string("Lua initialization failed: ") + e.what());
    } catch (const std::exception& e) {
        qCWarning(luaBridgeLog) << "Failed to initialize Lua environment:" << e.what();
        return make_error<void>(PluginErrorCode::InitializationFailed,
                               std::string("Lua initialization failed: ") + e.what());
    }
#endif
}

void LuaExecutionEnvironment::shutdown() {
#ifdef QTFORGE_LUA_BINDINGS
    QMutexLocker locker(&m_mutex);

    if (!m_initialized) {
        return;
    }

    try {
        m_loaded_plugins.clear();
        m_lua_state.reset();
        m_initialized = false;
        qCDebug(luaBridgeLog) << "Lua execution environment shut down";
    } catch (const std::exception& e) {
        qCWarning(luaBridgeLog) << "Error during Lua shutdown:" << e.what();
    }
#endif
}

qtplugin::expected<QJsonObject, PluginError> LuaExecutionEnvironment::execute_code(
    const QString& code, const QJsonObject& context) {
#ifndef QTFORGE_LUA_BINDINGS
    Q_UNUSED(code)
    Q_UNUSED(context)
    return make_error<QJsonObject>(PluginErrorCode::NotSupported,
                                  "Lua bindings not compiled in this build");
#else
    QMutexLocker locker(&m_mutex);

    if (!m_initialized) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState,
                                      "Lua environment not initialized");
    }

    try {
        // Set context variables
        if (!context.isEmpty()) {
            sol::table ctx_table = m_lua_state->create_table();
            for (auto it = context.begin(); it != context.end(); ++it) {
                ctx_table[it.key().toStdString()] = json_to_lua(it.value());
            }
            (*m_lua_state)["context"] = ctx_table;
        }

        // Execute the code
        sol::protected_function_result result = m_lua_state->safe_script(code.toStdString());

        if (!result.valid()) {
            sol::error err = result;
            qCWarning(luaBridgeLog) << "Lua execution error:" << err.what();
            return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed,
                                          std::string("Lua execution error: ") + err.what());
        }

        // Convert result to JSON
        QJsonObject response;
        if (result.get_type() != sol::type::nil) {
            response["result"] = lua_to_json(result);
        }
        response["success"] = true;

        return response;

    } catch (const sol::error& e) {
        qCWarning(luaBridgeLog) << "Lua execution error:" << e.what();
        return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed,
                                      std::string("Lua execution error: ") + e.what());
    } catch (const std::exception& e) {
        qCWarning(luaBridgeLog) << "Lua execution error:" << e.what();
        return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed,
                                      std::string("Lua execution error: ") + e.what());
    }
#endif
}

qtplugin::expected<QString, PluginError> LuaExecutionEnvironment::load_plugin_script(
    const QString& plugin_path) {
#ifndef QTFORGE_LUA_BINDINGS
    Q_UNUSED(plugin_path)
    return make_error<QString>(PluginErrorCode::NotSupported,
                              "Lua bindings not compiled in this build");
#else
    QMutexLocker locker(&m_mutex);

    if (!m_initialized) {
        return make_error<QString>(PluginErrorCode::InvalidState,
                                  "Lua environment not initialized");
    }

    QFileInfo file_info(plugin_path);
    if (!file_info.exists() || !file_info.isReadable()) {
        return make_error<QString>(PluginErrorCode::FileNotFound,
                                  "Plugin file not found or not readable: " + plugin_path.toStdString());
    }

    try {
        // Generate unique plugin ID
        QString plugin_id = QUuid::createUuid().toString(QUuid::WithoutBraces);

        // Load and execute the plugin script
        sol::protected_function_result result = m_lua_state->safe_script_file(plugin_path.toStdString());

        if (!result.valid()) {
            sol::error err = result;
            qCWarning(luaBridgeLog) << "Failed to load Lua plugin:" << err.what();
            return make_error<QString>(PluginErrorCode::LoadFailed,
                                      std::string("Failed to load Lua plugin: ") + err.what());
        }

        // Store plugin table if it exists
        sol::table plugin_table = (*m_lua_state)["plugin"];
        if (plugin_table.valid()) {
            m_loaded_plugins[plugin_id] = plugin_table;
        }

        qCDebug(luaBridgeLog) << "Loaded Lua plugin:" << plugin_path << "with ID:" << plugin_id;
        return plugin_id;

    } catch (const sol::error& e) {
        qCWarning(luaBridgeLog) << "Failed to load Lua plugin:" << e.what();
        return make_error<QString>(PluginErrorCode::LoadFailed,
                                  std::string("Failed to load Lua plugin: ") + e.what());
    } catch (const std::exception& e) {
        qCWarning(luaBridgeLog) << "Failed to load Lua plugin:" << e.what();
        return make_error<QString>(PluginErrorCode::LoadFailed,
                                  std::string("Failed to load Lua plugin: ") + e.what());
    }
#endif
}

void LuaExecutionEnvironment::setup_lua_environment() {
#ifdef QTFORGE_LUA_BINDINGS
    // Open standard Lua libraries
    m_lua_state->open_libraries(
        sol::lib::base,
        sol::lib::package,
        sol::lib::coroutine,
        sol::lib::string,
        sol::lib::os,
        sol::lib::math,
        sol::lib::table,
        sol::lib::debug,
        sol::lib::bit32,
        sol::lib::io,
        sol::lib::utf8
    );

    qCDebug(luaBridgeLog) << "Lua standard libraries loaded";
#endif
}

void LuaExecutionEnvironment::setup_sandbox() {
#ifdef QTFORGE_LUA_BINDINGS
    // Implement sandboxing by restricting dangerous functions
    // This is a basic implementation - production code should be more comprehensive

    // Disable dangerous functions
    (*m_lua_state)["os"]["execute"] = sol::nil;
    (*m_lua_state)["os"]["exit"] = sol::nil;
    (*m_lua_state)["os"]["remove"] = sol::nil;
    (*m_lua_state)["os"]["rename"] = sol::nil;
    (*m_lua_state)["os"]["tmpname"] = sol::nil;

    // Disable file I/O in sandbox mode
    (*m_lua_state)["io"]["open"] = sol::nil;
    (*m_lua_state)["io"]["popen"] = sol::nil;
    (*m_lua_state)["io"]["tmpfile"] = sol::nil;

    // Disable loading external modules
    (*m_lua_state)["require"] = sol::nil;
    (*m_lua_state)["dofile"] = sol::nil;
    (*m_lua_state)["loadfile"] = sol::nil;

    qCDebug(luaBridgeLog) << "Lua sandbox configured";
#endif
}

void LuaExecutionEnvironment::register_qt_bindings() {
#ifdef QTFORGE_LUA_BINDINGS
    // Register basic Qt types and functions
    // This will be expanded in the next implementation phase

    // Register logging function
    (*m_lua_state)["qtforge_log"] = [](const std::string& message) {
        qCDebug(luaBridgeLog) << "Lua:" << QString::fromStdString(message);
    };

    qCDebug(luaBridgeLog) << "Qt bindings registered";
#endif
}

QJsonValue LuaExecutionEnvironment::lua_to_json(const sol::object& obj) {
#ifdef QTFORGE_LUA_BINDINGS
    switch (obj.get_type()) {
        case sol::type::nil:
            return QJsonValue::Null;
        case sol::type::boolean:
            return obj.as<bool>();
        case sol::type::number:
            return obj.as<double>();
        case sol::type::string:
            return QString::fromStdString(obj.as<std::string>());
        case sol::type::table: {
            sol::table table = obj.as<sol::table>();
            QJsonObject json_obj;
            for (const auto& pair : table) {
                QString key = lua_to_json(pair.first).toString();
                json_obj[key] = lua_to_json(pair.second);
            }
            return json_obj;
        }
        default:
            return QString::fromStdString(obj.as<std::string>());
    }
#else
    Q_UNUSED(obj)
    return QJsonValue::Null;
#endif
}

sol::object LuaExecutionEnvironment::json_to_lua(const QJsonValue& value) {
#ifdef QTFORGE_LUA_BINDINGS
    switch (value.type()) {
        case QJsonValue::Null:
            return sol::nil;
        case QJsonValue::Bool:
            return sol::make_object(*m_lua_state, value.toBool());
        case QJsonValue::Double:
            return sol::make_object(*m_lua_state, value.toDouble());
        case QJsonValue::String:
            return sol::make_object(*m_lua_state, value.toString().toStdString());
        case QJsonValue::Array: {
            sol::table table = m_lua_state->create_table();
            QJsonArray array = value.toArray();
            for (int i = 0; i < array.size(); ++i) {
                table[i + 1] = json_to_lua(array[i]); // Lua arrays are 1-indexed
            }
            return table;
        }
        case QJsonValue::Object: {
            sol::table table = m_lua_state->create_table();
            QJsonObject obj = value.toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                table[it.key().toStdString()] = json_to_lua(it.value());
            }
            return table;
        }
        default:
            return sol::nil;
    }
#else
    Q_UNUSED(value)
    return sol::object();
#endif
}

// === LuaPluginBridge Implementation ===

LuaPluginBridge::LuaPluginBridge(QObject* parent)
    : QObject(parent), m_environment(std::make_unique<LuaExecutionEnvironment>()) {
    setup_environment();
}

LuaPluginBridge::~LuaPluginBridge() {
    shutdown();
}

std::string_view LuaPluginBridge::name() const noexcept {
    return "LuaPluginBridge";
}

std::string_view LuaPluginBridge::description() const noexcept {
    return "Bridge for executing Lua-based plugins";
}

Version LuaPluginBridge::version() const noexcept {
    return Version{3, 2, 0};
}

std::string_view LuaPluginBridge::author() const noexcept {
    return "QtForge Development Team";
}

std::string LuaPluginBridge::id() const noexcept {
    QMutexLocker locker(&m_mutex);
    return m_plugin_id.isEmpty() ? "qtplugin.LuaPluginBridge" : m_plugin_id.toStdString();
}

qtplugin::expected<void, PluginError> LuaPluginBridge::initialize() {
    QMutexLocker locker(&m_mutex);

    if (m_state != PluginState::Unloaded) {
        return make_error<void>(PluginErrorCode::InvalidState, "Plugin already initialized");
    }

    m_state = PluginState::Loading;

    auto init_result = m_environment->initialize();
    if (!init_result) {
        m_state = PluginState::Error;
        return init_result;
    }

    m_state = PluginState::Running;
    qCDebug(luaBridgeLog) << "LuaPluginBridge initialized";
    return make_success();
}

void LuaPluginBridge::shutdown() noexcept {
    QMutexLocker locker(&m_mutex);

    if (m_state == PluginState::Unloaded) {
        return;
    }

    try {
        m_environment->shutdown();
        m_state = PluginState::Unloaded;
        qCDebug(luaBridgeLog) << "LuaPluginBridge shut down";
    } catch (const std::exception& e) {
        qCWarning(luaBridgeLog) << "Error during LuaPluginBridge shutdown:" << e.what();
        m_state = PluginState::Error;
    }
}

PluginState LuaPluginBridge::state() const noexcept {
    QMutexLocker locker(&m_mutex);
    return m_state;
}

PluginCapabilities LuaPluginBridge::capabilities() const noexcept {
    return static_cast<PluginCapabilities>(PluginCapability::Scripting) |
           static_cast<PluginCapabilities>(PluginCapability::DataProcessing) |
           static_cast<PluginCapabilities>(PluginCapability::Configuration);
}

qtplugin::expected<QJsonObject, PluginError> LuaPluginBridge::execute_command(
    std::string_view command, const QJsonObject& params) {

    QMutexLocker locker(&m_mutex);

    if (m_state != PluginState::Running) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState, "Plugin not running");
    }

    QString cmd = QString::fromUtf8(command.data(), command.size());

    if (cmd == "execute_lua") {
        QString code = params.value("code").toString();
        if (code.isEmpty()) {
            return make_error<QJsonObject>(PluginErrorCode::InvalidParameter, "Missing 'code' parameter");
        }
        return m_environment->execute_code(code, params.value("context").toObject());
    }
    else if (cmd == "load_script") {
        QString script_path = params.value("path").toString();
        if (script_path.isEmpty()) {
            return make_error<QJsonObject>(PluginErrorCode::InvalidParameter, "Missing 'path' parameter");
        }

        auto load_result = load_lua_plugin(script_path);
        if (!load_result) {
            return make_error<QJsonObject>(load_result.error().code, load_result.error().message);
        }

        QJsonObject response;
        response["success"] = true;
        response["plugin_id"] = m_plugin_id;
        return response;
    }
    else {
        return make_error<QJsonObject>(PluginErrorCode::CommandNotFound,
                                      "Unknown command: " + std::string(command));
    }
}

std::vector<std::string> LuaPluginBridge::available_commands() const {
    return {"execute_lua", "load_script", "status"};
}

qtplugin::expected<QVariant, PluginError> LuaPluginBridge::invoke_method(
    const QString& method_name, const QList<QVariant>& arguments) {

    Q_UNUSED(method_name)
    Q_UNUSED(arguments)
    return make_error<QVariant>(PluginErrorCode::NotImplemented, "Method invocation not yet implemented");
}

qtplugin::expected<QVariant, PluginError> LuaPluginBridge::get_property(
    const QString& property_name) {

    Q_UNUSED(property_name)
    return make_error<QVariant>(PluginErrorCode::NotImplemented, "Property access not yet implemented");
}

qtplugin::expected<void, PluginError> LuaPluginBridge::set_property(
    const QString& property_name, const QVariant& value) {

    Q_UNUSED(property_name)
    Q_UNUSED(value)
    return make_error<void>(PluginErrorCode::NotImplemented, "Property setting not yet implemented");
}

qtplugin::expected<QStringList, PluginError> LuaPluginBridge::list_methods() const {
    return QStringList{"execute_lua", "load_script"};
}

qtplugin::expected<QStringList, PluginError> LuaPluginBridge::list_properties() const {
    return QStringList{"plugin_id", "state", "sandbox_enabled"};
}

qtplugin::expected<void, PluginError> LuaPluginBridge::load_lua_plugin(const QString& plugin_path) {
    auto load_result = m_environment->load_plugin_script(plugin_path);
    if (!load_result) {
        return make_error<void>(load_result.error().code, load_result.error().message);
    }

    m_plugin_path = plugin_path;
    m_plugin_id = load_result.value();
    return make_success();
}

qtplugin::expected<QVariant, PluginError> LuaPluginBridge::execute_code(
    const QString& code, const QJsonObject& context) {

    auto result = m_environment->execute_code(code, context);
    if (!result) {
        return make_error<QVariant>(result.error().code, result.error().message);
    }

    return QVariant::fromValue(result.value());
}

void LuaPluginBridge::handle_lua_error(const QString& error) {
    qCWarning(luaBridgeLog) << "Lua error in plugin" << m_plugin_id << ":" << error;
    m_state = PluginState::Error;
}

void LuaPluginBridge::setup_environment() {
    // Connect error handling
    connect(this, &LuaPluginBridge::destroyed, [this]() {
        shutdown();
    });
}

QString LuaPluginBridge::generate_plugin_id() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

} // namespace qtplugin

#include "lua_plugin_bridge.moc"
