/**
 * @file error_recovery.hpp
 * @brief Configurable error recovery strategies for workflow execution
 * @version 3.1.0
 */

#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QTimer>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>

#include "../utils/error_handling.hpp"

namespace qtplugin::workflow::recovery {

/**
 * @brief Error recovery strategy types
 */
enum class RecoveryStrategy {
    None = 0,                   // No recovery
    Retry = 1,                  // Retry operation
    Fallback = 2,               // Use fallback method
    Skip = 3,                   // Skip failed operation
    Abort = 4,                  // Abort workflow
    Compensate = 5,             // Apply compensation
    GracefulDegradation = 6,    // Continue with reduced functionality
    UserIntervention = 7,       // Require user intervention
    CircuitBreaker = 8          // Circuit breaker pattern
};

/**
 * @brief Retry policy configuration
 */
struct RetryPolicy {
    int max_attempts{3};
    std::chrono::milliseconds initial_delay{std::chrono::seconds(1)};
    std::chrono::milliseconds max_delay{std::chrono::minutes(5)};
    double backoff_multiplier{2.0};
    bool exponential_backoff{true};
    bool jitter_enabled{true};
    double jitter_factor{0.1};
    
    // Retry conditions
    std::vector<PluginErrorCode> retryable_errors;
    std::function<bool(const PluginError&)> should_retry;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<RetryPolicy, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Fallback configuration
 */
struct FallbackConfig {
    QString fallback_plugin_id;
    QString fallback_method;
    QJsonObject fallback_parameters;
    bool preserve_original_data{true};
    bool merge_results{false};
    
    // Fallback function
    std::function<qtplugin::expected<QJsonObject, PluginError>(const QJsonObject&)> fallback_func;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<FallbackConfig, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Circuit breaker configuration
 */
struct CircuitBreakerConfig {
    int failure_threshold{5};
    std::chrono::milliseconds timeout{std::chrono::minutes(1)};
    std::chrono::milliseconds recovery_timeout{std::chrono::minutes(5)};
    double failure_rate_threshold{0.5};
    int minimum_requests{10};
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<CircuitBreakerConfig, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Graceful degradation configuration
 */
struct GracefulDegradationConfig {
    QString degraded_plugin_id;
    QString degraded_method;
    QJsonObject degraded_parameters;
    bool notify_degradation{true};
    QString degradation_message;
    
    // Quality levels
    enum class QualityLevel {
        Full = 0,
        Reduced = 1,
        Minimal = 2,
        Emergency = 3
    };
    
    QualityLevel target_quality{QualityLevel::Reduced};
    
    // Degradation function
    std::function<qtplugin::expected<QJsonObject, PluginError>(const QJsonObject&, QualityLevel)> degradation_func;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<GracefulDegradationConfig, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Error recovery configuration
 */
struct ErrorRecoveryConfig {
    RecoveryStrategy primary_strategy{RecoveryStrategy::Retry};
    RecoveryStrategy secondary_strategy{RecoveryStrategy::Fallback};
    RecoveryStrategy tertiary_strategy{RecoveryStrategy::Abort};
    
    // Strategy-specific configurations
    RetryPolicy retry_policy;
    FallbackConfig fallback_config;
    CircuitBreakerConfig circuit_breaker_config;
    GracefulDegradationConfig degradation_config;
    
    // General settings
    std::chrono::milliseconds operation_timeout{std::chrono::minutes(10)};
    bool escalate_on_failure{true};
    bool log_recovery_attempts{true};
    bool notify_on_recovery{false};
    
    // Error classification
    std::unordered_map<PluginErrorCode, RecoveryStrategy> error_strategy_map;
    std::function<RecoveryStrategy(const PluginError&)> strategy_selector;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<ErrorRecoveryConfig, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Recovery attempt result
 */
struct RecoveryAttemptResult {
    QString attempt_id;
    RecoveryStrategy strategy;
    bool successful{false};
    QJsonObject result_data;
    PluginError error;
    std::chrono::milliseconds duration{0};
    QDateTime timestamp;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<RecoveryAttemptResult, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Recovery execution context
 */
struct RecoveryExecutionContext {
    QString execution_id;
    QString workflow_id;
    QString step_id;
    QString operation_id;
    QJsonObject original_parameters;
    QJsonObject shared_data;
    PluginError original_error;
    
    // Recovery state
    int attempt_count{0};
    std::vector<RecoveryAttemptResult> attempts;
    QDateTime first_failure_time;
    QDateTime last_attempt_time;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<RecoveryExecutionContext, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Circuit breaker state
 */
enum class CircuitBreakerState {
    Closed = 0,     // Normal operation
    Open = 1,       // Failing fast
    HalfOpen = 2    // Testing recovery
};

/**
 * @brief Circuit breaker implementation
 */
class CircuitBreaker : public QObject {
    Q_OBJECT

public:
    explicit CircuitBreaker(const CircuitBreakerConfig& config, QObject* parent = nullptr);
    
    // State management
    CircuitBreakerState state() const { return m_state; }
    bool can_execute() const;
    
    // Execution tracking
    void record_success();
    void record_failure();
    void reset();
    
