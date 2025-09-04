# Transaction Manager

The Transaction Manager provides ACID transaction support for plugin operations, ensuring data consistency and rollback capabilities.

## Overview

The Transaction Manager enables:
- **Atomic Operations**: All-or-nothing transaction execution
- **Consistency**: Maintain data integrity across operations
- **Isolation**: Prevent interference between concurrent transactions
- **Durability**: Persist changes reliably
- **Rollback Support**: Undo operations on failure

## Class Reference

### TransactionManager

```cpp
class TransactionManager {
public:
    // Transaction lifecycle
    TransactionId beginTransaction();
    bool commitTransaction(TransactionId id);
    bool rollbackTransaction(TransactionId id);
    
    // Transaction operations
    bool addOperation(TransactionId id, std::unique_ptr<TransactionOperation> operation);
    bool executeTransaction(TransactionId id);
    
    // Transaction state
    TransactionState getTransactionState(TransactionId id) const;
    QList<TransactionId> getActiveTransactions() const;
    
    // Configuration
    void setTransactionTimeout(int timeoutMs);
    void setMaxConcurrentTransactions(int maxTransactions);
    
signals:
    void transactionStarted(TransactionId id);
    void transactionCommitted(TransactionId id);
    void transactionRolledBack(TransactionId id);
    void transactionFailed(TransactionId id, const QString& error);
};
```

### TransactionOperation

```cpp
class TransactionOperation {
public:
    virtual ~TransactionOperation() = default;
    
    // Execute the operation
    virtual bool execute() = 0;
    
    // Rollback the operation
    virtual bool rollback() = 0;
    
    // Get operation description
    virtual QString getDescription() const = 0;
    
    // Check if operation can be rolled back
    virtual bool canRollback() const { return true; }
};
```

## Usage Examples

### Basic Transaction

```cpp
TransactionManager* manager = TransactionManager::instance();

// Begin transaction
TransactionId txId = manager->beginTransaction();

// Add operations
auto operation1 = std::make_unique<PluginLoadOperation>("plugin1");
auto operation2 = std::make_unique<ConfigUpdateOperation>("key", "value");

manager->addOperation(txId, std::move(operation1));
manager->addOperation(txId, std::move(operation2));

// Execute transaction
if (manager->executeTransaction(txId)) {
    manager->commitTransaction(txId);
    qDebug() << "Transaction completed successfully";
} else {
    manager->rollbackTransaction(txId);
    qDebug() << "Transaction failed, rolled back";
}
```

### Custom Transaction Operation

```cpp
class CustomOperation : public TransactionOperation {
private:
    QString m_data;
    QString m_originalData;
    
public:
    CustomOperation(const QString& data) : m_data(data) {}
    
    bool execute() override {
        // Store original state for rollback
        m_originalData = getCurrentData();
        
        // Perform the operation
        return setData(m_data);
    }
    
    bool rollback() override {
        // Restore original state
        return setData(m_originalData);
    }
    
    QString getDescription() const override {
        return QString("Custom operation: %1").arg(m_data);
    }
};
```

### Transaction with Timeout

```cpp
TransactionManager* manager = TransactionManager::instance();
manager->setTransactionTimeout(5000); // 5 seconds

TransactionId txId = manager->beginTransaction();
// Add operations...

// Transaction will automatically rollback if not completed within timeout
```

## Transaction States

- **Pending**: Transaction created but not started
- **Active**: Transaction is currently executing
- **Committed**: Transaction completed successfully
- **RolledBack**: Transaction was rolled back
- **Failed**: Transaction failed and was rolled back
- **TimedOut**: Transaction exceeded timeout and was rolled back

## Error Handling

The Transaction Manager provides comprehensive error handling:

```cpp
connect(manager, &TransactionManager::transactionFailed,
        [](TransactionId id, const QString& error) {
    qWarning() << "Transaction" << id << "failed:" << error;
});
```

## Thread Safety

The Transaction Manager is fully thread-safe and supports concurrent transactions from multiple threads.

## See Also

- [Plugin Transaction Manager](../transactions/plugin-transaction-manager.md)
- [Error Handling](../utils/error-handling.md)
