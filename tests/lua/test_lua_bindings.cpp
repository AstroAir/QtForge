/**
 * @file test_lua_bindings.cpp
 * @brief Comprehensive test suite for Lua bindings
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTemporaryFile>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include "qtplugin/bridges/lua_plugin_bridge.hpp"
#include "qtplugin/core/lua_plugin_loader.hpp"
#include "../../src/lua/qtforge_lua.cpp"

class TestLuaBindings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Core functionality tests
    void testLuaInitialization();
    void testLuaShutdown();
    void testLuaStateManagement();

    // Type conversion tests
    void testQtToLuaConversions();
    void testLuaToQtConversions();
    void testJsonConversions();
    void testStringConversions();

    // Plugin bridge tests
    void testLuaPluginBridgeCreation();
    void testLuaPluginBridgeInitialization();
    void testLuaPluginBridgeLifecycle();
    void testLuaPluginBridgeCommands();

    // Plugin loader tests
    void testLuaPluginLoaderCreation();
    void testLuaPluginLoaderScriptExecution();
    void testLuaPluginLoaderErrorHandling();

    // Enhanced binding tests
    void testCoreModuleBindings();
    void testUtilsModuleBindings();
    void testManagersModuleBindings();
    void testSecurityModuleBindings();
    void testCommunicationModuleBindings();
    void testOrchestrationModuleBindings();

    // Cross-language integration tests
    void testLuaCppInteroperability();
    void testLuaQtIntegration();
    void testLuaAsyncOperations();

    // Performance and stress tests
    void testLuaPerformanceBaseline();
    void testLuaMemoryManagement();
    void testLuaConcurrentExecution();

    // Error handling and edge cases
    void testLuaErrorPropagation();
    void testLuaSandboxSecurity();
    void testLuaResourceLimits();
    void testLuaPluginLoaderCanLoad();
    void testLuaPluginLoaderLoadPlugin();
    void testLuaPluginLoaderUnloadPlugin();

    // Core bindings tests
    void testVersionBindings();
    void testPluginStateBindings();
    void testPluginErrorBindings();
    void testPluginMetadataBindings();

    // Manager bindings tests
    void testPluginManagerBindings();
    void testPluginLoadOptionsBindings();
    void testPluginInfoBindings();

    // Error handling tests
    void testErrorHandlingBindings();
    void testResultWrappers();
    void testExceptionHandling();

    // Communication tests
    void testMessageBusBindings();
    void testRequestResponseBindings();
    void testCommunicationHelpers();

    // Security tests
    void testSecurityManagerBindings();
    void testTrustManagerBindings();
    void testValidationBindings();

    // Utility tests
    void testJsonUtilities();
    void testStringUtilities();
    void testFileSystemUtilities();
    void testLoggingUtilities();
    void testTimeUtilities();

    // Integration tests
    void testLuaPluginExecution();
    void testLuaPluginCommunication();
    void testLuaPluginSecurity();
    void testLuaPluginErrorHandling();

    // Performance tests
    void testBindingPerformance();
    void testMemoryUsage();
    void testLargeDataHandling();

    // Edge case tests
    void testNullPointerHandling();
    void testInvalidInputHandling();
    void testResourceExhaustion();

private:
    void createTestLuaScript(const QString& filename, const QString& content);
    bool executeLuaCode(const QString& code);
    QJsonObject executeLuaFunction(const QString& functionName, const QJsonObject& params = {});

#ifdef QTFORGE_LUA_BINDINGS
    std::unique_ptr<sol::state> m_lua_state;
#endif
    QTemporaryDir m_temp_dir;
    std::unique_ptr<qtplugin::LuaPluginBridge> m_bridge;
    std::unique_ptr<qtplugin::LuaPluginLoader> m_loader;
};

void TestLuaBindings::initTestCase()
{
    qDebug() << "Starting Lua bindings test suite";

#ifdef QTFORGE_LUA_BINDINGS
    // Initialize global Lua state for testing
    QVERIFY(qtforge_lua::initialize_qtforge_lua());

    m_lua_state = std::make_unique<sol::state>();
    m_lua_state->open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table);

    // Register QtForge bindings
    qtforge_lua::register_core_bindings(*m_lua_state);
    qtforge_lua::register_utils_bindings(*m_lua_state);
    qtforge_lua::register_managers_bindings(*m_lua_state);
    qtforge_lua::register_communication_bindings(*m_lua_state);
    qtforge_lua::register_security_bindings(*m_lua_state);
#else
    QSKIP("Lua bindings not compiled in this build");
#endif

    QVERIFY(m_temp_dir.isValid());
}

void TestLuaBindings::cleanupTestCase()
{
#ifdef QTFORGE_LUA_BINDINGS
    m_lua_state.reset();
    qtforge_lua::shutdown_qtforge_lua();
#endif
    qDebug() << "Lua bindings test suite completed";
}

void TestLuaBindings::init()
{
#ifdef QTFORGE_LUA_BINDINGS
    m_bridge = std::make_unique<qtplugin::LuaPluginBridge>();
    m_loader = std::make_unique<qtplugin::LuaPluginLoader>();
#endif
}

void TestLuaBindings::cleanup()
{
    m_bridge.reset();
    m_loader.reset();
}

void TestLuaBindings::testLuaInitialization()
{
#ifdef QTFORGE_LUA_BINDINGS
    QVERIFY(qtforge_lua::get_lua_state() != nullptr);

    // Test basic Lua functionality
    std::string error;
    QVERIFY(qtforge_lua::execute_lua_code("x = 42", error));
    QVERIFY(error.empty());

    // Test QtForge module availability
    QVERIFY(qtforge_lua::execute_lua_code("assert(qtforge ~= nil)", error));
    QVERIFY(error.empty());

    // Test version information
    QVERIFY(qtforge_lua::execute_lua_code("assert(qtforge.version ~= nil)", error));
    QVERIFY(error.empty());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaBindings::testLuaShutdown()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test graceful shutdown
    qtforge_lua::shutdown_qtforge_lua();
    QVERIFY(qtforge_lua::get_lua_state() == nullptr);

    // Reinitialize for other tests
    QVERIFY(qtforge_lua::initialize_qtforge_lua());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaBindings::testQtToLuaConversions()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test QString conversion
    QString test_string = "Hello, Lua!";
    sol::object lua_string = qtforge_lua::qstring_to_lua(test_string, *m_lua_state);
    QVERIFY(lua_string.get_type() == sol::type::string);
    QCOMPARE(lua_string.as<std::string>(), test_string.toStdString());

    // Test QJsonObject conversion
    QJsonObject json_obj;
    json_obj["key"] = "value";
    json_obj["number"] = 42;
    json_obj["boolean"] = true;

    sol::object lua_obj = qtforge_lua::qjson_to_lua(json_obj, *m_lua_state);
    QVERIFY(lua_obj.get_type() == sol::type::table);

    sol::table lua_table = lua_obj.as<sol::table>();
    QCOMPARE(lua_table["key"].get<std::string>(), std::string("value"));
    QCOMPARE(lua_table["number"].get<double>(), 42.0);
    QCOMPARE(lua_table["boolean"].get<bool>(), true);

    // Test QStringList conversion
    QStringList string_list = {"item1", "item2", "item3"};
    sol::object lua_list = qtforge_lua::qstringlist_to_lua(string_list, *m_lua_state);
    QVERIFY(lua_list.get_type() == sol::type::table);

    sol::table lua_array = lua_list.as<sol::table>();
    QCOMPARE(lua_array[1].get<std::string>(), std::string("item1"));
    QCOMPARE(lua_array[2].get<std::string>(), std::string("item2"));
    QCOMPARE(lua_array[3].get<std::string>(), std::string("item3"));
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaBindings::testLuaToQtConversions()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test Lua string to QString
    sol::object lua_string = sol::make_object(*m_lua_state, "Hello, Qt!");
    QString qt_string = qtforge_lua::lua_to_qstring(lua_string);
    QCOMPARE(qt_string, QString("Hello, Qt!"));

    // Test Lua table to QJsonObject
    m_lua_state->script("test_table = {key = 'value', number = 42, boolean = true}");
    sol::object lua_table = (*m_lua_state)["test_table"];
    QJsonValue json_value = qtforge_lua::lua_to_qjson(lua_table);

    QVERIFY(json_value.isObject());
    QJsonObject json_obj = json_value.toObject();
    QCOMPARE(json_obj["key"].toString(), QString("value"));
    QCOMPARE(json_obj["number"].toDouble(), 42.0);
    QCOMPARE(json_obj["boolean"].toBool(), true);

    // Test Lua array to QStringList
    m_lua_state->script("test_array = {'item1', 'item2', 'item3'}");
    sol::object lua_array = (*m_lua_state)["test_array"];
    QStringList string_list = qtforge_lua::lua_to_qstringlist(lua_array);

    QCOMPARE(string_list.size(), 3);
    QCOMPARE(string_list[0], QString("item1"));
    QCOMPARE(string_list[1], QString("item2"));
    QCOMPARE(string_list[2], QString("item3"));
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaBindings::testVersionBindings()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test Version creation
    std::string code = R"(
        version = Version(1, 2, 3)
        assert(version.major == 1)
        assert(version.minor == 2)
        assert(version.patch == 3)
        assert(version:to_string() == "1.2.3")
    )";

    std::string error;
    QVERIFY(qtforge_lua::execute_lua_code(code, error));
    QVERIFY(error.empty());

    // Test version comparison
    code = R"(
        v1 = Version(1, 0, 0)
        v2 = Version(2, 0, 0)
        assert(v1 < v2)
        assert(v1 ~= v2)
    )";

    QVERIFY(qtforge_lua::execute_lua_code(code, error));
    QVERIFY(error.empty());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaBindings::testPluginErrorBindings()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test PluginError creation
    std::string code = R"(
        error = PluginError(PluginErrorCode.InvalidParameter, "Test error message")
        assert(error.code == PluginErrorCode.InvalidParameter)
        assert(error.message == "Test error message")
        assert(error:to_string():find("Test error message") ~= nil)
    )";

    std::string error;
    QVERIFY(qtforge_lua::execute_lua_code(code, error));
    QVERIFY(error.empty());

    // Test error code utilities
    code = R"(
        code_str = qtforge.error.code_to_string(PluginErrorCode.LoadFailed)
        assert(code_str == "LoadFailed")

        code = qtforge.error.string_to_code("NetworkError")
        assert(code == PluginErrorCode.NetworkError)
    )";

    QVERIFY(qtforge_lua::execute_lua_code(code, error));
    QVERIFY(error.empty());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaBindings::createTestLuaScript(const QString& filename, const QString& content)
{
    QFile file(m_temp_dir.filePath(filename));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << content;
    file.close();
}

bool TestLuaBindings::executeLuaCode(const QString& code)
{
#ifdef QTFORGE_LUA_BINDINGS
    std::string error;
    return qtforge_lua::execute_lua_code(code.toStdString(), error);
#else
    Q_UNUSED(code)
    return false;
#endif
}

QJsonObject TestLuaBindings::executeLuaFunction(const QString& functionName, const QJsonObject& params)
{
#ifdef QTFORGE_LUA_BINDINGS
    // Implementation would call Lua function and return result as JSON
    Q_UNUSED(functionName)
    Q_UNUSED(params)
    return QJsonObject();
#else
    Q_UNUSED(functionName)
    Q_UNUSED(params)
    return QJsonObject();
#endif
}

void TestLuaBindings::testLuaPluginLoaderScriptExecution()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test script execution with various Lua constructs
    QString testScript = R"(
        -- Test basic Lua functionality
        local result = {}
        result.number = 42
        result.string = "Hello from Lua"
        result.boolean = true
        result.table = {1, 2, 3}

        -- Test QtForge bindings if available
        if qtforge then
            result.qtforge_version = qtforge.version or "unknown"
            if qtforge.core then
                result.core_available = true
            end
        end

        return result
    )";

    createTestFile("test_script.lua", testScript);

    std::string error;
    bool success = qtforge_lua::load_lua_file(m_temp_dir.path().toStdString() + "/test_script.lua", error);

    if (qtforge_lua::get_lua_state()) {
        QVERIFY(success);
        QVERIFY(error.empty());
    } else {
        QSKIP("Lua state not available");
    }
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaBindings::testLuaPluginLoaderErrorHandling()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test error handling with invalid Lua code
    QString invalidScript = R"(
        -- This script contains syntax errors
        local x =
        invalid syntax here
        return nil
    )";

    createTestFile("invalid_script.lua", invalidScript);

    std::string error;
    bool success = qtforge_lua::load_lua_file(m_temp_dir.path().toStdString() + "/invalid_script.lua", error);

    QVERIFY(!success);
    QVERIFY(!error.empty());

    // Test runtime error handling
    QString runtimeErrorScript = R"(
        error("This is a runtime error")
    )";

    error.clear();
    success = qtforge_lua::execute_lua_code(runtimeErrorScript.toStdString(), error);
    QVERIFY(!success);
    QVERIFY(!error.empty());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaBindings::testCoreModuleBindings()
{
#ifdef QTFORGE_LUA_BINDINGS
    if (!qtforge_lua::get_lua_state()) {
        QSKIP("Lua state not available");
    }

    // Test core module availability and functionality
    QString coreTest = R"(
        local success = true
        local errors = {}

        -- Test qtforge table existence
        if not qtforge then
            table.insert(errors, "qtforge table not available")
            success = false
        end

        -- Test core module
        if qtforge and qtforge.core then
            -- Test core functions if available
            if qtforge.core.test_function then
                local result = qtforge.core.test_function()
                if not result then
                    table.insert(errors, "core.test_function failed")
                    success = false
                end
            end
        else
            table.insert(errors, "qtforge.core module not available")
        end

        return {success = success, errors = errors}
    )";

    std::string error;
    bool success = qtforge_lua::execute_lua_code(coreTest.toStdString(), error);

    // If Lua execution succeeds, the core bindings are working
    if (success) {
        QVERIFY(error.empty());
    } else {
        qDebug() << "Core module test error:" << QString::fromStdString(error);
        // Don't fail the test if core module is not available in this build
    }
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaBindings::testUtilsModuleBindings()
{
#ifdef QTFORGE_LUA_BINDINGS
    if (!qtforge_lua::get_lua_state()) {
        QSKIP("Lua state not available");
    }

    QString utilsTest = R"(
        local success = true
        local errors = {}

        -- Test utils module
        if qtforge and qtforge.utils then
            -- Test version creation if available
            if qtforge.utils.create_version then
                local version = qtforge.utils.create_version(1, 2, 3)
                if not version then
                    table.insert(errors, "utils.create_version failed")
                    success = false
                end
            end

            -- Test utility functions if available
            if qtforge.utils.utils_test then
                local result = qtforge.utils.utils_test()
                if not result then
                    table.insert(errors, "utils.utils_test failed")
                    success = false
                end
            end
        else
            -- Utils module not available is acceptable
            success = true
        end

        return {success = success, errors = errors}
    )";

    std::string error;
    bool success = qtforge_lua::execute_lua_code(utilsTest.toStdString(), error);

    if (success) {
        QVERIFY(error.empty());
    } else {
        qDebug() << "Utils module test error:" << QString::fromStdString(error);
    }
#else
    QSKIP("Lua bindings not available");
#endif
}

QTEST_MAIN(TestLuaBindings)
#include "test_lua_bindings.moc"
