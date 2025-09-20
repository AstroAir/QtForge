# PluginTransactionManager API Reference

!!! info "Module Information"
**Header**: `qtplugin/workflow/transactions.hpp`
 **Namespace**: `qtplugin::workflow::transactions`
 **Since**: QtForge v3.0.0
 **Status**: Beta

## Overview

The PluginTransactionManager provides ACID transaction support for plugin operations, enabling atomic execution of multiple plugin commands with rollback capabilities. It supports various isolation levels and implements two-phase commit protocol for distributed transactions.

### Key Features

- **ACID Transactions**: Atomicity, Consistency, Isolation, and Durability guarantees
- **Multiple Isolation Levels**: ReadUncommitted, ReadCommitted, RepeatableRead, Serializable
- **Two-Phase Commit**: Distributed transaction support across multiple plugins
- **Savepoints**: Nested transaction support with partial rollback
- **Timeout Management**: Automatic transaction timeout and cleanup
- **Operation Queuing**: Sequential and prioritized operation execution

### Use Cases

- **Atomic Plugin Operations**: Execute multiple plugin commands atomically
- **Data Consistency**: Ensure data consistency across plugin boundaries
- **Rollback Scenarios**: Automatic rollback on failure with state restoration
- **Distributed Operations**: Coordinate operations across multiple plugins
- **Configuration Management**: Atomic configuration updates with rollback

## Quick Start

```cpp
#include <qtplugin/workflow/transactions.hpp>

using namespace qtplugin::workflow::transactions;

// Get transaction manager instance
auto& tx_manager = PluginTransactionManager::instance();

// Begin a transaction
auto tx_result = tx_manager.begin_transaction(IsolationLevel::ReadCommitted);
if (!tx_result) {
    qDebug() << "Failed to begin transaction:" << tx_result.error().message();
    return;
}

QString tx_id = tx_result.value();

// Create transaction operations
TransactionOperation op1("op1", "plugin1", OperationType::Write);
op1.method_name = "update_data";
op1.parameters = QJsonObject{{"key", "value1"}};

TransactionOperation op2("op2", "plugin2", OperationType::Configure);
op2.method_name = "set_config";
op2.parameters = QJsonObject{{"setting", "enabled"}};

// Add operations to transaction
tx_manager.add_operation(tx_id, op1);
tx_manager.add_operation(tx_id, op2);

// Commit transaction
auto commit_result = tx_manager.commit_transaction(tx_id);
if (commit_result) {
    qDebug() << "Transaction committed successfully";
} else {
    qDebug() << "Transaction commit failed:" << commit_result.error().message();
}
```

## Enumerations

### TransactionState

```cpp
enum class TransactionState {
    Active,      ///< Transaction is active and accepting operations
    Preparing,   ///< Transaction is in prepare phase
    Prepared,    ///< Transaction is prepared for commit
    Committed,   ///< Transaction has been committed
    Aborted,     ///< Transaction has been aborted/rolled back
    Timeout      ///< Transaction has timed out
};
```

### IsolationLevel

```cpp
enum class IsolationLevel {
    ReadUncommitted,  ///< Lowest isolation level
    ReadCommitted,    ///< Read committed data only
    RepeatableRead,   ///< Repeatable reads within transaction
    Serializable      ///< Highest isolation level
};
```

### OperationType

```cpp
enum class OperationType {
    Read,       ///< Read operation
    Write,      ///< Write operation
    Execute,    ///< Command execution
    Configure,  ///< Configuration change
    Custom      ///< Custom operation
};
```

## Data Structures

### TransactionOperation

```cpp
struct TransactionOperation {
    QString operation_id;       ///< Unique operation identifier
    QString plugin_id;          ///< Plugin that performs the operation
    OperationType type;         ///< Type of operation
    QString method_name;        ///< Method to call
    QJsonObject parameters;     ///< Operation parameters
    QJsonObject rollback_data;  ///< Data needed for rollback
    std::function<qtplugin::expected<QJsonObject, PluginError>()> execute_func;  ///< Execution function
    std::function<qtplugin::expected<void, PluginError>()> rollback_func;        ///< Rollback function
    std::chrono::system_clock::time_point timestamp;  ///< Operation timestamp
    int priority{0};            ///< Operation priority

    TransactionOperation();
    TransactionOperation(const QString& op_id, const QString& plugin, OperationType op_type);
};
```

### TransactionContext

