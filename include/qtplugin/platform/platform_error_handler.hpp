/**
 * @file platform_error_handler.hpp
 * @brief Platform-specific error handling and diagnostics
 * @version 3.0.0
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QStringList>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include "../utils/error_handling.hpp"

// Platform-specific includes
#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
// Note: Temporarily disable problematic Windows headers for compilation
// #include <dbghelp.h>
// #include <psapi.h>
#endif

#ifdef Q_OS_UNIX
#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#include <sys/ucontext.h>
#endif

#ifdef Q_OS_MAC
#include <mach/mach.h>
#include <mach/task.h>
#include <mach/thread_act.h>
#endif

namespace qtplugin {

/**
 * @brief Platform-specific error types
 */
enum class PlatformErrorType {
    SystemError,      ///< System-level error
    LibraryError,     ///< Library loading error
    MemoryError,      ///< Memory-related error
    SecurityError,    ///< Security violation error
    PermissionError,  ///< Permission denied error
    NetworkError,     ///< Network-related error
    FileSystemError,  ///< File system error
    ProcessError,     ///< Process-related error
    ThreadError,      ///< Threading error
    HardwareError,    ///< Hardware-related error
    DriverError,      ///< Driver-related error
    ServiceError,     ///< Service-related error
    RegistryError,    ///< Registry error (Windows)
    KernelError,      ///< Kernel-level error
    CustomError       ///< Custom platform error
};

/**
 * @brief Error severity levels
 */
enum class ErrorSeverity {
    Info,      ///< Informational
    Warning,   ///< Warning
    Error,     ///< Error
    Critical,  ///< Critical error
    Fatal      ///< Fatal error
};

/**
 * @brief Platform-specific error information
 */
struct PlatformErrorInfo {
    PlatformErrorType error_type;  ///< Error type
    ErrorSeverity severity;        ///< Error severity
    QString platform;              ///< Platform identifier
    int native_error_code = 0;     ///< Native error code
    QString native_error_message;  ///< Native error message
    QString error_description;     ///< Detailed error description
    QString suggested_solution;    ///< Suggested solution
    QStringList stack_trace;       ///< Stack trace
    QJsonObject system_info;       ///< System information
    QJsonObject process_info;      ///< Process information
    std::chrono::system_clock::time_point timestamp;  ///< Error timestamp
    QJsonObject additional_data;  ///< Additional platform-specific data

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static PlatformErrorInfo from_json(const QJsonObject& json);
};

/**
 * @brief Error recovery strategy
 */
enum class ErrorRecoveryStrategy {
    None,                ///< No recovery
    Retry,               ///< Retry operation
    Fallback,            ///< Use fallback method
    Restart,             ///< Restart component
    Ignore,              ///< Ignore error
    UserIntervention,    ///< Require user intervention
    AutomaticRecovery,   ///< Automatic recovery
    GracefulDegradation  ///< Graceful degradation
};

/**
 * @brief Error recovery configuration
 */
struct ErrorRecoveryConfig {
    ErrorRecoveryStrategy strategy;               ///< Recovery strategy
    int max_retry_attempts = 3;                   ///< Maximum retry attempts
    std::chrono::milliseconds retry_delay{1000};  ///< Delay between retries
    QString fallback_method;                      ///< Fallback method name
    QJsonObject recovery_parameters;              ///< Recovery parameters
    bool enable_automatic_recovery = true;        ///< Enable automatic recovery
    bool notify_user = false;                     ///< Notify user of recovery

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Error handler callback function type
 */
using PlatformErrorHandlerCallback =
    std::function<bool(const PlatformErrorInfo&)>;

/**
 * @brief Error recovery callback
 */
using ErrorRecoveryCallback =
    std::function<bool(const PlatformErrorInfo&, const ErrorRecoveryConfig&)>;

/**
 * @brief Crash dump information
 */
struct CrashDumpInfo {
    QString dump_file_path;                            ///< Crash dump file path
    QString process_name;                              ///< Process name
    int process_id = 0;                                ///< Process ID
    int thread_id = 0;                                 ///< Thread ID
    QString exception_type;                            ///< Exception type
    QString exception_message;                         ///< Exception message
    QStringList stack_trace;                           ///< Stack trace
    QJsonObject registers;                             ///< CPU registers
    QJsonObject memory_info;                           ///< Memory information
    std::chrono::system_clock::time_point crash_time;  ///< Crash timestamp

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Platform-specific error handler
 *
 * This class provides platform-specific error handling, diagnostics,
 * crash reporting, and recovery mechanisms.
 */
class PlatformErrorHandler : public QObject {
    Q_OBJECT

public:
    explicit PlatformErrorHandler(QObject* parent = nullptr);
    ~PlatformErrorHandler() override;

