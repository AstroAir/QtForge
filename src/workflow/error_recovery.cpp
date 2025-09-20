/**
 * @file error_recovery.cpp
 * @brief Implementation of configurable error recovery strategies
 * @version 3.1.0
 */

#include "qtplugin/workflow/error_recovery.hpp"
#include <QLoggingCategory>
#include <QUuid>
#include <QRandomGenerator>
#include <QJsonArray>
#include <QJsonValue>
#include <QThread>
#include <algorithm>
#include <cmath>

Q_LOGGING_CATEGORY(workflowErrorRecoveryLog, "qtplugin.workflow.error_recovery")

namespace qtplugin::workflow::recovery {

// === RetryPolicy Implementation ===

QJsonObject RetryPolicy::to_json() const {
    QJsonObject json;
    json["max_attempts"] = max_attempts;
    json["initial_delay_ms"] = static_cast<int>(initial_delay.count());
    json["max_delay_ms"] = static_cast<int>(max_delay.count());
    json["backoff_multiplier"] = backoff_multiplier;
    json["exponential_backoff"] = exponential_backoff;
    json["jitter_enabled"] = jitter_enabled;
    json["jitter_factor"] = jitter_factor;
    
    QJsonArray retryable_errors_array;
    for (PluginErrorCode code : retryable_errors) {
        retryable_errors_array.append(static_cast<int>(code));
    }
    json["retryable_errors"] = retryable_errors_array;
    
    return json;
}

qtplugin::expected<RetryPolicy, PluginError> RetryPolicy::from_json(const QJsonObject& json) {
    RetryPolicy policy;
    
    if (json.contains("max_attempts") && json["max_attempts"].isDouble()) {
        policy.max_attempts = json["max_attempts"].toInt();
    }
    
    if (json.contains("initial_delay_ms") && json["initial_delay_ms"].isDouble()) {
        policy.initial_delay = std::chrono::milliseconds(json["initial_delay_ms"].toInt());
    }
    
    if (json.contains("max_delay_ms") && json["max_delay_ms"].isDouble()) {
        policy.max_delay = std::chrono::milliseconds(json["max_delay_ms"].toInt());
    }
    
    if (json.contains("backoff_multiplier") && json["backoff_multiplier"].isDouble()) {
        policy.backoff_multiplier = json["backoff_multiplier"].toDouble();
    }
    
    if (json.contains("exponential_backoff") && json["exponential_backoff"].isBool()) {
        policy.exponential_backoff = json["exponential_backoff"].toBool();
    }
    
    if (json.contains("jitter_enabled") && json["jitter_enabled"].isBool()) {
        policy.jitter_enabled = json["jitter_enabled"].toBool();
    }
    
    if (json.contains("jitter_factor") && json["jitter_factor"].isDouble()) {
        policy.jitter_factor = json["jitter_factor"].toDouble();
    }
    
    if (json.contains("retryable_errors") && json["retryable_errors"].isArray()) {
        QJsonArray retryable_errors_array = json["retryable_errors"].toArray();
        for (const QJsonValue& value : retryable_errors_array) {
            if (value.isDouble()) {
                policy.retryable_errors.push_back(static_cast<PluginErrorCode>(value.toInt()));
            }
        }
    }
    
    return policy;
}

// === FallbackConfig Implementation ===

QJsonObject FallbackConfig::to_json() const {
    QJsonObject json;
    json["fallback_plugin_id"] = fallback_plugin_id;
    json["fallback_method"] = fallback_method;
    json["fallback_parameters"] = fallback_parameters;
    json["preserve_original_data"] = preserve_original_data;
    json["merge_results"] = merge_results;
    
    return json;
}

qtplugin::expected<FallbackConfig, PluginError> FallbackConfig::from_json(const QJsonObject& json) {
    FallbackConfig config;
    
    if (json.contains("fallback_plugin_id") && json["fallback_plugin_id"].isString()) {
        config.fallback_plugin_id = json["fallback_plugin_id"].toString();
    }
    
    if (json.contains("fallback_method") && json["fallback_method"].isString()) {
        config.fallback_method = json["fallback_method"].toString();
    }
    
    if (json.contains("fallback_parameters") && json["fallback_parameters"].isObject()) {
        config.fallback_parameters = json["fallback_parameters"].toObject();
    }
    
    if (json.contains("preserve_original_data") && json["preserve_original_data"].isBool()) {
        config.preserve_original_data = json["preserve_original_data"].toBool();
    }
    
    if (json.contains("merge_results") && json["merge_results"].isBool()) {
        config.merge_results = json["merge_results"].toBool();
    }
    
    return config;
}

// === CircuitBreakerConfig Implementation ===

QJsonObject CircuitBreakerConfig::to_json() const {
    QJsonObject json;
    json["failure_threshold"] = failure_threshold;
    json["timeout_ms"] = static_cast<int>(timeout.count());
    json["recovery_timeout_ms"] = static_cast<int>(recovery_timeout.count());
    json["failure_rate_threshold"] = failure_rate_threshold;
    json["minimum_requests"] = minimum_requests;
    
    return json;
}

qtplugin::expected<CircuitBreakerConfig, PluginError> CircuitBreakerConfig::from_json(const QJsonObject& json) {
    CircuitBreakerConfig config;
    
    if (json.contains("failure_threshold") && json["failure_threshold"].isDouble()) {
        config.failure_threshold = json["failure_threshold"].toInt();
    }
    
    if (json.contains("timeout_ms") && json["timeout_ms"].isDouble()) {
        config.timeout = std::chrono::milliseconds(json["timeout_ms"].toInt());
    }
    
    if (json.contains("recovery_timeout_ms") && json["recovery_timeout_ms"].isDouble()) {
        config.recovery_timeout = std::chrono::milliseconds(json["recovery_timeout_ms"].toInt());
    }
    
    if (json.contains("failure_rate_threshold") && json["failure_rate_threshold"].isDouble()) {
        config.failure_rate_threshold = json["failure_rate_threshold"].toDouble();
    }
    
    if (json.contains("minimum_requests") && json["minimum_requests"].isDouble()) {
        config.minimum_requests = json["minimum_requests"].toInt();
    }
    
    return config;
}

// === GracefulDegradationConfig Implementation ===

QJsonObject GracefulDegradationConfig::to_json() const {
    QJsonObject json;
    json["degraded_plugin_id"] = degraded_plugin_id;
    json["degraded_method"] = degraded_method;
    json["degraded_parameters"] = degraded_parameters;
    json["notify_degradation"] = notify_degradation;
    json["degradation_message"] = degradation_message;
    json["target_quality"] = static_cast<int>(target_quality);
    
    return json;
}

qtplugin::expected<GracefulDegradationConfig, PluginError> GracefulDegradationConfig::from_json(const QJsonObject& json) {
    GracefulDegradationConfig config;
    
    if (json.contains("degraded_plugin_id") && json["degraded_plugin_id"].isString()) {
        config.degraded_plugin_id = json["degraded_plugin_id"].toString();
    }
    
    if (json.contains("degraded_method") && json["degraded_method"].isString()) {
        config.degraded_method = json["degraded_method"].toString();
    }
    
    if (json.contains("degraded_parameters") && json["degraded_parameters"].isObject()) {
        config.degraded_parameters = json["degraded_parameters"].toObject();
    }
    
    if (json.contains("notify_degradation") && json["notify_degradation"].isBool()) {
        config.notify_degradation = json["notify_degradation"].toBool();
    }
    
    if (json.contains("degradation_message") && json["degradation_message"].isString()) {
        config.degradation_message = json["degradation_message"].toString();
    }
    
    if (json.contains("target_quality") && json["target_quality"].isDouble()) {
        config.target_quality = static_cast<GracefulDegradationConfig::QualityLevel>(json["target_quality"].toInt());
    }
    
    return config;
}

// === ErrorRecoveryConfig Implementation ===

QJsonObject ErrorRecoveryConfig::to_json() const {
    QJsonObject json;
    json["primary_strategy"] = static_cast<int>(primary_strategy);
    json["secondary_strategy"] = static_cast<int>(secondary_strategy);
    json["tertiary_strategy"] = static_cast<int>(tertiary_strategy);
    json["retry_policy"] = retry_policy.to_json();
    json["fallback_config"] = fallback_config.to_json();
    json["circuit_breaker_config"] = circuit_breaker_config.to_json();
    json["degradation_config"] = degradation_config.to_json();
    json["operation_timeout_ms"] = static_cast<int>(operation_timeout.count());
    json["escalate_on_failure"] = escalate_on_failure;
    json["log_recovery_attempts"] = log_recovery_attempts;
    json["notify_on_recovery"] = notify_on_recovery;
    
    QJsonObject error_strategy_map_json;
    for (const auto& [error_code, strategy] : error_strategy_map) {
        error_strategy_map_json[QString::number(static_cast<int>(error_code))] = static_cast<int>(strategy);
    }
    json["error_strategy_map"] = error_strategy_map_json;
    
    return json;
}

qtplugin::expected<ErrorRecoveryConfig, PluginError> ErrorRecoveryConfig::from_json(const QJsonObject& json) {
    ErrorRecoveryConfig config;
    
    if (json.contains("primary_strategy") && json["primary_strategy"].isDouble()) {
        config.primary_strategy = static_cast<RecoveryStrategy>(json["primary_strategy"].toInt());
    }
    
    if (json.contains("secondary_strategy") && json["secondary_strategy"].isDouble()) {
        config.secondary_strategy = static_cast<RecoveryStrategy>(json["secondary_strategy"].toInt());
    }
    
    if (json.contains("tertiary_strategy") && json["tertiary_strategy"].isDouble()) {
        config.tertiary_strategy = static_cast<RecoveryStrategy>(json["tertiary_strategy"].toInt());
    }
    
    if (json.contains("retry_policy") && json["retry_policy"].isObject()) {
        auto retry_result = RetryPolicy::from_json(json["retry_policy"].toObject());
        if (retry_result) {
            config.retry_policy = retry_result.value();
        }
    }
    
    if (json.contains("fallback_config") && json["fallback_config"].isObject()) {
        auto fallback_result = FallbackConfig::from_json(json["fallback_config"].toObject());
        if (fallback_result) {
            config.fallback_config = fallback_result.value();
        }
    }
    
    if (json.contains("circuit_breaker_config") && json["circuit_breaker_config"].isObject()) {
        auto circuit_result = CircuitBreakerConfig::from_json(json["circuit_breaker_config"].toObject());
        if (circuit_result) {
            config.circuit_breaker_config = circuit_result.value();
        }
    }
    
    if (json.contains("degradation_config") && json["degradation_config"].isObject()) {
        auto degradation_result = GracefulDegradationConfig::from_json(json["degradation_config"].toObject());
        if (degradation_result) {
            config.degradation_config = degradation_result.value();
        }
    }
    
    if (json.contains("operation_timeout_ms") && json["operation_timeout_ms"].isDouble()) {
        config.operation_timeout = std::chrono::milliseconds(json["operation_timeout_ms"].toInt());
    }
    
    if (json.contains("escalate_on_failure") && json["escalate_on_failure"].isBool()) {
        config.escalate_on_failure = json["escalate_on_failure"].toBool();
    }
    
    if (json.contains("log_recovery_attempts") && json["log_recovery_attempts"].isBool()) {
        config.log_recovery_attempts = json["log_recovery_attempts"].toBool();
    }
    
    if (json.contains("notify_on_recovery") && json["notify_on_recovery"].isBool()) {
        config.notify_on_recovery = json["notify_on_recovery"].toBool();
    }
    
    if (json.contains("error_strategy_map") && json["error_strategy_map"].isObject()) {
        QJsonObject error_strategy_map_json = json["error_strategy_map"].toObject();
        for (auto it = error_strategy_map_json.begin(); it != error_strategy_map_json.end(); ++it) {
            if (it.value().isDouble()) {
                PluginErrorCode error_code = static_cast<PluginErrorCode>(it.key().toInt());
                RecoveryStrategy strategy = static_cast<RecoveryStrategy>(it.value().toInt());
                config.error_strategy_map[error_code] = strategy;
            }
        }
    }
    
    return config;
}

// === CircuitBreaker Implementation ===

CircuitBreaker::CircuitBreaker(const CircuitBreakerConfig& config, QObject* parent)
    : QObject(parent), m_config(config) {

    m_timeout_timer = new QTimer(this);
    m_timeout_timer->setSingleShot(true);
    connect(m_timeout_timer, &QTimer::timeout, this, &CircuitBreaker::on_timeout);

    m_recovery_timer = new QTimer(this);
    m_recovery_timer->setSingleShot(true);
    connect(m_recovery_timer, &QTimer::timeout, this, &CircuitBreaker::on_recovery_timeout);

    qCDebug(workflowErrorRecoveryLog) << "Created circuit breaker with failure threshold:" << m_config.failure_threshold;
}

bool CircuitBreaker::can_execute() const {
    return m_state != CircuitBreakerState::Open;
}

void CircuitBreaker::record_success() {
    m_success_count++;
    m_request_count++;

    if (m_state == CircuitBreakerState::HalfOpen) {
        // Successful execution in half-open state - close the circuit
        transition_to_state(CircuitBreakerState::Closed);
        m_failure_count = 0; // Reset failure count
    }

    qCDebug(workflowErrorRecoveryLog) << "Circuit breaker recorded success, state:" << static_cast<int>(m_state);
}

void CircuitBreaker::record_failure() {
    m_failure_count++;
    m_request_count++;
    m_last_failure_time = QDateTime::currentDateTime();

    if (should_open_circuit()) {
        transition_to_state(CircuitBreakerState::Open);
        m_timeout_timer->start(static_cast<int>(m_config.timeout.count()));
    }

    qCDebug(workflowErrorRecoveryLog) << "Circuit breaker recorded failure, count:" << m_failure_count << "state:" << static_cast<int>(m_state);
}

void CircuitBreaker::reset() {
    m_failure_count = 0;
    m_success_count = 0;
    m_request_count = 0;
    transition_to_state(CircuitBreakerState::Closed);

    m_timeout_timer->stop();
    m_recovery_timer->stop();

    qCDebug(workflowErrorRecoveryLog) << "Circuit breaker reset";
}

double CircuitBreaker::failure_rate() const {
    if (m_request_count < m_config.minimum_requests) {
        return 0.0;
    }

    return static_cast<double>(m_failure_count) / static_cast<double>(m_request_count);
}

void CircuitBreaker::update_config(const CircuitBreakerConfig& config) {
    m_config = config;

    qCDebug(workflowErrorRecoveryLog) << "Updated circuit breaker config";
}

void CircuitBreaker::on_timeout() {
    if (m_state == CircuitBreakerState::Open) {
        transition_to_state(CircuitBreakerState::HalfOpen);
        m_recovery_timer->start(static_cast<int>(m_config.recovery_timeout.count()));
    }
}

void CircuitBreaker::on_recovery_timeout() {
    if (m_state == CircuitBreakerState::HalfOpen) {
        // Recovery timeout expired - go back to open state
        transition_to_state(CircuitBreakerState::Open);
        m_timeout_timer->start(static_cast<int>(m_config.timeout.count()));
    }
}

void CircuitBreaker::transition_to_state(CircuitBreakerState new_state) {
    if (m_state != new_state) {
        CircuitBreakerState old_state = m_state;
        m_state = new_state;

        emit state_changed(old_state, new_state);

        switch (new_state) {
            case CircuitBreakerState::Open:
                emit circuit_opened();
                break;
            case CircuitBreakerState::Closed:
                emit circuit_closed();
                break;
            case CircuitBreakerState::HalfOpen:
                emit circuit_half_opened();
                break;
        }

        qCDebug(workflowErrorRecoveryLog) << "Circuit breaker state changed from" << static_cast<int>(old_state) << "to" << static_cast<int>(new_state);
    }
}

bool CircuitBreaker::should_open_circuit() const {
    if (m_state != CircuitBreakerState::Closed) {
        return false;
    }

    // Check failure threshold
    if (m_failure_count >= m_config.failure_threshold) {
        return true;
    }

    // Check failure rate threshold
    if (m_request_count >= m_config.minimum_requests) {
        double current_failure_rate = failure_rate();
        if (current_failure_rate >= m_config.failure_rate_threshold) {
            return true;
        }
    }

    return false;
}

// === ErrorRecoveryExecutor Implementation ===

ErrorRecoveryExecutor::ErrorRecoveryExecutor(QObject* parent)
    : QObject(parent) {

    qCDebug(workflowErrorRecoveryLog) << "Created error recovery executor";
}

qtplugin::expected<QJsonObject, PluginError> ErrorRecoveryExecutor::execute_with_recovery(
    const QString& execution_id,
    const QString& operation_id,
    std::function<qtplugin::expected<QJsonObject, PluginError>()> operation,
    const ErrorRecoveryConfig& config) {

    // Create or get recovery context
    RecoveryExecutionContext& context = m_recovery_contexts[execution_id];
    context.execution_id = execution_id;
    context.operation_id = operation_id;
    context.first_failure_time = QDateTime::currentDateTime();

    emit recovery_started(execution_id, config.primary_strategy);

    // Try primary strategy first
    auto result = try_strategy(context, operation, config.primary_strategy, config);
    if (result) {
        emit recovery_completed(execution_id, true);
        return result;
    }

    // Try secondary strategy if configured and escalation is enabled
    if (config.escalate_on_failure && config.secondary_strategy != RecoveryStrategy::None) {
        result = try_strategy(context, operation, config.secondary_strategy, config);
        if (result) {
            emit recovery_completed(execution_id, true);
            return result;
        }
    }

    // Try tertiary strategy if configured and escalation is enabled
    if (config.escalate_on_failure && config.tertiary_strategy != RecoveryStrategy::None) {
        result = try_strategy(context, operation, config.tertiary_strategy, config);
        if (result) {
            emit recovery_completed(execution_id, true);
            return result;
        }
    }

    emit recovery_completed(execution_id, false);

    // All strategies failed
    return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed, "All recovery strategies failed");
}

