/**
 * @file test_message_types_refactor.cpp
 * @brief Test to verify that refactored message types produce identical JSON output
 */

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>

#include "include/qtplugin/communication/message_types.hpp"
#include "include/qtplugin/utils/error_handling.hpp"

using namespace qtplugin::messages;

void test_plugin_lifecycle_message() {
    std::cout << "Testing PluginLifecycleMessage..." << std::endl;
    
    PluginLifecycleMessage msg("test_sender", "test_plugin", PluginLifecycleMessage::Event::Started);
    QJsonObject json = msg.to_json();
    
    // Verify required fields are present
    assert(json.contains("type"));
    assert(json.contains("sender"));
    assert(json.contains("plugin_id"));
    assert(json.contains("event"));
    assert(json.contains("timestamp"));
    
    // Verify values
    assert(json["type"].toString() == "plugin_lifecycle");
    assert(json["sender"].toString() == "test_sender");
    assert(json["plugin_id"].toString() == "test_plugin");
    assert(json["event"].toString() == "started");
    
    std::cout << "✓ PluginLifecycleMessage test passed" << std::endl;
}

void test_configuration_changed_message() {
    std::cout << "Testing ConfigurationChangedMessage..." << std::endl;
    
    QJsonObject old_config{{"key1", "value1"}};
    QJsonObject new_config{{"key1", "value2"}};
    
    ConfigurationChangedMessage msg("test_sender", "test_plugin", old_config, new_config);
    QJsonObject json = msg.to_json();
    
    // Verify required fields
    assert(json.contains("type"));
    assert(json.contains("sender"));
    assert(json.contains("plugin_id"));
    assert(json.contains("old_config"));
    assert(json.contains("new_config"));
    assert(json.contains("timestamp"));
    
    // Verify values
    assert(json["type"].toString() == "configuration_changed");
    assert(json["sender"].toString() == "test_sender");
    assert(json["plugin_id"].toString() == "test_plugin");
    
    std::cout << "✓ ConfigurationChangedMessage test passed" << std::endl;
}

void test_system_status_message() {
    std::cout << "Testing SystemStatusMessage..." << std::endl;
    
    SystemStatusMessage msg("test_sender", SystemStatusMessage::Status::Running, "All systems operational");
    QJsonObject json = msg.to_json();
    
    // Verify required fields
    assert(json.contains("type"));
    assert(json.contains("sender"));
    assert(json.contains("status"));
    assert(json.contains("details"));
    assert(json.contains("timestamp"));
    
    // Verify values
    assert(json["type"].toString() == "system_status");
    assert(json["sender"].toString() == "test_sender");
    assert(json["status"].toString() == "running");
    assert(json["details"].toString() == "All systems operational");
    
    std::cout << "✓ SystemStatusMessage test passed" << std::endl;
}

void test_log_message() {
    std::cout << "Testing LogMessage..." << std::endl;
    
    LogMessage msg("test_sender", LogMessage::Level::Info, "Test log message", "test_category");
    QJsonObject json = msg.to_json();
    
    // Verify required fields
    assert(json.contains("type"));
    assert(json.contains("sender"));
    assert(json.contains("level"));
    assert(json.contains("message"));
    assert(json.contains("category"));
    assert(json.contains("timestamp"));
    
    // Verify values
    assert(json["type"].toString() == "log");
    assert(json["sender"].toString() == "test_sender");
    assert(json["level"].toString() == "info");
    assert(json["message"].toString() == "Test log message");
    assert(json["category"].toString() == "test_category");
    
    std::cout << "✓ LogMessage test passed" << std::endl;
}

void test_optional_fields() {
    std::cout << "Testing optional fields..." << std::endl;
    
    // Test LogMessage without category
    LogMessage msg_no_category("test_sender", LogMessage::Level::Debug, "Debug message");
    QJsonObject json = msg_no_category.to_json();
    
    // Category should not be present when empty
    assert(!json.contains("category"));
    
    // Test SystemStatusMessage without details
    SystemStatusMessage status_msg("test_sender", SystemStatusMessage::Status::Stopped);
    QJsonObject status_json = status_msg.to_json();
    
    // Details should not be present when empty
    assert(!status_json.contains("details"));
    
    std::cout << "✓ Optional fields test passed" << std::endl;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    std::cout << "Running message types refactor verification tests..." << std::endl;
    
    try {
        test_plugin_lifecycle_message();
        test_configuration_changed_message();
        test_system_status_message();
        test_log_message();
        test_optional_fields();
        
        std::cout << "\n✅ All tests passed! Refactoring preserved functionality." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
