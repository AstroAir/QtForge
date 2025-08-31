/**
 * @file test_task_processing.cpp
 * @brief Test enhanced task processing functionality
 * @version 3.0.0
 */

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QTimer>
#include <filesystem>
#include <qtplugin/qtplugin.hpp>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    qInfo() << "🚀 SERVICE PLUGIN TASK PROCESSING TEST";

    // Initialize plugin manager
    qtplugin::PluginManager manager;

    // Set plugin directory
    std::filesystem::path plugin_dir = std::filesystem::current_path();
    manager.add_search_path(plugin_dir);

    // Load and initialize service plugin
    auto load_result = manager.load_plugin("service_plugin.qtplugin");
    if (!load_result) {
        qCritical() << "Failed to load service plugin:"
                    << QString::fromStdString(load_result.error().message);
        return 1;
    }

    auto plugin = manager.get_plugin("com.example.service_plugin");
    if (!plugin) {
        qCritical() << "Failed to get service plugin instance";
        return 1;
    }

    // Check if plugin is already initialized
    if (!plugin->is_initialized()) {
        auto init_result = plugin->initialize();
        if (!init_result) {
            qCritical() << "Failed to initialize service plugin:"
                        << QString::fromStdString(init_result.error().message);
            return 1;
        }
        qInfo() << "✅ Service plugin initialized";
    } else {
        qInfo() << "✅ Service plugin already initialized";
    }

    // Test different types of tasks
    qInfo() << "\n=== Testing Enhanced Task Processing ===";

    // Test 1: Data processing task
    QJsonObject data_task{{"id", "data_task_001"},
                          {"type", "data_processing"},
                          {"processing_time", 500},
                          {"data", QJsonObject{{"input_file", "data.csv"},
                                               {"rows", 1000},
                                               {"columns", 50}}}};

    auto data_result = plugin->execute_command(
        "task", QJsonObject{{"action", "submit"}, {"task", data_task}});

    if (data_result) {
        qInfo() << "✅ Data processing task submitted:";
        qInfo() << QJsonDocument(data_result.value())
                       .toJson(QJsonDocument::Compact);
    }

    // Test 2: Calculation task
    QJsonObject calc_task{{"id", "calc_task_001"},
                          {"type", "calculation"},
                          {"processing_time", 200},
                          {"data", QJsonObject{{"input", 42}}}};

    auto calc_result = plugin->execute_command(
        "task", QJsonObject{{"action", "submit"}, {"task", calc_task}});

    if (calc_result) {
        qInfo() << "✅ Calculation task submitted:";
        qInfo() << QJsonDocument(calc_result.value())
                       .toJson(QJsonDocument::Compact);
    }

    // Test 3: Default task
    QJsonObject default_task{
        {"id", "default_task_001"},
        {"type", "default"},
        {"processing_time", 300},
        {"data", QJsonObject{{"message", "Hello from default task"}}}};

    auto default_result = plugin->execute_command(
        "task", QJsonObject{{"action", "submit"}, {"task", default_task}});

    if (default_result) {
        qInfo() << "✅ Default task submitted:";
        qInfo() << QJsonDocument(default_result.value())
                       .toJson(QJsonDocument::Compact);
    }

    // Wait for tasks to complete
    qInfo() << "\n⏳ Waiting for tasks to complete...";

    QTimer::singleShot(2000, [&]() {
        // Check task statistics
        auto stats_result =
            plugin->execute_command("task", QJsonObject{{"action", "stats"}});
        if (stats_result) {
            qInfo() << "\n📊 Task Statistics:";
            qInfo() << QJsonDocument(stats_result.value())
                           .toJson(QJsonDocument::Compact);
        }

        // Check monitoring data
        auto monitoring_result =
            plugin->execute_command("monitoring", QJsonObject{{"type", "all"}});
        if (monitoring_result) {
            qInfo() << "\n📈 Monitoring Data:";
            qInfo() << QJsonDocument(monitoring_result.value())
                           .toJson(QJsonDocument::Compact);
        }

        qInfo() << "\n🎉 Task processing test completed!";
        app.quit();
    });

    return app.exec();
}
