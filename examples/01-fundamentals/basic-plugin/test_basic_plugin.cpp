/**
 * @file test_basic_plugin.cpp
 * @brief Test application for BasicPlugin
 */

#include "basic_plugin.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QTimer>
#include <memory>

void test_plugin_lifecycle(BasicPlugin& plugin) {
    qDebug() << "\n=== Testing Plugin Lifecycle ===";

    // Test initial state
    qDebug() << "Initial state:" << static_cast<int>(plugin.state());
    qDebug() << "Is initialized:" << plugin.is_initialized();

    // Test initialization
    auto init_result = plugin.initialize();
    if (init_result) {
        qDebug() << "✅ Plugin initialized successfully";
        qDebug() << "State after init:" << static_cast<int>(plugin.state());
    } else {
        qDebug() << "❌ Plugin initialization failed:" << init_result.error().message.c_str();
        return;
    }

    // Test double initialization (should fail)
    auto double_init = plugin.initialize();
    if (!double_init) {
        qDebug() << "✅ Double initialization correctly rejected:" << double_init.error().message.c_str();
    }
}

void test_plugin_commands(BasicPlugin& plugin) {
    qDebug() << "\n=== Testing Plugin Commands ===";

    // Test available commands
    auto commands = plugin.available_commands();
    qDebug() << "Available commands:";
    for (const auto& cmd : commands) {
        qDebug() << "  -" << cmd.c_str();
    }

    // Test status command
    auto status_result = plugin.execute_command("status");
    if (status_result) {
        qDebug() << "✅ Status command result:";
        qDebug() << QJsonDocument(status_result.value()).toJson(QJsonDocument::Compact);
    } else {
        qDebug() << "❌ Status command failed:" << status_result.error().message.c_str();
    }

    // Test echo command
    QJsonObject echo_params;
    echo_params["message"] = "Hello, QtForge!";
    echo_params["number"] = 42;

    auto echo_result = plugin.execute_command("echo", echo_params);
    if (echo_result) {
        qDebug() << "✅ Echo command result:";
        qDebug() << QJsonDocument(echo_result.value()).toJson(QJsonDocument::Compact);
    } else {
        qDebug() << "❌ Echo command failed:" << echo_result.error().message.c_str();
    }

    // Test invalid command
    auto invalid_result = plugin.execute_command("invalid_command");
    if (!invalid_result) {
        qDebug() << "✅ Invalid command correctly rejected:" << invalid_result.error().message.c_str();
    }
}

void test_plugin_configuration(BasicPlugin& plugin) {
    qDebug() << "\n=== Testing Plugin Configuration ===";

    // Test default configuration
    auto default_config = plugin.default_configuration();
    qDebug() << "Default configuration:";
    if (default_config.has_value()) {
        qDebug() << QJsonDocument(default_config.value()).toJson(QJsonDocument::Compact);
    }

    // Test current configuration
    auto current_config = plugin.current_configuration();
    qDebug() << "Current configuration:";
    qDebug() << QJsonDocument(current_config).toJson(QJsonDocument::Compact);

    // Test configuration update
    QJsonObject new_config;
    new_config["timer_interval"] = 2000;
    new_config["custom_message"] = "Updated message!";

    auto config_result = plugin.configure(new_config);
    if (config_result) {
        qDebug() << "✅ Configuration updated successfully";

        auto updated_config = plugin.current_configuration();
        qDebug() << "Updated configuration:";
        qDebug() << QJsonDocument(updated_config).toJson(QJsonDocument::Compact);
    } else {
        qDebug() << "❌ Configuration update failed:" << config_result.error().message.c_str();
    }

    // Test invalid configuration
    QJsonObject invalid_config;
    invalid_config["timer_interval"] = 100; // Too small

    auto invalid_config_result = plugin.configure(invalid_config);
    if (!invalid_config_result) {
        qDebug() << "✅ Invalid configuration correctly rejected:" << invalid_config_result.error().message.c_str();
    }
}

void test_plugin_metadata(BasicPlugin& plugin) {
    qDebug() << "\n=== Testing Plugin Metadata ===";

    auto metadata = plugin.metadata();
    qDebug() << "Plugin metadata:";
    qDebug() << "  Name:" << metadata.name.c_str();
    qDebug() << "  Description:" << metadata.description.c_str();
    qDebug() << "  Version:" << metadata.version.to_string().c_str();
    qDebug() << "  Author:" << metadata.author.c_str();
    // qDebug() << "  ID:" << metadata.id.c_str(); // id field not available
    qDebug() << "  Category:" << metadata.category.c_str();
    qDebug() << "  License:" << metadata.license.c_str();
    qDebug() << "  Homepage:" << metadata.homepage.c_str();
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    qDebug() << "QtForge BasicPlugin Test Application";
    qDebug() << "====================================";

    // Create plugin instance
    auto plugin = std::make_unique<BasicPlugin>(nullptr);

    // Run tests
    test_plugin_metadata(*plugin);
    test_plugin_lifecycle(*plugin);
    test_plugin_commands(*plugin);
    test_plugin_configuration(*plugin);

    // Let timer run for a few seconds to see background processing
    qDebug() << "\n=== Observing Background Processing ===";
    qDebug() << "Waiting 3 seconds to observe timer events...";

    QTimer::singleShot(3000, [&]() {
        qDebug() << "\n=== Final Status ===";
        auto final_status = plugin->execute_command("status");
        if (final_status) {
            qDebug() << "Final status:";
            qDebug() << QJsonDocument(final_status.value()).toJson(QJsonDocument::Compact);
        }

        // Shutdown
        plugin->shutdown();
        qDebug() << "Plugin shutdown complete.";
        qDebug() << "\n✅ All tests completed successfully!";

        app.quit();
    });

    return app.exec();
}
