# Transaction Management

This guide covers transaction management in QtForge, providing ACID guarantees for plugin operations and ensuring data consistency across complex workflows.

## Overview

Transaction management in QtForge provides:
- **Atomicity**: All-or-nothing execution of operations
- **Consistency**: Maintain data integrity across operations
- **Isolation**: Prevent interference between concurrent transactions
- **Durability**: Persist changes reliably
- **Rollback Support**: Undo operations on failure

## Basic Transaction Usage

### Simple Transactions

Create and execute basic transactions:

```cpp
#include <QtForge/TransactionManager>

void performBasicTransaction() {
    TransactionManager* manager = TransactionManager::instance();
    
    // Begin transaction
    TransactionId txId = manager->beginTransaction();
    
    try {
        // Add operations to transaction
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
    } catch (const std::exception& e) {
        manager->rollbackTransaction(txId);
        qWarning() << "Transaction error:" << e.what();
    }
}
```

### Transaction with Timeout

Set timeouts to prevent hanging transactions:

```cpp
void performTimedTransaction() {
    TransactionManager* manager = TransactionManager::instance();
    
    // Set transaction timeout
    manager->setTransactionTimeout(5000); // 5 seconds
    
    TransactionId txId = manager->beginTransaction();
    
    // Add operations...
    auto operation = std::make_unique<LongRunningOperation>();
    manager->addOperation(txId, std::move(operation));
    
    // Transaction will automatically rollback if timeout exceeded
    if (manager->executeTransaction(txId)) {
        manager->commitTransaction(txId);
    } else {
        // Check if timeout occurred
        if (manager->getTransactionState(txId) == TransactionState::TimedOut) {
            qWarning() << "Transaction timed out";
        }
    }
}
```

## Custom Transaction Operations

### Implementing Transaction Operations

Create custom operations that support rollback:

```cpp
class FileWriteOperation : public TransactionOperation {
public:
    FileWriteOperation(const QString& filePath, const QByteArray& data)
        : m_filePath(filePath), m_data(data) {}
    
    bool execute() override {
        // Store original file content for rollback
        QFile file(m_filePath);
        if (file.exists()) {
            if (file.open(QIODevice::ReadOnly)) {
                m_originalData = file.readAll();
                m_fileExisted = true;
            } else {
                return false; // Cannot read original file
            }
        } else {
            m_fileExisted = false;
        }
        
        // Write new data
        if (file.open(QIODevice::WriteOnly)) {
            qint64 written = file.write(m_data);
            return written == m_data.size();
        }
        
        return false;
    }
    
    bool rollback() override {
        QFile file(m_filePath);
        
        if (m_fileExisted) {
            // Restore original content
            if (file.open(QIODevice::WriteOnly)) {
                return file.write(m_originalData) == m_originalData.size();
            }
        } else {
            // Remove file if it didn't exist before
            return file.remove();
        }
        
        return false;
    }
    
    QString getDescription() const override {
        return QString("Write file: %1").arg(m_filePath);
    }
    
private:
    QString m_filePath;
    QByteArray m_data;
    QByteArray m_originalData;
    bool m_fileExisted = false;
};
```

### Database Transaction Operation

Implement database operations with transaction support:

```cpp
class DatabaseOperation : public TransactionOperation {
public:
    DatabaseOperation(const QString& connectionName, const QString& query, 
                     const QVariantList& parameters = {})
        : m_connectionName(connectionName)
        , m_query(query)
        , m_parameters(parameters) {}
    
    bool execute() override {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        if (!db.isOpen()) {
            m_errorString = "Database not open";
            return false;
        }
        
        // Start database transaction
        if (!db.transaction()) {
            m_errorString = "Failed to start database transaction";
            return false;
        }
        
        QSqlQuery sqlQuery(db);
        sqlQuery.prepare(m_query);
        
        // Bind parameters
        for (int i = 0; i < m_parameters.size(); ++i) {
            sqlQuery.bindValue(i, m_parameters[i]);
        }
        
        if (sqlQuery.exec()) {
            m_dbTransactionStarted = true;
            return true;
        } else {
            m_errorString = sqlQuery.lastError().text();
            db.rollback();
            return false;
        }
    }
    
    bool rollback() override {
        if (m_dbTransactionStarted) {
            QSqlDatabase db = QSqlDatabase::database(m_connectionName);
            return db.rollback();
        }
        return true;
    }
    
    bool commit() {
        if (m_dbTransactionStarted) {
            QSqlDatabase db = QSqlDatabase::database(m_connectionName);
            bool result = db.commit();
            m_dbTransactionStarted = false;
            return result;
        }
        return true;
    }
    
    QString getDescription() const override {
        return QString("Database operation: %1").arg(m_query);
    }
    
    QString getErrorString() const { return m_errorString; }
    
private:
    QString m_connectionName;
    QString m_query;
    QVariantList m_parameters;
    bool m_dbTransactionStarted = false;
    QString m_errorString;
};
```

