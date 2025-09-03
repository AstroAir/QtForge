# Error Handling

QtForge provides a comprehensive error handling system based on the `expected<T, Error>` pattern, enabling robust error management throughout the framework.

## Overview

The error handling system in QtForge is designed to:
- Provide type-safe error handling without exceptions
- Enable composable error handling patterns
- Support detailed error information and context
- Facilitate error propagation and transformation
- Ensure consistent error handling across the framework

## Core Error Types

### Error Class

The base `Error` class provides comprehensive error information:

```cpp
namespace qtforge {

class Error {
public:
    // Constructors
    Error() = default;
    explicit Error(const std::string& message);
    Error(const std::string& message, int code);
    Error(const std::string& message, int code, const std::string& category);
    
    // Error information
    const std::string& message() const;
    int code() const;
    const std::string& category() const;
    const std::string& stackTrace() const;
    
    // Error context
    void addContext(const std::string& key, const std::string& value);
    std::string getContext(const std::string& key) const;
    const std::map<std::string, std::string>& getAllContext() const;
    
    // Error chaining
    void setCause(const Error& cause);
    const Error* getCause() const;
    bool hasCause() const;
    
    // Formatting
    std::string toString() const;
    std::string toDetailedString() const;
    
private:
    std::string message_;
    int code_ = 0;
    std::string category_ = "General";
    std::string stackTrace_;
    std::map<std::string, std::string> context_;
    std::unique_ptr<Error> cause_;
};

} // namespace qtforge
```

### Expected Template

The `expected<T, Error>` template provides monadic error handling:

```cpp
namespace qtforge {

template<typename T, typename E = Error>
class expected {
public:
    // Constructors
    expected(const T& value);
    expected(T&& value);
    expected(const E& error);
    expected(E&& error);
    
    // Value access
    bool has_value() const;
    explicit operator bool() const;
    
    const T& value() const;
    T& value();
    const T& operator*() const;
    T& operator*();
    const T* operator->() const;
    T* operator->();
    
    // Error access
    const E& error() const;
    E& error();
    
    // Monadic operations
    template<typename F>
    auto map(F&& f) -> expected<std::invoke_result_t<F, T>, E>;
    
    template<typename F>
    auto and_then(F&& f) -> std::invoke_result_t<F, T>;
    
    template<typename F>
    auto or_else(F&& f) -> expected<T, std::invoke_result_t<F, E>>;
    
    template<typename F>
    auto transform_error(F&& f) -> expected<T, std::invoke_result_t<F, E>>;
    
    // Value extraction
    T value_or(const T& default_value) const;
    
private:
    std::variant<T, E> data_;
};

} // namespace qtforge
```

## Error Categories

### System Errors

```cpp
namespace qtforge::errors {

class SystemError : public Error {
public:
    SystemError(const std::string& message, int systemCode);
    
    int getSystemCode() const;
    std::string getSystemMessage() const;
    
private:
    int systemCode_;
};

// Factory functions
SystemError makeFileError(const std::string& filename, int errorCode);
SystemError makeNetworkError(const std::string& operation, int errorCode);
SystemError makeMemoryError(const std::string& operation);

} // namespace qtforge::errors
```

### Plugin Errors

```cpp
namespace qtforge::errors {

enum class PluginErrorCode {
    LoadFailed = 1000,
    InitializationFailed,
    ActivationFailed,
    DeactivationFailed,
    DependencyNotFound,
    VersionIncompatible,
    SecurityViolation,
    ConfigurationError
};

class PluginError : public Error {
public:
    PluginError(PluginErrorCode code, const std::string& pluginName, const std::string& message);
    
    PluginErrorCode getPluginErrorCode() const;
    const std::string& getPluginName() const;
    
private:
    PluginErrorCode pluginErrorCode_;
    std::string pluginName_;
};

// Factory functions
PluginError makeLoadError(const std::string& pluginName, const std::string& reason);
PluginError makeDependencyError(const std::string& pluginName, const std::string& dependency);
PluginError makeVersionError(const std::string& pluginName, const std::string& required, const std::string& actual);

} // namespace qtforge::errors
```

