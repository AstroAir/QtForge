/**
 * @file python_plugin_bridge.hpp
 * @brief Python plugin bridge for executing Python-based plugins
 * @version 3.2.0
 * @author QtPlugin Development Team
 *
 * This file provides a bridge between the QtForge plugin system and Python
 * plugins, allowing Python scripts to be loaded and executed as plugins with
 * full integration into the QtForge ecosystem using subprocess communication
 * for secure and isolated execution.
 *
 * @section features Key Features
 * - Isolated Python process execution
 * - Full QtForge plugin interface implementation
 * - Dynamic method and property discovery
 * - Thread-safe inter-process communication
 * - Automatic type conversion between Python and Qt types
 * - Hot-reload support for development
 * - Event subscription and emission
 *
 * @section usage Basic Usage
 * @code{.cpp}
 * auto factory_result =
 * PythonPluginFactory::create_plugin("path/to/plugin.py"); if (factory_result)
 * { auto plugin = std::move(factory_result.value()); auto init_result =
 * plugin->initialize(); if (init_result) { auto command_result =
 * plugin->execute_command("my_command", params);
 *     }
 * }
 * @endcode
 *
 * @section python_plugin_structure Python Plugin Structure
 * Python plugins should inherit from the base plugin class:
 * @code{.python}
 * from qtforge import BasePlugin
 *
 * class MyPlugin(BasePlugin):
 *     def __init__(self):
 *         super().__init__()
 *         self.name = "My Python Plugin"
 *         self.version = "1.0.0"
 *
 *     def initialize(self):
 *         # Plugin initialization
 *         return True
 *
 *     def execute_command(self, command, params):
 *         # Command execution logic
 *         return {"result": "success"}
 * @endcode
 *
 * @see IDynamicPlugin
 * @see IPlugin
 * @see PythonExecutionEnvironment
 * @since QtForge 3.0.0
 */

#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>
#include <QVariant>
#include <QVariantList>
#include <QWaitCondition>

#include <functional>
#include <memory>

#include "../interfaces/core/dynamic_plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Python plugin execution environment
 *
 * Manages a separate Python process for executing Python-based plugins in
 * isolation from the main application. This provides security, stability,
 * and the ability to use different Python versions or environments.
 *
 * The environment supports:
 * - Isolated Python process execution
 * - JSON-based inter-process communication
 * - Plugin module loading and management
 * - Method calls with parameter passing
 * - Automatic type conversion
 * - Process lifecycle management
 *
 * @note This class is thread-safe and manages process communication
 * @warning Python process crashes are handled gracefully with error reporting
 * @since QtForge 3.0.0
 */
class PythonExecutionEnvironment {
public:
    /**
     * @brief Constructs a new Python execution environment
     *
     * Creates a Python execution environment that will use the specified
     * Python interpreter. The process is not started until initialize() is
     * called.
     *
     * @param python_path Path to the Python interpreter executable
     *                   Defaults to "python" (system PATH lookup)
     *
     * @note The Python interpreter must have the qtforge module available
     * @see initialize()
     */
    explicit PythonExecutionEnvironment(const QString& python_path = "python");

    /**
     * @brief Destructor
     *
     * Automatically shuts down the Python process and cleans up resources.
     * Any pending operations will be cancelled.
     */
    ~PythonExecutionEnvironment();

    /**
     * @brief Initialize the Python environment
     *
     * Starts the Python process and establishes communication channels.
     * The process will load the qtforge bridge module and prepare for
     * plugin execution.
     *
     * @return Success or PluginError on failure
     * @retval PluginError::InitializationFailed If Python process fails to
     * start
     * @retval PluginError::ConfigurationError If qtforge module is not
     * available
     * @retval PluginError::TimeoutError If process doesn't respond within
     * timeout
     *
     * @note This method is thread-safe
     * @note Process startup may take several seconds depending on Python
     * environment
     * @see shutdown()
     */
    qtplugin::expected<void, PluginError> initialize();

    /**
     * @brief Shutdown the Python environment
     *
     * Gracefully terminates the Python process and cleans up all resources.
     * Any pending operations will be cancelled and return errors.
     *
     * @note This method is thread-safe and can be called multiple times
     * @note Process termination is attempted gracefully first, then forcefully
     * if needed
     * @see initialize()
     */
    void shutdown();

