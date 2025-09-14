/**
 * @file core_bindings.cpp
 * @brief Minimal core plugin system bindings for Lua
 * @version 3.2.0
 */

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include "../../../include/qtplugin/utils/version.hpp"
#include "../../../include/qtplugin/core/plugin_interface.hpp"
#include "../../../include/qtplugin/core/advanced_plugin_interface.hpp"
#include "../../../include/qtplugin/core/dynamic_plugin_interface.hpp"
#include "../../../include/qtplugin/core/service_plugin_interface.hpp"
#include "../qt_conversions.cpp"

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register Version class with Lua
 */
void register_version_bindings(sol::state_view& lua) {
    auto version_type = lua.new_usertype<qtplugin::Version>("Version",
        sol::constructors<qtplugin::Version(int, int, int)>()
    );

    version_type["major"] = &qtplugin::Version::major;
    version_type["minor"] = &qtplugin::Version::minor;
    version_type["patch"] = &qtplugin::Version::patch;
    version_type["to_string"] = &qtplugin::Version::to_string;
    version_type["is_compatible_with"] = &qtplugin::Version::is_compatible_with;

    // String representation
    version_type[sol::meta_function::to_string] = [](const qtplugin::Version& v) {
        return v.to_string();
    };
}

/**
 * @brief Register advanced plugin interfaces with Lua
 */
void register_advanced_interfaces_bindings(sol::state_view& lua) {
    // Plugin type enum
    lua.new_enum<qtplugin::PluginType>("PluginType", {
        {"Native", qtplugin::PluginType::Native},
        {"Python", qtplugin::PluginType::Python},
        {"JavaScript", qtplugin::PluginType::JavaScript},
        {"Lua", qtplugin::PluginType::Lua},
        {"Remote", qtplugin::PluginType::Remote},
        {"Composite", qtplugin::PluginType::Composite}
    });

    // Service execution mode enum
    lua.new_enum<qtplugin::ServiceExecutionMode>("ServiceExecutionMode", {
        {"MainThread", qtplugin::ServiceExecutionMode::MainThread},
        {"WorkerThread", qtplugin::ServiceExecutionMode::WorkerThread},
        {"ThreadPool", qtplugin::ServiceExecutionMode::ThreadPool},
        {"Async", qtplugin::ServiceExecutionMode::Async},
        {"Custom", qtplugin::ServiceExecutionMode::Custom}
    });

    // Service state enum
    lua.new_enum<qtplugin::ServiceState>("ServiceState", {
        {"Stopped", qtplugin::ServiceState::Stopped},
        {"Starting", qtplugin::ServiceState::Starting},
        {"Running", qtplugin::ServiceState::Running},
        {"Pausing", qtplugin::ServiceState::Pausing},
        {"Paused", qtplugin::ServiceState::Paused},
        {"Resuming", qtplugin::ServiceState::Resuming},
        {"Stopping", qtplugin::ServiceState::Stopping},
        {"Error", qtplugin::ServiceState::Error},
        {"Restarting", qtplugin::ServiceState::Restarting}
    });

    // Service priority enum
    lua.new_enum<qtplugin::ServicePriority>("ServicePriority", {
        {"Idle", qtplugin::ServicePriority::Idle},
        {"Low", qtplugin::ServicePriority::Low},
        {"Normal", qtplugin::ServicePriority::Normal},
        {"High", qtplugin::ServicePriority::High},
        {"Critical", qtplugin::ServicePriority::Critical}
    });

    // Service health enum
    lua.new_enum<qtplugin::ServiceHealth>("ServiceHealth", {
        {"Unknown", qtplugin::ServiceHealth::Unknown},
        {"Healthy", qtplugin::ServiceHealth::Healthy},
        {"Warning", qtplugin::ServiceHealth::Warning},
        {"Critical", qtplugin::ServiceHealth::Critical},
        {"Unhealthy", qtplugin::ServiceHealth::Unhealthy}
    });
}

/**
 * @brief Register PluginState and related enums
 */
void register_plugin_state_bindings(sol::state& lua) {
    // PluginState enum
    lua.new_enum<qtplugin::PluginState>("PluginState", {
        {"Unloaded", qtplugin::PluginState::Unloaded},
        {"Loading", qtplugin::PluginState::Loading},
        {"Loaded", qtplugin::PluginState::Loaded},
        {"Initializing", qtplugin::PluginState::Initializing},
        {"Running", qtplugin::PluginState::Running},
        {"Stopping", qtplugin::PluginState::Stopping},
        {"Error", qtplugin::PluginState::Error}
    });

    // PluginCapability enum
    lua.new_enum<qtplugin::PluginCapability>("PluginCapability", {
        {"None", qtplugin::PluginCapability::None},
        {"DataProcessing", qtplugin::PluginCapability::DataProcessing},
        {"UserInterface", qtplugin::PluginCapability::UserInterface},
        {"NetworkAccess", qtplugin::PluginCapability::NetworkAccess},
        {"FileSystemAccess", qtplugin::PluginCapability::FileSystemAccess},
        {"DatabaseAccess", qtplugin::PluginCapability::DatabaseAccess},
        {"Scripting", qtplugin::PluginCapability::Scripting},
        {"Configuration", qtplugin::PluginCapability::Configuration},
        {"Logging", qtplugin::PluginCapability::Logging},
        {"Security", qtplugin::PluginCapability::Security},
        {"Monitoring", qtplugin::PluginCapability::Monitoring},
        {"Communication", qtplugin::PluginCapability::Communication}
    });

    // PluginPriority enum
    lua.new_enum<qtplugin::PluginPriority>("PluginPriority", {
        {"Lowest", qtplugin::PluginPriority::Lowest},
        {"Low", qtplugin::PluginPriority::Low},
        {"Normal", qtplugin::PluginPriority::Normal},
        {"High", qtplugin::PluginPriority::High},
        {"Highest", qtplugin::PluginPriority::Highest}
    });
}

