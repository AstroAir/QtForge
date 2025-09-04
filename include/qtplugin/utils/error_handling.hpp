/**
 * @file error_handling.hpp
 * @brief Error handling utilities for the plugin system
 * @version 3.0.0
 */

#pragma once

#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <variant>

// Simple expected implementation for C++20 compatibility
namespace qtplugin {
template <typename E>
class unexpected {
public:
    explicit unexpected(E error) : m_error(std::move(error)) {}
    const E& error() const& { return m_error; }
    E& error() & { return m_error; }
    E&& error() && { return std::move(m_error); }

private:
    E m_error;
};

template <typename T, typename E>
class expected {
public:
    expected(T value) : m_data(std::move(value)) {}
    expected(unexpected<E> error) : m_data(std::move(error)) {}

    bool has_value() const { return std::holds_alternative<T>(m_data); }
    explicit operator bool() const { return has_value(); }

    const T& value() const& { return std::get<T>(m_data); }
    T& value() & { return std::get<T>(m_data); }
    T&& value() && { return std::get<T>(std::move(m_data)); }

    const E& error() const& { return std::get<unexpected<E>>(m_data).error(); }
    E& error() & { return std::get<unexpected<E>>(m_data).error(); }
    E&& error() && {
        return std::get<unexpected<E>>(std::move(m_data)).error();
    }

    const T& operator*() const& { return value(); }
    T& operator*() & { return value(); }
    T&& operator*() && { return std::move(*this).value(); }

    bool has_error() const { return !has_value(); }

    T value_or(const T& default_value) const {
        return has_value() ? value() : default_value;
    }

private:
    std::variant<T, unexpected<E>> m_data;
};

template <typename E>
class expected<void, E> {
public:
    expected() : m_has_value(true) {}
    expected(unexpected<E> error)
        : m_has_value(false), m_error(std::move(error)) {}

    bool has_value() const { return m_has_value; }
    explicit operator bool() const { return has_value(); }

    void value() const {
        if (!m_has_value)
            throw std::runtime_error("bad expected access");
    }

    const E& error() const& { return m_error.error(); }
    E& error() & { return m_error.error(); }
    E&& error() && { return std::move(m_error).error(); }

private:
    bool m_has_value;
    unexpected<E> m_error{E{}};
};
}  // namespace qtplugin

// Fallback for compilers without source_location
#if !defined(__cpp_lib_source_location) && __cplusplus < 202002L
namespace std {
struct source_location {
    static constexpr source_location current() noexcept { return {}; }
    constexpr const char* file_name() const noexcept { return "unknown"; }
    constexpr unsigned int line() const noexcept { return 0; }
    constexpr const char* function_name() const noexcept { return "unknown"; }
};
}  // namespace std
#else
#include <source_location>
#endif

// Use standard library format
#include <format>

namespace qtplugin {

/**
 * @brief Error codes for plugin operations
 */
enum class PluginErrorCode {
    Success = 0,

    // Loading errors
    FileNotFound = 100,
    InvalidFormat,
    LoadFailed,
    UnloadFailed,
    SymbolNotFound,
    AlreadyLoaded,
    NotLoaded,
    PluginNotFound,

    // Initialization errors
    InitializationFailed = 200,
    ConfigurationError,
    DependencyMissing,
    VersionMismatch,

    // Runtime errors
    ExecutionFailed = 300,
    CommandNotFound,
    InvalidParameters,
    StateError,
    InvalidArgument,
    NotFound,
    ResourceUnavailable,
    AlreadyExists,
    NotImplemented,
    InvalidState,
    InvalidConfiguration,
    DuplicatePlugin,
    CircularDependency,
    OperationCancelled,
    NotSupported,
    IncompatibleVersion,
    SystemError,

    // Security errors
    SecurityViolation = 400,
    PermissionDenied,
    SignatureInvalid,
    UntrustedSource,

    // System errors
    OutOfMemory = 500,
    ResourceExhausted,
    NetworkError,
    FileSystemError,
    ThreadingError,
    TimeoutError,

    // Generic errors
    UnknownError = 999
};

/**
 * @brief Error category for plugin errors
 */
class PluginErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override { return "qtplugin"; }