    /**
     * @brief Execute Python code directly
     *
     * Executes arbitrary Python code within the plugin process environment.
     * This is useful for debugging, dynamic plugin modification, or one-off
     * operations.
     *
     * @param code Python code string to execute
     * @param context Optional execution context as JSON object (available as
     * 'context' variable)
     * @return Execution result as JSON object or PluginError
     * @retval PluginError::ExecutionFailed If Python code execution fails
     * @retval PluginError::InvalidState If environment is not running
     * @retval PluginError::TimeoutError If execution exceeds timeout
     *
     * @note This method is thread-safe
     * @note Code has access to all loaded plugin modules and qtforge APIs
     * @warning Direct code execution bypasses plugin structure validation
     *
     * @code{.cpp}
     * QJsonObject context;
     * context["debug"] = true;
     * auto result = env->execute_code("import sys; return sys.version",
     * context); if (result) { qDebug() << "Python version:" << result.value();
     * }
     * @endcode
     */
    qtplugin::expected<QJsonObject, PluginError> execute_code(
        const QString& code, const QJsonObject& context = {});

    /**
     * @brief Load a Python plugin module
     *
     * Loads a Python plugin module from the specified file path and
     * instantiates the plugin class. The plugin becomes available for
     * method calls and property access.
     *
     * @param plugin_path Absolute or relative path to the Python plugin file
     * @param plugin_class Name of the plugin class to instantiate
     * @return Unique plugin identifier or PluginError
     * @retval PluginError::FileNotFound If plugin file doesn't exist
     * @retval PluginError::LoadFailed If module import fails
     * @retval PluginError::InvalidPlugin If plugin class is not found or
     * invalid
     * @retval PluginError::InitializationFailed If plugin instantiation fails
     *
     * @note Plugin files should be valid Python modules with the specified
     * class
     * @note The plugin class should inherit from qtforge.BasePlugin
     * @note This method is thread-safe
     *
     * @code{.cpp}
     * auto plugin_id = env->load_plugin_module("plugins/my_plugin.py",
     * "MyPlugin"); if (plugin_id) { qDebug() << "Loaded plugin:" <<
     * plugin_id.value();
     * }
     * @endcode
     */
    qtplugin::expected<QString, PluginError> load_plugin_module(
        const QString& plugin_path, const QString& plugin_class);

    /**
     * @brief Call a method on a loaded plugin
     *
     * Invokes a specific method on a previously loaded plugin instance,
     * passing the provided parameters and returning the result.
     *
     * @param plugin_id Unique identifier of the loaded plugin
     * @param method_name Name of the method to call
     * @param parameters Method parameters as JSON array
     * @return Method result as JSON object or PluginError
     * @retval PluginError::PluginNotFound If plugin_id is invalid
     * @retval PluginError::MethodNotFound If method doesn't exist
     * @retval PluginError::ExecutionFailed If method execution fails
     * @retval PluginError::InvalidArgument If parameters are incompatible
     * @retval PluginError::TimeoutError If method execution exceeds timeout
     *
     * @note Parameters are automatically converted from JSON to Python types
     * @note Return values are automatically converted from Python to JSON
     * @note This method is thread-safe
     *
     * @code{.cpp}
     * QJsonArray params;
     * params.append("Hello");
     * params.append(42);
     * auto result = env->call_plugin_method("my_plugin", "process_data",
     * params); if (result) { qDebug() << "Result:" << result.value();
     * }
     * @endcode
     */
    qtplugin::expected<QJsonObject, PluginError> call_plugin_method(
        const QString& plugin_id, const QString& method_name,
        const QJsonArray& parameters = {});

    /**
     * @brief Get plugin information
     *
     * @param plugin_id Unique identifier of the loaded plugin
     * @return Plugin information including metadata, methods, and properties
     */
    qtplugin::expected<QJsonObject, PluginError> get_plugin_info(
        const QString& plugin_id);

    /**
     * @brief Get a property value from a loaded plugin
     *
     * @param plugin_id Unique identifier of the loaded plugin
     * @param property_name Name of the property to get
     * @return Property value as JSON object or PluginError
     */
    qtplugin::expected<QJsonObject, PluginError> get_plugin_property(
        const QString& plugin_id, const QString& property_name);

    /**
     * @brief Set a property value on a loaded plugin
     *
     * @param plugin_id Unique identifier of the loaded plugin
     * @param property_name Name of the property to set
     * @param value New property value
     * @return Success or PluginError
     */
    qtplugin::expected<QJsonObject, PluginError> set_plugin_property(
        const QString& plugin_id, const QString& property_name, const QJsonValue& value);

