/**
 * @file request_response_plugin.hpp
 * @brief Request-Response communication pattern example plugin
 * @version 1.0.0
 */

#pragma once

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QTimer>
#include <QUuid>
#include <atomic>
#include <memory>
#include <unordered_map>

#include "qtplugin/communication/message_bus.hpp"
#include "qtplugin/core/plugin_interface.hpp"

namespace qtplugin::examples {

/**
 * @brief Request-Response communication pattern example
 *
 * Demonstrates synchronous and asynchronous request-response patterns
 * including:
 * - Request routing and handling
 * - Response correlation and timeout handling
 * - Request queuing and prioritization
 * - Performance monitoring and statistics
 */
class RequestResponsePlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE
                          "request_response_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit RequestResponsePlugin(QObject* parent = nullptr);
    ~RequestResponsePlugin() override;

    // IPlugin interface
    bool initialize(const QJsonObject& config = {}) override;
    void shutdown() override;
    QString name() const override { return "RequestResponsePlugin"; }
    QString version() const override { return "1.0.0"; }
    QString description() const override {
        return "Request-Response communication pattern example";
    }
    qtplugin::PluginState state() const override { return m_state.load(); }
    QJsonObject metadata() const override;
    QJsonObject execute_command(const QString& command,
                                const QJsonObject& params = {}) override;

public slots:
    void on_request_received(const QString& request_id,
                             const QJsonObject& request);
    void on_response_received(const QString& request_id,
                              const QJsonObject& response);
    void on_request_timeout(const QString& request_id);

private slots:
    void cleanup_expired_requests();
    void process_request_queue();

private:
    struct PendingRequest {
        QString id;
        QJsonObject request;
        QDateTime timestamp;
        QDateTime timeout;
        QString sender;
        int priority;
        int retry_count;
        bool is_async;
    };

    struct RequestStats {
        std::atomic<int> total_requests{0};
        std::atomic<int> successful_responses{0};
        std::atomic<int> failed_responses{0};
        std::atomic<int> timeout_responses{0};
        std::atomic<int> pending_requests{0};
        std::atomic<double> average_response_time{0.0};
    };

    // Request handling
    QString send_request(const QJsonObject& request, const QString& target,
                         bool async = false, int priority = 0);
    void handle_request(const QString& request_id, const QJsonObject& request);
    void send_response(const QString& request_id, const QJsonObject& response,
                       bool success = true);
    void handle_response(const QString& request_id,
                         const QJsonObject& response);

    // Request management
    void add_pending_request(const PendingRequest& request);
    void remove_pending_request(const QString& request_id);
    PendingRequest* find_pending_request(const QString& request_id);
    void update_statistics(const QString& request_id, bool success,
                           double response_time);

    // Command implementations
    QJsonObject execute_send_request_command(const QJsonObject& params);
    QJsonObject execute_get_statistics_command(const QJsonObject& params);
    QJsonObject execute_clear_statistics_command(const QJsonObject& params);
    QJsonObject execute_list_pending_command(const QJsonObject& params);
    QJsonObject execute_cancel_request_command(const QJsonObject& params);

    // State management
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    mutable QMutex m_requests_mutex;

    // Request management
    std::unordered_map<QString, PendingRequest> m_pending_requests;
    std::unique_ptr<QTimer> m_cleanup_timer;
    std::unique_ptr<QTimer> m_queue_processor;

    // Configuration
    int m_default_timeout_ms{30000};
    int m_max_pending_requests{1000};
    int m_max_retry_count{3};
    bool m_enable_request_queuing{true};

    // Statistics
    RequestStats m_stats;
    QDateTime m_stats_start_time;

    // Message bus integration
    std::unique_ptr<qtplugin::MessageBus> m_message_bus;
};

}  // namespace qtplugin::examples