    // === Error Handling ===

    /**
     * @brief Handle platform-specific error
     * @param error_type Error type
     * @param native_error_code Native error code
     * @param context Error context
     * @return Platform error information
     */
    PlatformErrorInfo handle_platform_error(PlatformErrorType error_type,
                                            int native_error_code,
                                            const QString& context = QString());

    /**
     * @brief Convert native error to platform error
     * @param native_error_code Native error code
     * @return Platform error information
     */
    PlatformErrorInfo convert_native_error(int native_error_code);

    /**
     * @brief Get last platform error
     * @return Last platform error information
     */
    std::optional<PlatformErrorInfo> get_last_error() const;

    /**
     * @brief Clear last error
     */
    void clear_last_error();

    // === Error Handler Registration ===

    /**
     * @brief Register error handler
     * @param error_type Error type to handle
     * @param handler Error handler callback
     * @return Handler ID for unregistration
     */
    QString register_error_handler(PlatformErrorType error_type,
                                   PlatformErrorHandler handler);

    /**
     * @brief Unregister error handler
     * @param handler_id Handler identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unregister_error_handler(
        const QString& handler_id);

    /**
     * @brief Register global error handler
     * @param handler Global error handler callback
     * @return Handler ID for unregistration
     */
    QString register_global_error_handler(PlatformErrorHandler handler);

    // === Error Recovery ===

    /**
     * @brief Set error recovery configuration
     * @param error_type Error type
     * @param config Recovery configuration
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_recovery_config(
        PlatformErrorType error_type, const ErrorRecoveryConfig& config);

    /**
     * @brief Get error recovery configuration
     * @param error_type Error type
     * @return Recovery configuration or error
     */
    qtplugin::expected<ErrorRecoveryConfig, PluginError> get_recovery_config(
        PlatformErrorType error_type) const;

    /**
     * @brief Attempt error recovery
     * @param error_info Error information
     * @return Recovery success
     */
    bool attempt_recovery(const PlatformErrorInfo& error_info);

    /**
     * @brief Register recovery callback
     * @param error_type Error type
     * @param callback Recovery callback
     * @return Callback ID for unregistration
     */
    QString register_recovery_callback(PlatformErrorType error_type,
                                       ErrorRecoveryCallback callback);

    // === Stack Trace and Debugging ===

    /**
     * @brief Capture current stack trace
     * @param max_frames Maximum number of frames
     * @return Stack trace
     */
    QStringList capture_stack_trace(int max_frames = 50);

    /**
     * @brief Capture stack trace for thread
     * @param thread_id Thread identifier
     * @param max_frames Maximum number of frames
     * @return Stack trace
     */
    QStringList capture_thread_stack_trace(int thread_id, int max_frames = 50);

    /**
     * @brief Get symbol information for address
     * @param address Memory address
     * @return Symbol information
     */
    QString get_symbol_info(void* address);

    /**
     * @brief Demangle C++ symbol name
     * @param mangled_name Mangled symbol name
     * @return Demangled name
     */
    QString demangle_symbol(const QString& mangled_name);

    // === System Information ===

    /**
     * @brief Get system information
     * @return System information
     */
    QJsonObject get_system_info();

    /**
     * @brief Get process information
     * @return Process information
     */
    QJsonObject get_process_info();

    /**
     * @brief Get memory information
     * @return Memory information
     */
    QJsonObject get_memory_info();

    /**
     * @brief Get CPU information
     * @return CPU information
     */
    QJsonObject get_cpu_info();

    /**
     * @brief Get loaded modules information
     * @return Loaded modules information
     */
    QJsonObject get_loaded_modules_info();