### Communication Errors

```cpp
namespace qtforge::errors {

enum class CommunicationErrorCode {
    MessageDeliveryFailed = 2000,
    SerializationFailed,
    DeserializationFailed,
    TimeoutExpired,
    ChannelClosed,
    InvalidMessage,
    PermissionDenied
};

class CommunicationError : public Error {
public:
    CommunicationError(CommunicationErrorCode code, const std::string& message);
    CommunicationError(CommunicationErrorCode code, const std::string& channel, const std::string& message);
    
    CommunicationErrorCode getCommunicationErrorCode() const;
    const std::string& getChannel() const;
    
private:
    CommunicationErrorCode communicationErrorCode_;
    std::string channel_;
};

} // namespace qtforge::errors
```

## Error Handling Patterns

### Basic Error Handling

```cpp
// Function that can fail
qtforge::expected<std::string, qtforge::Error> readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return qtforge::Error("Failed to open file: " + filename);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    return content;
}

// Usage with explicit error checking
void processFile(const std::string& filename) {
    auto result = readFile(filename);
    if (result) {
        // Success case
        std::string content = result.value();
        processContent(content);
    } else {
        // Error case
        qtforge::Error error = result.error();
        qtforge::Logger::error("FileProcessor", "Failed to read file: " + error.message());
    }
}
```

### Monadic Error Handling

```cpp
// Chain operations with automatic error propagation
qtforge::expected<ProcessedData, qtforge::Error> processFileMonadic(const std::string& filename) {
    return readFile(filename)
        .and_then([](const std::string& content) {
            return parseContent(content);
        })
        .and_then([](const ParsedData& parsed) {
            return validateData(parsed);
        })
        .and_then([](const ValidatedData& validated) {
            return transformData(validated);
        });
}

// Usage
void handleFile(const std::string& filename) {
    auto result = processFileMonadic(filename);
    if (result) {
        saveProcessedData(result.value());
    } else {
        handleError(result.error());
    }
}
```

### Error Transformation

```cpp
// Transform errors to add context
qtforge::expected<DatabaseRecord, qtforge::Error> getUser(int userId) {
    return database_.query("SELECT * FROM users WHERE id = ?", userId)
        .transform_error([userId](const qtforge::Error& dbError) {
            qtforge::Error userError("Failed to retrieve user");
            userError.addContext("user_id", std::to_string(userId));
            userError.addContext("operation", "get_user");
            userError.setCause(dbError);
            return userError;
        });
}
```

### Error Recovery

```cpp
// Provide fallback values
std::string getConfigValue(const std::string& key) {
    return configManager_.getValue(key)
        .or_else([key](const qtforge::Error& error) -> qtforge::expected<std::string, qtforge::Error> {
            qtforge::Logger::warning("Config", "Using default for " + key + ": " + error.message());
            return getDefaultValue(key);
        })
        .value_or(""); // Final fallback
}

// Retry on failure
template<typename F>
auto retryOnFailure(F&& operation, int maxAttempts = 3) {
    using ReturnType = std::invoke_result_t<F>;
    
    for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
        auto result = operation();
        if (result) {
            return result;
        }
        
        if (attempt < maxAttempts) {
            qtforge::Logger::warning("Retry", 
                "Attempt " + std::to_string(attempt) + " failed: " + result.error().message());
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * attempt));
        }
    }
    
    return operation(); // Final attempt
}
```

## Error Context and Debugging

### Adding Context

