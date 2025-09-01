/**
 * @file network_plugin.cpp
 * @brief Implementation of network plugin demonstrating QtForge network
 * features
 * @version 3.0.0
 */

#include "network_plugin.hpp"
#include <QDateTime>
#include <QDebug>
#include <QHostInfo>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QSslConfiguration>
#include <QStringLiteral>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QWebSocket>
#include <QWebSocketServer>
#include <chrono>
#include <string_view>
#include <thread>

NetworkPlugin::NetworkPlugin(QObject* parent)
    : QObject(parent),
      m_network_manager(std::make_unique<QNetworkAccessManager>(this)),
      m_network_timer(std::make_unique<QTimer>(this)) {
    // Connect network manager signals
    connect(m_network_manager.get(), &QNetworkAccessManager::finished, this,
            &NetworkPlugin::on_http_request_finished);

    // Connect network timer
    connect(m_network_timer.get(), &QTimer::timeout, this,
            &NetworkPlugin::on_network_timer_timeout);

    // Initialize dependencies
    m_required_dependencies = {"qtplugin.NetworkManager"};
    m_optional_dependencies = {"qtplugin.MessageBus",
                               "qtplugin.ConfigurationManager"};

    log_info("NetworkPlugin constructed");
}

NetworkPlugin::~NetworkPlugin() {
    if (m_state != qtplugin::PluginState::Unloaded) {
        shutdown();
    }
}

// === Required IPlugin Interface Methods ===

std::string_view NetworkPlugin::name() const noexcept {
    return "NetworkPlugin";
}

std::string_view NetworkPlugin::description() const noexcept {
    return "Network plugin demonstrating QtForge network and REST API features";
}

qtplugin::Version NetworkPlugin::version() const noexcept {
    return qtplugin::Version{3, 0, 0};
}

std::string_view NetworkPlugin::author() const noexcept {
    return "QtForge Team";
}

std::string NetworkPlugin::id() const noexcept {
    return "qtforge.examples.network";
}

qtplugin::PluginState NetworkPlugin::state() const noexcept {
    return m_state.load();
}

qtplugin::expected<void, qtplugin::PluginError> NetworkPlugin::initialize() {
    if (m_state != qtplugin::PluginState::Unloaded &&
        m_state != qtplugin::PluginState::Loaded) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::StateError,
            "Plugin is not in a state that allows initialization");
    }

    m_state = qtplugin::PluginState::Initializing;
    m_initialization_time = std::chrono::system_clock::now();

    try {
        // Initialize network components
        initialize_network_components();

        // Setup SSL configuration if enabled
        if (m_ssl_enabled) {
            setup_ssl_configuration();
        }

        // Start network monitoring
        start_network_monitoring();

        m_state = qtplugin::PluginState::Running;
        log_info("NetworkPlugin initialized successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        m_state = qtplugin::PluginState::Error;
        std::string error_msg =
            "Initialization failed: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed, error_msg);
    }
}

void NetworkPlugin::shutdown() noexcept {
    try {
        QWriteLocker lock(&m_state_mutex);
        m_state = qtplugin::PluginState::Stopping;
        lock.unlock();

        // Stop network monitoring
        stop_network_monitoring();

        // Stop servers
        if (m_http_server) {
            stop_http_server();
        }

        if (m_websocket_server) {
            m_websocket_server->close();
        }

        // Disconnect WebSocket client
        if (m_websocket_client &&
            m_websocket_client->state() == QAbstractSocket::ConnectedState) {
            m_websocket_client->close();
        }

        // Cancel pending requests
        {
            QMutexLocker request_lock(&m_request_mutex);
            for (auto& [reply, request_data] : m_pending_requests) {
                if (reply && reply->isRunning()) {
                    reply->abort();
                }
            }
            m_pending_requests.clear();
        }

        lock.relock();
        m_state = qtplugin::PluginState::Stopped;
        lock.unlock();

        log_info("NetworkPlugin shutdown completed");
    } catch (...) {
        QWriteLocker lock(&m_state_mutex);
        m_state = qtplugin::PluginState::Error;
    }
}

bool NetworkPlugin::is_initialized() const noexcept {
    QReadLocker lock(&m_state_mutex);
    auto current_state = m_state.load();
    return current_state == qtplugin::PluginState::Running ||
           current_state == qtplugin::PluginState::Paused;
}

qtplugin::PluginMetadata NetworkPlugin::metadata() const {
    qtplugin::PluginMetadata meta;
    meta.name = "NetworkPlugin";
    meta.version = qtplugin::Version{3, 0, 0};
    meta.description =
        "Comprehensive network plugin demonstrating QtForge network features";
    meta.author = "QtForge Team";
    meta.license = "MIT";
    // meta.website = "https://github.com/QtForge/QtPlugin"; // Field not
    // available in current version
    meta.category = "Network";
    meta.tags = {"network", "http", "websocket", "rest-api", "ssl", "example"};

    // Network-specific metadata
    QJsonObject custom_data;
    custom_data[QStringLiteral("http_server_enabled")] = m_http_server_enabled;
    custom_data[QStringLiteral("websocket_server_enabled")] =
        m_websocket_server_enabled;
    custom_data[QStringLiteral("ssl_enabled")] = m_ssl_enabled;
    custom_data[QStringLiteral("http_server_port")] = m_http_server_port;
    custom_data[QStringLiteral("websocket_server_port")] =
        m_websocket_server_port;
    QJsonArray protocols;
    protocols.append(QStringLiteral("HTTP"));
    protocols.append(QStringLiteral("HTTPS"));
    protocols.append(QStringLiteral("WebSocket"));
    protocols.append(QStringLiteral("WebSocket Secure"));
    custom_data[QStringLiteral("supported_protocols")] = protocols;
    meta.custom_data = custom_data;

    return meta;
}

