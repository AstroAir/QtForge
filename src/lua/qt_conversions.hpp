/**
 * @file qt_conversions.hpp
 * @brief Qt type conversions for Lua bindings - Header declarations
 * @version 3.2.0
 */

#pragma once

#include <QJsonValue>
#include <QString>
#include <QStringList>
#include <QVariant>

// Forward declare sol types when Lua bindings are enabled
#ifdef QTFORGE_LUA_BINDINGS
namespace sol {
class state;
class object;
}  // namespace sol
#else
// Provide minimal forward declarations for when Lua is not available
namespace sol {
class state;
class object;
}  // namespace sol
#endif

namespace qtforge_lua {

/**
 * @brief Convert QJsonValue to Lua object
 * @param value The QJsonValue to convert
 * @param lua The Lua state
 * @return Lua object representation of the JSON value
 */
sol::object qjson_to_lua(const QJsonValue& value, sol::state& lua);

/**
 * @brief Convert Lua object to QJsonValue
 * @param obj The Lua object to convert
 * @return QJsonValue representation of the Lua object
 */
QJsonValue lua_to_qjson(const sol::object& obj);

/**
 * @brief Convert QString to Lua string
 * @param str The QString to convert
 * @param lua The Lua state
 * @return Lua string object
 */
sol::object qstring_to_lua(const QString& str, sol::state& lua);

/**
 * @brief Convert Lua string to QString
 * @param obj The Lua object to convert
 * @return QString representation
 */
QString lua_to_qstring(const sol::object& obj);

/**
 * @brief Convert QStringList to Lua table
 * @param list The QStringList to convert
 * @param lua The Lua state
 * @return Lua table containing the strings
 */
sol::object qstringlist_to_lua(const QStringList& list, sol::state& lua);

/**
 * @brief Convert Lua table to QStringList
 * @param obj The Lua table to convert
 * @return QStringList representation
 */
QStringList lua_to_qstringlist(const sol::object& obj);

/**
 * @brief Convert QVariant to Lua object
 * @param variant The QVariant to convert
 * @param lua The Lua state
 * @return Lua object representation of the variant
 */
sol::object qvariant_to_lua(const QVariant& variant, sol::state& lua);

/**
 * @brief Convert Lua object to QVariant
 * @param obj The Lua object to convert
 * @return QVariant representation
 */
QVariant lua_to_qvariant(const sol::object& obj);

}  // namespace qtforge_lua