## Nested Transactions

### Savepoint Support

Implement nested transactions using savepoints:

```cpp
class NestedTransactionManager {
public:
    class Savepoint {
    public:
        Savepoint(const QString& name, TransactionId parentTx)
            : m_name(name), m_parentTransaction(parentTx) {}
        
        QString getName() const { return m_name; }
        TransactionId getParentTransaction() const { return m_parentTransaction; }
        
    private:
        QString m_name;
        TransactionId m_parentTransaction;
    };
    
    Savepoint createSavepoint(TransactionId parentTx, const QString& name) {
        Savepoint savepoint(name, parentTx);
        m_savepoints[parentTx].append(savepoint);
        
        // Create savepoint in all operations that support it
        auto operations = getTransactionOperations(parentTx);
        for (auto* operation : operations) {
            if (auto* savepointOp = dynamic_cast<SavepointOperation*>(operation)) {
                savepointOp->createSavepoint(name);
            }
        }
        
        return savepoint;
    }
    
    bool rollbackToSavepoint(const Savepoint& savepoint) {
        TransactionId txId = savepoint.getParentTransaction();
        QString savepointName = savepoint.getName();
        
        auto operations = getTransactionOperations(txId);
        for (auto* operation : operations) {
            if (auto* savepointOp = dynamic_cast<SavepointOperation*>(operation)) {
                if (!savepointOp->rollbackToSavepoint(savepointName)) {
                    return false;
                }
            }
        }
        
        // Remove savepoints created after this one
        auto& savepoints = m_savepoints[txId];
        auto it = std::find_if(savepoints.begin(), savepoints.end(),
            [&savepointName](const Savepoint& sp) {
                return sp.getName() == savepointName;
            });
        
        if (it != savepoints.end()) {
            savepoints.erase(it + 1, savepoints.end());
        }
        
        return true;
    }
    
private:
    QMap<TransactionId, QList<Savepoint>> m_savepoints;
    
    QList<TransactionOperation*> getTransactionOperations(TransactionId txId);
};

// Savepoint-aware operation
class SavepointOperation : public TransactionOperation {
public:
    virtual bool createSavepoint(const QString& name) = 0;
    virtual bool rollbackToSavepoint(const QString& name) = 0;
    virtual bool releaseSavepoint(const QString& name) = 0;
};
```

## Distributed Transactions

### Two-Phase Commit Protocol

Implement distributed transactions across multiple resources:

```cpp
class DistributedTransactionManager {
public:
    enum Phase {
        Prepare,
        Commit,
        Abort
    };
    
    struct Participant {
        QString id;
        TransactionParticipant* participant;
        bool prepared = false;
        bool committed = false;
    };
    
    class TransactionParticipant {
    public:
        virtual ~TransactionParticipant() = default;
        virtual bool prepare(TransactionId txId) = 0;
        virtual bool commit(TransactionId txId) = 0;
        virtual bool abort(TransactionId txId) = 0;
    };
    
    TransactionId beginDistributedTransaction() {
        TransactionId txId = generateTransactionId();
        m_distributedTransactions[txId] = QList<Participant>();
        return txId;
    }
    
    void addParticipant(TransactionId txId, const QString& participantId, 
                       TransactionParticipant* participant) {
        Participant p;
        p.id = participantId;
        p.participant = participant;
        m_distributedTransactions[txId].append(p);
    }
    
    bool commitDistributedTransaction(TransactionId txId) {
        auto& participants = m_distributedTransactions[txId];
        
        // Phase 1: Prepare
        for (auto& participant : participants) {
            if (!participant.participant->prepare(txId)) {
                // Abort transaction
                abortDistributedTransaction(txId);
                return false;
            }
            participant.prepared = true;
        }
        
        // Phase 2: Commit
        bool allCommitted = true;
        for (auto& participant : participants) {
            if (!participant.participant->commit(txId)) {
                allCommitted = false;
                // Continue trying to commit other participants
            } else {
                participant.committed = true;
            }
        }
        
        if (!allCommitted) {
            // Handle partial commit scenario
            handlePartialCommit(txId);
        }
        
        m_distributedTransactions.remove(txId);
        return allCommitted;
    }
    
    void abortDistributedTransaction(TransactionId txId) {
        auto& participants = m_distributedTransactions[txId];
        
        for (auto& participant : participants) {
            participant.participant->abort(txId);
        }
        
        m_distributedTransactions.remove(txId);
    }
    
private:
    QMap<TransactionId, QList<Participant>> m_distributedTransactions;
    
    TransactionId generateTransactionId();
    void handlePartialCommit(TransactionId txId);
};
```

## Transaction Monitoring

### Transaction Metrics

Monitor transaction performance and health:

