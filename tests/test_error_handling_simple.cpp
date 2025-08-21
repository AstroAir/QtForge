/**
 * @file test_error_handling_simple.cpp
 * @brief Simple error handling tests
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <memory>
#include <string>

#include <qtplugin/utils/error_handling.hpp>

using namespace qtplugin;

class TestErrorHandlingSimple : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // PluginError tests
    void testPluginErrorCreation();
    void testPluginErrorComparison();
    
    // PluginErrorCode tests
    void testErrorCodeToString();
    
    // Expected type tests
    void testExpectedSuccess();
    void testExpectedError();
    void testExpectedValueAccess();
    
    // Error creation utilities tests
    void testMakeSuccess();
    void testMakeError();
    
    // Error formatting tests
    void testErrorToString();

private:
    // Helper methods
    expected<int, PluginError> divide(int a, int b);
    expected<std::string, PluginError> process_string(const std::string& input);
    void verify_error(const PluginError& error, PluginErrorCode expected_code, const std::string& expected_message);
};

void TestErrorHandlingSimple::initTestCase()
{
    qDebug() << "Starting simple error handling tests";
}

void TestErrorHandlingSimple::cleanupTestCase()
{
    qDebug() << "Simple error handling tests completed";
}

void TestErrorHandlingSimple::testPluginErrorCreation()
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
}

void TestErrorHandlingSimple::testPluginErrorComparison()
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

void TestErrorHandlingSimple::testErrorCodeToString()
{
    QCOMPARE(error_code_to_string(PluginErrorCode::Success), "Success");
    QCOMPARE(error_code_to_string(PluginErrorCode::InvalidArgument), "InvalidArgument");
    QCOMPARE(error_code_to_string(PluginErrorCode::FileNotFound), "FileNotFound");
    QCOMPARE(error_code_to_string(PluginErrorCode::LoadFailed), "LoadFailed");
    QCOMPARE(error_code_to_string(PluginErrorCode::SecurityViolation), "SecurityViolation");
}

void TestErrorHandlingSimple::testExpectedSuccess()
{
    auto result = make_success<int>(42);
    QVERIFY(result.has_value());
    QVERIFY(!result.has_error());
    QCOMPARE(result.value(), 42);
    QCOMPARE(*result, 42);
}

void TestErrorHandlingSimple::testExpectedError()
{
    auto result = make_error<int>(PluginErrorCode::InvalidArgument, "Test error");
    QVERIFY(!result.has_value());
    QVERIFY(result.has_error());
    QCOMPARE(result.error().code, PluginErrorCode::InvalidArgument);
    QCOMPARE(result.error().message, "Test error");
}

void TestErrorHandlingSimple::testExpectedValueAccess()
{
    auto success_result = make_success<int>(100);
    auto error_result = make_error<int>(PluginErrorCode::InvalidArgument, "Error");
    
    QCOMPARE(success_result.value(), 100);
    QCOMPARE(success_result.value_or(0), 100);
    QCOMPARE(error_result.value_or(0), 0);
}

void TestErrorHandlingSimple::testMakeSuccess()
{
    auto result = make_success<std::string>("Hello World");
    QVERIFY(result.has_value());
    QCOMPARE(result.value(), "Hello World");
}

void TestErrorHandlingSimple::testMakeError()
{
    auto simple_error = make_error<std::string>(PluginErrorCode::FileNotFound, "File not found");
    QVERIFY(!simple_error.has_value());
    QCOMPARE(simple_error.error().code, PluginErrorCode::FileNotFound);
    QCOMPARE(simple_error.error().message, "File not found");
    
    auto detailed_error = make_error<std::string>(PluginErrorCode::FileNotFound, "File not found", "path/to/file");
    QVERIFY(!detailed_error.has_value());
    QCOMPARE(detailed_error.error().code, PluginErrorCode::FileNotFound);
    QCOMPARE(detailed_error.error().message, "File not found");
    QCOMPARE(detailed_error.error().details, "path/to/file");
}

void TestErrorHandlingSimple::testErrorToString()
{
    PluginError error(PluginErrorCode::InvalidArgument, "Test error message", "Additional details");
    
    std::string error_string = error.to_string();
    QVERIFY(error_string.find("Test error message") != std::string::npos);
    QVERIFY(error_string.find("Additional details") != std::string::npos);
}

expected<int, PluginError> TestErrorHandlingSimple::divide(int a, int b)
{
    if (b == 0) {
        return make_error<int>(PluginErrorCode::InvalidArgument, "Division by zero");
    }
    return make_success<int>(a / b);
}

expected<std::string, PluginError> TestErrorHandlingSimple::process_string(const std::string& input)
{
    if (input.empty()) {
        return make_error<std::string>(PluginErrorCode::InvalidArgument, "Empty string not allowed");
    }
    return make_success<std::string>("Processed: " + input);
}

void TestErrorHandlingSimple::verify_error(const PluginError& error, PluginErrorCode expected_code, const std::string& expected_message)
{
    QCOMPARE(error.code, expected_code);
    QCOMPARE(error.message, expected_message);
}

QTEST_MAIN(TestErrorHandlingSimple)
#include "test_error_handling_simple.moc"
