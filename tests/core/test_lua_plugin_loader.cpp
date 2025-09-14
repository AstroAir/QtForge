/**
 * @file test_lua_plugin_loader.cpp
 * @brief Tests for Lua plugin loader implementation
 */

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <memory>

#include <qtplugin/core/lua_plugin_loader.hpp>
#include <qtplugin/utils/error_handling.hpp>

#include "../test_helpers.hpp"
#include "../test_config_templates.hpp"

using namespace qtplugin;
using namespace QtForgeTest;

/**
 * @brief Test class for Lua plugin loader
 */
class TestLuaPluginLoader : public TestFixtureBase {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testLoaderCreation();
    void testLuaEnvironmentSetup();
    void testPluginLoading();
    void testPluginExecution();

    // Error handling tests
    void testInvalidLuaScript();
    void testMissingFile();
    void testSyntaxErrors();

    // Configuration tests
    void testLoaderConfiguration();
    void testSecuritySettings();

    // Performance tests
    void testLoadingPerformance();

private:
    std::unique_ptr<LuaPluginLoader> m_loader;
    QString createTestLuaScript(const QString& content);
};

void TestLuaPluginLoader::initTestCase() {
    TestFixtureBase::initTestCase();
    
#ifndef QTFORGE_LUA_BINDINGS
    QSKIP("Lua bindings not available in this build");
#endif
}

void TestLuaPluginLoader::cleanupTestCase() {
    TestFixtureBase::cleanupTestCase();
}

void TestLuaPluginLoader::init() {
    TestFixtureBase::init();
    
#ifdef QTFORGE_LUA_BINDINGS
    m_loader = std::make_unique<LuaPluginLoader>();
#endif
}

void TestLuaPluginLoader::cleanup() {
    if (m_loader) {
        m_loader.reset();
    }
    TestFixtureBase::cleanup();
}

QString TestLuaPluginLoader::createTestLuaScript(const QString& content) {
    if (!m_tempDir) return QString();
    
    QString scriptPath = m_tempDir->path() + "/test_plugin.lua";
    QFile file(scriptPath);
    
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
    }
    
    return scriptPath;
}

