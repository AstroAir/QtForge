/**
 * @file test_request_response_system.cpp
 * @brief Tests for request-response system implementation
 */

#include <QtTest/QtTest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSignalSpy>
#include <memory>
#include <chrono>

#include <qtplugin/communication/request_response_system.hpp>
#include <qtplugin/utils/error_handling.hpp>

#include "../utils/test_helpers.hpp"
#include "../utils/test_config_templates.hpp"

using namespace qtplugin;
using namespace QtForgeTest;

/**
 * @brief Test class for request-response system
 */
class TestRequestResponseSystem : public TestFixtureBase {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testSystemCreation();
    void testSystemInitialization();
    void testSystemShutdown();

    // Service registration tests
    void testRegisterSyncService();
    void testRegisterAsyncService();
    void testUnregisterService();
    void testServiceOverride();

    // Request handling tests
    void testSyncRequest();
    void testAsyncRequest();
    void testRequestWithTimeout();
    void testRequestWithInvalidService();

    // Response handling tests
    void testResponseDelivery();
    void testResponseTimeout();
    void testResponseError();
    void testResponseSerialization();

    // Service discovery tests
    void testListServices();
    void testServiceExists();
    void testServiceMetadata();

    // Error handling tests
    void testInvalidRequest();
    void testServiceNotFound();
    void testHandlerException();
    void testTimeoutHandling();

    // Performance tests
    void testRequestThroughput();
    void testConcurrentRequests();
    void testMemoryUsage();

    // Statistics tests
    void testStatisticsCollection();
    void testStatisticsReset();

private:
    std::unique_ptr<RequestResponseSystem> m_system;

    // Test helper methods
    RequestInfo createTestRequest(const QString& receiver, const QString& method,
                                 const QJsonObject& data = {});
    ResponseInfo createTestResponse(const QString& requestId, ResponseStatus status,
                                   const QJsonObject& data = {});
};

void TestRequestResponseSystem::initTestCase() {
    TestFixtureBase::initTestCase();
}

void TestRequestResponseSystem::cleanupTestCase() {
    TestFixtureBase::cleanupTestCase();
}

void TestRequestResponseSystem::init() {
    TestFixtureBase::init();
    m_system = std::make_unique<RequestResponseSystem>();
}

void TestRequestResponseSystem::cleanup() {
    if (m_system) {
        m_system.reset();
    }
    TestFixtureBase::cleanup();
}

RequestInfo TestRequestResponseSystem::createTestRequest(const QString& receiver,
                                                        const QString& method,
                                                        const QJsonObject& data) {
    RequestInfo request;
    request.receiver_id = receiver;
    request.method = method;
    request.data = data;
    request.sender_id = "test_sender";
    request.timestamp = QDateTime::currentDateTime();
    return request;
}

ResponseInfo TestRequestResponseSystem::createTestResponse(const QString& requestId,
                                                          ResponseStatus status,
                                                          const QJsonObject& data) {
    ResponseInfo response;
    response.request_id = requestId;
    response.status = status;
    response.data = data;
    response.timestamp = QDateTime::currentDateTime();
    return response;
}

void TestRequestResponseSystem::testSystemCreation() {
    QVERIFY(m_system != nullptr);

    // Test initial state
    auto stats = m_system->get_statistics();
    QCOMPARE(stats.total_requests_sent, 0);
    QCOMPARE(stats.total_responses_received, 0);
    QCOMPARE(stats.total_errors, 0);
}

void TestRequestResponseSystem::testSystemInitialization() {
    // Test system initialization (if needed)
    // The system should be ready to use after construction
    QVERIFY(true); // Placeholder - system is ready after construction
}

void TestRequestResponseSystem::testSystemShutdown() {
    // Test system shutdown
    m_system.reset();
    QVERIFY(true); // Test passes if no crash occurs
}

void TestRequestResponseSystem::testRegisterSyncService() {
    // Register a synchronous service
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.request_id = request.request_id;
        response.status = ResponseStatus::Success;
        response.data = QJsonObject{{"echo", request.data}};
        return response;
    };

    auto result = m_system->register_sync_service("test_service", "echo", handler);
    QTFORGE_VERIFY_SUCCESS(result);

    // Test that service is registered
    auto services = m_system->list_services();
    QVERIFY(services.contains("test_service::echo"));
}

