/**
 * @file qtforge_lua_bindings.hpp
 * @brief Forward declarations for QtForge Lua binding functions
 * @version 3.2.0
 */

#pragma once

#ifdef QTFORGE_LUA_BINDINGS
// Only include sol2 when Lua bindings are enabled
#include <sol/sol.hpp>

namespace qtforge_lua {

// Core bindings
void register_core_bindings(sol::state& lua);
void register_utils_bindings(sol::state& lua);
void register_ui_bindings(sol::state& lua);

// Advanced bindings (currently disabled due to compilation issues)
void register_security_bindings(sol::state& lua);
void register_communication_bindings(sol::state& lua);
void register_managers_bindings(sol::state& lua);
void register_orchestration_bindings(sol::state& lua);
void register_monitoring_bindings(sol::state& lua);
void register_threading_bindings(sol::state& lua);
void register_transaction_bindings(sol::state& lua);
void register_composition_bindings(sol::state& lua);

}  // namespace qtforge_lua

#else  // QTFORGE_LUA_BINDINGS not defined

// When Lua bindings are not available, we need to provide stub declarations
// without including sol2 headers. We use a forward declaration approach
// that avoids the problematic sol::state type.

namespace qtforge_lua {

// Stub declarations that don't require sol2 types
void register_core_bindings_stub();
void register_utils_bindings_stub();
void register_security_bindings_stub();
void register_communication_bindings_stub();
void register_managers_bindings_stub();
void register_orchestration_bindings_stub();
void register_monitoring_bindings_stub();
void register_threading_bindings_stub();
void register_transaction_bindings_stub();
void register_composition_bindings_stub();

}  // namespace qtforge_lua

#endif  // QTFORGE_LUA_BINDINGS
