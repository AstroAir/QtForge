/**
 * @file lua_plugin_bridge.hpp
 * @brief Lua plugin bridge for executing Lua-based plugins
 * @version 3.2.0
 * @author QtPlugin Development Team
 *
 * This file provides a bridge between the QtForge plugin system and Lua
 * plugins, allowing Lua scripts to be loaded and executed as plugins with full
 * integration into the QtForge ecosystem using sol2 for C++/Lua
 * interoperability.
 *
 * @section features Key Features
 * - Sandboxed Lua execution environment
 * - Full QtForge plugin interface implementation
 * - Dynamic method and property access
 * - Thread-safe execution
 * - Automatic type conversion between Lua and Qt types
 *
 * @section usage Basic Usage
 * @code{.cpp}
 * auto bridge = std::make_unique<LuaPluginBridge>();
 * auto result = bridge->load_lua_plugin("path/to/plugin.lua");
 * if (result) {
 *     auto command_result = bridge->execute_command("my_command", params);
 * }
 * @endcode
 *
 * @see IDynamicPlugin
 * @see IPlugin
 * @since QtForge 3.0.0
 */

#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariant>
#include <QVariantList>

#include <functional>
#include <memory>

#include "qtplugin/interfaces/core/dynamic_plugin_interface.hpp"

// Forward declare sol types to avoid including sol2 in header
namespace sol {
class object;
class state;
class table;
class function;
class protected_function_result;
}  // namespace sol

namespace qtplugin {

/**
 * @brief Lua plugin execution environment
 *
 * Provides a secure, sandboxed environment for executing Lua code within
 * the QtForge plugin system. This class manages the Lua state, handles
 * script loading, and provides type conversion between Lua and Qt types.
 *
 * The environment supports:
 * - Sandboxed execution to prevent malicious code
 * - Plugin script loading and management
 * - Function calls with parameter passing
 * - Automatic type conversion
 * - Thread-safe operations
 *
 * @note This class is thread-safe and can be used from multiple threads
 * @warning Sandbox mode should be enabled for untrusted Lua scripts
 * @since QtForge 3.0.0
 */
class LuaExecutionEnvironment {
public:
    /**
     * @brief Constructs a new Lua execution environment
     *
     * Creates an uninitialized Lua execution environment. Call initialize()
     * before using any other methods.
     */
    explicit LuaExecutionEnvironment();

    /**
     * @brief Destructor
     *
     * Automatically shuts down the Lua environment and cleans up resources.
     */
    ~LuaExecutionEnvironment();

    /**
     * @brief Initialize the Lua environment
     *
     * Sets up the Lua state, registers Qt bindings, and configures the
     * sandbox if enabled. This must be called before any other operations.
     *
     * @return Success or PluginError on failure
     * @retval PluginError::InitializationFailed If Lua state creation fails
     * @retval PluginError::ConfigurationError If sandbox setup fails
     *
     * @note This method is thread-safe
     * @see shutdown()
     */
    qtplugin::expected<void, PluginError> initialize();

    /**
     * @brief Shutdown the Lua environment
     *
     * Cleans up the Lua state and releases all resources. After calling
     * this method, the environment must be reinitialized before use.
     *
     * @note This method is thread-safe and can be called multiple times
     * @see initialize()
     */
    void shutdown();

    /**
     * @brief Execute Lua code
     *
     * Executes the provided Lua code string within the current environment.
     * The code has access to all registered Qt bindings and loaded plugins.
     *
     * @param code Lua code string to execute
     * @param context Optional execution context as JSON object
     * @return Execution result as JSON object or PluginError
     * @retval PluginError::ExecutionFailed If Lua code execution fails
     * @retval PluginError::InvalidState If environment is not initialized
     *
     * @note This method is thread-safe
     * @warning Code execution is subject to sandbox restrictions if enabled
     *
     * @code{.cpp}
     * auto result = env->execute_code("return 2 + 2");
     * if (result) {
     *     qDebug() << "Result:" << result.value();
     * }
     * @endcode
     */
    qtplugin::expected<QJsonObject, PluginError> execute_code(
        const QString& code, const QJsonObject& context = {});