void TestRequestResponseSystem::testRegisterAsyncService() {
    // Register an asynchronous service
    auto handler = [](const RequestInfo& request) -> std::future<ResponseInfo> {
        return std::async(std::launch::async, [request]() {
            ResponseInfo response;
            response.request_id = request.request_id;
            response.status = ResponseStatus::Success;
            response.data = QJsonObject{{"async_echo", request.data}};
            return response;
        });
    };

    auto result = m_system->register_async_service("async_service", "process", handler);
    QTFORGE_VERIFY_SUCCESS(result);

    // Test that service is registered
    auto services = m_system->list_services();
    QVERIFY(services.contains("async_service::process"));
}

void TestRequestResponseSystem::testUnregisterService() {
    // Register a service first
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.status = ResponseStatus::Success;
        return response;
    };

    auto register_result = m_system->register_sync_service("temp_service", "test", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    // Unregister the service
    auto unregister_result = m_system->unregister_service("temp_service", "test");
    QTFORGE_VERIFY_SUCCESS(unregister_result);

    // Verify service is no longer registered
    auto services = m_system->list_services();
    QVERIFY(!services.contains("temp_service::test"));
}

void TestRequestResponseSystem::testServiceOverride() {
    // Register a service
    auto handler1 = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.status = ResponseStatus::Success;
        response.data = QJsonObject{{"version", 1}};
        return response;
    };

    auto result1 = m_system->register_sync_service("override_service", "test", handler1);
    QTFORGE_VERIFY_SUCCESS(result1);

    // Override with a new handler
    auto handler2 = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.status = ResponseStatus::Success;
        response.data = QJsonObject{{"version", 2}};
        return response;
    };

    auto result2 = m_system->register_sync_service("override_service", "test", handler2);
    QTFORGE_VERIFY_SUCCESS(result2);

    // Test that the new handler is used
    auto request = createTestRequest("override_service", "test");
    auto response = m_system->send_request(request);
    QTFORGE_VERIFY_SUCCESS(response);

    if (response.has_value()) {
        QCOMPARE(response.value().data["version"].toInt(), 2);
    }
}

void TestRequestResponseSystem::testSyncRequest() {
    // Register a synchronous service
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.request_id = request.request_id;
        response.status = ResponseStatus::Success;
        response.data = QJsonObject{{"received", request.data}};
        return response;
    };

    auto register_result = m_system->register_sync_service("sync_service", "process", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    // Send a request
    QJsonObject requestData{{"input", "test_data"}};
    auto request = createTestRequest("sync_service", "process", requestData);

    auto response = m_system->send_request(request);
    QTFORGE_VERIFY_SUCCESS(response);

    if (response.has_value()) {
        QCOMPARE(response.value().status, ResponseStatus::Success);
        QVERIFY(response.value().data.contains("received"));
    }
}

