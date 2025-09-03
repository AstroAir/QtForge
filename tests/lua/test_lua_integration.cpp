/**
 * @file test_lua_integration.cpp
 * @brief Integration tests for complete Lua plugin system
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "../../include/qtplugin/core/plugin_manager.hpp"
#include "../../include/qtplugin/core/lua_plugin_loader.hpp"
#include "../../include/qtplugin/bridges/lua_plugin_bridge.hpp"

class TestLuaIntegration : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // End-to-end plugin lifecycle tests
    void testCompletePluginLifecycle();
    void testPluginManagerIntegration();
    void testMultiplePluginManagement();
    
    // Plugin communication tests
    void testInterPluginCommunication();
    void testMessageBusIntegration();
    void testRequestResponseIntegration();
    
    // Security integration tests
    void testPluginSecurity();
    void testSandboxEnforcement();
    void testPermissionManagement();
    
    // Error handling integration tests
    void testErrorPropagation();
    void testRecoveryMechanisms();
    void testResourceCleanup();
    
    // Performance integration tests
    void testPluginPerformance();
    void testMemoryManagement();
    void testConcurrentExecution();
    
    // Real-world scenario tests
    void testDataProcessingPlugin();
    void testServicePlugin();
    void testUIIntegrationPlugin();
    void testConfigurationPlugin();

private:
    void createComplexLuaPlugin(const QString& filename, const QString& content);
    void createDataProcessingPlugin();
    void createServicePlugin();
    void createConfigurationPlugin();
    
    QTemporaryDir m_temp_dir;
    std::unique_ptr<qtplugin::PluginManager> m_plugin_manager;
};

void TestLuaIntegration::initTestCase()
{
    qDebug() << "Starting Lua integration test suite";
    QVERIFY(m_temp_dir.isValid());
    
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available for integration tests");
    }
}

void TestLuaIntegration::cleanupTestCase()
{
    qDebug() << "Lua integration test suite completed";
}

void TestLuaIntegration::init()
{
    if (qtplugin::LuaPluginLoader::is_lua_available()) {
        m_plugin_manager = std::make_unique<qtplugin::PluginManager>();
        
        // Register Lua plugin loader
        qtplugin::LuaPluginLoaderFactory::register_with_factory();
    }
}

void TestLuaIntegration::cleanup()
{
    if (m_plugin_manager) {
        m_plugin_manager->shutdown_all_plugins();
        m_plugin_manager.reset();
    }
}

void TestLuaIntegration::testCompletePluginLifecycle()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }
    
    QString plugin_content = R"(
--[[
@plugin_name: Lifecycle Test Plugin
@plugin_description: Tests complete plugin lifecycle
@plugin_version: 1.0.0
@plugin_author: Test Suite
]]

local plugin = {}
local state = {
    initialized = false,
    command_count = 0,
    last_command = nil
}

function plugin.initialize()
    qtforge.log.info("Plugin initializing...")
    state.initialized = true
    return {success = true}
end

function plugin.shutdown()
    qtforge.log.info("Plugin shutting down...")
    state.initialized = false
end

function plugin.get_state()
    return state
end

function plugin.execute_command(command, params)
    if not state.initialized then
        return {success = false, error = "Plugin not initialized"}
    end
    
    state.command_count = state.command_count + 1
    state.last_command = command
    
    if command == "ping" then
        return {success = true, result = "pong"}
    elseif command == "echo" then
        return {success = true, result = params.message or "empty"}
    elseif command == "get_stats" then
        return {
            success = true,
            result = {
                command_count = state.command_count,
                last_command = state.last_command,
                initialized = state.initialized
            }
        }
    else
        return {success = false, error = "Unknown command: " .. command}
    end
end

return plugin
)";
    
    createComplexLuaPlugin("lifecycle_plugin.lua", plugin_content);
    std::filesystem::path plugin_path(m_temp_dir.filePath("lifecycle_plugin.lua").toStdString());
    
    // Load plugin through plugin manager
    qtplugin::PluginLoadOptions options;
    options.initialize_immediately = true;
    
    auto load_result = m_plugin_manager->load_plugin(plugin_path, options);
    QVERIFY(load_result.has_value());
    
    std::string plugin_id = load_result.value();
    QVERIFY(!plugin_id.empty());
    
    // Verify plugin is loaded and running
    auto plugin = m_plugin_manager->get_plugin(plugin_id);
    QVERIFY(plugin != nullptr);
    QCOMPARE(plugin->state(), qtplugin::PluginState::Running);
    
    // Test plugin commands
    QJsonObject ping_params;
    auto ping_result = plugin->execute_command("ping", ping_params);
    QVERIFY(ping_result.has_value());
    QVERIFY(ping_result.value()["success"].toBool());
    QCOMPARE(ping_result.value()["result"].toString(), QString("pong"));
    
    // Test echo command
    QJsonObject echo_params;
    echo_params["message"] = "Hello, World!";
    auto echo_result = plugin->execute_command("echo", echo_params);
    QVERIFY(echo_result.has_value());
    QVERIFY(echo_result.value()["success"].toBool());
    QCOMPARE(echo_result.value()["result"].toString(), QString("Hello, World!"));
    
    // Test stats command
    auto stats_result = plugin->execute_command("get_stats", {});
    QVERIFY(stats_result.has_value());
    QVERIFY(stats_result.value()["success"].toBool());
    
    QJsonObject stats = stats_result.value()["result"].toObject();
    QCOMPARE(stats["command_count"].toInt(), 3); // ping, echo, get_stats
    QCOMPARE(stats["last_command"].toString(), QString("get_stats"));
    QVERIFY(stats["initialized"].toBool());
    
    // Unload plugin
    auto unload_result = m_plugin_manager->unload_plugin(plugin_id, false);
    QVERIFY(unload_result.has_value());
    
    // Verify plugin is unloaded
    QVERIFY(!m_plugin_manager->has_plugin(plugin_id));
}

void TestLuaIntegration::testPluginManagerIntegration()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }
    
    // Create multiple test plugins
    createDataProcessingPlugin();
    createServicePlugin();
    createConfigurationPlugin();
    
    // Load all plugins
    auto data_path = std::filesystem::path(m_temp_dir.filePath("data_processor.lua").toStdString());
    auto service_path = std::filesystem::path(m_temp_dir.filePath("service_plugin.lua").toStdString());
    auto config_path = std::filesystem::path(m_temp_dir.filePath("config_plugin.lua").toStdString());
    
    qtplugin::PluginLoadOptions options;
    options.initialize_immediately = true;
    
    auto data_result = m_plugin_manager->load_plugin(data_path, options);
    auto service_result = m_plugin_manager->load_plugin(service_path, options);
    auto config_result = m_plugin_manager->load_plugin(config_path, options);
    
    QVERIFY(data_result.has_value());
    QVERIFY(service_result.has_value());
    QVERIFY(config_result.has_value());
    
    // Verify all plugins are loaded
    QCOMPARE(m_plugin_manager->get_plugin_count(), 3u);
    
    auto plugin_ids = m_plugin_manager->get_plugin_ids();
    QCOMPARE(plugin_ids.size(), 3u);
    
    // Test plugin discovery
    auto discovered = m_plugin_manager->discover_plugins(
        std::filesystem::path(m_temp_dir.path().toStdString()), false);
    QVERIFY(discovered.size() >= 3);
    
    // Test system metrics
    auto metrics = m_plugin_manager->system_metrics();
    QVERIFY(metrics.contains("plugin_count"));
    QCOMPARE(metrics["plugin_count"].toInt(), 3);
}

void TestLuaIntegration::testInterPluginCommunication()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }
    
    QString sender_plugin = R"(
local plugin = {}

function plugin.initialize()
    return {success = true}
end

function plugin.send_message(topic, message)
    -- This would use the message bus to send a message
    qtforge.log.info("Sending message to topic: " .. topic)
    return {success = true, message_id = "msg_123"}
end

return plugin
)";
    
    QString receiver_plugin = R"(
local plugin = {}
local received_messages = {}

function plugin.initialize()
    -- Subscribe to messages
    return {success = true}
end

function plugin.handle_message(message)
    table.insert(received_messages, message)
    qtforge.log.info("Received message: " .. tostring(message.content))
    return {success = true}
end

function plugin.get_received_messages()
    return {success = true, result = received_messages}
end

return plugin
)";
    
    createComplexLuaPlugin("sender.lua", sender_plugin);
    createComplexLuaPlugin("receiver.lua", receiver_plugin);
    
    // Load both plugins
    qtplugin::PluginLoadOptions options;
    options.initialize_immediately = true;
    
    auto sender_path = std::filesystem::path(m_temp_dir.filePath("sender.lua").toStdString());
    auto receiver_path = std::filesystem::path(m_temp_dir.filePath("receiver.lua").toStdString());
    
    auto sender_result = m_plugin_manager->load_plugin(sender_path, options);
    auto receiver_result = m_plugin_manager->load_plugin(receiver_path, options);
    
    QVERIFY(sender_result.has_value());
    QVERIFY(receiver_result.has_value());
    
    // Test communication between plugins
    auto sender = m_plugin_manager->get_plugin(sender_result.value());
    auto receiver = m_plugin_manager->get_plugin(receiver_result.value());
    
    QVERIFY(sender != nullptr);
    QVERIFY(receiver != nullptr);
    
    // This test would require actual message bus implementation
    // For now, just verify plugins are loaded and can execute commands
    QJsonObject send_params;
    send_params["topic"] = "test_topic";
    send_params["message"] = "Hello from sender!";
    
    auto send_result = sender->execute_command("send_message", send_params);
    // Would verify message was sent and received in full implementation
}

void TestLuaIntegration::createComplexLuaPlugin(const QString& filename, const QString& content)
{
    QFile file(m_temp_dir.filePath(filename));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << content;
    file.close();
}

void TestLuaIntegration::createDataProcessingPlugin()
{
    QString content = R"(
--[[
@plugin_name: Data Processor
@plugin_description: Processes data using Lua
@plugin_version: 1.0.0
@plugin_author: Test Suite
]]

local plugin = {}

function plugin.initialize()
    return {success = true}
end

function plugin.process_data(data)
    local result = {}
    for i, item in ipairs(data) do
        result[i] = item * 2  -- Simple processing: double each value
    end
    return {success = true, result = result}
end

return plugin
)";
    
    createComplexLuaPlugin("data_processor.lua", content);
}

void TestLuaIntegration::createServicePlugin()
{
    QString content = R"(
--[[
@plugin_name: Service Plugin
@plugin_description: Provides service functionality
@plugin_version: 1.0.0
@plugin_author: Test Suite
]]

local plugin = {}
local service_state = {running = false, requests = 0}

function plugin.initialize()
    service_state.running = true
    return {success = true}
end

function plugin.start_service()
    service_state.running = true
    return {success = true, message = "Service started"}
end

function plugin.stop_service()
    service_state.running = false
    return {success = true, message = "Service stopped"}
end

function plugin.handle_request(request)
    if not service_state.running then
        return {success = false, error = "Service not running"}
    end
    
    service_state.requests = service_state.requests + 1
    return {
        success = true,
        result = "Processed request #" .. service_state.requests,
        request_id = service_state.requests
    }
end

return plugin
)";
    
    createComplexLuaPlugin("service_plugin.lua", content);
}

void TestLuaIntegration::createConfigurationPlugin()
{
    QString content = R"(
--[[
@plugin_name: Configuration Plugin
@plugin_description: Manages configuration settings
@plugin_version: 1.0.0
@plugin_author: Test Suite
]]

local plugin = {}
local config = {}

function plugin.initialize()
    config = {
        setting1 = "default_value1",
        setting2 = 42,
        setting3 = true
    }
    return {success = true}
end

function plugin.get_config(key)
    if key then
        return {success = true, result = config[key]}
    else
        return {success = true, result = config}
    end
end

function plugin.set_config(key, value)
    config[key] = value
    return {success = true, message = "Configuration updated"}
end

function plugin.validate_config(new_config)
    -- Simple validation
    for key, value in pairs(new_config) do
        if type(key) ~= "string" then
            return {success = false, error = "Keys must be strings"}
        end
    end
    return {success = true, message = "Configuration is valid"}
end

return plugin
)";
    
    createComplexLuaPlugin("config_plugin.lua", content);
}

QTEST_MAIN(TestLuaIntegration)
#include "test_lua_integration.moc"
