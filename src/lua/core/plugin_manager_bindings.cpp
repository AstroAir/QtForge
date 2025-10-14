/**
 * @file plugin_manager_bindings.cpp
 * @brief Comprehensive PluginManager bindings for Lua
 * @version 3.2.0
 */

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <filesystem>
#include <memory>

#include "../../../include/qtplugin/core/plugin_manager.hpp"
#include "../../../include/qtplugin/interfaces/core/plugin_interface.hpp"
#include "../../../include/qtplugin/utils/error_handling.hpp"
#include "../../../include/qtplugin/utils/version.hpp"
#include "../qt_conversions.hpp"

Q_LOGGING_CATEGORY(pluginManagerBindingsLog, "qtforge.lua.core.plugin_manager");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Convert PluginLoadOptions to Lua table
 */
sol::table plugin_load_options_to_lua(
    const qtplugin::PluginLoadOptions& options, sol::state& lua) {
    sol::table table = lua.create_table();
    table["validate_sha256"] = options.validate_sha256;
    table["expected_sha256"] = options.expected_sha256;
    table["check_dependencies"] = options.check_dependencies;
    table["initialize_immediately"] = options.initialize_immediately;
    table["enable_hot_reload"] = options.enable_hot_reload;
    table["timeout_ms"] = options.timeout.count();
    table["configuration"] = qjson_to_lua(options.configuration, lua);
    return table;
}

/**
 * @brief Convert Lua table to PluginLoadOptions
 */
qtplugin::PluginLoadOptions lua_to_plugin_load_options(
    const sol::table& table) {
    qtplugin::PluginLoadOptions options;

    if (table["validate_sha256"].valid()) {
        options.validate_sha256 = table["validate_sha256"];
    }
    if (table["expected_sha256"].valid()) {
        options.expected_sha256 = table["expected_sha256"];
    }
    if (table["check_dependencies"].valid()) {
        options.check_dependencies = table["check_dependencies"];
    }
    if (table["initialize_immediately"].valid()) {
        options.initialize_immediately = table["initialize_immediately"];
    }
    if (table["enable_hot_reload"].valid()) {
        options.enable_hot_reload = table["enable_hot_reload"];
    }
    if (table["timeout_ms"].valid()) {
        int timeout_ms = table["timeout_ms"];
        options.timeout = std::chrono::milliseconds(timeout_ms);
    }
    if (table["configuration"].valid()) {
        sol::table config_table = table["configuration"];
        options.configuration = lua_to_qjson(config_table).toObject();
    }

    return options;
}

/**
 * @brief Convert PluginInfo to Lua table
 */
sol::table plugin_info_to_lua(const qtplugin::PluginInfo& info,
                              sol::state& lua) {
    sol::table table = lua.create_table();
    table["id"] = info.id;
    table["file_path"] = info.file_path.string();
    table["state"] = static_cast<int>(info.state);
    table["hot_reload_enabled"] = info.hot_reload_enabled;
    table["configuration"] = qjson_to_lua(info.configuration, lua);
    table["metrics"] = qjson_to_lua(info.metrics, lua);

    // Convert metadata
    sol::table metadata = lua.create_table();
    metadata["name"] = info.metadata.name;
    metadata["description"] = info.metadata.description;
    metadata["author"] = info.metadata.author;
    metadata["version"] = info.metadata.version.to_string();
    metadata["capabilities"] = static_cast<int>(info.metadata.capabilities);
    table["metadata"] = metadata;

    // Convert error log
    sol::table errors = lua.create_table();
    for (size_t i = 0; i < info.error_log.size(); ++i) {
        errors[i + 1] = info.error_log[i];
    }
    table["error_log"] = errors;

    return table;
}

/**
 * @brief Register PluginLoadOptions bindings
 */
