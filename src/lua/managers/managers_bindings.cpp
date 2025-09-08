/**
 * @file managers_bindings.cpp
 * @brief Plugin manager bindings for Lua
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

#include <qtplugin/core/plugin_manager.hpp>
#include <qtplugin/core/plugin_interface.hpp>
#include <qtplugin/managers/configuration_manager.hpp>
#include <qtplugin/managers/logging_manager.hpp>
#include <qtplugin/managers/resource_manager.hpp>
#include <qtplugin/managers/plugin_version_manager.hpp>
#include "../qt_conversions.cpp"

Q_LOGGING_CATEGORY(managersBindingsLog, "qtforge.lua.managers");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register PluginLoadOptions with Lua
 */
void register_plugin_load_options_bindings(sol::state& lua) {
    auto options_type = lua.new_usertype<qtplugin::PluginLoadOptions>("PluginLoadOptions",
        sol::constructors<qtplugin::PluginLoadOptions>()
    );

    options_type["initialize_immediately"] = &qtplugin::PluginLoadOptions::initialize_immediately;
    options_type["check_dependencies"] = &qtplugin::PluginLoadOptions::check_dependencies;
    options_type["validate_signature"] = &qtplugin::PluginLoadOptions::validate_signature;
    options_type["enable_hot_reload"] = &qtplugin::PluginLoadOptions::enable_hot_reload;
    options_type["security_level"] = &qtplugin::PluginLoadOptions::security_level;

    // Configuration (QJsonObject)
    options_type["configuration"] = sol::property(
        [&lua](const qtplugin::PluginLoadOptions& options) -> sol::object {
            return qtforge_lua::qjson_to_lua(options.configuration, lua);
        },
        [](qtplugin::PluginLoadOptions& options, const sol::object& config) {
            if (config.get_type() == sol::type::table) {
                QJsonValue json_value = qtforge_lua::lua_to_qjson(config);
                if (json_value.isObject()) {
                    options.configuration = json_value.toObject();
                }
            }
        }
    );

    qCDebug(managersBindingsLog) << "PluginLoadOptions bindings registered";
}

/**
 * @brief Register PluginInfo with Lua
 */
void register_plugin_info_bindings(sol::state& lua) {
    auto info_type = lua.new_usertype<qtplugin::PluginInfo>("PluginInfo");

    info_type["id"] = sol::readonly(&qtplugin::PluginInfo::id);
    info_type["file_path"] = sol::property(
        [](const qtplugin::PluginInfo& info) -> std::string {
            return info.file_path.string();
        }
    );
    info_type["metadata"] = sol::readonly(&qtplugin::PluginInfo::metadata);
    info_type["state"] = sol::readonly(&qtplugin::PluginInfo::state);
    info_type["hot_reload_enabled"] = sol::readonly(&qtplugin::PluginInfo::hot_reload_enabled);

    // Load time
    info_type["load_time"] = sol::property(
        [](const qtplugin::PluginInfo& info) -> double {
            auto duration = info.load_time.time_since_epoch();
            return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
        }
    );

    // Last activity
    info_type["last_activity"] = sol::property(
        [](const qtplugin::PluginInfo& info) -> double {
            auto duration = info.last_activity.time_since_epoch();
            return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
        }
    );

    // Configuration (QJsonObject)
    info_type["configuration"] = sol::property(
        [&lua](const qtplugin::PluginInfo& info) -> sol::object {
            return qtforge_lua::qjson_to_lua(info.configuration, lua);
        }
    );

    // Metrics (QJsonObject)
    info_type["metrics"] = sol::property(
        [&lua](const qtplugin::PluginInfo& info) -> sol::object {
            return qtforge_lua::qjson_to_lua(info.metrics, lua);
        }
    );

    // Error log
    info_type["error_log"] = sol::property(
        [&lua](const qtplugin::PluginInfo& info) -> sol::object {
            sol::table table = lua.create_table();
            for (size_t i = 0; i < info.error_log.size(); ++i) {
                table[i + 1] = info.error_log[i];
            }
            return table;
        }
    );

    // Convert to JSON
    info_type["to_json"] = [&lua](const qtplugin::PluginInfo& info) -> sol::object {
        return qtforge_lua::qjson_to_lua(info.to_json(), lua);
    };

    qCDebug(managersBindingsLog) << "PluginInfo bindings registered";
}

