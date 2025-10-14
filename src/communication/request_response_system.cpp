#include "qtplugin/communication/request_response_system.hpp"
#include <QLoggingCategory>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QTimer>
#include <QUuid>
#include <QWaitCondition>
#include <chrono>
#include <future>
#include <queue>
#include <unordered_map>
#include <unordered_set>

Q_LOGGING_CATEGORY(rrsLog, "qtplugin.request_response")

namespace qtplugin {

struct PendingRequest {
    RequestInfo request;
    std::promise<qtplugin::expected<ResponseInfo, PluginError>> promise;
    QTimer* timeout_timer;
    std::chrono::steady_clock::time_point created_at;

    PendingRequest(const RequestInfo& req)
        : request(req),
          timeout_timer(nullptr),
          created_at(std::chrono::steady_clock::now()) {}
};

class RequestResponseSystem::Private {
public:
    // Service registry
    mutable QMutex services_mutex;
    std::unordered_map<QString, ServiceEndpoint> registered_services;
    std::unordered_map<QString, RequestHandler> sync_handlers;
    std::unordered_map<QString, AsyncRequestHandler> async_handlers;

    // Request tracking
    mutable QMutex requests_mutex;
    std::unordered_map<QString, std::unique_ptr<PendingRequest>>
        pending_requests;
    std::queue<QString> request_queue;

    // Statistics
    mutable QMutex stats_mutex;
    RequestResponseStatistics statistics;

    // Configuration
    std::chrono::milliseconds default_timeout{30000};

    // Processing
    QTimer* processing_timer;

    Private(QObject* parent) : processing_timer(new QTimer(parent)) {
        processing_timer->setSingleShot(false);
        processing_timer->setInterval(100);  // Process every 100ms
    }
};

RequestResponseSystem::RequestResponseSystem(QObject* parent)
    : QObject(parent), d(std::make_unique<Private>(this)) {
    connect(d->processing_timer, &QTimer::timeout, this,
            &RequestResponseSystem::process_pending_requests);
    d->processing_timer->start();

    qCDebug(rrsLog) << "RequestResponseSystem created with processing timer";
}

RequestResponseSystem::~RequestResponseSystem() {
    if (d && d->processing_timer) {
        d->processing_timer->stop();
    }

    // Cancel all pending requests
    QMutexLocker lock(&d->requests_mutex);
    for (auto& [id, pending] : d->pending_requests) {
        if (pending->timeout_timer) {
            pending->timeout_timer->stop();
            pending->timeout_timer->deleteLater();
        }
        pending->promise.set_value(
            make_error<ResponseInfo>(PluginErrorCode::SystemError,
                                     "RequestResponseSystem shutting down"));
    }
    d->pending_requests.clear();

    qCDebug(rrsLog) << "RequestResponseSystem destroyed";
}

qtplugin::expected<ResponseInfo, PluginError>
RequestResponseSystem::send_request(const RequestInfo& request) {
    // Generate unique request ID if not provided
    QString request_id =
        request.request_id.isEmpty()
            ? QUuid::createUuid().toString(QUuid::WithoutBraces)
            : request.request_id;

    // Find service handler
    QMutexLocker services_lock(&d->services_mutex);
    auto service_key = request.receiver_id + "::" + request.method;

    // Try sync handler first
    auto sync_it = d->sync_handlers.find(service_key);
    if (sync_it != d->sync_handlers.end()) {
        services_lock.unlock();

        try {
            auto response = sync_it->second(request);
            response.request_id = request_id;

            // Update statistics
            QMutexLocker stats_lock(&d->stats_mutex);
            d->statistics.total_requests_sent++;
            d->statistics.total_responses_received++;
            d->statistics.requests_by_method[request.method]++;
            d->statistics
                .responses_by_status[static_cast<int>(response.status)]++;

            return response;
        } catch (const std::exception& e) {
            ResponseInfo error_response;
            error_response.request_id = request_id;
            error_response.status = ResponseStatus::InternalError;
            error_response.status_message =
                QString("Handler exception: %1").arg(e.what());
            return error_response;
        }
    }

    // No handler found - return error
    QMutexLocker stats_lock(&d->stats_mutex);
    d->statistics.total_requests_sent++;
    d->statistics.total_errors++;

    return make_error<ResponseInfo>(
        PluginErrorCode::PluginNotFound,
        QString("No handler for service %1::%2")
            .arg(request.receiver_id, request.method)
            .toStdString());
}

qtplugin::expected<void, PluginError> RequestResponseSystem::register_service(
    const ServiceEndpoint& endpoint, RequestHandler handler) {
    if (endpoint.service_id.isEmpty() || endpoint.method.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration,
                                "Service ID and method cannot be empty");
    }

    QMutexLocker lock(&d->services_mutex);
    auto service_key = endpoint.service_id + "::" + endpoint.method;

    // Allow overriding existing services
    d->registered_services[service_key] = endpoint;
    d->sync_handlers[service_key] = std::move(handler);

