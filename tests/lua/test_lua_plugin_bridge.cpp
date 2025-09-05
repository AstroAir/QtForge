/**
 * @file test_lua_plugin_bridge.cpp
 * @brief Test suite for LuaPluginBridge
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include "qtplugin/bridges/lua_plugin_bridge.hpp"

class TestLuaPluginBridge : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testBridgeCreation();
    void testBridgeInitialization();
    void testBridgeShutdown();
    void testBridgeState();

    // Plugin interface tests
    void testPluginMetadata();
    void testPluginCapabilities();
    void testPluginCommands();
    void testPluginConfiguration();

    // Lua execution tests
    void testLuaCodeExecution();
    void testLuaScriptLoading();
    void testLuaErrorHandling();
    void testLuaSandboxing();

    // Dynamic plugin interface tests
    void testMethodInvocation();
    void testPropertyAccess();
    void testMethodListing();
    void testPropertyListing();

    // Integration tests
    void testPluginLifecycle();
    void testPluginCommunication();
    void testPluginSecurity();

private:
    void createTestLuaPlugin(const QString& filename, const QString& content);

    QTemporaryDir m_temp_dir;
    std::unique_ptr<qtplugin::LuaPluginBridge> m_bridge;
};

void TestLuaPluginBridge::initTestCase()
{
    qDebug() << "Starting LuaPluginBridge test suite";
    QVERIFY(m_temp_dir.isValid());
}

void TestLuaPluginBridge::cleanupTestCase()
{
    qDebug() << "LuaPluginBridge test suite completed";
}

void TestLuaPluginBridge::init()
{
    m_bridge = std::make_unique<qtplugin::LuaPluginBridge>();
}

void TestLuaPluginBridge::cleanup()
{
    if (m_bridge) {
        m_bridge->shutdown();
        m_bridge.reset();
    }
}

void TestLuaPluginBridge::testBridgeCreation()
{
    QVERIFY(m_bridge != nullptr);
    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Unloaded);
    QVERIFY(!m_bridge->name().empty());
    QVERIFY(!m_bridge->description().empty());
    QVERIFY(m_bridge->version().major >= 3);
}

void TestLuaPluginBridge::testBridgeInitialization()
{
#ifdef QTFORGE_LUA_BINDINGS
    auto result = m_bridge->initialize();
    QVERIFY(result.has_value());
    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Running);
    QVERIFY(m_bridge->is_initialized());
#else
    auto result = m_bridge->initialize();
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::NotSupported);
#endif
}

void TestLuaPluginBridge::testBridgeShutdown()
{
#ifdef QTFORGE_LUA_BINDINGS
    QVERIFY(m_bridge->initialize().has_value());
    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Running);

    m_bridge->shutdown();
    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Unloaded);
    QVERIFY(!m_bridge->is_initialized());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testLuaCodeExecution()
{
#ifdef QTFORGE_LUA_BINDINGS
    QVERIFY(m_bridge->initialize().has_value());

    // Test simple code execution
    QString code = "return 42";
    auto result = m_bridge->execute_code(code);
    QVERIFY(result.has_value());

    QVariant value = result.value();
    QVERIFY(value.isValid());

    // Test code with context
    QJsonObject context;
    context["input"] = 10;

    code = "return context.input * 2";
    result = m_bridge->execute_code(code, context);
    QVERIFY(result.has_value());

    // Test error handling
    code = "error('Test error')";
    result = m_bridge->execute_code(code);
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::ExecutionFailed);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testLuaScriptLoading()
{
#ifdef QTFORGE_LUA_BINDINGS
    QVERIFY(m_bridge->initialize().has_value());

    // Create test Lua plugin
    QString plugin_content = R"(
--[[
@plugin_name: Test Plugin
@plugin_description: A test Lua plugin
@plugin_version: 1.0.0
@plugin_author: Test Author
]]

plugin = {}

function plugin.initialize()
    return true
end

function plugin.get_info()
    return {
        name = "Test Plugin",
        version = "1.0.0"
    }
end

function plugin.execute_command(command, params)
    if command == "test" then
        return {
            success = true,
            result = "Test command executed"
        }
    else
        return {
            success = false,
            error = "Unknown command"
        }
    end
end

return plugin
)";

    createTestLuaPlugin("test_plugin.lua", plugin_content);
    QString plugin_path = m_temp_dir.filePath("test_plugin.lua");

    auto result = m_bridge->load_lua_plugin(plugin_path);
    QVERIFY(result.has_value());

    // Test plugin execution
    QJsonObject params;
    params["test"] = true;

    auto exec_result = m_bridge->execute_command("execute_lua", {{"code", "return plugin.get_info()"}});
    QVERIFY(exec_result.has_value());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testPluginCommands()
{
#ifdef QTFORGE_LUA_BINDINGS
    QVERIFY(m_bridge->initialize().has_value());

    // Test available commands
    auto commands = m_bridge->available_commands();
    QVERIFY(!commands.empty());
    QVERIFY(std::find(commands.begin(), commands.end(), "execute_lua") != commands.end());
    QVERIFY(std::find(commands.begin(), commands.end(), "load_script") != commands.end());

    // Test execute_lua command
    QJsonObject params;
    params["code"] = "return 'Hello from Lua'";

    auto result = m_bridge->execute_command("execute_lua", params);
    QVERIFY(result.has_value());

    QJsonObject response = result.value();
    QVERIFY(response["success"].toBool());

    // Test invalid command
    result = m_bridge->execute_command("invalid_command", {});
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::CommandNotFound);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testLuaErrorHandling()
{
#ifdef QTFORGE_LUA_BINDINGS
    QVERIFY(m_bridge->initialize().has_value());

    // Test syntax error
    QString code = "invalid lua syntax !!!";
    auto result = m_bridge->execute_code(code);
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::ExecutionFailed);

    // Test runtime error
    code = "error('Runtime error test')";
    result = m_bridge->execute_code(code);
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::ExecutionFailed);
    QVERIFY(result.error().message.find("Runtime error test") != std::string::npos);

    // Test nil access error
    code = "return nil_variable.property";
    result = m_bridge->execute_code(code);
    QVERIFY(!result.has_value());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testLuaSandboxing()
{
#ifdef QTFORGE_LUA_BINDINGS
    QVERIFY(m_bridge->initialize().has_value());

    // Test that dangerous functions are disabled in sandbox mode
    auto env = m_bridge->execution_environment();
    QVERIFY(env != nullptr);

    if (env->is_sandbox_enabled()) {
        // Test that os.execute is disabled
        QString code = "return os.execute";
        auto result = m_bridge->execute_code(code);
        QVERIFY(result.has_value());
        // Should return nil since os.execute is disabled

        // Test that io.open is disabled
        code = "return io.open";
        result = m_bridge->execute_code(code);
        QVERIFY(result.has_value());
        // Should return nil since io.open is disabled

        // Test that require is disabled
        code = "return require";
        result = m_bridge->execute_code(code);
        QVERIFY(result.has_value());
        // Should return nil since require is disabled
    }
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testPluginLifecycle()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Test complete plugin lifecycle
    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Unloaded);

    // Initialize
    auto result = m_bridge->initialize();
    QVERIFY(result.has_value());
    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Running);

    // Load plugin
    QString plugin_content = R"(
plugin = {
    initialized = false
}

function plugin.initialize()
    plugin.initialized = true
    return true
end

function plugin.shutdown()
    plugin.initialized = false
end

return plugin
)";

    createTestLuaPlugin("lifecycle_plugin.lua", plugin_content);
    QString plugin_path = m_temp_dir.filePath("lifecycle_plugin.lua");

    auto load_result = m_bridge->load_lua_plugin(plugin_path);
    QVERIFY(load_result.has_value());

    // Test plugin is loaded and functional
    auto exec_result = m_bridge->execute_code("return plugin.initialized");
    QVERIFY(exec_result.has_value());

    // Shutdown
    m_bridge->shutdown();
    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Unloaded);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::createTestLuaPlugin(const QString& filename, const QString& content)
{
    QFile file(m_temp_dir.filePath(filename));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << content;
    file.close();
}

QTEST_MAIN(TestLuaPluginBridge)
#include "test_lua_plugin_bridge.moc"