```cpp
qtforge::expected<void, qtforge::Error> processUserData(int userId, const UserData& data) {
    qtforge::Error error("User data processing failed");
    error.addContext("user_id", std::to_string(userId));
    error.addContext("operation", "process_user_data");
    error.addContext("timestamp", getCurrentTimestamp());
    
    // Validate data
    auto validation = validateUserData(data);
    if (!validation) {
        error.addContext("validation_stage", "input_validation");
        error.setCause(validation.error());
        return error;
    }
    
    // Process data
    auto processing = processData(validation.value());
    if (!processing) {
        error.addContext("processing_stage", "data_transformation");
        error.setCause(processing.error());
        return error;
    }
    
    // Save results
    auto saving = saveResults(processing.value());
    if (!saving) {
        error.addContext("saving_stage", "database_write");
        error.setCause(saving.error());
        return error;
    }
    
    return {};
}
```

### Error Logging

```cpp
class ErrorLogger {
public:
    static void logError(const qtforge::Error& error, const std::string& component = "") {
        std::ostringstream oss;
        oss << "[ERROR]";
        
        if (!component.empty()) {
            oss << " [" << component << "]";
        }
        
        oss << " " << error.message();
        
        if (error.code() != 0) {
            oss << " (Code: " << error.code() << ")";
        }
        
        // Add context information
        const auto& context = error.getAllContext();
        if (!context.empty()) {
            oss << " Context: {";
            bool first = true;
            for (const auto& [key, value] : context) {
                if (!first) oss << ", ";
                oss << key << "=" << value;
                first = false;
            }
            oss << "}";
        }
        
        qtforge::Logger::error("ErrorHandler", oss.str());
        
        // Log cause chain
        const qtforge::Error* cause = error.getCause();
        int depth = 1;
        while (cause && depth <= 5) { // Limit depth to prevent infinite loops
            qtforge::Logger::error("ErrorHandler", 
                std::string(depth * 2, ' ') + "Caused by: " + cause->message());
            cause = cause->getCause();
            ++depth;
        }
    }
    
    static void logWarning(const qtforge::Error& error, const std::string& component = "") {
        qtforge::Logger::warning(component.empty() ? "ErrorHandler" : component, error.message());
    }
};
```

## Error Handling Utilities

### Result Aggregation

```cpp
template<typename T>
class ResultAggregator {
public:
    void add(qtforge::expected<T, qtforge::Error> result) {
        if (result) {
            successes_.push_back(result.value());
        } else {
            errors_.push_back(result.error());
        }
    }
    
    bool hasErrors() const { return !errors_.empty(); }
    bool hasSuccesses() const { return !successes_.empty(); }
    
    const std::vector<T>& getSuccesses() const { return successes_; }
    const std::vector<qtforge::Error>& getErrors() const { return errors_; }
    
    qtforge::Error getCombinedError() const {
        if (errors_.empty()) {
            return qtforge::Error("No errors to combine");
        }
        
        qtforge::Error combined("Multiple errors occurred (" + std::to_string(errors_.size()) + " total)");
        
        for (size_t i = 0; i < errors_.size(); ++i) {
            combined.addContext("error_" + std::to_string(i), errors_[i].message());
        }
        
        return combined;
    }

private:
    std::vector<T> successes_;
    std::vector<qtforge::Error> errors_;
};

// Usage
ResultAggregator<ProcessedData> aggregator;

for (const auto& input : inputs) {
    aggregator.add(processInput(input));
}

if (aggregator.hasErrors()) {
    ErrorLogger::logError(aggregator.getCombinedError(), "BatchProcessor");
}

if (aggregator.hasSuccesses()) {
    saveResults(aggregator.getSuccesses());
}
```

### Error Boundaries

