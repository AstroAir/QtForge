/**
 * @file test_expected_comprehensive.cpp
 * @brief Comprehensive tests for expected<T, E> error handling system
 */

#include <QObject>
#include <QtTest/QtTest>
#include <memory>
#include <string>
#include <vector>

#include <qtplugin/qtplugin.hpp>

/**
 * @brief Test class for expected<T, E> functionality
 */
class TestExpected : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Basic functionality tests
    void testSuccessConstruction();
    void testErrorConstruction();
    void testCopyConstruction();
    void testMoveConstruction();
    void testAssignment();

    // Value access tests
    void testValueAccess();
    void testErrorAccess();
    void testHasValue();
    void testBoolConversion();

    // Utility function tests
    void testMakeSuccess();
    void testMakeError();
    void testUnexpected();

    // Type safety tests
    void testDifferentTypes();
    void testVoidType();
    void testComplexTypes();

    // Error handling patterns
    void testErrorPropagation();
    void testErrorChaining();
    void testErrorTransformation();

    // Performance tests
    void testPerformance();
    void testMemoryUsage();

    // Integration tests with QtPlugin
    void testPluginErrorIntegration();
    void testPluginManagerIntegration();

private:
    // Helper functions for testing
    qtplugin::expected<int, qtplugin::PluginError> divide(int a, int b);
    qtplugin::expected<std::string, qtplugin::PluginError> process_string(
        const std::string& input);
    qtplugin::expected<void, qtplugin::PluginError> validate_input(int value);
};

void TestExpected::initTestCase() {
    qDebug() << "Starting expected<T,E> tests";
}

void TestExpected::cleanupTestCase() {
    qDebug() << "expected<T,E> tests completed";
}

void TestExpected::testSuccessConstruction() {
    // Test construction with success value
    qtplugin::expected<int, qtplugin::PluginError> result(42);

    QVERIFY(result.has_value());
    QVERIFY(result);
    QCOMPARE(result.value(), 42);
    QCOMPARE(*result, 42);
}

void TestExpected::testErrorConstruction() {
    // Test construction with error
    qtplugin::expected<int, std::string> result =
        qtplugin::unexpected<std::string>("Error message");

    QVERIFY(!result.has_value());
    QVERIFY(!result);
    QCOMPARE(result.error(), "Error message");
}

void TestExpected::testCopyConstruction() {
    // Test copy construction with success
    qtplugin::expected<int, std::string> original(42);
    qtplugin::expected<int, std::string> copy(original);

    QVERIFY(copy.has_value());
    QCOMPARE(copy.value(), 42);

    // Test copy construction with error
    qtplugin::expected<int, std::string> error_original =
        qtplugin::unexpected<std::string>("Error");
    qtplugin::expected<int, std::string> error_copy(error_original);

    QVERIFY(!error_copy.has_value());
    QCOMPARE(error_copy.error(), "Error");
}

void TestExpected::testMoveConstruction() {
    // Test move construction with success
    qtplugin::expected<std::string, int> original("Hello");
    qtplugin::expected<std::string, int> moved(std::move(original));

    QVERIFY(moved.has_value());
    QCOMPARE(moved.value(), "Hello");

    // Test move construction with error
    qtplugin::expected<std::string, int> error_original =
        qtplugin::unexpected<int>(404);
    qtplugin::expected<std::string, int> error_moved(std::move(error_original));

    QVERIFY(!error_moved.has_value());
    QCOMPARE(error_moved.error(), 404);
}

void TestExpected::testAssignment() {
    qtplugin::expected<int, std::string> result(0);

    // Test assignment with success value
    result = 42;
    QVERIFY(result.has_value());
    QCOMPARE(result.value(), 42);

    // Test assignment with error
    result = qtplugin::unexpected<std::string>("Assignment error");
    QVERIFY(!result.has_value());
    QCOMPARE(result.error(), "Assignment error");

    // Test assignment from another expected
    qtplugin::expected<int, std::string> other(100);
    result = other;
    QVERIFY(result.has_value());
    QCOMPARE(result.value(), 100);
}

