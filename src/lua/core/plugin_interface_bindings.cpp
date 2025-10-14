/**
 * @file plugin_interface_bindings.cpp
 * @brief Lua bindings for IPlugin interface and plugin lifecycle management
 * @version 1.0.0
 * @author QtForge Development Team
 */

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// Sol2 includes
#include <sol/sol.hpp>

// QtForge includes
#include "../qt_conversions.hpp"
#include "../qtforge_lua_bindings.hpp"
#include "qtplugin/interfaces/core/plugin_interface.hpp"
#include "qtplugin/utils/error_handling.hpp"
#include "qtplugin/utils/version.hpp"

Q_LOGGING_CATEGORY(pluginInterfaceBindingsLog,
                   "qtforge.lua.bindings.plugin_interface")

namespace qtforge_lua {

/**
 * @brief Helper function to convert QJsonObject to Lua table manually
 */
sol::object qjsonobject_to_lua_simple(const QJsonObject& obj,
                                      sol::state_view lua) {
    sol::table result = lua.create_table();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        const QString& key = it.key();
        const QJsonValue& value = it.value();

        switch (value.type()) {
            case QJsonValue::Null:
                result[key.toStdString()] = sol::nil;
                break;
            case QJsonValue::Bool:
                result[key.toStdString()] = value.toBool();
                break;
            case QJsonValue::Double:
                result[key.toStdString()] = value.toDouble();
                break;
            case QJsonValue::String:
                result[key.toStdString()] = value.toString().toStdString();
                break;
            default:
                // For complex types, just convert to string for now
                result[key.toStdString()] =
                    value.toVariant().toString().toStdString();
                break;
        }
    }
    return result;
}

/**
 * @brief Convert Version to Lua table
 */
sol::object version_to_lua(const qtplugin::Version& version,
                           sol::state_view lua) {
    sol::table result = lua.create_table();
    result["major"] = version.major();
    result["minor"] = version.minor();
    result["patch"] = version.patch();
    result["to_string"] = [&version]() { return version.to_string(); };
    return result;
}

/**
 * @brief Convert PluginError to Lua table
 */
sol::object plugin_error_to_lua(const qtplugin::PluginError& error,
                                sol::state_view lua) {
    sol::table result = lua.create_table();
    result["code"] = static_cast<int>(error.code);
    result["message"] = error.message;
    result["details"] = error.details;
    result["context"] = error.context;
    return result;
}

/**
 * @brief Convert PluginCapability enum to Lua
 */
sol::object plugin_capability_to_lua(qtplugin::PluginCapability capability,
                                     sol::state_view lua) {
    sol::table result = lua.create_table();
    result["value"] = static_cast<uint32_t>(capability);

    // Add string representation
    switch (capability) {
        case qtplugin::PluginCapability::None:
            result["name"] = "None";
            break;
        case qtplugin::PluginCapability::UI:
            result["name"] = "UI";
            break;
        case qtplugin::PluginCapability::Service:
            result["name"] = "Service";
            break;
        case qtplugin::PluginCapability::Network:
            result["name"] = "Network";
            break;
        case qtplugin::PluginCapability::DataProcessing:
            result["name"] = "DataProcessing";
            break;
        case qtplugin::PluginCapability::Scripting:
            result["name"] = "Scripting";
            break;
        case qtplugin::PluginCapability::FileSystem:
            result["name"] = "FileSystem";
            break;
        case qtplugin::PluginCapability::Database:
            result["name"] = "Database";
            break;
        case qtplugin::PluginCapability::AsyncInit:
            result["name"] = "AsyncInit";
            break;
        case qtplugin::PluginCapability::HotReload:
            result["name"] = "HotReload";
            break;
        case qtplugin::PluginCapability::Configuration:
            result["name"] = "Configuration";
            break;
        case qtplugin::PluginCapability::Logging:
            result["name"] = "Logging";
            break;
        case qtplugin::PluginCapability::Security:
            result["name"] = "Security";
            break;
        case qtplugin::PluginCapability::Threading:
            result["name"] = "Threading";
            break;
        case qtplugin::PluginCapability::Monitoring:
            result["name"] = "Monitoring";
            break;
        default:
            result["name"] = "Unknown";
            break;
    }

    return result;
}

/**
 * @brief Convert PluginState enum to Lua
 */
