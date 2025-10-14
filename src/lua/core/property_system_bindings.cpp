/**
 * @file property_system_bindings.cpp
 * @brief Lua bindings for PluginPropertySystem
 * @version 3.2.0
 * @author QtForge Development Team
 */

#include <qtplugin/core/plugin_property_system.hpp>
#include <qtplugin/interfaces/core/plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>
#include <sol/sol.hpp>
#include "../qtforge_lua_bindings.hpp"

#include <QJsonObject>
#include <QVariant>

namespace qtforge_lua {

// Helper to convert QVariant to Lua value
sol::object qvariant_to_lua(const QVariant& variant, sol::state_view& lua) {
    switch (variant.type()) {
        case QVariant::Invalid:
            return sol::lua_nil;
        case QVariant::Bool:
            return sol::make_object(lua, variant.toBool());
        case QVariant::Int:
            return sol::make_object(lua, variant.toInt());
        case QVariant::UInt:
            return sol::make_object(lua, variant.toUInt());
        case QVariant::LongLong:
            return sol::make_object(lua, variant.toLongLong());
        case QVariant::ULongLong:
            return sol::make_object(lua, variant.toULongLong());
        case QVariant::Double:
            return sol::make_object(lua, variant.toDouble());
        case QVariant::String:
            return sol::make_object(lua, variant.toString().toStdString());
        case QVariant::StringList: {
            sol::table result = lua.create_table();
            int index = 1;
            for (const auto& str : variant.toStringList()) {
                result[index++] = str.toStdString();
            }
            return result;
        }
        case QVariant::List: {
            sol::table result = lua.create_table();
            int index = 1;
            for (const auto& item : variant.toList()) {
                result[index++] = qvariant_to_lua(item, lua);
            }
            return result;
        }
        case QVariant::Map: {
            sol::table result = lua.create_table();
            auto map = variant.toMap();
            for (auto it = map.begin(); it != map.end(); ++it) {
                result[it.key().toStdString()] =
                    qvariant_to_lua(it.value(), lua);
            }
            return result;
        }
        default:
            return sol::make_object(lua, variant.toString().toStdString());
    }
}

// Helper to convert Lua value to QVariant
QVariant lua_to_qvariant(const sol::object& obj) {
    if (obj.is<sol::lua_nil_t>()) {
        return QVariant();
    } else if (obj.is<bool>()) {
        return QVariant(obj.as<bool>());
    } else if (obj.is<int>()) {
        return QVariant(obj.as<int>());
    } else if (obj.is<double>()) {
        return QVariant(obj.as<double>());
    } else if (obj.is<std::string>()) {
        return QVariant(QString::fromStdString(obj.as<std::string>()));
    } else if (obj.is<sol::table>()) {
        sol::table table = obj.as<sol::table>();
        // Check if it's an array or a map
        bool is_array = true;
        for (const auto& pair : table) {
            if (!pair.first.is<int>()) {
                is_array = false;
                break;
            }
        }

        if (is_array) {
            QVariantList list;
            for (const auto& pair : table) {
                list.append(lua_to_qvariant(pair.second));
            }
            return QVariant(list);
        } else {
            QVariantMap map;
            for (const auto& pair : table) {
                std::string key = pair.first.as<std::string>();
                map[QString::fromStdString(key)] = lua_to_qvariant(pair.second);
            }
            return QVariant(map);
        }
    }
    return QVariant();
}

// Result wrapper for expected<void, PluginError>
struct LuaVoidResult {
    bool has_value;
    qtplugin::PluginError error;

    LuaVoidResult(
        const qtplugin::expected<void, qtplugin::PluginError>& expected) {
        if (expected) {
            has_value = true;
        } else {
            has_value = false;
            error = expected.error();
        }
    }
};

// Result wrapper for expected<QString, PluginError>
struct LuaStringResult {
    bool has_value;
    std::string value;
    qtplugin::PluginError error;