    qCDebug(rrsLog) << "Registered sync service:" << service_key;
    emit service_registered(endpoint.service_id, endpoint.provider_id,
                            endpoint.method);

    return make_success();
}

qtplugin::expected<void, PluginError>
RequestResponseSystem::register_async_service(const ServiceEndpoint& endpoint,
                                              AsyncRequestHandler handler) {
    if (endpoint.service_id.isEmpty() || endpoint.method.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration,
                                "Service ID and method cannot be empty");
    }

    QMutexLocker lock(&d->services_mutex);
    auto service_key = endpoint.service_id + "::" + endpoint.method;

    // Allow overriding existing services
    d->registered_services[service_key] = endpoint;
    d->async_handlers[service_key] = std::move(handler);

    qCDebug(rrsLog) << "Registered async service:" << service_key;
    emit service_registered(endpoint.service_id, endpoint.provider_id,
                            endpoint.method);

    return make_success();
}

std::future<qtplugin::expected<ResponseInfo, PluginError>>
RequestResponseSystem::send_request_async(const RequestInfo& request) {
    // Generate unique request ID if not provided
    QString request_id =
        request.request_id.isEmpty()
            ? QUuid::createUuid().toString(QUuid::WithoutBraces)
            : request.request_id;

    // Create pending request
    auto pending = std::make_unique<PendingRequest>(request);
    auto future = pending->promise.get_future();

    // Setup timeout timer
    pending->timeout_timer = new QTimer(this);
    pending->timeout_timer->setSingleShot(true);
    pending->timeout_timer->setInterval(request.timeout.count());

    connect(pending->timeout_timer, &QTimer::timeout,
            [request_id, this]() { this->handle_request_timeout(request_id); });

    // Store pending request
    {
        QMutexLocker lock(&d->requests_mutex);
        d->pending_requests[request_id] = std::move(pending);
        d->request_queue.push(request_id);
    }

    // Start timeout timer
    d->pending_requests[request_id]->timeout_timer->start();

    qCDebug(rrsLog) << "Queued async request:" << request_id;

    return future;
}

void RequestResponseSystem::on_request_timeout() {
    // Legacy slot - process all timeouts
    QMutexLocker lock(&d->requests_mutex);
    auto now = std::chrono::steady_clock::now();

    std::vector<QString> timed_out_requests;
    for (const auto& [id, pending] : d->pending_requests) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - pending->created_at);
        if (elapsed >= pending->request.timeout) {
            timed_out_requests.push_back(id);
        }
    }

    for (const QString& id : timed_out_requests) {
        auto it = d->pending_requests.find(id);
        if (it != d->pending_requests.end()) {
            ResponseInfo timeout_response;
            timeout_response.request_id = id;
            timeout_response.status = ResponseStatus::Timeout;
            timeout_response.status_message = "Request timed out";

            it->second->promise.set_value(timeout_response);
            if (it->second->timeout_timer) {
                it->second->timeout_timer->deleteLater();
            }
            d->pending_requests.erase(it);

            // Update statistics
            QMutexLocker stats_lock(&d->stats_mutex);
            d->statistics.total_timeouts++;
        }
    }
}

void RequestResponseSystem::handle_request_timeout(const QString& request_id) {
    QMutexLocker lock(&d->requests_mutex);
    auto it = d->pending_requests.find(request_id);
    if (it != d->pending_requests.end()) {
        ResponseInfo timeout_response;
        timeout_response.request_id = request_id;
        timeout_response.status = ResponseStatus::Timeout;
        timeout_response.status_message = "Request timed out";

        it->second->promise.set_value(timeout_response);
        if (it->second->timeout_timer) {
            it->second->timeout_timer->deleteLater();
        }
        d->pending_requests.erase(it);

        // Update statistics
        QMutexLocker stats_lock(&d->stats_mutex);
        d->statistics.total_timeouts++;

        qCDebug(rrsLog) << "Request timed out:" << request_id;
    }
}

