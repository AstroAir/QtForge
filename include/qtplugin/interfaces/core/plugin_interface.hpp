/**
 * @file plugin_interface.hpp
 * @brief Core plugin interface definitions
 * @version 3.2.0
 */

#pragma once

#include <QJsonObject>
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "../../utils/error_handling.hpp"
#include "../../utils/version.hpp"

namespace qtplugin {

/**
 * @brief Plugin lifecycle states
 */
enum class PluginState {
    Unloaded = 0,      ///< Plugin is not loaded
    Loading = 1,       ///< Plugin is being loaded
    Loaded = 2,        ///< Plugin is loaded but not initialized
    Initializing = 3,  ///< Plugin is being initialized
    Running = 4,       ///< Plugin is running normally
    Paused = 5,        ///< Plugin is paused
    Stopping = 6,      ///< Plugin is being stopped
    Stopped = 7,       ///< Plugin is stopped
    Error = 8,         ///< Plugin is in error state
    Reloading = 9      ///< Plugin is being reloaded
};

/**
 * @brief Plugin capability flags
 */
enum class PluginCapability : uint32_t {
    None = 0x0000,            ///< No special capabilities
    UI = 0x0001,              ///< Provides user interface
    Service = 0x0002,         ///< Provides background service
    Network = 0x0004,         ///< Network communication
    DataProcessing = 0x0008,  ///< Data processing capabilities
    Scripting = 0x0010,       ///< Scripting support
    FileSystem = 0x0020,      ///< File system access
    Database = 0x0040,        ///< Database access
    AsyncInit = 0x0080,       ///< Asynchronous initialization
    HotReload = 0x0100,       ///< Hot reload support
    Configuration = 0x0200,   ///< Configuration management
    Logging = 0x0400,         ///< Logging capabilities
    Security = 0x0800,        ///< Security features
    Threading = 0x1000,       ///< Multi-threading support
    Monitoring = 0x2000       ///< Monitoring and metrics
};

// Type alias for backward compatibility
using PluginCapabilities = uint32_t;

/**
 * @brief Plugin type enumeration
 */
enum class PluginType {
    Native = 0,      ///< Native C++ plugin
    Python = 1,      ///< Python script plugin
    JavaScript = 2,  ///< JavaScript plugin
    Lua = 3,         ///< Lua script plugin
    Remote = 4,      ///< Remote plugin
    Composite = 5,   ///< Composite plugin (combination of multiple plugins)
    // Legacy aliases for backward compatibility
    Core = Native,
    Extension = Native,
    Service = Native,
    UI = Native,
    Bridge = Native
};

/**
 * @brief Plugin execution context
 */
enum class PluginExecutionContext {
    MainThread = 0,
    WorkerThread = 1,
    Isolated = 2
};

/**
 * @brief Interface descriptor for plugin capabilities
 *
 * NOTE: This simple version is superseded by the extended version in
 * dynamic_plugin_interface.hpp Commented out to avoid redefinition conflicts
 */
/*
struct InterfaceDescriptor {
    std::string interface_id;
    Version version;
    std::string name;
    std::string description;
};
*/

/**
 * @brief Interface capability flags
 *
 * NOTE: This enum version is superseded by the struct version in
 * dynamic_plugin_interface.hpp Commented out to avoid redefinition conflicts
 */
/*
enum class InterfaceCapability : uint32_t {
    None = 0x0000,
    Synchronous = 0x0001,
    Asynchronous = 0x0002,
    EventDriven = 0x0004,
    Configurable = 0x0008
};
*/

/**
 * @brief Plugin priority levels
 */
enum class PluginPriority {
    Lowest = 0,
    Low = 1,
    Normal = 2,
    High = 3,
    Highest = 4
};

/**
 * @brief Plugin metadata structure
 */
struct PluginMetadata {
    std::string name;                       ///< Plugin name
    Version version;                        ///< Plugin version
    std::string description;                ///< Plugin description
    std::string author;                     ///< Plugin author
    std::string license;                    ///< Plugin license
    std::string category;                   ///< Plugin category
    std::string homepage;                   ///< Plugin homepage URL
    std::vector<std::string> dependencies;  ///< Plugin dependencies
    std::vector<std::string> tags;          ///< Plugin tags
    uint32_t capabilities = 0;              ///< Plugin capabilities (bitfield)
    PluginPriority priority = PluginPriority::Normal;  ///< Plugin priority

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
};

/**
 * @brief Base plugin interface
 */
class IPlugin {
public:
    virtual ~IPlugin() = default;

    /**
     * @brief Initialize the plugin
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> initialize() = 0;

    /**
     * @brief Shutdown the plugin
     */
    virtual void shutdown() noexcept = 0;

    /**
     * @brief Get plugin metadata
     * @return Plugin metadata
     */
    virtual PluginMetadata metadata() const = 0;

    /**
     * @brief Get current plugin state
     * @return Current state
     */
    virtual PluginState state() const noexcept = 0;

    /**
     * @brief Get plugin capabilities
     * @return Capability flags
     */
    virtual uint32_t capabilities() const noexcept = 0;

    /**
     * @brief Get plugin priority
     * @return Plugin priority
     */
    virtual PluginPriority priority() const noexcept = 0;

    /**
     * @brief Check if plugin is initialized
     * @return True if initialized
     */
    virtual bool is_initialized() const noexcept = 0;

    /**
     * @brief Execute a plugin command
     * @param command Command name
     * @param params Command parameters
     * @return Command result or error
     */
    virtual qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) = 0;

    /**
     * @brief Get available commands
     * @return List of command names
     */
    virtual std::vector<std::string> available_commands() const = 0;

    /**
     * @brief Configure the plugin
     * @param config Configuration object
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> configure(
        const QJsonObject& config) = 0;

    /**
     * @brief Get plugin configuration
     * @return Current configuration
     */
    virtual QJsonObject get_configuration() const = 0;
};

}  // namespace qtplugin

Q_DECLARE_INTERFACE(qtplugin::IPlugin, "qtplugin.IPlugin/3.0")