qtplugin::PluginCapabilities NetworkPlugin::capabilities() const noexcept {
    return qtplugin::PluginCapability::Network |
           qtplugin::PluginCapability::Configuration |
           qtplugin::PluginCapability::Monitoring |
           qtplugin::PluginCapability::Logging |
           qtplugin::PluginCapability::Threading;
}

qtplugin::PluginPriority NetworkPlugin::priority() const noexcept {
    return qtplugin::PluginPriority::Normal;
}

bool NetworkPlugin::is_thread_safe() const noexcept { return true; }

std::string_view NetworkPlugin::thread_model() const noexcept {
    return "multi-threaded";
}

std::optional<QJsonObject> NetworkPlugin::default_configuration() const {
    QJsonObject config;
    config[QStringLiteral("http_server_enabled")] = false;
    config[QStringLiteral("websocket_server_enabled")] = false;
    config[QStringLiteral("ssl_enabled")] = false;
    config[QStringLiteral("http_server_port")] = 8080;
    config[QStringLiteral("websocket_server_port")] = 8081;
    config[QStringLiteral("request_timeout")] = 30000;
    config[QStringLiteral("max_connections")] = 100;
    config[QStringLiteral("user_agent")] =
        QStringLiteral("QtForge-NetworkPlugin/3.0.0");
    config[QStringLiteral("network_monitoring_interval")] = 10000;
    config[QStringLiteral("enable_cors")] = true;
    QJsonArray corsOrigins;
    corsOrigins.append(QStringLiteral("*"));
    config[QStringLiteral("cors_origins")] = corsOrigins;
    config[QStringLiteral("ssl_certificate_path")] = QString();
    config[QStringLiteral("ssl_private_key_path")] = QString();
    config[QStringLiteral("authentication_enabled")] = false;
    config[QStringLiteral("api_key_header")] = QStringLiteral("X-API-Key");
    config[QStringLiteral("rate_limiting_enabled")] = false;
    config[QStringLiteral("rate_limit_requests_per_minute")] = 60;
    config[QStringLiteral("compression_enabled")] = true;
    config[QStringLiteral("keep_alive_enabled")] = true;
    config[QStringLiteral("connection_pool_size")] = 10;
    return config;
}

qtplugin::expected<void, qtplugin::PluginError> NetworkPlugin::configure(
    const QJsonObject& config) {
    if (!validate_configuration(config)) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError,
            "Invalid configuration provided");
    }

    // Store old configuration for comparison
    QJsonObject old_config = m_configuration;
    m_configuration = config;

    // Apply configuration changes
    if (config.contains(QStringLiteral("http_server_enabled"))) {
        m_http_server_enabled =
            config[QStringLiteral("http_server_enabled")].toBool();
    }

    if (config.contains(QStringLiteral("websocket_server_enabled"))) {
        m_websocket_server_enabled =
            config[QStringLiteral("websocket_server_enabled")].toBool();
    }

    if (config.contains(QStringLiteral("ssl_enabled"))) {
        m_ssl_enabled = config[QStringLiteral("ssl_enabled")].toBool();
    }

    if (config.contains(QStringLiteral("http_server_port"))) {
        m_http_server_port = config[QStringLiteral("http_server_port")].toInt();
    }

    if (config.contains(QStringLiteral("websocket_server_port"))) {
        m_websocket_server_port =
            config[QStringLiteral("websocket_server_port")].toInt();
    }

    if (config.contains(QStringLiteral("request_timeout"))) {
        m_request_timeout = config[QStringLiteral("request_timeout")].toInt();
    }

    if (config.contains(QStringLiteral("max_connections"))) {
        m_max_connections = config[QStringLiteral("max_connections")].toInt();
    }

    if (config.contains(QStringLiteral("user_agent"))) {
        m_user_agent = config[QStringLiteral("user_agent")].toString();
    }

    // Restart servers if configuration changed
    bool server_config_changed =
        old_config.value(QStringLiteral("http_server_port")) !=
            config.value(QStringLiteral("http_server_port")) ||
        old_config.value(QStringLiteral("websocket_server_port")) !=
            config.value(QStringLiteral("websocket_server_port")) ||
        old_config.value(QStringLiteral("ssl_enabled")) !=
            config.value(QStringLiteral("ssl_enabled"));

    if (server_config_changed && is_initialized()) {
        if (m_http_server_enabled && m_http_server) {
            stop_http_server();
            start_http_server(m_http_server_port);
        }

        if (m_websocket_server_enabled && m_websocket_server) {
            m_websocket_server->close();
            start_websocket_server(m_websocket_server_port);
        }
    }

    log_info("Network configuration updated successfully");
    return qtplugin::make_success();
}

QJsonObject NetworkPlugin::current_configuration() const {
    return m_configuration;
}