```cpp
class TransactionMonitor : public QObject {
    Q_OBJECT
    
public:
    struct TransactionMetrics {
        TransactionId id;
        QDateTime startTime;
        QDateTime endTime;
        TransactionState finalState;
        int operationCount;
        qint64 executionTimeMs;
        QString errorMessage;
    };
    
    void recordTransactionStart(TransactionId txId) {
        TransactionMetrics metrics;
        metrics.id = txId;
        metrics.startTime = QDateTime::currentDateTime();
        m_activeTransactions[txId] = metrics;
    }
    
    void recordTransactionEnd(TransactionId txId, TransactionState finalState, 
                            const QString& errorMessage = QString()) {
        auto it = m_activeTransactions.find(txId);
        if (it != m_activeTransactions.end()) {
            it->endTime = QDateTime::currentDateTime();
            it->finalState = finalState;
            it->executionTimeMs = it->startTime.msecsTo(it->endTime);
            it->errorMessage = errorMessage;
            
            m_completedTransactions.append(*it);
            m_activeTransactions.erase(it);
            
            emit transactionCompleted(*it);
            updateStatistics(*it);
        }
    }
    
    QList<TransactionMetrics> getActiveTransactions() const {
        return m_activeTransactions.values();
    }
    
    QList<TransactionMetrics> getCompletedTransactions(int limit = 100) const {
        return m_completedTransactions.mid(
            qMax(0, m_completedTransactions.size() - limit));
    }
    
    double getAverageExecutionTime() const {
        if (m_completedTransactions.isEmpty()) return 0.0;
        
        qint64 total = 0;
        for (const auto& metrics : m_completedTransactions) {
            total += metrics.executionTimeMs;
        }
        
        return static_cast<double>(total) / m_completedTransactions.size();
    }
    
    double getSuccessRate() const {
        if (m_completedTransactions.isEmpty()) return 0.0;
        
        int successful = 0;
        for (const auto& metrics : m_completedTransactions) {
            if (metrics.finalState == TransactionState::Committed) {
                successful++;
            }
        }
        
        return static_cast<double>(successful) / m_completedTransactions.size();
    }
    
signals:
    void transactionCompleted(const TransactionMetrics& metrics);
    void longRunningTransactionDetected(TransactionId txId, qint64 durationMs);
    void transactionFailureRateHigh(double failureRate);
    
private:
    QMap<TransactionId, TransactionMetrics> m_activeTransactions;
    QList<TransactionMetrics> m_completedTransactions;
    
    void updateStatistics(const TransactionMetrics& metrics);
};
```

## Best Practices

### Transaction Design Guidelines

1. **Keep Transactions Short**: Minimize transaction duration to reduce lock contention
2. **Design for Rollback**: Ensure all operations can be properly rolled back
3. **Handle Timeouts**: Set appropriate timeouts for all transactions
4. **Monitor Performance**: Track transaction metrics and performance
5. **Test Failure Scenarios**: Test rollback and error handling thoroughly

### Error Handling

```cpp
class TransactionErrorHandler {
public:
    enum ErrorType {
        TimeoutError,
        ResourceError,
        ConcurrencyError,
        ValidationError,
        SystemError
    };
    
    void handleTransactionError(TransactionId txId, ErrorType errorType, 
                              const QString& errorMessage) {
        switch (errorType) {
        case TimeoutError:
            handleTimeoutError(txId);
            break;
        case ResourceError:
            handleResourceError(txId, errorMessage);
            break;
        case ConcurrencyError:
            handleConcurrencyError(txId);
            break;
        case ValidationError:
            handleValidationError(txId, errorMessage);
            break;
        case SystemError:
            handleSystemError(txId, errorMessage);
            break;
        }
    }
    
private:
    void handleTimeoutError(TransactionId txId) {
        // Implement timeout-specific error handling
        qWarning() << "Transaction timeout:" << txId;
        // Consider retry with longer timeout
    }
    
    void handleResourceError(TransactionId txId, const QString& error) {
        // Handle resource-related errors
        qWarning() << "Resource error in transaction:" << txId << error;
        // Consider resource cleanup or alternative resources
    }
    
    void handleConcurrencyError(TransactionId txId) {
        // Handle concurrency conflicts
        qWarning() << "Concurrency error in transaction:" << txId;
        // Consider retry with backoff
    }
    
    void handleValidationError(TransactionId txId, const QString& error) {
        // Handle validation failures
        qWarning() << "Validation error in transaction:" << txId << error;
        // Log for debugging, don't retry
    }
    
    void handleSystemError(TransactionId txId, const QString& error) {
        // Handle system-level errors
        qCritical() << "System error in transaction:" << txId << error;
        // Consider system health check
    }
};
```

## See Also

- [Transaction Manager API](../api/transactions/transaction-manager.md)
- [Plugin Transaction Manager](../api/transactions/plugin-transaction-manager.md)
- [Error Handling](error-handling.md)
- [Performance Optimization](performance-optimization.md)
