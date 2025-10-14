/**
 * @file service_plugin_bindings.cpp
 * @brief Lua bindings for IServicePlugin interface
 * @version 3.2.0
 * @author QtForge Development Team
 */

#include <qtplugin/interfaces/core/plugin_interface.hpp>
#include <qtplugin/interfaces/core/service_plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>
#include <sol/sol.hpp>
#include "../qtforge_lua_bindings.hpp"

namespace qtforge_lua {

// Helper to convert expected<void, PluginError> to LuaResult
struct LuaVoidResult {
    bool has_value;
    qtplugin::PluginError error;

    LuaVoidResult(
        const qtplugin::expected<void, qtplugin::PluginError>& expected) {
        if (expected) {
            has_value = true;
        } else {
            has_value = false;
            error = expected.error();
        }
    }
};

void bind_service_plugin(sol::state& lua) {
    // === ServiceExecutionMode Enum ===
    lua.new_enum<qtplugin::ServiceExecutionMode>(
        "ServiceExecutionMode",
        {{"MainThread", qtplugin::ServiceExecutionMode::MainThread},
         {"WorkerThread", qtplugin::ServiceExecutionMode::WorkerThread},
         {"ThreadPool", qtplugin::ServiceExecutionMode::ThreadPool},
         {"Async", qtplugin::ServiceExecutionMode::Async},
         {"Custom", qtplugin::ServiceExecutionMode::Custom}});

    // === ServiceState Enum ===
    lua.new_enum<qtplugin::ServiceState>(
        "ServiceState", {{"Stopped", qtplugin::ServiceState::Stopped},
                         {"Starting", qtplugin::ServiceState::Starting},
                         {"Running", qtplugin::ServiceState::Running},
                         {"Pausing", qtplugin::ServiceState::Pausing},
                         {"Paused", qtplugin::ServiceState::Paused},
                         {"Resuming", qtplugin::ServiceState::Resuming},
                         {"Stopping", qtplugin::ServiceState::Stopping},
                         {"Error", qtplugin::ServiceState::Error},
                         {"Restarting", qtplugin::ServiceState::Restarting}});

    // === LuaVoidResult for service operations ===
    lua.new_usertype<LuaVoidResult>("LuaVoidResult", "has_value",
                                    &LuaVoidResult::has_value, "error",
                                    &LuaVoidResult::error);

    // === IServicePlugin Interface ===
    lua.new_usertype<qtplugin::IServicePlugin>(
        "IServicePlugin", sol::base_classes, sol::bases<qtplugin::IPlugin>(),

        // Service lifecycle methods
        "start_service",
        [](qtplugin::IServicePlugin& self) {
            return LuaVoidResult(self.start_service());
        },

        "stop_service",
        [](qtplugin::IServicePlugin& self) {
            return LuaVoidResult(self.stop_service());
        },

        "pause_service",
        [](qtplugin::IServicePlugin& self) {
            return LuaVoidResult(self.pause_service());
        },

        "resume_service",
        [](qtplugin::IServicePlugin& self) {
            return LuaVoidResult(self.resume_service());
        },

        "restart_service",
        [](qtplugin::IServicePlugin& self) {
            return LuaVoidResult(self.restart_service());
        },

        // Service state and info methods
        "service_state", &qtplugin::IServicePlugin::service_state,

        "execution_mode", &qtplugin::IServicePlugin::execution_mode,

        "is_service_running", &qtplugin::IServicePlugin::is_service_running,

        "service_uptime",
        [](const qtplugin::IServicePlugin& self) {
            return self.service_uptime().count();
        },

        "service_metrics",
        [](const qtplugin::IServicePlugin& self, sol::this_state s) {
            sol::state_view lua(s);
            auto metrics = self.service_metrics();
            sol::table result = lua.create_table();

            // Convert QJsonObject to Lua table
            for (auto it = metrics.begin(); it != metrics.end(); ++it) {
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
        },

        "configure_service",
        [](qtplugin::IServicePlugin& self, sol::table config) {
            // Convert Lua table to QJsonObject
            QJsonObject json_config;
            for (const auto& pair : config) {
                std::string key = pair.first.as<std::string>();
                auto value = pair.second;

                if (value.is<bool>()) {
                    json_config[QString::fromStdString(key)] = value.as<bool>();
                } else if (value.is<int>()) {
                    json_config[QString::fromStdString(key)] = value.as<int>();
                } else if (value.is<double>()) {
                    json_config[QString::fromStdString(key)] =
                        value.as<double>();
                } else if (value.is<std::string>()) {
                    json_config[QString::fromStdString(key)] =
                        QString::fromStdString(value.as<std::string>());
                } else if (value.is<sol::lua_nil_t>()) {
                    json_config[QString::fromStdString(key)] = QJsonValue::Null;
                }
            }

            return LuaVoidResult(self.configure_service(json_config));
        },

        "service_configuration",
        [](const qtplugin::IServicePlugin& self, sol::this_state s) {
            sol::state_view lua(s);
            auto config = self.service_configuration();
            sol::table result = lua.create_table();

            // Convert QJsonObject to Lua table
            for (auto it = config.begin(); it != config.end(); ++it) {
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
}

}  // namespace qtforge_lua