void register_plugin_load_options_bindings(sol::state& lua) {
    auto options_type = lua.new_usertype<qtplugin::PluginLoadOptions>(
        "PluginLoadOptions",
        sol::constructors<qtplugin::PluginLoadOptions()>());

    options_type["validate_sha256"] =
        &qtplugin::PluginLoadOptions::validate_sha256;
    options_type["expected_sha256"] =
        &qtplugin::PluginLoadOptions::expected_sha256;
    options_type["check_dependencies"] =
        &qtplugin::PluginLoadOptions::check_dependencies;
    options_type["initialize_immediately"] =
        &qtplugin::PluginLoadOptions::initialize_immediately;
    options_type["enable_hot_reload"] =
        &qtplugin::PluginLoadOptions::enable_hot_reload;

    // Configuration getter/setter using conversion functions
    options_type["get_configuration"] =
        [&lua](const qtplugin::PluginLoadOptions& options) -> sol::object {
        return qjson_to_lua(options.configuration, lua);
    };

    options_type["set_configuration"] = [](qtplugin::PluginLoadOptions& options,
                                           const sol::table& config) {
        options.configuration = lua_to_qjson(config).toObject();
    };

    // Timeout handling
    options_type["set_timeout_ms"] = [](qtplugin::PluginLoadOptions& opts,
                                        int ms) {
        opts.timeout = std::chrono::milliseconds(ms);
    };
    options_type["get_timeout_ms"] =
        [](const qtplugin::PluginLoadOptions& opts) {
            return static_cast<int>(opts.timeout.count());
        };

    qCDebug(pluginManagerBindingsLog)
        << "PluginLoadOptions bindings registered";
}

/**
 * @brief Register PluginInfo bindings
 */
void register_plugin_info_bindings(sol::state& lua) {
    auto info_type = lua.new_usertype<qtplugin::PluginInfo>("PluginInfo");

    info_type["id"] = sol::readonly(&qtplugin::PluginInfo::id);
    info_type["state"] = sol::readonly(&qtplugin::PluginInfo::state);
    info_type["hot_reload_enabled"] =
        sol::readonly(&qtplugin::PluginInfo::hot_reload_enabled);
    info_type["configuration"] =
        sol::readonly(&qtplugin::PluginInfo::configuration);
    info_type["metrics"] = sol::readonly(&qtplugin::PluginInfo::metrics);
    info_type["error_log"] = sol::readonly(&qtplugin::PluginInfo::error_log);

    // File path handling
    info_type["file_path"] =
        sol::property([](const qtplugin::PluginInfo& info) {
            return info.file_path.string();
        });

    // Metadata access
    info_type["metadata"] = sol::readonly(&qtplugin::PluginInfo::metadata);

    // Convert to Lua table
    info_type["to_table"] = [&lua](const qtplugin::PluginInfo& info) {
        return plugin_info_to_lua(info, lua);
    };

    qCDebug(pluginManagerBindingsLog) << "PluginInfo bindings registered";
}

/**
 * @brief Register core PluginManager bindings
 */
void register_plugin_manager_core_bindings(sol::state& lua) {
    // Register PluginManager usertype (no constructor due to incomplete type
    // dependencies)
    auto manager_type = lua.new_usertype<qtplugin::PluginManager>(
        "PluginManager", sol::no_constructor);

    // === Plugin Loading ===
    manager_type["load_plugin"] = sol::overload(
        // Simple load with path only
        [&lua](qtplugin::PluginManager& manager,
               const std::string& file_path) -> sol::object {
            std::filesystem::path path(file_path);
            auto result = manager.load_plugin(path);
            if (result) {
                return sol::make_object(lua, result.value());
            } else {
                return sol::make_object(lua, result.error());
            }
        },
        // Load with options
        [&lua](qtplugin::PluginManager& manager, const std::string& file_path,
               const qtplugin::PluginLoadOptions& options) -> sol::object {
            std::filesystem::path path(file_path);
            auto result = manager.load_plugin(path, options);
            if (result) {
                return sol::make_object(lua, result.value());
            } else {
                return sol::make_object(lua, result.error());
            }
        },
        // Load with Lua table options
        [&lua](qtplugin::PluginManager& manager, const std::string& file_path,
               const sol::table& options_table) -> sol::object {
            std::filesystem::path path(file_path);
            auto options = lua_to_plugin_load_options(options_table);
            auto result = manager.load_plugin(path, options);
            if (result) {
                return sol::make_object(lua, result.value());
            } else {
                return sol::make_object(lua, result.error());
            }
        });

    manager_type["unload_plugin"] = sol::overload(
        // Simple unload
        [&lua](qtplugin::PluginManager& manager,
               const std::string& plugin_id) -> sol::object {
            auto result = manager.unload_plugin(plugin_id);
            if (result) {
                return sol::make_object(lua, true);
            } else {
                return sol::make_object(lua, result.error());
            }
        },
        // Unload with force option
        [&lua](qtplugin::PluginManager& manager, const std::string& plugin_id,
               bool force) -> sol::object {
            auto result = manager.unload_plugin(plugin_id, force);
            if (result) {
                return sol::make_object(lua, true);
            } else {
                return sol::make_object(lua, result.error());
            }
        });

    manager_type["reload_plugin"] = sol::overload(
        // Simple reload
        [&lua](qtplugin::PluginManager& manager,
               const std::string& plugin_id) -> sol::object {
            auto result = manager.reload_plugin(plugin_id);
            if (result) {
                return sol::make_object(lua, true);
            } else {
                return sol::make_object(lua, result.error());
            }
        },
        // Reload with preserve state option
        [&lua](qtplugin::PluginManager& manager, const std::string& plugin_id,
               bool preserve_state) -> sol::object {
            auto result = manager.reload_plugin(plugin_id, preserve_state);
            if (result) {
                return sol::make_object(lua, true);
            } else {
                return sol::make_object(lua, result.error());
            }
        });

    qCDebug(pluginManagerBindingsLog)
        << "PluginManager core bindings registered";
}