    std::string message(int ev) const override {
        switch (static_cast<PluginErrorCode>(ev)) {
            case PluginErrorCode::Success:
                return "Success";

            // Loading errors
            case PluginErrorCode::FileNotFound:
                return "Plugin file not found";
            case PluginErrorCode::InvalidFormat:
                return "Invalid plugin format";
            case PluginErrorCode::LoadFailed:
                return "Failed to load plugin";
            case PluginErrorCode::SymbolNotFound:
                return "Required symbol not found in plugin";

            // Initialization errors
            case PluginErrorCode::InitializationFailed:
                return "Plugin initialization failed";
            case PluginErrorCode::ConfigurationError:
                return "Plugin configuration error";
            case PluginErrorCode::DependencyMissing:
                return "Required dependency missing";
            case PluginErrorCode::VersionMismatch:
                return "Plugin version mismatch";

            // Runtime errors
            case PluginErrorCode::ExecutionFailed:
                return "Plugin execution failed";
            case PluginErrorCode::CommandNotFound:
                return "Command not found";
            case PluginErrorCode::InvalidParameters:
                return "Invalid parameters";
            case PluginErrorCode::StateError:
                return "Invalid plugin state";
            case PluginErrorCode::InvalidArgument:
                return "Invalid argument";
            case PluginErrorCode::NotFound:
                return "Resource not found";
            case PluginErrorCode::ResourceUnavailable:
                return "Resource unavailable";
            case PluginErrorCode::AlreadyExists:
                return "Resource already exists";
            case PluginErrorCode::NotImplemented:
                return "Feature not implemented";

            // Security errors
            case PluginErrorCode::SecurityViolation:
                return "Security violation";
            case PluginErrorCode::PermissionDenied:
                return "Permission denied";
            case PluginErrorCode::SignatureInvalid:
                return "Invalid plugin signature";
            case PluginErrorCode::UntrustedSource:
                return "Untrusted plugin source";

            // System errors
            case PluginErrorCode::OutOfMemory:
                return "Out of memory";
            case PluginErrorCode::NetworkError:
                return "Network error";
            case PluginErrorCode::FileSystemError:
                return "File system error";
            case PluginErrorCode::ThreadingError:
                return "Threading error";

            default:
                return "Unknown error";
        }
    }

    std::error_condition default_error_condition(
        int ev) const noexcept override {
        switch (static_cast<PluginErrorCode>(ev)) {
            case PluginErrorCode::Success:
                return std::error_condition{};
            case PluginErrorCode::FileNotFound:
            case PluginErrorCode::FileSystemError:
                return std::errc::no_such_file_or_directory;
            case PluginErrorCode::PermissionDenied:
                return std::errc::permission_denied;
            case PluginErrorCode::OutOfMemory:
                return std::errc::not_enough_memory;
            case PluginErrorCode::NetworkError:
                return std::errc::network_unreachable;
            default:
                return std::error_condition{ev, *this};
        }
    }
};

/**
 * @brief Get the plugin error category instance
 */
inline const PluginErrorCategory& plugin_error_category() {
    static PluginErrorCategory instance;
    return instance;
}

/**
 * @brief Create an error code from a plugin error code
 */
inline std::error_code make_error_code(PluginErrorCode ec) {
    return {static_cast<int>(ec), plugin_error_category()};
}

/**
 * @brief Plugin error information
 */
struct PluginError {
    PluginErrorCode code;
    std::string message;
    std::string details;
    std::source_location location;

    /**
     * @brief Default constructor
     */
    PluginError()
        : code(PluginErrorCode::UnknownError),
          location(std::source_location::current()) {}

    /**
     * @brief Constructor with error code
     */
    explicit PluginError(
        PluginErrorCode ec,
        std::source_location loc = std::source_location::current())
        : code(ec),
          message(plugin_error_category().message(static_cast<int>(ec))),
          location(loc) {}

    /**
     * @brief Constructor with error code and custom message
     */
    PluginError(PluginErrorCode ec, std::string_view msg,
                std::source_location loc = std::source_location::current())
        : code(ec), message(msg), location(loc) {}

    /**
     * @brief Constructor with error code, message, and details
     */
    PluginError(PluginErrorCode ec, std::string_view msg, std::string_view det,
                std::source_location loc = std::source_location::current())
        : code(ec), message(msg), details(det), location(loc) {}

    /**
     * @brief Get formatted error message with location information
     */
    std::string formatted_message() const {
        std::string result = std::string(location.file_name()) + ":" +
                             std::to_string(location.line()) + " in " +
                             std::string(location.function_name()) + ": " +
                             message + " (" +
                             (details.empty() ? "no details" : details) + ")";
        return result;
    }

    /**
     * @brief Convert to error code
     */
    operator std::error_code() const { return make_error_code(code); }

    /**
     * @brief Convert to string representation
     */
    std::string to_string() const { return formatted_message(); }

