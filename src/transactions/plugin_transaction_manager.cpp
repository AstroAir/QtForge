/**
 * @file plugin_transaction_manager.cpp
 * @brief Implementation of plugin transaction manager
 * @version 3.1.0
 */

#include "qtplugin/transactions/plugin_transaction_manager.hpp"
#include <QLoggingCategory>
#include <QUuid>
#include <algorithm>
#include "qtplugin/core/plugin_manager.hpp"

Q_LOGGING_CATEGORY(transactionLog, "qtplugin.transactions")

namespace qtplugin::transactions {

// === PluginTransactionManager Implementation ===

PluginTransactionManager& PluginTransactionManager::instance() {
    static PluginTransactionManager manager;
    return manager;
}

qtplugin::expected<QString, PluginError>
PluginTransactionManager::begin_transaction(IsolationLevel isolation,
                                            std::chrono::milliseconds timeout) {
    QString transaction_id = generate_transaction_id();

    auto context =
        std::make_unique<TransactionContext>(transaction_id, isolation);
    context->set_timeout(timeout);

    // Setup timeout timer
    auto timer = std::make_unique<QTimer>(this);
    timer->setSingleShot(true);
    timer->setInterval(static_cast<int>(timeout.count()));
    connect(timer.get(), &QTimer::timeout, this,
            &PluginTransactionManager::on_transaction_timeout);
    timer->start();

    {
        std::unique_lock lock(m_transactions_mutex);
        m_active_transactions[transaction_id] = std::move(context);
        m_timeout_timers[transaction_id] = std::move(timer);
    }

    emit transaction_started(transaction_id);

    qCDebug(transactionLog) << "Started transaction:" << transaction_id
                            << "isolation:" << static_cast<int>(isolation)
                            << "timeout:" << timeout.count() << "ms";

    return transaction_id;
}

qtplugin::expected<void, PluginError>
PluginTransactionManager::commit_transaction(const QString& transaction_id) {
    auto validation_result = validate_transaction(transaction_id);
    if (!validation_result) {
        return validation_result;
    }

    std::unique_lock lock(m_transactions_mutex);
    auto it = m_active_transactions.find(transaction_id);
    if (it == m_active_transactions.end()) {
        return make_error<void>(
            PluginErrorCode::PluginNotFound,
            "Transaction not found: " + transaction_id.toStdString());
    }

    auto& context = *it->second;

    if (context.state() != TransactionState::Active &&
        context.state() != TransactionState::Prepared) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Transaction not in committable state");
    }

    context.set_state(TransactionState::Committing);

    qCDebug(transactionLog) << "Committing transaction:" << transaction_id;

    // Execute two-phase commit
    auto commit_result = execute_two_phase_commit(context);

    if (commit_result) {
        context.set_state(TransactionState::Committed);

        // Cleanup
        m_timeout_timers.erase(transaction_id);
        m_active_transactions.erase(it);

        emit transaction_committed(transaction_id);

        qCDebug(transactionLog)
            << "Transaction committed successfully:" << transaction_id;
    } else {
        context.set_state(TransactionState::Failed);
        emit transaction_failed(
            transaction_id,
            QString::fromStdString(commit_result.error().message));

        qCWarning(transactionLog)
            << "Transaction commit failed:" << transaction_id << "error:"
            << QString::fromStdString(commit_result.error().message);

        return commit_result;
    }

    return make_success();
}

qtplugin::expected<void, PluginError>
PluginTransactionManager::rollback_transaction(const QString& transaction_id) {
    auto validation_result = validate_transaction(transaction_id);
    if (!validation_result) {
        return validation_result;
    }

    std::unique_lock lock(m_transactions_mutex);
    auto it = m_active_transactions.find(transaction_id);
    if (it == m_active_transactions.end()) {
        return make_error<void>(
            PluginErrorCode::PluginNotFound,
            "Transaction not found: " + transaction_id.toStdString());
    }

    auto& context = *it->second;

    if (context.state() == TransactionState::Committed) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Cannot rollback committed transaction");
    }

    context.set_state(TransactionState::Aborting);

    qCDebug(transactionLog) << "Rolling back transaction:" << transaction_id;

    // Rollback all operations in reverse order
    auto operations = context.get_operations();
    auto rollback_result = rollback_operations(operations);

    if (rollback_result) {
        context.set_state(TransactionState::Aborted);

        // Cleanup
        m_timeout_timers.erase(transaction_id);
        m_active_transactions.erase(it);

        emit transaction_rolled_back(transaction_id);

        qCDebug(transactionLog)
            << "Transaction rolled back successfully:" << transaction_id;
    } else {
        context.set_state(TransactionState::Failed);
        emit transaction_failed(
            transaction_id,
            QString::fromStdString(rollback_result.error().message));

        qCWarning(transactionLog)
            << "Transaction rollback failed:" << transaction_id << "error:"
            << QString::fromStdString(rollback_result.error().message);

        return rollback_result;
    }

    return make_success();
}

