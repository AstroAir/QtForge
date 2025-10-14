/**
 * @file communication_bindings.cpp
 * @brief Communication system bindings for Lua
 * @version 3.2.0
 */

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include <qtplugin/communication/message_bus.hpp>
#include <qtplugin/communication/plugin_service_contracts.hpp>
#include <qtplugin/communication/request_response_system.hpp>
#include "../qt_conversions.hpp"

Q_LOGGING_CATEGORY(communicationBindingsLog, "qtforge.lua.communication");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register Message and MessageBus bindings
 */
void register_message_bus_bindings(sol::state& lua) {
    Q_UNUSED(lua)
    // TODO: Lua bindings need to be redesigned to work with the template-based
    // message system The current implementation has issues with template types
    // that can't be directly bound to Lua For now, these bindings are disabled
    // pending a proper redesign

    qCDebug(communicationBindingsLog)
        << "MessageBus bindings temporarily disabled - needs redesign for "
           "template system";
}

/**
 * @brief Register Request-Response bindings
 */
void register_request_response_bindings(sol::state& lua) {
    Q_UNUSED(lua)
    // TODO: Request-Response bindings also need redesign
    qCDebug(communicationBindingsLog)
        << "Request-Response bindings temporarily disabled - needs redesign";
}

/**
 * @brief Register all communication bindings
 */
void register_communication_bindings(sol::state& lua) {
    Q_UNUSED(lua)
    qCDebug(communicationBindingsLog)
        << "Communication bindings temporarily disabled - needs redesign for "
           "template system";

    // TODO: All communication bindings need to be redesigned to work with the
    // new template-based message system The current implementation has issues
    // with template types that can't be directly bound to Lua

    // Register disabled bindings (they will log their disabled status)
    register_message_bus_bindings(lua);
    register_request_response_bindings(lua);
}

#else  // QTFORGE_LUA_BINDINGS not defined

// Forward declare sol::state for when Lua is not available
namespace sol {
class state;
}

void register_communication_bindings(sol::state& lua) {
    (void)lua;  // Suppress unused parameter warning
    qCWarning(communicationBindingsLog)
        << "Communication bindings not available - Lua support not compiled";
}

#endif  // QTFORGE_LUA_BINDINGS

}  // namespace qtforge_lua