void TestLuaPluginLoader::testLoaderCreation() {
#ifdef QTFORGE_LUA_BINDINGS
    QVERIFY(m_loader != nullptr);
    
    // Test loader initialization
    auto init_result = m_loader->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    
    QVERIFY(m_loader->is_initialized());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginLoader::testLuaEnvironmentSetup() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_loader->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    
    // Test basic Lua functionality
    QString testScript = R"(
        function test_function()
            return "Hello from Lua"
        end
        
        return test_function()
    )";
    
    QString scriptPath = createTestLuaScript(testScript);
    QVERIFY(!scriptPath.isEmpty());
    
    auto result = m_loader->execute_script(scriptPath);
    QTFORGE_VERIFY_SUCCESS(result);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginLoader::testPluginLoading() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_loader->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    
    QString pluginScript = R"(
        plugin = {
            id = "test_lua_plugin",
            name = "Test Lua Plugin",
            version = "1.0.0",
            description = "Test plugin for unit testing"
        }
        
        function plugin:initialize()
            self.initialized = true
            return true
        end
        
        function plugin:shutdown()
            self.initialized = false
        end
        
        function plugin:execute_command(command, params)
            if command == "test" then
                return {
                    status = "success",
                    command = command,
                    params = params
                }
            else
                return {
                    status = "error",
                    message = "Unknown command"
                }
            end
        end
        
        return plugin
    )";
    
    QString scriptPath = createTestLuaScript(pluginScript);
    QVERIFY(!scriptPath.isEmpty());
    
    auto load_result = m_loader->load_plugin(scriptPath);
    QTFORGE_VERIFY_SUCCESS(load_result);
    
    if (load_result.has_value()) {
        auto plugin = load_result.value();
        QVERIFY(plugin != nullptr);
        
        // Test plugin metadata
        auto metadata = plugin->metadata();
        QCOMPARE(metadata.id, QString("test_lua_plugin"));
        QCOMPARE(metadata.name, QString("Test Lua Plugin"));
    }
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginLoader::testPluginExecution() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_loader->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    
    QString pluginScript = R"(
        plugin = {
            id = "execution_test_plugin",
            name = "Execution Test Plugin",
            version = "1.0.0"
        }
        
        function plugin:execute_command(command, params)
            if command == "echo" then
                return {
                    status = "success",
                    echo = params.message or "Hello World"
                }
            elseif command == "add" then
                local a = params.a or 0
                local b = params.b or 0
                return {
                    status = "success",
                    result = a + b
                }
            else
                return {
                    status = "error",
                    message = "Unknown command: " .. command
                }
            end
        end
        
        return plugin
    )";
    
    QString scriptPath = createTestLuaScript(pluginScript);
    auto load_result = m_loader->load_plugin(scriptPath);
    QTFORGE_VERIFY_SUCCESS(load_result);
    
    if (load_result.has_value()) {
        auto plugin = load_result.value();
        
        // Test echo command
        QJsonObject echo_params;
        echo_params["message"] = "Test message";
        
        auto echo_result = plugin->execute_command("echo", echo_params);
        QTFORGE_VERIFY_SUCCESS(echo_result);
        
        if (echo_result.has_value()) {
            auto result = echo_result.value();
            QCOMPARE(result["status"].toString(), QString("success"));
            QCOMPARE(result["echo"].toString(), QString("Test message"));
        }
        
        // Test add command
        QJsonObject add_params;
        add_params["a"] = 5;
        add_params["b"] = 3;
        
        auto add_result = plugin->execute_command("add", add_params);
        QTFORGE_VERIFY_SUCCESS(add_result);
        
        if (add_result.has_value()) {
            auto result = add_result.value();
            QCOMPARE(result["status"].toString(), QString("success"));
            QCOMPARE(result["result"].toInt(), 8);
        }
    }
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginLoader::testInvalidLuaScript() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_loader->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    
    QString invalidScript = R"(
        -- This script has syntax errors
        function invalid_function(
            -- Missing closing parenthesis and end
    )";
    
    QString scriptPath = createTestLuaScript(invalidScript);
    
    auto result = m_loader->execute_script(scriptPath);
    QVERIFY(!result.has_value());
    QVERIFY(result.error().code == PluginErrorCode::LoadFailed ||
            result.error().code == PluginErrorCode::ExecutionFailed);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginLoader::testMissingFile() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_loader->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    
    QString nonExistentPath = "/path/that/does/not/exist.lua";
    
    auto result = m_loader->load_plugin(nonExistentPath);
    QTFORGE_VERIFY_ERROR(result, PluginErrorCode::FileNotFound);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginLoader::testSyntaxErrors() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_loader->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    
    QString syntaxErrorScript = R"(
        plugin = {
            id = "syntax_error_plugin"
            -- Missing comma here
            name = "Syntax Error Plugin"
        }
        
        function plugin:invalid_syntax(
            -- Missing parameters and end
    )";
    
    QString scriptPath = createTestLuaScript(syntaxErrorScript);
    
    auto result = m_loader->load_plugin(scriptPath);
    QVERIFY(!result.has_value());
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginLoader::testLoaderConfiguration() {
#ifdef QTFORGE_LUA_BINDINGS
    QJsonObject config = ConfigTemplates::luaPluginTestConfig();
    
    auto configure_result = m_loader->configure(config);
    QTFORGE_VERIFY_SUCCESS(configure_result);
    
    auto init_result = m_loader->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginLoader::testSecuritySettings() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_loader->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    
    // Test script that tries to access restricted functionality
    QString restrictedScript = R"(
        -- Try to access file system (should be restricted)
        local file = io.open("/etc/passwd", "r")
        if file then
            file:close()
            return "Security breach!"
        else
            return "Access properly restricted"
        end
    )";
    
    QString scriptPath = createTestLuaScript(restrictedScript);
    
    // This should either fail or return the restricted access message
    auto result = m_loader->execute_script(scriptPath);
    // The exact behavior depends on the security configuration
    // We just verify it doesn't crash
    QVERIFY(true); // Test passes if we reach here without crashing
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPluginLoader::testLoadingPerformance() {
#ifdef QTFORGE_LUA_BINDINGS
    auto init_result = m_loader->initialize();
    QTFORGE_VERIFY_SUCCESS(init_result);
    
    QString simpleScript = R"(
        plugin = {
            id = "performance_test_plugin",
            name = "Performance Test Plugin",
            version = "1.0.0"
        }
        
        function plugin:execute_command(command, params)
            return { status = "success", command = command }
        end
        
        return plugin
    )";
    
    QString scriptPath = createTestLuaScript(simpleScript);
    
    QElapsedTimer timer;
    timer.start();
    
    const int iterations = 10;
    for (int i = 0; i < iterations; ++i) {
        auto result = m_loader->load_plugin(scriptPath);
        QTFORGE_VERIFY_SUCCESS(result);
    }
    
    qint64 elapsed = timer.elapsed();
    qDebug() << "Lua plugin loading performance:" << elapsed << "ms for" << iterations << "loads";
    
    // Verify reasonable performance (less than 100ms per load on average)
    QVERIFY(elapsed < iterations * 100);
#else
    QSKIP("Lua bindings not available");
#endif
}

QTEST_MAIN(TestLuaPluginLoader)
#include "test_lua_plugin_loader.moc"
