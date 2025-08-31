/**
 * @file test_monitoring_plugin.cpp
 * @brief Test application for MonitoringPlugin
 * @version 3.0.0
 */

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QTemporaryFile>
#include <iostream>
#include <memory>

#include "monitoring_plugin.hpp"
#include "qtplugin/core/plugin_manager.hpp"
#include "qtplugin/monitoring/plugin_hot_reload_manager.hpp"

class MonitoringPluginTester : public QObject {
    Q_OBJECT

public:
    explicit MonitoringPluginTester(QObject* parent = nullptr) : QObject(parent) {}

    int run_tests(const QString& test_type) {
        // Create plugin instance
        m_plugin = std::make_unique<MonitoringPlugin>(this);

        qInfo() << "=== MonitoringPlugin Test Suite ===";
        qInfo() << "Test Type:" << test_type;
        qInfo() << "";

        bool success = false;

        if (test_type == "basic") {
            success = test_basic_functionality();
        } else if (test_type == "hot_reload") {
            success = test_hot_reload_functionality();
        } else if (test_type == "metrics") {
            success = test_metrics_functionality();
        } else if (test_type == "alerts") {
            success = test_alerts_functionality();
        } else if (test_type == "all") {
            success = test_basic_functionality() &&
                     test_hot_reload_functionality() &&
                     test_metrics_functionality() &&
                     test_alerts_functionality() &&
                     test_dashboard_functionality();
        } else {
            qCritical() << "Unknown test type:" << test_type;
            qInfo() << "Available test types: basic, hot_reload, metrics, alerts, all";
            return 1;
        }

        qInfo() << "";
        qInfo() << "=== Test Results ===";
        qInfo() << "Overall Result:" << (success ? "PASSED" : "FAILED");

        return success ? 0 : 1;
    }

private:
    bool test_basic_functionality() {
        qInfo() << "--- Testing Basic Functionality ---";

        // Test plugin initialization
        auto init_result = m_plugin->initialize();
        if (!init_result) {
            qCritical() << "Plugin initialization failed:"
                       << QString::fromStdString(init_result.error().message);
            return false;
        }
        qInfo() << "✓ Plugin initialization successful";

        // Test metadata
        auto metadata = m_plugin->metadata();
        if (metadata.name != "MonitoringPlugin") {
            qCritical() << "Invalid plugin name:" << QString::fromStdString(metadata.name);
            return false;
        }
        qInfo() << "✓ Plugin metadata correct";

        // Test capabilities
        auto capabilities = m_plugin->capabilities();
        if (!(capabilities & qtplugin::PluginCapability::Monitoring)) {
            qCritical() << "Monitoring capability not present";
            return false;
        }
        qInfo() << "✓ Monitoring capability present";

        // Test configuration
        auto default_config = m_plugin->default_configuration();
        if (!default_config.has_value()) {
            qCritical() << "No default configuration available";
            return false;
        }
        qInfo() << "✓ Default configuration available";

        auto config_result = m_plugin->configure(default_config.value());
        if (!config_result) {
            qCritical() << "Configuration failed:"
                       << QString::fromStdString(config_result.error().message);
            return false;
        }
        qInfo() << "✓ Configuration successful";

        // Test status command
        auto status_result = m_plugin->execute_command("status", QJsonObject{});
        if (!status_result) {
            qCritical() << "Status command failed:"
                       << QString::fromStdString(status_result.error().message);
            return false;
        }
        qInfo() << "✓ Status command successful";

        qInfo() << "Basic functionality tests: PASSED";
        return true;
    }

    bool test_hot_reload_functionality() {
        qInfo() << "--- Testing Hot Reload Functionality ---";

        // Create a temporary file to monitor
        QTemporaryFile temp_file;
        if (!temp_file.open()) {
            qCritical() << "Failed to create temporary file";
            return false;
        }
        QString temp_path = temp_file.fileName();
        temp_file.close();

        // Test enable hot reload
        QJsonObject enable_params{
            {"action", "enable"},
            {"plugin_id", "test_plugin"},
            {"file_path", temp_path}
        };

        auto enable_result = m_plugin->execute_command("hot_reload", enable_params);
        if (!enable_result) {
            qCritical() << "Hot reload enable failed:"
                       << QString::fromStdString(enable_result.error().message);
            return false;
        }

        QJsonObject enable_response = enable_result.value();
        if (!enable_response["success"].toBool()) {
            qCritical() << "Hot reload enable unsuccessful:"
                       << enable_response["error"].toString();
            return false;
        }
        qInfo() << "✓ Hot reload enable successful";

        // Test hot reload status
        QJsonObject status_params{{"action", "status"}};
        auto status_result = m_plugin->execute_command("hot_reload", status_params);
        if (!status_result) {
            qCritical() << "Hot reload status failed:"
                       << QString::fromStdString(status_result.error().message);
            return false;
        }

        QJsonObject status_response = status_result.value();
        if (!status_response["success"].toBool()) {
            qCritical() << "Hot reload status unsuccessful";
            return false;
        }
        qInfo() << "✓ Hot reload status successful";

        // Test disable hot reload
        QJsonObject disable_params{
            {"action", "disable"},
            {"plugin_id", "test_plugin"}
        };

        auto disable_result = m_plugin->execute_command("hot_reload", disable_params);
        if (!disable_result) {
            qCritical() << "Hot reload disable failed:"
                       << QString::fromStdString(disable_result.error().message);
            return false;
        }

        QJsonObject disable_response = disable_result.value();
        if (!disable_response["success"].toBool()) {
            qCritical() << "Hot reload disable unsuccessful:"
                       << disable_response["error"].toString();
            return false;
        }
        qInfo() << "✓ Hot reload disable successful";

        qInfo() << "Hot reload functionality tests: PASSED";
        return true;
    }

