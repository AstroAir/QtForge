/**
 * @file transaction_error_handler.hpp
 * @brief Enhanced transaction error handling with classification and recovery
 * @version 3.1.0
 */

#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

#include "../utils/error_handling.hpp"
#include "transactions.hpp"
#include "error_recovery.hpp"

namespace qtplugin::workflow::transactions {

/**
 * @brief Transaction error classification
 */
enum class TransactionErrorCategory {
    Unknown = 0,
    Validation = 1,         // Parameter validation errors
    State = 2,              // Invalid state transitions
    Resource = 3,           // Resource unavailability
    Network = 4,            // Network/communication errors
    Timeout = 5,            // Operation timeouts
    Participant = 6,        // Participant-specific errors
    Rollback = 7,           // Rollback operation errors
    Commit = 8,             // Commit operation errors
    Prepare = 9,            // Prepare phase errors
    Deadlock = 10,          // Deadlock detection
    Concurrency = 11,       // Concurrency conflicts
    Data = 12,              // Data integrity errors
    System = 13             // System-level errors
};

/**
 * @brief Transaction error severity
 */
enum class TransactionErrorSeverity {
    Info = 0,
    Warning = 1,
    Error = 2,
    Critical = 3,
    Fatal = 4
};

/**
 * @brief Transaction error recovery action
 */
enum class TransactionRecoveryAction {
    None = 0,               // No recovery action
    Retry = 1,              // Retry the operation
    Rollback = 2,           // Rollback transaction
    Abort = 3,              // Abort transaction
    Escalate = 4,           // Escalate to higher level
    Compensate = 5,         // Apply compensation
    Ignore = 6,             // Ignore error and continue
    UserIntervention = 7    // Require user intervention
};

/**
 * @brief Detailed transaction error information
 */
struct TransactionErrorInfo {
    QString error_id;
    QString transaction_id;
    QString operation_id;
    QString plugin_id;
    
    // Error classification
    PluginErrorCode error_code;
    TransactionErrorCategory category;
    TransactionErrorSeverity severity;
    
    // Error details
    QString message;
    QString details;
    QString context;
    QJsonObject error_data;
    
    // Timing information
    QDateTime timestamp;
    std::chrono::milliseconds duration{0};
    
    // Recovery information
    TransactionRecoveryAction recommended_action;
    bool recoverable{false};
    bool retryable{false};
    int retry_count{0};
    int max_retries{3};
    
    // Related information
    std::vector<QString> related_errors;
    QString root_cause_id;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<TransactionErrorInfo, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Transaction error context
 */
struct TransactionErrorContext {
    QString transaction_id;
    TransactionState transaction_state;
    IsolationLevel isolation_level;
    
    // Operation context
    QString current_operation_id;
    QString current_plugin_id;
    QJsonObject operation_parameters;
    
    // Participants
    std::vector<QString> participants;
    QString failed_participant;
    
    // Timing
    QDateTime transaction_start_time;
    QDateTime error_occurrence_time;
    std::chrono::milliseconds timeout_duration{0};
    
    // Error history
    std::vector<TransactionErrorInfo> previous_errors;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<TransactionErrorContext, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Transaction error recovery strategy
 */
struct TransactionErrorRecoveryStrategy {
    TransactionErrorCategory applicable_category;
    TransactionRecoveryAction primary_action;
    TransactionRecoveryAction fallback_action;
    
    // Retry configuration
    int max_retry_attempts{3};
    std::chrono::milliseconds retry_delay{std::chrono::seconds(1)};
    double backoff_multiplier{2.0};
    
    // Conditions
    std::function<bool(const TransactionErrorInfo&)> should_apply;
    std::function<TransactionRecoveryAction(const TransactionErrorInfo&, const TransactionErrorContext&)> action_selector;
    
    // Recovery function
    std::function<qtplugin::expected<void, PluginError>(const TransactionErrorInfo&, const TransactionErrorContext&)> recovery_func;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<TransactionErrorRecoveryStrategy, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Transaction error analysis result
 */
struct TransactionErrorAnalysis {
    QString analysis_id;
    QString transaction_id;
    
    // Error classification
    TransactionErrorCategory primary_category;
    TransactionErrorSeverity max_severity;
    
    // Error patterns
    bool has_cascading_errors{false};
    bool has_recurring_errors{false};
    bool has_deadlock_potential{false};
    
    // Recovery recommendations
    TransactionRecoveryAction recommended_action;
    QString recovery_rationale;
    double recovery_confidence{0.0};
    
    // Error statistics
    int total_errors{0};
    int critical_errors{0};
    int retryable_errors{0};
    
    // Related information
    std::vector<QString> affected_operations;
    std::vector<QString> affected_participants;
    QString root_cause_analysis;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<TransactionErrorAnalysis, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Transaction error classifier
 */
class TransactionErrorClassifier : public QObject {
    Q_OBJECT

public:
    explicit TransactionErrorClassifier(QObject* parent = nullptr);
    