    /**
     * @brief Load a Lua plugin script
     *
     * Loads and initializes a Lua plugin script from the specified file path.
     * The plugin is registered with a unique identifier for later access.
     *
     * @param plugin_path Absolute or relative path to the Lua plugin file
     * @return Unique plugin identifier or PluginError
     * @retval PluginError::FileNotFound If plugin file doesn't exist
     * @retval PluginError::LoadFailed If script compilation fails
     * @retval PluginError::InvalidPlugin If plugin doesn't meet requirements
     *
     * @note Plugin files should implement the standard plugin interface
     * @note This method is thread-safe
     *
     * @code{.cpp}
     * auto plugin_id = env->load_plugin_script("plugins/my_plugin.lua");
     * if (plugin_id) {
     *     qDebug() << "Loaded plugin:" << plugin_id.value();
     * }
     * @endcode
     */
    qtplugin::expected<QString, PluginError> load_plugin_script(
        const QString& plugin_path);

    /**
     * @brief Call a function in a loaded plugin
     *
     * Invokes a specific function within a previously loaded plugin,
     * passing the provided parameters and returning the result.
     *
     * @param plugin_id Unique identifier of the loaded plugin
     * @param function_name Name of the function to call
     * @param parameters Function parameters as JSON array
     * @return Function result as JSON object or PluginError
     * @retval PluginError::PluginNotFound If plugin_id is invalid
     * @retval PluginError::FunctionNotFound If function doesn't exist
     * @retval PluginError::ExecutionFailed If function execution fails
     *
     * @note Parameters are automatically converted from JSON to Lua types
     * @note Return values are automatically converted from Lua to JSON
     * @note This method is thread-safe
     *
     * @code{.cpp}
     * QJsonArray params;
     * params.append("Hello");
     * params.append(42);
     * auto result = env->call_plugin_function("my_plugin", "process", params);
     * @endcode
     */
    qtplugin::expected<QJsonObject, PluginError> call_plugin_function(
        const QString& plugin_id, const QString& function_name,
        const QJsonArray& parameters = {});

    /**
     * @brief Check if environment is initialized
     *
     * @return true if the environment has been successfully initialized
     * @return false if the environment is not ready for use
     *
     * @note This method is thread-safe
     */
    bool is_initialized() const { return m_initialized; }

    /**
     * @brief Get Lua state (for advanced usage)
     *
     * Provides direct access to the underlying sol2 Lua state for
     * advanced operations not covered by the standard interface.
     *
     * @return Pointer to the sol::state or nullptr if not initialized
     * @warning Direct state access bypasses safety checks and sandboxing
     * @warning Use with caution and ensure thread safety manually
     * @note This is intended for advanced users only
     */
    sol::state* lua_state() const { return m_lua_state.get(); }

    /**
     * @brief Set sandbox mode
     *
     * Enables or disables the Lua sandbox, which restricts access to
     * potentially dangerous functions and modules.
     *
     * @param enabled true to enable sandbox, false to disable
     *
     * @note Changes take effect on next initialize() call
     * @note Sandbox should be enabled for untrusted scripts
     * @warning Disabling sandbox allows full system access to Lua scripts
     */
    void set_sandbox_enabled(bool enabled) { m_sandbox_enabled = enabled; }

    /**
     * @brief Check if sandbox is enabled
     *
     * @return true if sandbox mode is enabled
     * @return false if sandbox mode is disabled
     */
    bool is_sandbox_enabled() const { return m_sandbox_enabled; }

private:
    std::unique_ptr<sol::state> m_lua_state;  ///< Sol2 Lua state wrapper
    QMutex m_mutex;                           ///< Thread safety mutex
    bool m_initialized = false;               ///< Initialization state flag
    bool m_sandbox_enabled = true;            ///< Sandbox mode flag
    std::map<QString, sol::table> m_loaded_plugins;  ///< Loaded plugin registry

    /**
     * @brief Set up the basic Lua environment
     *
     * Configures the Lua state with standard libraries and basic settings.
     */
    void setup_lua_environment();

    /**
     * @brief Set up sandbox restrictions
     *
     * Removes or restricts access to dangerous Lua functions and modules.
     */
    void setup_sandbox();

    /**
     * @brief Register Qt type bindings
     *
     * Makes Qt types and functions available to Lua scripts.
     */
    void register_qt_bindings();

    /**
     * @brief Convert Lua object to JSON value
     *
     * @param obj Lua object to convert
     * @return Equivalent QJsonValue
     */
    QJsonValue lua_to_json(const sol::object& obj);

