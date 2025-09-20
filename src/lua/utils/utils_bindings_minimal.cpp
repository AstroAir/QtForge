/**
 * @file utils_bindings.cpp
 * @brief Minimal utility bindings for Lua
 * @version 3.2.0
 */

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include <string>

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register minimal utils bindings
 */
void register_utils_bindings(sol::state& lua) {
    // Create qtforge.utils namespace
    sol::table qtforge = lua["qtforge"];
    sol::table utils = qtforge["utils"].get_or_create<sol::table>();

    // Test function
    utils["utils_test"] = []() -> std::string {
        return "Utils module working!";
    };

    // Version creation function
    utils["create_version"] = [](int major, int minor, int patch) -> std::string {
        return "Version " + std::to_string(major) + "." + 
               std::to_string(minor) + "." + std::to_string(patch);
    };

    // Version parsing function
    utils["parse_version"] = [](const std::string& version_string) -> std::string {
        return "Parsed version: " + version_string;
    };

    // Error creation function
    utils["create_error"] = [](int code, const std::string& message) -> std::string {
        return "Error " + std::to_string(code) + ": " + message;
    };
}

#else // QTFORGE_LUA_BINDINGS not defined

// Forward declare sol::state for when Lua is not available
namespace sol { class state; }

void register_utils_bindings(sol::state& lua) {
    (void)lua; // Suppress unused parameter warning
    // No-op when Lua bindings are not available
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