/**
 * @brief Register PluginManager with Lua
 */
void register_plugin_manager_bindings(sol::state& lua) {
    auto manager_type = lua.new_usertype<qtplugin::PluginManager>("PluginManager");

    // === Plugin Loading ===
    manager_type["load_plugin"] = [&lua](qtplugin::PluginManager& manager,
                                         const std::string& file_path,
                                         const qtplugin::PluginLoadOptions& options) -> sol::object {
        std::filesystem::path path(file_path);
        auto result = manager.load_plugin(path, options);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    manager_type["load_plugin_simple"] = [&lua](qtplugin::PluginManager& manager,
                                                const std::string& file_path) -> sol::object {
        std::filesystem::path path(file_path);
        qtplugin::PluginLoadOptions options;
        auto result = manager.load_plugin(path, options);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    manager_type["unload_plugin"] = [&lua](qtplugin::PluginManager& manager,
                                           const std::string& plugin_id,
                                           bool force) -> sol::object {
        auto result = manager.unload_plugin(plugin_id, force);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    manager_type["reload_plugin"] = [&lua](qtplugin::PluginManager& manager,
                                           const std::string& plugin_id,
                                           bool preserve_state) -> sol::object {
        auto result = manager.reload_plugin(plugin_id, preserve_state);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    // === Plugin Querying ===
    manager_type["get_plugin"] = [](qtplugin::PluginManager& manager,
                                   const std::string& plugin_id) -> std::shared_ptr<qtplugin::IPlugin> {
        return manager.get_plugin(plugin_id);
    };

    manager_type["has_plugin"] = &qtplugin::PluginManager::has_plugin;

    manager_type["get_plugin_info"] = [](qtplugin::PluginManager& manager,
                                        const std::string& plugin_id) -> std::optional<qtplugin::PluginInfo> {
        return manager.get_plugin_info(plugin_id);
    };

    manager_type["get_all_plugins"] = [&lua](qtplugin::PluginManager& manager) -> sol::object {
        auto plugins = manager.get_all_plugins();
        sol::table table = lua.create_table();
        size_t index = 1;
        for (const auto& [id, plugin] : plugins) {
            sol::table entry = lua.create_table();
            entry["id"] = id;
            entry["plugin"] = plugin;
            table[index++] = entry;
        }
        return table;
    };

    manager_type["get_plugin_ids"] = [&lua](qtplugin::PluginManager& manager) -> sol::object {
        auto ids = manager.get_plugin_ids();
        sol::table table = lua.create_table();
        for (size_t i = 0; i < ids.size(); ++i) {
            table[i + 1] = ids[i];
        }
        return table;
    };

    // === Plugin Discovery ===
    manager_type["discover_plugins"] = [&lua](qtplugin::PluginManager& manager,
                                              const std::string& directory,
                                              bool recursive) -> sol::object {
        std::filesystem::path dir(directory);
        auto plugins = manager.discover_plugins(dir, recursive);
        sol::table table = lua.create_table();
        for (size_t i = 0; i < plugins.size(); ++i) {
            table[i + 1] = plugins[i].string();
        }
        return table;
    };

    manager_type["load_plugins_from_directory"] = [&lua](qtplugin::PluginManager& manager,
                                                         const std::string& directory,
                                                         const qtplugin::PluginLoadOptions& options) -> sol::object {
        std::filesystem::path dir(directory);
        auto result = manager.load_plugins_from_directory(dir, options);
        return sol::make_object(lua, result);
    };

    // === Plugin Management ===
    manager_type["shutdown_all_plugins"] = &qtplugin::PluginManager::shutdown_all_plugins;

    manager_type["get_plugin_count"] = &qtplugin::PluginManager::get_plugin_count;

    // === Hot Reload ===
    manager_type["enable_hot_reload"] = &qtplugin::PluginManager::enable_hot_reload;
    manager_type["disable_hot_reload"] = &qtplugin::PluginManager::disable_hot_reload;

    // === Metrics ===
    manager_type["system_metrics"] = [&lua](qtplugin::PluginManager& manager) -> sol::object {
        return qtforge_lua::qjson_to_lua(manager.system_metrics(), lua);
    };

    qCDebug(managersBindingsLog) << "PluginManager bindings registered";
}

/**
 * @brief Register ConfigurationScope enum with Lua
 */
void register_configuration_scope_bindings(sol::state& lua) {
    lua.new_enum<qtplugin::ConfigurationScope>("ConfigurationScope", {
        {"Global", qtplugin::ConfigurationScope::Global},
        {"Plugin", qtplugin::ConfigurationScope::Plugin},
        {"User", qtplugin::ConfigurationScope::User},
        {"Session", qtplugin::ConfigurationScope::Session},
        {"Runtime", qtplugin::ConfigurationScope::Runtime}
    });

    qCDebug(managersBindingsLog) << "ConfigurationScope bindings registered";
}

/**
 * @brief Register ConfigurationManager with Lua
 */
void register_configuration_manager_bindings(sol::state& lua) {
    auto config_manager_type = lua.new_usertype<qtplugin::ConfigurationManager>("ConfigurationManager",
        sol::constructors<qtplugin::ConfigurationManager>()
    );

    // Static factory method
    config_manager_type["create"] = &qtplugin::ConfigurationManager::create;

    // Configuration access methods
    config_manager_type["get_value"] = [&lua](qtplugin::ConfigurationManager& manager,
                                              const std::string& key,
                                              qtplugin::ConfigurationScope scope,
                                              const std::string& plugin_id) -> sol::object {
        auto result = manager.get_value(key, scope, plugin_id);
        if (result) {
            return qtforge_lua::qjson_to_lua(result.value(), lua);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    config_manager_type["get_value_or_default"] = [&lua](qtplugin::ConfigurationManager& manager,
                                                          const std::string& key,
                                                          const sol::object& default_value,
                                                          qtplugin::ConfigurationScope scope,
                                                          const std::string& plugin_id) -> sol::object {
        QJsonValue default_json = qtforge_lua::lua_to_qjson(default_value);
        QJsonValue result = manager.get_value_or_default(key, default_json, scope, plugin_id);
        return qtforge_lua::qjson_to_lua(result, lua);
    };

    config_manager_type["set_value"] = [&lua](qtplugin::ConfigurationManager& manager,
                                              const std::string& key,
                                              const sol::object& value,
                                              qtplugin::ConfigurationScope scope,
                                              const std::string& plugin_id) -> sol::object {
        QJsonValue json_value = qtforge_lua::lua_to_qjson(value);
        auto result = manager.set_value(key, json_value, scope, plugin_id);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    config_manager_type["has_key"] = [](qtplugin::ConfigurationManager& manager,
                                        const std::string& key,
                                        qtplugin::ConfigurationScope scope,
                                        const std::string& plugin_id) -> bool {
        return manager.has_key(key, scope, plugin_id);
    };

    config_manager_type["remove_key"] = [&lua](qtplugin::ConfigurationManager& manager,
                                               const std::string& key,
                                               qtplugin::ConfigurationScope scope,
                                               const std::string& plugin_id) -> sol::object {
        auto result = manager.remove_key(key, scope, plugin_id);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    config_manager_type["get_configuration"] = [&lua](qtplugin::ConfigurationManager& manager,
                                                       qtplugin::ConfigurationScope scope,
                                                       const std::string& plugin_id) -> sol::object {
        auto result = manager.get_configuration(scope, plugin_id);
        if (result) {
            return qtforge_lua::qjson_to_lua(result.value(), lua);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    qCDebug(managersBindingsLog) << "ConfigurationManager bindings registered";
}

/**
 * @brief Register LoggingManager with Lua
 */
void register_logging_manager_bindings(sol::state& lua) {
    // Log level enum
    lua.new_enum<qtplugin::LogLevel>("LogLevel", {
        {"Debug", qtplugin::LogLevel::Debug},
        {"Info", qtplugin::LogLevel::Info},
        {"Warning", qtplugin::LogLevel::Warning},
        {"Error", qtplugin::LogLevel::Error},
        {"Critical", qtplugin::LogLevel::Critical}
    });

    auto logging_manager_type = lua.new_usertype<qtplugin::LoggingManager>("LoggingManager",
        sol::constructors<qtplugin::LoggingManager>()
    );

    // Static factory method
    logging_manager_type["create"] = &qtplugin::LoggingManager::create;

    // Logging methods
    logging_manager_type["log"] = &qtplugin::LoggingManager::log;
    logging_manager_type["debug"] = &qtplugin::LoggingManager::debug;
    logging_manager_type["info"] = &qtplugin::LoggingManager::info;
    logging_manager_type["warning"] = &qtplugin::LoggingManager::warning;
    logging_manager_type["error"] = &qtplugin::LoggingManager::error;
    logging_manager_type["critical"] = &qtplugin::LoggingManager::critical;

    // Configuration methods
    logging_manager_type["set_log_level"] = &qtplugin::LoggingManager::set_log_level;
    logging_manager_type["get_log_level"] = &qtplugin::LoggingManager::get_log_level;
    logging_manager_type["enable_file_logging"] = &qtplugin::LoggingManager::enable_file_logging;
    logging_manager_type["disable_file_logging"] = &qtplugin::LoggingManager::disable_file_logging;
    logging_manager_type["is_file_logging_enabled"] = &qtplugin::LoggingManager::is_file_logging_enabled;
    logging_manager_type["get_log_file_path"] = &qtplugin::LoggingManager::get_log_file_path;
    logging_manager_type["flush"] = &qtplugin::LoggingManager::flush;

    qCDebug(managersBindingsLog) << "LoggingManager bindings registered";
}

/**
 * @brief Register ResourceManager with Lua
 */
void register_resource_manager_bindings(sol::state& lua) {
    auto resource_manager_type = lua.new_usertype<qtplugin::ResourceManager>("ResourceManager",
        sol::constructors<qtplugin::ResourceManager>()
    );

    // Static factory method
    resource_manager_type["create"] = &qtplugin::ResourceManager::create;

    // Resource management methods
    resource_manager_type["allocate_resource"] = [&lua](qtplugin::ResourceManager& manager,
                                                         const std::string& resource_id,
                                                         const std::string& resource_type,
                                                         const sol::object& config) -> sol::object {
        QJsonObject json_config;
        if (config.get_type() == sol::type::table) {
            QJsonValue json_value = qtforge_lua::lua_to_qjson(config);
            if (json_value.isObject()) {
                json_config = json_value.toObject();
            }
        }

        auto result = manager.allocate_resource(resource_id, resource_type, json_config);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    resource_manager_type["deallocate_resource"] = [&lua](qtplugin::ResourceManager& manager,
                                                           const std::string& resource_id) -> sol::object {
        auto result = manager.deallocate_resource(resource_id);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    resource_manager_type["has_resource"] = &qtplugin::ResourceManager::has_resource;
    resource_manager_type["list_resources"] = &qtplugin::ResourceManager::list_resources;
    resource_manager_type["cleanup"] = &qtplugin::ResourceManager::cleanup;

    qCDebug(managersBindingsLog) << "ResourceManager bindings registered";
}

/**
 * @brief Register all manager bindings
 */
void register_managers_bindings(sol::state& lua) {
    qCDebug(managersBindingsLog) << "Registering managers bindings...";

    // Create qtforge.managers namespace
    sol::table qtforge = lua["qtforge"];
    sol::table managers = qtforge.get_or_create<sol::table>("managers");

    // Register all manager types
    register_plugin_load_options_bindings(lua);
    register_plugin_info_bindings(lua);
    register_plugin_manager_bindings(lua);
    register_configuration_scope_bindings(lua);
    register_configuration_manager_bindings(lua);
    register_logging_manager_bindings(lua);
    register_resource_manager_bindings(lua);

    // Add convenience functions to managers namespace
    managers["create_load_options"] = []() {
        return qtplugin::PluginLoadOptions{};
    };

    managers["create_default_options"] = []() {
        qtplugin::PluginLoadOptions options;
        options.initialize_immediately = true;
        options.check_dependencies = true;
        options.validate_signature = false;
        options.enable_hot_reload = false;
        return options;
    };

    // Factory functions for managers
    managers["create_configuration_manager"] = []() {
        return qtplugin::ConfigurationManager::create();
    };

    managers["create_logging_manager"] = []() {
        return qtplugin::LoggingManager::create();
    };

    managers["create_resource_manager"] = []() {
        return qtplugin::ResourceManager::create();
    };

    qCDebug(managersBindingsLog) << "Managers bindings registered successfully";
}

#else // QTFORGE_LUA_BINDINGS not defined

// Forward declare sol::state for when Lua is not available
namespace sol { class state; }

void register_managers_bindings(sol::state& lua) {
    (void)lua; // Suppress unused parameter warning
    qCWarning(managersBindingsLog) << "Managers bindings not available - Lua support not compiled";
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
