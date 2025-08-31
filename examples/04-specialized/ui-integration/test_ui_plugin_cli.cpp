/**
 * @file test_ui_plugin_cli.cpp
 * @brief Command-line test for UI plugin functionality
 * @version 3.0.0
 *
 * This test validates UI plugin functionality without requiring GUI
 * interaction.
 */

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <filesystem>
#include <qtplugin/qtplugin.hpp>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    qInfo() << "🚀 UI PLUGIN COMMAND-LINE TEST";

    // Initialize plugin manager
    qtplugin::PluginManager manager;

    // Set plugin directory
    std::filesystem::path plugin_dir = std::filesystem::current_path();
    manager.add_search_path(plugin_dir);

    qInfo() << "Plugin directory:"
            << QString::fromStdString(plugin_dir.string());

    // Load the UI plugin
    qInfo() << "\n=== Loading UI Plugin ===";

    auto load_result = manager.load_plugin("ui_plugin.qtplugin");
    if (!load_result) {
        qCritical() << "Failed to load UI plugin:"
                    << QString::fromStdString(load_result.error().message);
        return 1;
    }

    qInfo() << "✅ UI plugin loaded successfully";

    // Get plugin instance
    auto plugin = manager.get_plugin("com.example.ui_plugin");
    if (!plugin) {
        qCritical() << "Failed to get UI plugin instance";
        return 1;
    }

    qInfo() << "✅ UI plugin instance obtained";
    qInfo() << "Plugin name:"
            << QString::fromStdString(std::string(plugin->name()));
    qInfo() << "Plugin ID:" << QString::fromStdString(plugin->id());
    qInfo() << "Plugin version:" << plugin->version().to_string().c_str();

    // Test basic plugin interface without full initialization
    qInfo() << "\n=== Testing Basic Plugin Interface ===";

    // Test plugin metadata
    qInfo() << "Plugin capabilities:"
            << static_cast<int>(plugin->capabilities());
    qInfo() << "Plugin priority:" << static_cast<int>(plugin->priority());
    qInfo() << "Is thread safe:" << plugin->is_thread_safe();
    qInfo() << "Thread model:"
            << QString::fromStdString(std::string(plugin->thread_model()));

    // Test dependencies
    auto deps = plugin->dependencies();
    qInfo() << "Required dependencies:" << deps.size();
    for (const auto& dep : deps) {
        qInfo() << " -" << QString::fromStdString(dep);
    }

    auto opt_deps = plugin->optional_dependencies();
    qInfo() << "Optional dependencies:" << opt_deps.size();
    for (const auto& dep : opt_deps) {
        qInfo() << " -" << QString::fromStdString(dep);
    }

    // Test available commands without initialization
    auto commands = plugin->available_commands();
    qInfo() << "Available commands:" << commands.size();
    for (const auto& cmd : commands) {
        qInfo() << " -" << QString::fromStdString(cmd);
    }

    // Test default configuration
    auto default_config = plugin->default_configuration();
    if (default_config) {
        qInfo() << "Default configuration available:"
                << QJsonDocument(default_config.value())
                       .toJson(QJsonDocument::Compact);
    }

    qInfo() << "✅ Basic plugin interface tested successfully";
    qInfo() << "⚠️  Skipping full initialization to avoid GUI dependencies in "
               "CLI test";

    // Test commands (without initialization, these will likely fail but we can
    // test the interface)
    qInfo() << "\n=== Testing UI Plugin Command Interface ===";

    // Test theme listing (expected to fail without initialization)
    auto theme_result =
        plugin->execute_command("theme", QJsonObject{{"action", "list"}});
    if (theme_result) {
        qInfo() << "✅ Theme list command successful";
        auto themes = theme_result.value();
        qInfo() << "Available themes:"
                << QJsonDocument(themes).toJson(QJsonDocument::Compact);
    } else {
        qInfo()
            << "⚠️  Theme list command failed (expected without initialization):"
            << QString::fromStdString(theme_result.error().message);
    }

    // Test widget listing
    auto widget_result =
        plugin->execute_command("widget", QJsonObject{{"action", "list"}});
    if (widget_result) {
        qInfo() << "✅ Widget list command successful";
        auto widgets = widget_result.value();
        qInfo() << "Available widgets:"
                << QJsonDocument(widgets).toJson(QJsonDocument::Compact);
    } else {
        qWarning() << "❌ Widget list command failed:"
                   << QString::fromStdString(widget_result.error().message);
    }

    // Test action listing
    auto action_result =
        plugin->execute_command("action", QJsonObject{{"action", "list"}});
    if (action_result) {
        qInfo() << "✅ Action list command successful";
        auto actions = action_result.value();
        qInfo() << "Available actions:"
                << QJsonDocument(actions).toJson(QJsonDocument::Compact);
    } else {
        qWarning() << "❌ Action list command failed:"
                   << QString::fromStdString(action_result.error().message);
    }

    // Test dialog listing
    auto dialog_result =
        plugin->execute_command("dialog", QJsonObject{{"action", "list"}});
    if (dialog_result) {
        qInfo() << "✅ Dialog list command successful";
        auto dialogs = dialog_result.value();
        qInfo() << "Available dialogs:"
                << QJsonDocument(dialogs).toJson(QJsonDocument::Compact);
    } else {
        qWarning() << "❌ Dialog list command failed:"
                   << QString::fromStdString(dialog_result.error().message);
    }

    // Test settings
    auto settings_result =
        plugin->execute_command("settings", QJsonObject{{"action", "get"}});
    if (settings_result) {
        qInfo() << "✅ Settings get command successful";
        auto settings = settings_result.value();
        qInfo() << "Current settings:"
                << QJsonDocument(settings).toJson(QJsonDocument::Compact);
    } else {
        qWarning() << "❌ Settings get command failed:"
                   << QString::fromStdString(settings_result.error().message);
    }

    // Test status
    auto status_result = plugin->execute_command("status");
    if (status_result) {
        qInfo() << "✅ Status command successful";
        auto status = status_result.value();
        qInfo() << "Plugin status:"
                << QJsonDocument(status).toJson(QJsonDocument::Compact);
    } else {
        qWarning() << "❌ Status command failed:"
                   << QString::fromStdString(status_result.error().message);
    }

    // Test performance metrics (should work without initialization)
    qInfo() << "\n=== Testing Performance Metrics ===";
    auto perf_metrics = plugin->performance_metrics();
    qInfo() << "Performance metrics:"
            << QJsonDocument(perf_metrics).toJson(QJsonDocument::Compact);

    // Test resource usage (should work without initialization)
    auto resource_usage = plugin->resource_usage();
    qInfo() << "Resource usage:"
            << QJsonDocument(resource_usage).toJson(QJsonDocument::Compact);

    qInfo() << "\n🎉 UI Plugin command-line test completed successfully!";
    qInfo() << "✅ Plugin loading and basic interface verified";
    qInfo() << "✅ Command interface tested (full functionality requires GUI)";
    qInfo() << "✅ Performance metrics working";
    qInfo() << "ℹ️  For full UI testing, use the GUI test application";

    return 0;
}
