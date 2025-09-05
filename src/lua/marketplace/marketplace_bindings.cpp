/**
 * @file marketplace_bindings.cpp
 * @brief Marketplace bindings for Lua (stub implementation)
 * @version 3.2.0
 */

#include <QDebug>
#include <QLoggingCategory>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include "../../../include/qtplugin/marketplace/plugin_marketplace.hpp"
#include "../qt_conversions.cpp"

Q_LOGGING_CATEGORY(marketplaceBindingsLog, "qtforge.lua.marketplace");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

void register_marketplace_bindings(sol::state& lua) {
    qCDebug(marketplaceBindingsLog) << "Registering marketplace bindings...";

    // Create qtforge.marketplace namespace
    sol::table qtforge = lua["qtforge"];
    sol::table marketplace = qtforge.get_or_create<sol::table>("marketplace");

    // Plugin marketplace
    auto marketplace_type = lua.new_usertype<qtplugin::PluginMarketplace>("PluginMarketplace",
        sol::constructors<qtplugin::PluginMarketplace(const QString&, QObject*)>());

    marketplace_type["initialize"] = &qtplugin::PluginMarketplace::initialize;
    marketplace_type["search_plugins"] = &qtplugin::PluginMarketplace::search_plugins;
    marketplace_type["get_plugin_details"] = &qtplugin::PluginMarketplace::get_plugin_details;
    marketplace_type["get_plugin_reviews"] = &qtplugin::PluginMarketplace::get_plugin_reviews;
    marketplace_type["install_plugin"] = &qtplugin::PluginMarketplace::install_plugin;
    marketplace_type["update_plugin"] = &qtplugin::PluginMarketplace::update_plugin;
    marketplace_type["uninstall_plugin"] = &qtplugin::PluginMarketplace::uninstall_plugin;
    marketplace_type["get_installed_plugins"] = &qtplugin::PluginMarketplace::get_installed_plugins;
    marketplace_type["check_for_updates"] = &qtplugin::PluginMarketplace::check_for_updates;
    marketplace_type["get_categories"] = &qtplugin::PluginMarketplace::get_categories;
    marketplace_type["get_featured_plugins"] = &qtplugin::PluginMarketplace::get_featured_plugins;
    marketplace_type["set_api_key"] = &qtplugin::PluginMarketplace::set_api_key;
    marketplace_type["is_authenticated"] = &qtplugin::PluginMarketplace::is_authenticated;

    // Factory function
    marketplace["create_marketplace"] = [](const std::string& url) {
        return std::make_shared<qtplugin::PluginMarketplace>(QString::fromStdString(url));
    };

    qCDebug(marketplaceBindingsLog) << "Marketplace bindings registered successfully";
}

#else // QTFORGE_LUA_BINDINGS not defined

namespace sol { class state; }

void register_marketplace_bindings(sol::state& lua) {
    (void)lua;
    qCWarning(marketplaceBindingsLog) << "Marketplace bindings not available - Lua support not compiled";
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