```cpp
class TransactionContext {
public:
    TransactionContext(const QString& transaction_id, IsolationLevel isolation);

    // State management
    QString transaction_id() const noexcept;
    TransactionState state() const noexcept;
    IsolationLevel isolation_level() const noexcept;
    std::chrono::system_clock::time_point start_time() const noexcept;
    std::chrono::milliseconds timeout() const noexcept;

    void set_state(TransactionState new_state);
    void set_timeout(std::chrono::milliseconds timeout);

    // Operation management
    void add_operation(const TransactionOperation& operation);
    std::vector<TransactionOperation> get_operations() const;

    // Participant management
    void add_participant(const QString& plugin_id);
    std::unordered_set<QString> get_participants() const;

    // Data management
    void set_data(const QString& key, const QJsonValue& value);
    QJsonValue get_data(const QString& key) const;
    QJsonObject get_all_data() const;

    // Savepoint management
    void create_savepoint(const QString& name);
    bool has_savepoint(const QString& name) const;
    size_t get_savepoint_position(const QString& name) const;
    void remove_savepoint(const QString& name);
};
```

## Interface: ITransactionParticipant

Interface that plugins must implement to participate in transactions.

### Virtual Methods

#### `prepare()`

```cpp
virtual qtplugin::expected<void, PluginError> prepare(const QString& transaction_id) = 0;
```

Prepares the plugin for transaction commit (Phase 1 of two-phase commit).

#### `commit()`

```cpp
virtual qtplugin::expected<void, PluginError> commit(const QString& transaction_id) = 0;
```

Commits the transaction changes (Phase 2 of two-phase commit).

#### `abort()`

```cpp
virtual qtplugin::expected<void, PluginError> abort(const QString& transaction_id) = 0;
```

Aborts the transaction and rolls back changes.

#### `supports_transactions()`

```cpp
virtual bool supports_transactions() const = 0;
```

Returns true if the plugin supports transactions.

#### `supported_isolation_level()`

```cpp
virtual IsolationLevel supported_isolation_level() const;
```

Returns the maximum isolation level supported by the plugin.

## Class: PluginTransactionManager

Main transaction manager class implementing singleton pattern.

### Static Methods

#### `instance()`

```cpp
static PluginTransactionManager& instance();
```

Gets the singleton instance of the transaction manager.

### Transaction Lifecycle

#### `begin_transaction()`

```cpp
qtplugin::expected<QString, PluginError> begin_transaction(
    IsolationLevel isolation = IsolationLevel::ReadCommitted,
    std::chrono::milliseconds timeout = std::chrono::milliseconds{300000});
```

Begins a new transaction.

**Parameters:**

- `isolation` - Transaction isolation level
- `timeout` - Transaction timeout (default: 5 minutes)

**Returns:**

- `expected<QString, PluginError>` - Transaction ID or error

**Example:**

```cpp
auto tx_result = tx_manager.begin_transaction(IsolationLevel::Serializable,
                                             std::chrono::minutes(10));
if (tx_result) {
    QString tx_id = tx_result.value();
    // Use transaction
}
```

#### `commit_transaction()`

```cpp
qtplugin::expected<void, PluginError> commit_transaction(const QString& transaction_id);
```

Commits a transaction using two-phase commit protocol.

**Parameters:**

- `transaction_id` - Transaction identifier

**Returns:**

- `expected<void, PluginError>` - Success or error

#### `rollback_transaction()`

```cpp
qtplugin::expected<void, PluginError> rollback_transaction(const QString& transaction_id);
```

Rolls back a transaction and undoes all operations.

#### `prepare_transaction()`

```cpp
qtplugin::expected<void, PluginError> prepare_transaction(const QString& transaction_id);
```

Prepares a transaction for commit (Phase 1 of two-phase commit).

### Transaction Operations

#### `add_operation()`

```cpp
qtplugin::expected<void, PluginError> add_operation(
    const QString& transaction_id,
    const TransactionOperation& operation);
```

Adds an operation to a transaction.

**Parameters:**

- `transaction_id` - Transaction identifier
- `operation` - Operation to add

**Returns:**

- `expected<void, PluginError>` - Success or error

#### `execute_operation()`

```cpp
qtplugin::expected<QJsonObject, PluginError> execute_operation(
    const QString& transaction_id,
    const QString& operation_id);
```

Executes a specific operation within a transaction.

#### `execute_transactional_command()`

```cpp
qtplugin::expected<void, PluginError> execute_transactional_command(
    const QString& transaction_id,
    const QString& plugin_id,
    const QString& method_name,
    const QJsonObject& parameters);
```

Executes a plugin command within a transaction context.

### Savepoint Management

#### `create_savepoint()`

```cpp
qtplugin::expected<void, PluginError> create_savepoint(
    const QString& transaction_id,
    const QString& savepoint_name);
```

Creates a savepoint within a transaction for partial rollback.

#### `rollback_to_savepoint()`

```cpp
qtplugin::expected<void, PluginError> rollback_to_savepoint(
    const QString& transaction_id,
    const QString& savepoint_name);
```

Rolls back to a specific savepoint.

#### `release_savepoint()`

