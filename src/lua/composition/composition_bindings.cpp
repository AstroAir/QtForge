/**
 * @file composition_bindings.cpp
 * @brief Composition bindings for Lua (stub implementation)
 * @version 3.2.0
 */

#include <QDebug>
#include <QLoggingCategory>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include <qtplugin/composition/plugin_composition.hpp>
#include "../qt_conversions.hpp"

Q_LOGGING_CATEGORY(compositionBindingsLog, "qtforge.lua.composition");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

void register_composition_bindings(sol::state& lua) {
    qCDebug(compositionBindingsLog) << "Registering composition bindings...";

    // Create qtforge.composition namespace
    sol::table qtforge = lua["qtforge"];
    sol::table composition = qtforge["composition"].get_or_create<sol::table>();

    // Composition strategy enum
    lua.new_enum<qtplugin::CompositionStrategy>(
        "CompositionStrategy",
        {{"Aggregation", qtplugin::CompositionStrategy::Aggregation},
         {"Pipeline", qtplugin::CompositionStrategy::Pipeline},
         {"Facade", qtplugin::CompositionStrategy::Facade},
         {"Decorator", qtplugin::CompositionStrategy::Decorator},
         {"Proxy", qtplugin::CompositionStrategy::Proxy},
         {"Adapter", qtplugin::CompositionStrategy::Adapter},
         {"Bridge", qtplugin::CompositionStrategy::Bridge}});

    // Plugin role enum
    lua.new_enum<qtplugin::PluginRole>(
        "PluginRole", {{"Primary", qtplugin::PluginRole::Primary},
                       {"Secondary", qtplugin::PluginRole::Secondary},
                       {"Auxiliary", qtplugin::PluginRole::Auxiliary},
                       {"Decorator", qtplugin::PluginRole::Decorator},
                       {"Adapter", qtplugin::PluginRole::Adapter},
                       {"Bridge", qtplugin::PluginRole::Bridge}});

    // Plugin composition manager (singleton)
    auto manager_type =
        lua.new_usertype<qtplugin::CompositionManager>("CompositionManager");
    manager_type["register_composition"] =
        &qtplugin::CompositionManager::register_composition;
    manager_type["unregister_composition"] =
        &qtplugin::CompositionManager::unregister_composition;
    manager_type["get_composition"] =
        &qtplugin::CompositionManager::get_composition;
    manager_type["list_compositions"] =
        &qtplugin::CompositionManager::list_compositions;
    manager_type["create_composite_plugin"] =
        &qtplugin::CompositionManager::create_composite_plugin;
    manager_type["destroy_composite_plugin"] =
        &qtplugin::CompositionManager::destroy_composite_plugin;
    manager_type["list_composite_plugins"] =
        &qtplugin::CompositionManager::list_composite_plugins;
    manager_type["get_composite_plugin"] =
        &qtplugin::CompositionManager::get_composite_plugin;

    // Factory function for singleton access
    composition["get_composition_manager"] =
        []() -> qtplugin::CompositionManager& {
        return qtplugin::CompositionManager::instance();
    };

    qCDebug(compositionBindingsLog)
        << "Composition bindings registered successfully";
}

#else  // QTFORGE_LUA_BINDINGS not defined

namespace sol {
class state;
}

void register_composition_bindings(sol::state& lua) {
    (void)lua;
    qCWarning(compositionBindingsLog)
        << "Composition bindings not available - Lua support not compiled";
}

#endif  // QTFORGE_LUA_BINDINGS

}  // namespace qtforge_lua