qtplugin::expected<void, PluginError>
PluginTransactionManager::prepare_transaction(const QString& transaction_id) {
    auto validation_result = validate_transaction(transaction_id);
    if (!validation_result) {
        return validation_result;
    }

    std::shared_lock lock(m_transactions_mutex);
    auto it = m_active_transactions.find(transaction_id);
    if (it == m_active_transactions.end()) {
        return make_error<void>(
            PluginErrorCode::PluginNotFound,
            "Transaction not found: " + transaction_id.toStdString());
    }

    auto& context = *it->second;

    if (context.state() != TransactionState::Active) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Transaction not in active state");
    }

    context.set_state(TransactionState::Preparing);

    qCDebug(transactionLog) << "Preparing transaction:" << transaction_id;

    // Prepare all participants
    auto participants = context.get_participants();
    std::shared_lock participants_lock(m_participants_mutex);

    for (const QString& plugin_id : participants) {
        auto participant_it = m_participants.find(plugin_id);
        if (participant_it != m_participants.end()) {
            auto prepare_result =
                participant_it->second->prepare(transaction_id);
            if (!prepare_result) {
                context.set_state(TransactionState::Failed);
                return prepare_result;
            }
        }
    }

    context.set_state(TransactionState::Prepared);

    qCDebug(transactionLog)
        << "Transaction prepared successfully:" << transaction_id;

    return make_success();
}

qtplugin::expected<void, PluginError> PluginTransactionManager::add_operation(
    const QString& transaction_id, const TransactionOperation& operation) {
    auto validation_result = validate_transaction(transaction_id);
    if (!validation_result) {
        return validation_result;
    }

    std::shared_lock lock(m_transactions_mutex);
    auto it = m_active_transactions.find(transaction_id);
    if (it == m_active_transactions.end()) {
        return make_error<void>(
            PluginErrorCode::PluginNotFound,
            "Transaction not found: " + transaction_id.toStdString());
    }

    auto& context = *it->second;

    if (context.state() != TransactionState::Active) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Transaction not in active state");
    }

    context.add_operation(operation);
    context.add_participant(operation.plugin_id);

    qCDebug(transactionLog)
        << "Added operation to transaction:" << transaction_id
        << "operation:" << operation.operation_id
        << "plugin:" << operation.plugin_id;

    return make_success();
}

qtplugin::expected<QJsonObject, PluginError>
PluginTransactionManager::execute_operation(const QString& transaction_id,
                                            const QString& operation_id) {
    auto validation_result = validate_transaction(transaction_id);
    if (!validation_result) {
        return qtplugin::unexpected<PluginError>(validation_result.error());
    }

    std::shared_lock lock(m_transactions_mutex);
    auto it = m_active_transactions.find(transaction_id);
    if (it == m_active_transactions.end()) {
        return make_error<QJsonObject>(
            PluginErrorCode::PluginNotFound,
            "Transaction not found: " + transaction_id.toStdString());
    }

    auto& context = *it->second;
    auto operations = context.get_operations();

    // Find the operation
    auto op_it = std::find_if(operations.begin(), operations.end(),
                              [&operation_id](const TransactionOperation& op) {
                                  return op.operation_id == operation_id;
                              });

    if (op_it == operations.end()) {
        return make_error<QJsonObject>(
            PluginErrorCode::PluginNotFound,
            "Operation not found: " + operation_id.toStdString());
    }

    // Execute the operation
    if (op_it->execute_func) {
        auto result = op_it->execute_func();
        if (result) {
            qCDebug(transactionLog) << "Executed operation:" << operation_id
                                    << "in transaction:" << transaction_id;
            return result.value();
        } else {
            qCWarning(transactionLog)
                << "Operation execution failed:" << operation_id
                << "error:" << QString::fromStdString(result.error().message);
            return qtplugin::unexpected<PluginError>(result.error());
        }
    }

    return make_error<QJsonObject>(PluginErrorCode::NotSupported,
                                   "Operation has no execution function");
}