    /**
     * @brief Check if environment is running
     *
     * @return true if the Python process is running and responsive
     * @return false if the process is not started, crashed, or unresponsive
     *
     * @note This method is thread-safe
     * @note A running process doesn't guarantee it's responsive - use with
     * caution
     */
    bool is_running() const {
        return m_process && m_process->state() == QProcess::Running;
    }

    /**
     * @brief Get Python interpreter path
     *
     * @return Path to the Python interpreter being used
     * @note This is the path specified in the constructor
     */
    QString python_path() const { return m_python_path; }

    /**
     * @brief Set request timeout
     *
     * Sets the maximum time to wait for responses from the Python process.
     *
     * @param timeout_ms Timeout in milliseconds (default: 30000ms)
     * @note Longer timeouts may be needed for complex operations
     * @note This affects all future requests until changed
     */
    void set_request_timeout(int timeout_ms) { m_request_timeout = timeout_ms; }

    /**
     * @brief Get current request timeout
     *
     * @return Current timeout value in milliseconds
     */
    int request_timeout() const { return m_request_timeout; }

private:
    QString m_python_path;                ///< Path to Python interpreter
    std::unique_ptr<QProcess> m_process;  ///< Python process instance
    QMutex m_mutex;                       ///< Thread safety mutex
    QWaitCondition m_response_condition;  ///< Response waiting condition
    QJsonObject m_last_response;          ///< Last received response
    bool m_waiting_for_response = false;  ///< Response waiting flag
    int m_request_id = 0;                 ///< Request ID counter
    int m_request_timeout = 30000;        ///< Request timeout in ms
    QHash<int, QJsonObject> m_pending_responses;  ///< Pending responses by ID

    /**
     * @brief Set up the Python process
     *
     * Configures process parameters, environment, and communication channels.
     */
    void setup_process();

    /**
     * @brief Handle process output
     *
     * Processes JSON responses from the Python process.
     */
    void handle_process_output();

    /**
     * @brief Handle process errors
     *
     * Handles process crashes, communication errors, and other failures.
     */
    void handle_process_error();

    /**
     * @brief Send request to Python process
     *
     * Sends a JSON request and waits for the response with timeout.
     *
     * @param request JSON request object
     * @return Response JSON object or PluginError
     */
    qtplugin::expected<QJsonObject, PluginError> send_request(
        const QJsonObject& request);
};

/**
 * @brief Python plugin bridge implementation
 *
 * Provides a complete bridge between QtForge and Python plugins, implementing
 * the IDynamicPlugin interface to allow Python scripts to function as full
 * plugins within the QtForge ecosystem.
 *
 * This class enables:
 * - Loading Python scripts as QtForge plugins
 * - Dynamic method and property access
 * - Command execution with parameter passing
 * - Full plugin lifecycle management
 * - Hot-reload support for development
 * - Event subscription and emission
 * - Interface adaptation and capability negotiation
 * - Thread-safe operations
 * - Automatic type conversion
 *
 * @section lifecycle Plugin Lifecycle
 * 1. Construction with plugin path
 * 2. Metadata loading and validation
 * 3. Interface discovery and setup
 * 4. Initialization (starts Python process)
 * 5. Normal operation (method calls, commands, etc.)
 * 6. Shutdown (terminates Python process)
 *
 * @note This class is thread-safe
 * @see IDynamicPlugin
 * @see IPlugin
 * @see PythonExecutionEnvironment
 * @since QtForge 3.0.0
 */
class PythonPluginBridge : public QObject, public IDynamicPlugin {
    Q_OBJECT

public:
    /**
     * @brief Constructs a new Python plugin bridge
     *
     * Creates a Python plugin bridge for the specified plugin file.
     * The plugin metadata is loaded during construction, but the Python
     * process is not started until initialize() is called.
     *
     * @param plugin_path Absolute or relative path to the Python plugin file
     * @param parent Parent QObject for memory management
     *
     * @note Plugin metadata must be available (either in separate .json file or
     * embedded)
     * @see initialize()
     */
    explicit PythonPluginBridge(const QString& plugin_path,
                                QObject* parent = nullptr);

    /**
     * @brief Destructor
     *
     * Automatically shuts down the plugin and cleans up all resources.
     * The Python process will be terminated gracefully.
     */
    ~PythonPluginBridge() override;

    // === IPlugin Implementation ===

    /**
     * @brief Get plugin name
     *
     * @return Plugin name as defined in the plugin metadata
     * @note Returns "Unknown Python Plugin" if metadata is not available
     */
    std::string_view name() const noexcept override;