    // Statistics
    int failure_count() const { return m_failure_count; }
    int success_count() const { return m_success_count; }
    double failure_rate() const;
    
    // Configuration
    void update_config(const CircuitBreakerConfig& config);
    const CircuitBreakerConfig& config() const { return m_config; }

signals:
    void state_changed(CircuitBreakerState old_state, CircuitBreakerState new_state);
    void circuit_opened();
    void circuit_closed();
    void circuit_half_opened();

private slots:
    void on_timeout();
    void on_recovery_timeout();

private:
    CircuitBreakerConfig m_config;
    CircuitBreakerState m_state{CircuitBreakerState::Closed};
    
    int m_failure_count{0};
    int m_success_count{0};
    int m_request_count{0};
    
    QDateTime m_last_failure_time;
    QTimer* m_timeout_timer;
    QTimer* m_recovery_timer;
    
    void transition_to_state(CircuitBreakerState new_state);
    bool should_open_circuit() const;
};

/**
 * @brief Error recovery executor
 */
class ErrorRecoveryExecutor : public QObject {
    Q_OBJECT

public:
    explicit ErrorRecoveryExecutor(QObject* parent = nullptr);
    
    // Recovery execution
    qtplugin::expected<QJsonObject, PluginError> execute_with_recovery(
        const QString& execution_id,
        const QString& operation_id,
        std::function<qtplugin::expected<QJsonObject, PluginError>()> operation,
        const ErrorRecoveryConfig& config);
    
    // Recovery context management
    std::optional<RecoveryExecutionContext> get_recovery_context(const QString& execution_id) const;
    void clear_recovery_context(const QString& execution_id);
    
    // Circuit breaker management
    CircuitBreaker* get_circuit_breaker(const QString& operation_id);
    void register_circuit_breaker(const QString& operation_id, const CircuitBreakerConfig& config);

signals:
    void recovery_started(const QString& execution_id, RecoveryStrategy strategy);
    void recovery_completed(const QString& execution_id, bool successful);
    void recovery_attempt_completed(const QString& execution_id, const RecoveryAttemptResult& result);

private:
    std::unordered_map<QString, RecoveryExecutionContext> m_recovery_contexts;
    std::unordered_map<QString, std::unique_ptr<CircuitBreaker>> m_circuit_breakers;

    // Recovery strategy implementations
    qtplugin::expected<QJsonObject, PluginError> try_strategy(
        RecoveryExecutionContext& context,
        std::function<qtplugin::expected<QJsonObject, PluginError>()> operation,
        RecoveryStrategy strategy,
        const ErrorRecoveryConfig& config);

    qtplugin::expected<QJsonObject, PluginError> execute_retry_strategy(
        RecoveryExecutionContext& context,
        std::function<qtplugin::expected<QJsonObject, PluginError>()> operation,
        const RetryPolicy& policy);

    qtplugin::expected<QJsonObject, PluginError> execute_fallback_strategy(
        RecoveryExecutionContext& context,
        const FallbackConfig& config);

    qtplugin::expected<QJsonObject, PluginError> execute_degradation_strategy(
        RecoveryExecutionContext& context,
        const GracefulDegradationConfig& config);

    // Helper methods
    std::chrono::milliseconds calculate_retry_delay(int attempt, const RetryPolicy& policy);
    RecoveryStrategy select_strategy(const PluginError& error, const ErrorRecoveryConfig& config);
    QString generate_attempt_id() const;
};

/**
 * @brief Error recovery manager
 */
class ErrorRecoveryManager : public QObject {
    Q_OBJECT

public:
    explicit ErrorRecoveryManager(QObject* parent = nullptr);
    ~ErrorRecoveryManager() override;
    
    // Service lifecycle
    qtplugin::expected<void, PluginError> initialize();
    void shutdown();
    bool is_initialized() const { return m_initialized; }
    
    // Configuration management
    void register_recovery_config(const QString& operation_id, const ErrorRecoveryConfig& config);
    void unregister_recovery_config(const QString& operation_id);
    std::optional<ErrorRecoveryConfig> get_recovery_config(const QString& operation_id) const;
    
    // Recovery execution
    qtplugin::expected<QJsonObject, PluginError> execute_with_recovery(
        const QString& execution_id,
        const QString& operation_id,
        std::function<qtplugin::expected<QJsonObject, PluginError>()> operation);
    
    // Statistics
    size_t total_recovery_attempts() const { return m_total_attempts; }
    size_t successful_recoveries() const { return m_successful_recoveries; }
    size_t failed_recoveries() const { return m_failed_recoveries; }
    
    // Singleton access
    static ErrorRecoveryManager& instance();

signals:
    void recovery_config_registered(const QString& operation_id);
    void recovery_config_unregistered(const QString& operation_id);

private:
    bool m_initialized{false};
    std::unordered_map<QString, ErrorRecoveryConfig> m_recovery_configs;
    std::unique_ptr<ErrorRecoveryExecutor> m_executor;
    
    // Statistics
    size_t m_total_attempts{0};
    size_t m_successful_recoveries{0};
    size_t m_failed_recoveries{0};
};

} // namespace qtplugin::workflow::recovery
