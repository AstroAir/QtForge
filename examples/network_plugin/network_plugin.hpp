/**
 * @file network_plugin.hpp
 * @brief Network plugin demonstrating QtForge network and REST API features
 * @version 3.0.0
 *
 * This plugin demonstrates comprehensive network functionality including:
 * - REST API client and server capabilities
 * - HTTP/HTTPS request handling with authentication
 * - WebSocket communication for real-time data
 * - Network monitoring and diagnostics
 * - SSL/TLS security and certificate management
 */

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpServer>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QTimer>
#include <QJsonObject>
#include <QMutex>
#include <QReadWriteLock>
#include <atomic>
#include <memory>
#include <vector>
#include <unordered_map>

#include "qtplugin/core/plugin_interface.hpp"
#include "qtplugin/communication/message_bus.hpp"
#include "qtplugin/utils/error_handling.hpp"

/**
 * @brief Network plugin demonstrating QtForge network features
 *
 * This plugin showcases:
 * - REST API client with full HTTP method support
 * - HTTP server with routing and middleware
 * - WebSocket server and client functionality
 * - Network diagnostics and monitoring
 * - SSL/TLS security implementation
 * - Authentication and authorization
 */
class NetworkPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.NetworkPlugin" FILE "network_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit NetworkPlugin(QObject* parent = nullptr);
    ~NetworkPlugin() override;

    // === IPlugin Interface ===

    // Required pure virtual methods
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    qtplugin::Version version() const noexcept override;
    std::string_view author() const noexcept override;
    std::string id() const noexcept override;
    qtplugin::PluginState state() const noexcept override;

    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    bool is_initialized() const noexcept override;

    qtplugin::PluginMetadata metadata() const override;
    qtplugin::PluginCapabilities capabilities() const noexcept override;
    qtplugin::PluginPriority priority() const noexcept override;
    bool is_thread_safe() const noexcept override;
    std::string_view thread_model() const noexcept override;

    // === Configuration Management ===
    std::optional<QJsonObject> default_configuration() const override;
    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;
    QJsonObject current_configuration() const override;
    bool validate_configuration(const QJsonObject& config) const override;

    // === Command Execution ===
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params) override;
    std::vector<std::string> available_commands() const override;

    // === Lifecycle Management ===
    qtplugin::expected<void, qtplugin::PluginError> pause() override;
    qtplugin::expected<void, qtplugin::PluginError> resume() override;
    qtplugin::expected<void, qtplugin::PluginError> restart() override;

    // === Dependency Management ===
    std::vector<std::string> dependencies() const override;
    std::vector<std::string> optional_dependencies() const override;
    bool dependencies_satisfied() const override;

    // === Monitoring ===
    std::chrono::milliseconds uptime() const override;
    QJsonObject performance_metrics() const override;
    QJsonObject resource_usage() const override;
    void clear_errors() override;

    // === Network-Specific Methods ===

    /**
     * @brief Make HTTP request
     * @param method HTTP method (GET, POST, PUT, DELETE, etc.)
     * @param url Target URL
     * @param headers Request headers
     * @param body Request body (for POST/PUT)
     * @return Response data or error
     */
    qtplugin::expected<QJsonObject, qtplugin::PluginError> make_http_request(
        const QString& method, const QString& url,
        const QJsonObject& headers = {}, const QJsonObject& body = {});

    /**
     * @brief Start HTTP server
     * @param port Port to listen on
     * @param routes Server routes configuration
     * @return Success or error
     */
    qtplugin::expected<void, qtplugin::PluginError> start_http_server(
        int port, const QJsonObject& routes = {});

    /**
     * @brief Stop HTTP server
     * @return Success or error
     */
    qtplugin::expected<void, qtplugin::PluginError> stop_http_server();

    /**
     * @brief Start WebSocket server
     * @param port Port to listen on
     * @return Success or error
     */
    qtplugin::expected<void, qtplugin::PluginError> start_websocket_server(int port);

    /**
     * @brief Connect to WebSocket server
     * @param url WebSocket server URL
     * @return Success or error
     */
    qtplugin::expected<void, qtplugin::PluginError> connect_websocket(const QString& url);

    /**
     * @brief Send WebSocket message
     * @param message Message to send
     * @return Success or error
     */
    qtplugin::expected<void, qtplugin::PluginError> send_websocket_message(
        const QJsonObject& message);

    /**
     * @brief Get network diagnostics
     * @return Network diagnostic information
     */
    QJsonObject get_network_diagnostics() const;

    /**
     * @brief Test network connectivity
     * @param host Host to test
     * @param port Port to test
     * @return Connectivity test results
     */
    QJsonObject test_connectivity(const QString& host, int port = 80);

