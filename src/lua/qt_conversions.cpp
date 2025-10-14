/**
 * @file qt_conversions.cpp
 * @brief Qt type conversions for Lua bindings
 * @version 3.2.0
 */

#include "qt_conversions.hpp"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

Q_LOGGING_CATEGORY(qtConversionsLog, "qtforge.lua.conversions");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Convert QJsonValue to Lua object
 */
sol::object qjson_to_lua(const QJsonValue& value, sol::state& lua) {
    switch (value.type()) {
        case QJsonValue::Null:
            return sol::nil;

        case QJsonValue::Bool:
            return sol::make_object(lua, value.toBool());

        case QJsonValue::Double:
            return sol::make_object(lua, value.toDouble());

        case QJsonValue::String:
            return sol::make_object(lua, value.toString().toStdString());

        case QJsonValue::Array: {
            sol::table table = lua.create_table();
            QJsonArray array = value.toArray();
            for (int i = 0; i < array.size(); ++i) {
                table[i + 1] =
                    qjson_to_lua(array[i], lua);  // Lua arrays are 1-indexed
            }
            return table;
        }

        case QJsonValue::Object: {
            sol::table table = lua.create_table();
            QJsonObject obj = value.toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                table[it.key().toStdString()] = qjson_to_lua(it.value(), lua);
            }
            return table;
        }

        default:
            qCWarning(qtConversionsLog)
                << "Unknown QJsonValue type:" << value.type();
            return sol::nil;
    }
}

/**
 * @brief Convert Lua object to QJsonValue
 */
QJsonValue lua_to_qjson(const sol::object& obj) {
    switch (obj.get_type()) {
        case sol::type::nil:
            return QJsonValue::Null;

        case sol::type::boolean:
            return obj.as<bool>();

        case sol::type::number:
            return obj.as<double>();

        case sol::type::string:
            return QString::fromStdString(obj.as<std::string>());

        case sol::type::table: {
            sol::table table = obj.as<sol::table>();

            // Check if it's an array (consecutive integer keys starting from 1)
            bool is_array = true;
            size_t expected_index = 1;
            size_t count = 0;

            for (const auto& pair : table) {
                count++;
                if (pair.first.get_type() != sol::type::number) {
                    is_array = false;
                    break;
                }

                double index = pair.first.as<double>();
                if (index != expected_index) {
                    is_array = false;
                    break;
                }
                expected_index++;
            }

            if (is_array && count > 0) {
                // Convert to QJsonArray
                QJsonArray array;
                for (size_t i = 1; i <= count; ++i) {
                    array.append(lua_to_qjson(table[i]));
                }
                return array;
            } else {
                // Convert to QJsonObject
                QJsonObject json_obj;
                for (const auto& pair : table) {
                    QString key;
                    if (pair.first.get_type() == sol::type::string) {
                        key = QString::fromStdString(
                            pair.first.as<std::string>());
                    } else if (pair.first.get_type() == sol::type::number) {
                        key = QString::number(pair.first.as<double>());
                    } else {
                        key = "unknown_key";
                    }
                    json_obj[key] = lua_to_qjson(pair.second);
                }
                return json_obj;
            }
        }

        case sol::type::function:
            return QString("function");

        case sol::type::userdata:
            return QString("userdata");

        default:
            qCWarning(qtConversionsLog)
                << "Unknown Lua type:" << static_cast<int>(obj.get_type());
            return QJsonValue::Null;
    }
}

/**
 * @brief Convert QString to Lua string
 */
sol::object qstring_to_lua(const QString& str, sol::state& lua) {
    return sol::make_object(lua, str.toStdString());
}

/**
 * @brief Convert Lua string to QString
 */
QString lua_to_qstring(const sol::object& obj) {
    if (obj.get_type() == sol::type::string) {
        return QString::fromStdString(obj.as<std::string>());
    } else if (obj.get_type() == sol::type::number) {
        return QString::number(obj.as<double>());
    } else if (obj.get_type() == sol::type::boolean) {
        return obj.as<bool>() ? "true" : "false";
    } else if (obj.get_type() == sol::type::nil) {
        return QString();
    } else {
        return QString("unknown");
    }
}

/**
 * @brief Convert QStringList to Lua table
 */
sol::object qstringlist_to_lua(const QStringList& list, sol::state& lua) {
    sol::table table = lua.create_table();
    for (int i = 0; i < list.size(); ++i) {
        table[i + 1] = list[i].toStdString();  // Lua arrays are 1-indexed
    }
    return table;
}

