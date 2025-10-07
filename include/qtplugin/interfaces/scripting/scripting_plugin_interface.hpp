/**
 * @file scripting_plugin_interface.hpp
 * @brief Scripting plugin interface for script execution and automation
 * @version 3.0.0
 */

#pragma once

#include <QJSEngine>
#include <QJSValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QObject>
#include <QVariant>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>
#include "../core/plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Scripting language types
 */
enum class ScriptingLanguage {
    JavaScript,  ///< JavaScript/ECMAScript
    Python,      ///< Python scripting
    Lua,         ///< Lua scripting
    Custom       ///< Custom scripting language
};

/**
 * @brief Script execution modes
 */
enum class ScriptExecutionMode {
    Synchronous,   ///< Execute synchronously
    Asynchronous,  ///< Execute asynchronously
    Background,    ///< Execute in background thread
    Scheduled      ///< Execute on schedule
};

/**
 * @brief Script capabilities
 */
enum class ScriptCapability : uint32_t {
    None = 0x0000,
    FileSystem = 0x0001,  ///< File system access
    Network = 0x0002,     ///< Network access
    Database = 0x0004,    ///< Database access
    UI = 0x0008,          ///< UI manipulation
    System = 0x0010,      ///< System calls
    Plugins = 0x0020,     ///< Plugin interaction
    Events = 0x0040,      ///< Event handling
    Timers = 0x0080,      ///< Timer operations
    Threading = 0x0100,   ///< Multi-threading
    Debugging = 0x0200,   ///< Debugging support
    Profiling = 0x0400,   ///< Performance profiling
    Sandboxed = 0x0800    ///< Sandboxed execution
};

using ScriptCapabilities = std::underlying_type_t<ScriptCapability>;

/**
 * @brief Script execution context
 */
struct ScriptExecutionContext {
    QString script_id;                            ///< Script identifier
    QString script_name;                          ///< Script name
    ScriptingLanguage language;                   ///< Scripting language
    ScriptExecutionMode execution_mode;           ///< Execution mode
    ScriptCapabilities allowed_capabilities = 0;  ///< Allowed capabilities
    std::chrono::milliseconds timeout{30000};     ///< Execution timeout
    QJsonObject parameters;                       ///< Script parameters
    QJsonObject environment;                      ///< Environment variables
    QString working_directory;                    ///< Working directory
    bool debug_mode = false;                      ///< Debug mode enabled
    bool profile_execution = false;               ///< Profile execution
    QJsonObject custom_options;                   ///< Custom options

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static ScriptExecutionContext from_json(const QJsonObject& json);
};

/**
 * @brief Script execution result
 */
struct ScriptExecutionResult {
    bool success = false;                         ///< Execution success
    QVariant return_value;                        ///< Script return value
    QString output;                               ///< Script output
    QString error_output;                         ///< Error output
    QString error_message;                        ///< Error message
    int exit_code = 0;                            ///< Exit code
    std::chrono::milliseconds execution_time{0};  ///< Execution time
    QJsonObject debug_info;                       ///< Debug information
    QJsonObject profile_data;                     ///< Profiling data
    QJsonObject metadata;                         ///< Additional metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static ScriptExecutionResult from_json(const QJsonObject& json);
};

/**
 * @brief Script progress callback
 */
using ScriptProgressCallback =
    std::function<void(int progress, const QString& status)>;

/**
 * @brief Script output callback
 */
using ScriptOutputCallback =
    std::function<void(const QString& output, bool is_error)>;

/**
 * @brief Script information
 */
struct ScriptInfo {
    QString id;                                    ///< Script identifier
    QString name;                                  ///< Script name
    QString description;                           ///< Script description
    QString author;                                ///< Script author
    QString version;                               ///< Script version
    ScriptingLanguage language;                    ///< Scripting language
    ScriptCapabilities required_capabilities = 0;  ///< Required capabilities
    QStringList dependencies;                      ///< Script dependencies
    QString source_code;                           ///< Script source code
    QString file_path;                             ///< Script file path
    QJsonObject metadata;                          ///< Script metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static ScriptInfo from_json(const QJsonObject& json);
};

/**
 * @brief Scripting plugin interface
 *
 * This interface extends the base plugin interface with scripting
 * capabilities for executing scripts in various languages and
 * providing automation functionality.
 */
class IScriptingPlugin : public virtual IPlugin {
public:
    ~IScriptingPlugin() override = default;

    // === Language Support ===

    /**
     * @brief Get supported scripting languages
     * @return Vector of supported languages
     */
    virtual std::vector<ScriptingLanguage> supported_languages() const = 0;

    /**
     * @brief Check if language is supported
     * @param language Language to check
     * @return true if language is supported
     */
    virtual bool supports_language(ScriptingLanguage language) const {
        auto languages = supported_languages();
        return std::find(languages.begin(), languages.end(), language) !=
               languages.end();
    }

    /**
     * @brief Get supported script capabilities
     * @return Bitfield of supported capabilities
     */
    virtual ScriptCapabilities supported_capabilities() const noexcept = 0;