    /**
     * @brief Convert JSON value to Lua object
     *
     * @param value JSON value to convert
     * @return Equivalent sol::object
     */
    sol::object json_to_lua(const QJsonValue& value);
};

/**
 * @brief Lua plugin bridge
 *
 * Provides a bridge between QtForge and Lua plugins, implementing the
 * IDynamicPlugin interface to allow Lua scripts to function as full plugins
 * within the QtForge ecosystem.
 *
 * This class enables:
 * - Loading Lua scripts as QtForge plugins
 * - Dynamic method and property access
 * - Command execution with parameter passing
 * - Full plugin lifecycle management
 * - Thread-safe operations
 * - Automatic type conversion
 *
 * @section lua_plugin_structure Lua Plugin Structure
 * Lua plugins should implement the following structure:
 * @code{.lua}
 * local plugin = {}
 *
 * function plugin.initialize()
 *     -- Plugin initialization code
 *     return true
 * end
 *
 * function plugin.shutdown()
 *     -- Plugin cleanup code
 * end
 *
 * function plugin.execute_command(command, params)
 *     -- Command execution logic
 *     return result
 * end
 *
 * return plugin
 * @endcode
 *
 * @note This class is thread-safe
 * @see IDynamicPlugin
 * @see IPlugin
 * @see LuaExecutionEnvironment
 * @since QtForge 3.0.0
 */
class LuaPluginBridge : public QObject, public IDynamicPlugin {
    Q_OBJECT
    Q_INTERFACES(qtplugin::IDynamicPlugin qtplugin::IPlugin)

public:
    /**
     * @brief Constructs a new Lua plugin bridge
     *
     * Creates an uninitialized Lua plugin bridge. Use load_lua_plugin()
     * to load a specific Lua plugin script.
     *
     * @param parent Parent QObject for memory management
     */
    explicit LuaPluginBridge(QObject* parent = nullptr);

    /**
     * @brief Destructor
     *
     * Automatically shuts down the plugin and cleans up resources.
     */
    ~LuaPluginBridge() override;

    // === IPlugin Implementation ===

    /**
     * @brief Get plugin name
     *
     * @return Plugin name as defined in the Lua script
     * @note Returns "Lua Plugin Bridge" if no script is loaded
     */
    std::string_view name() const noexcept override;

    /**
     * @brief Get plugin description
     *
     * @return Plugin description as defined in the Lua script
     * @note Returns default description if no script is loaded
     */
    std::string_view description() const noexcept override;

    /**
     * @brief Get plugin version
     *
     * @return Plugin version as defined in the Lua script
     * @note Returns default version if no script is loaded
     */
    Version version() const noexcept override;

    /**
     * @brief Get plugin author
     *
     * @return Plugin author as defined in the Lua script
     * @note Returns default author if no script is loaded
     */
    std::string_view author() const noexcept override;

    /**
     * @brief Get unique plugin identifier
     *
     * @return Unique plugin ID generated from script path and content
     */
    std::string id() const noexcept override;

    /**
     * @brief Initialize the plugin
     *
     * Sets up the Lua execution environment and calls the plugin's
     * initialize function if available.
     *
     * @return Success or PluginError on failure
     * @retval PluginError::InitializationFailed If environment setup fails
     * @retval PluginError::InvalidState If no plugin is loaded
     *
     * @note This method is thread-safe
     */
    qtplugin::expected<void, PluginError> initialize() override;

    /**
     * @brief Shutdown the plugin
     *
     * Calls the plugin's shutdown function if available and cleans up
     * the execution environment.
     *
     * @note This method is thread-safe and never throws
     */
    void shutdown() noexcept override;

    /**
     * @brief Get current plugin state
     *
     * @return Current PluginState (Unloaded, Loaded, Initialized, etc.)
     * @note This method is thread-safe
     */
    PluginState state() const noexcept override;

    /**
     * @brief Get plugin capabilities
     *
     * @return Set of PluginCapabilities supported by this plugin
     * @note Lua plugins support dynamic capabilities by default
     */
    PluginCapabilities capabilities() const noexcept override;

    /**
     * @brief Execute a plugin command
     *
     * Calls the plugin's execute_command function with the specified
     * command and parameters.
     *
     * @param command Command name to execute
     * @param params Command parameters as JSON object
     * @return Command result as JSON object or PluginError
     * @retval PluginError::CommandNotFound If command is not supported
     * @retval PluginError::ExecutionFailed If command execution fails
     * @retval PluginError::InvalidState If plugin is not initialized
     *
     * @note This method is thread-safe
     * @note Parameters are automatically converted to Lua types
     */
    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override;

    /**
     * @brief Get list of available commands
     *
     * @return Vector of command names supported by the plugin
     * @note This method is thread-safe
     */
    std::vector<std::string> available_commands() const override;

