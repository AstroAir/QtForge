/**
 * @file transaction_error_handler.cpp
 * @brief Implementation of enhanced transaction error handling
 * @version 3.1.0
 */

#include "qtplugin/workflow/transaction_error_handler.hpp"
#include <QJsonArray>
#include <QLoggingCategory>
#include <QUuid>
#include <algorithm>

namespace {
Q_LOGGING_CATEGORY(transactionErrorHandlerLog,
                   "qtplugin.workflow.transaction_error_handler")
}  // namespace

namespace qtplugin::workflow::transactions {

// === TransactionErrorInfo Implementation ===

QJsonObject TransactionErrorInfo::to_json() const {
    QJsonObject json;
    json["error_id"] = error_id;
    json["transaction_id"] = transaction_id;
    json["operation_id"] = operation_id;
    json["plugin_id"] = plugin_id;
    json["error_code"] = static_cast<int>(error_code);
    json["category"] = static_cast<int>(category);
    json["severity"] = static_cast<int>(severity);
    json["message"] = message;
    json["details"] = details;
    json["context"] = context;
    json["error_data"] = error_data;
    json["timestamp"] = timestamp.toString(Qt::ISODate);
    json["duration_ms"] = static_cast<int>(duration.count());
    json["recommended_action"] = static_cast<int>(recommended_action);
    json["recoverable"] = recoverable;
    json["retryable"] = retryable;
    json["retry_count"] = retry_count;
    json["max_retries"] = max_retries;
    json["root_cause_id"] = root_cause_id;

    QJsonArray related_errors_array;
    for (const QString& related_error : related_errors) {
        related_errors_array.append(related_error);
    }
    json["related_errors"] = related_errors_array;

    return json;
}

qtplugin::expected<TransactionErrorInfo, PluginError>
TransactionErrorInfo::from_json(const QJsonObject& json) {
    TransactionErrorInfo info;

    if (json.contains("error_id") && json["error_id"].isString()) {
        info.error_id = json["error_id"].toString();
    }

    if (json.contains("transaction_id") && json["transaction_id"].isString()) {
        info.transaction_id = json["transaction_id"].toString();
    }

    if (json.contains("operation_id") && json["operation_id"].isString()) {
        info.operation_id = json["operation_id"].toString();
    }

    if (json.contains("plugin_id") && json["plugin_id"].isString()) {
        info.plugin_id = json["plugin_id"].toString();
    }

    if (json.contains("error_code") && json["error_code"].isDouble()) {
        info.error_code =
            static_cast<PluginErrorCode>(json["error_code"].toInt());
    }

    if (json.contains("category") && json["category"].isDouble()) {
        info.category =
            static_cast<TransactionErrorCategory>(json["category"].toInt());
    }

    if (json.contains("severity") && json["severity"].isDouble()) {
        info.severity =
            static_cast<TransactionErrorSeverity>(json["severity"].toInt());
    }

    if (json.contains("message") && json["message"].isString()) {
        info.message = json["message"].toString();
    }

    if (json.contains("details") && json["details"].isString()) {
        info.details = json["details"].toString();
    }

    if (json.contains("context") && json["context"].isString()) {
        info.context = json["context"].toString();
    }

    if (json.contains("error_data") && json["error_data"].isObject()) {
        info.error_data = json["error_data"].toObject();
    }

    if (json.contains("timestamp") && json["timestamp"].isString()) {
        info.timestamp =
            QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    }

    if (json.contains("duration_ms") && json["duration_ms"].isDouble()) {
        info.duration = std::chrono::milliseconds(json["duration_ms"].toInt());
    }

    if (json.contains("recommended_action") &&
        json["recommended_action"].isDouble()) {
        info.recommended_action = static_cast<TransactionRecoveryAction>(
            json["recommended_action"].toInt());
    }

    if (json.contains("recoverable") && json["recoverable"].isBool()) {
        info.recoverable = json["recoverable"].toBool();
    }

    if (json.contains("retryable") && json["retryable"].isBool()) {
        info.retryable = json["retryable"].toBool();
    }

    if (json.contains("retry_count") && json["retry_count"].isDouble()) {
        info.retry_count = json["retry_count"].toInt();
    }

    if (json.contains("max_retries") && json["max_retries"].isDouble()) {
        info.max_retries = json["max_retries"].toInt();
    }

    if (json.contains("root_cause_id") && json["root_cause_id"].isString()) {
        info.root_cause_id = json["root_cause_id"].toString();
    }

    if (json.contains("related_errors") && json["related_errors"].isArray()) {
        QJsonArray related_errors_array = json["related_errors"].toArray();
        for (const QJsonValue& value : related_errors_array) {
            if (value.isString()) {
                info.related_errors.push_back(value.toString());
            }
        }
    }

    return info;
}

// === TransactionErrorContext Implementation ===

QJsonObject TransactionErrorContext::to_json() const {
    QJsonObject json;
    json["transaction_id"] = transaction_id;
    json["transaction_state"] = static_cast<int>(transaction_state);
    json["isolation_level"] = static_cast<int>(isolation_level);
    json["current_operation_id"] = current_operation_id;
    json["current_plugin_id"] = current_plugin_id;
    json["operation_parameters"] = operation_parameters;
    json["failed_participant"] = failed_participant;
    json["transaction_start_time"] =
        transaction_start_time.toString(Qt::ISODate);
    json["error_occurrence_time"] = error_occurrence_time.toString(Qt::ISODate);
    json["timeout_duration_ms"] = static_cast<int>(timeout_duration.count());

    QJsonArray participants_array;
    for (const QString& participant : participants) {
        participants_array.append(participant);
    }
    json["participants"] = participants_array;

    QJsonArray previous_errors_array;
    for (const TransactionErrorInfo& error : previous_errors) {
        previous_errors_array.append(error.to_json());
    }
    json["previous_errors"] = previous_errors_array;

    return json;
}

qtplugin::expected<TransactionErrorContext, PluginError>
TransactionErrorContext::from_json(const QJsonObject& json) {
    TransactionErrorContext context;

    if (json.contains("transaction_id") && json["transaction_id"].isString()) {
        context.transaction_id = json["transaction_id"].toString();
    }

    if (json.contains("transaction_state") &&
        json["transaction_state"].isDouble()) {
        context.transaction_state =
            static_cast<TransactionState>(json["transaction_state"].toInt());
    }

    if (json.contains("isolation_level") &&
        json["isolation_level"].isDouble()) {
        context.isolation_level =
            static_cast<IsolationLevel>(json["isolation_level"].toInt());
    }

    if (json.contains("current_operation_id") &&
        json["current_operation_id"].isString()) {
        context.current_operation_id = json["current_operation_id"].toString();
    }

    if (json.contains("current_plugin_id") &&
        json["current_plugin_id"].isString()) {
        context.current_plugin_id = json["current_plugin_id"].toString();
    }

    if (json.contains("operation_parameters") &&
        json["operation_parameters"].isObject()) {
        context.operation_parameters = json["operation_parameters"].toObject();
    }

    if (json.contains("failed_participant") &&
        json["failed_participant"].isString()) {
        context.failed_participant = json["failed_participant"].toString();
    }

    if (json.contains("transaction_start_time") &&
        json["transaction_start_time"].isString()) {
        context.transaction_start_time = QDateTime::fromString(
            json["transaction_start_time"].toString(), Qt::ISODate);
    }

    if (json.contains("error_occurrence_time") &&
        json["error_occurrence_time"].isString()) {
        context.error_occurrence_time = QDateTime::fromString(
            json["error_occurrence_time"].toString(), Qt::ISODate);
    }

    if (json.contains("timeout_duration_ms") &&
        json["timeout_duration_ms"].isDouble()) {
        context.timeout_duration =
            std::chrono::milliseconds(json["timeout_duration_ms"].toInt());
    }

    if (json.contains("participants") && json["participants"].isArray()) {
        QJsonArray participants_array = json["participants"].toArray();
        for (const QJsonValue& value : participants_array) {
            if (value.isString()) {
                context.participants.push_back(value.toString());
            }
        }
    }

    if (json.contains("previous_errors") && json["previous_errors"].isArray()) {
        QJsonArray previous_errors_array = json["previous_errors"].toArray();
        for (const QJsonValue& value : previous_errors_array) {
            if (value.isObject()) {
                auto error_result =
                    TransactionErrorInfo::from_json(value.toObject());
                if (error_result) {
                    context.previous_errors.push_back(error_result.value());
                }
            }
        }
    }

    return context;
}

// === TransactionErrorClassifier Implementation ===

TransactionErrorClassifier::TransactionErrorClassifier(QObject* parent)
    : QObject(parent) {
    // Initialize default classification rules
    register_classification_rule(PluginErrorCode::InvalidParameters,
                                 TransactionErrorCategory::Validation,
                                 TransactionErrorSeverity::Error);
    register_classification_rule(PluginErrorCode::InvalidState,
                                 TransactionErrorCategory::State,
                                 TransactionErrorSeverity::Error);
    register_classification_rule(PluginErrorCode::PluginNotFound,
                                 TransactionErrorCategory::Resource,
                                 TransactionErrorSeverity::Error);
    register_classification_rule(PluginErrorCode::NetworkError,
                                 TransactionErrorCategory::Network,
                                 TransactionErrorSeverity::Error);
    register_classification_rule(PluginErrorCode::Timeout,
                                 TransactionErrorCategory::Timeout,
                                 TransactionErrorSeverity::Warning);
    register_classification_rule(PluginErrorCode::ExecutionFailed,
                                 TransactionErrorCategory::Participant,
                                 TransactionErrorSeverity::Error);
    register_classification_rule(PluginErrorCode::CircularDependency,
                                 TransactionErrorCategory::Deadlock,
                                 TransactionErrorSeverity::Critical);
    register_classification_rule(PluginErrorCode::ThreadingError,
                                 TransactionErrorCategory::Concurrency,
                                 TransactionErrorSeverity::Error);
    register_classification_rule(PluginErrorCode::SystemError,
                                 TransactionErrorCategory::Data,
                                 TransactionErrorSeverity::Critical);
    register_classification_rule(PluginErrorCode::SystemError,
                                 TransactionErrorCategory::System,
                                 TransactionErrorSeverity::Critical);

    qCDebug(transactionErrorHandlerLog)
        << "Created transaction error classifier with default rules";
}

TransactionErrorCategory TransactionErrorClassifier::classify_error(
    const PluginError& error, const TransactionErrorContext& context) const {
    Q_UNUSED(context)

    auto it = m_classification_rules.find(error.code);
    if (it != m_classification_rules.end()) {
        return it->second.first;
    }

    return TransactionErrorCategory::Unknown;
}

TransactionErrorSeverity TransactionErrorClassifier::determine_severity(
    const PluginError& error, const TransactionErrorContext& context) const {
    Q_UNUSED(context)

    auto it = m_classification_rules.find(error.code);
    if (it != m_classification_rules.end()) {
        return it->second.second;
    }

    return TransactionErrorSeverity::Error;
}

TransactionRecoveryAction TransactionErrorClassifier::recommend_action(
    const TransactionErrorInfo& error_info,
    const TransactionErrorContext& context) const {
    Q_UNUSED(context)

    // Check recovery strategies
    for (const auto& strategy : m_recovery_strategies) {
        if (strategy.applicable_category == error_info.category) {
            if (!strategy.should_apply || strategy.should_apply(error_info)) {
                if (strategy.action_selector) {
                    return strategy.action_selector(error_info, context);
                }
                return strategy.primary_action;
            }
        }
    }

    // Default recommendations based on category
    switch (error_info.category) {
        case TransactionErrorCategory::Validation:
            return TransactionRecoveryAction::Abort;
        case TransactionErrorCategory::State:
            return TransactionRecoveryAction::Rollback;
        case TransactionErrorCategory::Resource:
            return TransactionRecoveryAction::Retry;
        case TransactionErrorCategory::Network:
            return TransactionRecoveryAction::Retry;
        case TransactionErrorCategory::Timeout:
            return TransactionRecoveryAction::Retry;
        case TransactionErrorCategory::Participant:
            return TransactionRecoveryAction::Rollback;
        case TransactionErrorCategory::Rollback:
            return TransactionRecoveryAction::Abort;
        case TransactionErrorCategory::Commit:
            return TransactionRecoveryAction::Rollback;
        case TransactionErrorCategory::Prepare:
            return TransactionRecoveryAction::Rollback;
        case TransactionErrorCategory::Deadlock:
            return TransactionRecoveryAction::Rollback;
        case TransactionErrorCategory::Concurrency:
            return TransactionRecoveryAction::Retry;
        case TransactionErrorCategory::Data:
            return TransactionRecoveryAction::Abort;
        case TransactionErrorCategory::System:
            return TransactionRecoveryAction::Escalate;
        default:
            return TransactionRecoveryAction::Rollback;
    }
}

void TransactionErrorClassifier::register_classification_rule(
    PluginErrorCode error_code, TransactionErrorCategory category,
    TransactionErrorSeverity severity) {
    m_classification_rules[error_code] = std::make_pair(category, severity);

    qCDebug(transactionErrorHandlerLog)
        << "Registered classification rule for error code:"
        << static_cast<int>(error_code)
        << "category:" << static_cast<int>(category)
        << "severity:" << static_cast<int>(severity);
}

void TransactionErrorClassifier::register_recovery_strategy(
    const TransactionErrorRecoveryStrategy& strategy) {
    m_recovery_strategies.push_back(strategy);

    qCDebug(transactionErrorHandlerLog)
        << "Registered recovery strategy for category:"
        << static_cast<int>(strategy.applicable_category);
}

TransactionErrorAnalysis TransactionErrorClassifier::analyze_transaction_errors(
    const QString& transaction_id,
    const std::vector<TransactionErrorInfo>& errors) const {
    TransactionErrorAnalysis analysis;
    analysis.analysis_id = generate_analysis_id();
    analysis.transaction_id = transaction_id;
    analysis.total_errors = static_cast<int>(errors.size());

    if (errors.empty()) {
        return analysis;
    }

    // Determine primary category and max severity
    std::unordered_map<TransactionErrorCategory, int> category_counts;
    TransactionErrorSeverity max_severity = TransactionErrorSeverity::Info;

    for (const auto& error : errors) {
        category_counts[error.category]++;

        if (error.severity > max_severity) {
            max_severity = error.severity;
        }

        if (error.severity >= TransactionErrorSeverity::Critical) {
            analysis.critical_errors++;
        }

        if (error.retryable) {
            analysis.retryable_errors++;
        }

        // Collect affected operations and participants
        if (!error.operation_id.isEmpty() &&
            std::find(analysis.affected_operations.begin(),
                      analysis.affected_operations.end(), error.operation_id) ==
                analysis.affected_operations.end()) {
            analysis.affected_operations.push_back(error.operation_id);
        }

        if (!error.plugin_id.isEmpty() &&
            std::find(analysis.affected_participants.begin(),
                      analysis.affected_participants.end(), error.plugin_id) ==
                analysis.affected_participants.end()) {
            analysis.affected_participants.push_back(error.plugin_id);
        }
    }

    // Find primary category (most frequent)
    auto primary_category_it = std::max_element(
        category_counts.begin(), category_counts.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    if (primary_category_it != category_counts.end()) {
        analysis.primary_category = primary_category_it->first;
    }

    analysis.max_severity = max_severity;

    // Detect error patterns
    analysis.has_cascading_errors = detect_cascading_errors(errors);
    analysis.has_recurring_errors = detect_recurring_errors(errors);
    analysis.has_deadlock_potential = detect_deadlock_potential(errors);

    // Determine recommended action
    if (analysis.critical_errors > 0) {
        analysis.recommended_action = TransactionRecoveryAction::Abort;
        analysis.recovery_rationale =
            "Critical errors detected - abort recommended";
    } else if (analysis.has_deadlock_potential) {
        analysis.recommended_action = TransactionRecoveryAction::Rollback;
        analysis.recovery_rationale =
            "Deadlock potential detected - rollback recommended";
    } else if (analysis.retryable_errors > 0 &&
               analysis.retryable_errors == analysis.total_errors) {
        analysis.recommended_action = TransactionRecoveryAction::Retry;
        analysis.recovery_rationale =
            "All errors are retryable - retry recommended";
    } else {
        analysis.recommended_action = TransactionRecoveryAction::Rollback;
        analysis.recovery_rationale =
            "Mixed error types - rollback recommended";
    }

    analysis.recovery_confidence = calculate_recovery_confidence(analysis);

    // Generate root cause analysis
    if (analysis.has_cascading_errors) {
        analysis.root_cause_analysis =
            "Cascading failure pattern detected - likely root cause in first "
            "error";
    } else if (analysis.has_recurring_errors) {
        analysis.root_cause_analysis =
            "Recurring error pattern detected - systematic issue likely";
    } else if (analysis.primary_category ==
               TransactionErrorCategory::Resource) {
        analysis.root_cause_analysis = "Resource availability issues detected";
    } else if (analysis.primary_category == TransactionErrorCategory::Network) {
        analysis.root_cause_analysis = "Network connectivity issues detected";
    } else {
        analysis.root_cause_analysis =
            "Multiple error types - complex failure scenario";
    }

    return analysis;
}

bool TransactionErrorClassifier::detect_cascading_errors(
    const std::vector<TransactionErrorInfo>& errors) const {
    if (errors.size() < 2) {
        return false;
    }

    // Check if errors occurred in rapid succession
    for (size_t i = 1; i < errors.size(); ++i) {
        auto time_diff = errors[i].timestamp.msecsTo(errors[i - 1].timestamp);
        if (std::abs(time_diff) < 1000) {  // Within 1 second
            return true;
        }
    }

    return false;
}

bool TransactionErrorClassifier::detect_recurring_errors(
    const std::vector<TransactionErrorInfo>& errors) const {
    if (errors.size() < 2) {
        return false;
    }

    // Check for same error code appearing multiple times
    std::unordered_map<PluginErrorCode, int> error_counts;
    for (const auto& error : errors) {
        error_counts[error.error_code]++;
        if (error_counts[error.error_code] > 1) {
            return true;
        }
    }

    return false;
}

bool TransactionErrorClassifier::detect_deadlock_potential(
    const std::vector<TransactionErrorInfo>& errors) const {
    // Check for deadlock-related error categories
    for (const auto& error : errors) {
        if (error.category == TransactionErrorCategory::Deadlock ||
            error.category == TransactionErrorCategory::Concurrency ||
            error.error_code == PluginErrorCode::CircularDependency) {
            return true;
        }
    }

    return false;
}

QString TransactionErrorClassifier::generate_analysis_id() const {
    return "analysis_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

double TransactionErrorClassifier::calculate_recovery_confidence(
    const TransactionErrorAnalysis& analysis) const {
    double confidence = 0.5;  // Base confidence

    // Increase confidence for clear patterns
    if (analysis.retryable_errors == analysis.total_errors) {
        confidence += 0.3;  // All errors retryable
    }

    if (analysis.critical_errors == 0) {
        confidence += 0.2;  // No critical errors
    }

    if (!analysis.has_cascading_errors && !analysis.has_recurring_errors) {
        confidence += 0.1;  // No complex patterns
    }

    // Decrease confidence for complex scenarios
    if (analysis.has_deadlock_potential) {
        confidence -= 0.2;
    }

    if (analysis.affected_participants.size() > 3) {
        confidence -= 0.1;  // Many participants affected
    }

    return std::max(0.0, std::min(1.0, confidence));
}

// === TransactionErrorHandler Implementation ===

TransactionErrorHandler::TransactionErrorHandler(QObject* parent)
    : QObject(parent) {
    m_classifier = std::make_unique<TransactionErrorClassifier>(this);

    qCDebug(transactionErrorHandlerLog) << "Created transaction error handler";
}

TransactionErrorHandler::~TransactionErrorHandler() {
    if (m_initialized) {
        shutdown();
    }
}

qtplugin::expected<void, PluginError> TransactionErrorHandler::initialize() {
    if (m_initialized) {
        return make_success();
    }

    initialize_default_classification_rules();
    initialize_default_recovery_strategies();

    m_initialized = true;

    qCDebug(transactionErrorHandlerLog)
        << "Initialized transaction error handler";

    return make_success();
}

void TransactionErrorHandler::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_errors.clear();
    m_transaction_errors.clear();
    m_transaction_analyses.clear();

    m_initialized = false;

    qCDebug(transactionErrorHandlerLog) << "Shutdown transaction error handler";
}

TransactionErrorHandler& TransactionErrorHandler::instance() {
    static TransactionErrorHandler handler;
    return handler;
}

void TransactionErrorHandler::initialize_default_classification_rules() {
    // Default classification rules are already registered in the constructor
    // This method is a placeholder for future extensibility
    qCDebug(transactionErrorHandlerLog)
        << "Initialized default classification rules";
}

void TransactionErrorHandler::initialize_default_recovery_strategies() {
    // Default recovery strategies are already registered in the constructor
    // This method is a placeholder for future extensibility
    qCDebug(transactionErrorHandlerLog)
        << "Initialized default recovery strategies";
}

}  // namespace qtplugin::workflow::transactions