bool NetworkPlugin::validate_configuration(const QJsonObject& config) const {
    // Validate port numbers
    QStringList port_keys = {QStringLiteral("http_server_port"),
                             QStringLiteral("websocket_server_port")};
    for (const QString& port_key : port_keys) {
        if (config.contains(port_key)) {
            if (!config[port_key].isDouble()) {
                return false;
            }
            int port = config[port_key].toInt();
            if (port < 1 || port > 65535) {
                return false;
            }
        }
    }

    // Validate timeout values
    if (config.contains(QStringLiteral("request_timeout"))) {
        if (!config[QStringLiteral("request_timeout")].isDouble()) {
            return false;
        }
        int timeout = config[QStringLiteral("request_timeout")].toInt();
        if (timeout < 1000 || timeout > 300000) {  // 1 second to 5 minutes
            return false;
        }
    }

    // Validate max_connections
    if (config.contains(QStringLiteral("max_connections"))) {
        if (!config[QStringLiteral("max_connections")].isDouble()) {
            return false;
        }
        int max_conn = config[QStringLiteral("max_connections")].toInt();
        if (max_conn < 1 || max_conn > 10000) {
            return false;
        }
    }

    // Validate boolean flags
    QStringList flags = {QStringLiteral("http_server_enabled"),
                         QStringLiteral("websocket_server_enabled"),
                         QStringLiteral("ssl_enabled")};
    for (const QString& flag : flags) {
        if (config.contains(flag)) {
            if (!config[flag].isBool()) {
                return false;
            }
        }
    }

    return true;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
NetworkPlugin::execute_command(std::string_view command,
                               const QJsonObject& params) {
    if (command == "http") {
        return handle_http_command(params);
    } else if (command == "server") {
        return handle_server_command(params);
    } else if (command == "websocket") {
        return handle_websocket_command(params);
    } else if (command == "diagnostics") {
        return handle_diagnostics_command(params);
    } else if (command == "status") {
        return handle_status_command(params);
    } else if (command == "connectivity") {
        return handle_connectivity_command(params);
    } else {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound,
            "Unknown command: " + std::string(command));
    }
}

std::vector<std::string> NetworkPlugin::available_commands() const {
    return {"http",        "server", "websocket",
            "diagnostics", "status", "connectivity"};
}

// === Lifecycle Management ===

