/**
 * @file configuration_bindings.cpp
 * @brief Lua bindings for QtForge configuration management
 */

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QString>
#include <QVariant>
#include <filesystem>
#include <memory>
#include <sol/sol.hpp>
#include <string>
#include <string_view>

#include "qtplugin/managers/configuration_manager.hpp"
#include "qtplugin/managers/configuration_manager_impl.hpp"
#include "qtplugin/utils/error_handling.hpp"

Q_LOGGING_CATEGORY(configBindingsLog, "qtplugin.lua.bindings.configuration")

namespace qtplugin {

// Forward declarations
void register_configuration_scope_bindings(sol::state& lua);
void register_configuration_manager_bindings(sol::state& lua);
void register_configuration_bindings(sol::state& lua);

/**
 * @brief Convert ConfigurationScope to Lua table
 */
sol::object configuration_scope_to_lua(sol::this_state s,
                                       ConfigurationScope scope) {
    sol::state_view lua(s);

    switch (scope) {
        case ConfigurationScope::Global:
            return sol::make_object(lua, "Global");
        case ConfigurationScope::Plugin:
            return sol::make_object(lua, "Plugin");
        case ConfigurationScope::User:
            return sol::make_object(lua, "User");
        case ConfigurationScope::Session:
            return sol::make_object(lua, "Session");
        case ConfigurationScope::Runtime:
            return sol::make_object(lua, "Runtime");
        default:
            return sol::make_object(lua, "Unknown");
    }
}

/**
 * @brief Convert Lua string to ConfigurationScope
 */
ConfigurationScope lua_to_configuration_scope(const std::string& scope_str) {
    if (scope_str == "Global")
        return ConfigurationScope::Global;
    if (scope_str == "Plugin")
        return ConfigurationScope::Plugin;
    if (scope_str == "User")
        return ConfigurationScope::User;
    if (scope_str == "Session")
        return ConfigurationScope::Session;
    if (scope_str == "Runtime")
        return ConfigurationScope::Runtime;
    return ConfigurationScope::Global;  // Default fallback
}

/**
 * @brief Convert QJsonValue to Lua object (simple conversion)
 */
sol::object qjsonvalue_to_lua_simple(sol::this_state s,
                                     const QJsonValue& value) {
    sol::state_view lua(s);

    switch (value.type()) {
        case QJsonValue::Bool:
            return sol::make_object(lua, value.toBool());
        case QJsonValue::Double:
            return sol::make_object(lua, value.toDouble());
        case QJsonValue::String:
            return sol::make_object(lua, value.toString().toStdString());
        case QJsonValue::Array: {
            sol::table arr = lua.create_table();
            const auto json_array = value.toArray();
            for (int i = 0; i < json_array.size(); ++i) {
                arr[i + 1] = qjsonvalue_to_lua_simple(s, json_array[i]);
            }
            return arr;
        }
        case QJsonValue::Object: {
            sol::table obj = lua.create_table();
            const auto json_obj = value.toObject();
            for (auto it = json_obj.begin(); it != json_obj.end(); ++it) {
                obj[it.key().toStdString()] =
                    qjsonvalue_to_lua_simple(s, it.value());
            }
            return obj;
        }
        case QJsonValue::Null:
        case QJsonValue::Undefined:
        default:
            return sol::lua_nil;
    }
}

/**
 * @brief Convert Lua object to QJsonValue (simple conversion)
 */
QJsonValue lua_to_qjsonvalue_simple(const sol::object& obj) {
    switch (obj.get_type()) {
        case sol::type::boolean:
            return QJsonValue(obj.as<bool>());
        case sol::type::number:
            return QJsonValue(obj.as<double>());
        case sol::type::string:
            return QJsonValue(QString::fromStdString(obj.as<std::string>()));
        case sol::type::table: {
            sol::table tbl = obj.as<sol::table>();

            // Check if it's an array (consecutive integer keys starting from 1)
            bool is_array = true;
            size_t expected_index = 1;
            for (const auto& pair : tbl) {
                if (pair.first.get_type() != sol::type::number ||
                    pair.first.as<size_t>() != expected_index) {
                    is_array = false;
                    break;
                }
                ++expected_index;
            }

            if (is_array) {
                QJsonArray arr;
                for (size_t i = 1; i <= tbl.size(); ++i) {
                    arr.append(lua_to_qjsonvalue_simple(tbl[i]));
                }
                return QJsonValue(arr);
            } else {
                QJsonObject obj_json;
                for (const auto& pair : tbl) {
                    if (pair.first.get_type() == sol::type::string) {
                        obj_json[QString::fromStdString(
                            pair.first.as<std::string>())] =
                            lua_to_qjsonvalue_simple(pair.second);
                    }
                }
                return QJsonValue(obj_json);
            }
        }
        case sol::type::lua_nil:
        default:
            return QJsonValue();
    }
}

/**
 * @brief Register ConfigurationScope enum bindings
 */
void register_configuration_scope_bindings(sol::state& lua) {
    lua.new_enum<ConfigurationScope>(
        "ConfigurationScope", {{"Global", ConfigurationScope::Global},
                               {"Plugin", ConfigurationScope::Plugin},
                               {"User", ConfigurationScope::User},
                               {"Session", ConfigurationScope::Session},
                               {"Runtime", ConfigurationScope::Runtime}});

    qCDebug(configBindingsLog) << "ConfigurationScope enum bindings registered";
}

/**
 * @brief Register ConfigurationManager bindings
 */
void register_configuration_manager_bindings(sol::state& lua) {
    auto config_type = lua.new_usertype<ConfigurationManager>(
        "ConfigurationManager", sol::no_constructor);

    // === Core Value Access ===

    // Get value with scope and plugin_id
    config_type["get_value"] =
        [](ConfigurationManager& mgr, const std::string& key,
           ConfigurationScope scope, const std::string& plugin_id,
           sol::this_state s) -> sol::object {
        auto result = mgr.get_value(key, scope, plugin_id);
        if (result) {
            return qjsonvalue_to_lua_simple(s, result.value());
        } else {
            return sol::lua_nil;
        }
    };

    // Get value with scope only (no plugin_id)
    config_type["get_value_scoped"] =
        [](ConfigurationManager& mgr, const std::string& key,
           ConfigurationScope scope, sol::this_state s) -> sol::object {
        auto result = mgr.get_value(key, scope);
        if (result) {
            return qjsonvalue_to_lua_simple(s, result.value());
        } else {
            return sol::lua_nil;
        }
    };

    // Get value with default scope (Global)
    config_type["get_value_simple"] = [](ConfigurationManager& mgr,
                                         const std::string& key,
                                         sol::this_state s) -> sol::object {
        auto result = mgr.get_value(key, ConfigurationScope::Global);
        if (result) {
            return qjsonvalue_to_lua_simple(s, result.value());
        } else {
            return sol::lua_nil;
        }
    };

    // Set value with scope and plugin_id
    config_type["set_value"] =
        [](ConfigurationManager& mgr, const std::string& key,
           const sol::object& value, ConfigurationScope scope,
           const std::string& plugin_id) -> bool {
        QJsonValue json_value = lua_to_qjsonvalue_simple(value);
        auto result = mgr.set_value(key, json_value, scope, plugin_id);
        return result.has_value();
    };

    // Set value with scope only
    config_type["set_value_scoped"] =
        [](ConfigurationManager& mgr, const std::string& key,
           const sol::object& value, ConfigurationScope scope) -> bool {
        QJsonValue json_value = lua_to_qjsonvalue_simple(value);
        auto result = mgr.set_value(key, json_value, scope);
        return result.has_value();
    };

    // Set value with default scope (Global)
    config_type["set_value_simple"] = [](ConfigurationManager& mgr,
                                         const std::string& key,
                                         const sol::object& value) -> bool {
        QJsonValue json_value = lua_to_qjsonvalue_simple(value);
        auto result =
            mgr.set_value(key, json_value, ConfigurationScope::Global);
        return result.has_value();
    };

    // === Convenience Methods ===

    // Get string value with default
    config_type["get_string"] = [](ConfigurationManager& mgr,
                                   const std::string& key,
                                   const std::string& default_value,
                                   ConfigurationScope scope) -> std::string {
        QJsonValue default_json =
            QJsonValue(QString::fromStdString(default_value));
        QJsonValue result = mgr.get_value_or_default(key, default_json, scope);
        if (result.isString()) {
            return result.toString().toStdString();
        }
        return default_value;
    };

    // Get boolean value with default
    config_type["get_bool"] = [](ConfigurationManager& mgr,
                                 const std::string& key, bool default_value,
                                 ConfigurationScope scope) -> bool {
        QJsonValue default_json = QJsonValue(default_value);
        QJsonValue result = mgr.get_value_or_default(key, default_json, scope);
        if (result.isBool()) {
            return result.toBool();
        }
        return default_value;
    };

    // Get integer value with default
    config_type["get_int"] = [](ConfigurationManager& mgr,
                                const std::string& key, int default_value,
                                ConfigurationScope scope) -> int {
        QJsonValue default_json = QJsonValue(default_value);
        QJsonValue result = mgr.get_value_or_default(key, default_json, scope);
        if (result.isDouble()) {
            return static_cast<int>(result.toDouble());
        }
        return default_value;
    };

    // Get double value with default
    config_type["get_double"] = [](ConfigurationManager& mgr,
                                   const std::string& key, double default_value,
                                   ConfigurationScope scope) -> double {
        QJsonValue default_json = QJsonValue(default_value);
        QJsonValue result = mgr.get_value_or_default(key, default_json, scope);
        if (result.isDouble()) {
            return result.toDouble();
        }
        return default_value;
    };

    // === Persistence Methods ===

    // Load from file with all parameters
    config_type["load_from_file"] =
        [](ConfigurationManager& mgr, const std::string& file_path,
           ConfigurationScope scope, const std::string& plugin_id,
           bool merge) -> bool {
        auto result = mgr.load_from_file(std::filesystem::path(file_path),
                                         scope, plugin_id, merge);
        return result.has_value();
    };

    // Load from file with scope and merge
    config_type["load_from_file_scoped"] =
        [](ConfigurationManager& mgr, const std::string& file_path,
           ConfigurationScope scope, bool merge) -> bool {
        auto result = mgr.load_from_file(std::filesystem::path(file_path),
                                         scope, "", merge);
        return result.has_value();
    };

    // Load from file with default parameters
    config_type["load_from_file_simple"] =
        [](ConfigurationManager& mgr, const std::string& file_path) -> bool {
        auto result = mgr.load_from_file(std::filesystem::path(file_path),
                                         ConfigurationScope::Global, "", true);
        return result.has_value();
    };

    // Save to file with all parameters
    config_type["save_to_file"] =
        [](ConfigurationManager& mgr, const std::string& file_path,
           ConfigurationScope scope, const std::string& plugin_id) -> bool {
        auto result = mgr.save_to_file(std::filesystem::path(file_path), scope,
                                       plugin_id);
        return result.has_value();
    };

    // Save to file with scope only
    config_type["save_to_file_scoped"] = [](ConfigurationManager& mgr,
                                            const std::string& file_path,
                                            ConfigurationScope scope) -> bool {
        auto result =
            mgr.save_to_file(std::filesystem::path(file_path), scope, "");
        return result.has_value();
    };

    // Save to file with default scope
    config_type["save_to_file_simple"] =
        [](ConfigurationManager& mgr, const std::string& file_path) -> bool {
        auto result = mgr.save_to_file(std::filesystem::path(file_path),
                                       ConfigurationScope::Global, "");
        return result.has_value();
    };

    // Note: load_from_json and export_to_json methods are not available in the
    // interface Users can use load_from_file/save_to_file with JSON files
    // instead

    // === Configuration Management ===

    // Get configuration object
    config_type["get_configuration"] =
        [](ConfigurationManager& mgr, ConfigurationScope scope,
           const std::string& plugin_id, sol::this_state s) -> sol::object {
        auto result = mgr.get_configuration(scope, plugin_id);
        if (result) {
            return qjsonvalue_to_lua_simple(s, QJsonValue(result.value()));
        } else {
            return sol::lua_nil;
        }
    };

    // Get configuration object with default plugin_id
    config_type["get_configuration_scoped"] =
        [](ConfigurationManager& mgr, ConfigurationScope scope,
           sol::this_state s) -> sol::object {
        auto result = mgr.get_configuration(scope, "");
        if (result) {
            return qjsonvalue_to_lua_simple(s, QJsonValue(result.value()));
        } else {
            return sol::lua_nil;
        }
    };

    // Set configuration object
    config_type["set_configuration"] =
        [](ConfigurationManager& mgr, const sol::table& config_table,
           ConfigurationScope scope, const std::string& plugin_id,
           bool merge) -> bool {
        QJsonValue json_value = lua_to_qjsonvalue_simple(config_table);
        if (json_value.isObject()) {
            auto result = mgr.set_configuration(json_value.toObject(), scope,
                                                plugin_id, merge);
            return result.has_value();
        }
        return false;
    };

    // Set configuration object with default parameters
    config_type["set_configuration_simple"] =
        [](ConfigurationManager& mgr, const sol::table& config_table,
           ConfigurationScope scope) -> bool {
        QJsonValue json_value = lua_to_qjsonvalue_simple(config_table);
        if (json_value.isObject()) {
            auto result =
                mgr.set_configuration(json_value.toObject(), scope, "", true);
            return result.has_value();
        }
        return false;
    };

    // === Utility Methods ===

    // Check if key exists
    config_type["has_value"] =
        [](ConfigurationManager& mgr, const std::string& key,
           ConfigurationScope scope, const std::string& plugin_id) -> bool {
        auto result = mgr.get_value(key, scope, plugin_id);
        return result.has_value();
    };

    // Check if key exists with default parameters
    config_type["has_value_simple"] = [](ConfigurationManager& mgr,
                                         const std::string& key) -> bool {
        auto result = mgr.get_value(key, ConfigurationScope::Global);
        return result.has_value();
    };

    // Remove key (using the correct method name)
    config_type["remove_key"] =
        [](ConfigurationManager& mgr, const std::string& key,
           ConfigurationScope scope, const std::string& plugin_id) -> bool {
        auto result = mgr.remove_key(key, scope, plugin_id);
        return result.has_value();
    };

    // Remove key with default parameters
    config_type["remove_key_simple"] = [](ConfigurationManager& mgr,
                                          const std::string& key) -> bool {
        auto result = mgr.remove_key(key, ConfigurationScope::Global);
        return result.has_value();
    };

    // Clear configuration
    config_type["clear_configuration"] =
        [](ConfigurationManager& mgr, ConfigurationScope scope,
           const std::string& plugin_id) -> bool {
        auto result = mgr.clear_configuration(scope, plugin_id);
        return result.has_value();
    };

    // Clear configuration with default plugin_id
    config_type["clear_configuration_scoped"] =
        [](ConfigurationManager& mgr, ConfigurationScope scope) -> bool {
        auto result = mgr.clear_configuration(scope, "");
        return result.has_value();
    };

    // Reload configuration
    config_type["reload_configuration"] =
        [](ConfigurationManager& mgr, ConfigurationScope scope,
           const std::string& plugin_id) -> bool {
        auto result = mgr.reload_configuration(scope, plugin_id);
        return result.has_value();
    };

    // Reload configuration with default plugin_id
    config_type["reload_configuration_scoped"] =
        [](ConfigurationManager& mgr, ConfigurationScope scope) -> bool {
        auto result = mgr.reload_configuration(scope, "");
        return result.has_value();
    };

    qCDebug(configBindingsLog) << "ConfigurationManager bindings registered";
}

/**
 * @brief Register all configuration-related bindings
 */
void register_configuration_bindings(sol::state& lua) {
    register_configuration_scope_bindings(lua);
    register_configuration_manager_bindings(lua);

    qCDebug(configBindingsLog) << "All configuration bindings registered";
}

}  // namespace qtplugin
