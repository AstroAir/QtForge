/**
 * @file test_request_response_plugin.cpp
 * @brief Test application for request-response plugin
 * @version 1.0.0
 */

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QTimer>

#include "request_response_plugin.hpp"

using namespace qtplugin::examples;

class RequestResponseTester : public QObject {
    Q_OBJECT

public:
    explicit RequestResponseTester(QObject* parent = nullptr)
        : QObject(parent) {}

public slots:
    void run_tests() {
        qDebug() << "=== Request-Response Plugin Test Suite ===";

        // Create plugin instance
        auto plugin = std::make_unique<RequestResponsePlugin>();

        // Test 1: Basic initialization
        qDebug() << "\n--- Test 1: Basic Initialization ---";
        QJsonObject init_config{{"default_timeout_ms", 10000},
                                {"max_pending_requests", 100},
                                {"enable_request_queuing", true}};

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

        // Test 2: Send simple request
        qDebug() << "\n--- Test 2: Send Simple Request ---";
        QJsonObject send_result = plugin->execute_command(
            "send_request",
            QJsonObject{
                {"target", "test_service"},
                {"request", QJsonObject{{"action", "get_data"},
                                        {"params", QJsonObject{{"id", 123}}}}},
                {"async", true},
                {"priority", 1}});
        qDebug() << "Send request result:"
                 << QJsonDocument(send_result).toJson(QJsonDocument::Compact);

        // Test 3: Send multiple requests
        qDebug() << "\n--- Test 3: Send Multiple Requests ---";
        for (int i = 0; i < 5; ++i) {
            QJsonObject multi_result = plugin->execute_command(
                "send_request",
                QJsonObject{
                    {"target", "batch_service"},
                    {"request", QJsonObject{{"action", "process_item"},
                                            {"item_id", i},
                                            {"batch_id", "batch_001"}}},
                    {"async", true},
                    {"priority", i % 3}  // Different priorities
                });
            qDebug()
                << "Batch request" << i << "result:"
                << QJsonDocument(multi_result).toJson(QJsonDocument::Compact);
        }

        // Test 4: List pending requests
        qDebug() << "\n--- Test 4: List Pending Requests ---";
        QJsonObject pending_result = plugin->execute_command("list_pending");
        qDebug()
            << "Pending requests:"
            << QJsonDocument(pending_result).toJson(QJsonDocument::Compact);

        // Test 5: Get statistics
        qDebug() << "\n--- Test 5: Get Statistics ---";
        QJsonObject stats_result = plugin->execute_command("get_statistics");
        qDebug() << "Statistics:"
                 << QJsonDocument(stats_result).toJson(QJsonDocument::Compact);

        // Test 6: Cancel a request
        qDebug() << "\n--- Test 6: Cancel Request ---";
        QString request_id_to_cancel = send_result["request_id"].toString();
        if (!request_id_to_cancel.isEmpty()) {
            QJsonObject cancel_result = plugin->execute_command(
                "cancel_request",
                QJsonObject{{"request_id", request_id_to_cancel}});
            qDebug()
                << "Cancel request result:"
                << QJsonDocument(cancel_result).toJson(QJsonDocument::Compact);
        }

        // Test 7: Invalid request (missing parameters)
        qDebug() << "\n--- Test 7: Invalid Request ---";
        QJsonObject invalid_result = plugin->execute_command(
            "send_request", QJsonObject{
                                {"target", ""}  // Empty target
                            });
        qDebug()
            << "Invalid request result:"
            << QJsonDocument(invalid_result).toJson(QJsonDocument::Compact);

        // Test 8: Unknown command
        qDebug() << "\n--- Test 8: Unknown Command ---";
        QJsonObject unknown_result = plugin->execute_command("unknown_command");
        qDebug()
            << "Unknown command result:"
            << QJsonDocument(unknown_result).toJson(QJsonDocument::Compact);

        // Wait for some responses to arrive
        qDebug() << "\n--- Waiting for responses (5 seconds) ---";
        QTimer::singleShot(5000, [this, &plugin]() {
            // Test 9: Get updated statistics
            qDebug() << "\n--- Test 9: Updated Statistics ---";
            QJsonObject final_stats = plugin->execute_command("get_statistics");
            qDebug()
                << "Final statistics:"
                << QJsonDocument(final_stats).toJson(QJsonDocument::Compact);

            // Test 10: List remaining pending requests
            qDebug() << "\n--- Test 10: Remaining Pending Requests ---";
            QJsonObject remaining_pending =
                plugin->execute_command("list_pending");
            qDebug() << "Remaining pending:"
                     << QJsonDocument(remaining_pending)
                            .toJson(QJsonDocument::Compact);

            // Test 11: Clear statistics
            qDebug() << "\n--- Test 11: Clear Statistics ---";
            QJsonObject clear_result =
                plugin->execute_command("clear_statistics");
            qDebug()
                << "Clear statistics result:"
                << QJsonDocument(clear_result).toJson(QJsonDocument::Compact);

            // Test 12: Verify statistics cleared
            qDebug() << "\n--- Test 12: Verify Statistics Cleared ---";
            QJsonObject cleared_stats =
                plugin->execute_command("get_statistics");
            qDebug()
                << "Cleared statistics:"
                << QJsonDocument(cleared_stats).toJson(QJsonDocument::Compact);

            // Test 13: Final metadata check
            qDebug() << "\n--- Test 13: Final Metadata Check ---";
            QJsonObject final_metadata = plugin->metadata();
            qDebug()
                << "Final metadata:"
                << QJsonDocument(final_metadata).toJson(QJsonDocument::Compact);

            // Shutdown
            qDebug() << "\n--- Shutdown ---";
            plugin->shutdown();
            qDebug() << "Plugin state after shutdown:"
                     << static_cast<int>(plugin->state());

            qDebug() << "\n=== Request-Response Plugin Tests Complete ===";

            // Schedule application exit
            QTimer::singleShot(100, []() { QCoreApplication::exit(0); });
        });
    }
};

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    app.setApplicationName("RequestResponsePluginTest");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("QtForge");
    app.setOrganizationDomain("qtforge.org");

    qDebug() << "Starting Request-Response Plugin Test...";
    qDebug() << "Qt version:" << QT_VERSION_STR;
    qDebug() << "Application:" << app.applicationName()
             << app.applicationVersion();

    RequestResponseTester tester;

    // Start tests after event loop starts
    QTimer::singleShot(0, &tester, &RequestResponseTester::run_tests);

    return app.exec();
}

#include "test_request_response_plugin.moc"