sol::object plugin_state_to_lua(qtplugin::PluginState state,
                                sol::state_view lua) {
    sol::table result = lua.create_table();
    result["value"] = static_cast<int>(state);

    // Add string representation
    switch (state) {
        case qtplugin::PluginState::Unloaded:
            result["name"] = "Unloaded";
            break;
        case qtplugin::PluginState::Loading:
            result["name"] = "Loading";
            break;
        case qtplugin::PluginState::Loaded:
            result["name"] = "Loaded";
            break;
        case qtplugin::PluginState::Initializing:
            result["name"] = "Initializing";
            break;
        case qtplugin::PluginState::Running:
            result["name"] = "Running";
            break;
        case qtplugin::PluginState::Paused:
            result["name"] = "Paused";
            break;
        case qtplugin::PluginState::Stopping:
            result["name"] = "Stopping";
            break;
        case qtplugin::PluginState::Stopped:
            result["name"] = "Stopped";
            break;
        case qtplugin::PluginState::Error:
            result["name"] = "Error";
            break;
        case qtplugin::PluginState::Reloading:
            result["name"] = "Reloading";
            break;
        default:
            result["name"] = "Unknown";
            break;
    }

    return result;
}

/**
 * @brief Convert PluginPriority enum to Lua
 */
sol::object plugin_priority_to_lua(qtplugin::PluginPriority priority,
                                   sol::state_view lua) {
    sol::table result = lua.create_table();
    result["value"] = static_cast<int>(priority);

    // Add string representation
    switch (priority) {
        case qtplugin::PluginPriority::Lowest:
            result["name"] = "Lowest";
            break;
        case qtplugin::PluginPriority::Low:
            result["name"] = "Low";
            break;
        case qtplugin::PluginPriority::Normal:
            result["name"] = "Normal";
            break;
        case qtplugin::PluginPriority::High:
            result["name"] = "High";
            break;
        case qtplugin::PluginPriority::Highest:
            result["name"] = "Highest";
            break;
        case qtplugin::PluginPriority::Critical:
            result["name"] = "Critical";
            break;
        default:
            result["name"] = "Unknown";
            break;
    }

    return result;
}

/**
 * @brief Convert PluginMetadata to Lua table
 */
sol::object plugin_metadata_to_lua(const qtplugin::PluginMetadata& metadata,
                                   sol::state_view lua) {
    sol::table result = lua.create_table();

    result["name"] = metadata.name;
    result["description"] = metadata.description;
    result["version"] = version_to_lua(metadata.version, lua);
    result["author"] = metadata.author;
    result["license"] = metadata.license;
    result["homepage"] = metadata.homepage;
    result["category"] = metadata.category;

    // Convert tags
    sol::table tags = lua.create_table();
    for (size_t i = 0; i < metadata.tags.size(); ++i) {
        tags[i + 1] = metadata.tags[i];
    }
    result["tags"] = tags;

    // Convert dependencies
    sol::table deps = lua.create_table();
    for (size_t i = 0; i < metadata.dependencies.size(); ++i) {
        deps[i + 1] = metadata.dependencies[i];
    }
    result["dependencies"] = deps;

    result["capabilities"] = metadata.capabilities;
    result["priority"] = plugin_priority_to_lua(metadata.priority, lua);

    if (metadata.min_host_version) {
        result["min_host_version"] =
            version_to_lua(*metadata.min_host_version, lua);
    }

    if (metadata.max_host_version) {
        result["max_host_version"] =
            version_to_lua(*metadata.max_host_version, lua);
    }

    result["custom_data"] =
        qjsonobject_to_lua_simple(metadata.custom_data, lua);

    return result;
}

/**
 * @brief Register PluginCapability enum bindings
 */
void register_plugin_capability_bindings(sol::state& lua) {
    qCDebug(pluginInterfaceBindingsLog)
        << "Registering PluginCapability bindings...";

    sol::table qtforge = lua["qtforge"].get_or_create<sol::table>();

    // Create PluginCapability enum table
    sol::table capability_enum = lua.create_table();
    capability_enum["None"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::None);
    capability_enum["UI"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::UI);
    capability_enum["Service"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::Service);
    capability_enum["Network"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::Network);
    capability_enum["DataProcessing"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::DataProcessing);
    capability_enum["Scripting"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::Scripting);
    capability_enum["FileSystem"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::FileSystem);
    capability_enum["Database"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::Database);
    capability_enum["AsyncInit"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::AsyncInit);
    capability_enum["HotReload"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::HotReload);
    capability_enum["Configuration"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::Configuration);
    capability_enum["Logging"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::Logging);
    capability_enum["Security"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::Security);
    capability_enum["Threading"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::Threading);
    capability_enum["Monitoring"] =
        static_cast<uint32_t>(qtplugin::PluginCapability::Monitoring);

    qtforge["PluginCapability"] = capability_enum;
}

/**
 * @brief Register PluginState enum bindings
 */
