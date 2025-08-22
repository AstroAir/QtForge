/**
 * @file request_response_system.hpp
 * @brief Request/response communication system for plugin interactions
 * @version 3.0.0
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUuid>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include "../core/plugin_interface.hpp"
#include "../utils/error_handling.hpp"

namespace qtplugin {

/**
 * @brief Request types
 */
enum class RequestType {
    Query,    ///< Query request (read-only)
    Command,  ///< Command request (may modify state)
    Event,    ///< Event notification request
    Stream,   ///< Streaming request
    Batch,    ///< Batch request
    Custom    ///< Custom request type
};

/**
 * @brief Response status codes
 */
enum class ResponseStatus {
    Success = 200,            ///< Request successful
    Accepted = 202,           ///< Request accepted (async processing)
    BadRequest = 400,         ///< Bad request
    Unauthorized = 401,       ///< Unauthorized
    Forbidden = 403,          ///< Forbidden
    NotFound = 404,           ///< Resource not found
    MethodNotAllowed = 405,   ///< Method not allowed
    Timeout = 408,            ///< Request timeout
    Conflict = 409,           ///< Conflict
    InternalError = 500,      ///< Internal server error
    NotImplemented = 501,     ///< Not implemented
    ServiceUnavailable = 503  ///< Service unavailable
};

/**
 * @brief Request priority levels
 */
enum class RequestPriority {
    Lowest = 0,
    Low = 25,
    Normal = 50,
    High = 75,
    Highest = 100,
    Critical = 125
};

/**
 * @brief Request information
 */
struct RequestInfo {
    QString request_id;                        ///< Unique request identifier
    QString sender_id;                         ///< Sender identifier
    QString receiver_id;                       ///< Receiver identifier
    QString method;                            ///< Request method/operation
    RequestType type;                          ///< Request type
    RequestPriority priority;                  ///< Request priority
    QJsonObject parameters;                    ///< Request parameters
    QJsonObject headers;                       ///< Request headers
    std::chrono::milliseconds timeout{30000};  ///< Request timeout
    std::chrono::system_clock::time_point timestamp;  ///< Request timestamp
    QJsonObject metadata;                             ///< Additional metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static RequestInfo from_json(const QJsonObject& json);
};

/**
 * @brief Response information
 */
struct ResponseInfo {
    QString request_id;      ///< Corresponding request identifier
    QString responder_id;    ///< Responder identifier
    ResponseStatus status;   ///< Response status
    QString status_message;  ///< Status message
    QJsonObject data;        ///< Response data
    QJsonObject headers;     ///< Response headers
    std::chrono::milliseconds processing_time{0};     ///< Processing time
    std::chrono::system_clock::time_point timestamp;  ///< Response timestamp
    QJsonObject metadata;                             ///< Additional metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static ResponseInfo from_json(const QJsonObject& json);

    /**
     * @brief Check if response indicates success
     */
    bool is_success() const {
        return static_cast<int>(status) >= 200 &&
               static_cast<int>(status) < 300;
    }

    /**
     * @brief Check if response indicates error
     */
    bool is_error() const { return static_cast<int>(status) >= 400; }
};

/**
 * @brief Request handler callback
 */
using RequestHandler = std::function<ResponseInfo(const RequestInfo&)>;

/**
 * @brief Async request handler callback
 */
using AsyncRequestHandler =
    std::function<std::future<ResponseInfo>(const RequestInfo&)>;

/**
 * @brief Request interceptor callback
 */
using RequestInterceptor =
    std::function<std::optional<RequestInfo>(const RequestInfo&)>;

/**
 * @brief Response interceptor callback
 */
using ResponseInterceptor =
    std::function<std::optional<ResponseInfo>(const ResponseInfo&)>;

/**
 * @brief Service endpoint information
 */
struct ServiceEndpoint {
    QString service_id;                   ///< Service identifier
    QString provider_id;                  ///< Service provider identifier
    QString method;                       ///< Service method
    QString description;                  ///< Service description
    QStringList supported_request_types;  ///< Supported request types
    QJsonObject method_schema;            ///< Method parameter schema
    QJsonObject response_schema;          ///< Response schema
    bool is_async = false;                ///< Whether service is asynchronous
    std::chrono::milliseconds default_timeout{30000};  ///< Default timeout
    RequestPriority min_priority =
        RequestPriority::Lowest;  ///< Minimum priority
    QJsonObject metadata;         ///< Service metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static ServiceEndpoint from_json(const QJsonObject& json);
};

/**
 * @brief Request/response statistics
 */