```cpp
qtplugin::expected<void, PluginError> release_savepoint(
    const QString& transaction_id,
    const QString& savepoint_name);
```

Releases a savepoint, making it unavailable for rollback.

### Transaction Monitoring

#### `get_transaction_status()`

```cpp
qtplugin::expected<QJsonObject, PluginError> get_transaction_status(
    const QString& transaction_id) const;
```

Gets detailed status information for a transaction.

#### `list_active_transactions()`

```cpp
std::vector<QString> list_active_transactions() const;
```

Lists all currently active transaction IDs.

#### `get_transaction_participants()`

```cpp
std::vector<QString> get_transaction_participants(const QString& transaction_id) const;
```

Gets list of plugins participating in a transaction.

### Participant Management

#### `register_participant()`

```cpp
void register_participant(const QString& plugin_id,
                         std::shared_ptr<ITransactionParticipant> participant);
```

Registers a plugin as a transaction participant.

#### `unregister_participant()`

```cpp
void unregister_participant(const QString& plugin_id);
```

Unregisters a plugin from transaction participation.

## Signals

The PluginTransactionManager emits the following Qt signals:

```cpp
signals:
    void transaction_started(const QString& transaction_id);
    void transaction_committed(const QString& transaction_id);
    void transaction_rolled_back(const QString& transaction_id);
    void transaction_failed(const QString& transaction_id, const QString& error);
    void transaction_timeout(const QString& transaction_id);
```

## Error Handling

Common error codes and their meanings:

| Error Code            | Description                       | Resolution                                 |
| --------------------- | --------------------------------- | ------------------------------------------ |
| `NotFound`            | Transaction not found             | Verify transaction ID is valid             |
| `InvalidState`        | Invalid transaction state         | Check transaction state before operation   |
| `Timeout`             | Transaction timeout               | Increase timeout or optimize operations    |
| `ParticipantNotFound` | Plugin participant not registered | Register plugin as transaction participant |
| `PrepareFailed`       | Prepare phase failed              | Check plugin prepare implementation        |
| `CommitFailed`        | Commit phase failed               | Review plugin commit logic                 |
| `RollbackFailed`      | Rollback failed                   | Check plugin rollback implementation       |

## Thread Safety

- **Thread-safe methods**: All public methods are thread-safe
- **Signal emissions**: Signals are emitted from the main thread
- **Concurrent transactions**: Multiple transactions can run concurrently
- **Participant access**: Participant registration is synchronized

## Performance Considerations

- **Memory usage**: Approximately 2-5KB per active transaction
- **CPU usage**: Low overhead for transaction management
- **Isolation levels**: Higher isolation levels may impact performance
- **Operation count**: Large numbers of operations may affect commit time

## Integration Examples

### Basic Transaction Usage

```cpp
#include <qtplugin/workflow/transactions.hpp>

class TransactionalService {
private:
    PluginTransactionManager& m_tx_manager;

public:
    TransactionalService() : m_tx_manager(PluginTransactionManager::instance()) {
        // Connect to transaction signals
        connect(&m_tx_manager, &PluginTransactionManager::transaction_committed,
                this, &TransactionalService::on_transaction_committed);

        connect(&m_tx_manager, &PluginTransactionManager::transaction_failed,
                this, &TransactionalService::on_transaction_failed);
    }

    bool update_user_profile(const QString& user_id, const QJsonObject& profile_data) {
        // Begin transaction
        auto tx_result = m_tx_manager.begin_transaction(IsolationLevel::ReadCommitted);
        if (!tx_result) {
            qWarning() << "Failed to begin transaction:" << tx_result.error().message();
            return false;
        }

        QString tx_id = tx_result.value();

        try {
            // Operation 1: Update user data
            TransactionOperation update_user("update_user", "user_service", OperationType::Write);
            update_user.method_name = "update_profile";
            update_user.parameters = QJsonObject{
                {"user_id", user_id},
                {"profile", profile_data}
            };

            auto add_result1 = m_tx_manager.add_operation(tx_id, update_user);
            if (!add_result1) {
                m_tx_manager.rollback_transaction(tx_id);
                return false;
            }

            // Operation 2: Update search index
            TransactionOperation update_index("update_index", "search_service", OperationType::Write);
            update_index.method_name = "reindex_user";
            update_index.parameters = QJsonObject{{"user_id", user_id}};

            auto add_result2 = m_tx_manager.add_operation(tx_id, update_index);
            if (!add_result2) {
                m_tx_manager.rollback_transaction(tx_id);
                return false;
            }

            // Operation 3: Send notification
            TransactionOperation send_notification("notify", "notification_service", OperationType::Execute);
            send_notification.method_name = "send_profile_updated";
            send_notification.parameters = QJsonObject{{"user_id", user_id}};

            auto add_result3 = m_tx_manager.add_operation(tx_id, send_notification);
            if (!add_result3) {
                m_tx_manager.rollback_transaction(tx_id);
                return false;
            }

            // Commit transaction
            auto commit_result = m_tx_manager.commit_transaction(tx_id);
            if (commit_result) {
                qDebug() << "User profile updated successfully in transaction:" << tx_id;
                return true;
            } else {
                qWarning() << "Transaction commit failed:" << commit_result.error().message();
                return false;
            }

        } catch (const std::exception& e) {
            qWarning() << "Exception during transaction:" << e.what();
            m_tx_manager.rollback_transaction(tx_id);
            return false;
        }
    }

private slots:
    void on_transaction_committed(const QString& transaction_id) {
        qDebug() << "Transaction committed successfully:" << transaction_id;
    }

    void on_transaction_failed(const QString& transaction_id, const QString& error) {
        qWarning() << "Transaction failed:" << transaction_id << "Error:" << error;
    }
};
```