void register_plugin_state_bindings(sol::state& lua) {
    qCDebug(pluginInterfaceBindingsLog)
        << "Registering PluginState bindings...";

    sol::table qtforge = lua["qtforge"].get_or_create<sol::table>();

    // Create PluginState enum table
    sol::table state_enum = lua.create_table();
    state_enum["Unloaded"] = static_cast<int>(qtplugin::PluginState::Unloaded);
    state_enum["Loading"] = static_cast<int>(qtplugin::PluginState::Loading);
    state_enum["Loaded"] = static_cast<int>(qtplugin::PluginState::Loaded);
    state_enum["Initializing"] =
        static_cast<int>(qtplugin::PluginState::Initializing);
    state_enum["Running"] = static_cast<int>(qtplugin::PluginState::Running);
    state_enum["Paused"] = static_cast<int>(qtplugin::PluginState::Paused);
    state_enum["Stopping"] = static_cast<int>(qtplugin::PluginState::Stopping);
    state_enum["Stopped"] = static_cast<int>(qtplugin::PluginState::Stopped);
    state_enum["Error"] = static_cast<int>(qtplugin::PluginState::Error);
    state_enum["Reloading"] =
        static_cast<int>(qtplugin::PluginState::Reloading);

    qtforge["PluginState"] = state_enum;
}

/**
 * @brief Register PluginPriority enum bindings
 */
void register_plugin_priority_bindings(sol::state& lua) {
    qCDebug(pluginInterfaceBindingsLog)
        << "Registering PluginPriority bindings...";

    sol::table qtforge = lua["qtforge"].get_or_create<sol::table>();

    // Create PluginPriority enum table
    sol::table priority_enum = lua.create_table();
    priority_enum["Lowest"] =
        static_cast<int>(qtplugin::PluginPriority::Lowest);
    priority_enum["Low"] = static_cast<int>(qtplugin::PluginPriority::Low);
    priority_enum["Normal"] =
        static_cast<int>(qtplugin::PluginPriority::Normal);
    priority_enum["High"] = static_cast<int>(qtplugin::PluginPriority::High);
    priority_enum["Highest"] =
        static_cast<int>(qtplugin::PluginPriority::Highest);
    priority_enum["Critical"] =
        static_cast<int>(qtplugin::PluginPriority::Critical);

    qtforge["PluginPriority"] = priority_enum;
}

/**
 * @brief Register PluginMetadata bindings
 */
void register_plugin_metadata_bindings(sol::state& lua) {
    qCDebug(pluginInterfaceBindingsLog)
        << "Registering PluginMetadata bindings...";

    // Register PluginMetadata usertype (no constructor due to complexity)
    auto metadata_type = lua.new_usertype<qtplugin::PluginMetadata>(
        "PluginMetadata", sol::no_constructor);

    // Read-only properties
    metadata_type["name"] = sol::readonly(&qtplugin::PluginMetadata::name);
    metadata_type["description"] =
        sol::readonly(&qtplugin::PluginMetadata::description);
    metadata_type["author"] = sol::readonly(&qtplugin::PluginMetadata::author);
    metadata_type["license"] =
        sol::readonly(&qtplugin::PluginMetadata::license);
    metadata_type["homepage"] =
        sol::readonly(&qtplugin::PluginMetadata::homepage);
    metadata_type["category"] =
        sol::readonly(&qtplugin::PluginMetadata::category);
    metadata_type["capabilities"] =
        sol::readonly(&qtplugin::PluginMetadata::capabilities);

    // Methods
    metadata_type["to_json"] = &qtplugin::PluginMetadata::to_json;

    // Conversion function
    metadata_type["to_lua"] = [](const qtplugin::PluginMetadata& metadata,
                                 sol::this_state s) {
        sol::state_view lua(s);
        return plugin_metadata_to_lua(metadata, lua);
    };
}

/**
 * @brief Register IPlugin interface bindings
 */