std::optional<RecoveryExecutionContext> ErrorRecoveryExecutor::get_recovery_context(const QString& execution_id) const {
    auto it = m_recovery_contexts.find(execution_id);
    if (it != m_recovery_contexts.end()) {
        return it->second;
    }
    return std::nullopt;
}

void ErrorRecoveryExecutor::clear_recovery_context(const QString& execution_id) {
    m_recovery_contexts.erase(execution_id);

    qCDebug(workflowErrorRecoveryLog) << "Cleared recovery context for execution:" << execution_id;
}

CircuitBreaker* ErrorRecoveryExecutor::get_circuit_breaker(const QString& operation_id) {
    auto it = m_circuit_breakers.find(operation_id);
    if (it != m_circuit_breakers.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ErrorRecoveryExecutor::register_circuit_breaker(const QString& operation_id, const CircuitBreakerConfig& config) {
    m_circuit_breakers[operation_id] = std::make_unique<CircuitBreaker>(config, this);

    qCDebug(workflowErrorRecoveryLog) << "Registered circuit breaker for operation:" << operation_id;
}

qtplugin::expected<QJsonObject, PluginError> ErrorRecoveryExecutor::try_strategy(
    RecoveryExecutionContext& context,
    std::function<qtplugin::expected<QJsonObject, PluginError>()> operation,
    RecoveryStrategy strategy,
    const ErrorRecoveryConfig& config) {

    RecoveryAttemptResult attempt;
    attempt.attempt_id = generate_attempt_id();
    attempt.strategy = strategy;
    attempt.timestamp = QDateTime::currentDateTime();

    auto start_time = std::chrono::steady_clock::now();

    qtplugin::expected<QJsonObject, PluginError> result;

    switch (strategy) {
        case RecoveryStrategy::Retry:
            result = execute_retry_strategy(context, operation, config.retry_policy);
            break;

        case RecoveryStrategy::Fallback:
            result = execute_fallback_strategy(context, config.fallback_config);
            break;

        case RecoveryStrategy::GracefulDegradation:
            result = execute_degradation_strategy(context, config.degradation_config);
            break;

        case RecoveryStrategy::Skip:
            // Skip strategy - return empty result
            result = QJsonObject();
            break;

        case RecoveryStrategy::Abort:
            result = make_error<QJsonObject>(PluginErrorCode::ExecutionFailed, "Operation aborted by recovery strategy");
            break;

        case RecoveryStrategy::None:
        default:
            result = make_error<QJsonObject>(PluginErrorCode::NotSupported, "Recovery strategy not supported");
            break;
    }

    auto end_time = std::chrono::steady_clock::now();
    attempt.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    if (result) {
        attempt.successful = true;
        attempt.result_data = result.value();
    } else {
        attempt.successful = false;
        attempt.error = result.error();
    }

    context.attempts.push_back(attempt);
    context.attempt_count++;
    context.last_attempt_time = QDateTime::currentDateTime();

    emit recovery_attempt_completed(context.execution_id, attempt);

    return result;
}

qtplugin::expected<QJsonObject, PluginError> ErrorRecoveryExecutor::execute_retry_strategy(
    RecoveryExecutionContext& /* context */,
    std::function<qtplugin::expected<QJsonObject, PluginError>()> operation,
    const RetryPolicy& policy) {

    for (int attempt = 0; attempt < policy.max_attempts; ++attempt) {
        if (attempt > 0) {
            // Calculate delay for retry
            auto delay = calculate_retry_delay(attempt, policy);

            qCDebug(workflowErrorRecoveryLog) << "Retrying operation after delay:" << delay.count() << "ms, attempt:" << (attempt + 1);

            // For simplicity, we'll use a blocking delay here
            // In a real implementation, you might want to use QTimer for non-blocking delays
            QThread::msleep(static_cast<unsigned long>(delay.count()));
        }

        auto result = operation();
        if (result) {
            qCDebug(workflowErrorRecoveryLog) << "Retry strategy succeeded on attempt:" << (attempt + 1);
            return result;
        }

        // Check if error is retryable
        if (policy.should_retry && !policy.should_retry(result.error())) {
            qCDebug(workflowErrorRecoveryLog) << "Error not retryable, stopping retry attempts";
            return result;
        }

        // Check if error code is in retryable list
        if (!policy.retryable_errors.empty()) {
            auto it = std::find(policy.retryable_errors.begin(), policy.retryable_errors.end(), result.error().code);
            if (it == policy.retryable_errors.end()) {
                qCDebug(workflowErrorRecoveryLog) << "Error code not in retryable list, stopping retry attempts";
                return result;
            }
        }

        qCDebug(workflowErrorRecoveryLog) << "Retry attempt" << (attempt + 1) << "failed, error:" << QString::fromStdString(result.error().message);
    }

    return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed, "All retry attempts failed");
}

qtplugin::expected<QJsonObject, PluginError> ErrorRecoveryExecutor::execute_fallback_strategy(
    RecoveryExecutionContext& context,
    const FallbackConfig& config) {

    if (config.fallback_func) {
        auto result = config.fallback_func(context.original_parameters);
        if (result) {
            qCDebug(workflowErrorRecoveryLog) << "Fallback strategy succeeded";
            return result;
        } else {
            qCDebug(workflowErrorRecoveryLog) << "Fallback strategy failed:" << QString::fromStdString(result.error().message);
            return result;
        }
    }

    // If no fallback function is provided, return a basic fallback result
    QJsonObject fallback_result;
    fallback_result["fallback"] = true;
    fallback_result["original_error"] = QString::fromStdString(context.original_error.message);

    if (config.preserve_original_data) {
        fallback_result["original_data"] = context.original_parameters;
    }

    qCDebug(workflowErrorRecoveryLog) << "Fallback strategy completed with default result";

    return fallback_result;
}

qtplugin::expected<QJsonObject, PluginError> ErrorRecoveryExecutor::execute_degradation_strategy(
    RecoveryExecutionContext& context,
    const GracefulDegradationConfig& config) {

    if (config.degradation_func) {
        auto result = config.degradation_func(context.original_parameters, config.target_quality);
        if (result) {
            qCDebug(workflowErrorRecoveryLog) << "Graceful degradation strategy succeeded";
            return result;
        } else {
            qCDebug(workflowErrorRecoveryLog) << "Graceful degradation strategy failed:" << QString::fromStdString(result.error().message);
            return result;
        }
    }

    // If no degradation function is provided, return a basic degraded result
    QJsonObject degraded_result;
    degraded_result["degraded"] = true;
    degraded_result["quality_level"] = static_cast<int>(config.target_quality);
    degraded_result["original_error"] = QString::fromStdString(context.original_error.message);

    if (config.notify_degradation && !config.degradation_message.isEmpty()) {
        degraded_result["degradation_message"] = config.degradation_message;
    }

    qCDebug(workflowErrorRecoveryLog) << "Graceful degradation strategy completed with default result";

    return degraded_result;
}

std::chrono::milliseconds ErrorRecoveryExecutor::calculate_retry_delay(int attempt, const RetryPolicy& policy) {
    std::chrono::milliseconds delay = policy.initial_delay;

    if (policy.exponential_backoff && attempt > 0) {
        // Calculate exponential backoff
        double multiplier = std::pow(policy.backoff_multiplier, attempt);
        delay = std::chrono::milliseconds(static_cast<long long>(delay.count() * multiplier));

        // Cap at max delay
        if (delay > policy.max_delay) {
            delay = policy.max_delay;
        }
    }

    // Add jitter if enabled
    if (policy.jitter_enabled) {
        double jitter_range = delay.count() * policy.jitter_factor;
        double jitter = (QRandomGenerator::global()->generateDouble() - 0.5) * 2.0 * jitter_range;
        delay += std::chrono::milliseconds(static_cast<long long>(jitter));

        // Ensure delay is not negative
        if (delay.count() < 0) {
            delay = std::chrono::milliseconds(0);
        }
    }

    return delay;
}

RecoveryStrategy ErrorRecoveryExecutor::select_strategy(const PluginError& error, const ErrorRecoveryConfig& config) {
    // Check if there's a specific strategy for this error code
    auto it = config.error_strategy_map.find(error.code);
    if (it != config.error_strategy_map.end()) {
        return it->second;
    }

    // Use strategy selector function if provided
    if (config.strategy_selector) {
        return config.strategy_selector(error);
    }

    // Default to primary strategy
    return config.primary_strategy;
}

QString ErrorRecoveryExecutor::generate_attempt_id() const {
    return "attempt_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

// === ErrorRecoveryManager Implementation ===

ErrorRecoveryManager::ErrorRecoveryManager(QObject* parent)
    : QObject(parent), m_initialized(false) {

    m_executor = std::make_unique<ErrorRecoveryExecutor>(this);

    qCDebug(workflowErrorRecoveryLog) << "Created error recovery manager";
}

ErrorRecoveryManager::~ErrorRecoveryManager() {
    shutdown();
    qCDebug(workflowErrorRecoveryLog) << "Destroyed error recovery manager";
}

qtplugin::expected<void, PluginError> ErrorRecoveryManager::initialize() {
    if (m_initialized) {
        return make_error<void>(PluginErrorCode::InvalidState, "Error recovery manager already initialized");
    }

    m_initialized = true;

    qCDebug(workflowErrorRecoveryLog) << "Initialized error recovery manager";
    return make_success();
}

void ErrorRecoveryManager::shutdown() {
    if (m_initialized) {
        m_recovery_configs.clear();
        m_total_attempts = 0;
        m_successful_recoveries = 0;
        m_failed_recoveries = 0;
        m_initialized = false;

        qCDebug(workflowErrorRecoveryLog) << "Shutdown error recovery manager";
    }
}

void ErrorRecoveryManager::register_recovery_config(
    const QString& operation_id,
    const ErrorRecoveryConfig& config) {

    m_recovery_configs[operation_id] = config;
    emit recovery_config_registered(operation_id);

    qCDebug(workflowErrorRecoveryLog) << "Registered recovery config for operation:" << operation_id;
}

void ErrorRecoveryManager::unregister_recovery_config(const QString& operation_id) {
    auto it = m_recovery_configs.find(operation_id);
    if (it != m_recovery_configs.end()) {
        m_recovery_configs.erase(it);
        emit recovery_config_unregistered(operation_id);

        qCDebug(workflowErrorRecoveryLog) << "Unregistered recovery config for operation:" << operation_id;
    }
}

std::optional<ErrorRecoveryConfig> ErrorRecoveryManager::get_recovery_config(const QString& operation_id) const {
    auto it = m_recovery_configs.find(operation_id);
    if (it != m_recovery_configs.end()) {
        return it->second;
    }
    return std::nullopt;
}

qtplugin::expected<QJsonObject, PluginError> ErrorRecoveryManager::execute_with_recovery(
    const QString& execution_id,
    const QString& operation_id,
    std::function<qtplugin::expected<QJsonObject, PluginError>()> operation) {

    if (!m_initialized) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState, "Error recovery manager not initialized");
    }

    auto config_it = m_recovery_configs.find(operation_id);
    if (config_it == m_recovery_configs.end()) {
        // No recovery config - execute directly
        return operation();
    }

    m_total_attempts++;

    auto result = m_executor->execute_with_recovery(execution_id, operation_id, operation, config_it->second);

    if (result) {
        m_successful_recoveries++;
    } else {
        m_failed_recoveries++;
    }

    return result;
}

ErrorRecoveryManager& ErrorRecoveryManager::instance() {
    static ErrorRecoveryManager instance;
    return instance;
}

} // namespace qtplugin::workflow::recovery