    // === IDynamicPlugin Implementation ===

    /**
     * @brief Invoke a method dynamically
     *
     * Calls a method in the loaded Lua plugin with the specified arguments.
     *
     * @param method_name Name of the method to invoke
     * @param arguments List of method arguments
     * @return Method result as QVariant or PluginError
     * @retval PluginError::MethodNotFound If method doesn't exist
     * @retval PluginError::ExecutionFailed If method execution fails
     *
     * @note This method is thread-safe
     * @note Arguments are automatically converted to Lua types
     */
    qtplugin::expected<QVariant, PluginError> invoke_method(
        const QString& method_name, const QVariantList& parameters = {},
        const QString& interface_id = {}) override;

    /**
     * @brief Get a property value
     *
     * Retrieves the value of a property from the loaded Lua plugin.
     *
     * @param property_name Name of the property to get
     * @return Property value as QVariant or PluginError
     * @retval PluginError::PropertyNotFound If property doesn't exist
     * @retval PluginError::AccessDenied If property is not readable
     *
     * @note This method is thread-safe
     */
    qtplugin::expected<QVariant, PluginError> get_property(
        const QString& property_name, const QString& interface_id = {}) override;

    /**
     * @brief Set a property value
     *
     * Sets the value of a property in the loaded Lua plugin.
     *
     * @param property_name Name of the property to set
     * @param value New property value
     * @return Success or PluginError
     * @retval PluginError::PropertyNotFound If property doesn't exist
     * @retval PluginError::AccessDenied If property is not writable
     * @retval PluginError::InvalidArgument If value type is incompatible
     *
     * @note This method is thread-safe
     * @note Value is automatically converted to appropriate Lua type
     */
    qtplugin::expected<void, PluginError> set_property(
        const QString& property_name, const QVariant& value,
        const QString& interface_id = {}) override;

    // === Multi-Language Support ===

    /**
     * @brief Get plugin type
     * @return Plugin type (Lua)
     */
    PluginType get_plugin_type() const override;

    /**
     * @brief Get execution context
     * @return Plugin execution context
     */
    PluginExecutionContext get_execution_context() const override;

    /**
     * @brief Execute code in plugin's runtime environment
     * @param code Code to execute
     * @param context Execution context
     * @return Execution result or error
     */
    qtplugin::expected<QVariant, PluginError> execute_code(
        const QString& code, const QJsonObject& context = {}) override;

    // === Interface Discovery ===

    /**
     * @brief Get supported interface descriptors
     * @return Vector of interface descriptors
     */
    std::vector<InterfaceDescriptor> get_interface_descriptors() const override;

    /**
     * @brief Check if plugin supports a specific interface
     * @param interface_id Interface identifier
     * @param min_version Minimum required version
     * @return True if interface is supported
     */
    bool supports_interface(
        const QString& interface_id,
        const Version& min_version = Version{}) const override;

    /**
     * @brief Get interface descriptor by ID
     * @param interface_id Interface identifier
     * @return Interface descriptor or nullopt if not found
     */
    std::optional<InterfaceDescriptor> get_interface_descriptor(
        const QString& interface_id) const override;

    // === Runtime Adaptation ===

    /**
     * @brief Adapt to a specific interface version
     * @param interface_id Interface identifier
     * @param target_version Target version to adapt to
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> adapt_to_interface(
        const QString& interface_id, const Version& target_version) override;

    /**
     * @brief Negotiate capabilities with another plugin
     * @param other_plugin_id Other plugin identifier
     * @param requested_capabilities Requested capabilities
     * @return Negotiated capabilities or error
     */
    qtplugin::expected<std::vector<InterfaceCapability>, PluginError>
    negotiate_capabilities(
        const QString& other_plugin_id,
        const std::vector<InterfaceCapability>& requested_capabilities) override;

    /**
     * @brief List available methods
     *
     * @return List of method names available in the plugin or PluginError
     * @retval PluginError::InvalidState If no plugin is loaded
     *
     * @note This method is thread-safe
     */
    qtplugin::expected<QStringList, PluginError> list_methods() const;

    /**
     * @brief List available properties
     *
     * @return List of property names available in the plugin or PluginError
     * @retval PluginError::InvalidState If no plugin is loaded
     *
     * @note This method is thread-safe
     */
    qtplugin::expected<QStringList, PluginError> list_properties() const;