/**
 * @brief Convert Lua table to QStringList
 */
QStringList lua_to_qstringlist(const sol::object& obj) {
    QStringList list;

    if (obj.get_type() == sol::type::table) {
        sol::table table = obj.as<sol::table>();

        // Iterate through table assuming it's an array
        for (size_t i = 1;; ++i) {
            sol::object element = table[i];
            if (element.get_type() == sol::type::nil) {
                break;  // End of array
            }
            list.append(lua_to_qstring(element));
        }
    }

    return list;
}

/**
 * @brief Convert QVariant to Lua object
 */
sol::object qvariant_to_lua(const QVariant& variant, sol::state& lua) {
    switch (variant.type()) {
        case QVariant::Invalid:
            return sol::nil;

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
            QStringList list = variant.toStringList();
            return qstringlist_to_lua(list, lua);
        }

        case QVariant::List: {
            sol::table table = lua.create_table();
            QVariantList list = variant.toList();
            for (int i = 0; i < list.size(); ++i) {
                table[i + 1] =
                    qvariant_to_lua(list[i], lua);  // Lua arrays are 1-indexed
            }
            return table;
        }

        case QVariant::Map: {
            sol::table table = lua.create_table();
            QVariantMap map = variant.toMap();
            for (auto it = map.begin(); it != map.end(); ++it) {
                table[it.key().toStdString()] =
                    qvariant_to_lua(it.value(), lua);
            }
            return table;
        }

        default:
            // Try to convert to string as fallback
            return sol::make_object(lua, variant.toString().toStdString());
    }
}

/**
 * @brief Convert Lua object to QVariant
 */
QVariant lua_to_qvariant(const sol::object& obj) {
    switch (obj.get_type()) {
        case sol::type::nil:
            return QVariant();

        case sol::type::boolean:
            return obj.as<bool>();

        case sol::type::number:
            return obj.as<double>();

        case sol::type::string:
            return QString::fromStdString(obj.as<std::string>());

        case sol::type::table: {
            sol::table table = obj.as<sol::table>();

            // Check if it's an array
            bool is_array = true;
            size_t expected_index = 1;
            size_t count = 0;

            for (const auto& pair : table) {
                count++;
                if (pair.first.get_type() != sol::type::number) {
                    is_array = false;
                    break;
                }

                double index = pair.first.as<double>();
                if (index != expected_index) {
                    is_array = false;
                    break;
                }
                expected_index++;
            }

            if (is_array && count > 0) {
                // Convert to QVariantList
                QVariantList list;
                for (size_t i = 1; i <= count; ++i) {
                    list.append(lua_to_qvariant(table[i]));
                }
                return list;
            } else {
                // Convert to QVariantMap
                QVariantMap map;
                for (const auto& pair : table) {
                    QString key = lua_to_qstring(pair.first);
                    map[key] = lua_to_qvariant(pair.second);
                }
                return map;
            }
        }

        default:
            return QVariant();
    }
}

#else  // QTFORGE_LUA_BINDINGS not defined

// Stub implementations when Lua bindings are not available
sol::object qjson_to_lua(const QJsonValue& value, sol::state& lua) {
    Q_UNUSED(value)
    Q_UNUSED(lua)
    return sol::object();
}

QJsonValue lua_to_qjson(const sol::object& obj) {
    Q_UNUSED(obj)
    return QJsonValue::Null;
}

sol::object qstring_to_lua(const QString& str, sol::state& lua) {
    Q_UNUSED(str)
    Q_UNUSED(lua)
    return sol::object();
}

QString lua_to_qstring(const sol::object& obj) {
    Q_UNUSED(obj)
    return QString();
}

sol::object qstringlist_to_lua(const QStringList& list, sol::state& lua) {
    Q_UNUSED(list)
    Q_UNUSED(lua)
    return sol::object();
}

QStringList lua_to_qstringlist(const sol::object& obj) {
    Q_UNUSED(obj)
    return QStringList();
}

sol::object qvariant_to_lua(const QVariant& variant, sol::state& lua) {
    Q_UNUSED(variant)
    Q_UNUSED(lua)
    return sol::object();
}

QVariant lua_to_qvariant(const sol::object& obj) {
    Q_UNUSED(obj)
    return QVariant();
}

#endif  // QTFORGE_LUA_BINDINGS

}  // namespace qtforge_lua
