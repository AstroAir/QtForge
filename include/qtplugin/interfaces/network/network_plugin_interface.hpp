/**
 * @file network_plugin_interface.hpp
 * @brief Network plugin interface for network operations and communication
 * @version 3.0.0
 */

#pragma once

#include <QByteArray>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QSslConfiguration>
#include <QUrl>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>
#include "../core/plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Network operation types
 */
enum class NetworkOperation : uint32_t {
    None = 0x0000,
    HttpGet = 0x0001,         ///< HTTP GET request
    HttpPost = 0x0002,        ///< HTTP POST request
    HttpPut = 0x0004,         ///< HTTP PUT request
    HttpDelete = 0x0008,      ///< HTTP DELETE request
    HttpPatch = 0x0010,       ///< HTTP PATCH request
    WebSocket = 0x0020,       ///< WebSocket communication
    TcpSocket = 0x0040,       ///< TCP socket communication
    UdpSocket = 0x0080,       ///< UDP socket communication
    SslSocket = 0x0100,       ///< SSL/TLS socket communication
    FileTransfer = 0x0200,    ///< File transfer operations
    Streaming = 0x0400,       ///< Data streaming
    Proxy = 0x0800,           ///< Proxy operations
    Authentication = 0x1000,  ///< Authentication operations
    Monitoring = 0x2000,      ///< Network monitoring
    Discovery = 0x4000        ///< Network service discovery
};

using NetworkOperations = std::underlying_type_t<NetworkOperation>;

/**
 * @brief Network protocol types
 */
enum class NetworkProtocol {
    Http,
    Https,
    WebSocket,
    WebSocketSecure,
    Tcp,
    Udp,
    Ssl,
    Ftp,
    Sftp,
    Custom
};

/**
 * @brief Network request context
 */
struct NetworkRequestContext {
    QString request_id;   ///< Unique request identifier
    QUrl url;             ///< Request URL
    QJsonObject headers;  ///< Request headers
    QByteArray data;      ///< Request data
    NetworkProtocol protocol = NetworkProtocol::Http;  ///< Network protocol
    std::chrono::milliseconds timeout{30000};          ///< Request timeout
    int max_redirects = 5;                             ///< Maximum redirects
    bool verify_ssl = true;                            ///< SSL verification
    QJsonObject authentication;                        ///< Authentication data
    QJsonObject proxy_config;                          ///< Proxy configuration
    QJsonObject custom_options;                        ///< Custom options

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static NetworkRequestContext from_json(const QJsonObject& json);
};

/**
 * @brief Network response data
 */
struct NetworkResponse {
    bool success = false;                        ///< Request success status
    int status_code = 0;                         ///< HTTP status code
    QString status_message;                      ///< Status message
    QJsonObject headers;                         ///< Response headers
    QByteArray data;                             ///< Response data
    QUrl final_url;                              ///< Final URL after redirects
    std::chrono::milliseconds response_time{0};  ///< Response time
    QString error_message;                       ///< Error message if failed
    QJsonObject metadata;                        ///< Additional metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static NetworkResponse from_json(const QJsonObject& json);

    /**
     * @brief Get response data as string
     */
    QString data_as_string() const { return QString::fromUtf8(data); }

    /**
     * @brief Get response data as JSON
     */
    QJsonObject data_as_json() const;
};

/**
 * @brief Network progress callback
 */
using NetworkProgressCallback =
    std::function<void(qint64 bytes_received, qint64 bytes_total)>;

/**
 * @brief Network connection info
 */
struct NetworkConnectionInfo {
    QString connection_id;        ///< Connection identifier
    NetworkProtocol protocol;     ///< Connection protocol
    QHostAddress local_address;   ///< Local address
    quint16 local_port = 0;       ///< Local port
    QHostAddress remote_address;  ///< Remote address
    quint16 remote_port = 0;      ///< Remote port
    bool is_connected = false;    ///< Connection status
    std::chrono::system_clock::time_point
        established_time;       ///< Connection time
    qint64 bytes_sent = 0;      ///< Bytes sent
    qint64 bytes_received = 0;  ///< Bytes received

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Network plugin interface
 *
 * This interface extends the base plugin interface with network
 * communication capabilities for various protocols and operations.
 */
class INetworkPlugin : public virtual IPlugin {
public:
    ~INetworkPlugin() override = default;

    // === Network Operations ===

    /**
     * @brief Get supported network operations
     * @return Bitfield of supported operations
     */
    virtual NetworkOperations supported_operations() const noexcept = 0;

    /**
     * @brief Check if operation is supported
     * @param operation Operation to check
     * @return true if operation is supported
     */
    bool supports_operation(NetworkOperation operation) const noexcept {
        return (supported_operations() &
                static_cast<NetworkOperations>(operation)) != 0;
    }

