/**
 * @file test_security_plugin.cpp
 * @brief Test application for SecurityPlugin
 * @version 3.0.0
 */

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <iostream>
#include <memory>

#include "security_plugin.hpp"
#include "qtplugin/core/plugin_manager.hpp"
#include "qtplugin/security/security_manager.hpp"

class SecurityPluginTester : public QObject {
    Q_OBJECT

public:
    explicit SecurityPluginTester(QObject* parent = nullptr) : QObject(parent) {}

    int run_tests(const QString& test_type) {
        // Create plugin instance
        m_plugin = std::make_unique<SecurityPlugin>();
        
        qInfo() << "=== SecurityPlugin Test Suite ===";
        qInfo() << "Test Type:" << test_type;
        qInfo() << "";

        bool success = false;
        
        if (test_type == "basic") {
            success = test_basic_functionality();
        } else if (test_type == "validation") {
            success = test_validation_functionality();
        } else if (test_type == "permission") {
            success = test_permission_functionality();
        } else if (test_type == "all") {
            success = test_basic_functionality() &&
                     test_validation_functionality() &&
                     test_permission_functionality() &&
                     test_audit_functionality() &&
                     test_policy_functionality();
        } else {
            qCritical() << "Unknown test type:" << test_type;
            qInfo() << "Available test types: basic, validation, permission, all";
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
        if (metadata.name != "SecurityPlugin") {
            qCritical() << "Invalid plugin name:" << QString::fromStdString(metadata.name);
            return false;
        }
        qInfo() << "✓ Plugin metadata correct";

        // Test capabilities
        auto capabilities = m_plugin->capabilities();
        if (!(capabilities & qtplugin::PluginCapability::Security)) {
            qCritical() << "Security capability not present";
            return false;
        }
        qInfo() << "✓ Security capability present";

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

    bool test_validation_functionality() {
        qInfo() << "--- Testing Validation Functionality ---";

        // Test validation command with dummy file
        QJsonObject params{
            {"file_path", QCoreApplication::applicationFilePath()},
            {"security_level", 1}
        };

        auto result = m_plugin->execute_command("validate", params);
        if (!result) {
            qCritical() << "Validation command failed:" 
                       << QString::fromStdString(result.error().message);
            return false;
        }

        QJsonObject response = result.value();
        qInfo() << "Validation result:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
        qInfo() << "✓ Validation command executed";

        // Test security test command
        QJsonObject test_params{{"test_type", "validation"}};
        auto test_result = m_plugin->execute_command("security_test", test_params);
        if (!test_result) {
            qCritical() << "Security test command failed:" 
                       << QString::fromStdString(test_result.error().message);
            return false;
        }
        qInfo() << "✓ Security test command successful";

        qInfo() << "Validation functionality tests: PASSED";
        return true;
    }

    bool test_permission_functionality() {
        qInfo() << "--- Testing Permission Functionality ---";

        // Test permission command
        QJsonObject params{
            {"operation", "read"},
            {"context", QJsonObject{{"resource", "test_file.txt"}}}
        };

        auto result = m_plugin->execute_command("permission", params);
        if (!result) {
            qCritical() << "Permission command failed:" 
                       << QString::fromStdString(result.error().message);
            return false;
        }

        QJsonObject response = result.value();
        qInfo() << "Permission check result:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
        qInfo() << "✓ Permission command executed";

        // Test permission security test
        QJsonObject test_params{{"test_type", "permission"}};
        auto test_result = m_plugin->execute_command("security_test", test_params);
        if (!test_result) {
            qCritical() << "Permission security test failed:" 
                       << QString::fromStdString(test_result.error().message);
            return false;
        }
        qInfo() << "✓ Permission security test successful";

        qInfo() << "Permission functionality tests: PASSED";
        return true;
    }

    bool test_audit_functionality() {
        qInfo() << "--- Testing Audit Functionality ---";

        // Test audit get command
        QJsonObject params{{"action", "get"}, {"limit", 10}};
        auto result = m_plugin->execute_command("audit", params);
        if (!result) {
            qCritical() << "Audit get command failed:" 
                       << QString::fromStdString(result.error().message);
            return false;
        }
        qInfo() << "✓ Audit get command successful";

        // Test audit clear command
        QJsonObject clear_params{{"action", "clear"}};
        auto clear_result = m_plugin->execute_command("audit", clear_params);
        if (!clear_result) {
            qCritical() << "Audit clear command failed:" 
                       << QString::fromStdString(clear_result.error().message);
            return false;
        }
        qInfo() << "✓ Audit clear command successful";

        qInfo() << "Audit functionality tests: PASSED";
        return true;
    }

    bool test_policy_functionality() {
        qInfo() << "--- Testing Policy Functionality ---";

        // Test policy list command
        QJsonObject list_params{{"action", "list"}};
        auto list_result = m_plugin->execute_command("policy", list_params);
        if (!list_result) {
            qCritical() << "Policy list command failed:" 
                       << QString::fromStdString(list_result.error().message);
            return false;
        }
        qInfo() << "✓ Policy list command successful";

        // Test policy set command
        QJsonObject policy_config{
            {"allow_unsigned", false},
            {"require_trusted_publisher", true}
        };
        QJsonObject set_params{
            {"action", "set"},
            {"policy_name", "test_policy"},
            {"policy_config", policy_config}
        };
        auto set_result = m_plugin->execute_command("policy", set_params);
        if (!set_result) {
            qCritical() << "Policy set command failed:" 
                       << QString::fromStdString(set_result.error().message);
            return false;
        }
        qInfo() << "✓ Policy set command successful";

        qInfo() << "Policy functionality tests: PASSED";
        return true;
    }

private:
    std::unique_ptr<SecurityPlugin> m_plugin;
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    QString test_type = "basic";
    if (argc > 1) {
        test_type = QString::fromLocal8Bit(argv[1]);
    }

    SecurityPluginTester tester;
    int result = tester.run_tests(test_type);
    
    return result;
}

#include "test_security_plugin.moc"
