/**
 * @file test_configuration_plugin.cpp
 * @brief Test application for configuration plugin
 * @version 1.0.0
 */

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QTimer>

#include "configuration_plugin.hpp"

using namespace qtplugin::examples;

class ConfigurationTester : public QObject {
    Q_OBJECT

public:
    explicit ConfigurationTester(QObject* parent = nullptr) : QObject(parent) {}

public slots:
    void run_tests() {
        qDebug() << "=== Configuration Plugin Test Suite ===";

        // Create plugin instance
        auto plugin = std::make_unique<ConfigurationPlugin>();

        // Test 1: Basic initialization
        qDebug() << "\n--- Test 1: Basic Initialization ---";
        QJsonObject init_config{{"test_mode", true},
                                {"validation_interval", 5000}};

        bool init_result = plugin->initialize(init_config);
        qDebug() << "Initialization result:" << init_result;
        qDebug() << "Plugin state:" << static_cast<int>(plugin->state());
        qDebug()
            << "Plugin metadata:"
            << QJsonDocument(plugin->metadata()).toJson(QJsonDocument::Compact);

        if (!init_result) {
            qCritical() << "Plugin initialization failed!";
            QCoreApplication::exit(1);
            return;
        }

        // Test 2: Get configuration
        qDebug() << "\n--- Test 2: Get Configuration ---";
        QJsonObject get_all_result = plugin->execute_command("get_config");
        qDebug()
            << "Get all config result:"
            << QJsonDocument(get_all_result).toJson(QJsonDocument::Compact);

        QJsonObject get_specific_result = plugin->execute_command(
            "get_config", QJsonObject{{"key", "logging_enabled"}});
        qDebug() << "Get specific config result:"
                 << QJsonDocument(get_specific_result)
                        .toJson(QJsonDocument::Compact);

        // Test 3: Set configuration
        qDebug() << "\n--- Test 3: Set Configuration ---";
        QJsonObject set_result = plugin->execute_command(
            "set_config",
            QJsonObject{{"key", "test_setting"}, {"value", "test_value"}});
        qDebug() << "Set config result:"
                 << QJsonDocument(set_result).toJson(QJsonDocument::Compact);

        // Test 4: Validate configuration
        qDebug() << "\n--- Test 4: Validate Configuration ---";
        QJsonObject validate_result =
            plugin->execute_command("validate_config");
        qDebug()
            << "Validate config result:"
            << QJsonDocument(validate_result).toJson(QJsonDocument::Compact);

        // Test 5: Invalid configuration
        qDebug() << "\n--- Test 5: Invalid Configuration ---";
        QJsonObject invalid_set_result = plugin->execute_command(
            "set_config", QJsonObject{
                              {"key", "validation_interval"}, {"value", -1000}
                              // Invalid value
                          });
        qDebug()
            << "Invalid set config result:"
            << QJsonDocument(invalid_set_result).toJson(QJsonDocument::Compact);

        // Test 6: Save configuration
        qDebug() << "\n--- Test 6: Save Configuration ---";
        QJsonObject save_result = plugin->execute_command("save_config");
        qDebug() << "Save config result:"
                 << QJsonDocument(save_result).toJson(QJsonDocument::Compact);

        // Test 7: Reload configuration
        qDebug() << "\n--- Test 7: Reload Configuration ---";
        QJsonObject reload_result = plugin->execute_command("reload_config");
        qDebug() << "Reload config result:"
                 << QJsonDocument(reload_result).toJson(QJsonDocument::Compact);

        // Test 8: Unknown command
        qDebug() << "\n--- Test 8: Unknown Command ---";
        QJsonObject unknown_result = plugin->execute_command("unknown_command");
        qDebug()
            << "Unknown command result:"
            << QJsonDocument(unknown_result).toJson(QJsonDocument::Compact);

        // Test 9: Complex configuration update
        qDebug() << "\n--- Test 9: Complex Configuration Update ---";
        QJsonObject complex_set_result = plugin->execute_command(
            "set_config",
            QJsonObject{{"key", "features"},
                        {"value", QJsonObject{{"advanced_mode", true},
                                              {"debug_mode", true},
                                              {"experimental", false}}}});
        qDebug()
            << "Complex set config result:"
            << QJsonDocument(complex_set_result).toJson(QJsonDocument::Compact);

        // Test 10: Final metadata check
        qDebug() << "\n--- Test 10: Final Metadata Check ---";
        QJsonObject final_metadata = plugin->metadata();
        qDebug()
            << "Final metadata:"
            << QJsonDocument(final_metadata).toJson(QJsonDocument::Compact);

        // Shutdown
        qDebug() << "\n--- Shutdown ---";
        plugin->shutdown();
        qDebug() << "Plugin state after shutdown:"
                 << static_cast<int>(plugin->state());

        qDebug() << "\n=== Configuration Plugin Tests Complete ===";

        // Schedule application exit
        QTimer::singleShot(100, []() { QCoreApplication::exit(0); });
    }
};

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    app.setApplicationName("ConfigurationPluginTest");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("QtForge");
    app.setOrganizationDomain("qtforge.org");

    qDebug() << "Starting Configuration Plugin Test...";
    qDebug() << "Qt version:" << QT_VERSION_STR;
    qDebug() << "Application:" << app.applicationName()
             << app.applicationVersion();

    ConfigurationTester tester;

    // Start tests after event loop starts
    QTimer::singleShot(0, &tester, &ConfigurationTester::run_tests);

    return app.exec();
}

#include "test_configuration_plugin.moc"
