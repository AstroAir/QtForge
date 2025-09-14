/**
 * @file core_bindings.cpp
 * @brief Minimal core plugin system bindings for Lua
 * @version 3.2.0
 */

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include "../../../include/qtplugin/utils/version.hpp"

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register Version class with Lua
 */
void register_version_bindings(sol::state& lua) {
    auto version_type = lua.new_usertype<qtplugin::Version>("Version",
        sol::constructors<qtplugin::Version(int, int, int)>()
    );

    version_type["major"] = &qtplugin::Version::major;
    version_type["minor"] = &qtplugin::Version::minor;
    version_type["patch"] = &qtplugin::Version::patch;
    version_type["to_string"] = &qtplugin::Version::to_string;
    version_type["is_compatible_with"] = &qtplugin::Version::is_compatible_with;

    // String representation
    version_type[sol::meta_function::to_string] = [](const qtplugin::Version& v) {
        return v.to_string();
    };
}

/**
 * @brief Register minimal core bindings
 */
void register_core_bindings(sol::state& lua) {
    // Create qtforge.core namespace
    sol::table qtforge = lua["qtforge"];
    sol::table core = qtforge.get_or_create<sol::table>("core");

    // Register only the Version class for now
    register_version_bindings(lua);

    // Add convenience functions to core namespace
    core["version"] = [](int major, int minor, int patch) {
        return qtplugin::Version{major, minor, patch};
    };

    // Test function
    core["test_function"] = []() -> std::string {
        return "QtForge Lua core bindings are working!";
    };

    // Simple math function
    core["add"] = [](int a, int b) -> int {
        return a + b;
    };
}

#else // QTFORGE_LUA_BINDINGS not defined

// Forward declare sol::state for when Lua is not available
namespace sol { class state; }

void register_core_bindings(sol::state& lua) {
    (void)lua; // Suppress unused parameter warning
    // No-op when Lua bindings are not available
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
