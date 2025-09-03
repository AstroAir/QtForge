/**
 * @file test_qt_conversions.cpp
 * @brief Test suite for Qt-Lua type conversions
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>
#include <QVariantMap>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

// Include the conversion functions
#include "../../src/lua/qt_conversions.cpp"

class TestQtConversions : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // QJsonValue conversions
    void testQJsonToLua();
    void testLuaToQJson();
    void testJsonObjectConversion();
    void testJsonArrayConversion();
    void testJsonValueTypes();
    
    // QString conversions
    void testQStringToLua();
    void testLuaToQString();
    void testStringEdgeCases();
    
    // QStringList conversions
    void testQStringListToLua();
    void testLuaToQStringList();
    void testEmptyStringList();
    
    // QVariant conversions
    void testQVariantToLua();
    void testLuaToQVariant();
    void testVariantTypes();
    void testVariantContainers();
    
    // Complex data structures
    void testNestedObjects();
    void testMixedArrays();
    void testLargeDataStructures();
    
    // Edge cases and error handling
    void testNullValues();
    void testCircularReferences();
    void testInvalidTypes();
    void testMemoryManagement();
    
    // Performance tests
    void testConversionPerformance();
    void testLargeDataPerformance();

private:
#ifdef QTFORGE_LUA_BINDINGS
    std::unique_ptr<sol::state> m_lua_state;
#endif
};

void TestQtConversions::initTestCase()
{
    qDebug() << "Starting Qt-Lua conversions test suite";
    
#ifdef QTFORGE_LUA_BINDINGS
    m_lua_state = std::make_unique<sol::state>();
    m_lua_state->open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table);
#endif
}

void TestQtConversions::cleanupTestCase()
{
#ifdef QTFORGE_LUA_BINDINGS
    m_lua_state.reset();
#endif
    qDebug() << "Qt-Lua conversions test suite completed";
}

void TestQtConversions::init()
{
    // Reset Lua state for each test
#ifdef QTFORGE_LUA_BINDINGS
    if (m_lua_state) {
        m_lua_state->collect_garbage();
    }
#endif
}

void TestQtConversions::cleanup()
{
    // Cleanup after each test
}

void TestQtConversions::testQJsonToLua()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test null value
    QJsonValue null_value = QJsonValue::Null;
    sol::object lua_null = qtforge_lua::qjson_to_lua(null_value, *m_lua_state);
    QVERIFY(lua_null.get_type() == sol::type::nil);
    
    // Test boolean value
    QJsonValue bool_value = true;
    sol::object lua_bool = qtforge_lua::qjson_to_lua(bool_value, *m_lua_state);
    QVERIFY(lua_bool.get_type() == sol::type::boolean);
    QCOMPARE(lua_bool.as<bool>(), true);
    
    // Test number value
    QJsonValue num_value = 42.5;
    sol::object lua_num = qtforge_lua::qjson_to_lua(num_value, *m_lua_state);
    QVERIFY(lua_num.get_type() == sol::type::number);
    QCOMPARE(lua_num.as<double>(), 42.5);
    
    // Test string value
    QJsonValue str_value = "Hello, Lua!";
    sol::object lua_str = qtforge_lua::qjson_to_lua(str_value, *m_lua_state);
    QVERIFY(lua_str.get_type() == sol::type::string);
    QCOMPARE(lua_str.as<std::string>(), std::string("Hello, Lua!"));
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestQtConversions::testLuaToQJson()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test nil value
    sol::object lua_nil = sol::nil;
    QJsonValue json_null = qtforge_lua::lua_to_qjson(lua_nil);
    QVERIFY(json_null.isNull());
    
    // Test boolean value
    sol::object lua_bool = sol::make_object(*m_lua_state, true);
    QJsonValue json_bool = qtforge_lua::lua_to_qjson(lua_bool);
    QVERIFY(json_bool.isBool());
    QCOMPARE(json_bool.toBool(), true);
    
    // Test number value
    sol::object lua_num = sol::make_object(*m_lua_state, 42.5);
    QJsonValue json_num = qtforge_lua::lua_to_qjson(lua_num);
    QVERIFY(json_num.isDouble());
    QCOMPARE(json_num.toDouble(), 42.5);
    
    // Test string value
    sol::object lua_str = sol::make_object(*m_lua_state, "Hello, Qt!");
    QJsonValue json_str = qtforge_lua::lua_to_qjson(lua_str);
    QVERIFY(json_str.isString());
    QCOMPARE(json_str.toString(), QString("Hello, Qt!"));
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestQtConversions::testJsonObjectConversion()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Create test JSON object
    QJsonObject json_obj;
    json_obj["string"] = "value";
    json_obj["number"] = 42;
    json_obj["boolean"] = true;
    json_obj["null"] = QJsonValue::Null;
    
    // Convert to Lua
    sol::object lua_obj = qtforge_lua::qjson_to_lua(json_obj, *m_lua_state);
    QVERIFY(lua_obj.get_type() == sol::type::table);
    
    sol::table lua_table = lua_obj.as<sol::table>();
    QCOMPARE(lua_table["string"].get<std::string>(), std::string("value"));
    QCOMPARE(lua_table["number"].get<double>(), 42.0);
    QCOMPARE(lua_table["boolean"].get<bool>(), true);
    QVERIFY(lua_table["null"].get_type() == sol::type::nil);
    
    // Convert back to JSON
    QJsonValue json_back = qtforge_lua::lua_to_qjson(lua_obj);
    QVERIFY(json_back.isObject());
    
    QJsonObject json_obj_back = json_back.toObject();
    QCOMPARE(json_obj_back["string"].toString(), QString("value"));
    QCOMPARE(json_obj_back["number"].toDouble(), 42.0);
    QCOMPARE(json_obj_back["boolean"].toBool(), true);
    QVERIFY(json_obj_back["null"].isNull());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestQtConversions::testJsonArrayConversion()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Create test JSON array
    QJsonArray json_array;
    json_array.append("item1");
    json_array.append(42);
    json_array.append(true);
    json_array.append(QJsonValue::Null);
    
    // Convert to Lua
    sol::object lua_obj = qtforge_lua::qjson_to_lua(json_array, *m_lua_state);
    QVERIFY(lua_obj.get_type() == sol::type::table);
    
    sol::table lua_table = lua_obj.as<sol::table>();
    QCOMPARE(lua_table[1].get<std::string>(), std::string("item1")); // Lua arrays are 1-indexed
    QCOMPARE(lua_table[2].get<double>(), 42.0);
    QCOMPARE(lua_table[3].get<bool>(), true);
    QVERIFY(lua_table[4].get_type() == sol::type::nil);
    
    // Convert back to JSON
    QJsonValue json_back = qtforge_lua::lua_to_qjson(lua_obj);
    QVERIFY(json_back.isArray());
    
    QJsonArray json_array_back = json_back.toArray();
    QCOMPARE(json_array_back.size(), 4);
    QCOMPARE(json_array_back[0].toString(), QString("item1"));
    QCOMPARE(json_array_back[1].toDouble(), 42.0);
    QCOMPARE(json_array_back[2].toBool(), true);
    QVERIFY(json_array_back[3].isNull());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestQtConversions::testQStringListToLua()
{
#ifdef QTFORGE_LUA_BINDINGS
    QStringList string_list = {"first", "second", "third"};
    
    sol::object lua_obj = qtforge_lua::qstringlist_to_lua(string_list, *m_lua_state);
    QVERIFY(lua_obj.get_type() == sol::type::table);
    
    sol::table lua_table = lua_obj.as<sol::table>();
    QCOMPARE(lua_table[1].get<std::string>(), std::string("first"));
    QCOMPARE(lua_table[2].get<std::string>(), std::string("second"));
    QCOMPARE(lua_table[3].get<std::string>(), std::string("third"));
    
    // Test empty list
    QStringList empty_list;
    sol::object lua_empty = qtforge_lua::qstringlist_to_lua(empty_list, *m_lua_state);
    QVERIFY(lua_empty.get_type() == sol::type::table);
    
    sol::table empty_table = lua_empty.as<sol::table>();
    QVERIFY(empty_table[1].get_type() == sol::type::nil);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestQtConversions::testLuaToQStringList()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Create Lua array
    m_lua_state->script("test_array = {'first', 'second', 'third'}");
    sol::object lua_array = (*m_lua_state)["test_array"];
    
    QStringList string_list = qtforge_lua::lua_to_qstringlist(lua_array);
    QCOMPARE(string_list.size(), 3);
    QCOMPARE(string_list[0], QString("first"));
    QCOMPARE(string_list[1], QString("second"));
    QCOMPARE(string_list[2], QString("third"));
    
    // Test empty array
    m_lua_state->script("empty_array = {}");
    sol::object lua_empty = (*m_lua_state)["empty_array"];
    
    QStringList empty_list = qtforge_lua::lua_to_qstringlist(lua_empty);
    QVERIFY(empty_list.isEmpty());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestQtConversions::testVariantTypes()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test various QVariant types
    QVariant int_variant = 42;
    sol::object lua_int = qtforge_lua::qvariant_to_lua(int_variant, *m_lua_state);
    QVERIFY(lua_int.get_type() == sol::type::number);
    QCOMPARE(lua_int.as<int>(), 42);
    
    QVariant string_variant = QString("test string");
    sol::object lua_string = qtforge_lua::qvariant_to_lua(string_variant, *m_lua_state);
    QVERIFY(lua_string.get_type() == sol::type::string);
    QCOMPARE(lua_string.as<std::string>(), std::string("test string"));
    
    QVariant bool_variant = true;
    sol::object lua_bool = qtforge_lua::qvariant_to_lua(bool_variant, *m_lua_state);
    QVERIFY(lua_bool.get_type() == sol::type::boolean);
    QCOMPARE(lua_bool.as<bool>(), true);
    
    QVariant invalid_variant;
    sol::object lua_invalid = qtforge_lua::qvariant_to_lua(invalid_variant, *m_lua_state);
    QVERIFY(lua_invalid.get_type() == sol::type::nil);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestQtConversions::testNestedObjects()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Create nested JSON structure
    QJsonObject inner_obj;
    inner_obj["inner_key"] = "inner_value";
    
    QJsonArray inner_array;
    inner_array.append(1);
    inner_array.append(2);
    inner_array.append(3);
    
    QJsonObject outer_obj;
    outer_obj["object"] = inner_obj;
    outer_obj["array"] = inner_array;
    outer_obj["simple"] = "value";
    
    // Convert to Lua
    sol::object lua_obj = qtforge_lua::qjson_to_lua(outer_obj, *m_lua_state);
    QVERIFY(lua_obj.get_type() == sol::type::table);
    
    sol::table lua_table = lua_obj.as<sol::table>();
    
    // Check nested object
    sol::object nested_obj = lua_table["object"];
    QVERIFY(nested_obj.get_type() == sol::type::table);
    sol::table nested_table = nested_obj.as<sol::table>();
    QCOMPARE(nested_table["inner_key"].get<std::string>(), std::string("inner_value"));
    
    // Check nested array
    sol::object nested_array = lua_table["array"];
    QVERIFY(nested_array.get_type() == sol::type::table);
    sol::table array_table = nested_array.as<sol::table>();
    QCOMPARE(array_table[1].get<double>(), 1.0);
    QCOMPARE(array_table[2].get<double>(), 2.0);
    QCOMPARE(array_table[3].get<double>(), 3.0);
    
    // Convert back to JSON
    QJsonValue json_back = qtforge_lua::lua_to_qjson(lua_obj);
    QVERIFY(json_back.isObject());
    
    QJsonObject json_obj_back = json_back.toObject();
    QVERIFY(json_obj_back.contains("object"));
    QVERIFY(json_obj_back.contains("array"));
    QCOMPARE(json_obj_back["simple"].toString(), QString("value"));
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestQtConversions::testNullValues()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test various null/nil scenarios
    QJsonValue null_json = QJsonValue::Null;
    sol::object lua_nil = qtforge_lua::qjson_to_lua(null_json, *m_lua_state);
    QVERIFY(lua_nil.get_type() == sol::type::nil);
    
    // Convert back
    QJsonValue json_null = qtforge_lua::lua_to_qjson(lua_nil);
    QVERIFY(json_null.isNull());
    
    // Test invalid QVariant
    QVariant invalid_variant;
    sol::object lua_invalid = qtforge_lua::qvariant_to_lua(invalid_variant, *m_lua_state);
    QVERIFY(lua_invalid.get_type() == sol::type::nil);
    
    QVariant variant_back = qtforge_lua::lua_to_qvariant(lua_invalid);
    QVERIFY(!variant_back.isValid());
#else
    QSKIP("Lua bindings not available");
#endif
}

QTEST_MAIN(TestQtConversions)
#include "test_qt_conversions.moc"