    bool test_metrics_functionality() {
        qInfo() << "--- Testing Metrics Functionality ---";

        // Test metrics collection for all plugins
        auto all_metrics_result = m_plugin->execute_command("metrics", QJsonObject{});
        if (!all_metrics_result) {
            qCritical() << "All metrics collection failed:"
                       << QString::fromStdString(all_metrics_result.error().message);
            return false;
        }

        QJsonObject all_metrics_response = all_metrics_result.value();
        if (!all_metrics_response["success"].toBool()) {
            qCritical() << "All metrics collection unsuccessful";
            return false;
        }
        qInfo() << "✓ All metrics collection successful";

        // Test metrics collection for specific plugin
        QJsonObject specific_params{{"plugin_id", "test_plugin"}};
        auto specific_result = m_plugin->execute_command("metrics", specific_params);
        if (!specific_result) {
            qCritical() << "Specific metrics collection failed:"
                       << QString::fromStdString(specific_result.error().message);
            return false;
        }

        QJsonObject specific_response = specific_result.value();
        if (!specific_response["success"].toBool()) {
            qCritical() << "Specific metrics collection unsuccessful";
            return false;
        }
        qInfo() << "✓ Specific metrics collection successful";

        // Test historical metrics
        QJsonObject history_params{
            {"time_range", QJsonObject{
                {"start", QDateTime::currentDateTime().addSecs(-3600).toString(Qt::ISODate)},
                {"end", QDateTime::currentDateTime().toString(Qt::ISODate)}
            }}
        };

        auto history_result = m_plugin->execute_command("history", history_params);
        if (!history_result) {
            qCritical() << "Historical metrics failed:"
                       << QString::fromStdString(history_result.error().message);
            return false;
        }

        QJsonObject history_response = history_result.value();
        if (!history_response["success"].toBool()) {
            qCritical() << "Historical metrics unsuccessful";
            return false;
        }
        qInfo() << "✓ Historical metrics successful";

        qInfo() << "Metrics functionality tests: PASSED";
        return true;
    }

    bool test_alerts_functionality() {
        qInfo() << "--- Testing Alerts Functionality ---";

        // Test alert setup
        QJsonObject alert_config{
            {"cpu_usage_max", QJsonObject{
                {"metric", "cpu_usage"},
                {"operator", "greater_than"},
                {"threshold", 80.0},
                {"severity", "warning"}
            }},
            {"memory_usage_max", QJsonObject{
                {"metric", "memory_usage"},
                {"operator", "greater_than"},
                {"threshold", 1024.0},
                {"severity", "critical"}
            }}
        };

        QJsonObject setup_params{
            {"action", "setup"},
            {"config", alert_config}
        };

        auto setup_result = m_plugin->execute_command("alerts", setup_params);
        if (!setup_result) {
            qCritical() << "Alert setup failed:"
                       << QString::fromStdString(setup_result.error().message);
            return false;
        }

        QJsonObject setup_response = setup_result.value();
        if (!setup_response["success"].toBool()) {
            qCritical() << "Alert setup unsuccessful:"
                       << setup_response["error"].toString();
            return false;
        }
        qInfo() << "✓ Alert setup successful";

        // Test get alerts
        QJsonObject get_params{{"action", "get"}};
        auto get_result = m_plugin->execute_command("alerts", get_params);
        if (!get_result) {
            qCritical() << "Get alerts failed:"
                       << QString::fromStdString(get_result.error().message);
            return false;
        }

        QJsonObject get_response = get_result.value();
        if (!get_response["success"].toBool()) {
            qCritical() << "Get alerts unsuccessful";
            return false;
        }
        qInfo() << "✓ Get alerts successful";

        // Test clear alerts
        QJsonObject clear_params{{"action", "clear"}};
        auto clear_result = m_plugin->execute_command("alerts", clear_params);
        if (!clear_result) {
            qCritical() << "Clear alerts failed:"
                       << QString::fromStdString(clear_result.error().message);
            return false;
        }

        QJsonObject clear_response = clear_result.value();
        if (!clear_response["success"].toBool()) {
            qCritical() << "Clear alerts unsuccessful";
            return false;
        }
        qInfo() << "✓ Clear alerts successful";

        qInfo() << "Alerts functionality tests: PASSED";
        return true;
    }

    bool test_dashboard_functionality() {
        qInfo() << "--- Testing Dashboard Functionality ---";

        // Test dashboard command
        auto dashboard_result = m_plugin->execute_command("dashboard", QJsonObject{});
        if (!dashboard_result) {
            qCritical() << "Dashboard command failed:"
                       << QString::fromStdString(dashboard_result.error().message);
            return false;
        }

        QJsonObject dashboard_response = dashboard_result.value();
        if (!dashboard_response["success"].toBool()) {
            qCritical() << "Dashboard command unsuccessful";
            return false;
        }

        // Verify dashboard contains expected sections
        QJsonObject dashboard = dashboard_response["dashboard"].toObject();
        if (!dashboard.contains("system_overview") ||
            !dashboard.contains("plugins") ||
            !dashboard.contains("performance")) {
            qCritical() << "Dashboard missing required sections";
            return false;
        }

        qInfo() << "✓ Dashboard command successful";
        qInfo() << "Dashboard functionality tests: PASSED";
        return true;
    }

private:
    std::unique_ptr<MonitoringPlugin> m_plugin;
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QString test_type = "basic";
    if (argc > 1) {
        test_type = QString::fromLocal8Bit(argv[1]);
    }

    MonitoringPluginTester tester;
    int result = tester.run_tests(test_type);

    return result;
}

#include "test_monitoring_plugin.moc"
