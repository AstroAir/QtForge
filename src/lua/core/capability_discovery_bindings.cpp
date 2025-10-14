/**
 * @file capability_discovery_bindings.cpp
 * @brief Lua bindings for PluginCapabilityDiscovery
 * @version 3.2.0
 * @author QtForge Development Team
 */

#include <qtplugin/core/plugin_capability_discovery.hpp>
#include <qtplugin/interfaces/core/plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>
#include <sol/sol.hpp>
#include "../qtforge_lua_bindings.hpp"

namespace qtforge_lua {

void bind_capability_discovery(sol::state& lua) {
    // === PluginCapabilityInfo Struct ===
    lua.new_usertype<qtplugin::PluginCapabilityInfo>(
        "PluginCapabilityInfo",
        sol::constructors<qtplugin::PluginCapabilityInfo()>(), "name",
        &qtplugin::PluginCapabilityInfo::name, "description",
        &qtplugin::PluginCapabilityInfo::description, "capability_flag",
        &qtplugin::PluginCapabilityInfo::capability_flag,

        "to_json",
        [](const qtplugin::PluginCapabilityInfo& self, sol::this_state s) {
            sol::state_view lua(s);
            auto json = self.to_json();
            sol::table result = lua.create_table();

            for (auto it = json.begin(); it != json.end(); ++it) {
                const auto& key = it.key();
                const auto& value = it.value();

                if (value.isBool()) {
                    result[key.toStdString()] = value.toBool();
                } else if (value.isDouble()) {
                    result[key.toStdString()] = value.toDouble();
                } else if (value.isString()) {
                    result[key.toStdString()] = value.toString().toStdString();
                } else if (value.isNull()) {
                    result[key.toStdString()] = sol::lua_nil;
                }
            }

            return result;
        });

    // === PluginMethodInfo Struct ===
    lua.new_usertype<qtplugin::PluginMethodInfo>(
        "PluginMethodInfo", sol::constructors<qtplugin::PluginMethodInfo()>(),
        "name", &qtplugin::PluginMethodInfo::name, "signature",
        &qtplugin::PluginMethodInfo::signature, "return_type",
        &qtplugin::PluginMethodInfo::return_type, "is_invokable",
        &qtplugin::PluginMethodInfo::is_invokable, "is_slot",
        &qtplugin::PluginMethodInfo::is_slot, "is_signal",
        &qtplugin::PluginMethodInfo::is_signal,

        "to_json",
        [](const qtplugin::PluginMethodInfo& self, sol::this_state s) {
            sol::state_view lua(s);
            auto json = self.to_json();
            sol::table result = lua.create_table();

            for (auto it = json.begin(); it != json.end(); ++it) {
                const auto& key = it.key();
                const auto& value = it.value();

                if (value.isBool()) {
                    result[key.toStdString()] = value.toBool();
                } else if (value.isDouble()) {
                    result[key.toStdString()] = value.toDouble();
                } else if (value.isString()) {
                    result[key.toStdString()] = value.toString().toStdString();
                } else if (value.isNull()) {
                    result[key.toStdString()] = sol::lua_nil;
                }
            }

            return result;
        });

    // === PluginPropertyInfo Struct ===
    lua.new_usertype<qtplugin::PluginPropertyInfo>(
        "PluginPropertyInfo",
        sol::constructors<qtplugin::PluginPropertyInfo()>(), "name",
        &qtplugin::PluginPropertyInfo::name, "type",
        &qtplugin::PluginPropertyInfo::type, "is_readable",
        &qtplugin::PluginPropertyInfo::is_readable, "is_writable",
        &qtplugin::PluginPropertyInfo::is_writable, "is_resettable",
        &qtplugin::PluginPropertyInfo::is_resettable, "has_notify_signal",
        &qtplugin::PluginPropertyInfo::has_notify_signal, "notify_signal",
        &qtplugin::PluginPropertyInfo::notify_signal,

        "to_json",
        [](const qtplugin::PluginPropertyInfo& self, sol::this_state s) {
            sol::state_view lua(s);
            auto json = self.to_json();
            sol::table result = lua.create_table();

            for (auto it = json.begin(); it != json.end(); ++it) {
                const auto& key = it.key();
                const auto& value = it.value();

                if (value.isBool()) {
                    result[key.toStdString()] = value.toBool();
                } else if (value.isDouble()) {
                    result[key.toStdString()] = value.toDouble();
                } else if (value.isString()) {
                    result[key.toStdString()] = value.toString().toStdString();
                } else if (value.isNull()) {
                    result[key.toStdString()] = sol::lua_nil;
                }
            }

            return result;
        });

    // === PluginInterfaceInfo Struct ===
    lua.new_usertype<qtplugin::PluginInterfaceInfo>(
        "PluginInterfaceInfo",
        sol::constructors<qtplugin::PluginInterfaceInfo()>(), "interface_id",
        &qtplugin::PluginInterfaceInfo::interface_id, "interface_name",
        &qtplugin::PluginInterfaceInfo::interface_name, "version",
        &qtplugin::PluginInterfaceInfo::version,

        "to_json",
        [](const qtplugin::PluginInterfaceInfo& self, sol::this_state s) {
            sol::state_view lua(s);
            auto json = self.to_json();
            sol::table result = lua.create_table();

            for (auto it = json.begin(); it != json.end(); ++it) {
                const auto& key = it.key();
                const auto& value = it.value();

                if (value.isBool()) {
                    result[key.toStdString()] = value.toBool();
                } else if (value.isDouble()) {
                    result[key.toStdString()] = value.toDouble();
                } else if (value.isString()) {
                    result[key.toStdString()] = value.toString().toStdString();
                } else if (value.isNull()) {
                    result[key.toStdString()] = sol::lua_nil;
                }
            }

            return result;
        });

    // === PluginCapabilityDiscovery Class ===
    lua.new_usertype<qtplugin::PluginCapabilityDiscovery>(
        "PluginCapabilityDiscovery",
        sol::constructors<qtplugin::PluginCapabilityDiscovery(QObject*)>(),

        // Plugin analysis
        "discover_capabilities",
        [](qtplugin::PluginCapabilityDiscovery& self,
           std::shared_ptr<qtplugin::IPlugin> plugin,
           sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto result = self.discover_capabilities(plugin);
            if (result) {
                sol::table discovery_table = lua.create_table();
                discovery_table["plugin_id"] =
                    result.value().plugin_id.toStdString();
                // Add more fields as needed
                return discovery_table;
            }
            return sol::lua_nil;
        },

        "get_plugin_methods",
        [](qtplugin::PluginCapabilityDiscovery& self,
           std::shared_ptr<qtplugin::IPlugin> plugin,
           sol::this_state s) -> sol::table {
            sol::state_view lua(s);
            auto methods = self.get_plugin_methods(plugin);
            sol::table result = lua.create_table();
            int index = 1;
            for (const auto& method : methods) {
                result[index++] = method;
            }
            return result;
        },

        "get_plugin_properties",
        [](qtplugin::PluginCapabilityDiscovery& self,
           std::shared_ptr<qtplugin::IPlugin> plugin,
           sol::this_state s) -> sol::table {
            sol::state_view lua(s);
            auto properties = self.get_plugin_properties(plugin);
            sol::table result = lua.create_table();
            int index = 1;
            for (const auto& prop : properties) {
                result[index++] = prop;
            }
            return result;
        },

        "validate_interface",
        [](qtplugin::PluginCapabilityDiscovery& self,
           std::shared_ptr<qtplugin::IPlugin> plugin,
           const std::string& interface_id) -> bool {
            auto result = self.validate_interface(
                plugin, QString::fromStdString(interface_id));
            if (result) {
                return result.value();
            }
            return false;
        });
}

}  // namespace qtforge_lua