void TestExpected::testValueAccess() {
    qtplugin::expected<int, std::string> success(42);
    qtplugin::expected<int, std::string> error =
        qtplugin::unexpected<std::string>("Error");

    // Test value access for success case
    QCOMPARE(success.value(), 42);
    QCOMPARE(*success, 42);
    QCOMPARE(success.value_or(0), 42);

    // Test value access for error case
    QCOMPARE(error.value_or(0), 0);
    QCOMPARE(error.value_or(99), 99);

    // Test that accessing value on error throws (if implemented)
    // Note: This depends on the implementation of expected<T,E>
}

void TestExpected::testErrorAccess() {
    qtplugin::expected<int, std::string> success(42);
    qtplugin::expected<int, std::string> error =
        qtplugin::unexpected<std::string>("Test error");

    // Test error access for error case
    QCOMPARE(error.error(), "Test error");

    // Test that accessing error on success might throw or be undefined
    // This depends on the implementation
}

void TestExpected::testHasValue() {
    qtplugin::expected<int, std::string> success(42);
    qtplugin::expected<int, std::string> error =
        qtplugin::unexpected<std::string>("Error");

    QVERIFY(success.has_value());
    QVERIFY(!error.has_value());
}

void TestExpected::testBoolConversion() {
    qtplugin::expected<int, std::string> success(42);
    qtplugin::expected<int, std::string> error =
        qtplugin::unexpected<std::string>("Error");

    QVERIFY(success);
    QVERIFY(!error);

    // Test in conditional statements
    if (success) {
        QVERIFY(true);  // Should reach here
    } else {
        QFAIL("Success case should be true");
    }

    if (error) {
        QFAIL("Error case should be false");
    } else {
        QVERIFY(true);  // Should reach here
    }
}

void TestExpected::testMakeSuccess() {
    auto result = qtplugin::make_success<int>(42);
    QVERIFY(result.has_value());
    QCOMPARE(result.value(), 42);

    // Test void success
    auto void_result = qtplugin::make_success();
    QVERIFY(void_result.has_value());
}

void TestExpected::testMakeError() {
    auto result = qtplugin::make_error<int>(
        qtplugin::PluginErrorCode::UnknownError, "Test error");
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().message, "Test error");

    // Test with PluginError
    auto plugin_error_result = qtplugin::make_error<int>(
        qtplugin::PluginErrorCode::LoadFailed, "Plugin load failed");
    QVERIFY(!plugin_error_result.has_value());
    QCOMPARE(plugin_error_result.error().code,
             qtplugin::PluginErrorCode::LoadFailed);
    QCOMPARE(plugin_error_result.error().message, "Plugin load failed");
}

void TestExpected::testUnexpected() {
    auto unexpected_error =
        qtplugin::unexpected<std::string>("Unexpected error");
    qtplugin::expected<int, std::string> result = unexpected_error;

    QVERIFY(!result.has_value());
    QCOMPARE(result.error(), "Unexpected error");
}

void TestExpected::testDifferentTypes() {
    // Test with different value types
    qtplugin::expected<std::string, int> string_result("Hello");
    QVERIFY(string_result.has_value());
    QCOMPARE(string_result.value(), "Hello");

    qtplugin::expected<std::vector<int>, std::string> vector_result(
        std::vector<int>{1, 2, 3});
    QVERIFY(vector_result.has_value());
    QCOMPARE(vector_result.value().size(), 3);
    QCOMPARE(vector_result.value()[0], 1);

    // Test with custom types
    struct CustomType {
        int value;
        std::string name;
        bool operator==(const CustomType& other) const {
            return value == other.value && name == other.name;
        }
    };

    qtplugin::expected<CustomType, std::string> custom_result(
        CustomType{42, "test"});
    QVERIFY(custom_result.has_value());
    QCOMPARE(custom_result.value().value, 42);
    QCOMPARE(custom_result.value().name, "test");
}

