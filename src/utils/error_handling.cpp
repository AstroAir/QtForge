/**
 * @file error_handling.cpp
 * @brief Enhanced error handling implementation with advanced error management
 * @version 3.2.0
 */

#include <qtplugin/utils/error_handling.hpp>
#include <QLoggingCategory>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <QCoreApplication>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <iomanip>

Q_LOGGING_CATEGORY(errorHandlingLog, "qtplugin.error")

namespace qtplugin {

namespace {
// Error statistics and context tracking
struct ErrorStatistics {
    std::atomic<size_t> total_errors{0};
    std::atomic<size_t> critical_errors{0};
    std::atomic<size_t> warnings{0};
    std::unordered_map<PluginErrorCode, std::atomic<size_t>> error_counts;
    mutable std::mutex stats_mutex;
};

struct ErrorContext {
    std::chrono::system_clock::time_point timestamp;
    QString thread_id;
    QString function_name;
    QString file_name;
    int line_number = 0;
    QJsonObject metadata;

    ErrorContext() : timestamp(std::chrono::system_clock::now()) {
        if (QThread::currentThread()) {
            thread_id = QString::number(reinterpret_cast<quintptr>(QThread::currentThread()), 16);
        }
    }
};

// Global error tracking
static ErrorStatistics g_error_stats;
static std::vector<PluginError> g_error_history;
static std::mutex g_error_history_mutex;
static constexpr size_t MAX_ERROR_HISTORY = 1000;

// Error severity mapping implementation moved to header to avoid duplicate declaration
} // namespace

const char* error_code_to_string(PluginErrorCode code) noexcept {
    switch (code) {
        case PluginErrorCode::Success:
            return "Success";
        case PluginErrorCode::FileNotFound:
            return "FileNotFound";
        case PluginErrorCode::InvalidFormat:
            return "InvalidFormat";
        case PluginErrorCode::LoadFailed:
            return "LoadFailed";
        case PluginErrorCode::UnloadFailed:
            return "UnloadFailed";
        case PluginErrorCode::SymbolNotFound:
            return "SymbolNotFound";
        case PluginErrorCode::AlreadyLoaded:
            return "AlreadyLoaded";
        case PluginErrorCode::NotLoaded:
            return "NotLoaded";
        case PluginErrorCode::PluginNotFound:
            return "PluginNotFound";
        case PluginErrorCode::InitializationFailed:
            return "InitializationFailed";
        case PluginErrorCode::ConfigurationError:
            return "ConfigurationError";
        case PluginErrorCode::DependencyMissing:
            return "DependencyMissing";
        case PluginErrorCode::VersionMismatch:
            return "VersionMismatch";
        case PluginErrorCode::ExecutionFailed:
            return "ExecutionFailed";
        case PluginErrorCode::CommandNotFound:
            return "CommandNotFound";
        case PluginErrorCode::InvalidParameters:
            return "InvalidParameters";
        case PluginErrorCode::StateError:
            return "StateError";
        case PluginErrorCode::InvalidArgument:
            return "InvalidArgument";
        case PluginErrorCode::NotFound:
            return "NotFound";
        case PluginErrorCode::ResourceUnavailable:
            return "ResourceUnavailable";
        case PluginErrorCode::AlreadyExists:
            return "AlreadyExists";
        case PluginErrorCode::NotImplemented:
            return "NotImplemented";
        case PluginErrorCode::SecurityViolation:
            return "SecurityViolation";
        case PluginErrorCode::PermissionDenied:
            return "PermissionDenied";
        case PluginErrorCode::SignatureInvalid:
            return "SignatureInvalid";
        case PluginErrorCode::UntrustedSource:
            return "UntrustedSource";
        case PluginErrorCode::OutOfMemory:
            return "OutOfMemory";
        case PluginErrorCode::ResourceExhausted:
            return "ResourceExhausted";
        case PluginErrorCode::NetworkError:
            return "NetworkError";
        case PluginErrorCode::FileSystemError:
            return "FileSystemError";
        case PluginErrorCode::ThreadingError:
            return "ThreadingError";
        case PluginErrorCode::TimeoutError:
            return "TimeoutError";
        case PluginErrorCode::UnknownError:
        default:
            return "UnknownError";
    }
}

// === Enhanced Error Management Functions ===

void record_error(const PluginError& error) {
    // Update statistics
    g_error_stats.total_errors++;

    auto severity = get_error_severity(error.code);
    switch (severity) {
        case ErrorSeverity::Warning:
            g_error_stats.warnings++;
            break;
        case ErrorSeverity::Critical:
        case ErrorSeverity::Fatal:
            g_error_stats.critical_errors++;
            break;
        default:
            break;
    }

    // Update error code statistics
    {
        std::lock_guard lock(g_error_stats.stats_mutex);
        g_error_stats.error_counts[error.code]++;
    }

    // Add to history
    {
        std::lock_guard lock(g_error_history_mutex);
        g_error_history.push_back(error);

        // Maintain maximum history size
        if (g_error_history.size() > MAX_ERROR_HISTORY) {
            g_error_history.erase(g_error_history.begin());
        }
    }

    // Log the error
    auto code_str = error_code_to_string(error.code);

    switch (severity) {
        case ErrorSeverity::Info:
            qCInfo(errorHandlingLog) << "[" << code_str << "]" << error.message.c_str();
            break;
        case ErrorSeverity::Warning:
            qCWarning(errorHandlingLog) << "[" << code_str << "]" << error.message.c_str();
            break;
        case ErrorSeverity::Error:
        case ErrorSeverity::Critical:
            qCCritical(errorHandlingLog) << "[" << code_str << "]" << error.message.c_str();
            break;
        case ErrorSeverity::Fatal:
            qCFatal(errorHandlingLog) << "[" << code_str << "]" << error.message.c_str();
            break;
    }
}

PluginError make_error(PluginErrorCode code, const std::string& message,
                       const std::string& plugin_id, const std::string& context) {
    PluginError error;
    error.code = code;
    error.message = message;
    error.plugin_id = plugin_id;
    error.context = context;
    error.timestamp = std::chrono::system_clock::now();

    // Record the error
    record_error(error);

    return error;
}

const char* error_severity_to_string(ErrorSeverity severity) noexcept {
    switch (severity) {
        case ErrorSeverity::Info: return "Info";
        case ErrorSeverity::Warning: return "Warning";
        case ErrorSeverity::Error: return "Error";
        case ErrorSeverity::Critical: return "Critical";
        case ErrorSeverity::Fatal: return "Fatal";
        default: return "Unknown";
    }
}

ErrorSeverity get_error_severity_for_code(PluginErrorCode code) {
    return get_error_severity(code);
}

// Error statistics functions
size_t get_total_error_count() {
    return g_error_stats.total_errors.load();
}

size_t get_critical_error_count() {
    return g_error_stats.critical_errors.load();
}

size_t get_warning_count() {
    return g_error_stats.warnings.load();
}

size_t get_error_count_for_code(PluginErrorCode code) {
    std::lock_guard lock(g_error_stats.stats_mutex);
    auto it = g_error_stats.error_counts.find(code);
    return (it != g_error_stats.error_counts.end()) ? it->second.load() : 0;
}

std::vector<PluginError> get_error_history(size_t max_count) {
    std::lock_guard lock(g_error_history_mutex);

    if (max_count == 0 || max_count >= g_error_history.size()) {
        return g_error_history;
    }

    // Return the most recent errors
    auto start_it = g_error_history.end() - static_cast<long>(max_count);
    return std::vector<PluginError>(start_it, g_error_history.end());
}

void clear_error_history() {
    std::lock_guard lock(g_error_history_mutex);
    g_error_history.clear();
}

void reset_error_statistics() {
    std::lock_guard lock(g_error_stats.stats_mutex);

    g_error_stats.total_errors = 0;
    g_error_stats.critical_errors = 0;
    g_error_stats.warnings = 0;
    g_error_stats.error_counts.clear();
}

// Error reporting functions
std::string generate_error_report() {
    std::stringstream report;

    report << "=== QtPlugin Error Report ===\n";
    report << "Generated: " << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << "\n\n";

    // Statistics summary
    report << "Error Statistics:\n";
    report << "  Total Errors: " << get_total_error_count() << "\n";
    report << "  Critical Errors: " << get_critical_error_count() << "\n";
    report << "  Warnings: " << get_warning_count() << "\n\n";

    // Error code breakdown
    report << "Error Code Breakdown:\n";
    {
        std::lock_guard lock(g_error_stats.stats_mutex);
        for (const auto& [code, count] : g_error_stats.error_counts) {
            if (count.load() > 0) {
                report << "  " << error_code_to_string(code) << ": " << count.load() << "\n";
            }
        }
    }
    report << "\n";

    // Recent error history (last 10 errors)
    auto recent_errors = get_error_history(10);
    if (!recent_errors.empty()) {
        report << "Recent Errors (last " << recent_errors.size() << "):\n";

        for (const auto& error : recent_errors) {
            auto time_t = std::chrono::system_clock::to_time_t(error.timestamp);
            report << "  [" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
            report << error_code_to_string(error.code) << ": " << error.message;

            if (!error.plugin_id.empty()) {
                report << " (Plugin: " << error.plugin_id << ")";
            }

            if (!error.context.empty()) {
                report << " [" << error.context << "]";
            }

            report << "\n";
        }
    }

    report << "\n=== End of Report ===\n";

    return report.str();
}

QJsonObject generate_error_report_json() {
    QJsonObject report;

    // Metadata
    QJsonObject metadata;
    metadata["generated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metadata["version"] = "3.2.0";
    report["metadata"] = metadata;

    // Statistics
    QJsonObject stats;
    stats["total_errors"] = static_cast<qint64>(get_total_error_count());
    stats["critical_errors"] = static_cast<qint64>(get_critical_error_count());
    stats["warnings"] = static_cast<qint64>(get_warning_count());
    report["statistics"] = stats;

    // Error code breakdown
    QJsonObject error_codes;
    {
        std::lock_guard lock(g_error_stats.stats_mutex);
        for (const auto& [code, count] : g_error_stats.error_counts) {
            if (count.load() > 0) {
                error_codes[error_code_to_string(code)] = static_cast<qint64>(count.load());
            }
        }
    }
    report["error_codes"] = error_codes;

    // Recent error history
    auto recent_errors = get_error_history(20);
    QJsonArray errors_array;

    for (const auto& error : recent_errors) {
        QJsonObject error_obj;
        error_obj["timestamp"] = QDateTime::fromSecsSinceEpoch(
            std::chrono::duration_cast<std::chrono::seconds>(
                error.timestamp.time_since_epoch()).count()
        ).toString(Qt::ISODate);
        error_obj["code"] = error_code_to_string(error.code);
        error_obj["severity"] = error_severity_to_string(get_error_severity(error.code));
        error_obj["message"] = QString::fromStdString(error.message);

        if (!error.plugin_id.empty()) {
            error_obj["plugin_id"] = QString::fromStdString(error.plugin_id);
        }

        if (!error.context.empty()) {
            error_obj["context"] = QString::fromStdString(error.context);
        }

        errors_array.append(error_obj);
    }

    report["recent_errors"] = errors_array;

    return report;
}

bool save_error_report_to_file(const std::string& filename, bool json_format) {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        if (json_format) {
            auto json_report = generate_error_report_json();
            QJsonDocument doc(json_report);
            file << doc.toJson(QJsonDocument::Indented).toStdString();
        } else {
            file << generate_error_report();
        }

        file.close();
        return true;

    } catch (const std::exception&) {
        return false;
    }
}

// Error filtering functions
std::vector<PluginError> filter_errors_by_code(PluginErrorCode code, size_t max_count) {
    std::lock_guard lock(g_error_history_mutex);

    std::vector<PluginError> filtered;

    for (auto it = g_error_history.rbegin();
         it != g_error_history.rend() && (max_count == 0 || filtered.size() < max_count);
         ++it) {
        if (it->code == code) {
            filtered.push_back(*it);
        }
    }

    // Reverse to maintain chronological order
    std::reverse(filtered.begin(), filtered.end());
    return filtered;
}

std::vector<PluginError> filter_errors_by_plugin(const std::string& plugin_id, size_t max_count) {
    std::lock_guard lock(g_error_history_mutex);

    std::vector<PluginError> filtered;

    for (auto it = g_error_history.rbegin();
         it != g_error_history.rend() && (max_count == 0 || filtered.size() < max_count);
         ++it) {
        if (it->plugin_id == plugin_id) {
            filtered.push_back(*it);
        }
    }

    std::reverse(filtered.begin(), filtered.end());
    return filtered;
}

std::vector<PluginError> filter_errors_by_severity(ErrorSeverity severity, size_t max_count) {
    std::lock_guard lock(g_error_history_mutex);

    std::vector<PluginError> filtered;

    for (auto it = g_error_history.rbegin();
         it != g_error_history.rend() && (max_count == 0 || filtered.size() < max_count);
         ++it) {
        if (get_error_severity(it->code) == severity) {
            filtered.push_back(*it);
        }
    }

    std::reverse(filtered.begin(), filtered.end());
    return filtered;
}

}  // namespace qtplugin
