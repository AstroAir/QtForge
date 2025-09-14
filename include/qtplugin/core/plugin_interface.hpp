/**
 * @file plugin_interface.hpp
 * @brief Core plugin interface definitions using modern C++ features
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QUuid>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "../utils/error_handling.hpp"
#include "../utils/version.hpp"

// Forward declarations
class QWidget;

namespace qtplugin {

/**
 * @brief Plugin capabilities enumeration
 */
enum class PluginCapability : uint32_t {
    None = 0x0000,
    UI = 0x0001,
    Service = 0x0002,
    Network = 0x0004,
    DataProcessing = 0x0008,
    Scripting = 0x0010,
    FileSystem = 0x0020,
    Database = 0x0040,
    AsyncInit = 0x0080,
    HotReload = 0x0100,
    Configuration = 0x0200,
    Logging = 0x0400,
    Security = 0x0800,
    Threading = 0x1000,
    Monitoring = 0x2000
};

using PluginCapabilities = std::underlying_type_t<PluginCapability>;

/**
 * @brief Plugin state enumeration
 */
enum class PluginState {
    Unloaded,      ///< Plugin is not loaded
    Loading,       ///< Plugin is being loaded
    Loaded,        ///< Plugin is loaded but not initialized
    Initializing,  ///< Plugin is being initialized
    Running,       ///< Plugin is running normally
    Paused,        ///< Plugin is paused
    Stopping,      ///< Plugin is being stopped
    Stopped,       ///< Plugin is stopped
    Error,         ///< Plugin is in error state
    Reloading      ///< Plugin is being reloaded
};

/**
 * @brief Plugin priority levels
 */
enum class PluginPriority {
    Lowest = 0,
    Low = 25,
    Normal = 50,
    High = 75,
    Highest = 100,
    Critical = 125
};

/**
 * @brief Plugin metadata structure
 */
struct PluginMetadata {
    std::string name;
    std::string description;
    Version version;
    std::string author;
    std::string license;
    std::string homepage;
    std::string category;
    std::vector<std::string> tags;
    std::vector<std::string> dependencies;
    PluginCapabilities capabilities = 0;
    PluginPriority priority = PluginPriority::Normal;
    std::optional<Version> min_host_version;
    std::optional<Version> max_host_version;
    QJsonObject custom_data;

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static qtplugin::expected<PluginMetadata, PluginError> from_json(
        const QJsonObject& json);
};

/**
 * @brief Base plugin interface
 *
 * This is the core interface that all plugins must implement. It provides
 * essential functionality for plugin lifecycle management, metadata access,
 * and basic operations.
 */
class IPlugin {
public:
    virtual ~IPlugin() = default;

    // === Metadata ===

    /**
     * @brief Get plugin name
     * @return Plugin name as string view
     */
    virtual std::string_view name() const noexcept = 0;

    /**
     * @brief Get plugin description
     * @return Plugin description as string view
     */
    virtual std::string_view description() const noexcept = 0;

    /**
     * @brief Get plugin version
     * @return Plugin version
     */
    virtual Version version() const noexcept = 0;

    /**
     * @brief Get plugin author
     * @return Plugin author as string view
     */
    virtual std::string_view author() const noexcept = 0;

    /**
     * @brief Get unique plugin identifier
     * @return Plugin ID as string
     */
    virtual std::string id() const noexcept = 0;

    /**
     * @brief Get plugin UUID
     * @return Plugin UUID
     */
    virtual QUuid uuid() const noexcept {
        return QUuid::createUuidV5(QUuid(), QString::fromStdString(id()));
    }

    /**
     * @brief Get plugin category
     * @return Plugin category
     */
    virtual std::string_view category() const noexcept { return "General"; }

    /**
     * @brief Get plugin license
     * @return Plugin license
     */
    virtual std::string_view license() const noexcept { return "Unknown"; }

    /**
     * @brief Get plugin homepage URL
     * @return Plugin homepage URL
     */
    virtual std::string_view homepage() const noexcept { return ""; }

    /**
     * @brief Get plugin metadata
     * @return Complete plugin metadata
     */
    virtual PluginMetadata metadata() const {
        PluginMetadata meta;
        meta.name = name();
        meta.description = description();
        meta.version = version();
        meta.author = author();
        meta.license = license();
        meta.homepage = homepage();
        meta.category = category();
        meta.capabilities = capabilities();
        return meta;
    }

    // === Lifecycle Management ===

    /**
     * @brief Initialize the plugin
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> initialize() = 0;

    /**
     * @brief Shutdown the plugin
     *
     * This method should never throw exceptions and should clean up
     * all resources used by the plugin.
     */
    virtual void shutdown() noexcept = 0;

    /**
     * @brief Get current plugin state
     * @return Current plugin state
     */
    virtual PluginState state() const noexcept = 0;

    /**
     * @brief Check if plugin is initialized
     * @return true if plugin is initialized
     */
    virtual bool is_initialized() const noexcept {
        auto current_state = state();
        return current_state == PluginState::Running ||
               current_state == PluginState::Paused;
    }