void TestExpected::testVoidType() {
    // Test expected<void, E>
    qtplugin::expected<void, qtplugin::PluginError> success_void =
        qtplugin::make_success();
    QVERIFY(success_void.has_value());
    QVERIFY(success_void);

    qtplugin::expected<void, qtplugin::PluginError> error_void =
        qtplugin::make_error<void>(qtplugin::PluginErrorCode::UnknownError,
                                   "Void error");
    QVERIFY(!error_void.has_value());
    QVERIFY(!error_void);
    QCOMPARE(error_void.error().message, "Void error");
}

void TestExpected::testComplexTypes() {
    // Test with smart pointers
    auto ptr = std::make_unique<int>(42);
    qtplugin::expected<std::unique_ptr<int>, std::string> ptr_result(
        std::move(ptr));
    QVERIFY(ptr_result.has_value());
    QCOMPARE(*ptr_result.value(), 42);

    // Test with shared pointers
    auto shared_ptr = std::make_shared<std::string>("Hello");
    qtplugin::expected<std::shared_ptr<std::string>, int> shared_result(
        shared_ptr);
    QVERIFY(shared_result.has_value());
    QCOMPARE(*shared_result.value(), "Hello");
}

void TestExpected::testErrorPropagation() {
    // Test error propagation through function calls
    auto result1 = divide(10, 2);
    QVERIFY(result1.has_value());
    QCOMPARE(result1.value(), 5);

    auto result2 = divide(10, 0);
    QVERIFY(!result2.has_value());
    QVERIFY(result2.error().message.find("Division by zero") !=
            std::string::npos);
}

void TestExpected::testErrorChaining() {
    // Test chaining operations with expected
    auto result = process_string("valid_input");
    QVERIFY(result.has_value());

    auto error_result = process_string("invalid_input");
    QVERIFY(!error_result.has_value());
    QCOMPARE(error_result.error().code,
             qtplugin::PluginErrorCode::InvalidArgument);
}

void TestExpected::testErrorTransformation() {
    // Test transforming one error type to another
    auto string_error = qtplugin::make_error<int>(
        qtplugin::PluginErrorCode::UnknownError, "String error");

    // Transform to different PluginError
    qtplugin::expected<int, qtplugin::PluginError> plugin_error =
        qtplugin::make_error<int>(qtplugin::PluginErrorCode::ExecutionFailed,
                                  string_error.error().message);

    QVERIFY(!plugin_error.has_value());
    QCOMPARE(plugin_error.error().code,
             qtplugin::PluginErrorCode::ExecutionFailed);
    QCOMPARE(plugin_error.error().message, "String error");
}

void TestExpected::testPerformance() {
    const int iterations = 100000;

    // Test performance of success case
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < iterations; ++i) {
        auto result = qtplugin::make_success<int>(static_cast<int>(i));
        if (result.has_value()) {
            volatile int value = result.value();  // Prevent optimization
            Q_UNUSED(value)
        }
    }

    auto end = std::chrono::steady_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    qDebug() << "Success case performance:" << duration.count() << "μs for"
             << iterations << "iterations";
    qDebug() << "Average per operation:"
             << (static_cast<double>(duration.count()) / iterations) << "μs";

    // Test performance of error case
    start = std::chrono::steady_clock::now();

    for (int i = 0; i < iterations; ++i) {
        auto result = qtplugin::make_error<int>(
            qtplugin::PluginErrorCode::UnknownError, "Error");
        if (!result.has_value()) {
            volatile auto error = result.error();  // Prevent optimization
            Q_UNUSED(error)
        }
    }

    end = std::chrono::steady_clock::now();
    duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    qDebug() << "Error case performance:" << duration.count() << "μs for"
             << iterations << "iterations";
    qDebug() << "Average per operation:"
             << (static_cast<double>(duration.count()) / iterations) << "μs";
}

