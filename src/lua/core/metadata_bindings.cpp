/**
 * @file metadata_bindings.cpp
 * @brief Plugin metadata and lifecycle bindings for Lua
 * @version 3.2.0
 */

#include <QDebug>
#include <QLoggingCategory>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include "../../../include/qtplugin/core/plugin_interface.hpp"
#include "../../../include/qtplugin/utils/version.hpp"
#include "../qt_conversions.cpp"

Q_LOGGING_CATEGORY(metadataBindingsLog, "qtforge.lua.metadata");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register PluginMetadata with comprehensive bindings
 */
void register_plugin_metadata_bindings(sol::state& lua) {
    auto metadata_type = lua.new_usertype<qtplugin::PluginMetadata>("PluginMetadata",
        sol::constructors<qtplugin::PluginMetadata>()
    );
    
    // Basic properties
    metadata_type["name"] = &qtplugin::PluginMetadata::name;
    metadata_type["description"] = &qtplugin::PluginMetadata::description;
    metadata_type["version"] = &qtplugin::PluginMetadata::version;
    metadata_type["author"] = &qtplugin::PluginMetadata::author;
    metadata_type["license"] = &qtplugin::PluginMetadata::license;
    metadata_type["homepage"] = &qtplugin::PluginMetadata::homepage;
    metadata_type["category"] = &qtplugin::PluginMetadata::category;
    metadata_type["capabilities"] = &qtplugin::PluginMetadata::capabilities;
    metadata_type["priority"] = &qtplugin::PluginMetadata::priority;
    
    // Tags (vector<string>)
    metadata_type["tags"] = sol::property(
        [&lua](const qtplugin::PluginMetadata& metadata) -> sol::object {
            sol::table table = lua.create_table();
            for (size_t i = 0; i < metadata.tags.size(); ++i) {
                table[i + 1] = metadata.tags[i];
            }
            return table;
        },
        [](qtplugin::PluginMetadata& metadata, const sol::table& tags) {
            metadata.tags.clear();
            for (const auto& pair : tags) {
                if (pair.second.get_type() == sol::type::string) {
                    metadata.tags.push_back(pair.second.as<std::string>());
                }
            }
        }
    );
    
    // Dependencies (vector<string>)
    metadata_type["dependencies"] = sol::property(
        [&lua](const qtplugin::PluginMetadata& metadata) -> sol::object {
            sol::table table = lua.create_table();
            for (size_t i = 0; i < metadata.dependencies.size(); ++i) {
                table[i + 1] = metadata.dependencies[i];
            }
            return table;
        },
        [](qtplugin::PluginMetadata& metadata, const sol::table& deps) {
            metadata.dependencies.clear();
            for (const auto& pair : deps) {
                if (pair.second.get_type() == sol::type::string) {
                    metadata.dependencies.push_back(pair.second.as<std::string>());
                }
            }
        }
    );
    
    // Version constraints
    metadata_type["min_host_version"] = &qtplugin::PluginMetadata::min_host_version;
    metadata_type["max_host_version"] = &qtplugin::PluginMetadata::max_host_version;
    
    // Custom data (QJsonObject)
    metadata_type["custom_data"] = sol::property(
        [&lua](const qtplugin::PluginMetadata& metadata) -> sol::object {
            return qtforge_lua::qjson_to_lua(metadata.custom_data, lua);
        },
        [](qtplugin::PluginMetadata& metadata, const sol::object& data) {
            if (data.get_type() == sol::type::table) {
                QJsonValue json_value = qtforge_lua::lua_to_qjson(data);
                if (json_value.isObject()) {
                    metadata.custom_data = json_value.toObject();
                }
            }
        }
    );
    
    // Utility methods
    metadata_type["to_json"] = [&lua](const qtplugin::PluginMetadata& metadata) -> sol::object {
        return qtforge_lua::qjson_to_lua(metadata.to_json(), lua);
    };
    
    metadata_type["from_json"] = [](qtplugin::PluginMetadata& metadata, const sol::object& json) {
        if (json.get_type() == sol::type::table) {
            QJsonValue json_value = qtforge_lua::lua_to_qjson(json);
            if (json_value.isObject()) {
                metadata.from_json(json_value.toObject());
            }
        }
    };
    
    metadata_type["is_valid"] = &qtplugin::PluginMetadata::is_valid;
    
    metadata_type["validate"] = [&lua](const qtplugin::PluginMetadata& metadata) -> sol::object {
        auto result = metadata.validate();
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };
    
    qCDebug(metadataBindingsLog) << "PluginMetadata bindings registered";
}

/**
 * @brief Register plugin lifecycle management functions
 */
