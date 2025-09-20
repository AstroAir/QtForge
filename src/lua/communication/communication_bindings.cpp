/**
 * @file communication_bindings.cpp
 * @brief Communication system bindings for Lua
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

#include <qtplugin/communication/message_bus.hpp>
#include <qtplugin/communication/request_response.hpp>
#include <qtplugin/communication/plugin_communication.hpp>
#include "../qt_conversions.cpp"

Q_LOGGING_CATEGORY(communicationBindingsLog, "qtforge.lua.communication");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register Message and MessageBus bindings
 */
void register_message_bus_bindings(sol::state& lua) {
    // Message type
    auto message_type = lua.new_usertype<qtplugin::Message>("Message",
        sol::constructors<qtplugin::Message(), qtplugin::Message(const std::string&, const QJsonObject&)>()
    );

    message_type["id"] = &qtplugin::Message::id;
    message_type["topic"] = &qtplugin::Message::topic;
    message_type["sender_id"] = &qtplugin::Message::sender_id;
    message_type["priority"] = &qtplugin::Message::priority;

    // Payload (QJsonObject)
    message_type["payload"] = sol::property(
        [&lua](const qtplugin::Message& message) -> sol::object {
            return qtforge_lua::qjson_to_lua(message.payload, lua);
        },
        [](qtplugin::Message& message, const sol::object& payload) {
            if (payload.get_type() == sol::type::table) {
                QJsonValue json_value = qtforge_lua::lua_to_qjson(payload);
                if (json_value.isObject()) {
                    message.payload = json_value.toObject();
                }
            }
        }
    );

    // Timestamp
    message_type["timestamp"] = sol::property(
        [](const qtplugin::Message& message) -> double {
            auto duration = message.timestamp.time_since_epoch();
            return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
        }
    );

    // Headers (QJsonObject)
    message_type["headers"] = sol::property(
        [&lua](const qtplugin::Message& message) -> sol::object {
            return qtforge_lua::qjson_to_lua(message.headers, lua);
        },
        [](qtplugin::Message& message, const sol::object& headers) {
            if (headers.get_type() == sol::type::table) {
                QJsonValue json_value = qtforge_lua::lua_to_qjson(headers);
                if (json_value.isObject()) {
                    message.headers = json_value.toObject();
                }
            }
        }
    );

    message_type["to_json"] = [&lua](const qtplugin::Message& message) -> sol::object {
        return qtforge_lua::qjson_to_lua(message.to_json(), lua);
    };

    message_type["from_json"] = [](qtplugin::Message& message, const sol::object& json) {
        if (json.get_type() == sol::type::table) {
            QJsonValue json_value = qtforge_lua::lua_to_qjson(json);
            if (json_value.isObject()) {
                message.from_json(json_value.toObject());
            }
        }
    };

    // MessageBus type
    auto bus_type = lua.new_usertype<qtplugin::MessageBus>("MessageBus");

    bus_type["publish"] = [&lua](qtplugin::MessageBus& bus, const qtplugin::Message& message) -> sol::object {
        auto result = bus.publish(message);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    bus_type["publish_simple"] = [&lua](qtplugin::MessageBus& bus, const std::string& topic,
                                        const sol::object& payload) -> sol::object {
        QJsonObject json_payload;
        if (payload.get_type() == sol::type::table) {
            QJsonValue json_value = qtforge_lua::lua_to_qjson(payload);
            if (json_value.isObject()) {
                json_payload = json_value.toObject();
            }
        }

        qtplugin::Message message(topic, json_payload);
        auto result = bus.publish(message);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    bus_type["subscribe"] = [&lua](qtplugin::MessageBus& bus, const std::string& topic,
                                   const sol::function& callback) -> sol::object {
        auto handler = [callback, &lua](const qtplugin::Message& message) {
            try {
                sol::object lua_message = sol::make_object(lua, message);
                callback(lua_message);
            } catch (const std::exception& e) {
                qCWarning(communicationBindingsLog) << "Error in Lua message handler:" << e.what();
            }
        };

        auto result = bus.subscribe(topic, handler);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    bus_type["unsubscribe"] = [&lua](qtplugin::MessageBus& bus, const std::string& subscription_id) -> sol::object {
        auto result = bus.unsubscribe(subscription_id);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    bus_type["get_topics"] = [&lua](qtplugin::MessageBus& bus) -> sol::object {
        auto topics = bus.get_topics();
        sol::table table = lua.create_table();
        for (size_t i = 0; i < topics.size(); ++i) {
            table[i + 1] = topics[i];
        }
        return table;
    };

    bus_type["get_subscriber_count"] = &qtplugin::MessageBus::get_subscriber_count;

    qCDebug(communicationBindingsLog) << "MessageBus bindings registered";
}

/**
 * @brief Register Request-Response bindings
 */
void register_request_response_bindings(sol::state& lua) {
    // Request type
    auto request_type = lua.new_usertype<qtplugin::Request>("Request",
        sol::constructors<qtplugin::Request(), qtplugin::Request(const std::string&, const QJsonObject&)>()
    );

    request_type["id"] = &qtplugin::Request::id;
    request_type["method"] = &qtplugin::Request::method;
    request_type["sender_id"] = &qtplugin::Request::sender_id;
    request_type["timeout_ms"] = &qtplugin::Request::timeout_ms;

    // Parameters (QJsonObject)
    request_type["parameters"] = sol::property(
        [&lua](const qtplugin::Request& request) -> sol::object {
            return qtforge_lua::qjson_to_lua(request.parameters, lua);
        },
        [](qtplugin::Request& request, const sol::object& params) {
            if (params.get_type() == sol::type::table) {
                QJsonValue json_value = qtforge_lua::lua_to_qjson(params);
                if (json_value.isObject()) {
                    request.parameters = json_value.toObject();
                }
            }
        }
    );

    // Response type
    auto response_type = lua.new_usertype<qtplugin::Response>("Response",
        sol::constructors<qtplugin::Response(), qtplugin::Response(const std::string&, const QJsonObject&)>()
    );

    response_type["request_id"] = &qtplugin::Response::request_id;
    response_type["success"] = &qtplugin::Response::success;
    response_type["error_code"] = &qtplugin::Response::error_code;
    response_type["error_message"] = &qtplugin::Response::error_message;

    // Result (QJsonObject)
    response_type["result"] = sol::property(
        [&lua](const qtplugin::Response& response) -> sol::object {
            return qtforge_lua::qjson_to_lua(response.result, lua);
        },
        [](qtplugin::Response& response, const sol::object& result) {
            if (result.get_type() == sol::type::table) {
                QJsonValue json_value = qtforge_lua::lua_to_qjson(result);
                if (json_value.isObject()) {
                    response.result = json_value.toObject();
                }
            }
        }
    );

    // RequestResponseManager type
    auto rr_manager_type = lua.new_usertype<qtplugin::RequestResponseManager>("RequestResponseManager");

    rr_manager_type["send_request"] = [&lua](qtplugin::RequestResponseManager& manager,
                                             const qtplugin::Request& request) -> sol::object {
        auto result = manager.send_request(request);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    rr_manager_type["send_request_simple"] = [&lua](qtplugin::RequestResponseManager& manager,
                                                    const std::string& method,
                                                    const sol::object& params) -> sol::object {
        QJsonObject json_params;
        if (params.get_type() == sol::type::table) {
            QJsonValue json_value = qtforge_lua::lua_to_qjson(params);
            if (json_value.isObject()) {
                json_params = json_value.toObject();
            }
        }

        qtplugin::Request request(method, json_params);
        auto result = manager.send_request(request);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    rr_manager_type["register_handler"] = [&lua](qtplugin::RequestResponseManager& manager,
                                                 const std::string& method,
                                                 const sol::function& handler) -> sol::object {
        auto cpp_handler = [handler, &lua](const qtplugin::Request& request) -> qtplugin::Response {
            try {
                sol::object lua_request = sol::make_object(lua, request);
                sol::object result = handler(lua_request);

                if (result.get_type() == sol::type::table) {
                    sol::table table = result.as<sol::table>();
                    qtplugin::Response response;
                    response.request_id = request.id;
                    response.success = table.get_or("success", true);

                    if (response.success) {
                        sol::object result_obj = table["result"];
                        if (result_obj.get_type() == sol::type::table) {
                            QJsonValue json_value = qtforge_lua::lua_to_qjson(result_obj);
                            if (json_value.isObject()) {
                                response.result = json_value.toObject();
                            }
                        }
                    } else {
                        response.error_message = table.get_or<std::string>("error", "Unknown error");
                    }

                    return response;
                } else {
                    qtplugin::Response error_response;
                    error_response.request_id = request.id;
                    error_response.success = false;
                    error_response.error_message = "Invalid response from Lua handler";
                    return error_response;
                }
            } catch (const std::exception& e) {
                qtplugin::Response error_response;
                error_response.request_id = request.id;
                error_response.success = false;
                error_response.error_message = std::string("Lua handler error: ") + e.what();
                return error_response;
            }
        };

        auto result = manager.register_handler(method, cpp_handler);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    rr_manager_type["unregister_handler"] = [&lua](qtplugin::RequestResponseManager& manager,
                                                   const std::string& method) -> sol::object {
        auto result = manager.unregister_handler(method);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    qCDebug(communicationBindingsLog) << "Request-Response bindings registered";
}

/**
 * @brief Register all communication bindings
 */
void register_communication_bindings(sol::state& lua) {
    qCDebug(communicationBindingsLog) << "Registering communication bindings...";

    // Create qtforge.communication namespace
    sol::table qtforge = lua["qtforge"];
    sol::table comm = qtforge["communication"].get_or_create<sol::table>();

    // Register all communication types
    register_message_bus_bindings(lua);
    register_request_response_bindings(lua);

    // Add convenience functions to communication namespace
    comm["create_message"] = [](const std::string& topic, const sol::object& payload) {
        QJsonObject json_payload;
        if (payload.get_type() == sol::type::table) {
            QJsonValue json_value = qtforge_lua::lua_to_qjson(payload);
            if (json_value.isObject()) {
                json_payload = json_value.toObject();
            }
        }
        return qtplugin::Message(topic, json_payload);
    };

    comm["create_request"] = [](const std::string& method, const sol::object& params) {
        QJsonObject json_params;
        if (params.get_type() == sol::type::table) {
            QJsonValue json_value = qtforge_lua::lua_to_qjson(params);
            if (json_value.isObject()) {
                json_params = json_value.toObject();
            }
        }
        return qtplugin::Request(method, json_params);
    };

    comm["create_success_response"] = [](const std::string& request_id, const sol::object& result) {
        QJsonObject json_result;
        if (result.get_type() == sol::type::table) {
            QJsonValue json_value = qtforge_lua::lua_to_qjson(result);
            if (json_value.isObject()) {
                json_result = json_value.toObject();
            }
        }

        qtplugin::Response response;
        response.request_id = request_id;
        response.success = true;
        response.result = json_result;
        return response;
    };

    comm["create_error_response"] = [](const std::string& request_id, const std::string& error_message) {
        qtplugin::Response response;
        response.request_id = request_id;
        response.success = false;
        response.error_message = error_message;
        return response;
    };

    qCDebug(communicationBindingsLog) << "Communication bindings registered successfully";
}

#else // QTFORGE_LUA_BINDINGS not defined

// Forward declare sol::state for when Lua is not available
namespace sol { class state; }

void register_communication_bindings(sol::state& lua) {
    (void)lua; // Suppress unused parameter warning
    qCWarning(communicationBindingsLog) << "Communication bindings not available - Lua support not compiled";
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