void TestExpected::testMemoryUsage() {
    // Test memory usage of expected<T,E>
    const int num_objects = 10000;
    std::vector<qtplugin::expected<std::string, qtplugin::PluginError>> objects;
    objects.reserve(num_objects);

    // Create many expected objects
    for (int i = 0; i < num_objects; ++i) {
        if (i % 2 == 0) {
            objects.emplace_back(std::string("Success ") + std::to_string(i));
        } else {
            objects.emplace_back(qtplugin::unexpected<qtplugin::PluginError>(
                qtplugin::PluginError{
                    qtplugin::PluginErrorCode::ExecutionFailed,
                    "Error " + std::to_string(i)}));
        }
    }

    // Verify objects are created correctly
    QCOMPARE(objects.size(), num_objects);

    int success_count = 0;
    int error_count = 0;

    for (const auto& obj : objects) {
        if (obj.has_value()) {
            success_count++;
        } else {
            error_count++;
        }
    }

    QCOMPARE(success_count, num_objects / 2);
    QCOMPARE(error_count, num_objects / 2);

    qDebug() << "Memory test completed with" << num_objects << "objects";
}

void TestExpected::testPluginErrorIntegration() {
    // Test integration with PluginError
    qtplugin::PluginError error{qtplugin::PluginErrorCode::LoadFailed,
                                "Plugin not found"};

    qtplugin::expected<std::string, qtplugin::PluginError> result =
        qtplugin::unexpected<qtplugin::PluginError>(error);

    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::LoadFailed);
    QCOMPARE(result.error().message, "Plugin not found");

    // Test with different error codes
    std::vector<qtplugin::PluginErrorCode> error_codes = {
        qtplugin::PluginErrorCode::Success,
        qtplugin::PluginErrorCode::LoadFailed,
        qtplugin::PluginErrorCode::InitializationFailed,
        qtplugin::PluginErrorCode::ConfigurationError,
        qtplugin::PluginErrorCode::CommandNotFound,
        qtplugin::PluginErrorCode::ExecutionFailed,
        qtplugin::PluginErrorCode::StateError,
        qtplugin::PluginErrorCode::SecurityViolation};

    for (auto code : error_codes) {
        auto error_result = qtplugin::make_error<int>(code, "Test error");
        QVERIFY(!error_result.has_value());
        QCOMPARE(error_result.error().code, code);
    }
}

void TestExpected::testPluginManagerIntegration() {
    // Test expected<T,E> integration with PluginManager
    // This test demonstrates how expected is used in the plugin system

    // Simulate plugin manager operations that return expected<T,E>
    auto simulate_load_success =
        []() -> qtplugin::expected<std::string, qtplugin::PluginError> {
        return std::string("com.test.plugin");
    };

    auto simulate_load_failure =
        []() -> qtplugin::expected<std::string, qtplugin::PluginError> {
        return qtplugin::make_error<std::string>(
            qtplugin::PluginErrorCode::LoadFailed, "Plugin file not found");
    };

    // Test successful operation
    auto success_result = simulate_load_success();
    QVERIFY(success_result.has_value());
    QCOMPARE(success_result.value(), "com.test.plugin");

    // Test failed operation
    auto failure_result = simulate_load_failure();
    QVERIFY(!failure_result.has_value());
    QCOMPARE(failure_result.error().code,
             qtplugin::PluginErrorCode::LoadFailed);
    QVERIFY(failure_result.error().message.find("not found") !=
            std::string::npos);
}

// Helper function implementations
qtplugin::expected<int, qtplugin::PluginError> TestExpected::divide(int a,
                                                                    int b) {
    if (b == 0) {
        return qtplugin::make_error<int>(
            qtplugin::PluginErrorCode::InvalidParameters, "Division by zero");
    }
    return a / b;
}

qtplugin::expected<std::string, qtplugin::PluginError>
TestExpected::process_string(const std::string& input) {
    if (input == "invalid_input") {
        return qtplugin::make_error<std::string>(
            qtplugin::PluginErrorCode::InvalidArgument,
            "Invalid input provided");
    }
    return "Processed: " + input;
}

qtplugin::expected<void, qtplugin::PluginError> TestExpected::validate_input(
    int value) {
    if (value < 0) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InvalidArgument,
            "Value must be non-negative");
    }
    return qtplugin::make_success();
}

#include "test_expected_comprehensive.moc"

QTEST_MAIN(TestExpected)
