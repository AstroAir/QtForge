/**
 * @file plugin_transaction_manager.hpp
 * @brief Transactional plugin operations system
 * @version 3.1.0
 * @author QtPlugin Development Team
 *
 * This file defines the transaction management system that provides
 * atomic operations across multiple plugins with transaction management,
 * commit/rollback capabilities, and consistency guarantees.
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../core/advanced_plugin_interface.hpp"
#include "../core/plugin_interface.hpp"
#include "../utils/error_handling.hpp"

namespace qtplugin::transactions {

/**
 * @brief Transaction state
 */
enum class TransactionState {
    Active,      // Transaction is active and accepting operations
    Preparing,   // Transaction is preparing to commit
    Prepared,    // Transaction is prepared and ready to commit
    Committing,  // Transaction is being committed
    Committed,   // Transaction has been committed successfully
    Aborting,    // Transaction is being aborted
    Aborted,     // Transaction has been aborted
    Failed,      // Transaction failed during commit/abort
    Timeout      // Transaction timed out
};

/**
 * @brief Transaction isolation level
 */
enum class IsolationLevel {
    ReadUncommitted,  // Lowest isolation level
    ReadCommitted,    // Read committed data only
    RepeatableRead,   // Repeatable reads within transaction
    Serializable      // Highest isolation level
};

/**
 * @brief Transaction operation type
 */
enum class OperationType {
    Read,       // Read operation
    Write,      // Write operation
    Execute,    // Command execution
    Configure,  // Configuration change
    Custom      // Custom operation
};

/**
 * @brief Transaction operation definition
 */
struct TransactionOperation {
    QString operation_id;       // Unique operation identifier
    QString plugin_id;          // Plugin that performs the operation
    OperationType type;         // Type of operation
    QString method_name;        // Method to call
    QJsonObject parameters;     // Operation parameters
    QJsonObject rollback_data;  // Data needed for rollback
    std::function<qtplugin::expected<QJsonObject, PluginError>()>
        execute_func;  // Execution function
    std::function<qtplugin::expected<void, PluginError>()>
        rollback_func;                                // Rollback function
    std::chrono::system_clock::time_point timestamp;  // Operation timestamp
    int priority{0};                                  // Operation priority

    TransactionOperation() : timestamp(std::chrono::system_clock::now()) {}
    TransactionOperation(const QString& op_id, const QString& plugin,
                         OperationType op_type)
        : operation_id(op_id),
          plugin_id(plugin),
          type(op_type),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @brief Transaction participant interface
 */
class ITransactionParticipant {
public:
    virtual ~ITransactionParticipant() = default;

    /**
     * @brief Prepare for transaction commit
     * @param transaction_id Transaction identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> prepare(
        const QString& transaction_id) = 0;

    /**
     * @brief Commit the transaction
     * @param transaction_id Transaction identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> commit(
        const QString& transaction_id) = 0;

    /**
     * @brief Abort the transaction
     * @param transaction_id Transaction identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> abort(
        const QString& transaction_id) = 0;

    /**
     * @brief Check if plugin can participate in transactions
     * @return True if plugin supports transactions
     */
    virtual bool supports_transactions() const = 0;

    /**
     * @brief Get transaction isolation level supported
     * @return Supported isolation level
     */
    virtual IsolationLevel supported_isolation_level() const {
        return IsolationLevel::ReadCommitted;
    }
};

/**
 * @brief Transaction context
 */
class TransactionContext {
public:
    TransactionContext(const QString& transaction_id,
                       IsolationLevel isolation = IsolationLevel::ReadCommitted)
        : m_transaction_id(transaction_id),
          m_state(TransactionState::Active),
          m_isolation_level(isolation),
          m_start_time(std::chrono::system_clock::now()) {}

    // === Transaction Information ===

    const QString& transaction_id() const noexcept { return m_transaction_id; }
    TransactionState state() const noexcept { return m_state; }
    IsolationLevel isolation_level() const noexcept {
        return m_isolation_level;
    }
    std::chrono::system_clock::time_point start_time() const noexcept {
        return m_start_time;
    }
    std::chrono::milliseconds timeout() const noexcept { return m_timeout; }

    void set_state(TransactionState new_state) { m_state = new_state; }
    void set_timeout(std::chrono::milliseconds timeout) { m_timeout = timeout; }

    // === Operation Management ===

    void add_operation(const TransactionOperation& operation) {
        std::lock_guard lock(m_operations_mutex);
        m_operations.push_back(operation);
    }

    std::vector<TransactionOperation> get_operations() const {
        std::lock_guard lock(m_operations_mutex);
        return m_operations;
    }

    void add_participant(const QString& plugin_id) {
        std::lock_guard lock(m_participants_mutex);
        m_participants.insert(plugin_id);
    }

    std::unordered_set<QString> get_participants() const {
        std::lock_guard lock(m_participants_mutex);
        return m_participants;
    }

    // === Data Management ===

    void set_data(const QString& key, const QJsonValue& value) {
        std::lock_guard lock(m_data_mutex);
        m_transaction_data[key] = value;
    }

    QJsonValue get_data(const QString& key) const {
        std::lock_guard lock(m_data_mutex);
        return m_transaction_data.value(key);
    }

