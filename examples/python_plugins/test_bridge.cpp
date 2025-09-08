/**
 * @file test_bridge.cpp
 * @brief Test program for Python plugin bridge functionality
 */

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDateTime>

#include <qtplugin/bridges/python_plugin_bridge.hpp>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    qDebug() << "Testing Python Plugin Bridge";

    // Create a Python plugin bridge
    QString plugin_path = QDir::currentPath() + "/examples/python_plugins/sample_plugin.py";
    auto bridge = std::make_unique<qtplugin::PythonPluginBridge>(plugin_path);

    qDebug() << "Created bridge for plugin:" << plugin_path;

    // Test initialization
    auto init_result = bridge->initialize();
    if (!init_result) {
        qCritical() << "Failed to initialize bridge:" << init_result.error().message.c_str();
        return 1;
    }

    qDebug() << "Bridge initialized successfully";

    // Test basic plugin information
    qDebug() << "Plugin name:" << bridge->name().data();
    qDebug() << "Plugin description:" << bridge->description().data();
    qDebug() << "Plugin state:" << static_cast<int>(bridge->state());

    // Test available commands
    auto commands = bridge->available_commands();
    qDebug() << "Available commands:" << commands.size();
    for (const auto& cmd : commands) {
        qDebug() << "  -" << cmd.c_str();
    }

    // Test command execution
    QJsonObject params;
    params["test_data"] = "Hello from C++";

    auto exec_result = bridge->execute_command("process_data", params);
    if (exec_result) {
        qDebug() << "Command execution result:" << QJsonDocument(exec_result.value()).toJson();
    } else {
        qWarning() << "Command execution failed:" << exec_result.error().message.c_str();
    }

    // Test method invocation
    QVariantList args;
    args << "test_input";

    auto method_result = bridge->invoke_method("process_data", args);
    if (method_result) {
        qDebug() << "Method invocation result:" << method_result.value();
    } else {
        qWarning() << "Method invocation failed:" << method_result.error().message.c_str();
    }

    // Test property access
    auto prop_result = bridge->get_property("counter");
    if (prop_result) {
        qDebug() << "Counter property value:" << prop_result.value();
    } else {
        qWarning() << "Property access failed:" << prop_result.error().message.c_str();
    }

    // Test property setting
    auto set_result = bridge->set_property("counter", QVariant(42));
    if (set_result) {
        qDebug() << "Property set successfully";

        // Verify the change
        auto verify_result = bridge->get_property("counter");
        if (verify_result) {
            qDebug() << "New counter value:" << verify_result.value();
        }
    } else {
        qWarning() << "Property setting failed:" << set_result.error().message.c_str();
    }

    // Test code execution
    QString test_code = "plugin.get_info()";
    auto code_result = bridge->execute_code(test_code, QJsonObject());
    if (code_result) {
        qDebug() << "Code execution result:" << code_result.value();
    } else {
        qWarning() << "Code execution failed:" << code_result.error().message.c_str();
    }

    // Test event system
    qDebug() << "\n=== Testing Event System ===";

    // Subscribe to events
    std::vector<QString> event_names = {"test_event", "custom_event"};
    bool event_received = false;
    QString received_event_name;
    QJsonObject received_event_data;

    auto event_callback = [&](const QString& event_name, const QJsonObject& event_data) {
        qDebug() << "Event received:" << event_name << "with data:" << QJsonDocument(event_data).toJson();
        event_received = true;
        received_event_name = event_name;
        received_event_data = event_data;
    };

    auto subscribe_result = bridge->subscribe_to_events("", event_names, event_callback);
    if (subscribe_result) {
        qDebug() << "Successfully subscribed to events";
    } else {
        qWarning() << "Event subscription failed:" << subscribe_result.error().message.c_str();
    }

    // Emit an event
    QJsonObject event_data;
    event_data["test_message"] = "Hello from C++";
    event_data["timestamp"] = QDateTime::currentDateTime().toString();

    auto emit_result = bridge->emit_event("test_event", event_data);
    if (emit_result) {
        qDebug() << "Event emitted successfully";
    } else {
        qWarning() << "Event emission failed:" << emit_result.error().message.c_str();
    }

    // Test triggering custom event from Python
    auto trigger_result = bridge->execute_command("trigger_custom_event", QJsonObject{{"data", "test"}});
    if (trigger_result) {
        qDebug() << "Custom event triggered:" << QJsonDocument(trigger_result.value()).toJson();
    } else {
        qWarning() << "Custom event trigger failed:" << trigger_result.error().message.c_str();
    }

    // Unsubscribe from events
    auto unsubscribe_result = bridge->unsubscribe_from_events("", event_names);
    if (unsubscribe_result) {
        qDebug() << "Successfully unsubscribed from events";
    } else {
        qWarning() << "Event unsubscription failed:" << unsubscribe_result.error().message.c_str();
    }

    // Test method signature retrieval
    qDebug() << "\n=== Testing Method Signature Retrieval ===";

    auto signature = bridge->get_method_signature("process_data");
    if (signature.has_value()) {
        qDebug() << "Method signature retrieved:" << QJsonDocument(signature.value()).toJson();
    } else {
        qWarning() << "Method signature retrieval failed";
    }

    // Test available methods and properties
    qDebug() << "\n=== Testing Discovery Functions ===";

    auto methods = bridge->get_available_methods();
    qDebug() << "Available methods:" << methods.size();
    for (const auto& method : methods) {
        qDebug() << "  -" << method;
    }

    auto properties = bridge->get_available_properties();
    qDebug() << "Available properties:" << properties.size();
    for (const auto& property : properties) {
        qDebug() << "  -" << property;
    }

    // Test dependency change handling
    qDebug() << "\n=== Testing Dependency Change Handling ===";

    auto dep_result = bridge->handle_dependency_change("test_dependency", qtplugin::PluginState::Running);
    if (dep_result) {
        qDebug() << "Dependency change handled successfully";
    } else {
        qWarning() << "Dependency change handling failed:" << dep_result.error().message.c_str();
    }

    // Test hot reload
    qDebug() << "\n=== Testing Hot Reload ===";

    auto reload_result = bridge->hot_reload();
    if (reload_result) {
        qDebug() << "Hot reload completed successfully";

        // Verify plugin still works after reload
        auto verify_result = bridge->invoke_method("get_info", QVariantList());
        if (verify_result) {
            qDebug() << "Plugin verification after reload: SUCCESS";
        } else {
            qWarning() << "Plugin verification after reload: FAILED";
        }
    } else {
        qWarning() << "Hot reload failed:" << reload_result.error().message.c_str();
    }

    qDebug() << "\n=== All tests completed ===";

    // Shutdown
    bridge->shutdown();
    qDebug() << "Bridge shutdown completed";

    return 0;
}