    /**
     * @brief Get plugin description
     *
     * @return Plugin description as defined in the plugin metadata
     * @note Returns default description if metadata is not available
     */
    std::string_view description() const noexcept override;

    /**
     * @brief Get plugin version
     *
     * @return Plugin version as defined in the plugin metadata
     * @note Returns Version{0,0,0} if metadata is not available
     */
    qtplugin::Version version() const noexcept override;

    /**
     * @brief Get plugin author
     *
     * @return Plugin author as defined in the plugin metadata
     * @note Returns "Unknown" if metadata is not available
     */
    std::string_view author() const noexcept override;

    /**
     * @brief Get unique plugin identifier
     *
     * @return Unique plugin ID generated from plugin path and metadata
     */
    std::string id() const noexcept override;

    /**
     * @brief Initialize the plugin
     *
     * Starts the Python execution environment, loads the plugin module,
     * and calls the plugin's initialize method if available.
     *
     * @return Success or PluginError on failure
     * @retval PluginError::InitializationFailed If Python environment setup
     * fails
     * @retval PluginError::LoadFailed If plugin module loading fails
     * @retval PluginError::InvalidState If plugin is already initialized
     *
     * @note This method is thread-safe
     * @note Initialization may take several seconds for complex plugins
     */
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;

    /**
     * @brief Shutdown the plugin
     *
     * Calls the plugin's shutdown method if available, terminates the Python
     * process, and cleans up all resources.
     *
     * @note This method is thread-safe and never throws
     * @note Shutdown is always attempted gracefully first
     */
    void shutdown() noexcept override;

    /**
     * @brief Get current plugin state
     *
     * @return Current PluginState (Unloaded, Loaded, Initialized, etc.)
     * @note This method is thread-safe
     */
    qtplugin::PluginState state() const noexcept override;

    /**
     * @brief Get plugin capabilities
     *
     * @return Set of PluginCapabilities supported by this plugin
     * @note Python plugins support dynamic capabilities by default
     * @note Capabilities are determined from plugin metadata and runtime
     * discovery
     */
    qtplugin::PluginCapabilities capabilities() const noexcept override;

    /**
     * @brief Configure the plugin
     *
     * Applies configuration settings to the plugin. The configuration
     * is validated against the plugin's schema if available.
     *
     * @param config Configuration object to apply
     * @return Success or PluginError
     * @retval PluginError::InvalidConfiguration If config doesn't match schema
     * @retval PluginError::InvalidState If plugin is not initialized
     *
     * @note Configuration is passed to the Python plugin's configure method
     * @note This method is thread-safe
     */
    qtplugin::expected<void, qtplugin::PluginError> configure(
        const QJsonObject& config) override;

    /**
     * @brief Get current configuration
     *
     * @return Current plugin configuration as JSON object
     * @note Returns empty object if no configuration has been applied
     */
    QJsonObject current_configuration() const override;

    /**
     * @brief Execute a plugin command
     *
     * Calls the plugin's execute_command method with the specified
     * command and parameters.
     *
     * @param command Command name to execute
     * @param params Command parameters as JSON object
     * @return Command result as JSON object or PluginError
     * @retval PluginError::CommandNotFound If command is not supported
     * @retval PluginError::ExecutionFailed If command execution fails
     * @retval PluginError::InvalidState If plugin is not initialized
     * @retval PluginError::InvalidArgument If parameters are invalid
     *
     * @note This method is thread-safe
     * @note Parameters are automatically converted to Python types
     */
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override;

    /**
     * @brief Get list of available commands
     *
     * @return Vector of command names supported by the plugin
     * @note Commands are discovered during plugin initialization
     * @note This method is thread-safe
     */
    std::vector<std::string> available_commands() const override;

    // === Advanced Plugin Features ===

    /**
     * @brief Hot-reload the plugin
     *
     * Reloads the Python plugin module without restarting the process.
     * This is useful for development and testing.
     *
     * @return Success or PluginError
     * @retval PluginError::ReloadFailed If module reload fails
     * @retval PluginError::InvalidState If plugin is not initialized
     *
     * @note Plugin state is preserved across reloads when possible
     * @note This method is thread-safe
     * @warning Hot-reload may cause temporary service interruption
     */
    qtplugin::expected<void, qtplugin::PluginError> hot_reload();

    /**
     * @brief Validate configuration against schema
     *
     * Checks if the provided configuration is valid according to the
     * plugin's configuration schema.
     *
     * @param config Configuration to validate
     * @return true if configuration is valid, false otherwise
     * @note Uses JSON Schema validation if schema is available
     * @note This method is thread-safe
     */
    bool validate_configuration(const QJsonObject& config) const override;