    /**
     * @brief Check if capability is supported
     * @param capability Capability to check
     * @return true if capability is supported
     */
    bool supports_capability(ScriptCapability capability) const noexcept {
        return (supported_capabilities() &
                static_cast<ScriptCapabilities>(capability)) != 0;
    }

    // === Script Execution ===

    /**
     * @brief Execute script synchronously
     * @param script_code Script source code
     * @param context Execution context
     * @return Execution result or error
     */
    virtual qtplugin::expected<ScriptExecutionResult, PluginError>
    execute_script(const QString& script_code,
                   const ScriptExecutionContext& context) = 0;

    /**
     * @brief Execute script asynchronously
     * @param script_code Script source code
     * @param context Execution context
     * @param progress_callback Optional progress callback
     * @param output_callback Optional output callback
     * @return Future with execution result
     */
    virtual std::future<qtplugin::expected<ScriptExecutionResult, PluginError>>
    execute_script_async(const QString& script_code,
                         const ScriptExecutionContext& context,
                         ScriptProgressCallback progress_callback = nullptr,
                         ScriptOutputCallback output_callback = nullptr) = 0;

    /**
     * @brief Execute script from file
     * @param file_path Script file path
     * @param context Execution context
     * @return Execution result or error
     */
    virtual qtplugin::expected<ScriptExecutionResult, PluginError>
    execute_script_file(const QString& file_path,
                        const ScriptExecutionContext& context) = 0;

    /**
     * @brief Stop script execution
     * @param script_id Script identifier
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> stop_script(
        const QString& script_id) = 0;

    // === Script Management ===

    /**
     * @brief Load script from file
     * @param file_path Script file path
     * @return Script information or error
     */
    virtual qtplugin::expected<ScriptInfo, PluginError> load_script(
        const QString& file_path) = 0;

    /**
     * @brief Save script to file
     * @param script_info Script information
     * @param file_path Target file path
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> save_script(
        const ScriptInfo& script_info, const QString& file_path) = 0;

    /**
     * @brief Validate script syntax
     * @param script_code Script source code
     * @param language Scripting language
     * @return Validation result or error
     */
    virtual qtplugin::expected<bool, PluginError> validate_script(
        const QString& script_code, ScriptingLanguage language) = 0;

    /**
     * @brief Get running scripts
     * @return Vector of running script IDs
     */
    virtual std::vector<QString> get_running_scripts() const = 0;

    /**
     * @brief Get script status
     * @param script_id Script identifier
     * @return Script status information
     */
    virtual qtplugin::expected<QJsonObject, PluginError> get_script_status(
        const QString& script_id) const = 0;

    // === Environment Management ===

    /**
     * @brief Set global variable
     * @param name Variable name
     * @param value Variable value
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> set_global_variable(
        const QString& name, const QVariant& value) = 0;

    /**
     * @brief Get global variable
     * @param name Variable name
     * @return Variable value or error
     */
    virtual qtplugin::expected<QVariant, PluginError> get_global_variable(
        const QString& name) const = 0;

    /**
     * @brief Register function
     * @param name Function name
     * @param function Function implementation
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> register_function(
        const QString& name, const QJSValue& function) = 0;

    /**
     * @brief Unregister function
     * @param name Function name
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> unregister_function(
        const QString& name) = 0;

    // === Event System ===

    /**
     * @brief Register event handler
     * @param event_name Event name
     * @param handler_script Script to execute on event
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> register_event_handler(
        const QString& event_name, const QString& handler_script) = 0;

    /**
     * @brief Unregister event handler
     * @param event_name Event name
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> unregister_event_handler(
        const QString& event_name) = 0;

    /**
     * @brief Trigger event
     * @param event_name Event name
     * @param event_data Event data
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> trigger_event(
        const QString& event_name, const QJsonObject& event_data = {}) = 0;

    // === Debugging Support ===

    /**
     * @brief Set breakpoint
     * @param script_id Script identifier
     * @param line_number Line number
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> set_breakpoint(
        const QString& script_id, int line_number) {
        Q_UNUSED(script_id)
        Q_UNUSED(line_number)
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "Debugging not supported");
    }

    /**
     * @brief Remove breakpoint
     * @param script_id Script identifier
     * @param line_number Line number
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> remove_breakpoint(
        const QString& script_id, int line_number) {
        Q_UNUSED(script_id)
        Q_UNUSED(line_number)
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "Debugging not supported");
    }

    // === Statistics ===

    /**
     * @brief Get execution statistics
     * @return Execution statistics as JSON object
     */
    virtual QJsonObject get_execution_statistics() const {
        return QJsonObject{};
    }

    /**
     * @brief Reset execution statistics
     */
    virtual void reset_statistics() {}
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::ScriptingLanguage)
Q_DECLARE_METATYPE(qtplugin::ScriptExecutionMode)
Q_DECLARE_METATYPE(qtplugin::ScriptCapability)
Q_DECLARE_METATYPE(qtplugin::ScriptExecutionContext)
Q_DECLARE_METATYPE(qtplugin::ScriptExecutionResult)
Q_DECLARE_METATYPE(qtplugin::ScriptInfo)

Q_DECLARE_INTERFACE(qtplugin::IScriptingPlugin, "qtplugin.IScriptingPlugin/3.0")