private slots:
    void on_network_timer_timeout();
    void on_http_request_finished();
    void on_websocket_connected();
    void on_websocket_disconnected();
    void on_websocket_message_received(const QString& message);
    void on_websocket_error(QAbstractSocket::SocketError error);
    void on_http_server_new_request();

private:
    // === Network Components ===
    std::unique_ptr<QNetworkAccessManager> m_network_manager;
    std::unique_ptr<QHttpServer> m_http_server;
    std::unique_ptr<QWebSocketServer> m_websocket_server;
    std::unique_ptr<QWebSocket> m_websocket_client;

    // === State Management ===
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    std::atomic<bool> m_dependencies_satisfied{false};
    mutable QReadWriteLock m_state_mutex;

    // === Configuration ===
    QJsonObject m_configuration;
    bool m_http_server_enabled{false};
    bool m_websocket_server_enabled{false};
    bool m_ssl_enabled{false};
    int m_http_server_port{8080};
    int m_websocket_server_port{8081};
    int m_request_timeout{30000}; // 30 seconds
    int m_max_connections{100};
    QString m_user_agent{QStringLiteral("QtForge-NetworkPlugin/3.0.0")};

    // === Network Monitoring ===
    std::unique_ptr<QTimer> m_network_timer;
    std::chrono::system_clock::time_point m_initialization_time;

    // === Request Tracking ===
    mutable QMutex m_request_mutex;
    std::unordered_map<QNetworkReply*, QJsonObject> m_pending_requests;
    std::atomic<uint64_t> m_requests_sent{0};
    std::atomic<uint64_t> m_requests_completed{0};
    std::atomic<uint64_t> m_requests_failed{0};

    // === WebSocket Tracking ===
    mutable QMutex m_websocket_mutex;
    std::vector<QWebSocket*> m_websocket_clients;
    std::atomic<uint64_t> m_websocket_messages_sent{0};
    std::atomic<uint64_t> m_websocket_messages_received{0};
    std::atomic<uint64_t> m_websocket_connections{0};

    // === Server Statistics ===
    std::atomic<uint64_t> m_server_requests_handled{0};
    std::atomic<uint64_t> m_server_errors{0};
    std::atomic<uint64_t> m_active_connections{0};

    // === Dependencies ===
    std::vector<std::string> m_required_dependencies;
    std::vector<std::string> m_optional_dependencies;

    // === Error Handling ===
    mutable QMutex m_error_mutex;
    std::vector<std::string> m_error_log;
    std::string m_last_error;
    std::atomic<uint64_t> m_error_count{0};
    static constexpr size_t MAX_ERROR_LOG_SIZE = 100;

    // === Command Handlers ===
    QJsonObject handle_http_command(const QJsonObject& params);
    QJsonObject handle_server_command(const QJsonObject& params);
    QJsonObject handle_websocket_command(const QJsonObject& params);
    QJsonObject handle_diagnostics_command(const QJsonObject& params);
    QJsonObject handle_status_command(const QJsonObject& params);
    QJsonObject handle_connectivity_command(const QJsonObject& params);

    // === Helper Methods ===
    void log_error(const std::string& error);
    void log_info(const std::string& message);
    void update_metrics();
    void initialize_network_components();
    void setup_http_server_routes();
    void setup_ssl_configuration();
    void start_network_monitoring();
    void stop_network_monitoring();
    QJsonObject create_response_object(QNetworkReply* reply);
    void cleanup_finished_request(QNetworkReply* reply);

public:
    // === Plugin Factory ===
    static std::unique_ptr<NetworkPlugin> create_instance();
    static qtplugin::PluginMetadata get_static_metadata();
};