    /**
     * @brief Pause plugin execution
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> pause() {
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "Pause not supported");
    }

    /**
     * @brief Resume plugin execution
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> resume() {
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "Resume not supported");
    }

    /**
     * @brief Restart the plugin
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> restart() {
        shutdown();
        return initialize();
    }

    // === Capabilities ===

    /**
     * @brief Get plugin capabilities
     * @return Bitfield of plugin capabilities
     */
    virtual PluginCapabilities capabilities() const noexcept = 0;

    /**
     * @brief Check if plugin has specific capability
     * @param capability Capability to check
     * @return true if plugin has the capability
     */
    bool has_capability(PluginCapability capability) const noexcept {
        return (capabilities() & static_cast<PluginCapabilities>(capability)) !=
               0;
    }

    /**
     * @brief Get plugin priority
     * @return Plugin priority level
     */
    virtual PluginPriority priority() const noexcept {
        return PluginPriority::Normal;
    }

    // === Configuration ===

    /**
     * @brief Get default configuration
     * @return Default configuration as JSON object, or nullopt if not
     * configurable
     */
    virtual std::optional<QJsonObject> default_configuration() const {
        return std::nullopt;
    }

    /**
     * @brief Configure the plugin
     * @param config Configuration data
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> configure(
        const QJsonObject& config) {
        Q_UNUSED(config)
        return make_success();
    }

    /**
     * @brief Get current configuration
     * @return Current configuration as JSON object
     */
    virtual QJsonObject current_configuration() const { return QJsonObject{}; }

    /**
     * @brief Validate configuration
     * @param config Configuration to validate
     * @return true if configuration is valid
     */
    virtual bool validate_configuration(const QJsonObject& config) const {
        Q_UNUSED(config)
        return true;
    }

    // === Commands ===

    /**
     * @brief Execute a plugin command
     * @param command Command name
     * @param params Command parameters
     * @return Command result or error information
     */
    virtual qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) = 0;

    /**
     * @brief Get list of available commands
     * @return Vector of command names
     */
    virtual std::vector<std::string> available_commands() const = 0;

    /**
     * @brief Check if command is available
     * @param command Command name to check
     * @return true if command is available
     */
    bool has_command(std::string_view command) const {
        auto commands = available_commands();
        return std::find(commands.begin(), commands.end(), command) !=
               commands.end();
    }

    // === Dependencies ===

    /**
     * @brief Get list of required dependencies
     * @return Vector of dependency identifiers
     */
    virtual std::vector<std::string> dependencies() const { return {}; }

    /**
     * @brief Get list of optional dependencies
     * @return Vector of optional dependency identifiers
     */
    virtual std::vector<std::string> optional_dependencies() const {
        return {};
    }

    /**
     * @brief Check if all dependencies are satisfied
     * @return true if all dependencies are available
     */
    virtual bool dependencies_satisfied() const {
        return true;  // Override in derived classes
    }

    // === Error Handling ===

    /**
     * @brief Get last error message
     * @return Last error message, empty if no error
     */
    virtual std::string last_error() const { return {}; }

    /**
     * @brief Get error log
     * @return Vector of error messages
     */
    virtual std::vector<std::string> error_log() const { return {}; }

    /**
     * @brief Clear error log
     */
    virtual void clear_errors() {}

    // === Monitoring ===

    /**
     * @brief Get plugin uptime
     * @return Duration since plugin was initialized
     */
    virtual std::chrono::milliseconds uptime() const {
        return std::chrono::milliseconds{0};
    }

    /**
     * @brief Get performance metrics
     * @return Performance metrics as JSON object
     */
    virtual QJsonObject performance_metrics() const { return QJsonObject{}; }

    /**
     * @brief Get resource usage information
     * @return Resource usage as JSON object
     */
    virtual QJsonObject resource_usage() const { return QJsonObject{}; }

    // === Threading ===

    /**
     * @brief Check if plugin is thread-safe
     * @return true if plugin can be safely used from multiple threads
     */
    virtual bool is_thread_safe() const noexcept { return false; }

    /**
     * @brief Get supported thread model
     * @return Thread model description
     */
    virtual std::string_view thread_model() const noexcept {
        return "single-threaded";
    }
};

/**
 * @brief Bitwise OR operator for plugin capabilities
 */
constexpr PluginCapabilities operator|(PluginCapability lhs,
                                       PluginCapability rhs) noexcept {
    return static_cast<PluginCapabilities>(lhs) |
           static_cast<PluginCapabilities>(rhs);
}

/**
 * @brief Bitwise OR operator for plugin capabilities
 */
constexpr PluginCapabilities operator|(PluginCapabilities lhs,
                                       PluginCapability rhs) noexcept {
    return lhs | static_cast<PluginCapabilities>(rhs);
}

/**
 * @brief Bitwise AND operator for plugin capabilities
 */
constexpr PluginCapabilities operator&(PluginCapabilities lhs,
                                       PluginCapability rhs) noexcept {
    return lhs & static_cast<PluginCapabilities>(rhs);
}

}  // namespace qtplugin

Q_DECLARE_INTERFACE(qtplugin::IPlugin, "qtplugin.IPlugin/3.0")
