/**
 * @file error_handling_bindings.cpp
 * @brief Error handling and expected<T,E> bindings for Lua
 * @version 3.2.0
 */

#include <QDebug>
#include <QLoggingCategory>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include <qtplugin/utils/error_handling.hpp>
#include <qtplugin/core/plugin_interface.hpp>
#include "../qt_conversions.cpp"

Q_LOGGING_CATEGORY(errorHandlingBindingsLog, "qtforge.lua.error");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Lua wrapper for expected<T,E> results
 */
template<typename T>
struct LuaResult {
    bool has_value;
    T value;
    qtplugin::PluginError error;

    LuaResult(const T& val) : has_value(true), value(val) {}
    LuaResult(const qtplugin::PluginError& err) : has_value(false), error(err) {}

    template<typename U>
    LuaResult(const qtplugin::expected<U, qtplugin::PluginError>& expected) {
        if (expected) {
            has_value = true;
            if constexpr (std::is_same_v<T, U>) {
                value = expected.value();
            } else if constexpr (std::is_same_v<T, bool> && std::is_same_v<U, void>) {
                value = true;
            }
        } else {
            has_value = false;
            error = expected.error();
        }
    }
};

/**
 * @brief Register error handling utilities
 */
void register_error_handling_bindings(sol::state& lua) {
    sol::table qtforge = lua["qtforge"];
    sol::table error_ns = qtforge.get_or_create<sol::table>("error");

    // Error creation functions
    error_ns["create"] = [](qtplugin::PluginErrorCode code, const std::string& message) {
        return qtplugin::PluginError{code, message};
    };

    error_ns["create_simple"] = [](const std::string& message) {
        return qtplugin::PluginError{qtplugin::PluginErrorCode::UnknownError, message};
    };

    // Error code utilities
    error_ns["code_to_string"] = [](qtplugin::PluginErrorCode code) -> std::string {
        switch (code) {
            case qtplugin::PluginErrorCode::None: return "None";
            case qtplugin::PluginErrorCode::UnknownError: return "UnknownError";
            case qtplugin::PluginErrorCode::InvalidParameter: return "InvalidParameter";
            case qtplugin::PluginErrorCode::InvalidState: return "InvalidState";
            case qtplugin::PluginErrorCode::NotFound: return "NotFound";
            case qtplugin::PluginErrorCode::AlreadyExists: return "AlreadyExists";
            case qtplugin::PluginErrorCode::LoadFailed: return "LoadFailed";
            case qtplugin::PluginErrorCode::InitializationFailed: return "InitializationFailed";
            case qtplugin::PluginErrorCode::ExecutionFailed: return "ExecutionFailed";
            case qtplugin::PluginErrorCode::ConfigurationError: return "ConfigurationError";
            case qtplugin::PluginErrorCode::DependencyError: return "DependencyError";
            case qtplugin::PluginErrorCode::SecurityError: return "SecurityError";
            case qtplugin::PluginErrorCode::NetworkError: return "NetworkError";
            case qtplugin::PluginErrorCode::FileSystemError: return "FileSystemError";
            case qtplugin::PluginErrorCode::DatabaseError: return "DatabaseError";
            case qtplugin::PluginErrorCode::TimeoutError: return "TimeoutError";
            case qtplugin::PluginErrorCode::PermissionDenied: return "PermissionDenied";
            case qtplugin::PluginErrorCode::ResourceExhausted: return "ResourceExhausted";
            case qtplugin::PluginErrorCode::NotSupported: return "NotSupported";
            case qtplugin::PluginErrorCode::NotImplemented: return "NotImplemented";
            case qtplugin::PluginErrorCode::CommandNotFound: return "CommandNotFound";
            default: return "Unknown";
        }
    };

    error_ns["string_to_code"] = [](const std::string& code_str) -> qtplugin::PluginErrorCode {
        if (code_str == "None") return qtplugin::PluginErrorCode::None;
        if (code_str == "UnknownError") return qtplugin::PluginErrorCode::UnknownError;
        if (code_str == "InvalidParameter") return qtplugin::PluginErrorCode::InvalidParameter;
        if (code_str == "InvalidState") return qtplugin::PluginErrorCode::InvalidState;
        if (code_str == "NotFound") return qtplugin::PluginErrorCode::NotFound;
        if (code_str == "AlreadyExists") return qtplugin::PluginErrorCode::AlreadyExists;
        if (code_str == "LoadFailed") return qtplugin::PluginErrorCode::LoadFailed;
        if (code_str == "InitializationFailed") return qtplugin::PluginErrorCode::InitializationFailed;
        if (code_str == "ExecutionFailed") return qtplugin::PluginErrorCode::ExecutionFailed;
        if (code_str == "ConfigurationError") return qtplugin::PluginErrorCode::ConfigurationError;
        if (code_str == "DependencyError") return qtplugin::PluginErrorCode::DependencyError;
        if (code_str == "SecurityError") return qtplugin::PluginErrorCode::SecurityError;
        if (code_str == "NetworkError") return qtplugin::PluginErrorCode::NetworkError;
        if (code_str == "FileSystemError") return qtplugin::PluginErrorCode::FileSystemError;
        if (code_str == "DatabaseError") return qtplugin::PluginErrorCode::DatabaseError;
        if (code_str == "TimeoutError") return qtplugin::PluginErrorCode::TimeoutError;
        if (code_str == "PermissionDenied") return qtplugin::PluginErrorCode::PermissionDenied;
        if (code_str == "ResourceExhausted") return qtplugin::PluginErrorCode::ResourceExhausted;
        if (code_str == "NotSupported") return qtplugin::PluginErrorCode::NotSupported;
        if (code_str == "NotImplemented") return qtplugin::PluginErrorCode::NotImplemented;
        if (code_str == "CommandNotFound") return qtplugin::PluginErrorCode::CommandNotFound;
        return qtplugin::PluginErrorCode::UnknownError;
    };

    // Result wrapper types
    auto bool_result_type = lua.new_usertype<LuaResult<bool>>("BoolResult");
    bool_result_type["has_value"] = &LuaResult<bool>::has_value;
    bool_result_type["value"] = sol::property([](const LuaResult<bool>& result) -> bool {
        return result.has_value ? result.value : false;
    });
    bool_result_type["error"] = sol::property([](const LuaResult<bool>& result) -> qtplugin::PluginError {
        return result.error;
    });
    bool_result_type["is_ok"] = [](const LuaResult<bool>& result) -> bool {
        return result.has_value;
    };
    bool_result_type["is_error"] = [](const LuaResult<bool>& result) -> bool {
        return !result.has_value;
    };

    auto string_result_type = lua.new_usertype<LuaResult<std::string>>("StringResult");
    string_result_type["has_value"] = &LuaResult<std::string>::has_value;
    string_result_type["value"] = sol::property([](const LuaResult<std::string>& result) -> std::string {
        return result.has_value ? result.value : "";
    });
    string_result_type["error"] = sol::property([](const LuaResult<std::string>& result) -> qtplugin::PluginError {
        return result.error;
    });
    string_result_type["is_ok"] = [](const LuaResult<std::string>& result) -> bool {
        return result.has_value;
    };
    string_result_type["is_error"] = [](const LuaResult<std::string>& result) -> bool {
        return !result.has_value;
    };

    auto json_result_type = lua.new_usertype<LuaResult<QJsonObject>>("JsonResult");
    json_result_type["has_value"] = &LuaResult<QJsonObject>::has_value;
    json_result_type["value"] = sol::property([&lua](const LuaResult<QJsonObject>& result) -> sol::object {
        if (result.has_value) {
            return qtforge_lua::qjson_to_lua(result.value, lua);
        } else {
            return sol::nil;
        }
    });
    json_result_type["error"] = sol::property([](const LuaResult<QJsonObject>& result) -> qtplugin::PluginError {
        return result.error;
    });
    json_result_type["is_ok"] = [](const LuaResult<QJsonObject>& result) -> bool {
        return result.has_value;
    };
    json_result_type["is_error"] = [](const LuaResult<QJsonObject>& result) -> bool {
        return !result.has_value;
    };

    // Result creation functions
    error_ns["ok_bool"] = [](bool value) {
        return LuaResult<bool>(value);
    };

    error_ns["ok_string"] = [](const std::string& value) {
        return LuaResult<std::string>(value);
    };

    error_ns["ok_json"] = [](const sol::object& value) {
        QJsonObject json_obj;
        if (value.get_type() == sol::type::table) {
            QJsonValue json_value = qtforge_lua::lua_to_qjson(value);
            if (json_value.isObject()) {
                json_obj = json_value.toObject();
            }
        }
        return LuaResult<QJsonObject>(json_obj);
    };

    error_ns["error_bool"] = [](const qtplugin::PluginError& error) {
        return LuaResult<bool>(error);
    };

    error_ns["error_string"] = [](const qtplugin::PluginError& error) {
        return LuaResult<std::string>(error);
    };

    error_ns["error_json"] = [](const qtplugin::PluginError& error) {
        return LuaResult<QJsonObject>(error);
    };

    // Error handling utilities
    error_ns["try_call"] = [&lua](const sol::function& func) -> sol::object {
        try {
            auto result = func();
            sol::table success_result = lua.create_table();
            success_result["success"] = true;
            success_result["result"] = result;
            return success_result;
        } catch (const std::exception& e) {
            sol::table error_result = lua.create_table();
            error_result["success"] = false;
            error_result["error"] = qtplugin::PluginError{qtplugin::PluginErrorCode::ExecutionFailed, e.what()};
            return error_result;
        } catch (...) {
            sol::table error_result = lua.create_table();
            error_result["success"] = false;
            error_result["error"] = qtplugin::PluginError{qtplugin::PluginErrorCode::UnknownError, "Unknown exception"};
            return error_result;
        }
    };

    error_ns["assert_ok"] = [](const sol::object& result, const std::string& message) {
        if (result.get_type() == sol::type::table) {
            sol::table table = result.as<sol::table>();
            bool has_value = table.get_or("has_value", false);
            if (!has_value) {
                std::string error_msg = message.empty() ? "Assertion failed" : message;
                throw std::runtime_error(error_msg);
            }
        }
    };

    qCDebug(errorHandlingBindingsLog) << "Error handling bindings registered";
}

/**
 * @brief Register all error handling bindings
 */
void register_error_bindings(sol::state& lua) {
    qCDebug(errorHandlingBindingsLog) << "Registering error handling bindings...";

    register_error_handling_bindings(lua);

    qCDebug(errorHandlingBindingsLog) << "Error handling bindings registered successfully";
}

#else // QTFORGE_LUA_BINDINGS not defined

// Forward declare sol::state for when Lua is not available
namespace sol { class state; }

void register_error_bindings(sol::state& lua) {
    (void)lua; // Suppress unused parameter warning
    qCWarning(errorHandlingBindingsLog) << "Error handling bindings not available - Lua support not compiled";
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