void register_iplugin_bindings(sol::state& lua) {
    qCDebug(pluginInterfaceBindingsLog)
        << "Registering IPlugin interface bindings...";

    // Register IPlugin usertype (no constructor - interface only)
    auto plugin_type =
        lua.new_usertype<qtplugin::IPlugin>("IPlugin", sol::no_constructor);

    // === Metadata methods ===
    plugin_type["name"] = [](const qtplugin::IPlugin& plugin) {
        return std::string(plugin.name());
    };

    plugin_type["description"] = [](const qtplugin::IPlugin& plugin) {
        return std::string(plugin.description());
    };

    plugin_type["version"] = [](const qtplugin::IPlugin& plugin,
                                sol::this_state s) {
        sol::state_view lua(s);
        return version_to_lua(plugin.version(), lua);
    };

    plugin_type["author"] = [](const qtplugin::IPlugin& plugin) {
        return std::string(plugin.author());
    };

    plugin_type["id"] = &qtplugin::IPlugin::id;
    plugin_type["uuid"] = &qtplugin::IPlugin::uuid;

    plugin_type["category"] = [](const qtplugin::IPlugin& plugin) {
        return std::string(plugin.category());
    };

    plugin_type["license"] = [](const qtplugin::IPlugin& plugin) {
        return std::string(plugin.license());
    };

    plugin_type["homepage"] = [](const qtplugin::IPlugin& plugin) {
        return std::string(plugin.homepage());
    };

    plugin_type["metadata"] = [](const qtplugin::IPlugin& plugin,
                                 sol::this_state s) {
        sol::state_view lua(s);
        return plugin_metadata_to_lua(plugin.metadata(), lua);
    };

    // === Lifecycle methods ===
    plugin_type["initialize"] = [](qtplugin::IPlugin& plugin,
                                   sol::this_state s) -> sol::object {
        auto result = plugin.initialize();
        sol::state_view lua(s);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return plugin_error_to_lua(result.error(), lua);
        }
    };

    plugin_type["shutdown"] = &qtplugin::IPlugin::shutdown;

    plugin_type["state"] = [](const qtplugin::IPlugin& plugin,
                              sol::this_state s) {
        sol::state_view lua(s);
        return plugin_state_to_lua(plugin.state(), lua);
    };

    plugin_type["is_initialized"] = &qtplugin::IPlugin::is_initialized;

    plugin_type["pause"] = [](qtplugin::IPlugin& plugin,
                              sol::this_state s) -> sol::object {
        auto result = plugin.pause();
        sol::state_view lua(s);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return plugin_error_to_lua(result.error(), lua);
        }
    };

    plugin_type["resume"] = [](qtplugin::IPlugin& plugin,
                               sol::this_state s) -> sol::object {
        auto result = plugin.resume();
        sol::state_view lua(s);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return plugin_error_to_lua(result.error(), lua);
        }
    };

    plugin_type["restart"] = [](qtplugin::IPlugin& plugin,
                                sol::this_state s) -> sol::object {
        auto result = plugin.restart();
        sol::state_view lua(s);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return plugin_error_to_lua(result.error(), lua);
        }
    };

    // === Capabilities ===
    plugin_type["capabilities"] = &qtplugin::IPlugin::capabilities;

    plugin_type["has_capability"] = [](const qtplugin::IPlugin& plugin,
                                       uint32_t capability) {
        return plugin.has_capability(
            static_cast<qtplugin::PluginCapability>(capability));
    };

    plugin_type["priority"] = [](const qtplugin::IPlugin& plugin,
                                 sol::this_state s) {
        sol::state_view lua(s);
        return plugin_priority_to_lua(plugin.priority(), lua);
    };

    // === Configuration ===
    plugin_type["default_configuration"] =
        [](const qtplugin::IPlugin& plugin, sol::this_state s) -> sol::object {
        auto config = plugin.default_configuration();
        if (config) {
            sol::state_view lua(s);
            return qjsonobject_to_lua_simple(*config, lua);
        } else {
            return sol::nil;
        }
    };

    plugin_type["configure"] = [](qtplugin::IPlugin& plugin,
                                  const QJsonObject& config,
                                  sol::this_state s) -> sol::object {
        auto result = plugin.configure(config);
        sol::state_view lua(s);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return plugin_error_to_lua(result.error(), lua);
        }
    };

    plugin_type["current_configuration"] = [](const qtplugin::IPlugin& plugin,
                                              sol::this_state s) {
        sol::state_view lua(s);
        return qjsonobject_to_lua_simple(plugin.current_configuration(), lua);
    };

    plugin_type["validate_configuration"] =
        &qtplugin::IPlugin::validate_configuration;
}

/**
 * @brief Main registration function for plugin interface bindings
 */
void register_plugin_interface_core_bindings_main(sol::state& lua) {
    qCDebug(pluginInterfaceBindingsLog)
        << "Registering plugin interface core bindings...";

    // Create main qtforge namespace
    sol::table qtforge = lua["qtforge"].get_or_create<sol::table>();

    // Register enum bindings
    register_plugin_capability_bindings(lua);
    register_plugin_state_bindings(lua);
    register_plugin_priority_bindings(lua);

    // Register structure bindings
    register_plugin_metadata_bindings(lua);

    // Register interface bindings
    register_iplugin_bindings(lua);

    qCDebug(pluginInterfaceBindingsLog)
        << "Plugin interface core bindings registration complete";
}

}  // namespace qtforge_lua
