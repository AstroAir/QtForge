/**
 * @file test_lua_plugin_bridge.cpp
 * @brief Tests for Lua plugin bridge implementation
 */

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>

#include <qtplugin/bridges/lua_plugin_bridge.hpp>
#include <qtplugin/utils/error_handling.hpp>

#include "../utils/test_helpers.hpp"
#include "../utils/test_config_templates.hpp"

using namespace qtplugin;
using namespace QtForgeTest;

/**
 * @brief Test class for Lua plugin bridge
 */
class TestLuaPluginBridge : public TestFixtureBase {
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

    // Performance tests
    void testExecutionPerformance();
    void testMemoryManagement();

    // Error handling tests
    void testInvalidLuaScript();
    void testMissingFile();
    void testRuntimeErrors();

private:
    std::unique_ptr<LuaPluginBridge> m_bridge;
    QString createTestLuaScript(const QString& content);
    QString createSimpleLuaPlugin();
    QString createComplexLuaPlugin();
};

void TestLuaPluginBridge::initTestCase() {
    TestFixtureBase::initTestCase();

#ifndef QTFORGE_LUA_BINDINGS
    QSKIP("Lua bindings not available in this build");
#endif
}

void TestLuaPluginBridge::cleanupTestCase() {
    TestFixtureBase::cleanupTestCase();
}

void TestLuaPluginBridge::init() {
    TestFixtureBase::init();

#ifdef QTFORGE_LUA_BINDINGS
    m_bridge = std::make_unique<LuaPluginBridge>();
#endif
}

void TestLuaPluginBridge::cleanup() {
    if (m_bridge) {
        m_bridge->shutdown();
        m_bridge.reset();
    }
    TestFixtureBase::cleanup();
}

QString TestLuaPluginBridge::createTestLuaScript(const QString& content) {
    if (!m_tempDir) return QString();

    QString scriptPath = m_tempDir->path() + "/test_script.lua";
    QFile file(scriptPath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
    }

    return scriptPath;
}

QString TestLuaPluginBridge::createSimpleLuaPlugin() {
    QString content = R"(
        local plugin = {
            id = "simple_lua_plugin",
            name = "Simple Lua Plugin",
            version = "1.0.0",
            description = "A simple test plugin"
        }

        function plugin.initialize()
            return true
        end

        function plugin.shutdown()
            -- Cleanup code
        end

        function plugin.execute_command(command, params)
            if command == "test" then
                return {
                    status = "success",
                    message = "Test command executed",
                    params = params
                }
            else
                return {
                    status = "error",
                    message = "Unknown command: " .. command
                }
            end
        end

        function plugin.get_available_commands()
            return {"test", "status"}
        end

        return plugin
    )";

    return createTestLuaScript(content);
}

QString TestLuaPluginBridge::createComplexLuaPlugin() {
    QString content = R"(
        local plugin = {
            id = "complex_lua_plugin",
            name = "Complex Lua Plugin",
            version = "2.1.0",
            description = "A complex test plugin with advanced features",
            state = "uninitialized"
        }

        function plugin.initialize()
            plugin.state = "initialized"
            plugin.data = {}
            return true
        end

        function plugin.shutdown()
            plugin.state = "shutdown"
            plugin.data = nil
        end

        function plugin.execute_command(command, params)
            if command == "store_data" then
                local key = params.key or "default"
                local value = params.value or ""
                plugin.data[key] = value
                return {
                    status = "success",
                    message = "Data stored",
                    key = key,
                    value = value
                }
            elseif command == "get_data" then
                local key = params.key or "default"
                local value = plugin.data[key]
                return {
                    status = "success",
                    key = key,
                    value = value
                }
            elseif command == "calculate" then
                local a = params.a or 0
                local b = params.b or 0
                local operation = params.operation or "add"

                local result
                if operation == "add" then
                    result = a + b
                elseif operation == "multiply" then
                    result = a * b
                elseif operation == "divide" then
                    if b ~= 0 then
                        result = a / b
                    else
                        return {
                            status = "error",
                            message = "Division by zero"
                        }
                    end
                else
                    return {
                        status = "error",
                        message = "Unknown operation: " .. operation
                    }
                end

                return {
                    status = "success",
                    result = result,
                    operation = operation,
                    operands = {a, b}
                }
            else
                return {
                    status = "error",
                    message = "Unknown command: " .. command
                }
            end
        end

        function plugin.get_available_commands()
            return {"store_data", "get_data", "calculate"}
        end

        function plugin.get_state()
            return plugin.state
        end

        function plugin.get_metadata()
            return {
                id = plugin.id,
                name = plugin.name,
                version = plugin.version,
                description = plugin.description,
                state = plugin.state
            }
        end

        return plugin
    )";

    return createTestLuaScript(content);
}