    /**
     * @brief Execute network request synchronously
     * @param operation Network operation to perform
     * @param context Request context
     * @return Network response or error
     */
    virtual qtplugin::expected<NetworkResponse, PluginError> execute_request(
        NetworkOperation operation, const NetworkRequestContext& context) = 0;

    /**
     * @brief Execute network request asynchronously
     * @param operation Network operation to perform
     * @param context Request context
     * @param progress_callback Optional progress callback
     * @return Future with network response
     */
    virtual std::future<qtplugin::expected<NetworkResponse, PluginError>>
    execute_request_async(
        NetworkOperation operation, const NetworkRequestContext& context,
        NetworkProgressCallback progress_callback = nullptr) = 0;

    /**
     * @brief Execute batch requests
     * @param operation Network operation to perform
     * @param contexts Vector of request contexts
     * @return Vector of network responses
     */
    virtual qtplugin::expected<std::vector<NetworkResponse>, PluginError>
    execute_batch_requests(
        NetworkOperation operation,
        const std::vector<NetworkRequestContext>& contexts) = 0;

    // === Protocol Support ===

    /**
     * @brief Get supported network protocols
     * @return Vector of supported protocols
     */
    virtual std::vector<NetworkProtocol> supported_protocols() const = 0;

    /**
     * @brief Check if protocol is supported
     * @param protocol Protocol to check
     * @return true if protocol is supported
     */
    virtual bool supports_protocol(NetworkProtocol protocol) const {
        auto protocols = supported_protocols();
        return std::find(protocols.begin(), protocols.end(), protocol) !=
               protocols.end();
    }

    // === Connection Management ===

    /**
     * @brief Establish network connection
     * @param context Connection context
     * @return Connection info or error
     */
    virtual qtplugin::expected<NetworkConnectionInfo, PluginError>
    establish_connection(const NetworkRequestContext& context) = 0;

    /**
     * @brief Close network connection
     * @param connection_id Connection identifier
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> close_connection(
        const QString& connection_id) = 0;

    /**
     * @brief Get active connections
     * @return Vector of active connection info
     */
    virtual std::vector<NetworkConnectionInfo> get_active_connections()
        const = 0;

    /**
     * @brief Check connection status
     * @param connection_id Connection identifier
     * @return Connection info or error
     */
    virtual qtplugin::expected<NetworkConnectionInfo, PluginError>
    get_connection_info(const QString& connection_id) const = 0;

    // === SSL/TLS Support ===

    /**
     * @brief Configure SSL settings
     * @param ssl_config SSL configuration
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> configure_ssl(
        const QSslConfiguration& ssl_config) {
        Q_UNUSED(ssl_config)
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "SSL configuration not supported");
    }

    /**
     * @brief Get SSL configuration
     * @return Current SSL configuration
     */
    virtual std::optional<QSslConfiguration> get_ssl_configuration() const {
        return std::nullopt;
    }

    // === Authentication ===

    /**
     * @brief Set authentication credentials
     * @param auth_type Authentication type (e.g., "basic", "bearer", "oauth")
     * @param credentials Authentication credentials
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> set_authentication(
        const QString& auth_type, const QJsonObject& credentials) = 0;

    /**
     * @brief Clear authentication credentials
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> clear_authentication() = 0;

    // === Monitoring and Statistics ===

    /**
     * @brief Get network statistics
     * @return Network statistics as JSON object
     */
    virtual QJsonObject get_network_statistics() const { return QJsonObject{}; }

    /**
     * @brief Reset network statistics
     */
    virtual void reset_statistics() {}

    /**
     * @brief Get connection latency
     * @param target_url Target URL to test
     * @return Latency in milliseconds or error
     */
    virtual qtplugin::expected<std::chrono::milliseconds, PluginError>
    measure_latency(const QUrl& target_url) {
        Q_UNUSED(target_url)
        return make_error<std::chrono::milliseconds>(
            PluginErrorCode::CommandNotFound,
            "Latency measurement not supported");
    }

    // === Proxy Support ===

    /**
     * @brief Configure proxy settings
     * @param proxy_config Proxy configuration
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> configure_proxy(
        const QJsonObject& proxy_config) {
        Q_UNUSED(proxy_config)
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "Proxy configuration not supported");
    }

    /**
     * @brief Get proxy configuration
     * @return Current proxy configuration
     */
    virtual std::optional<QJsonObject> get_proxy_configuration() const {
        return std::nullopt;
    }
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::NetworkOperation)
Q_DECLARE_METATYPE(qtplugin::NetworkProtocol)
Q_DECLARE_METATYPE(qtplugin::NetworkRequestContext)
Q_DECLARE_METATYPE(qtplugin::NetworkResponse)
Q_DECLARE_METATYPE(qtplugin::NetworkConnectionInfo)

Q_DECLARE_INTERFACE(qtplugin::INetworkPlugin, "qtplugin.INetworkPlugin/3.0")
