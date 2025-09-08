/**
 * @file marketplace_bindings.cpp
 * @brief Marketplace bindings for Lua (stub implementation)
 * @version 3.2.0
 */

#include <QDebug>
#include <QLoggingCategory>
#include <cstdio>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include <qtplugin/marketplace/plugin_marketplace.hpp>
#include "../qt_conversions.cpp"

Q_LOGGING_CATEGORY(marketplaceBindingsLog, "qtforge.lua.marketplace");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register PluginInfo and related types for marketplace
 */
void register_marketplace_types_bindings(sol::state& lua) {
    // Plugin category enum
    lua.new_enum<qtplugin::PluginCategory>("PluginCategory", {
        {"Utility", qtplugin::PluginCategory::Utility},
        {"Development", qtplugin::PluginCategory::Development},
        {"Graphics", qtplugin::PluginCategory::Graphics},
        {"Audio", qtplugin::PluginCategory::Audio},
        {"Video", qtplugin::PluginCategory::Video},
        {"Network", qtplugin::PluginCategory::Network},
        {"Database", qtplugin::PluginCategory::Database},
        {"Security", qtplugin::PluginCategory::Security},
        {"System", qtplugin::PluginCategory::System},
        {"Game", qtplugin::PluginCategory::Game},
        {"Education", qtplugin::PluginCategory::Education},
        {"Business", qtplugin::PluginCategory::Business},
        {"Other", qtplugin::PluginCategory::Other}
    });

    // Plugin rating struct
    auto rating_type = lua.new_usertype<qtplugin::PluginRating>("PluginRating");
    rating_type["average"] = &qtplugin::PluginRating::average;
    rating_type["count"] = &qtplugin::PluginRating::count;
    rating_type["five_star"] = &qtplugin::PluginRating::five_star;
    rating_type["four_star"] = &qtplugin::PluginRating::four_star;
    rating_type["three_star"] = &qtplugin::PluginRating::three_star;
    rating_type["two_star"] = &qtplugin::PluginRating::two_star;
    rating_type["one_star"] = &qtplugin::PluginRating::one_star;

    // Plugin download info
    auto download_type = lua.new_usertype<qtplugin::PluginDownloadInfo>("PluginDownloadInfo");
    download_type["url"] = &qtplugin::PluginDownloadInfo::url;
    download_type["size_bytes"] = &qtplugin::PluginDownloadInfo::size_bytes;
    download_type["checksum"] = &qtplugin::PluginDownloadInfo::checksum;
    download_type["signature"] = &qtplugin::PluginDownloadInfo::signature;

    qCDebug(marketplaceBindingsLog) << "Marketplace types bindings registered";
}

void register_marketplace_bindings(sol::state& lua) {
    qCDebug(marketplaceBindingsLog) << "Registering marketplace bindings...";

    // Create qtforge.marketplace namespace
    sol::table qtforge = lua["qtforge"];
    sol::table marketplace = qtforge.get_or_create<sol::table>("marketplace");

    // Register marketplace types
    register_marketplace_types_bindings(lua);

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

    // Factory functions
    marketplace["create_marketplace"] = [](const std::string& url) {
        return std::make_shared<qtplugin::PluginMarketplace>(QString::fromStdString(url));
    };

    marketplace["create_rating"] = [](double average, int count) {
        qtplugin::PluginRating rating;
        rating.average = average;
        rating.count = count;
        return rating;
    };

    marketplace["create_download_info"] = [](const std::string& url, size_t size) {
        qtplugin::PluginDownloadInfo info;
        info.url = QString::fromStdString(url);
        info.size_bytes = size;
        return info;
    };

    // Utility functions
    marketplace["category_to_string"] = [](qtplugin::PluginCategory category) -> std::string {
        switch (category) {
            case qtplugin::PluginCategory::Utility: return "Utility";
            case qtplugin::PluginCategory::Development: return "Development";
            case qtplugin::PluginCategory::Graphics: return "Graphics";
            case qtplugin::PluginCategory::Audio: return "Audio";
            case qtplugin::PluginCategory::Video: return "Video";
            case qtplugin::PluginCategory::Network: return "Network";
            case qtplugin::PluginCategory::Database: return "Database";
            case qtplugin::PluginCategory::Security: return "Security";
            case qtplugin::PluginCategory::System: return "System";
            case qtplugin::PluginCategory::Game: return "Game";
            case qtplugin::PluginCategory::Education: return "Education";
            case qtplugin::PluginCategory::Business: return "Business";
            case qtplugin::PluginCategory::Other: return "Other";
            default: return "Unknown";
        }
    };

    marketplace["format_file_size"] = [](size_t bytes) -> std::string {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit = 0;
        double size = static_cast<double>(bytes);

        while (size >= 1024.0 && unit < 4) {
            size /= 1024.0;
            unit++;
        }

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%.1f %s", size, units[unit]);
        return std::string(buffer);
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
