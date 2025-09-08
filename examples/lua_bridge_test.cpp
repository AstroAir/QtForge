/**
 * @file lua_bridge_test.cpp
 * @brief Test program for the enhanced Lua plugin bridge
 * @version 3.2.0
 */

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <iostream>

#include "qtplugin/bridges/lua_plugin_bridge.hpp"

void test_basic_functionality(qtplugin::LuaPluginBridge& bridge) {
    qDebug() << "\n=== Testing Basic Functionality ===";
    
    // Test plugin information
    qDebug() << "Plugin Name:" << QString::fromStdString(bridge.name());
    qDebug() << "Plugin Description:" << QString::fromStdString(bridge.description());
    qDebug() << "Plugin Version:" << bridge.version().to_string().c_str();
    qDebug() << "Plugin ID:" << QString::fromStdString(bridge.id());
    qDebug() << "Plugin State:" << static_cast<int>(bridge.state());
    qDebug() << "Plugin Capabilities:" << static_cast<int>(bridge.capabilities());
    
    // Test available commands
    auto commands = bridge.available_commands();
    qDebug() << "Available Commands:";
    for (const auto& cmd : commands) {
        qDebug() << "  -" << QString::fromStdString(cmd);
    }
}

void test_lua_execution(qtplugin::LuaPluginBridge& bridge) {
    qDebug() << "\n=== Testing Lua Code Execution ===";
    
    // Test simple Lua code execution
    QJsonObject params;
    params["code"] = "return qtforge.core.test_function()";
    
    auto result = bridge.execute_command("execute_lua", params);
    if (result) {
        qDebug() << "Lua execution result:" << QJsonDocument(result.value()).toJson(QJsonDocument::Compact);
    } else {
        qDebug() << "Lua execution failed:" << result.error().message.c_str();
    }
    
    // Test Lua code with context
    QJsonObject context;
    context["name"] = "Test User";
    context["value"] = 42;
    
    params["code"] = "return 'Hello ' .. context.name .. ', value is ' .. context.value";
    params["context"] = context;
    
    result = bridge.execute_command("execute_lua", params);
    if (result) {
        qDebug() << "Lua execution with context:" << QJsonDocument(result.value()).toJson(QJsonDocument::Compact);
    } else {
        qDebug() << "Lua execution with context failed:" << result.error().message.c_str();
    }
}

void test_plugin_loading(qtplugin::LuaPluginBridge& bridge) {
    qDebug() << "\n=== Testing Plugin Loading ===";
    
    // Load the test plugin
    QJsonObject params;
    params["path"] = "examples/lua_test_plugin.lua";
    
    auto result = bridge.execute_command("load_script", params);
    if (result) {
        qDebug() << "Plugin loaded successfully:" << QJsonDocument(result.value()).toJson(QJsonDocument::Compact);
    } else {
        qDebug() << "Plugin loading failed:" << result.error().message.c_str();
        return;
    }
    
    // Test plugin initialization
    params.remove("path");
    params["code"] = "return plugin.initialize()";
    
    result = bridge.execute_command("execute_lua", params);
    if (result) {
        qDebug() << "Plugin initialization:" << QJsonDocument(result.value()).toJson(QJsonDocument::Compact);
    } else {
        qDebug() << "Plugin initialization failed:" << result.error().message.c_str();
    }
}

void test_method_invocation(qtplugin::LuaPluginBridge& bridge) {
    qDebug() << "\n=== Testing Method Invocation ===";
    
    // Test method with parameters
    QList<QVariant> args;
    args << 10 << 20;
    
    auto result = bridge.invoke_method("test_method", args);
    if (result) {
        qDebug() << "Method invocation result:" << result.value();
    } else {
        qDebug() << "Method invocation failed:" << result.error().message.c_str();
    }
    
    // Test method without parameters
    args.clear();
    result = bridge.invoke_method("get_status", args);
    if (result) {
        qDebug() << "Status method result:" << result.value();
    } else {
        qDebug() << "Status method failed:" << result.error().message.c_str();
    }
}

void test_property_access(qtplugin::LuaPluginBridge& bridge) {
    qDebug() << "\n=== Testing Property Access ===";
    
    // Test property getter
    auto result = bridge.get_property("counter");
    if (result) {
        qDebug() << "Counter property:" << result.value();
    } else {
        qDebug() << "Get counter property failed:" << result.error().message.c_str();
    }
    
    // Test property setter
    auto set_result = bridge.set_property("message", "Updated from C++!");
    if (set_result) {
        qDebug() << "Property set successfully";
        
        // Verify the change
        auto get_result = bridge.get_property("message");
        if (get_result) {
            qDebug() << "Updated message property:" << get_result.value();
        }
    } else {
        qDebug() << "Set property failed:" << set_result.error().message.c_str();
    }
}

void test_advanced_features(qtplugin::LuaPluginBridge& bridge) {
    qDebug() << "\n=== Testing Advanced Features ===";
    
    // Test QtForge utilities through Lua
    QJsonObject params;
    params["code"] = R"(
        local uuid = qtforge.utils.generate_uuid()
        local timestamp = qtforge.utils.current_timestamp()
        local thread_count = qtforge.threading.get_thread_count()
        
        return {
            uuid = uuid,
            timestamp = timestamp,
            thread_count = thread_count,
            version_info = qtforge.version
        }
    )";
    
    auto result = bridge.execute_command("execute_lua", params);
    if (result) {
        qDebug() << "Advanced features test:" << QJsonDocument(result.value()).toJson(QJsonDocument::Compact);
    } else {
        qDebug() << "Advanced features test failed:" << result.error().message.c_str();
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "QtForge Lua Plugin Bridge Test";
    qDebug() << "==============================";
    
    // Create and initialize the Lua plugin bridge
    qtplugin::LuaPluginBridge bridge;
    
    auto init_result = bridge.initialize();
    if (!init_result) {
        qDebug() << "Failed to initialize Lua plugin bridge:" << init_result.error().message.c_str();
        return 1;
    }
    
    qDebug() << "Lua plugin bridge initialized successfully!";
    
    // Run tests
    test_basic_functionality(bridge);
    test_lua_execution(bridge);
    test_plugin_loading(bridge);
    test_method_invocation(bridge);
    test_property_access(bridge);
    test_advanced_features(bridge);
    
    // Shutdown
    bridge.shutdown();
    qDebug() << "\nAll tests completed!";
    
    return 0;
}