    // IDynamicPlugin interface implementation
    std::vector<QString> get_available_methods(
        const QString& interface_id = {}) const override;
    std::vector<QString> get_available_properties(
        const QString& interface_id = {}) const override;

    /**
     * @brief Get method signature
     * @param method_name Method name
     * @param interface_id Interface identifier
     * @return Method signature as JSON schema
     */
    std::optional<QJsonObject> get_method_signature(
        const QString& method_name, const QString& interface_id = {}) const override;

    // === Event System ===

    /**
     * @brief Subscribe to events from another plugin
     * @param source_plugin_id Source plugin identifier
     * @param event_types Event types to subscribe to
     * @param callback Event callback function
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> subscribe_to_events(
        const QString& source_plugin_id,
        const std::vector<QString>& event_types,
        std::function<void(const QString&, const QJsonObject&)> callback) override;

    /**
     * @brief Unsubscribe from events
     * @param source_plugin_id Source plugin identifier
     * @param event_types Event types to unsubscribe from
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> unsubscribe_from_events(
        const QString& source_plugin_id,
        const std::vector<QString>& event_types = {}) override;

    /**
     * @brief Emit an event
     * @param event_type Event type
     * @param event_data Event data
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> emit_event(
        const QString& event_type, const QJsonObject& event_data) override;

    // === Lua-specific Methods ===

    /**
     * @brief Load Lua plugin from file
     *
     * Loads a Lua plugin script from the specified file path and prepares
     * it for execution within the QtForge plugin system.
     *
     * @param plugin_path Absolute or relative path to the Lua plugin file
     * @return Success or PluginError
     * @retval PluginError::FileNotFound If plugin file doesn't exist
     * @retval PluginError::LoadFailed If script compilation fails
     * @retval PluginError::InvalidPlugin If plugin structure is invalid
     *
     * @note This method is thread-safe
     * @note Previous plugin will be unloaded if one was already loaded
     *
     * @code{.cpp}
     * auto bridge = std::make_unique<LuaPluginBridge>();
     * auto result = bridge->load_lua_plugin("plugins/my_plugin.lua");
     * if (result) {
     *     bridge->initialize();
     * }
     * @endcode
     */
    qtplugin::expected<void, PluginError> load_lua_plugin(
        const QString& plugin_path);

    /**
     * @brief Execute Lua code directly
     *
     * Executes arbitrary Lua code within the plugin's execution environment.
     * This is useful for debugging or dynamic plugin modification.
     *
     * @param code Lua code string to execute
     * @param context Optional execution context as JSON object
     * @return Execution result as QVariant or PluginError
     * @retval PluginError::ExecutionFailed If code execution fails
     * @retval PluginError::InvalidState If environment is not ready
     *
     * @note This method is thread-safe
     * @warning Direct code execution bypasses plugin structure validation
     * @warning Use with caution in production environments
     *
     * @code{.cpp}
     * auto result = bridge->execute_code("return plugin.get_status()");
     * if (result) {
     *     qDebug() << "Status:" << result.value();
     * }
     * @endcode
     */

    /**
     * @brief Get Lua execution environment
     *
     * Provides access to the underlying Lua execution environment for
     * advanced operations.
     *
     * @return Pointer to LuaExecutionEnvironment or nullptr if not available
     * @note Use with caution - direct environment access bypasses safety checks
     */
    LuaExecutionEnvironment* execution_environment() const {
        return m_environment.get();
    }

private slots:
    /**
     * @brief Handle Lua execution errors
     *
     * Processes and logs errors that occur during Lua script execution.
     *
     * @param error Error message from Lua environment
     */
    void handle_lua_error(const QString& error);

private:
    std::unique_ptr<LuaExecutionEnvironment>
        m_environment;      ///< Lua execution environment
    QString m_plugin_path;  ///< Path to loaded plugin file
    QString m_plugin_id;    ///< Unique plugin identifier
    PluginState m_state = PluginState::Unloaded;  ///< Current plugin state
    mutable QMutex m_mutex;                       ///< Thread safety mutex

    /**
     * @brief Set up the execution environment
     *
     * Initializes the Lua execution environment with proper configuration.
     */
    void setup_environment();

    /**
     * @brief Generate unique plugin identifier
     *
     * Creates a unique ID based on plugin path and content hash.
     *
     * @return Generated plugin identifier
     */
    QString generate_plugin_id() const;
};

}  // namespace qtplugin

Q_DECLARE_INTERFACE(qtplugin::LuaPluginBridge, "qtplugin.LuaPluginBridge/3.2")