    /**
     * @brief Get error message (for exception compatibility)
     */
    const char* what() const noexcept {
        static thread_local std::string msg;
        msg = message;
        return msg.c_str();
    }

    /**
     * @brief Equality comparison
     */
    bool operator==(const PluginError& other) const {
        return code == other.code && message == other.message &&
               details == other.details;
    }

    /**
     * @brief Inequality comparison
     */
    bool operator!=(const PluginError& other) const {
        return !(*this == other);
    }

    /**
     * @brief Check if this is a success (no error)
     */
    bool is_success() const noexcept {
        return code == PluginErrorCode::Success;
    }

    /**
     * @brief Check if this is a loading error
     */
    bool is_loading_error() const noexcept {
        return static_cast<int>(code) >= 100 && static_cast<int>(code) < 200;
    }

    /**
     * @brief Check if this is an initialization error
     */
    bool is_initialization_error() const noexcept {
        return static_cast<int>(code) >= 200 && static_cast<int>(code) < 300;
    }

    /**
     * @brief Check if this is a runtime error
     */
    bool is_runtime_error() const noexcept {
        return static_cast<int>(code) >= 300 && static_cast<int>(code) < 400;
    }

    /**
     * @brief Check if this is a security error
     */
    bool is_security_error() const noexcept {
        return static_cast<int>(code) >= 400 && static_cast<int>(code) < 500;
    }

    /**
     * @brief Check if this is a system error
     */
    bool is_system_error() const noexcept {
        return static_cast<int>(code) >= 500 && static_cast<int>(code) < 600;
    }
};

/**
 * @brief Result type for plugin operations
 */
template <typename T>
using PluginResult = expected<T, PluginError>;

/**
 * @brief Void result type for plugin operations
 */
using PluginVoidResult = expected<void, PluginError>;

/**
 * @brief Helper function to create a success result
 */
template <typename T>
constexpr PluginResult<T> make_success(T&& value) {
    return PluginResult<T>{std::forward<T>(value)};
}

/**
 * @brief Helper function to create a success result for void operations
 */
inline PluginVoidResult make_success() { return PluginVoidResult{}; }

/**
 * @brief Helper function to create an error result
 */
template <typename T>
inline PluginResult<T> make_error(PluginError error) {
    return PluginResult<T>{unexpected<PluginError>{std::move(error)}};
}

/**
 * @brief Helper function to create an error result with error code
 */
template <typename T>
inline PluginResult<T> make_error(
    PluginErrorCode code,
    std::source_location loc = std::source_location::current()) {
    return PluginResult<T>{unexpected<PluginError>{PluginError{code, loc}}};
}

/**
 * @brief Helper function to create an error result with error code and message
 */
template <typename T>
inline PluginResult<T> make_error(
    PluginErrorCode code, std::string_view message,
    std::source_location loc = std::source_location::current()) {
    return PluginResult<T>{
        unexpected<PluginError>{PluginError{code, message, loc}}};
}

/**
 * @brief Helper function to create an error result with error code, message,
 * and details
 */
template <typename T>
inline PluginResult<T> make_error(
    PluginErrorCode code, std::string_view message, std::string_view details,
    std::source_location loc = std::source_location::current()) {
    return PluginResult<T>{
        unexpected<PluginError>{PluginError{code, message, details, loc}}};
}

/**
 * @brief Macro to create an error with current location
 */
#define QTPLUGIN_ERROR(code, ...)                            \
    qtplugin::PluginError {                                  \
        code, ##__VA_ARGS__, std::source_location::current() \
    }

/**
 * @brief Macro to return an error result with current location
 */
#define QTPLUGIN_RETURN_ERROR(code, ...)                 \
    return qtplugin::unexpected<qtplugin::PluginError> { \
        QTPLUGIN_ERROR(code, ##__VA_ARGS__)              \
    }

/**
 * @brief Convert error code to string
 */
const char* error_code_to_string(PluginErrorCode code) noexcept;

/**
 * @brief Exception class for plugin errors (for compatibility with
 * exception-based code)
 */
class PluginException : public std::exception {
public:
    explicit PluginException(PluginError error) : m_error(std::move(error)) {}

    const char* what() const noexcept override {
        return m_error.message.c_str();
    }

    const PluginError& error() const noexcept { return m_error; }

private:
    PluginError m_error;
};

}  // namespace qtplugin

// Enable std::error_code support
namespace std {
template <>
struct is_error_code_enum<qtplugin::PluginErrorCode> : true_type {};
}  // namespace std
