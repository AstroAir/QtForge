/**
 * @file request_response_service_impl.cpp
 * @brief Request-response service implementation for communication system
 * @version 3.0.0
 */

#include "request_response_service_impl.hpp"
#include <atomic>
#include <random>
#include <sstream>

namespace qtplugin::communication {

RequestResponseServiceImpl::RequestResponseServiceImpl(
    const qtplugin::communication::CommunicationConfig::RequestResponseConfig&
        config)
    : config_(config) {}

RequestResponseServiceImpl::~RequestResponseServiceImpl() {
    shutdown_ = true;
    pending_cv_.notify_all();
}

Result<void> RequestResponseServiceImpl::register_service(
    std::string_view service_name, RequestHandler handler) {
    std::lock_guard<std::mutex> lock(services_mutex_);

    std::string name(service_name);
    if (services_.find(name) != services_.end()) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::SystemError, "Service already registered",
            "Service name: " + name});
    }

    services_[name] = std::move(handler);
    return {};
}

Result<void> RequestResponseServiceImpl::unregister_service(
    std::string_view service_name) {
    std::lock_guard<std::mutex> lock(services_mutex_);

    std::string name(service_name);
    auto it = services_.find(name);
    if (it == services_.end()) {
        return qtplugin::unexpected(
            CommunicationError{CommunicationError::Type::SystemError,
                               "Service not found", "Service name: " + name});
    }

    services_.erase(it);
    return {};
}

Result<QJsonObject> RequestResponseServiceImpl::call_service(
    std::string_view service_name, const QJsonObject& request,
    std::chrono::milliseconds timeout) {
    std::lock_guard<std::mutex> lock(services_mutex_);

    std::string name(service_name);
    auto it = services_.find(name);
    if (it == services_.end()) {
        return qtplugin::unexpected(
            CommunicationError{CommunicationError::Type::SystemError,
                               "Service not found", "Service name: " + name});
    }

    try {
        // Direct synchronous call to the handler
        QJsonObject response = it->second(request);
        return response;
    } catch (const std::exception& e) {
        return qtplugin::unexpected(
            CommunicationError{CommunicationError::Type::SystemError,
                               "Service handler error", e.what()});
    }
}

std::future<Result<QJsonObject>> RequestResponseServiceImpl::call_service_async(
    std::string_view service_name, const QJsonObject& request,
    std::chrono::milliseconds timeout) {
    auto promise = std::make_shared<std::promise<Result<QJsonObject>>>();
    auto future = promise->get_future();

    // For simplicity, we'll execute this synchronously in a thread
    // In a real implementation, this would be truly asynchronous
    std::thread([this, service_name = std::string(service_name), request,
                 timeout, promise]() {
        auto result = this->call_service(service_name, request, timeout);
        promise->set_value(result);
    }).detach();

    return future;
}

std::vector<std::string> RequestResponseServiceImpl::list_services() const {
    std::lock_guard<std::mutex> lock(services_mutex_);

    std::vector<std::string> services;
    services.reserve(services_.size());

    for (const auto& [name, handler] : services_) {
        services.push_back(name);
    }

    return services;
}

void RequestResponseServiceImpl::cleanup_expired_requests() {
    std::lock_guard<std::mutex> lock(pending_mutex_);

    auto now = std::chrono::steady_clock::now();
    auto it = pending_requests_.begin();

    while (it != pending_requests_.end()) {
        if (now >= it->second.timeout) {
            it->second.promise.set_value(
                qtplugin::unexpected(CommunicationError{
                    CommunicationError::Type::TimeoutExpired, "Request timeout",
                    "Request ID: " + it->first}));
            it = pending_requests_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string RequestResponseServiceImpl::generate_request_id() {
    static std::atomic<uint64_t> counter{0};
    static thread_local std::mt19937 generator(std::random_device{}());
    static thread_local std::uniform_int_distribution<uint32_t> distribution(
        0, 999999);

    std::ostringstream oss;
    oss << "req_" << counter++ << "_" << distribution(generator);
    return oss.str();
}

}  // namespace qtplugin::communication