    /**
     * @brief Get configuration schema
     *
     * @return JSON Schema object describing valid configuration structure
     * @note Returns empty object if no schema is defined
     * @note Schema is loaded from plugin metadata or discovered at runtime
     */
    QJsonObject get_configuration_schema() const;

    /**
     * @brief Handle dependency state change
     *
     * Called when a plugin dependency changes state. Allows the plugin
     * to react to dependency availability changes.
     *
     * @param dependency_id ID of the dependency that changed
     * @param new_state New state of the dependency
     * @return Success or PluginError
     * @retval PluginError::DependencyError If dependency change causes issues
     *
     * @note This method is called automatically by the plugin manager
     * @note The plugin's handle_dependency_change method is called if available
     */
    qtplugin::expected<void, qtplugin::PluginError> handle_dependency_change(
        const QString& dependency_id, qtplugin::PluginState new_state);

    // === IDynamicPlugin Implementation ===

    /**
     * @brief Get interface descriptors
     *
     * @return Vector of all interfaces supported by this plugin
     * @note Interfaces are discovered during plugin initialization
     * @note This method is thread-safe
     */
    std::vector<qtplugin::InterfaceDescriptor> get_interface_descriptors()
        const override;

    /**
     * @brief Check if plugin supports an interface
     *
     * @param interface_id Interface identifier to check
     * @param min_version Minimum required version (optional)
     * @return true if interface is supported with compatible version
     * @note This method is thread-safe
     */
    bool supports_interface(const QString& interface_id,
                            const qtplugin::Version& min_version =
                                qtplugin::Version{}) const override;

    /**
     * @brief Get specific interface descriptor
     *
     * @param interface_id Interface identifier
     * @return Interface descriptor if found, std::nullopt otherwise
     * @note This method is thread-safe
     */
    std::optional<qtplugin::InterfaceDescriptor> get_interface_descriptor(
        const QString& interface_id) const override;

    /**
     * @brief Adapt to a specific interface version
     *
     * Configures the plugin to work with a specific version of an interface.
     * This may involve enabling compatibility layers or feature restrictions.
     *
     * @param interface_id Interface to adapt to
     * @param target_version Target interface version
     * @return Success or PluginError
     * @retval PluginError::InterfaceNotSupported If interface is not available
     * @retval PluginError::VersionNotSupported If version is not compatible
     *
     * @note Adaptation may affect plugin behavior and available features
     * @note This method is thread-safe
     */
    qtplugin::expected<void, qtplugin::PluginError> adapt_to_interface(
        const QString& interface_id,
        const qtplugin::Version& target_version) override;

    /**
     * @brief Negotiate capabilities with another plugin
     *
     * Determines which capabilities can be provided to another plugin
     * based on the requested capabilities and current plugin state.
     *
     * @param other_plugin_id ID of the requesting plugin
     * @param requested_capabilities List of requested capabilities
     * @return Available capabilities or PluginError
     * @retval PluginError::NegotiationFailed If no capabilities can be provided
     *
     * @note Capability negotiation may involve resource allocation
     * @note This method is thread-safe
     */
    qtplugin::expected<std::vector<qtplugin::InterfaceCapability>,
                       qtplugin::PluginError>
    negotiate_capabilities(const QString& other_plugin_id,
                           const std::vector<qtplugin::InterfaceCapability>&
                               requested_capabilities) override;

    /**
     * @brief Get plugin type
     *
     * @return PluginType indicating the nature of this plugin
     * @note Python plugins are typically Service or Extension type
     */
    qtplugin::PluginType get_plugin_type() const override;

    /**
     * @brief Get execution context
     *
     * @return PluginExecutionContext describing the execution environment
     * @note Python plugins run in External process context
     */
    qtplugin::PluginExecutionContext get_execution_context() const override;

    /**
     * @brief Execute arbitrary code
     *
     * Executes Python code within the plugin's execution environment.
     *
     * @param code Python code to execute
     * @param context Optional execution context
     * @return Execution result or PluginError
     * @retval PluginError::ExecutionFailed If code execution fails
     * @retval PluginError::InvalidState If plugin is not initialized
     *
     * @note This method is thread-safe
     * @warning Direct code execution bypasses plugin structure validation
     */
    qtplugin::expected<QVariant, qtplugin::PluginError> execute_code(
        const QString& code, const QJsonObject& context = {}) override;