/**
 * @brief Register PluginError and error handling types
 */
void register_error_handling_bindings(sol::state& lua) {
    // PluginErrorCode enum
    lua.new_enum<qtplugin::PluginErrorCode>("PluginErrorCode", {
        {"None", qtplugin::PluginErrorCode::None},
        {"LoadFailed", qtplugin::PluginErrorCode::LoadFailed},
        {"InitializationFailed", qtplugin::PluginErrorCode::InitializationFailed},
        {"ExecutionFailed", qtplugin::PluginErrorCode::ExecutionFailed},
        {"InvalidState", qtplugin::PluginErrorCode::InvalidState},
        {"InvalidParameters", qtplugin::PluginErrorCode::InvalidParameters},
        {"MethodNotFound", qtplugin::PluginErrorCode::MethodNotFound},
        {"PropertyNotFound", qtplugin::PluginErrorCode::PropertyNotFound},
        {"CommandNotFound", qtplugin::PluginErrorCode::CommandNotFound},
        {"NotImplemented", qtplugin::PluginErrorCode::NotImplemented},
        {"NotSupported", qtplugin::PluginErrorCode::NotSupported},
        {"FileNotFound", qtplugin::PluginErrorCode::FileNotFound},
        {"PermissionDenied", qtplugin::PluginErrorCode::PermissionDenied},
        {"NetworkError", qtplugin::PluginErrorCode::NetworkError},
        {"TimeoutError", qtplugin::PluginErrorCode::TimeoutError},
        {"ConfigurationError", qtplugin::PluginErrorCode::ConfigurationError},
        {"DependencyError", qtplugin::PluginErrorCode::DependencyError},
        {"SecurityError", qtplugin::PluginErrorCode::SecurityError},
        {"UnknownError", qtplugin::PluginErrorCode::UnknownError}
    });

    // PluginError type
    auto error_type = lua.new_usertype<qtplugin::PluginError>("PluginError",
        sol::constructors<qtplugin::PluginError(), qtplugin::PluginError(qtplugin::PluginErrorCode, const std::string&)>()
    );

    error_type["code"] = &qtplugin::PluginError::code;
    error_type["message"] = &qtplugin::PluginError::message;
    error_type["to_string"] = &qtplugin::PluginError::to_string;
}

/**
 * @brief Register core bindings
 */
void register_core_bindings(sol::state& lua) {
    // Get or create qtforge.core namespace
    sol::table qtforge = lua["qtforge"];
    sol::table core = lua.create_table();
    qtforge["core"] = core;

    // Register core types
    register_version_bindings(lua);
    register_plugin_state_bindings(lua);
    register_error_handling_bindings(lua);
    register_advanced_interfaces_bindings(lua);

    // Add convenience functions to core namespace
    core["version"] = [](int major, int minor, int patch) {
        return qtplugin::Version{major, minor, patch};
    };

    core["create_error"] = [](qtplugin::PluginErrorCode code, const std::string& message) {
        return qtplugin::PluginError{code, message};
    };

    // Utility functions
    core["state_to_string"] = [](qtplugin::PluginState state) -> std::string {
        switch (state) {
            case qtplugin::PluginState::Unloaded: return "Unloaded";
            case qtplugin::PluginState::Loading: return "Loading";
            case qtplugin::PluginState::Loaded: return "Loaded";
            case qtplugin::PluginState::Initializing: return "Initializing";
            case qtplugin::PluginState::Running: return "Running";
            case qtplugin::PluginState::Stopping: return "Stopping";
            case qtplugin::PluginState::Error: return "Error";
            default: return "Unknown";
        }
    };

    core["capability_to_string"] = [](qtplugin::PluginCapability capability) -> std::string {
        switch (capability) {
            case qtplugin::PluginCapability::None: return "None";
            case qtplugin::PluginCapability::DataProcessing: return "DataProcessing";
            case qtplugin::PluginCapability::UserInterface: return "UserInterface";
            case qtplugin::PluginCapability::NetworkAccess: return "NetworkAccess";
            case qtplugin::PluginCapability::FileSystemAccess: return "FileSystemAccess";
            case qtplugin::PluginCapability::DatabaseAccess: return "DatabaseAccess";
            case qtplugin::PluginCapability::Scripting: return "Scripting";
            case qtplugin::PluginCapability::Configuration: return "Configuration";
            case qtplugin::PluginCapability::Logging: return "Logging";
            case qtplugin::PluginCapability::Security: return "Security";
            case qtplugin::PluginCapability::Monitoring: return "Monitoring";
            case qtplugin::PluginCapability::Communication: return "Communication";
            default: return "Unknown";
        }
    };

    // Test function
    core["test_function"] = []() -> std::string {
        return "QtForge Lua core bindings are working!";
    };
}

#else // QTFORGE_LUA_BINDINGS not defined

// Forward declare sol::state for when Lua is not available
namespace sol { class state; }

void register_core_bindings(sol::state& lua) {
    (void)lua; // Suppress unused parameter warning
    // No-op when Lua bindings are not available
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