    QJsonObject get_all_data() const {
        std::lock_guard lock(m_data_mutex);
        return m_transaction_data;
    }

    // === Savepoints ===

    void create_savepoint(const QString& savepoint_name) {
        std::lock_guard lock(m_savepoints_mutex);
        m_savepoints[savepoint_name] = m_operations.size();
    }

    bool has_savepoint(const QString& savepoint_name) const {
        std::lock_guard lock(m_savepoints_mutex);
        return m_savepoints.find(savepoint_name) != m_savepoints.end();
    }

    size_t get_savepoint_position(const QString& savepoint_name) const {
        std::lock_guard lock(m_savepoints_mutex);
        auto it = m_savepoints.find(savepoint_name);
        return it != m_savepoints.end() ? it->second : 0;
    }

private:
    QString m_transaction_id;
    TransactionState m_state;
    IsolationLevel m_isolation_level;
    std::chrono::system_clock::time_point m_start_time;
    std::chrono::milliseconds m_timeout{
        std::chrono::milliseconds{300000}};  // 5 minutes default

    mutable std::mutex m_operations_mutex;
    std::vector<TransactionOperation> m_operations;

    mutable std::mutex m_participants_mutex;
    std::unordered_set<QString> m_participants;

    mutable std::mutex m_data_mutex;
    QJsonObject m_transaction_data;

    mutable std::mutex m_savepoints_mutex;
    std::unordered_map<QString, size_t> m_savepoints;
};

/**
 * @brief Plugin transaction manager
 */
class PluginTransactionManager : public QObject {
    Q_OBJECT

public:
    static PluginTransactionManager& instance();

    // === Transaction Lifecycle ===

    qtplugin::expected<QString, PluginError> begin_transaction(
        IsolationLevel isolation = IsolationLevel::ReadCommitted,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{300000});

    qtplugin::expected<void, PluginError> commit_transaction(
        const QString& transaction_id);
    qtplugin::expected<void, PluginError> rollback_transaction(
        const QString& transaction_id);

    qtplugin::expected<void, PluginError> prepare_transaction(
        const QString& transaction_id);

    // === Transaction Operations ===

    qtplugin::expected<void, PluginError> add_operation(
        const QString& transaction_id, const TransactionOperation& operation);

    qtplugin::expected<QJsonObject, PluginError> execute_operation(
        const QString& transaction_id, const QString& operation_id);

    qtplugin::expected<void, PluginError> execute_transactional_command(
        const QString& transaction_id, const QString& plugin_id,
        const QString& method_name, const QJsonObject& parameters);

    // === Savepoints ===

    qtplugin::expected<void, PluginError> create_savepoint(
        const QString& transaction_id, const QString& savepoint_name);

    qtplugin::expected<void, PluginError> rollback_to_savepoint(
        const QString& transaction_id, const QString& savepoint_name);

    qtplugin::expected<void, PluginError> release_savepoint(
        const QString& transaction_id, const QString& savepoint_name);

    // === Transaction Information ===

    qtplugin::expected<TransactionContext, PluginError> get_transaction(
        const QString& transaction_id) const;
    std::vector<QString> list_active_transactions() const;

    qtplugin::expected<TransactionState, PluginError> get_transaction_state(
        const QString& transaction_id) const;

    // === Participant Management ===

    qtplugin::expected<void, PluginError> register_participant(
        const QString& plugin_id,
        std::shared_ptr<ITransactionParticipant> participant);

    qtplugin::expected<void, PluginError> unregister_participant(
        const QString& plugin_id);

    bool is_participant_registered(const QString& plugin_id) const;

signals:
    void transaction_started(const QString& transaction_id);
    void transaction_committed(const QString& transaction_id);
    void transaction_rolled_back(const QString& transaction_id);
    void transaction_failed(const QString& transaction_id,
                            const QString& error);
    void transaction_timeout(const QString& transaction_id);

private slots:
    void on_transaction_timeout();

private:
    PluginTransactionManager() = default;

    mutable std::shared_mutex m_transactions_mutex;
    std::unordered_map<QString, std::unique_ptr<TransactionContext>>
        m_active_transactions;

    mutable std::shared_mutex m_participants_mutex;
    std::unordered_map<QString, std::shared_ptr<ITransactionParticipant>>
        m_participants;

    std::unordered_map<QString, std::unique_ptr<QTimer>> m_timeout_timers;

    // Helper methods
    QString generate_transaction_id() const;
    qtplugin::expected<void, PluginError> validate_transaction(
        const QString& transaction_id) const;
    qtplugin::expected<void, PluginError> execute_two_phase_commit(
        TransactionContext& context);
    qtplugin::expected<void, PluginError> rollback_operations(
        const std::vector<TransactionOperation>& operations,
        size_t start_index = 0);
};

}  // namespace qtplugin::transactions

Q_DECLARE_METATYPE(qtplugin::transactions::TransactionState)
Q_DECLARE_METATYPE(qtplugin::transactions::IsolationLevel)
Q_DECLARE_METATYPE(qtplugin::transactions::OperationType)
Q_DECLARE_METATYPE(qtplugin::transactions::TransactionOperation)