    LuaStringResult(
        const qtplugin::expected<QString, qtplugin::PluginError>& expected) {
        if (expected) {
            has_value = true;
            value = expected.value().toStdString();
        } else {
            has_value = false;
            error = expected.error();
        }
    }
};

void bind_property_system(sol::state& lua) {
    // === PropertyBindingType Enum ===
    lua.new_enum<qtplugin::PropertyBindingType>(
        "PropertyBindingType",
        {{"OneWay", qtplugin::PropertyBindingType::OneWay},
         {"TwoWay", qtplugin::PropertyBindingType::TwoWay},
         {"OneTime", qtplugin::PropertyBindingType::OneTime}});

    // === PropertyValidationType Enum ===
    lua.new_enum<qtplugin::PropertyValidationType>(
        "PropertyValidationType",
        {{"None", qtplugin::PropertyValidationType::None},
         {"Range", qtplugin::PropertyValidationType::Range},
         {"Enum", qtplugin::PropertyValidationType::Enum},
         {"Regex", qtplugin::PropertyValidationType::Regex},
         {"Custom", qtplugin::PropertyValidationType::Custom}});

    // === PropertyNotificationMode Enum ===
    lua.new_enum<qtplugin::PropertyNotificationMode>(
        "PropertyNotificationMode",
        {{"Immediate", qtplugin::PropertyNotificationMode::Immediate},
         {"Debounced", qtplugin::PropertyNotificationMode::Debounced},
         {"Throttled", qtplugin::PropertyNotificationMode::Throttled},
         {"Batched", qtplugin::PropertyNotificationMode::Batched}});

    // === Result wrappers ===
    lua.new_usertype<LuaVoidResult>("LuaVoidResult", "has_value",
                                    &LuaVoidResult::has_value, "error",
                                    &LuaVoidResult::error);

    lua.new_usertype<LuaStringResult>(
        "LuaStringResult", "has_value", &LuaStringResult::has_value, "value",
        &LuaStringResult::value, "error", &LuaStringResult::error);

    // === PropertyMetadata Struct ===
    lua.new_usertype<qtplugin::PropertyMetadata>(
        "PropertyMetadata", sol::constructors<qtplugin::PropertyMetadata()>(),
        "name", &qtplugin::PropertyMetadata::name, "display_name",
        &qtplugin::PropertyMetadata::display_name, "description",
        &qtplugin::PropertyMetadata::description, "category",
        &qtplugin::PropertyMetadata::category, "validation_type",
        &qtplugin::PropertyMetadata::validation_type, "is_required",
        &qtplugin::PropertyMetadata::is_required, "is_readonly",
        &qtplugin::PropertyMetadata::is_readonly, "is_specialized",
        &qtplugin::PropertyMetadata::is_specialized, "units",
        &qtplugin::PropertyMetadata::units);

    // === PropertyBinding Struct ===
    lua.new_usertype<qtplugin::PropertyBinding>(
        "PropertyBinding", sol::constructors<qtplugin::PropertyBinding()>(),
        "binding_id", &qtplugin::PropertyBinding::binding_id,
        "source_plugin_id", &qtplugin::PropertyBinding::source_plugin_id,
        "source_property", &qtplugin::PropertyBinding::source_property,
        "target_plugin_id", &qtplugin::PropertyBinding::target_plugin_id,
        "target_property", &qtplugin::PropertyBinding::target_property,
        "binding_type", &qtplugin::PropertyBinding::binding_type, "is_active",
        &qtplugin::PropertyBinding::is_active);

    // === PropertyChangeEvent Struct ===
    lua.new_usertype<qtplugin::PropertyChangeEvent>(
        "PropertyChangeEvent",
        sol::constructors<qtplugin::PropertyChangeEvent()>(), "plugin_id",
        &qtplugin::PropertyChangeEvent::plugin_id, "property_name",
        &qtplugin::PropertyChangeEvent::property_name, "source",
        &qtplugin::PropertyChangeEvent::source);

    // === PluginPropertySystem Class ===
    lua.new_usertype<qtplugin::PluginPropertySystem>(
        "PluginPropertySystem",
        sol::constructors<qtplugin::PluginPropertySystem(QObject*)>(),

        // Plugin registration
        "register_plugin",
        [](qtplugin::PluginPropertySystem& self,
           std::shared_ptr<qtplugin::IPlugin> plugin) {
            return LuaVoidResult(self.register_plugin(plugin));
        },

        "unregister_plugin",
        [](qtplugin::PluginPropertySystem& self, const std::string& plugin_id) {
            return LuaVoidResult(
                self.unregister_plugin(QString::fromStdString(plugin_id)));
        },

        "is_plugin_registered",
        [](const qtplugin::PluginPropertySystem& self,
           const std::string& plugin_id) {
            return self.is_plugin_registered(QString::fromStdString(plugin_id));
        },

        // Property value access
        "get_property_value",
        [](const qtplugin::PluginPropertySystem& self,
           const std::string& plugin_id, const std::string& property_name,
           sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto result =
                self.get_property_value(QString::fromStdString(plugin_id),
                                        QString::fromStdString(property_name));
            if (result) {
                return qvariant_to_lua(result.value(), lua);
            }
            return sol::lua_nil;
        },

        "set_property_value",
        [](qtplugin::PluginPropertySystem& self, const std::string& plugin_id,
           const std::string& property_name, sol::object value,
           const std::string& source) {
            return LuaVoidResult(self.set_property_value(
                QString::fromStdString(plugin_id),
                QString::fromStdString(property_name), lua_to_qvariant(value),
                QString::fromStdString(source)));
        },

        // Property binding
        "create_property_binding",
        [](qtplugin::PluginPropertySystem& self,
           const std::string& source_plugin_id,
           const std::string& source_property,
           const std::string& target_plugin_id,
           const std::string& target_property,
           qtplugin::PropertyBindingType binding_type) {
            return LuaStringResult(self.create_property_binding(
                QString::fromStdString(source_plugin_id),
                QString::fromStdString(source_property),
                QString::fromStdString(target_plugin_id),
                QString::fromStdString(target_property), binding_type));
        },

        "remove_property_binding",
        [](qtplugin::PluginPropertySystem& self,
           const std::string& binding_id) {
            return LuaVoidResult(self.remove_property_binding(
                QString::fromStdString(binding_id)));
        });
}

}  // namespace qtforge_lua
