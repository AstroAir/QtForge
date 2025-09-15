/**
 * @file qtforge_lua.cpp
 * @brief Minimal Lua module entry point for QtForge
 * @version 3.2.0
 */

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#include <lua.hpp>  // for lua_State and luaL_error
#endif

#include <memory>
#include <string>
#include <cstring>  // for strncpy

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

// Forward declarations
void register_core_bindings(sol::state_view& lua);
void register_utils_bindings(sol::state_view& lua);
void register_security_bindings(sol::state& lua);
void register_communication_bindings(sol::state& lua);
void register_managers_bindings(sol::state& lua);
void register_orchestration_bindings(sol::state& lua);
void register_monitoring_bindings(sol::state& lua);
void register_threading_bindings(sol::state& lua);
void register_transaction_bindings(sol::state& lua);
void register_composition_bindings(sol::state& lua);


/**
 * @brief Global Lua state for QtForge bindings
 */
static std::unique_ptr<sol::state> g_lua_state;

/**
 * @brief Initialize QtForge Lua bindings
 */
bool initialize_qtforge_lua() {
    try {
        printf("Creating Lua state...\n");
        g_lua_state = std::make_unique<sol::state>();
        printf("Lua state created successfully\n");

        // Open standard Lua libraries
        printf("Opening Lua libraries...\n");
        g_lua_state->open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::string,
            sol::lib::math,
            sol::lib::table
        );
        printf("Lua libraries opened successfully\n");

        // Set up QtForge module table
        printf("Creating QtForge module table...\n");
        sol::table qtforge = g_lua_state->create_table();
        (*g_lua_state)["qtforge"] = qtforge;
        printf("QtForge module table created\n");

        // Add version information
        printf("Adding version information...\n");
        qtforge["version"] = "3.2.0";
        qtforge["version_major"] = 3;
        qtforge["version_minor"] = 2;
        qtforge["version_patch"] = 0;
        printf("Version information added\n");

        // Add logging function
        printf("Adding logging function...\n");
        qtforge["log"] = [](const std::string& message) {
            printf("Lua: %s\n", message.c_str());
        };
        printf("Logging function added\n");

        // Register bindings
        printf("Registering core bindings...\n");
        register_core_bindings(*g_lua_state);
        printf("Core bindings registered\n");

        printf("Registering utils bindings...\n");
        register_utils_bindings(*g_lua_state);
        printf("Utils bindings registered\n");

        printf("Registering security bindings...\n");
        register_security_bindings(*g_lua_state);
        printf("Security bindings registered\n");

        printf("Registering communication bindings...\n");
        register_communication_bindings(*g_lua_state);
        printf("Communication bindings registered\n");

        printf("Registering managers bindings...\n");
        register_managers_bindings(*g_lua_state);
        printf("Managers bindings registered\n");

        printf("Registering orchestration bindings...\n");
        register_orchestration_bindings(*g_lua_state);
        printf("Orchestration bindings registered\n");

        printf("Registering monitoring bindings...\n");
        register_monitoring_bindings(*g_lua_state);
        printf("Monitoring bindings registered\n");

        printf("Registering threading bindings...\n");
        register_threading_bindings(*g_lua_state);
        printf("Threading bindings registered\n");

        printf("Registering transaction bindings...\n");
        register_transaction_bindings(*g_lua_state);
        printf("Transaction bindings registered\n");

        printf("Registering composition bindings...\n");
        register_composition_bindings(*g_lua_state);
        printf("Composition bindings registered\n");



        printf("QtForge Lua bindings initialized successfully!\n");
        return true;

    } catch (const std::exception& e) {
        printf("Failed to initialize QtForge Lua bindings: %s\n", e.what());
        return false;
    } catch (...) {
        printf("Failed to initialize QtForge Lua bindings: Unknown error\n");
        return false;
    }
}

/**
 * @brief Shutdown QtForge Lua bindings
 */
void shutdown_qtforge_lua() {
    try {
        g_lua_state.reset();
    } catch (const std::exception& e) {
        printf("Error during QtForge Lua shutdown: %s\n", e.what());
    }
}

/**
 * @brief Get global Lua state
 */
sol::state* get_lua_state() {
    return g_lua_state.get();
}

/**
 * @brief Execute Lua code
 */