    /**
     * @brief Invoke a method dynamically
     *
     * Calls a method on the plugin with the specified parameters.
     *
     * @param method_name Name of the method to invoke
     * @param parameters Method parameters
     * @param interface_id Optional interface context
     * @return Method result or PluginError
     * @retval PluginError::MethodNotFound If method doesn't exist
     * @retval PluginError::ExecutionFailed If method execution fails
     * @retval PluginError::InvalidArgument If parameters are incompatible
     *
     * @note This method is thread-safe
     * @note Parameters are automatically converted to Python types
     */
    qtplugin::expected<QVariant, qtplugin::PluginError> invoke_method(
        const QString& method_name, const QVariantList& parameters = {},
        const QString& interface_id = {}) override;

    /**
     * @brief Get available methods
     *
     * @param interface_id Optional interface filter
     * @return List of available method names
     * @note Methods are discovered during plugin initialization
     * @note This method is thread-safe
     */
    std::vector<QString> get_available_methods(
        const QString& interface_id = {}) const override;

    /**
     * @brief Get method signature
     *
     * @param method_name Method name to query
     * @param interface_id Optional interface context
     * @return Method signature as JSON object or std::nullopt
     * @note Signature includes parameter types, return type, and documentation
     * @note This method is thread-safe
     */
    std::optional<QJsonObject> get_method_signature(
        const QString& method_name,
        const QString& interface_id = {}) const override;

    /**
     * @brief Get property value
     *
     * Retrieves the value of a property from the plugin.
     *
     * @param property_name Name of the property to get
     * @param interface_id Optional interface context
     * @return Property value or PluginError
     * @retval PluginError::PropertyNotFound If property doesn't exist
     * @retval PluginError::AccessDenied If property is not readable
     *
     * @note This method is thread-safe
     * @note Values are automatically converted from Python types
     */
    qtplugin::expected<QVariant, qtplugin::PluginError> get_property(
        const QString& property_name,
        const QString& interface_id = {}) override;

    /**
     * @brief Set property value
     *
     * Sets the value of a property in the plugin.
     *
     * @param property_name Name of the property to set
     * @param value New property value
     * @param interface_id Optional interface context
     * @return Success or PluginError
     * @retval PluginError::PropertyNotFound If property doesn't exist
     * @retval PluginError::AccessDenied If property is not writable
     * @retval PluginError::InvalidArgument If value type is incompatible
     *
     * @note This method is thread-safe
     * @note Values are automatically converted to Python types
     */
    qtplugin::expected<void, qtplugin::PluginError> set_property(
        const QString& property_name, const QVariant& value,
        const QString& interface_id = {}) override;

    /**
     * @brief Get available properties
     *
     * @param interface_id Optional interface filter
     * @return List of available property names
     * @note Properties are discovered during plugin initialization
     * @note This method is thread-safe
     */
    std::vector<QString> get_available_properties(
        const QString& interface_id = {}) const override;

    /**
     * @brief Subscribe to events from another plugin
     *
     * Registers to receive events of specified types from a source plugin.
     *
     * @param source_plugin_id ID of the plugin emitting events
     * @param event_types List of event types to subscribe to
     * @param callback Function to call when events are received
     * @return Success or PluginError
     * @retval PluginError::SubscriptionFailed If subscription setup fails
     *
     * @note Callback is called from the plugin's thread context
     * @note This method is thread-safe
     */
    qtplugin::expected<void, qtplugin::PluginError> subscribe_to_events(
        const QString& source_plugin_id,
        const std::vector<QString>& event_types,
        std::function<void(const QString&, const QJsonObject&)> callback)
        override;

    /**
     * @brief Unsubscribe from events
     *
     * Removes event subscriptions for the specified source and event types.
     *
     * @param source_plugin_id ID of the source plugin
     * @param event_types Event types to unsubscribe from (empty = all)
     * @return Success or PluginError
     * @retval PluginError::SubscriptionNotFound If subscription doesn't exist
     *
     * @note This method is thread-safe
     */
    qtplugin::expected<void, qtplugin::PluginError> unsubscribe_from_events(
        const QString& source_plugin_id,
        const std::vector<QString>& event_types = {}) override;

    /**
     * @brief Emit an event
     *
     * Sends an event to all subscribed plugins.
     *
     * @param event_type Type of event being emitted
     * @param event_data Event data as JSON object
     * @return Success or PluginError
     * @retval PluginError::EmissionFailed If event emission fails
     *
     * @note Events are delivered asynchronously
     * @note This method is thread-safe
     */
    qtplugin::expected<void, qtplugin::PluginError> emit_event(
        const QString& event_type, const QJsonObject& event_data) override;