void register_lifecycle_bindings(sol::state& lua) {
    sol::table qtforge = lua["qtforge"];
    sol::table lifecycle = qtforge.get_or_create<sol::table>("lifecycle");
    
    // Plugin state utilities
    lifecycle["state_to_string"] = [](qtplugin::PluginState state) -> std::string {
        switch (state) {
            case qtplugin::PluginState::Unloaded: return "Unloaded";
            case qtplugin::PluginState::Loading: return "Loading";
            case qtplugin::PluginState::Loaded: return "Loaded";
            case qtplugin::PluginState::Initializing: return "Initializing";
            case qtplugin::PluginState::Running: return "Running";
            case qtplugin::PluginState::Paused: return "Paused";
            case qtplugin::PluginState::Stopping: return "Stopping";
            case qtplugin::PluginState::Stopped: return "Stopped";
            case qtplugin::PluginState::Error: return "Error";
            case qtplugin::PluginState::Reloading: return "Reloading";
            default: return "Unknown";
        }
    };
    
    lifecycle["string_to_state"] = [](const std::string& state_str) -> qtplugin::PluginState {
        if (state_str == "Unloaded") return qtplugin::PluginState::Unloaded;
        if (state_str == "Loading") return qtplugin::PluginState::Loading;
        if (state_str == "Loaded") return qtplugin::PluginState::Loaded;
        if (state_str == "Initializing") return qtplugin::PluginState::Initializing;
        if (state_str == "Running") return qtplugin::PluginState::Running;
        if (state_str == "Paused") return qtplugin::PluginState::Paused;
        if (state_str == "Stopping") return qtplugin::PluginState::Stopping;
        if (state_str == "Stopped") return qtplugin::PluginState::Stopped;
        if (state_str == "Error") return qtplugin::PluginState::Error;
        if (state_str == "Reloading") return qtplugin::PluginState::Reloading;
        return qtplugin::PluginState::Unloaded;
    };
    
    // Plugin capability utilities
    lifecycle["capability_to_string"] = [](qtplugin::PluginCapability capability) -> std::string {
        switch (capability) {
            case qtplugin::PluginCapability::None: return "None";
            case qtplugin::PluginCapability::UI: return "UI";
            case qtplugin::PluginCapability::Service: return "Service";
            case qtplugin::PluginCapability::Network: return "Network";
            case qtplugin::PluginCapability::DataProcessing: return "DataProcessing";
            case qtplugin::PluginCapability::Scripting: return "Scripting";
            case qtplugin::PluginCapability::FileSystem: return "FileSystem";
            case qtplugin::PluginCapability::Database: return "Database";
            case qtplugin::PluginCapability::AsyncInit: return "AsyncInit";
            case qtplugin::PluginCapability::HotReload: return "HotReload";
            case qtplugin::PluginCapability::Configuration: return "Configuration";
            case qtplugin::PluginCapability::Logging: return "Logging";
            case qtplugin::PluginCapability::Security: return "Security";
            case qtplugin::PluginCapability::Threading: return "Threading";
            case qtplugin::PluginCapability::Monitoring: return "Monitoring";
            default: return "Unknown";
        }
    };
    
    lifecycle["has_capability"] = [](qtplugin::PluginCapabilities capabilities, qtplugin::PluginCapability capability) -> bool {
        return (capabilities & static_cast<qtplugin::PluginCapabilities>(capability)) != 0;
    };
    
    lifecycle["add_capability"] = [](qtplugin::PluginCapabilities& capabilities, qtplugin::PluginCapability capability) {
        capabilities |= static_cast<qtplugin::PluginCapabilities>(capability);
    };
    
    lifecycle["remove_capability"] = [](qtplugin::PluginCapabilities& capabilities, qtplugin::PluginCapability capability) {
        capabilities &= ~static_cast<qtplugin::PluginCapabilities>(capability);
    };
    
    // Plugin priority utilities
    lifecycle["priority_to_string"] = [](qtplugin::PluginPriority priority) -> std::string {
        switch (priority) {
            case qtplugin::PluginPriority::Lowest: return "Lowest";
            case qtplugin::PluginPriority::Low: return "Low";
            case qtplugin::PluginPriority::Normal: return "Normal";
            case qtplugin::PluginPriority::High: return "High";
            case qtplugin::PluginPriority::Highest: return "Highest";
            case qtplugin::PluginPriority::Critical: return "Critical";
            default: return "Normal";
        }
    };
    
    lifecycle["priority_to_int"] = [](qtplugin::PluginPriority priority) -> int {
        return static_cast<int>(priority);
    };
    
    // Metadata creation helpers
    lifecycle["create_metadata"] = [](const std::string& name, const std::string& description, 
                                     int major, int minor, int patch) -> qtplugin::PluginMetadata {
        qtplugin::PluginMetadata metadata;
        metadata.name = name;
        metadata.description = description;
        metadata.version = qtplugin::Version{major, minor, patch};
        return metadata;
    };
    
    lifecycle["create_basic_metadata"] = [](const std::string& name) -> qtplugin::PluginMetadata {
        qtplugin::PluginMetadata metadata;
        metadata.name = name;
        metadata.description = "Lua Plugin";
        metadata.version = qtplugin::Version{1, 0, 0};
        metadata.author = "Unknown";
        metadata.capabilities = static_cast<qtplugin::PluginCapabilities>(qtplugin::PluginCapability::Scripting);
        return metadata;
    };
    
    qCDebug(metadataBindingsLog) << "Lifecycle bindings registered";
}

/**
 * @brief Register all metadata and lifecycle bindings
 */
void register_metadata_lifecycle_bindings(sol::state& lua) {
    qCDebug(metadataBindingsLog) << "Registering metadata and lifecycle bindings...";
    
    register_plugin_metadata_bindings(lua);
    register_lifecycle_bindings(lua);
    
    qCDebug(metadataBindingsLog) << "Metadata and lifecycle bindings registered successfully";
}

#else // QTFORGE_LUA_BINDINGS not defined

// Forward declare sol::state for when Lua is not available
namespace sol { class state; }

void register_metadata_lifecycle_bindings(sol::state& lua) {
    (void)lua; // Suppress unused parameter warning
    qCWarning(metadataBindingsLog) << "Metadata bindings not available - Lua support not compiled";
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