/**
 * @brief Register PluginManager discovery and management bindings
 */
void register_plugin_manager_discovery_bindings(sol::state& lua) {
    auto manager_type = lua["PluginManager"];

    // === Plugin Discovery ===
    manager_type["discover_plugins"] = sol::overload(
        // Simple discovery
        [&lua](qtplugin::PluginManager& manager,
               const std::string& directory) -> sol::table {
            std::filesystem::path dir_path(directory);
            auto paths = manager.discover_plugins(dir_path);
            sol::table result = lua.create_table();
            for (size_t i = 0; i < paths.size(); ++i) {
                result[i + 1] = paths[i].string();
            }
            return result;
        },
        // Discovery with recursive option
        [&lua](qtplugin::PluginManager& manager, const std::string& directory,
               bool recursive) -> sol::table {
            std::filesystem::path dir_path(directory);
            auto paths = manager.discover_plugins(dir_path, recursive);
            sol::table result = lua.create_table();
            for (size_t i = 0; i < paths.size(); ++i) {
                result[i + 1] = paths[i].string();
            }
            return result;
        });

    // === Search Path Management ===
    manager_type["add_search_path"] = [](qtplugin::PluginManager& manager,
                                         const std::string& path) {
        std::filesystem::path search_path(path);
        manager.add_search_path(search_path);
    };

    manager_type["remove_search_path"] = [](qtplugin::PluginManager& manager,
                                            const std::string& path) {
        std::filesystem::path search_path(path);
        manager.remove_search_path(search_path);
    };

    manager_type["search_paths"] =
        [&lua](qtplugin::PluginManager& manager) -> sol::table {
        auto paths = manager.search_paths();
        sol::table result = lua.create_table();
        for (size_t i = 0; i < paths.size(); ++i) {
            result[i + 1] = paths[i].string();
        }
        return result;
    };

    manager_type["load_all_plugins"] = sol::overload(
        // Load all with default options
        [](qtplugin::PluginManager& manager) -> int {
            return manager.load_all_plugins();
        },
        // Load all with custom options
        [](qtplugin::PluginManager& manager,
           const qtplugin::PluginLoadOptions& options) -> int {
            return manager.load_all_plugins(options);
        },
        // Load all with Lua table options
        [](qtplugin::PluginManager& manager,
           const sol::table& options_table) -> int {
            auto options = lua_to_plugin_load_options(options_table);
            return manager.load_all_plugins(options);
        });

    qCDebug(pluginManagerBindingsLog)
        << "PluginManager discovery bindings registered";
}

/**
 * @brief Register PluginManager query and information bindings
 */