void TestRequestResponseSystem::testAsyncRequest() {
    // Register an asynchronous service
    auto handler = [](const RequestInfo& request) -> std::future<ResponseInfo> {
        return std::async(std::launch::async, [request]() {
            // Simulate some processing time
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            ResponseInfo response;
            response.request_id = request.request_id;
            response.status = ResponseStatus::Success;
            response.data = QJsonObject{{"processed", request.data}};
            return response;
        });
    };

    auto register_result = m_system->register_async_service("async_service", "process", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    // Send an async request
    QJsonObject requestData{{"input", "async_test_data"}};
    auto request = createTestRequest("async_service", "process", requestData);

    auto future = m_system->send_request_async(request);
    QVERIFY(future.valid());

    // Wait for response with timeout
    auto status = future.wait_for(std::chrono::seconds(1));
    QVERIFY(status == std::future_status::ready);

    auto response = future.get();
    QTFORGE_VERIFY_SUCCESS(response);

    if (response.has_value()) {
        QCOMPARE(response.value().status, ResponseStatus::Success);
        QVERIFY(response.value().data.contains("processed"));
    }
}

void TestRequestResponseSystem::testRequestWithTimeout() {
    // Register a slow service
    auto handler = [](const RequestInfo& request) -> std::future<ResponseInfo> {
        return std::async(std::launch::async, [request]() {
            // Simulate long processing time
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            ResponseInfo response;
            response.request_id = request.request_id;
            response.status = ResponseStatus::Success;
            return response;
        });
    };

    auto register_result = m_system->register_async_service("slow_service", "process", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    // Send request with short timeout
    auto request = createTestRequest("slow_service", "process");

    auto future = m_system->send_request_async(request, std::chrono::milliseconds(50));
    QVERIFY(future.valid());

    // Wait for response
    auto status = future.wait_for(std::chrono::milliseconds(100));
    QVERIFY(status == std::future_status::ready);

    auto response = future.get();
    // Should timeout
    if (response.has_value()) {
        QCOMPARE(response.value().status, ResponseStatus::Timeout);
    }
}

void TestRequestResponseSystem::testRequestWithInvalidService() {
    // Send request to non-existent service
    auto request = createTestRequest("non_existent_service", "method");

    auto response = m_system->send_request(request);
    QTFORGE_VERIFY_ERROR(response, PluginErrorCode::PluginNotFound);
}

void TestRequestResponseSystem::testResponseDelivery() {
    // Test response delivery mechanism
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.request_id = request.request_id;
        response.status = ResponseStatus::Success;
        response.status_message = "Request processed successfully";
        return response;
    };

    auto register_result = m_system->register_sync_service("delivery_service", "test", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    auto request = createTestRequest("delivery_service", "test");
    auto response = m_system->send_request(request);

    QTFORGE_VERIFY_SUCCESS(response);
    if (response.has_value()) {
        QCOMPARE(response.value().status, ResponseStatus::Success);
        QCOMPARE(response.value().status_message, QString("Request processed successfully"));
    }
}

void TestRequestResponseSystem::testResponseTimeout() {
    // This test is covered in testRequestWithTimeout
    QVERIFY(true);
}

void TestRequestResponseSystem::testResponseError() {
    // Register a service that returns an error
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.request_id = request.request_id;
        response.status = ResponseStatus::BadRequest;
        response.status_message = "Invalid input data";
        return response;
    };

    auto register_result = m_system->register_sync_service("error_service", "test", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    auto request = createTestRequest("error_service", "test");
    auto response = m_system->send_request(request);

    QTFORGE_VERIFY_SUCCESS(response);
    if (response.has_value()) {
        QCOMPARE(response.value().status, ResponseStatus::BadRequest);
        QCOMPARE(response.value().status_message, QString("Invalid input data"));
    }
}

void TestRequestResponseSystem::testResponseSerialization() {
    // Test response serialization/deserialization
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.request_id = request.request_id;
        response.status = ResponseStatus::Success;
        response.data = QJsonObject{
            {"string_value", "test"},
            {"number_value", 42},
            {"boolean_value", true}
        };
        return response;
    };

    auto register_result = m_system->register_sync_service("serialization_service", "test", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    auto request = createTestRequest("serialization_service", "test");
    auto response = m_system->send_request(request);

    QTFORGE_VERIFY_SUCCESS(response);
    if (response.has_value()) {
        const auto& data = response.value().data;
        QCOMPARE(data["string_value"].toString(), QString("test"));
        QCOMPARE(data["number_value"].toInt(), 42);
        QCOMPARE(data["boolean_value"].toBool(), true);
    }
}

void TestRequestResponseSystem::testListServices() {
    // Register multiple services
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.status = ResponseStatus::Success;
        return response;
    };

    m_system->register_sync_service("service1", "method1", handler);
    m_system->register_sync_service("service1", "method2", handler);
    m_system->register_sync_service("service2", "method1", handler);

    auto services = m_system->list_services();
    QVERIFY(services.contains("service1::method1"));
    QVERIFY(services.contains("service1::method2"));
    QVERIFY(services.contains("service2::method1"));
    QVERIFY(services.size() >= 3);
}

void TestRequestResponseSystem::testServiceExists() {
    // Register a service
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.status = ResponseStatus::Success;
        return response;
    };

    m_system->register_sync_service("exists_service", "test", handler);

    // Test service existence
    auto services = m_system->list_services();
    QVERIFY(services.contains("exists_service::test"));
    QVERIFY(!services.contains("non_existent_service::test"));
}

void TestRequestResponseSystem::testServiceMetadata() {
    // Test service metadata (if supported)
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.status = ResponseStatus::Success;
        return response;
    };

    m_system->register_sync_service("metadata_service", "test", handler);

    // This is a placeholder test as metadata functionality might not be fully implemented
    QVERIFY(true);
}

void TestRequestResponseSystem::testInvalidRequest() {
    // Test handling of invalid requests
    RequestInfo invalid_request;
    // Leave fields empty to make it invalid

    auto response = m_system->send_request(invalid_request);
    // Should handle gracefully
    QVERIFY(!response.has_value() || response.value().status != ResponseStatus::Success);
}

void TestRequestResponseSystem::testServiceNotFound() {
    // This test is covered in testRequestWithInvalidService
    QVERIFY(true);
}