qtplugin::expected<void, qtplugin::PluginError> NetworkPlugin::pause() {
    QWriteLocker lock(&m_state_mutex);

    if (m_state != qtplugin::PluginState::Running) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be running to pause");
    }

    try {
        // Pause network monitoring
        if (m_network_timer && m_network_timer->isActive()) {
            m_network_timer->stop();
        }

        m_state = qtplugin::PluginState::Paused;
        log_info("NetworkPlugin paused successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to pause plugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> NetworkPlugin::resume() {
    QWriteLocker lock(&m_state_mutex);

    if (m_state != qtplugin::PluginState::Paused) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be paused to resume");
    }

    try {
        // Resume network monitoring
        start_network_monitoring();

        m_state = qtplugin::PluginState::Running;
        log_info("NetworkPlugin resumed successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to resume plugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> NetworkPlugin::restart() {
    log_info("Restarting NetworkPlugin...");

    // Shutdown first
    shutdown();

    // Wait a brief moment for cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Initialize again
    return initialize();
}

// === Dependency Management ===

std::vector<std::string> NetworkPlugin::dependencies() const {
    return m_required_dependencies;
}

std::vector<std::string> NetworkPlugin::optional_dependencies() const {
    return m_optional_dependencies;
}

bool NetworkPlugin::dependencies_satisfied() const {
    return m_dependencies_satisfied.load();
}

// === Monitoring ===

std::chrono::milliseconds NetworkPlugin::uptime() const {
    if (m_state == qtplugin::PluginState::Running) {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_initialization_time);
    }
    return std::chrono::milliseconds{0};
}

QJsonObject NetworkPlugin::performance_metrics() const {
    auto current_uptime = uptime();
    auto requests_per_second =
        current_uptime.count() > 0
            ? (m_requests_completed.load() * 1000.0) / current_uptime.count()
            : 0.0;

    QJsonObject metrics;
    metrics[QStringLiteral("uptime_ms")] =
        static_cast<qint64>(current_uptime.count());
    metrics[QStringLiteral("requests_sent")] =
        static_cast<qint64>(m_requests_sent.load());
    metrics[QStringLiteral("requests_completed")] =
        static_cast<qint64>(m_requests_completed.load());
    metrics[QStringLiteral("requests_failed")] =
        static_cast<qint64>(m_requests_failed.load());
    metrics[QStringLiteral("requests_per_second")] = requests_per_second;
    metrics[QStringLiteral("websocket_messages_sent")] =
        static_cast<qint64>(m_websocket_messages_sent.load());
    metrics[QStringLiteral("websocket_messages_received")] =
        static_cast<qint64>(m_websocket_messages_received.load());
    metrics[QStringLiteral("websocket_connections")] =
        static_cast<qint64>(m_websocket_connections.load());
    metrics[QStringLiteral("server_requests_handled")] =
        static_cast<qint64>(m_server_requests_handled.load());
    metrics[QStringLiteral("server_errors")] =
        static_cast<qint64>(m_server_errors.load());
    metrics[QStringLiteral("active_connections")] =
        static_cast<qint64>(m_active_connections.load());
    metrics[QStringLiteral("state")] = static_cast<int>(m_state.load());
    metrics[QStringLiteral("http_server_enabled")] = m_http_server_enabled;
    metrics[QStringLiteral("websocket_server_enabled")] =
        m_websocket_server_enabled;
    metrics[QStringLiteral("ssl_enabled")] = m_ssl_enabled;
    return metrics;
}

QJsonObject NetworkPlugin::resource_usage() const {
    QMutexLocker lock(&m_request_mutex);

    // Estimate resource usage
    auto memory_estimate = 1024 + (m_pending_requests.size() * 50) +
                           (m_websocket_clients.size() * 100);
    auto cpu_estimate =
        (m_network_timer && m_network_timer->isActive()) ? 1.5 : 0.1;

    QJsonObject usage;
    usage[QStringLiteral("estimated_memory_kb")] =
        static_cast<qint64>(memory_estimate);
    usage[QStringLiteral("estimated_cpu_percent")] = cpu_estimate;
    usage[QStringLiteral("thread_count")] = 1;
    usage[QStringLiteral("network_timer_active")] =
        m_network_timer && m_network_timer->isActive();
    usage[QStringLiteral("pending_requests")] =
        static_cast<int>(m_pending_requests.size());
    usage[QStringLiteral("websocket_clients")] =
        static_cast<int>(m_websocket_clients.size());
    usage[QStringLiteral("http_server_running")] = m_http_server != nullptr;
    usage[QStringLiteral("websocket_server_running")] =
        m_websocket_server && m_websocket_server->isListening();
    usage[QStringLiteral("error_log_size")] =
        static_cast<qint64>(m_error_log.size());
    usage[QStringLiteral("dependencies_satisfied")] = dependencies_satisfied();
    return usage;
}

void NetworkPlugin::clear_errors() {
    QMutexLocker lock(&m_error_mutex);
    m_error_log.clear();
    m_last_error.clear();
    m_error_count = 0;
}

// === Network-Specific Methods ===

qtplugin::expected<QJsonObject, qtplugin::PluginError>
NetworkPlugin::make_http_request(const QString& method, const QString& url,
                                 const QJsonObject& headers,
                                 const QJsonObject& body) {
    if (!m_network_manager) {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::InitializationFailed,
            "Network manager not initialized");
    }

    try {
        QNetworkRequest request{QUrl(url)};

        // Set default headers
        request.setHeader(QNetworkRequest::UserAgentHeader, m_user_agent);
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/json"));

        // Set custom headers
        for (auto it = headers.begin(); it != headers.end(); ++it) {
            request.setRawHeader(it.key().toUtf8(),
                                 it.value().toString().toUtf8());
        }

        // Set timeout
        request.setTransferTimeout(m_request_timeout);

        QNetworkReply* reply = nullptr;
        QByteArray request_body;

        if (!body.isEmpty()) {
            request_body = QJsonDocument(body).toJson(QJsonDocument::Compact);
        }

        // Make request based on method
        if (method.toUpper() == QStringLiteral("GET")) {
            reply = m_network_manager->get(request);
        } else if (method.toUpper() == QStringLiteral("POST")) {
            reply = m_network_manager->post(request, request_body);
        } else if (method.toUpper() == QStringLiteral("PUT")) {
            reply = m_network_manager->put(request, request_body);
        } else if (method.toUpper() == QStringLiteral("DELETE")) {
            reply = m_network_manager->deleteResource(request);
        } else if (method.toUpper() == QStringLiteral("HEAD")) {
            reply = m_network_manager->head(request);
        } else {
            return qtplugin::make_error<QJsonObject>(
                qtplugin::PluginErrorCode::InvalidParameters,
                "Unsupported HTTP method: " + method.toStdString());
        }

        if (!reply) {
            return qtplugin::make_error<QJsonObject>(
                qtplugin::PluginErrorCode::ExecutionFailed,
                "Failed to create network request");
        }

        // Track the request
        {
            QMutexLocker lock(&m_request_mutex);
            QJsonObject request_data;
            request_data[QStringLiteral("method")] = method;
            request_data[QStringLiteral("url")] = url;
            request_data[QStringLiteral("timestamp")] =
                QDateTime::currentDateTime().toString(Qt::ISODate);
            m_pending_requests[reply] = request_data;
        }

        m_requests_sent.fetch_add(1);

        // For synchronous operation, wait for completion
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        auto response = create_response_object(reply);
        cleanup_finished_request(reply);

        return response;

    } catch (const std::exception& e) {
        std::string error_msg = "HTTP request failed: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError>
NetworkPlugin::start_http_server(int port, const QJsonObject& routes) {
    try {
        if (!m_http_server) {
            m_http_server = std::make_unique<QHttpServer>(this);
        }

        // Setup routes
        setup_http_server_routes();

        // Add custom routes from configuration
        for (auto it = routes.begin(); it != routes.end(); ++it) {
            QString path = it.key();
            QJsonObject route_config = it.value().toObject();

            // This would need to be implemented based on QHttpServer API
            // m_http_server->route(path, [route_config](const
            // QHttpServerRequest& request) {
            //     // Handle route based on configuration
            // });
        }

        auto tcpServer = new QTcpServer();
        if (!tcpServer->listen(QHostAddress::Any, port) ||
            !m_http_server->bind(tcpServer)) {
            delete tcpServer;
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::ExecutionFailed,
                "Failed to start HTTP server on port " + std::to_string(port));
        }

        m_http_server_port = port;
        log_info("HTTP server started on port " + std::to_string(port));

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to start HTTP server: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError>
NetworkPlugin::stop_http_server() {
    try {
        if (m_http_server && !m_http_server->serverPorts().isEmpty()) {
            // QHttpServer doesn't have a direct stop method, we need to destroy
            // it
            m_http_server.reset();
            log_info("HTTP server stopped");
        }

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to stop HTTP server: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError>
NetworkPlugin::start_websocket_server(int port) {
    try {
        if (!m_websocket_server) {
            m_websocket_server = std::make_unique<QWebSocketServer>(
                QStringLiteral("QtForge NetworkPlugin WebSocket Server"),
                m_ssl_enabled ? QWebSocketServer::SecureMode
                              : QWebSocketServer::NonSecureMode,
                this);
        }

        if (!m_websocket_server->listen(QHostAddress::Any, port)) {
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::ExecutionFailed,
                "Failed to start WebSocket server on port " +
                    std::to_string(port));
        }

        m_websocket_server_port = port;
        log_info("WebSocket server started on port " + std::to_string(port));

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to start WebSocket server: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError>
NetworkPlugin::connect_websocket(const QString& url) {
    try {
        if (!m_websocket_client) {
            m_websocket_client = std::make_unique<QWebSocket>(
                QString(), QWebSocketProtocol::VersionLatest, this);

            // Connect signals
            connect(m_websocket_client.get(), &QWebSocket::connected, this,
                    &NetworkPlugin::on_websocket_connected);
            connect(m_websocket_client.get(), &QWebSocket::disconnected, this,
                    &NetworkPlugin::on_websocket_disconnected);
            connect(m_websocket_client.get(), &QWebSocket::textMessageReceived,
                    this, &NetworkPlugin::on_websocket_message_received);
            connect(m_websocket_client.get(), &QWebSocket::errorOccurred, this,
                    &NetworkPlugin::on_websocket_error);
        }

        m_websocket_client->open(QUrl(url));
        log_info("Connecting to WebSocket: " + url.toStdString());

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to connect WebSocket: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError>
NetworkPlugin::send_websocket_message(const QJsonObject& message) {
    if (!m_websocket_client ||
        m_websocket_client->state() != QAbstractSocket::ConnectedState) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "WebSocket client not connected");
    }

    try {
        QString message_text = QString::fromUtf8(
            QJsonDocument(message).toJson(QJsonDocument::Compact));
        qint64 bytes_sent = m_websocket_client->sendTextMessage(message_text);

        if (bytes_sent == -1) {
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::ExecutionFailed,
                "Failed to send WebSocket message");
        }

        m_websocket_messages_sent.fetch_add(1);
        log_info("WebSocket message sent: " + std::to_string(bytes_sent) +
                 " bytes");

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to send WebSocket message: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

QJsonObject NetworkPlugin::get_network_diagnostics() const {
    QJsonObject diagnostics;

    diagnostics[QStringLiteral("timestamp")] =
        QDateTime::currentDateTime().toString(Qt::ISODate);
    diagnostics[QStringLiteral("uptime_ms")] =
        static_cast<qint64>(uptime().count());

    // Network statistics
    QJsonObject stats;
    stats[QStringLiteral("requests_sent")] =
        static_cast<qint64>(m_requests_sent.load());
    stats[QStringLiteral("requests_completed")] =
        static_cast<qint64>(m_requests_completed.load());
    stats[QStringLiteral("requests_failed")] =
        static_cast<qint64>(m_requests_failed.load());
    stats[QStringLiteral("websocket_messages_sent")] =
        static_cast<qint64>(m_websocket_messages_sent.load());
    stats[QStringLiteral("websocket_messages_received")] =
        static_cast<qint64>(m_websocket_messages_received.load());
    stats[QStringLiteral("websocket_connections")] =
        static_cast<qint64>(m_websocket_connections.load());
    stats[QStringLiteral("server_requests_handled")] =
        static_cast<qint64>(m_server_requests_handled.load());
    stats[QStringLiteral("server_errors")] =
        static_cast<qint64>(m_server_errors.load());
    stats[QStringLiteral("active_connections")] =
        static_cast<qint64>(m_active_connections.load());
    diagnostics[QStringLiteral("statistics")] = stats;

    // Server status
    QJsonObject servers;
    servers[QStringLiteral("http_server_running")] =
        m_http_server && !m_http_server->serverPorts().isEmpty();
    servers[QStringLiteral("http_server_port")] = m_http_server_port;
    servers[QStringLiteral("websocket_server_running")] =
        m_websocket_server && m_websocket_server->isListening();
    servers[QStringLiteral("websocket_server_port")] = m_websocket_server_port;
    diagnostics[QStringLiteral("servers")] = servers;

    // Client status
    QJsonObject clients;
    clients[QStringLiteral("websocket_client_connected")] =
        m_websocket_client &&
        m_websocket_client->state() == QAbstractSocket::ConnectedState;
    if (m_websocket_client) {
        clients[QStringLiteral("websocket_client_url")] =
            m_websocket_client->requestUrl().toString();
        clients[QStringLiteral("websocket_client_state")] =
            static_cast<int>(m_websocket_client->state());
    }
    diagnostics[QStringLiteral("clients")] = clients;

    // Configuration
    QJsonObject config;
    config[QStringLiteral("ssl_enabled")] = m_ssl_enabled;
    config[QStringLiteral("request_timeout")] = m_request_timeout;
    config[QStringLiteral("max_connections")] = m_max_connections;
    config[QStringLiteral("user_agent")] = m_user_agent;
    diagnostics[QStringLiteral("configuration")] = config;

    return diagnostics;
}

QJsonObject NetworkPlugin::test_connectivity(const QString& host, int port) {
    QJsonObject result;
    result[QStringLiteral("host")] = host;
    result[QStringLiteral("port")] = port;
    result[QStringLiteral("timestamp")] =
        QDateTime::currentDateTime().toString(Qt::ISODate);

    try {
        QTcpSocket socket;
        socket.connectToHost(host, port);

        bool connected = socket.waitForConnected(5000);  // 5 second timeout

        result[QStringLiteral("connected")] = connected;
        result[QStringLiteral("response_time_ms")] =
            connected ? QDateTime::currentMSecsSinceEpoch() -
                            QDateTime::fromString(
                                result[QStringLiteral("timestamp")].toString(),
                                Qt::ISODate)
                                .toMSecsSinceEpoch()
                      : -1;

        if (connected) {
            result[QStringLiteral("local_address")] =
                socket.localAddress().toString();
            result[QStringLiteral("local_port")] = socket.localPort();
            result[QStringLiteral("peer_address")] =
                socket.peerAddress().toString();
            result[QStringLiteral("peer_port")] = socket.peerPort();
            socket.disconnectFromHost();
        } else {
            result[QStringLiteral("error")] = socket.errorString();
        }

    } catch (const std::exception& e) {
        result[QStringLiteral("connected")] = false;
        result[QStringLiteral("error")] = QString::fromStdString(e.what());
    }

    return result;
}

// === Private Slots ===

void NetworkPlugin::on_network_timer_timeout() {
    // Perform periodic network monitoring
    update_metrics();
    log_info("Network monitoring cycle completed");
}

void NetworkPlugin::on_http_request_finished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        m_requests_completed.fetch_add(1);
    } else {
        m_requests_failed.fetch_add(1);
        log_error("HTTP request failed: " + reply->errorString().toStdString());
    }

    cleanup_finished_request(reply);
}

void NetworkPlugin::on_websocket_connected() {
    m_websocket_connections.fetch_add(1);
    log_info("WebSocket connected successfully");
}

void NetworkPlugin::on_websocket_disconnected() {
    log_info("WebSocket disconnected");
}

void NetworkPlugin::on_websocket_message_received(const QString& message) {
    m_websocket_messages_received.fetch_add(1);
    log_info("WebSocket message received: " + std::to_string(message.length()) +
             " characters");
}

void NetworkPlugin::on_websocket_error(QAbstractSocket::SocketError error) {
    log_error("WebSocket error: " + std::to_string(static_cast<int>(error)));
}

void NetworkPlugin::on_http_server_new_request() {
    m_server_requests_handled.fetch_add(1);
}

// === Command Handlers ===

QJsonObject NetworkPlugin::handle_http_command(const QJsonObject& params) {
    QString method =
        params.value(QStringLiteral("method")).toString(QStringLiteral("GET"));
    QString url = params.value(QStringLiteral("url")).toString();
    QJsonObject headers = params.value(QStringLiteral("headers")).toObject();
    QJsonObject body = params.value(QStringLiteral("body")).toObject();

    if (url.isEmpty()) {
        QJsonObject error;
        error[QStringLiteral("error")] =
            QStringLiteral("Missing required parameter: url");
        error[QStringLiteral("success")] = false;
        return error;
    }

    auto result = make_http_request(method, url, headers, body);

    if (result) {
        QJsonObject response = result.value();
        response[QStringLiteral("success")] = true;
        return response;
    } else {
        QJsonObject error;
        error[QStringLiteral("error")] =
            QString::fromStdString(result.error().message);
        error[QStringLiteral("success")] = false;
        error[QStringLiteral("method")] = method;
        error[QStringLiteral("url")] = url;
        error[QStringLiteral("timestamp")] =
            QDateTime::currentDateTime().toString(Qt::ISODate);
        return error;
    }
}

QJsonObject NetworkPlugin::handle_server_command(const QJsonObject& params) {
    QString action = params.value(QStringLiteral("action"))
                         .toString(QStringLiteral("status"));

    if (action == QStringLiteral("start_http")) {
        int port =
            params.value(QStringLiteral("port")).toInt(m_http_server_port);
        QJsonObject routes = params.value(QStringLiteral("routes")).toObject();

        auto result = start_http_server(port, routes);

        QJsonObject response;
        response[QStringLiteral("action")] = QStringLiteral("start_http");
        response[QStringLiteral("port")] = port;
        response[QStringLiteral("success")] = result.has_value();
        response[QStringLiteral("error")] =
            result ? QString() : QString::fromStdString(result.error().message);
        response[QStringLiteral("timestamp")] =
            QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else if (action == QStringLiteral("stop_http")) {
        auto result = stop_http_server();

        QJsonObject response;
        response[QStringLiteral("action")] = QStringLiteral("stop_http");
        response[QStringLiteral("success")] = result.has_value();
        response[QStringLiteral("error")] =
            result ? QString() : QString::fromStdString(result.error().message);
        response[QStringLiteral("timestamp")] =
            QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else if (action == QStringLiteral("start_websocket")) {
        int port =
            params.value(QStringLiteral("port")).toInt(m_websocket_server_port);

        auto result = start_websocket_server(port);

        QJsonObject response;
        response[QStringLiteral("action")] = QStringLiteral("start_websocket");
        response[QStringLiteral("port")] = port;
        response[QStringLiteral("success")] = result.has_value();
        response[QStringLiteral("error")] =
            result ? QString() : QString::fromStdString(result.error().message);
        response[QStringLiteral("timestamp")] =
            QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else if (action == QStringLiteral("status")) {
        QJsonObject status;
        status[QStringLiteral("action")] = QStringLiteral("status");
        status[QStringLiteral("http_server_running")] =
            m_http_server && !m_http_server->serverPorts().isEmpty();
        status[QStringLiteral("http_server_port")] = m_http_server_port;
        status[QStringLiteral("websocket_server_running")] =
            m_websocket_server && m_websocket_server->isListening();
        status[QStringLiteral("websocket_server_port")] =
            m_websocket_server_port;
        status[QStringLiteral("success")] = true;
        status[QStringLiteral("timestamp")] =
            QDateTime::currentDateTime().toString(Qt::ISODate);
        return status;
    } else {
        QJsonObject error;
        error[QStringLiteral("error")] = QStringLiteral(
            "Invalid action. Supported: start_http, stop_http, "
            "start_websocket, status");
        error[QStringLiteral("success")] = false;
        return error;
    }
}

QJsonObject NetworkPlugin::handle_websocket_command(const QJsonObject& params) {
    QString action = params.value(QStringLiteral("action"))
                         .toString(QStringLiteral("status"));

    if (action == QStringLiteral("connect")) {
        QString url = params.value(QStringLiteral("url")).toString();

        if (url.isEmpty()) {
            QJsonObject error;
            error[QStringLiteral("error")] =
                QStringLiteral("Missing required parameter: url");
            error[QStringLiteral("success")] = false;
            return error;
        }

        auto result = connect_websocket(url);

        QJsonObject response;
        response[QStringLiteral("action")] = QStringLiteral("connect");
        response[QStringLiteral("url")] = url;
        response[QStringLiteral("success")] = result.has_value();
        response[QStringLiteral("error")] =
            result ? QString() : QString::fromStdString(result.error().message);
        response[QStringLiteral("timestamp")] =
            QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else if (action == QStringLiteral("send")) {
        QJsonObject message =
            params.value(QStringLiteral("message")).toObject();

        if (message.isEmpty()) {
            QJsonObject error;
            error[QStringLiteral("error")] =
                QStringLiteral("Missing required parameter: message");
            error[QStringLiteral("success")] = false;
            return error;
        }

        auto result = send_websocket_message(message);

        QJsonObject response;
        response[QStringLiteral("action")] = QStringLiteral("send");
        response[QStringLiteral("message")] = message;
        response[QStringLiteral("success")] = result.has_value();
        response[QStringLiteral("error")] =
            result ? QString() : QString::fromStdString(result.error().message);
        response[QStringLiteral("timestamp")] =
            QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else if (action == QStringLiteral("status")) {
        QJsonObject status;
        status[QStringLiteral("action")] = QStringLiteral("status");
        status[QStringLiteral("client_connected")] =
            m_websocket_client &&
            m_websocket_client->state() == QAbstractSocket::ConnectedState;
        if (m_websocket_client) {
            status[QStringLiteral("client_url")] =
                m_websocket_client->requestUrl().toString();
            status[QStringLiteral("client_state")] =
                static_cast<int>(m_websocket_client->state());
        }
        status[QStringLiteral("server_running")] =
            m_websocket_server && m_websocket_server->isListening();
        status[QStringLiteral("server_port")] = m_websocket_server_port;
        status[QStringLiteral("success")] = true;
        status[QStringLiteral("timestamp")] =
            QDateTime::currentDateTime().toString(Qt::ISODate);

        return status;
    } else {
        QJsonObject error;
        error[QStringLiteral("error")] =
            QStringLiteral("Invalid action. Supported: connect, send, status");
        error[QStringLiteral("success")] = false;
        return error;
    }
}

QJsonObject NetworkPlugin::handle_diagnostics_command(
    const QJsonObject& params) {
    Q_UNUSED(params)

    auto diagnostics = get_network_diagnostics();

    QJsonObject response;
    response[QStringLiteral("action")] = QStringLiteral("diagnostics");
    response[QStringLiteral("diagnostics")] = diagnostics;
    response[QStringLiteral("success")] = true;
    return response;
}

QJsonObject NetworkPlugin::handle_status_command(const QJsonObject& params) {
    Q_UNUSED(params)

    QJsonObject status;
    status[QStringLiteral("plugin_name")] = QStringLiteral("NetworkPlugin");
    status[QStringLiteral("state")] = static_cast<int>(m_state.load());
    status[QStringLiteral("uptime_ms")] = static_cast<qint64>(uptime().count());
    status[QStringLiteral("http_server_enabled")] = m_http_server_enabled;
    status[QStringLiteral("websocket_server_enabled")] =
        m_websocket_server_enabled;
    status[QStringLiteral("ssl_enabled")] = m_ssl_enabled;

    // Component status
    QJsonObject components;
    components[QStringLiteral("network_manager")] =
        m_network_manager != nullptr;
    components[QStringLiteral("http_server")] = m_http_server != nullptr;
    components[QStringLiteral("websocket_server")] =
        m_websocket_server != nullptr;
    components[QStringLiteral("websocket_client")] =
        m_websocket_client != nullptr;
    status[QStringLiteral("components")] = components;

    // Statistics
    status[QStringLiteral("statistics")] = performance_metrics();

    // Resource usage
    status[QStringLiteral("resource_usage")] = resource_usage();

    status[QStringLiteral("timestamp")] =
        QDateTime::currentDateTime().toString(Qt::ISODate);
    status[QStringLiteral("success")] = true;

    return status;
}

QJsonObject NetworkPlugin::handle_connectivity_command(
    const QJsonObject& params) {
    QString host = params.value(QStringLiteral("host")).toString();
    int port = params.value(QStringLiteral("port")).toInt(80);

    if (host.isEmpty()) {
        QJsonObject error;
        error[QStringLiteral("error")] =
            QStringLiteral("Missing required parameter: host");
        error[QStringLiteral("success")] = false;
        return error;
    }

    auto result = test_connectivity(host, port);
    result[QStringLiteral("action")] = QStringLiteral("connectivity");
    result[QStringLiteral("success")] = true;

    return result;
}

// === Helper Methods ===

void NetworkPlugin::log_error(const std::string& error) {
    {
        QMutexLocker lock(&m_error_mutex);
        m_last_error = error;
        m_error_log.push_back(error);

        // Maintain error log size
        if (m_error_log.size() > MAX_ERROR_LOG_SIZE) {
            m_error_log.erase(m_error_log.begin());
        }
    }

    m_error_count.fetch_add(1);
    qWarning() << "NetworkPlugin Error:" << QString::fromStdString(error);
}

void NetworkPlugin::log_info(const std::string& message) {
    qInfo() << "NetworkPlugin:" << QString::fromStdString(message);
}

void NetworkPlugin::update_metrics() {
    // Update internal metrics - could be expanded for more detailed monitoring
}

void NetworkPlugin::initialize_network_components() {
    // Network manager is already initialized in constructor

    // Set up network access manager configuration
    if (m_network_manager) {
        m_network_manager->setTransferTimeout(m_request_timeout);
    }

    log_info("Network components initialized");
}

void NetworkPlugin::setup_http_server_routes() {
    if (!m_http_server) {
        return;
    }

    // Setup basic routes - this would need to be expanded based on QHttpServer
    // API Example routes that could be implemented:

    // Health check endpoint
    // m_http_server->route("/health", [this](const QHttpServerRequest& request)
    // {
    //     QJsonObject health;
    //     health["status"] = "ok";
    //     health["timestamp"] =
    //     QDateTime::currentDateTime().toString(Qt::ISODate);
    //     health["uptime_ms"] = static_cast<qint64>(uptime().count());
    //
    //     return QHttpServerResponse(QJsonDocument(health).toJson(),
    //     "application/json");
    // });

    // Status endpoint
    // m_http_server->route("/status", [this](const QHttpServerRequest& request)
    // {
    //     auto status = get_network_diagnostics();
    //     return QHttpServerResponse(QJsonDocument(status).toJson(),
    //     "application/json");
    // });

    log_info("HTTP server routes configured");
}

void NetworkPlugin::setup_ssl_configuration() {
    if (!m_ssl_enabled) {
        return;
    }

    // Setup SSL configuration for network manager
    QSslConfiguration ssl_config = QSslConfiguration::defaultConfiguration();

    // Configure SSL settings based on configuration
    QString cert_path =
        m_configuration.value(QStringLiteral("ssl_certificate_path"))
            .toString();
    QString key_path =
        m_configuration.value(QStringLiteral("ssl_private_key_path"))
            .toString();

    if (!cert_path.isEmpty() && !key_path.isEmpty()) {
        // Load certificate and private key
        // ssl_config.setLocalCertificate(QSslCertificate(cert_path));
        // ssl_config.setPrivateKey(QSslKey(key_path));
    }

    // Set the configuration
    QSslConfiguration::setDefaultConfiguration(ssl_config);

    log_info("SSL configuration setup completed");
}

void NetworkPlugin::start_network_monitoring() {
    if (m_network_timer) {
        int interval =
            m_configuration.value(QStringLiteral("network_monitoring_interval"))
                .toInt(10000);
        m_network_timer->setInterval(interval);
        m_network_timer->start();
        log_info("Network monitoring started");
    }
}

void NetworkPlugin::stop_network_monitoring() {
    if (m_network_timer && m_network_timer->isActive()) {
        m_network_timer->stop();
        log_info("Network monitoring stopped");
    }
}

QJsonObject NetworkPlugin::create_response_object(QNetworkReply* reply) {
    QJsonObject response;

    if (!reply) {
        response[QStringLiteral("error")] =
            QStringLiteral("Invalid reply object");
        return response;
    }

    // Basic response information
    response[QStringLiteral("status_code")] =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    response[QStringLiteral("status_text")] =
        reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    response[QStringLiteral("url")] = reply->url().toString();
    response[QStringLiteral("timestamp")] =
        QDateTime::currentDateTime().toString(Qt::ISODate);

    // Headers
    QJsonObject headers;
    for (const auto& header : reply->rawHeaderPairs()) {
        headers[QString::fromUtf8(header.first)] =
            QString::fromUtf8(header.second);
    }
    response[QStringLiteral("headers")] = headers;

    // Body
    QByteArray data = reply->readAll();
    response[QStringLiteral("body_size")] = data.size();

    // Try to parse as JSON
    QJsonParseError parse_error;
    QJsonDocument json_doc = QJsonDocument::fromJson(data, &parse_error);

    if (parse_error.error == QJsonParseError::NoError) {
        if (json_doc.isObject()) {
            response[QStringLiteral("body")] = json_doc.object();
        } else if (json_doc.isArray()) {
            response[QStringLiteral("body")] = json_doc.array();
        }
        response[QStringLiteral("content_type")] =
            QStringLiteral("application/json");
    } else {
        // Store as text
        response[QStringLiteral("body")] = QString::fromUtf8(data);
        response[QStringLiteral("content_type")] = QStringLiteral("text/plain");
    }

    // Error information
    if (reply->error() != QNetworkReply::NoError) {
        response[QStringLiteral("error")] = reply->errorString();
        response[QStringLiteral("error_code")] =
            static_cast<int>(reply->error());
    }

    return response;
}

void NetworkPlugin::cleanup_finished_request(QNetworkReply* reply) {
    if (!reply) {
        return;
    }

    {
        QMutexLocker lock(&m_request_mutex);
        m_pending_requests.erase(reply);
    }

    reply->deleteLater();
}

// === Plugin Factory ===

std::unique_ptr<NetworkPlugin> NetworkPlugin::create_instance() {
    return std::make_unique<NetworkPlugin>();
}

qtplugin::PluginMetadata NetworkPlugin::get_static_metadata() {
    qtplugin::PluginMetadata meta;
    meta.name = "NetworkPlugin";
    meta.version = qtplugin::Version{3, 0, 0};
    meta.description =
        "Comprehensive network plugin demonstrating QtForge network features";
    meta.author = "QtForge Team";
    meta.license = "MIT";
    meta.category = "Network";
    meta.tags = {"network", "http", "websocket", "rest-api", "ssl", "example"};
    return meta;
}

// #include "network_plugin.moc" // Removed - not needed