void RequestResponseSystem::process_pending_requests() {
    QMutexLocker lock(&d->requests_mutex);

    while (!d->request_queue.empty()) {
        QString request_id = d->request_queue.front();
        d->request_queue.pop();

        auto pending_it = d->pending_requests.find(request_id);
        if (pending_it == d->pending_requests.end()) {
            continue;  // Request was already processed or timed out
        }

        auto& pending = pending_it->second;
        const auto& request = pending->request;

        // Find async handler
        QMutexLocker services_lock(&d->services_mutex);
        auto service_key = request.receiver_id + "::" + request.method;
        auto async_it = d->async_handlers.find(service_key);

        if (async_it != d->async_handlers.end()) {
            services_lock.unlock();
            lock.unlock();

            try {
                // Execute async handler
                auto response_future = async_it->second(request);

                // Handle response in a separate thread to avoid blocking
                std::thread([this, request_id,
                             response_future =
                                 std::move(response_future)]() mutable {
                    try {
                        auto response = response_future.get();
                        response.request_id = request_id;

                        QMutexLocker req_lock(&d->requests_mutex);
                        auto it = d->pending_requests.find(request_id);
                        if (it != d->pending_requests.end()) {
                            it->second->promise.set_value(response);
                            if (it->second->timeout_timer) {
                                it->second->timeout_timer->stop();
                                it->second->timeout_timer->deleteLater();
                            }
                            d->pending_requests.erase(it);

                            // Update statistics
                            QMutexLocker stats_lock(&d->stats_mutex);
                            d->statistics.total_responses_received++;
                            d->statistics.responses_by_status[static_cast<int>(
                                response.status)]++;
                        }
                    } catch (const std::exception& e) {
                        QMutexLocker req_lock(&d->requests_mutex);
                        auto it = d->pending_requests.find(request_id);
                        if (it != d->pending_requests.end()) {
                            ResponseInfo error_response;
                            error_response.request_id = request_id;
                            error_response.status =
                                ResponseStatus::InternalError;
                            error_response.status_message =
                                QString("Async handler exception: %1")
                                    .arg(e.what());

                            it->second->promise.set_value(error_response);
                            if (it->second->timeout_timer) {
                                it->second->timeout_timer->stop();
                                it->second->timeout_timer->deleteLater();
                            }
                            d->pending_requests.erase(it);

                            QMutexLocker stats_lock(&d->stats_mutex);
                            d->statistics.total_errors++;
                        }
                    }
                }).detach();

            } catch (const std::exception& e) {
                lock.relock();
                ResponseInfo error_response;
                error_response.request_id = request_id;
                error_response.status = ResponseStatus::InternalError;
                error_response.status_message =
                    QString("Failed to start async handler: %1").arg(e.what());

                pending->promise.set_value(error_response);
                if (pending->timeout_timer) {
                    pending->timeout_timer->stop();
                    pending->timeout_timer->deleteLater();
                }
                d->pending_requests.erase(pending_it);

                QMutexLocker stats_lock(&d->stats_mutex);
                d->statistics.total_errors++;
            }

            lock.relock();
        } else {
            // No async handler found
            ResponseInfo not_found_response;
            not_found_response.request_id = request_id;
            not_found_response.status = ResponseStatus::NotFound;
            not_found_response.status_message =
                QString("No async handler for service %1::%2")
                    .arg(request.receiver_id, request.method);

            pending->promise.set_value(not_found_response);
            if (pending->timeout_timer) {
                pending->timeout_timer->stop();
                pending->timeout_timer->deleteLater();
            }
            d->pending_requests.erase(pending_it);

            QMutexLocker stats_lock(&d->stats_mutex);
            d->statistics.total_errors++;
        }
    }
}

RequestResponseStatistics RequestResponseSystem::get_statistics() const {
    QMutexLocker lock(&d->stats_mutex);
    return d->statistics;
}

void RequestResponseSystem::reset_statistics() {
    QMutexLocker lock(&d->stats_mutex);
    d->statistics = RequestResponseStatistics{};
}

void RequestResponseSystem::set_default_timeout(
    std::chrono::milliseconds timeout) {
    d->default_timeout = timeout;
}

std::chrono::milliseconds RequestResponseSystem::get_default_timeout() const {
    return d->default_timeout;
}

std::vector<ServiceEndpoint> RequestResponseSystem::get_registered_services(
    const QString& provider_id) const {
    QMutexLocker lock(&d->services_mutex);
    std::vector<ServiceEndpoint> result;

    for (const auto& [service_id, endpoint] : d->registered_services) {
        if (provider_id.isEmpty() || endpoint.provider_id == provider_id) {
            result.push_back(endpoint);
        }
    }

    return result;
}

bool RequestResponseSystem::is_service_registered(
    const QString& service_id) const {
    QMutexLocker lock(&d->services_mutex);
    // Check if any service with this service_id exists (may have multiple
    // methods)
    for (const auto& [key, endpoint] : d->registered_services) {
        if (endpoint.service_id == service_id) {
            return true;
        }
    }
    return false;
}

qtplugin::expected<void, PluginError> RequestResponseSystem::unregister_service(
    const QString& service_id) {
    QMutexLocker lock(&d->services_mutex);

    // Find all services with this service_id and remove them
    std::vector<QString> keys_to_remove;
    for (const auto& [key, endpoint] : d->registered_services) {
        if (endpoint.service_id == service_id) {
            keys_to_remove.push_back(key);
        }
    }

    if (keys_to_remove.empty()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Service not found: " + service_id.toStdString());
    }

    // Remove all handlers and registrations for this service
    for (const auto& key : keys_to_remove) {
        d->sync_handlers.erase(key);
        d->async_handlers.erase(key);
        d->registered_services.erase(key);
    }

    qCDebug(rrsLog) << "Unregistered service:" << service_id;
    emit service_unregistered(service_id);

    return make_success();
}

}  // namespace qtplugin