### Savepoint Usage

```cpp
class SavepointExample {
public:
    bool complex_operation_with_savepoints() {
        auto& tx_manager = PluginTransactionManager::instance();

        auto tx_result = tx_manager.begin_transaction();
        if (!tx_result) return false;

        QString tx_id = tx_result.value();

        try {
            // Phase 1: Initial operations
            add_initial_operations(tx_id);

            // Create savepoint before risky operations
            auto sp_result = tx_manager.create_savepoint(tx_id, "before_risky_ops");
            if (!sp_result) {
                tx_manager.rollback_transaction(tx_id);
                return false;
            }

            // Phase 2: Risky operations
            if (!add_risky_operations(tx_id)) {
                // Rollback to savepoint instead of entire transaction
                auto rollback_result = tx_manager.rollback_to_savepoint(tx_id, "before_risky_ops");
                if (rollback_result) {
                    // Try alternative approach
                    add_alternative_operations(tx_id);
                } else {
                    tx_manager.rollback_transaction(tx_id);
                    return false;
                }
            }

            // Phase 3: Final operations
            add_final_operations(tx_id);

            // Release savepoint and commit
            tx_manager.release_savepoint(tx_id, "before_risky_ops");
            auto commit_result = tx_manager.commit_transaction(tx_id);
            return commit_result.has_value();

        } catch (...) {
            tx_manager.rollback_transaction(tx_id);
            return false;
        }
    }

private:
    void add_initial_operations(const QString& tx_id) { /* ... */ }
    bool add_risky_operations(const QString& tx_id) { /* ... */ return true; }
    void add_alternative_operations(const QString& tx_id) { /* ... */ }
    void add_final_operations(const QString& tx_id) { /* ... */ }
};
```

## Python Bindings

!!! note "Python Support"
This component is available in Python through the `qtforge.transactions` module.

```python
import qtforge

# Get transaction manager
tx_manager = qtforge.transactions.PluginTransactionManager.instance()

# Begin transaction
tx_result = tx_manager.begin_transaction(
    qtforge.transactions.IsolationLevel.ReadCommitted
)
if tx_result:
    tx_id = tx_result.value()

    # Create operation
    operation = qtforge.transactions.TransactionOperation()
    operation.operation_id = "op1"
    operation.plugin_id = "my_plugin"
    operation.type = qtforge.transactions.OperationType.Write
    operation.method_name = "update_data"
    operation.parameters = {"key": "value"}

    # Add operation and commit
    tx_manager.add_operation(tx_id, operation)
    commit_result = tx_manager.commit_transaction(tx_id)

    if commit_result:
        print("Transaction committed successfully")

# Helper function for atomic operations
operations = [operation1, operation2, operation3]
success = qtforge.transactions.execute_atomic_operation(
    tx_manager, operations, qtforge.transactions.IsolationLevel.Serializable
)
```

## Related Components

- **[PluginOrchestrator](../orchestration/plugin-orchestrator.md)**: Workflow transaction integration
- **[PluginManager](../core/plugin-manager.md)**: Core plugin management for participants
- **[MessageBus](../communication/message-bus.md)**: Transactional messaging support
- **[SecurityManager](../security/security-manager.md)**: Transaction security validation

## Migration Notes

### From v2.x to v3.0

- **New Features**: Savepoint support, enhanced isolation levels, timeout management
- **API Changes**: None (backward compatible)
- **Performance**: Improved two-phase commit implementation

## See Also

- [Transaction Management User Guide](../../user-guide/transaction-management.md)
- [Transaction Examples](../../examples/transaction-examples.md)
- [ACID Properties Guide](../../concepts/acid-transactions.md)
- [Architecture Overview](../../architecture/system-design.md)

---

_Last updated: December 2024 | QtForge v3.0.0_