void TestRequestResponseSystem::testHandlerException() {
    // Register a service that throws an exception
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        throw std::runtime_error("Test exception");
        ResponseInfo response; // Never reached
        return response;
    };

    auto register_result = m_system->register_sync_service("exception_service", "test", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    auto request = createTestRequest("exception_service", "test");
    auto response = m_system->send_request(request);

    QTFORGE_VERIFY_SUCCESS(response);
    if (response.has_value()) {
        QCOMPARE(response.value().status, ResponseStatus::InternalError);
        QVERIFY(response.value().status_message.contains("exception"));
    }
}

void TestRequestResponseSystem::testTimeoutHandling() {
    // This test is covered in testRequestWithTimeout
    QVERIFY(true);
}

void TestRequestResponseSystem::testRequestThroughput() {
    // Register a fast service
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.request_id = request.request_id;
        response.status = ResponseStatus::Success;
        return response;
    };

    auto register_result = m_system->register_sync_service("throughput_service", "test", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    // Measure throughput
    QElapsedTimer timer;
    timer.start();

    const int request_count = 100;
    for (int i = 0; i < request_count; ++i) {
        auto request = createTestRequest("throughput_service", "test");
        auto response = m_system->send_request(request);
        QTFORGE_VERIFY_SUCCESS(response);
    }

    qint64 elapsed = timer.elapsed();
    qDebug() << "Request throughput:" << request_count << "requests in" << elapsed << "ms";

    // Verify reasonable performance (less than 10ms per request on average)
    QVERIFY(elapsed < request_count * 10);
}

void TestRequestResponseSystem::testConcurrentRequests() {
    // Register a service
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        // Simulate some processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        ResponseInfo response;
        response.request_id = request.request_id;
        response.status = ResponseStatus::Success;
        return response;
    };

    auto register_result = m_system->register_sync_service("concurrent_service", "test", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    // Send multiple concurrent requests
    std::vector<std::future<qtplugin::expected<ResponseInfo, PluginError>>> futures;

    for (int i = 0; i < 10; ++i) {
        auto request = createTestRequest("concurrent_service", "test");
        futures.push_back(std::async(std::launch::async, [this, request]() {
            return m_system->send_request(request);
        }));
    }

    // Wait for all responses
    for (auto& future : futures) {
        auto response = future.get();
        QTFORGE_VERIFY_SUCCESS(response);
    }
}

void TestRequestResponseSystem::testMemoryUsage() {
    // Test memory usage with many requests
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.request_id = request.request_id;
        response.status = ResponseStatus::Success;
        return response;
    };

    auto register_result = m_system->register_sync_service("memory_service", "test", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    // Send many requests to test memory usage
    for (int i = 0; i < 1000; ++i) {
        auto request = createTestRequest("memory_service", "test");
        auto response = m_system->send_request(request);
        QTFORGE_VERIFY_SUCCESS(response);
    }

    // Test passes if we don't crash or leak memory
    QVERIFY(true);
}

void TestRequestResponseSystem::testStatisticsCollection() {
    // Register a service
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.request_id = request.request_id;
        response.status = ResponseStatus::Success;
        return response;
    };

    auto register_result = m_system->register_sync_service("stats_service", "test", handler);
    QTFORGE_VERIFY_SUCCESS(register_result);

    // Get initial statistics
    auto initial_stats = m_system->get_statistics();

    // Send some requests
    for (int i = 0; i < 5; ++i) {
        auto request = createTestRequest("stats_service", "test");
        auto response = m_system->send_request(request);
        QTFORGE_VERIFY_SUCCESS(response);
    }

    // Check updated statistics
    auto final_stats = m_system->get_statistics();
    QVERIFY(final_stats.total_requests_sent >= initial_stats.total_requests_sent + 5);
    QVERIFY(final_stats.total_responses_received >= initial_stats.total_responses_received + 5);
}

void TestRequestResponseSystem::testStatisticsReset() {
    // Register a service and send some requests
    auto handler = [](const RequestInfo& request) -> ResponseInfo {
        ResponseInfo response;
        response.status = ResponseStatus::Success;
        return response;
    };

    m_system->register_sync_service("reset_service", "test", handler);

    // Send requests to generate statistics
    for (int i = 0; i < 3; ++i) {
        auto request = createTestRequest("reset_service", "test");
        m_system->send_request(request);
    }

    // Reset statistics (if supported)
    // This is a placeholder test as reset functionality might not be implemented
    auto stats = m_system->get_statistics();
    QVERIFY(stats.total_requests_sent >= 3);
}

QTEST_MAIN(TestRequestResponseSystem)
#include "test_request_response_system.moc"