```cpp
template<typename F>
auto withErrorBoundary(F&& operation, const std::string& operationName) {
    using ReturnType = std::invoke_result_t<F>;
    
    try {
        return operation();
    } catch (const std::bad_alloc& e) {
        qtforge::Error error("Memory allocation failed in " + operationName);
        error.addContext("exception_type", "std::bad_alloc");
        error.addContext("what", e.what());
        
        if constexpr (std::is_same_v<ReturnType, void>) {
            ErrorLogger::logError(error, "ErrorBoundary");
        } else {
            return ReturnType{error};
        }
    } catch (const std::runtime_error& e) {
        qtforge::Error error("Runtime error in " + operationName);
        error.addContext("exception_type", "std::runtime_error");
        error.addContext("what", e.what());
        
        if constexpr (std::is_same_v<ReturnType, void>) {
            ErrorLogger::logError(error, "ErrorBoundary");
        } else {
            return ReturnType{error};
        }
    } catch (const std::exception& e) {
        qtforge::Error error("Exception in " + operationName);
        error.addContext("exception_type", typeid(e).name());
        error.addContext("what", e.what());
        
        if constexpr (std::is_same_v<ReturnType, void>) {
            ErrorLogger::logError(error, "ErrorBoundary");
        } else {
            return ReturnType{error};
        }
    } catch (...) {
        qtforge::Error error("Unknown exception in " + operationName);
        error.addContext("exception_type", "unknown");
        
        if constexpr (std::is_same_v<ReturnType, void>) {
            ErrorLogger::logError(error, "ErrorBoundary");
        } else {
            return ReturnType{error};
        }
    }
}

// Usage
auto result = withErrorBoundary([&]() {
    return riskyOperation(data);
}, "riskyOperation");
```

## Testing Error Handling

### Error Injection

```cpp
class ErrorInjector {
public:
    static void setFailureRate(const std::string& operation, double rate) {
        failureRates_[operation] = rate;
    }
    
    static bool shouldFail(const std::string& operation) {
        auto it = failureRates_.find(operation);
        if (it == failureRates_.end()) {
            return false;
        }
        
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        return dis(gen) < it->second;
    }
    
    static qtforge::Error makeInjectedError(const std::string& operation) {
        qtforge::Error error("Injected failure for testing");
        error.addContext("operation", operation);
        error.addContext("injected", "true");
        return error;
    }

private:
    static std::unordered_map<std::string, double> failureRates_;
};

// Usage in tests
TEST(ErrorHandlingTest, RetryMechanism) {
    ErrorInjector::setFailureRate("database_query", 0.7); // 70% failure rate
    
    int attempts = 0;
    auto result = retryOnFailure([&]() -> qtforge::expected<std::string, qtforge::Error> {
        attempts++;
        if (ErrorInjector::shouldFail("database_query")) {
            return ErrorInjector::makeInjectedError("database_query");
        }
        return std::string("success");
    }, 5);
    
    EXPECT_TRUE(result.has_value());
    EXPECT_GT(attempts, 1); // Should have retried
}
```

## Best Practices

### 1. Error Message Guidelines

- **Be Specific**: Provide clear, actionable error messages
- **Include Context**: Add relevant context information
- **Avoid Technical Jargon**: Use user-friendly language when appropriate
- **Suggest Solutions**: When possible, suggest how to fix the problem

### 2. Error Propagation

- **Fail Fast**: Don't continue processing with invalid data
- **Preserve Context**: Maintain error context through the call stack
- **Transform Appropriately**: Convert low-level errors to domain-specific errors
- **Log at Boundaries**: Log errors at system boundaries

### 3. Error Recovery

- **Graceful Degradation**: Provide fallback functionality when possible
- **Retry Strategies**: Implement appropriate retry mechanisms
- **Circuit Breakers**: Prevent cascading failures
- **User Notification**: Inform users of errors appropriately

### 4. Testing

- **Test Error Paths**: Ensure error handling code is tested
- **Error Injection**: Use error injection for testing resilience
- **Boundary Conditions**: Test edge cases and boundary conditions
- **Recovery Testing**: Test error recovery mechanisms

## See Also

- **[Plugin Interface](../core/plugin-interface.md)**: Plugin error handling patterns
- **[Message Bus](../communication/message-bus.md)**: Communication error handling
- **[Security Manager](../security/security-manager.md)**: Security error handling
- **[Testing Guide](../../contributing/testing.md)**: Error handling testing strategies
