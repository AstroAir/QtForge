/**
 * @file lua_plugin_bridge.hpp
 * @brief Lua plugin bridge for executing Lua-based plugins
 * @version 3.2.0
 * @author QtPlugin Development Team
 *
 * This file provides a bridge between the QtForge plugin system and Lua plugins,
 * allowing Lua scripts to be loaded and executed as plugins with full integration
 * into the QtForge ecosystem using sol2 for C++/Lua interoperability.
 */

#pragma once

#include "../core/dynamic_plugin_interface.hpp"
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QTimer>
#include <QMutex>
#include <memory>
#include <functional>

// Forward declare sol types to avoid including sol2 in header
namespace sol {
    class state;
    class table;
    class function;
    class protected_function_result;
}

namespace qtplugin {

/**
 * @brief Lua plugin execution environment
 */
class LuaExecutionEnvironment {
public:
    explicit LuaExecutionEnvironment();
    ~LuaExecutionEnvironment();

    /**
     * @brief Initialize the Lua environment
     */
    qtplugin::expected<void, PluginError> initialize();

    /**
     * @brief Shutdown the Lua environment
     */
    void shutdown();

    /**
     * @brief Execute Lua code
     * @param code Lua code to execute
     * @param context Execution context
     * @return Execution result
     */
    qtplugin::expected<QJsonObject, PluginError> execute_code(
        const QString& code, const QJsonObject& context = {});

    /**
     * @brief Load a Lua plugin script
     * @param plugin_path Path to the Lua plugin file
     * @return Plugin identifier or error
     */
    qtplugin::expected<QString, PluginError> load_plugin_script(
        const QString& plugin_path);

    /**
     * @brief Call a function in a loaded plugin
     * @param plugin_id Plugin identifier
     * @param function_name Function name
     * @param parameters Function parameters
     * @return Function result
     */
    qtplugin::expected<QJsonObject, PluginError> call_plugin_function(
        const QString& plugin_id, const QString& function_name,
        const QJsonArray& parameters = {});

    /**
     * @brief Check if environment is initialized
     */
    bool is_initialized() const { return m_initialized; }

    /**
     * @brief Get Lua state (for advanced usage)
     */
    sol::state* lua_state() const { return m_lua_state.get(); }

    /**
     * @brief Set sandbox mode
     * @param enabled Enable/disable sandboxing
     */
    void set_sandbox_enabled(bool enabled) { m_sandbox_enabled = enabled; }

    /**
     * @brief Check if sandbox is enabled
     */
    bool is_sandbox_enabled() const { return m_sandbox_enabled; }

private:
    std::unique_ptr<sol::state> m_lua_state;
    QMutex m_mutex;
    bool m_initialized = false;
    bool m_sandbox_enabled = true;
    std::map<QString, sol::table> m_loaded_plugins;

    void setup_lua_environment();
    void setup_sandbox();
    void register_qt_bindings();
    QJsonValue lua_to_json(const sol::object& obj);
    sol::object json_to_lua(const QJsonValue& value);
};

/**
 * @brief Lua plugin bridge
 * 
 * Provides a bridge between QtForge and Lua plugins, implementing the
 * IDynamicPlugin interface to allow Lua scripts to function as full plugins.
 */
class LuaPluginBridge : public QObject, public IDynamicPlugin {
    Q_OBJECT
    Q_INTERFACES(qtplugin::IDynamicPlugin qtplugin::IPlugin)

public:
    explicit LuaPluginBridge(QObject* parent = nullptr);
    ~LuaPluginBridge() override;

    // === IPlugin Implementation ===
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    Version version() const noexcept override;
    std::string_view author() const noexcept override;
    std::string id() const noexcept override;

    qtplugin::expected<void, PluginError> initialize() override;
    void shutdown() noexcept override;
    PluginState state() const noexcept override;
    PluginCapabilities capabilities() const noexcept override;

    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override;
    std::vector<std::string> available_commands() const override;

    // === IDynamicPlugin Implementation ===
    qtplugin::expected<QVariant, PluginError> invoke_method(
        const QString& method_name, const QList<QVariant>& arguments = {}) override;

    qtplugin::expected<QVariant, PluginError> get_property(
        const QString& property_name) override;

    qtplugin::expected<void, PluginError> set_property(
        const QString& property_name, const QVariant& value) override;

    qtplugin::expected<QStringList, PluginError> list_methods() const override;
    qtplugin::expected<QStringList, PluginError> list_properties() const override;

    // === Lua-specific Methods ===

    /**
     * @brief Load Lua plugin from file
     * @param plugin_path Path to Lua plugin file
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> load_lua_plugin(const QString& plugin_path);

    /**
     * @brief Execute Lua code directly
     * @param code Lua code to execute
     * @param context Execution context
     * @return Execution result
     */
    qtplugin::expected<QVariant, PluginError> execute_code(
        const QString& code, const QJsonObject& context = {});

    /**
     * @brief Get Lua execution environment
     */
    LuaExecutionEnvironment* execution_environment() const { return m_environment.get(); }

private slots:
    void handle_lua_error(const QString& error);

private:
    std::unique_ptr<LuaExecutionEnvironment> m_environment;
    QString m_plugin_path;
    QString m_plugin_id;
    PluginState m_state = PluginState::Unloaded;
    mutable QMutex m_mutex;

    void setup_environment();
    QString generate_plugin_id() const;
};

} // namespace qtplugin

Q_DECLARE_INTERFACE(qtplugin::LuaPluginBridge, "qtplugin.LuaPluginBridge/3.2")
