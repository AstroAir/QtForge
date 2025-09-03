/**
 * @file qtforge_lua.cpp
 * @brief Minimal Lua module entry point for QtForge
 * @version 3.2.0
 */

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include <memory>
#include <string>

namespace qtforge_lua {

// Forward declarations
void register_core_bindings(sol::state& lua);
void register_utils_bindings(sol::state& lua);

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Global Lua state for QtForge bindings
 */
static std::unique_ptr<sol::state> g_lua_state;

/**
 * @brief Initialize QtForge Lua bindings
 */
bool initialize_qtforge_lua() {
    try {
        g_lua_state = std::make_unique<sol::state>();
        
        // Open standard Lua libraries
        g_lua_state->open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::coroutine,
            sol::lib::string,
            sol::lib::os,
            sol::lib::math,
            sol::lib::table,
            sol::lib::debug,
            sol::lib::bit32,
            sol::lib::io,
            sol::lib::utf8
        );
        
        // Set up QtForge module table
        sol::table qtforge = g_lua_state->create_table();
        (*g_lua_state)["qtforge"] = qtforge;
        
        // Add version information
        qtforge["version"] = "3.2.0";
        qtforge["version_major"] = 3;
        qtforge["version_minor"] = 2;
        qtforge["version_patch"] = 0;
        
        // Add logging function
        qtforge["log"] = [](const std::string& message) {
            // Simple console output instead of Qt logging
            printf("Lua: %s\n", message.c_str());
        };
        
        // Register bindings
        register_core_bindings(*g_lua_state);
        register_utils_bindings(*g_lua_state);
        
        return true;
        
    } catch (const std::exception& e) {
        printf("Failed to initialize QtForge Lua bindings: %s\n", e.what());
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