    // === Service Contract Implementation ===

    /**
     * @brief Get service contracts provided by this plugin
     * @return Vector of service contracts
     */
    std::vector<contracts::ServiceContract> get_service_contracts() const override;

    /**
     * @brief Call a service method
     * @param service_name Name of the service
     * @param method_name Name of the method
     * @param parameters Method parameters
     * @param timeout Timeout for the call
     * @return Service call result or PluginError
     */
    qtplugin::expected<QJsonObject, qtplugin::PluginError> call_service(
        const QString& service_name, const QString& method_name,
        const QJsonObject& parameters,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) override;

    /**
     * @brief Call a service method asynchronously
     * @param service_name Name of the service
     * @param method_name Name of the method
     * @param parameters Method parameters
     * @param timeout Timeout for the call
     * @return Future with service call result
     */
    std::future<qtplugin::expected<QJsonObject, qtplugin::PluginError>>
    call_service_async(const QString& service_name, const QString& method_name,
                      const QJsonObject& parameters,
                      std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) override;

    /**
     * @brief Handle incoming service call
     * @param service_name Name of the service
     * @param method_name Name of the method
     * @param parameters Method parameters
     * @return Service call result or PluginError
     */
    qtplugin::expected<QJsonObject, qtplugin::PluginError> handle_service_call(
        const QString& service_name, const QString& method_name,
        const QJsonObject& parameters) override;

private slots:
    /**
     * @brief Handle execution environment errors
     *
     * Processes errors from the Python execution environment and
     * updates plugin state accordingly.
     */
    void handle_environment_error();

    /**
     * @brief Extract plugin information from response data
     *
     * Extract plugin information from response data to avoid code duplication.
     * Updates m_metadata, m_available_methods, and m_available_properties.
     */
    void extract_plugin_info_from_response(const QJsonObject& response_data);

private:
    QString m_plugin_path;   ///< Path to plugin file
    QString m_plugin_id;     ///< Unique plugin identifier
    QJsonObject m_metadata;  ///< Plugin metadata
    qtplugin::PluginState m_state{
        qtplugin::PluginState::Unloaded};  ///< Current plugin state
    std::unique_ptr<PythonExecutionEnvironment>
        m_environment;  ///< Python execution environment
    std::vector<qtplugin::InterfaceDescriptor>
        m_interfaces;                             ///< Supported interfaces
    std::vector<QString> m_available_methods;     ///< Available method names
    std::vector<QString> m_available_properties;  ///< Available property names
    QJsonObject m_configuration;                  ///< Current configuration
    QString m_current_plugin_id;                  ///< Current loaded plugin ID in Python environment
    QHash<QString, QString> m_loaded_plugins;     ///< Map of plugin IDs to their paths
    QHash<QString, std::function<void(const QString&, const QJsonObject&)>> m_event_callbacks;  ///< Event callbacks

    /**
     * @brief Load plugin metadata
     *
     * Loads metadata from plugin file or separate metadata file.
     *
     * @return Success or PluginError
     */
    qtplugin::expected<void, qtplugin::PluginError> load_metadata();

    /**
     * @brief Set up plugin interfaces
     *
     * Discovers and configures supported interfaces.
     *
     * @return Success or PluginError
     */
    qtplugin::expected<void, qtplugin::PluginError> setup_interfaces();

    /**
     * @brief Discover methods and properties
     *
     * Introspects the plugin to find available methods and properties.
     *
     * @return Success or PluginError
     */
    qtplugin::expected<void, qtplugin::PluginError>
    discover_methods_and_properties();

    /**
     * @brief Convert QVariantList to JSON array
     *
     * @param list QVariantList to convert
     * @return Equivalent QJsonArray
     */
    QJsonArray convert_variant_list_to_json(const QVariantList& list) const;

    /**
     * @brief Convert JSON value to QVariant
     *
     * @param value JSON value to convert
     * @return Equivalent QVariant
     */
    QVariant convert_json_to_variant(const QJsonValue& value) const;
};

/**
 * @brief Python plugin factory for creating Python plugin bridges
 *
 * Provides static factory methods for creating and validating Python plugin
 * bridges. This class handles the complexity of plugin creation, validation,
 * and environment setup.
 *
 * The factory provides:
 * - Plugin creation with validation
 * - Python environment checking
 * - Plugin file validation
 * - Version compatibility checking
 * - Error handling and reporting
 *
 * @note All methods are static and thread-safe
 * @since QtForge 3.0.0
 */