struct RequestResponseStatistics {
    uint64_t total_requests_sent = 0;       ///< Total requests sent
    uint64_t total_requests_received = 0;   ///< Total requests received
    uint64_t total_responses_sent = 0;      ///< Total responses sent
    uint64_t total_responses_received = 0;  ///< Total responses received
    uint64_t total_timeouts = 0;            ///< Total request timeouts
    uint64_t total_errors = 0;              ///< Total errors
    std::chrono::milliseconds average_response_time{
        0};  ///< Average response time
    std::unordered_map<QString, uint64_t>
        requests_by_method;  ///< Requests by method
    std::unordered_map<int, uint64_t>
        responses_by_status;  ///< Responses by status code

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Request/response communication system
 *
 * This class provides a request/response communication system for plugins
 * with service discovery, method routing, and async support.
 */
class RequestResponseSystem : public QObject {
    Q_OBJECT

public:
    explicit RequestResponseSystem(QObject* parent = nullptr);
    ~RequestResponseSystem() override;

    // === Service Registration ===

    /**
     * @brief Register service endpoint
     * @param endpoint Service endpoint information
     * @param handler Request handler
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> register_service(
        const ServiceEndpoint& endpoint, RequestHandler handler);

    /**
     * @brief Register async service endpoint
     * @param endpoint Service endpoint information
     * @param handler Async request handler
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> register_async_service(
        const ServiceEndpoint& endpoint, AsyncRequestHandler handler);

    /**
     * @brief Unregister service endpoint
     * @param service_id Service identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unregister_service(
        const QString& service_id);

    /**
     * @brief Check if service is registered
     * @param service_id Service identifier
     * @return true if service is registered
     */
    bool is_service_registered(const QString& service_id) const;

    /**
     * @brief Get registered services
     * @param provider_id Optional provider filter
     * @return Vector of service endpoints
     */
    std::vector<ServiceEndpoint> get_registered_services(
        const QString& provider_id = QString()) const;

    // === Request/Response Operations ===

    /**
     * @brief Send request synchronously
     * @param request Request information
     * @return Response or error
     */
    qtplugin::expected<ResponseInfo, PluginError> send_request(
        const RequestInfo& request);

    /**
     * @brief Send request asynchronously
     * @param request Request information
     * @return Future with response
     */
    std::future<qtplugin::expected<ResponseInfo, PluginError>>
    send_request_async(const RequestInfo& request);

    /**
     * @brief Send batch requests
     * @param requests Vector of requests
     * @return Vector of responses
     */
    std::vector<qtplugin::expected<ResponseInfo, PluginError>>
    send_batch_requests(const std::vector<RequestInfo>& requests);

    /**
     * @brief Create and send request
     * @param sender_id Sender identifier
     * @param receiver_id Receiver identifier
     * @param method Request method
     * @param parameters Request parameters
     * @param type Request type
     * @param priority Request priority
     * @param timeout Request timeout
     * @return Response or error
     */
    qtplugin::expected<ResponseInfo, PluginError> call_service(
        const QString& sender_id, const QString& receiver_id,
        const QString& method, const QJsonObject& parameters = {},
        RequestType type = RequestType::Query,
        RequestPriority priority = RequestPriority::Normal,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{30000});

    /**
     * @brief Create and send async request
     * @param sender_id Sender identifier
     * @param receiver_id Receiver identifier
     * @param method Request method
     * @param parameters Request parameters
     * @param type Request type
     * @param priority Request priority
     * @param timeout Request timeout
     * @return Future with response
     */
    std::future<qtplugin::expected<ResponseInfo, PluginError>>
    call_service_async(
        const QString& sender_id, const QString& receiver_id,
        const QString& method, const QJsonObject& parameters = {},
        RequestType type = RequestType::Query,
        RequestPriority priority = RequestPriority::Normal,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{30000});

    // === Service Discovery ===

    /**
     * @brief Discover services by method
     * @param method Method name
     * @return Vector of matching service endpoints
     */
    std::vector<ServiceEndpoint> discover_services_by_method(
        const QString& method) const;

    /**
     * @brief Discover services by provider
     * @param provider_id Provider identifier
     * @return Vector of service endpoints
     */
    std::vector<ServiceEndpoint> discover_services_by_provider(
        const QString& provider_id) const;

    /**
     * @brief Find best service for request
     * @param method Method name
     * @param request_type Request type
     * @param priority Request priority
     * @return Best matching service endpoint or error
     */
    qtplugin::expected<ServiceEndpoint, PluginError> find_best_service(
        const QString& method, RequestType request_type = RequestType::Query,
        RequestPriority priority = RequestPriority::Normal) const;