void register_plugin_manager_query_bindings(sol::state& lua) {
    auto manager_type = lua["PluginManager"];

    // === Plugin Queries ===
    manager_type["get_plugin"] =
        [](qtplugin::PluginManager& manager,
           const std::string& plugin_id) -> std::shared_ptr<qtplugin::IPlugin> {
        return manager.get_plugin(plugin_id);
    };

    manager_type["get_plugin_info"] =
        [&lua](qtplugin::PluginManager& manager,
               const std::string& plugin_id) -> sol::object {
        auto info = manager.get_plugin_info(plugin_id);
        if (info) {
            return sol::make_object(lua, plugin_info_to_lua(*info, lua));
        } else {
            return sol::nil;
        }
    };

    manager_type["loaded_plugins"] =
        [&lua](qtplugin::PluginManager& manager) -> sol::table {
        auto plugins = manager.loaded_plugins();
        sol::table result = lua.create_table();
        for (size_t i = 0; i < plugins.size(); ++i) {
            result[i + 1] = plugins[i];
        }
        return result;
    };

    manager_type["all_plugin_info"] =
        [&lua](qtplugin::PluginManager& manager) -> sol::table {
        auto infos = manager.all_plugin_info();
        sol::table result = lua.create_table();
        for (size_t i = 0; i < infos.size(); ++i) {
            result[i + 1] = plugin_info_to_lua(infos[i], lua);
        }
        return result;
    };

    manager_type["plugins_with_capability"] =
        [&lua](qtplugin::PluginManager& manager, int capability) -> sol::table {
        auto plugins = manager.plugins_with_capability(
            static_cast<qtplugin::PluginCapability>(capability));
        sol::table result = lua.create_table();
        for (size_t i = 0; i < plugins.size(); ++i) {
            result[i + 1] = plugins[i];
        }
        return result;
    };

    manager_type["plugins_in_category"] =
        [&lua](qtplugin::PluginManager& manager,
               const std::string& category) -> sol::table {
        auto plugins = manager.plugins_in_category(category);
        sol::table result = lua.create_table();
        for (size_t i = 0; i < plugins.size(); ++i) {
            result[i + 1] = plugins[i];
        }
        return result;
    };

    qCDebug(pluginManagerBindingsLog)
        << "PluginManager query bindings registered";
}

/**
 * @brief Register PluginManager lifecycle and management bindings
 */