class PythonPluginFactory {
public:
    /**
     * @brief Create a Python plugin bridge
     *
     * Creates and configures a new Python plugin bridge for the specified
     * plugin file. The plugin is validated before creation.
     *
     * @param plugin_path Path to the Python plugin file
     * @param parent Parent QObject for the created plugin
     * @return Unique pointer to plugin bridge or PluginError
     * @retval PluginError::FileNotFound If plugin file doesn't exist
     * @retval PluginError::ValidationFailed If plugin validation fails
     * @retval PluginError::PythonNotAvailable If Python interpreter is not
     * available
     * @retval PluginError::CreationFailed If plugin creation fails
     *
     * @note Plugin validation includes metadata, structure, and dependency
     * checks
     * @note This method is thread-safe
     *
     * @code{.cpp}
     * auto result = PythonPluginFactory::create_plugin("plugins/my_plugin.py");
     * if (result) {
     *     auto plugin = std::move(result.value());
     *     // Use plugin...
     * } else {
     *     qDebug() << "Failed to create plugin:" << result.error().message();
     * }
     * @endcode
     */
    static qtplugin::expected<std::unique_ptr<PythonPluginBridge>, PluginError>
    create_plugin(const QString& plugin_path, QObject* parent = nullptr);

    /**
     * @brief Validate Python plugin
     *
     * Performs comprehensive validation of a Python plugin file without
     * creating a plugin instance. This includes syntax checking, metadata
     * validation, and dependency verification.
     *
     * @param plugin_path Path to the Python plugin file
     * @return Success or PluginError with validation details
     * @retval PluginError::FileNotFound If plugin file doesn't exist
     * @retval PluginError::SyntaxError If Python syntax is invalid
     * @retval PluginError::MetadataError If metadata is missing or invalid
     * @retval PluginError::DependencyError If dependencies are not satisfied
     *
     * @note This method is useful for plugin discovery and pre-loading
     * validation
     * @note Validation is performed without executing the plugin code
     * @note This method is thread-safe
     *
     * @code{.cpp}
     * auto validation =
     * PythonPluginFactory::validate_plugin("plugins/test.py"); if (!validation)
     * { qDebug() << "Plugin validation failed:" <<
     * validation.error().message();
     * }
     * @endcode
     */
    static qtplugin::expected<void, PluginError> validate_plugin(
        const QString& plugin_path);

    /**
     * @brief Check if Python is available
     *
     * Tests if the specified Python interpreter is available and functional.
     * This includes checking if the qtforge module can be imported.
     *
     * @param python_path Path to Python interpreter (default: "python")
     * @return true if Python is available and functional
     * @return false if Python is not available or qtforge module is missing
     *
     * @note This method may take a few seconds as it starts a Python process
     * @note This method is thread-safe
     *
     * @code{.cpp}
     * if (PythonPluginFactory::is_python_available()) {
     *     qDebug() << "Python is ready for plugin execution";
     * } else {
     *     qDebug() << "Python is not available or qtforge module is missing";
     * }
     * @endcode
     */
    static bool is_python_available(const QString& python_path = "python");

    /**
     * @brief Get Python version
     *
     * Retrieves the version string of the specified Python interpreter.
     *
     * @param python_path Path to Python interpreter (default: "python")
     * @return Python version string (e.g., "3.9.7") or empty string if
     * unavailable
     *
     * @note This method may take a few seconds as it starts a Python process
     * @note This method is thread-safe
     *
     * @code{.cpp}
     * QString version = PythonPluginFactory::get_python_version();
     * if (!version.isEmpty()) {
     *     qDebug() << "Python version:" << version;
     * }
     * @endcode
     */
    static QString get_python_version(const QString& python_path = "python");

    /**
     * @brief Get required Python modules
     *
     * @return List of Python modules required for plugin execution
     * @note These modules must be available in the Python environment
     */
    static QStringList required_python_modules();

    /**
     * @brief Check if required modules are available
     *
     * Verifies that all required Python modules are available in the
     * specified Python environment.
     *
     * @param python_path Path to Python interpreter
     * @return List of missing modules (empty if all are available)
     * @note This method is thread-safe
     */
    static QStringList check_required_modules(
        const QString& python_path = "python");
};

}  // namespace qtplugin

Q_DECLARE_INTERFACE(qtplugin::PythonPluginBridge,
                    "qtplugin.PythonPluginBridge/3.2")