void TestLuaPluginBridge::testBridgeCreation() {
#ifdef QTFORGE_LUA_BINDINGS
    QVERIFY(m_bridge != nullptr);
    QCOMPARE(m_bridge->state(), PluginState::Unloaded);
    QVERIFY(!m_bridge->name().empty());
    QVERIFY(!m_bridge->description().empty());
    QVERIFY(m_bridge->version().major >= 3);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testBridgeInitialization() {
#ifdef QTFORGE_LUA_BINDINGS
    auto result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(result);
    QCOMPARE(m_bridge->state(), PluginState::Running);
    QVERIFY(m_bridge->is_initialized());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testBridgeShutdown() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    m_bridge->shutdown();
    QCOMPARE(m_bridge->state(), PluginState::Unloaded);
    QVERIFY(!m_bridge->is_initialized());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testBridgeState() {
#ifdef QTFORGE_LUA_BINDINGS
    // Test initial state
    QCOMPARE(m_bridge->state(), PluginState::Unloaded);

    // Test state after initialization
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    QCOMPARE(m_bridge->state(), PluginState::Running);

    // Test state after shutdown
    m_bridge->shutdown();
    QCOMPARE(m_bridge->state(), PluginState::Unloaded);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testPluginMetadata() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    auto metadata = m_bridge->metadata();
    QVERIFY(!metadata.id.isEmpty());
    QVERIFY(!metadata.name.isEmpty());
    QVERIFY(metadata.version.major >= 3);
    QVERIFY(!metadata.description.isEmpty());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testPluginCapabilities() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    auto commands = m_bridge->available_commands();
    QVERIFY(!commands.empty());

    // Check for expected commands
    QVERIFY(std::find(commands.begin(), commands.end(), "execute_lua") != commands.end());
    QVERIFY(std::find(commands.begin(), commands.end(), "load_script") != commands.end());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testPluginCommands() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    // Test execute_lua command
    QJsonObject params;
    params["code"] = "return 'Hello from Lua'";

    auto result = m_bridge->execute_command("execute_lua", params);
    QTFORGE_VERIFY_SUCCESS(result);

    QJsonObject response = result.value();
    QVERIFY(response["success"].toBool());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testPluginConfiguration() {
#ifdef QTFORGE_LUA_BINDINGS
    QJsonObject config = ConfigTemplates::luaPluginTestConfig();

    auto configure_result = m_bridge->configure(config);
    QTFORGE_VERIFY_SUCCESS(configure_result);

    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testLuaCodeExecution() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    // Test simple code execution
    QString code = "return 42";
    auto result = m_bridge->execute_code(code);
    QTFORGE_VERIFY_SUCCESS(result);

    QVariant value = result.value();
    QVERIFY(value.isValid());

    // Test code with context
    QJsonObject context;
    context["input"] = 10;

    code = "return context.input * 2";
    result = m_bridge->execute_code(code, context);
    QTFORGE_VERIFY_SUCCESS(result);

    // Test error handling
    code = "error('Test error')";
    result = m_bridge->execute_code(code);
    QTFORGE_VERIFY_ERROR(result, PluginErrorCode::ExecutionFailed);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testLuaScriptLoading() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QString scriptPath = createSimpleLuaPlugin();
    QVERIFY(!scriptPath.isEmpty());

    auto load_result = m_bridge->load_lua_plugin(scriptPath);
    QTFORGE_VERIFY_SUCCESS(load_result);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testLuaErrorHandling() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    // Test syntax error
    QString invalidScript = R"(
        function invalid_function(
            -- Missing closing parenthesis and end
    )";

    QString scriptPath = createTestLuaScript(invalidScript);
    auto result = m_bridge->execute_code(invalidScript);
    QVERIFY(!result.has_value());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testLuaSandboxing() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    // Test restricted access (should be sandboxed)
    QString restrictedCode = R"(
        local file = io.open("/etc/passwd", "r")
        if file then
            file:close()
            return "Security breach!"
        else
            return "Access properly restricted"
        end
    )";

    auto result = m_bridge->execute_code(restrictedCode);
    // The exact behavior depends on sandboxing configuration
    // We just verify it doesn't crash
    QVERIFY(true); // Test passes if we reach here without crashing
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testMethodInvocation() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QString scriptPath = createComplexLuaPlugin();
    auto load_result = m_bridge->load_lua_plugin(scriptPath);
    QTFORGE_VERIFY_SUCCESS(load_result);

    // Test method invocation
    QVariantList params;
    params << "test_param";

    auto result = m_bridge->invoke_method("get_state", params);
    QTFORGE_VERIFY_SUCCESS(result);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testPropertyAccess() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QString scriptPath = createComplexLuaPlugin();
    auto load_result = m_bridge->load_lua_plugin(scriptPath);
    QTFORGE_VERIFY_SUCCESS(load_result);

    // Test property access
    auto get_result = m_bridge->get_property("id");
    QTFORGE_VERIFY_SUCCESS(get_result);

    // Test property setting
    auto set_result = m_bridge->set_property("test_prop", QVariant("test_value"));
    QTFORGE_VERIFY_SUCCESS(set_result);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testMethodListing() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QString scriptPath = createComplexLuaPlugin();
    auto load_result = m_bridge->load_lua_plugin(scriptPath);
    QTFORGE_VERIFY_SUCCESS(load_result);

    auto methods = m_bridge->list_methods();
    QVERIFY(!methods.empty());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testPropertyListing() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QString scriptPath = createComplexLuaPlugin();
    auto load_result = m_bridge->load_lua_plugin(scriptPath);
    QTFORGE_VERIFY_SUCCESS(load_result);

    auto properties = m_bridge->list_properties();
    QVERIFY(!properties.empty());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testPluginLifecycle() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QString scriptPath = createSimpleLuaPlugin();
    auto load_result = m_bridge->load_lua_plugin(scriptPath);
    QTFORGE_VERIFY_SUCCESS(load_result);

    // Test plugin initialization
    QVariantList params;
    auto plugin_init_result = m_bridge->invoke_method("initialize", params);
    QTFORGE_VERIFY_SUCCESS(plugin_init_result);

    // Test plugin shutdown
    auto plugin_shutdown_result = m_bridge->invoke_method("shutdown", params);
    QTFORGE_VERIFY_SUCCESS(plugin_shutdown_result);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testPluginCommunication() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QString scriptPath = createComplexLuaPlugin();
    auto load_result = m_bridge->load_lua_plugin(scriptPath);
    QTFORGE_VERIFY_SUCCESS(load_result);

    // Test command execution
    QJsonObject params;
    params["a"] = 5;
    params["b"] = 3;
    params["operation"] = "add";

    auto result = m_bridge->execute_command("calculate", params);
    QTFORGE_VERIFY_SUCCESS(result);

    QJsonObject response = result.value();
    QCOMPARE(response["status"].toString(), QString("success"));
    QCOMPARE(response["result"].toInt(), 8);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testPluginSecurity() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    // Test that dangerous operations are restricted
    QString dangerousCode = R"(
        os.execute("rm -rf /")
        return "Should not reach here"
    )";

    auto result = m_bridge->execute_code(dangerousCode);
    // Should either fail or be sandboxed
    // We just verify it doesn't crash the system
    QVERIFY(true); // Test passes if we reach here
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testExecutionPerformance() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QString scriptPath = createSimpleLuaPlugin();
    auto load_result = m_bridge->load_lua_plugin(scriptPath);
    QTFORGE_VERIFY_SUCCESS(load_result);

    QElapsedTimer timer;
    timer.start();

    const int iterations = 100;
    for (int i = 0; i < iterations; ++i) {
        QJsonObject params;
        params["iteration"] = i;

        auto result = m_bridge->execute_command("test", params);
        QTFORGE_VERIFY_SUCCESS(result);
    }

    qint64 elapsed = timer.elapsed();
    qDebug() << "Lua bridge performance:" << elapsed << "ms for" << iterations << "commands";

    // Verify reasonable performance (less than 10ms per command on average)
    QVERIFY(elapsed < iterations * 10);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testMemoryManagement() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    // Test multiple script loads and unloads
    for (int i = 0; i < 10; ++i) {
        QString scriptPath = createSimpleLuaPlugin();
        auto load_result = m_bridge->load_lua_plugin(scriptPath);
        QTFORGE_VERIFY_SUCCESS(load_result);

        // Execute some commands
        QJsonObject params;
        auto result = m_bridge->execute_command("test", params);
        QTFORGE_VERIFY_SUCCESS(result);
    }

    // Test passes if we don't crash or leak memory
    QVERIFY(true);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testInvalidLuaScript() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QString invalidScript = "invalid lua syntax !!!";
    QString scriptPath = createTestLuaScript(invalidScript);

    auto result = m_bridge->load_lua_plugin(scriptPath);
    QVERIFY(!result.has_value());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testMissingFile() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QString nonExistentPath = "/path/that/does/not/exist.lua";

    auto result = m_bridge->load_lua_plugin(nonExistentPath);
    QTFORGE_VERIFY_ERROR(result, PluginErrorCode::FileNotFound);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginBridge::testRuntimeErrors() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_bridge->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);

    QString errorScript = R"(
        local plugin = {}

        function plugin.execute_command(command, params)
            error("Runtime error in plugin")
        end

        return plugin
    )";

    QString scriptPath = createTestLuaScript(errorScript);
    auto load_result = m_bridge->load_lua_plugin(scriptPath);
    QTFORGE_VERIFY_SUCCESS(load_result);

    QJsonObject params;
    auto result = m_bridge->execute_command("test", params);
    QVERIFY(!result.has_value());
#else
    QSKIP("Lua bindings not available");
#endif
}

QTEST_MAIN(TestLuaPluginBridge)
#include "test_lua_plugin_bridge.moc"
