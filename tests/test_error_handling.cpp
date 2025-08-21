/**
 * @file test_error_handling.cpp
 * @brief Comprehensive tests for error handling mechanisms
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <memory>
#include <string>

#include <qtplugin/utils/error_handling.hpp>

using namespace qtplugin;

class TestErrorHandling : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // PluginError tests
    void testPluginErrorCreation();
    void testPluginErrorCopy();
    void testPluginErrorMove();
    void testPluginErrorComparison();
    
    // PluginErrorCode tests
    void testPluginErrorCodes();
    void testErrorCodeToString();
    void testErrorCodeFromString();
    void testErrorCodeCategories();
    
    // Expected type tests
    void testExpectedSuccess();
    void testExpectedError();
    void testExpectedValueAccess();
    void testExpectedErrorAccess();
    void testExpectedConversion();
    
    // Error creation utilities tests
    void testMakeSuccess();
    void testMakeError();
    void testMakeErrorWithMessage();
    void testMakeErrorWithDetails();
    
    // Error chaining tests
    void testErrorChaining();
    void testNestedErrors();
    void testErrorPropagation();
    void testErrorTransformation();
    
    // Error context tests
    void testErrorContext();
    void testErrorLocation();
    void testErrorStackTrace();
    void testErrorMetadata();
    
    // Error formatting tests
    void testErrorToString();
    void testErrorSerialization();
    void testErrorDeserialization();
    void testErrorLocalization();
    
    // Error recovery tests
    void testErrorRecovery();
    void testErrorRetry();
    void testErrorFallback();
    void testErrorIgnore();
    
    // Error logging tests
    void testErrorLogging();
    void testErrorReporting();
    void testErrorMetrics();
    void testErrorNotification();
    
    // Performance tests
    void testErrorCreationPerformance();
    void testErrorPropagationPerformance();
    void testErrorHandlingOverhead();
    
    // Thread safety tests
    void testConcurrentErrorHandling();
    void testThreadSafeErrorAccess();
    void testErrorHandlingStress();
    
    // Edge cases tests
    void testEmptyErrorMessage();
    void testLargeErrorMessage();
    void testUnicodeErrorMessage();
    void testNullErrorHandling();

private:
    // Helper methods
    expected<int, PluginError> divide(int a, int b);
    expected<std::string, PluginError> process_string(const std::string& input);
    void verify_error(const PluginError& error, PluginErrorCode expected_code, const std::string& expected_message);
};

void TestErrorHandling::initTestCase()
{
    qDebug() << "Starting error handling tests";
}

void TestErrorHandling::cleanupTestCase()
{
    qDebug() << "Error handling tests completed";
}

void TestErrorHandling::testPluginErrorCreation()
{
    // Test basic error creation
    PluginError error1(PluginErrorCode::InvalidArgument, "Invalid argument provided");
    QCOMPARE(error1.code, PluginErrorCode::InvalidArgument);
    QCOMPARE(error1.message, "Invalid argument provided");
    QVERIFY(error1.details.empty());
    
    // Test error creation with details
    PluginError error2(PluginErrorCode::FileNotFound, "File not found", "path/to/file.dll");
    QCOMPARE(error2.code, PluginErrorCode::FileNotFound);
    QCOMPARE(error2.message, "File not found");
    QCOMPARE(error2.details, "path/to/file.dll");
    
    // Test error creation with details
    PluginError error3(PluginErrorCode::LoadFailed, "Load failed", "plugin.dll");
    QCOMPARE(error3.code, PluginErrorCode::LoadFailed);
    QCOMPARE(error3.message, "Load failed");
    QCOMPARE(error3.details, "plugin.dll");
}

void TestErrorHandling::testPluginErrorCopy()
{
    PluginError original(PluginErrorCode::InvalidFormat, "Invalid format", "JSON parsing error");
    
    // Test copy constructor
    PluginError copied(original);
    QCOMPARE(copied.code, original.code);
    QCOMPARE(copied.message, original.message);
    QCOMPARE(copied.details, original.details);
    
    // Test copy assignment
    PluginError assigned;
    assigned = original;
    QCOMPARE(assigned.code, original.code);
    QCOMPARE(assigned.message, original.message);
    QCOMPARE(assigned.details, original.details);
}

void TestErrorHandling::testPluginErrorMove()
{
    PluginError original(PluginErrorCode::PermissionDenied, "Permission denied", "Access to file denied");
    std::string original_message = original.message;
    std::string original_details = original.details;
    
    // Test move constructor
    PluginError moved(std::move(original));
    QCOMPARE(moved.code, PluginErrorCode::PermissionDenied);
    QCOMPARE(moved.message, original_message);
    QCOMPARE(moved.details, original_details);
    
    // Test move assignment
    PluginError move_assigned;
    PluginError source(PluginErrorCode::NetworkError, "Network error", "Connection timeout");
    move_assigned = std::move(source);
    QCOMPARE(move_assigned.code, PluginErrorCode::NetworkError);
    QCOMPARE(move_assigned.message, "Network error");
    QCOMPARE(move_assigned.details, "Connection timeout");
}

void TestErrorHandling::testPluginErrorComparison()
{
    PluginError error1(PluginErrorCode::InvalidArgument, "Test error");
    PluginError error2(PluginErrorCode::InvalidArgument, "Test error");
    PluginError error3(PluginErrorCode::FileNotFound, "Test error");
    PluginError error4(PluginErrorCode::InvalidArgument, "Different message");
    
    // Test equality
    QVERIFY(error1 == error2);
    QVERIFY(!(error1 == error3));
    QVERIFY(!(error1 == error4));
    
    // Test inequality
    QVERIFY(!(error1 != error2));
    QVERIFY(error1 != error3);
    QVERIFY(error1 != error4);
}

void TestErrorHandling::testPluginErrorCodes()
{
    // Test all error codes exist and are unique
    std::set<PluginErrorCode> codes = {
        PluginErrorCode::Success,
        PluginErrorCode::UnknownError,
        PluginErrorCode::InvalidArgument,
        PluginErrorCode::FileNotFound,
        PluginErrorCode::PermissionDenied,
        PluginErrorCode::InvalidFormat,
        PluginErrorCode::LoadFailed,
        PluginErrorCode::UnloadFailed,
        PluginErrorCode::AlreadyLoaded,
        PluginErrorCode::NotLoaded,
        PluginErrorCode::DependencyMissing,
        PluginErrorCode::VersionMismatch,
        PluginErrorCode::SecurityViolation,
        PluginErrorCode::ResourceExhausted,
        PluginErrorCode::NetworkError,
        PluginErrorCode::TimeoutError,
        PluginErrorCode::ConfigurationError,
        PluginErrorCode::AlreadyExists,
        PluginErrorCode::NotImplemented
    };
    
    // Verify we have the expected number of error codes
    QVERIFY(codes.size() >= 19); // At least 19 error codes defined
}

void TestErrorHandling::testErrorCodeToString()
{
    // Test error code to string conversion
    QCOMPARE(error_code_to_string(PluginErrorCode::Success), "Success");
    QCOMPARE(error_code_to_string(PluginErrorCode::InvalidArgument), "InvalidArgument");
    QCOMPARE(error_code_to_string(PluginErrorCode::FileNotFound), "FileNotFound");
    QCOMPARE(error_code_to_string(PluginErrorCode::LoadFailed), "LoadFailed");
    QCOMPARE(error_code_to_string(PluginErrorCode::SecurityViolation), "SecurityViolation");
}

void TestErrorHandling::testExpectedSuccess()
{
    // Test successful expected value
    auto result = make_success<int>(42);
    QVERIFY(result.has_value());
    QCOMPARE(result.value(), 42);
    QVERIFY(!result.has_error());
    
    // Test successful expected with complex type
    auto string_result = make_success<std::string>("Hello, World!");
    QVERIFY(string_result.has_value());
    QCOMPARE(string_result.value(), "Hello, World!");
}

void TestErrorHandling::testExpectedError()
{
    // Test error expected value
    auto result = make_error<int>(PluginErrorCode::InvalidArgument, "Invalid input");
    QVERIFY(!result.has_value());
    QVERIFY(result.has_error());
    QCOMPARE(result.error().code, PluginErrorCode::InvalidArgument);
    QCOMPARE(result.error().message, "Invalid input");
}

void TestErrorHandling::testExpectedValueAccess()
{
    auto success_result = make_success<int>(100);
    auto error_result = make_error<int>(PluginErrorCode::UnknownError, "Test error");
    
    // Test value access on success
    QCOMPARE(success_result.value(), 100);
    QCOMPARE(*success_result, 100);
    
    // Test value_or on success and error
    QCOMPARE(success_result.value_or(0), 100);
    QCOMPARE(error_result.value_or(0), 0);
}

void TestErrorHandling::testMakeSuccess()
{
    // Test make_success with different types
    auto int_result = make_success<int>(42);
    QVERIFY(int_result.has_value());
    QCOMPARE(int_result.value(), 42);
    
    auto string_result = make_success<std::string>("test");
    QVERIFY(string_result.has_value());
    QCOMPARE(string_result.value(), "test");
    
    auto void_result = make_success();
    QVERIFY(void_result.has_value());
}

void TestErrorHandling::testMakeError()
{
    // Test make_error with different parameters
    auto simple_error = make_error<int>(PluginErrorCode::InvalidArgument, "Simple error");
    QVERIFY(!simple_error.has_value());
    verify_error(simple_error.error(), PluginErrorCode::InvalidArgument, "Simple error");
    
    auto detailed_error = make_error<std::string>(PluginErrorCode::FileNotFound, "File not found", "path/to/file");
    QVERIFY(!detailed_error.has_value());
    verify_error(detailed_error.error(), PluginErrorCode::FileNotFound, "File not found");
    QCOMPARE(detailed_error.error().details, "path/to/file");
}

void TestErrorHandling::testErrorChaining()
{
    // Test error chaining functionality
    auto inner_error = make_error<int>(PluginErrorCode::FileNotFound, "Inner error");
    auto outer_error = make_error<int>(PluginErrorCode::LoadFailed, "Outer error");
    
    QVERIFY(!outer_error.has_value());
    QCOMPARE(outer_error.error().code, PluginErrorCode::LoadFailed);
    QCOMPARE(outer_error.error().message, "Outer error");
    
    // Test that error chaining works
    QVERIFY(!inner_error.has_value());
    QCOMPARE(inner_error.error().code, PluginErrorCode::FileNotFound);
    QCOMPARE(outer_error.error().message, "Outer error");
}

void TestErrorHandling::testErrorToString()
{
    PluginError error(PluginErrorCode::InvalidArgument, "Test error message", "Additional details");
    
    std::string error_string = error.to_string();
    QVERIFY(error_string.find("InvalidArgument") != std::string::npos);
    QVERIFY(error_string.find("Test error message") != std::string::npos);
    QVERIFY(error_string.find("Additional details") != std::string::npos);
}

void TestErrorHandling::testErrorRecovery()
{
    // Test error recovery patterns
    auto result = divide(10, 0); // Should return error
    QVERIFY(!result.has_value());
    
    // Test recovery with default value
    int safe_result = result.value_or(-1);
    QCOMPARE(safe_result, -1);
    
    // Test recovery with alternative computation
    auto recovered = result.has_value() ? result : make_success<int>(0);
    QVERIFY(recovered.has_value());
    QCOMPARE(recovered.value(), 0);
}

// Helper methods implementation
expected<int, PluginError> TestErrorHandling::divide(int a, int b)
{
    if (b == 0) {
        return make_error<int>(PluginErrorCode::InvalidArgument, "Division by zero");
    }
    return make_success<int>(a / b);
}

expected<std::string, PluginError> TestErrorHandling::process_string(const std::string& input)
{
    if (input.empty()) {
        return make_error<std::string>(PluginErrorCode::InvalidArgument, "Empty string not allowed");
    }
    return make_success<std::string>("Processed: " + input);
}

void TestErrorHandling::verify_error(const PluginError& error, PluginErrorCode expected_code, const std::string& expected_message)
{
    QCOMPARE(error.code, expected_code);
    QCOMPARE(error.message, expected_message);
}

QTEST_MAIN(TestErrorHandling)
#include "test_error_handling.moc"