bool execute_lua_code(const std::string& code, std::string& error_message) {
    if (!g_lua_state) {
        error_message = "Lua state not initialized";
        return false;
    }

    try {
        sol::protected_function_result result = g_lua_state->safe_script(code);
        if (!result.valid()) {
            sol::error err = result;
            error_message = err.what();
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        error_message = e.what();
        return false;
    }
}

/**
 * @brief Load Lua file
 */
bool load_lua_file(const std::string& file_path, std::string& error_message) {
    if (!g_lua_state) {
        error_message = "Lua state not initialized";
        return false;
    }

    try {
        sol::protected_function_result result = g_lua_state->safe_script_file(file_path);
        if (!result.valid()) {
            sol::error err = result;
            error_message = err.what();
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        error_message = e.what();
        return false;
    }
}

#else // QTFORGE_LUA_BINDINGS not defined

// Forward declare sol::state for when Lua is not available
namespace sol { class state; }

bool initialize_qtforge_lua() {
    printf("QtForge Lua bindings not compiled in this build\n");
    return false;
}

void shutdown_qtforge_lua() {
    // No-op when Lua bindings are not available
}

sol::state* get_lua_state() {
    return nullptr;
}

bool execute_lua_code(const std::string& code, std::string& error_message) {
    (void)code;
    error_message = "Lua bindings not compiled in this build";
    return false;
}

bool load_lua_file(const std::string& file_path, std::string& error_message) {
    (void)file_path;
    error_message = "Lua bindings not compiled in this build";
    return false;
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua

// C-style API for external usage
extern "C" {

/**
 * @brief Initialize QtForge Lua module
 */
int qtforge_lua_init() {
    return qtforge_lua::initialize_qtforge_lua() ? 1 : 0;
}

/**
 * @brief Shutdown QtForge Lua module
 */
void qtforge_lua_shutdown() {
    qtforge_lua::shutdown_qtforge_lua();
}

/**
 * @brief Execute Lua code
 */
int qtforge_lua_execute(const char* code, char* error_buffer, size_t buffer_size) {
    if (!code) {
        if (error_buffer && buffer_size > 0) {
            strncpy(error_buffer, "Invalid code parameter", buffer_size - 1);
            error_buffer[buffer_size - 1] = '\0';
        }
        return 0;
    }

    std::string error_message;
    bool success = qtforge_lua::execute_lua_code(code, error_message);

    if (!success && error_buffer && buffer_size > 0) {
        strncpy(error_buffer, error_message.c_str(), buffer_size - 1);
        error_buffer[buffer_size - 1] = '\0';
    }

    return success ? 1 : 0;
}

/**
 * @brief Load Lua file
 */
int qtforge_lua_load_file(const char* file_path, char* error_buffer, size_t buffer_size) {
    if (!file_path) {
        if (error_buffer && buffer_size > 0) {
            strncpy(error_buffer, "Invalid file path parameter", buffer_size - 1);
            error_buffer[buffer_size - 1] = '\0';
        }
        return 0;
    }

    std::string error_message;
    bool success = qtforge_lua::load_lua_file(file_path, error_message);

    if (!success && error_buffer && buffer_size > 0) {
        strncpy(error_buffer, error_message.c_str(), buffer_size - 1);
        error_buffer[buffer_size - 1] = '\0';
    }

    return success ? 1 : 0;
}

} // extern "C"

#ifdef QTFORGE_LUA_BINDINGS
/**
 * @brief Lua module entry point
 */
extern "C" int luaopen_qtforge(lua_State* L) {
    // Create a sol::state_view from the provided Lua state
    sol::state_view lua(L);

    // Create QtForge module table directly in the calling Lua state
    sol::table qtforge = lua.create_table();

    // Set the qtforge table in the global state so bindings can find it
    lua["qtforge"] = qtforge;

    // Add version information
    qtforge["version"] = "3.2.0";
    qtforge["version_major"] = 3;
    qtforge["version_minor"] = 2;
    qtforge["version_patch"] = 0;

    // Add logging function
    qtforge["log"] = [](const std::string& message) {
        printf("Lua: %s\n", message.c_str());
    };

    // Register bindings directly in this Lua state
    qtforge_lua::register_core_bindings(lua);
    qtforge_lua::register_utils_bindings(lua);
    qtforge_lua::register_security_bindings(lua);
    qtforge_lua::register_communication_bindings(lua);
    qtforge_lua::register_managers_bindings(lua);
    qtforge_lua::register_orchestration_bindings(lua);
    qtforge_lua::register_monitoring_bindings(lua);
    qtforge_lua::register_threading_bindings(lua);
    qtforge_lua::register_transaction_bindings(lua);
    qtforge_lua::register_composition_bindings(lua);


    // Push the qtforge table onto the stack
    qtforge.push(L);

    return 1;  // Return the qtforge table
}
#endif // QTFORGE_LUA_BINDINGS