    // === Interceptors ===

    /**
     * @brief Add request interceptor
     * @param interceptor Request interceptor function
     * @return Interceptor ID for removal
     */
    QString add_request_interceptor(RequestInterceptor interceptor);

    /**
     * @brief Add response interceptor
     * @param interceptor Response interceptor function
     * @return Interceptor ID for removal
     */
    QString add_response_interceptor(ResponseInterceptor interceptor);

    /**
     * @brief Remove request interceptor
     * @param interceptor_id Interceptor identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> remove_request_interceptor(
        const QString& interceptor_id);

    /**
     * @brief Remove response interceptor
     * @param interceptor_id Interceptor identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> remove_response_interceptor(
        const QString& interceptor_id);

    // === Request Management ===

    /**
     * @brief Get pending requests
     * @param receiver_id Optional receiver filter
     * @return Vector of pending request IDs
     */
    std::vector<QString> get_pending_requests(
        const QString& receiver_id = QString()) const;

    /**
     * @brief Cancel request
     * @param request_id Request identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> cancel_request(
        const QString& request_id);

    /**
     * @brief Get request status
     * @param request_id Request identifier
     * @return Request status information
     */
    qtplugin::expected<QJsonObject, PluginError> get_request_status(
        const QString& request_id) const;

    // === Statistics and Monitoring ===

    /**
     * @brief Get request/response statistics
     * @return System statistics
     */
    RequestResponseStatistics get_statistics() const;

    /**
     * @brief Reset statistics
     */
    void reset_statistics();

    /**
     * @brief Get service health status
     * @param service_id Service identifier
     * @return Health status information
     */
    qtplugin::expected<QJsonObject, PluginError> get_service_health(
        const QString& service_id) const;

    // === Configuration ===

    /**
     * @brief Set default request timeout
     * @param timeout Default timeout
     */
    void set_default_timeout(std::chrono::milliseconds timeout);

    /**
     * @brief Get default request timeout
     * @return Default timeout
     */
    std::chrono::milliseconds get_default_timeout() const;

    /**
     * @brief Set maximum concurrent requests
     * @param max_requests Maximum concurrent requests
     */
    void set_max_concurrent_requests(int max_requests);

    /**
     * @brief Get maximum concurrent requests
     * @return Maximum concurrent requests
     */
    int get_max_concurrent_requests() const;

signals:
    /**
     * @brief Emitted when request is sent
     * @param request_id Request identifier
     * @param sender_id Sender identifier
     * @param receiver_id Receiver identifier
     * @param method Request method
     */
    void request_sent(const QString& request_id, const QString& sender_id,
                      const QString& receiver_id, const QString& method);

    /**
     * @brief Emitted when request is received
     * @param request_id Request identifier
     * @param sender_id Sender identifier
     * @param receiver_id Receiver identifier
     * @param method Request method
     */
    void request_received(const QString& request_id, const QString& sender_id,
                          const QString& receiver_id, const QString& method);

    /**
     * @brief Emitted when response is sent
     * @param request_id Request identifier
     * @param responder_id Responder identifier
     * @param status Response status
     */
    void response_sent(const QString& request_id, const QString& responder_id,
                       ResponseStatus status);

    /**
     * @brief Emitted when response is received
     * @param request_id Request identifier
     * @param responder_id Responder identifier
     * @param status Response status
     */
    void response_received(const QString& request_id,
                           const QString& responder_id, ResponseStatus status);

    /**
     * @brief Emitted when service is registered
     * @param service_id Service identifier
     * @param provider_id Provider identifier
     * @param method Service method
     */
    void service_registered(const QString& service_id,
                            const QString& provider_id, const QString& method);

    /**
     * @brief Emitted when service is unregistered
     * @param service_id Service identifier
     */
    void service_unregistered(const QString& service_id);

private slots:
    void on_request_timeout();
    void process_pending_requests();

private:
    void handle_request_timeout(const QString& request_id);

private:
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::RequestType)
Q_DECLARE_METATYPE(qtplugin::ResponseStatus)
Q_DECLARE_METATYPE(qtplugin::RequestPriority)
Q_DECLARE_METATYPE(qtplugin::RequestInfo)
Q_DECLARE_METATYPE(qtplugin::ResponseInfo)
Q_DECLARE_METATYPE(qtplugin::ServiceEndpoint)
Q_DECLARE_METATYPE(qtplugin::RequestResponseStatistics)
