/**
 * @file test_basic_plugin.cpp
 * @brief Test application for the basic plugin example
 * @version 3.0.0
 */

#include <qtplugin/qtplugin.hpp>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QJsonDocument>
#include <iostream>
#include <filesystem>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Initialize the QtPlugin library
    qtplugin::LibraryInitializer init;
    if (!init.is_initialized()) {
        qCritical() << "Failed to initialize QtPlugin library";
        return -1;
    }
    
    qInfo() << "QtPlugin library initialized successfully";
    qInfo() << "Library version:" << qtplugin::version();
    
    // Create plugin manager
    qtplugin::PluginManager manager;
    
    // Find the basic plugin
    std::filesystem::path plugin_path;
    
    // Try different possible locations for the plugin
    std::vector<std::filesystem::path> search_paths = {
        ".",
        "./examples",
        "../examples",
        "./lib/examples/basic_plugin",
        "../lib/examples/basic_plugin",
        "../../lib/examples/basic_plugin"
    };
    
    for (const auto& search_path : search_paths) {
        auto candidates = {
            search_path / "basic_plugin.qtplugin",
            search_path / "libbasic_plugin.so",
            search_path / "basic_plugin.dll",
            search_path / "libbasic_plugin.dylib"
        };
        
        for (const auto& candidate : candidates) {
            if (std::filesystem::exists(candidate)) {
                plugin_path = candidate;
                break;
            }
        }
        
        if (!plugin_path.empty()) {
            break;
        }
    }
    
    if (plugin_path.empty()) {
        qCritical() << "Could not find basic plugin. Please ensure it's built and in the correct location.";
        qInfo() << "Searched in the following locations:";
        for (const auto& search_path : search_paths) {
            qInfo() << " -" << QString::fromStdString(search_path.string());
        }
        return -1;
    }
    
    qInfo() << "Found plugin at:" << QString::fromStdString(plugin_path.string());
    
    // Configure plugin loading options
    qtplugin::PluginLoadOptions options;
    options.initialize_immediately = true;
    options.validate_signature = false; // Disable for example
    options.configuration = QJsonObject{
        {"timer_interval", 3000},
        {"logging_enabled", true},
        {"custom_message", "Hello from test application!"}
    };
    
    // Load the plugin
    auto load_result = manager.load_plugin(plugin_path, options);
    if (!load_result) {
        qCritical() << "Failed to load plugin:" << QString::fromStdString(load_result.error().message);
        return -1;
    }
    
    std::string plugin_id = load_result.value();
    qInfo() << "Plugin loaded successfully with ID:" << QString::fromStdString(plugin_id);
    
    // Get the plugin instance
    auto plugin = manager.get_plugin(plugin_id);
    if (!plugin) {
        qCritical() << "Failed to get plugin instance";
        return -1;
    }
    
    // Display plugin information
    qInfo() << "Plugin name:" << QString::fromStdString(std::string(plugin->name()));
    qInfo() << "Plugin version:" << QString::fromStdString(plugin->version().to_string());
    qInfo() << "Plugin description:" << QString::fromStdString(std::string(plugin->description()));
    qInfo() << "Plugin author:" << QString::fromStdString(std::string(plugin->author()));
    
    // Test plugin commands
    qInfo() << "\n=== Testing Plugin Commands ===";
    
    // Test status command
    auto status_result = plugin->execute_command("status");
    if (status_result) {
        qInfo() << "Status command result:";
        qInfo() << QJsonDocument(status_result.value()).toJson();
    } else {
        qWarning() << "Status command failed:" << QString::fromStdString(status_result.error().message);
    }
    
    // Test echo command
    auto echo_result = plugin->execute_command("echo", QJsonObject{{"message", "Test message from application"}});
    if (echo_result) {
        qInfo() << "Echo command result:";
        qInfo() << QJsonDocument(echo_result.value()).toJson();
    } else {
        qWarning() << "Echo command failed:" << QString::fromStdString(echo_result.error().message);
    }
    
    // Test metrics command
    auto metrics_result = plugin->execute_command("metrics");
    if (metrics_result) {
        qInfo() << "Metrics command result:";
        qInfo() << QJsonDocument(metrics_result.value()).toJson();
    } else {
        qWarning() << "Metrics command failed:" << QString::fromStdString(metrics_result.error().message);
    }
    
    // Test configuration command
    auto config_result = plugin->execute_command("config", QJsonObject{{"action", "get"}});
    if (config_result) {
        qInfo() << "Configuration command result:";
        qInfo() << QJsonDocument(config_result.value()).toJson();
    } else {
        qWarning() << "Configuration command failed:" << QString::fromStdString(config_result.error().message);
    }
    
    // Test basic self-test
    auto test_result = plugin->execute_command("test", QJsonObject{{"test_type", "basic"}});
    if (test_result) {
        qInfo() << "Basic test result:";
        qInfo() << QJsonDocument(test_result.value()).toJson();
    } else {
        qWarning() << "Basic test failed:" << QString::fromStdString(test_result.error().message);
    }
    
    // Display available commands
    auto commands = plugin->available_commands();
    qInfo() << "\nAvailable commands:";
    for (const auto& command : commands) {
        qInfo() << " -" << QString::fromStdString(command);
    }
    
    // Let the plugin run for a few seconds to see timer output
    qInfo() << "\n=== Letting plugin run for 10 seconds ===";
    
    QTimer::singleShot(10000, [&]() {
        qInfo() << "\n=== Final Status ===";
        
        // Get final metrics
        auto final_metrics = plugin->execute_command("metrics");
        if (final_metrics) {
            qInfo() << "Final metrics:";
            qInfo() << QJsonDocument(final_metrics.value()).toJson();
        }
        
        // Get plugin manager statistics
        auto system_metrics = manager.system_metrics();
        qInfo() << "System metrics:";
        qInfo() << QJsonDocument(system_metrics).toJson();
        
        qInfo() << "Test completed successfully!";
        app.quit();
    });
    
    return app.exec();
}