void register_plugin_manager_lifecycle_bindings(sol::state& lua) {
    auto manager_type = lua["PluginManager"];

    // === Plugin Lifecycle ===
    manager_type["initialize_all_plugins"] =
        [](qtplugin::PluginManager& manager) -> int {
        return manager.initialize_all_plugins();
    };

    manager_type["shutdown_all_plugins"] =
        [](qtplugin::PluginManager& manager) {
            manager.shutdown_all_plugins();
        };

    manager_type["start_all_services"] =
        [](qtplugin::PluginManager& manager) -> int {
        return manager.start_all_services();
    };

    manager_type["stop_all_services"] =
        [](qtplugin::PluginManager& manager) -> int {
        return manager.stop_all_services();
    };

    // === Dependency Management ===
    manager_type["resolve_dependencies"] =
        [&lua](qtplugin::PluginManager& manager) -> sol::object {
        auto result = manager.resolve_dependencies();
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    manager_type["get_load_order"] =
        [&lua](qtplugin::PluginManager& manager) -> sol::table {
        auto order = manager.get_load_order();
        sol::table result = lua.create_table();
        for (size_t i = 0; i < order.size(); ++i) {
            result[i + 1] = order[i];
        }
        return result;
    };

    manager_type["can_unload_safely"] =
        [](qtplugin::PluginManager& manager,
           const std::string& plugin_id) -> bool {
        return manager.can_unload_safely(plugin_id);
    };

    // === Hot Reload ===
    manager_type["enable_hot_reload"] =
        [&lua](qtplugin::PluginManager& manager,
               const std::string& plugin_id) -> sol::object {
        auto result = manager.enable_hot_reload(plugin_id);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    manager_type["disable_hot_reload"] = [](qtplugin::PluginManager& manager,
                                            const std::string& plugin_id) {
        manager.disable_hot_reload(plugin_id);
    };

    manager_type["is_hot_reload_enabled"] =
        [](qtplugin::PluginManager& manager,
           const std::string& plugin_id) -> bool {
        return manager.is_hot_reload_enabled(plugin_id);
    };

    manager_type["enable_global_hot_reload"] = sol::overload(
        // Enable with default directories
        [](qtplugin::PluginManager& manager) {
            manager.enable_global_hot_reload();
        },
        // Enable with specific directories
        [](qtplugin::PluginManager& manager, const sol::table& directories) {
            std::vector<std::filesystem::path> paths;
            for (const auto& [key, value] : directories) {
                if (value.is<std::string>()) {
                    paths.emplace_back(value.as<std::string>());
                }
            }
            manager.enable_global_hot_reload(paths);
        });

    manager_type["disable_global_hot_reload"] =
        [](qtplugin::PluginManager& manager) {
            manager.disable_global_hot_reload();
        };

    // === Configuration Management ===
    manager_type["configure_plugin"] =
        [&lua](qtplugin::PluginManager& manager, const std::string& plugin_id,
               const sol::table& config) -> sol::object {
        auto json_config = lua_to_qjson(config).toObject();
        auto result = manager.configure_plugin(plugin_id, json_config);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    manager_type["get_plugin_configuration"] =
        [&lua](qtplugin::PluginManager& manager,
               const std::string& plugin_id) -> sol::object {
        auto config = manager.get_plugin_configuration(plugin_id);
        return qjson_to_lua(config, lua);
    };

    // === Communication ===
    manager_type["send_command"] = sol::overload(
        // Send command without parameters
        [&lua](qtplugin::PluginManager& manager, const std::string& plugin_id,
               const std::string& command) -> sol::object {
            auto result = manager.send_command(plugin_id, command);
            if (result) {
                return qjson_to_lua(result.value(), lua);
            } else {
                return sol::make_object(lua, result.error());
            }
        },
        // Send command with parameters
        [&lua](qtplugin::PluginManager& manager, const std::string& plugin_id,
               const std::string& command,
               const sol::table& params) -> sol::object {
            auto json_params = lua_to_qjson(params).toObject();
            auto result = manager.send_command(plugin_id, command, json_params);
            if (result) {
                return qjson_to_lua(result.value(), lua);
            } else {
                return sol::make_object(lua, result.error());
            }
        });

    // === Metrics and Monitoring ===
    manager_type["system_metrics"] =
        [&lua](qtplugin::PluginManager& manager) -> sol::object {
        return qjson_to_lua(manager.system_metrics(), lua);
    };

    manager_type["plugin_metrics"] =
        [&lua](qtplugin::PluginManager& manager,
               const std::string& plugin_id) -> sol::object {
        return qjson_to_lua(manager.plugin_metrics(plugin_id), lua);
    };

    manager_type["start_monitoring"] = sol::overload(
        // Start with default interval
        [](qtplugin::PluginManager& manager) { manager.start_monitoring(); },
        // Start with custom interval
        [](qtplugin::PluginManager& manager, int interval_ms) {
            manager.start_monitoring(std::chrono::milliseconds(interval_ms));
        });

    manager_type["stop_monitoring"] = [](qtplugin::PluginManager& manager) {
        manager.stop_monitoring();
    };

    manager_type["is_monitoring_active"] =
        [](qtplugin::PluginManager& manager) -> bool {
        return manager.is_monitoring_active();
    };

    qCDebug(pluginManagerBindingsLog)
        << "PluginManager lifecycle bindings registered";
}

/**
 * @brief Register all PluginManager bindings
 */
void register_plugin_manager_bindings(sol::state& lua) {
    qCDebug(pluginManagerBindingsLog)
        << "Registering PluginManager bindings...";

    // Register supporting types first
    register_plugin_load_options_bindings(lua);
    register_plugin_info_bindings(lua);

    // Register core PluginManager functionality
    register_plugin_manager_core_bindings(lua);
    register_plugin_manager_discovery_bindings(lua);
    register_plugin_manager_query_bindings(lua);
    register_plugin_manager_lifecycle_bindings(lua);

    // Create qtforge.core namespace and add convenience functions
    sol::table qtforge = lua["qtforge"].get_or_create<sol::table>();
    sol::table core = qtforge["core"].get_or_create<sol::table>();

    // Note: PluginManager factory function removed due to incomplete type
    // dependencies Users should create PluginManager instances through the main
    // application

    // Add PluginManager to core namespace for direct access
    core["PluginManager"] = lua["PluginManager"];

    qCDebug(pluginManagerBindingsLog)
        << "PluginManager bindings registration complete";
}

/**
 * @brief Main registration function for PluginManager bindings
 */
void register_plugin_manager_core_bindings_main(sol::state& lua) {
    qCDebug(pluginManagerBindingsLog)
        << "Registering PluginManager core bindings...";

    // Create main qtforge namespace
    sol::table qtforge = lua["qtforge"].get_or_create<sol::table>();

    // Register PluginManager bindings
    register_plugin_manager_bindings(lua);

    qCDebug(pluginManagerBindingsLog)
        << "PluginManager core bindings registration complete";
}

#else  // QTFORGE_LUA_BINDINGS not defined

void register_plugin_load_options_bindings_stub() {
    qCWarning(pluginManagerBindingsLog)
        << "PluginLoadOptions bindings not available - Lua support not "
           "compiled";
}

void register_plugin_info_bindings_stub() {
    qCWarning(pluginManagerBindingsLog)
        << "PluginInfo bindings not available - Lua support not compiled";
}

void register_plugin_manager_core_bindings_stub() {
    qCWarning(pluginManagerBindingsLog)
        << "PluginManager core bindings not available - Lua support not "
           "compiled";
}

#endif  // QTFORGE_LUA_BINDINGS

}  // namespace qtforge_lua