QString PluginTransactionManager::generate_transaction_id() const {
    return "tx_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

qtplugin::expected<void, PluginError>
PluginTransactionManager::validate_transaction(
    const QString& transaction_id) const {
    if (transaction_id.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Transaction ID cannot be empty");
    }

    return make_success();
}

qtplugin::expected<void, PluginError>
PluginTransactionManager::execute_two_phase_commit(
    TransactionContext& context) {
    auto participants = context.get_participants();

    // Phase 1: Prepare all participants
    {
        std::shared_lock participants_lock(m_participants_mutex);

        for (const QString& plugin_id : participants) {
            auto participant_it = m_participants.find(plugin_id);
            if (participant_it != m_participants.end()) {
                auto prepare_result =
                    participant_it->second->prepare(context.transaction_id());
                if (!prepare_result) {
                    // Abort all participants that were prepared
                    for (const QString& abort_plugin_id : participants) {
                        if (abort_plugin_id == plugin_id)
                            break;

                        auto abort_participant_it =
                            m_participants.find(abort_plugin_id);
                        if (abort_participant_it != m_participants.end()) {
                            abort_participant_it->second->abort(
                                context.transaction_id());
                        }
                    }

                    return prepare_result;
                }
            }
        }
    }

    // Phase 2: Commit all participants
    {
        std::shared_lock participants_lock(m_participants_mutex);

        for (const QString& plugin_id : participants) {
            auto participant_it = m_participants.find(plugin_id);
            if (participant_it != m_participants.end()) {
                auto commit_result =
                    participant_it->second->commit(context.transaction_id());
                if (!commit_result) {
                    qCWarning(transactionLog)
                        << "Participant commit failed:" << plugin_id
                        << "transaction:" << context.transaction_id();
                    return commit_result;
                }
            }
        }
    }

    return make_success();
}

qtplugin::expected<void, PluginError>
PluginTransactionManager::rollback_operations(
    const std::vector<TransactionOperation>& operations, size_t start_index) {
    // Rollback operations in reverse order
    for (size_t i = operations.size(); i > start_index; --i) {
        const auto& operation = operations[i - 1];

        if (operation.rollback_func) {
            auto rollback_result = operation.rollback_func();
            if (!rollback_result) {
                qCWarning(transactionLog)
                    << "Operation rollback failed:" << operation.operation_id
                    << "error:"
                    << QString::fromStdString(rollback_result.error().message);
                return rollback_result;
            }
        }
    }

    return make_success();
}

qtplugin::expected<void, PluginError>
PluginTransactionManager::register_participant(
    const QString& plugin_id,
    std::shared_ptr<ITransactionParticipant> participant) {
    if (!participant) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Participant cannot be null");
    }

    std::unique_lock lock(m_participants_mutex);
    m_participants[plugin_id] = participant;

    qCDebug(transactionLog)
        << "Registered transaction participant:" << plugin_id;

    return make_success();
}

std::vector<QString> PluginTransactionManager::list_active_transactions()
    const {
    std::shared_lock lock(m_transactions_mutex);
    std::vector<QString> transactions;
    transactions.reserve(m_active_transactions.size());

    for (const auto& [transaction_id, _] : m_active_transactions) {
        transactions.push_back(transaction_id);
    }

    return transactions;
}

void PluginTransactionManager::on_transaction_timeout() {
    QTimer* timer = qobject_cast<QTimer*>(sender());
    if (!timer)
        return;

    // Find the transaction associated with this timer
    std::unique_lock lock(m_transactions_mutex);
    for (auto it = m_timeout_timers.begin(); it != m_timeout_timers.end();
         ++it) {
        if (it->second.get() == timer) {
            QString transaction_id = it->first;

            auto tx_it = m_active_transactions.find(transaction_id);
            if (tx_it != m_active_transactions.end()) {
                tx_it->second->set_state(TransactionState::Timeout);

                // Attempt to rollback the timed-out transaction
                auto operations = tx_it->second->get_operations();
                rollback_operations(operations);

                m_active_transactions.erase(tx_it);
            }

            m_timeout_timers.erase(it);

            emit transaction_timeout(transaction_id);
            qCWarning(transactionLog)
                << "Transaction timeout:" << transaction_id;

            break;
        }
    }
}

}  // namespace qtplugin::transactions
