/**
 * @file request_response_service_impl.hpp
 * @brief Request-response service implementation for communication system
 * @version 3.0.0
 */

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <qtplugin/communication/factory.hpp>
#include <qtplugin/communication/interfaces.hpp>

namespace qtplugin::communication {

/**
 * @brief Implementation of IRequestResponseService
 */
class RequestResponseServiceImpl : public IRequestResponseService {
public:
    explicit RequestResponseServiceImpl(
        const qtplugin::communication::CommunicationConfig::
            RequestResponseConfig& config = {});
    ~RequestResponseServiceImpl() override;

    Result<void> register_service(std::string_view service_name,
                                  RequestHandler handler) override;

    Result<void> unregister_service(std::string_view service_name) override;

    Result<QJsonObject> call_service(
        std::string_view service_name, const QJsonObject& request,
        std::chrono::milliseconds timeout) override;

    std::future<Result<QJsonObject>> call_service_async(
        std::string_view service_name, const QJsonObject& request,
        std::chrono::milliseconds timeout) override;

    std::vector<std::string> list_services() const override;

private:
    struct PendingRequest {
        std::promise<Result<QJsonObject>> promise;
        std::chrono::steady_clock::time_point timeout;
    };

    mutable std::mutex services_mutex_;
    std::map<std::string, RequestHandler> services_;

    mutable std::mutex pending_mutex_;
    std::map<std::string, PendingRequest> pending_requests_;
    std::condition_variable pending_cv_;
    std::atomic<bool> shutdown_{false};

    qtplugin::communication::CommunicationConfig::RequestResponseConfig config_;

    void cleanup_expired_requests();
    std::string generate_request_id();
};

}  // namespace qtplugin::communication