    // === Crash Handling ===

    /**
     * @brief Enable crash dump generation
     * @param dump_directory Directory for crash dumps
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> enable_crash_dumps(
        const QString& dump_directory);

    /**
     * @brief Disable crash dump generation
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> disable_crash_dumps();

    /**
     * @brief Generate crash dump
     * @param dump_file_path Dump file path
     * @return Crash dump information or error
     */
    qtplugin::expected<CrashDumpInfo, PluginError> generate_crash_dump(
        const QString& dump_file_path = QString());

    /**
     * @brief Analyze crash dump
     * @param dump_file_path Dump file path
     * @return Crash analysis or error
     */
    qtplugin::expected<QJsonObject, PluginError> analyze_crash_dump(
        const QString& dump_file_path);

    // === Error Logging ===

    /**
     * @brief Enable error logging
     * @param log_file_path Log file path
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> enable_error_logging(
        const QString& log_file_path);

    /**
     * @brief Disable error logging
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> disable_error_logging();

    /**
     * @brief Log platform error
     * @param error_info Error information
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> log_error(
        const PlatformErrorInfo& error_info);

    /**
     * @brief Get error log entries
     * @param max_entries Maximum number of entries
     * @return Vector of error log entries
     */
    std::vector<PlatformErrorInfo> get_error_log(int max_entries = 100);

    // === Platform-Specific Methods ===

#ifdef Q_OS_WIN
    /**
     * @brief Get Windows error message
     * @param error_code Windows error code
     * @return Error message
     */
    QString get_windows_error_message(DWORD error_code);

    /**
     * @brief Handle Windows structured exception
     * @param exception_pointers Exception pointers
     * @return Exception handling result
     */
    int handle_windows_exception(EXCEPTION_POINTERS* exception_pointers);

    /**
     * @brief Get Windows system error info
     * @return System error information
     */
    QJsonObject get_windows_system_error_info();
#endif

#ifdef Q_OS_UNIX
    /**
     * @brief Handle Unix signal
     * @param signal Signal number
     * @param info Signal information
     * @param context Signal context
     */
    void handle_unix_signal(int signal, siginfo_t* info, void* context);

    /**
     * @brief Get Unix error message
     * @param error_code Unix error code
     * @return Error message
     */
    QString get_unix_error_message(int error_code);

    /**
     * @brief Get Unix system error info
     * @return System error information
     */
    QJsonObject get_unix_system_error_info();
#endif

#ifdef Q_OS_MAC
    /**
     * @brief Handle macOS Mach exception
     * @param task Task port
     * @param thread Thread port
     * @param exception_type Exception type
     * @param exception_data Exception data
     * @return Exception handling result
     */
    kern_return_t handle_mach_exception(mach_port_t task, mach_port_t thread,
                                        exception_type_t exception_type,
                                        exception_data_t exception_data);

    /**
     * @brief Get macOS system error info
     * @return System error information
     */
    QJsonObject get_macos_system_error_info();
#endif

signals:
    /**
     * @brief Emitted when platform error occurs
     * @param error_info Error information
     */
    void platform_error_occurred(const PlatformErrorInfo& error_info);

    /**
     * @brief Emitted when error recovery is attempted
     * @param error_info Error information
     * @param recovery_success Whether recovery was successful
     */
    void error_recovery_attempted(const PlatformErrorInfo& error_info,
                                  bool recovery_success);

    /**
     * @brief Emitted when crash dump is generated
     * @param dump_info Crash dump information
     */
    void crash_dump_generated(const CrashDumpInfo& dump_info);

private slots:
    void on_error_recovery_timer();

private:
    class Private;
    std::unique_ptr<Private> d;

    void setup_platform_handlers();
    void cleanup_platform_handlers();
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::PlatformErrorType)
Q_DECLARE_METATYPE(qtplugin::ErrorSeverity)
Q_DECLARE_METATYPE(qtplugin::ErrorRecoveryStrategy)
Q_DECLARE_METATYPE(qtplugin::PlatformErrorInfo)
Q_DECLARE_METATYPE(qtplugin::ErrorRecoveryConfig)
Q_DECLARE_METATYPE(qtplugin::CrashDumpInfo)
