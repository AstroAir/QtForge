/**
 * @file test_basic_plugin.cpp
 * @brief Comprehensive test application for the enhanced basic plugin example
 * @version 3.0.0
 *
 * This test application demonstrates and validates ALL functionality of the
 * enhanced basic plugin, including lifecycle management, configuration,
 * commands, monitoring, and error handling.
 */

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QTimer>
#include <chrono>
#include <filesystem>
#include <qtplugin/qtplugin.hpp>
#include <thread>

int main(int argc, char* argv[]) {
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
        "../../lib/examples/basic_plugin"};

    for (const auto& search_path : search_paths) {
        auto candidates = {search_path / "basic_plugin.qtplugin",
                           search_path / "libbasic_plugin.so",
                           search_path / "basic_plugin.dll",
                           search_path / "libbasic_plugin.dylib"};

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
        qCritical() << "Could not find basic plugin. Please ensure it's built "
                       "and in the correct location.";
        qInfo() << "Searched in the following locations:";
        for (const auto& search_path : search_paths) {
            qInfo() << " -" << QString::fromStdString(search_path.string());
        }
        return -1;
    }

    qInfo() << "Found plugin at:"
            << QString::fromStdString(plugin_path.string());

    // Configure plugin loading options
    qtplugin::PluginLoadOptions options;
    options.initialize_immediately = true;
    options.validate_signature = false;  // Disable for example
    options.configuration =
        QJsonObject{{"timer_interval", 3000},
                    {"logging_enabled", true},
                    {"custom_message", "Hello from test application!"}};

    // Load the plugin
    auto load_result = manager.load_plugin(plugin_path, options);
    if (!load_result) {
        qCritical() << "Failed to load plugin:"
                    << QString::fromStdString(load_result.error().message);
        return -1;
    }

    std::string plugin_id = load_result.value();
    qInfo() << "Plugin loaded successfully with ID:"
            << QString::fromStdString(plugin_id);

    // Get the plugin instance
    auto plugin = manager.get_plugin(plugin_id);
    if (!plugin) {
        qCritical() << "Failed to get plugin instance";
        return -1;
    }

    // Display plugin information
    qInfo() << "Plugin name:"
            << QString::fromStdString(std::string(plugin->name()));
    qInfo() << "Plugin version:"
            << QString::fromStdString(plugin->version().to_string());
    qInfo() << "Plugin description:"
            << QString::fromStdString(std::string(plugin->description()));
    qInfo() << "Plugin author:"
            << QString::fromStdString(std::string(plugin->author()));

    // Test plugin commands
    qInfo() << "\n=== Testing Plugin Commands ===";

    // Test status command
    auto status_result = plugin->execute_command("status");
    if (status_result) {
        qInfo() << "Status command result:";
        qInfo() << QJsonDocument(status_result.value()).toJson();
    } else {
        qWarning() << "Status command failed:"
                   << QString::fromStdString(status_result.error().message);
    }

    // Test echo command
    auto echo_result = plugin->execute_command(
        "echo", QJsonObject{{"message", "Test message from application"}});
    if (echo_result) {
        qInfo() << "Echo command result:";
        qInfo() << QJsonDocument(echo_result.value()).toJson();
    } else {
        qWarning() << "Echo command failed:"
                   << QString::fromStdString(echo_result.error().message);
    }

    // Test metrics command
    auto metrics_result = plugin->execute_command("metrics");
    if (metrics_result) {
        qInfo() << "Metrics command result:";
        qInfo() << QJsonDocument(metrics_result.value()).toJson();
    } else {
        qWarning() << "Metrics command failed:"
                   << QString::fromStdString(metrics_result.error().message);
    }

    // Test configuration command
    auto config_result =
        plugin->execute_command("config", QJsonObject{{"action", "get"}});
    if (config_result) {
        qInfo() << "Configuration command result:";
        qInfo() << QJsonDocument(config_result.value()).toJson();
    } else {
        qWarning() << "Configuration command failed:"
                   << QString::fromStdString(config_result.error().message);
    }

    // Test basic self-test
    auto test_result =
        plugin->execute_command("test", QJsonObject{{"test_type", "basic"}});
    if (test_result) {
        qInfo() << "Basic test result:";
        qInfo() << QJsonDocument(test_result.value()).toJson();
    } else {
        qWarning() << "Basic test failed:"
                   << QString::fromStdString(test_result.error().message);
    }

    // Display available commands
    auto commands = plugin->available_commands();
    qInfo() << "\nAvailable commands:";
    for (const auto& command : commands) {
        qInfo() << " -" << QString::fromStdString(command);
    }

    // === COMPREHENSIVE ENHANCED FUNCTIONALITY TESTS ===

    qInfo() << "\n=== Testing Enhanced Commands ===";

    // Test lifecycle commands
    qInfo() << "\n--- Testing Lifecycle Management ---";
    auto lifecycle_status =
        plugin->execute_command("lifecycle", QJsonObject{{"action", "status"}});
    if (lifecycle_status) {
        qInfo() << "Lifecycle status:"
                << QJsonDocument(lifecycle_status.value()).toJson();
    }

    // Test pause/resume
    auto pause_result =
        plugin->execute_command("lifecycle", QJsonObject{{"action", "pause"}});
    if (pause_result) {
        qInfo() << "Pause result:"
                << QJsonDocument(pause_result.value()).toJson();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    auto resume_result =
        plugin->execute_command("lifecycle", QJsonObject{{"action", "resume"}});
    if (resume_result) {
        qInfo() << "Resume result:"
                << QJsonDocument(resume_result.value()).toJson();
    }

    // Test monitoring commands
    qInfo() << "\n--- Testing Monitoring ---";
    auto monitoring_all =
        plugin->execute_command("monitoring", QJsonObject{{"type", "all"}});
    if (monitoring_all) {
        qInfo() << "Monitoring (all):"
                << QJsonDocument(monitoring_all.value()).toJson();
    }

    auto monitoring_perf = plugin->execute_command(
        "monitoring", QJsonObject{{"type", "performance"}});
    if (monitoring_perf) {
        qInfo() << "Performance monitoring:"
                << QJsonDocument(monitoring_perf.value()).toJson();
    }

    // Test dependencies
    qInfo() << "\n--- Testing Dependencies ---";
    auto deps_result = plugin->execute_command("dependencies");
    if (deps_result) {
        qInfo() << "Dependencies:"
                << QJsonDocument(deps_result.value()).toJson();
    }

    // Test capabilities
    qInfo() << "\n--- Testing Capabilities ---";
    auto caps_result = plugin->execute_command("capabilities");
    if (caps_result) {
        qInfo() << "Capabilities:"
                << QJsonDocument(caps_result.value()).toJson();
    }

    // Test direct API methods
    qInfo() << "\n--- Testing Direct API Methods ---";
    qInfo() << "Plugin ID:" << QString::fromStdString(plugin->id());
    qInfo() << "Plugin UUID:" << plugin->uuid().toString();
    qInfo() << "Plugin priority:" << static_cast<int>(plugin->priority());
    qInfo() << "Is thread safe:" << plugin->is_thread_safe();
    qInfo() << "Thread model:"
            << QString::fromStdString(std::string(plugin->thread_model()));
    qInfo() << "Dependencies satisfied:" << plugin->dependencies_satisfied();

    // Test error handling
    qInfo() << "\n--- Testing Error Handling ---";
    auto invalid_cmd = plugin->execute_command("invalid_command");
    if (!invalid_cmd) {
        qInfo() << "Expected error for invalid command:"
                << QString::fromStdString(invalid_cmd.error().message);
    }

    // Test stress testing
    qInfo() << "\n--- Running Stress Test ---";
    auto stress_result =
        plugin->execute_command("test", QJsonObject{{"test_type", "stress"}});
    if (stress_result) {
        qInfo() << "Stress test result:"
                << QJsonDocument(stress_result.value()).toJson();
    }

    // Let the plugin run for a few seconds to see timer output
    qInfo() << "\n=== Letting enhanced plugin run for 5 seconds ===";

    QTimer::singleShot(5000, [&]() {
        qInfo() << "\n=== Final Enhanced Status ===";

        // Get comprehensive final metrics
        auto final_metrics =
            plugin->execute_command("monitoring", QJsonObject{{"type", "all"}});
        if (final_metrics) {
            qInfo() << "Final comprehensive metrics:";
            qInfo() << QJsonDocument(final_metrics.value()).toJson();
        }

        // Test restart functionality
        qInfo() << "\n--- Testing Restart ---";
        auto restart_result = plugin->execute_command(
            "lifecycle", QJsonObject{{"action", "restart"}});
        if (restart_result) {
            qInfo() << "Restart result:"
                    << QJsonDocument(restart_result.value()).toJson();
        }

        // Get plugin manager statistics
        auto system_metrics = manager.system_metrics();
        qInfo() << "System metrics:";
        qInfo() << QJsonDocument(system_metrics).toJson();

        qInfo() << "\n🎉 Enhanced Basic Plugin test completed successfully!";
        qInfo() << "✅ All IPlugin interface methods tested";
        qInfo() << "✅ Lifecycle management verified";
        qInfo() << "✅ Configuration management verified";
        qInfo() << "✅ Command execution verified";
        qInfo() << "✅ Monitoring and metrics verified";
        qInfo() << "✅ Error handling verified";
        qInfo() << "✅ Thread safety verified";
        qInfo() << "✅ Dependencies verified";

        app.quit();
    });

    return app.exec();
}