    // Error classification
    TransactionErrorCategory classify_error(const PluginError& error, const TransactionErrorContext& context) const;
    TransactionErrorSeverity determine_severity(const PluginError& error, const TransactionErrorContext& context) const;
    TransactionRecoveryAction recommend_action(const TransactionErrorInfo& error_info, const TransactionErrorContext& context) const;
    
    // Error analysis
    TransactionErrorAnalysis analyze_transaction_errors(const QString& transaction_id, const std::vector<TransactionErrorInfo>& errors) const;
    
    // Pattern detection
    bool detect_cascading_errors(const std::vector<TransactionErrorInfo>& errors) const;
    bool detect_recurring_errors(const std::vector<TransactionErrorInfo>& errors) const;
    bool detect_deadlock_potential(const std::vector<TransactionErrorInfo>& errors) const;
    
    // Configuration
    void register_classification_rule(PluginErrorCode error_code, TransactionErrorCategory category, TransactionErrorSeverity severity);
    void register_recovery_strategy(const TransactionErrorRecoveryStrategy& strategy);

private:
    std::unordered_map<PluginErrorCode, std::pair<TransactionErrorCategory, TransactionErrorSeverity>> m_classification_rules;
    std::vector<TransactionErrorRecoveryStrategy> m_recovery_strategies;
    
    // Helper methods
    QString generate_analysis_id() const;
    double calculate_recovery_confidence(const TransactionErrorAnalysis& analysis) const;
};

/**
 * @brief Enhanced transaction error handler
 */
class TransactionErrorHandler : public QObject {
    Q_OBJECT

public:
    explicit TransactionErrorHandler(QObject* parent = nullptr);
    ~TransactionErrorHandler() override;
    
    // Service lifecycle
    qtplugin::expected<void, PluginError> initialize();
    void shutdown();
    bool is_initialized() const { return m_initialized; }
    
    // Error handling
    qtplugin::expected<TransactionRecoveryAction, PluginError> handle_transaction_error(
        const QString& transaction_id,
        const PluginError& error,
        const TransactionErrorContext& context);
    
    // Error recording
    QString record_error(const QString& transaction_id, const PluginError& error, const TransactionErrorContext& context);
    void record_recovery_attempt(const QString& error_id, TransactionRecoveryAction action, bool successful);
    
    // Error retrieval
    std::vector<TransactionErrorInfo> get_transaction_errors(const QString& transaction_id) const;
    std::optional<TransactionErrorInfo> get_error_info(const QString& error_id) const;
    TransactionErrorAnalysis get_transaction_analysis(const QString& transaction_id) const;
    
    // Recovery execution
    qtplugin::expected<void, PluginError> execute_recovery_action(
        const TransactionErrorInfo& error_info,
        const TransactionErrorContext& context,
        TransactionRecoveryAction action);
    
    // Configuration
    void configure_recovery_strategy(const TransactionErrorRecoveryStrategy& strategy);
    void set_default_recovery_actions(const std::unordered_map<TransactionErrorCategory, TransactionRecoveryAction>& actions);
    
    // Statistics
    size_t total_errors_handled() const { return m_total_errors; }
    size_t successful_recoveries() const { return m_successful_recoveries; }
    size_t failed_recoveries() const { return m_failed_recoveries; }
    
    // Singleton access
    static TransactionErrorHandler& instance();

signals:
    void error_recorded(const QString& error_id, const TransactionErrorInfo& error_info);
    void recovery_attempted(const QString& error_id, TransactionRecoveryAction action);
    void recovery_completed(const QString& error_id, bool successful);
    void error_analysis_completed(const QString& transaction_id, const TransactionErrorAnalysis& analysis);

private:
    bool m_initialized{false};
    std::unique_ptr<TransactionErrorClassifier> m_classifier;
    std::unique_ptr<recovery::ErrorRecoveryManager> m_recovery_manager;
    
    // Error storage
    std::unordered_map<QString, TransactionErrorInfo> m_errors;
    std::unordered_map<QString, std::vector<QString>> m_transaction_errors; // transaction_id -> error_ids
    std::unordered_map<QString, TransactionErrorAnalysis> m_transaction_analyses;
    
    // Configuration
    std::unordered_map<TransactionErrorCategory, TransactionRecoveryAction> m_default_actions;
    std::vector<TransactionErrorRecoveryStrategy> m_recovery_strategies;
    
    // Statistics
    size_t m_total_errors{0};
    size_t m_successful_recoveries{0};
    size_t m_failed_recoveries{0};
    
    // Helper methods
    QString generate_error_id() const;
    TransactionErrorInfo create_error_info(const QString& transaction_id, const PluginError& error, const TransactionErrorContext& context);
    void initialize_default_classification_rules();
    void initialize_default_recovery_strategies();
};

} // namespace qtplugin::workflow::transactions
