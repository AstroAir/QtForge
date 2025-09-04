# Hot Reload Examples

This document provides comprehensive examples of hot reload functionality in QtForge, demonstrating how to implement and use dynamic plugin reloading without application restart.

## Overview

Hot reload enables:
- **Dynamic Plugin Updates**: Update plugins without stopping the application
- **Development Efficiency**: Faster development cycles with instant feedback
- **Zero-Downtime Updates**: Update production systems without service interruption
- **State Preservation**: Maintain application state during plugin updates
- **Rollback Support**: Revert to previous plugin versions if needed

## Basic Hot Reload

### Simple Plugin Hot Reload

```cpp
#include <QtForge/PluginHotReloadManager>
#include <QFileSystemWatcher>

class BasicHotReloadExample : public QObject {
    Q_OBJECT
    
public:
    BasicHotReloadExample(QObject* parent = nullptr) : QObject(parent) {
        setupHotReload();
    }
    
private slots:
    void onPluginFileChanged(const QString& filePath) {
        qDebug() << "Plugin file changed:" << filePath;
        
        // Get plugin ID from file path
        QString pluginId = getPluginIdFromPath(filePath);
        if (pluginId.isEmpty()) {
            return;
        }
        
        // Perform hot reload
        PluginHotReloadManager* manager = PluginHotReloadManager::instance();
        if (manager->reloadPlugin(pluginId)) {
            qDebug() << "Plugin reloaded successfully:" << pluginId;
        } else {
            qWarning() << "Failed to reload plugin:" << pluginId;
        }
    }
    
    void onPluginReloaded(const QString& pluginId, bool success) {
        if (success) {
            qDebug() << "Hot reload completed for:" << pluginId;
            emit pluginUpdated(pluginId);
        } else {
            qWarning() << "Hot reload failed for:" << pluginId;
            emit pluginUpdateFailed(pluginId);
        }
    }
    
signals:
    void pluginUpdated(const QString& pluginId);
    void pluginUpdateFailed(const QString& pluginId);
    
private:
    void setupHotReload() {
        // Watch plugin directories for changes
        QFileSystemWatcher* watcher = new QFileSystemWatcher(this);
        watcher->addPath("./plugins");
        
        connect(watcher, &QFileSystemWatcher::fileChanged,
                this, &BasicHotReloadExample::onPluginFileChanged);
        
        // Connect to hot reload manager signals
        PluginHotReloadManager* manager = PluginHotReloadManager::instance();
        connect(manager, &PluginHotReloadManager::pluginReloaded,
                this, &BasicHotReloadExample::onPluginReloaded);
    }
    
    QString getPluginIdFromPath(const QString& filePath) {
        // Extract plugin ID from file path
        QFileInfo fileInfo(filePath);
        QString baseName = fileInfo.baseName();
        
        // Assuming plugin files follow naming convention: pluginid.dll
        return baseName;
    }
};
```

### Advanced Hot Reload with State Management

```cpp
class AdvancedHotReloadExample : public QObject {
    Q_OBJECT
    
public:
    struct PluginState {
        QString pluginId;
        QVariantMap state;
        QDateTime lastUpdate;
        int version;
    };
    
    AdvancedHotReloadExample(QObject* parent = nullptr) : QObject(parent) {
        setupAdvancedHotReload();
    }
    
    bool reloadPluginWithStatePreservation(const QString& pluginId) {
        PluginHotReloadManager* manager = PluginHotReloadManager::instance();
        
        // Save current plugin state
        PluginState savedState = savePluginState(pluginId);
        
        // Perform hot reload
        bool success = manager->reloadPlugin(pluginId);
        
        if (success) {
            // Restore plugin state
            restorePluginState(savedState);
            qDebug() << "Plugin reloaded with state preservation:" << pluginId;
        } else {
            qWarning() << "Failed to reload plugin:" << pluginId;
        }
        
        return success;
    }
    
private:
    PluginState savePluginState(const QString& pluginId) {
        PluginState state;
        state.pluginId = pluginId;
        state.lastUpdate = QDateTime::currentDateTime();
        
        // Get plugin instance
        auto* plugin = PluginManager::instance()->getPlugin(pluginId);
        if (plugin) {
            // Save plugin-specific state
            if (auto* stateful = qobject_cast<StatefulPlugin*>(plugin)) {
                state.state = stateful->saveState();
                state.version = stateful->getStateVersion();
            }
        }
        
        m_savedStates[pluginId] = state;
        return state;
    }
    
    void restorePluginState(const PluginState& state) {
        // Get reloaded plugin instance
        auto* plugin = PluginManager::instance()->getPlugin(state.pluginId);
        if (plugin) {
            if (auto* stateful = qobject_cast<StatefulPlugin*>(plugin)) {
                // Check state compatibility
                if (stateful->isStateCompatible(state.version)) {
                    stateful->restoreState(state.state);
                    qDebug() << "State restored for plugin:" << state.pluginId;
                } else {
                    qWarning() << "State incompatible for plugin:" << state.pluginId;
                    // Handle state migration or reset
                    handleStateIncompatibility(state);
                }
            }
        }
    }
    
    void handleStateIncompatibility(const PluginState& state) {
        // Attempt state migration
        auto* plugin = PluginManager::instance()->getPlugin(state.pluginId);
        if (auto* migratable = qobject_cast<MigratablePlugin*>(plugin)) {
            QVariantMap migratedState = migratable->migrateState(
                state.state, state.version, migratable->getStateVersion());
            migratable->restoreState(migratedState);
        } else {
            // Reset to default state
            qWarning() << "Resetting plugin state due to incompatibility:" << state.pluginId;
        }
    }
    
    void setupAdvancedHotReload() {
        PluginHotReloadManager* manager = PluginHotReloadManager::instance();
        
        // Configure hot reload settings
        manager->setReloadStrategy(PluginHotReloadManager::GracefulReload);
        manager->setStatePreservation(true);
        manager->setRollbackOnFailure(true);
        
        connect(manager, &PluginHotReloadManager::reloadStarted,
                this, &AdvancedHotReloadExample::onReloadStarted);
        connect(manager, &PluginHotReloadManager::reloadCompleted,
                this, &AdvancedHotReloadExample::onReloadCompleted);
        connect(manager, &PluginHotReloadManager::reloadFailed,
                this, &AdvancedHotReloadExample::onReloadFailed);
    }
    
private slots:
    void onReloadStarted(const QString& pluginId) {
        qDebug() << "Hot reload started for:" << pluginId;
        emit reloadProgress(pluginId, 0);
    }
    
    void onReloadCompleted(const QString& pluginId) {
        qDebug() << "Hot reload completed for:" << pluginId;
        emit reloadProgress(pluginId, 100);
        
        // Clean up saved state
        m_savedStates.remove(pluginId);
    }
    
    void onReloadFailed(const QString& pluginId, const QString& error) {
        qWarning() << "Hot reload failed for:" << pluginId << "Error:" << error;
        emit reloadProgress(pluginId, -1);
        
        // Attempt rollback
        attemptRollback(pluginId);
    }
    
    void attemptRollback(const QString& pluginId) {
        PluginHotReloadManager* manager = PluginHotReloadManager::instance();
        if (manager->rollbackPlugin(pluginId)) {
            qDebug() << "Plugin rolled back successfully:" << pluginId;
        } else {
            qCritical() << "Failed to rollback plugin:" << pluginId;
        }
    }
    
signals:
    void reloadProgress(const QString& pluginId, int percentage);
    
private:
    QMap<QString, PluginState> m_savedStates;
};
```

## Development Hot Reload

### Development Server Integration

```cpp
class DevelopmentHotReloadServer : public QObject {
    Q_OBJECT
    
public:
    DevelopmentHotReloadServer(int port = 8080, QObject* parent = nullptr) 
        : QObject(parent), m_port(port) {
        setupServer();
    }
    
    void startServer() {
        if (m_server->listen(QHostAddress::LocalHost, m_port)) {
            qDebug() << "Hot reload server started on port:" << m_port;
        } else {
            qWarning() << "Failed to start hot reload server:" << m_server->errorString();
        }
    }
    
private slots:
    void onNewConnection() {
        QTcpSocket* socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, [this, socket]() {
            handleClientRequest(socket);
        });
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
    
    void handleClientRequest(QTcpSocket* socket) {
        QByteArray data = socket->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject request = doc.object();
        
        QString action = request["action"].toString();
        QString pluginId = request["pluginId"].toString();
        
        QJsonObject response;
        response["pluginId"] = pluginId;
        
        if (action == "reload") {
            bool success = performHotReload(pluginId);
            response["success"] = success;
            response["action"] = "reload";
            
            if (!success) {
                response["error"] = getLastError(pluginId);
            }
        } else if (action == "status") {
            response["status"] = getPluginStatus(pluginId);
            response["action"] = "status";
        }
        
        // Send response
        QJsonDocument responseDoc(response);
        socket->write(responseDoc.toJson());
        socket->flush();
    }
    
private:
    void setupServer() {
        m_server = new QTcpServer(this);
        connect(m_server, &QTcpServer::newConnection,
                this, &DevelopmentHotReloadServer::onNewConnection);
    }
    
    bool performHotReload(const QString& pluginId) {
        PluginHotReloadManager* manager = PluginHotReloadManager::instance();
        return manager->reloadPlugin(pluginId);
    }
    
    QString getPluginStatus(const QString& pluginId) {
        auto* plugin = PluginManager::instance()->getPlugin(pluginId);
        if (!plugin) {
            return "not_loaded";
        }
        
        if (plugin->isInitialized()) {
            return "running";
        } else {
            return "loaded";
        }
    }
    
    QString getLastError(const QString& pluginId) {
        PluginHotReloadManager* manager = PluginHotReloadManager::instance();
        return manager->getLastError(pluginId);
    }
    
    QTcpServer* m_server;
    int m_port;
};
```

### File Watcher with Debouncing

```cpp
class DebouncedFileWatcher : public QObject {
    Q_OBJECT
    
public:
    DebouncedFileWatcher(int debounceMs = 500, QObject* parent = nullptr)
        : QObject(parent), m_debounceMs(debounceMs) {
        
        m_debounceTimer.setSingleShot(true);
        connect(&m_debounceTimer, &QTimer::timeout,
                this, &DebouncedFileWatcher::processPendingChanges);
    }
    
    void addPath(const QString& path) {
        m_watcher.addPath(path);
    }
    
    void addPaths(const QStringList& paths) {
        m_watcher.addPaths(paths);
    }
    
private slots:
    void onFileChanged(const QString& path) {
        m_pendingChanges.insert(path);
        m_debounceTimer.start(m_debounceMs);
    }
    
    void processPendingChanges() {
        for (const QString& path : m_pendingChanges) {
            emit fileChanged(path);
        }
        m_pendingChanges.clear();
    }
    
signals:
    void fileChanged(const QString& path);
    
private:
    QFileSystemWatcher m_watcher;
    QTimer m_debounceTimer;
    QSet<QString> m_pendingChanges;
    int m_debounceMs;
};
```

## Production Hot Reload

### Safe Production Hot Reload

```cpp
class ProductionHotReloadManager : public QObject {
    Q_OBJECT
    
public:
    enum ReloadStrategy {
        BlueGreenDeployment,
        RollingUpdate,
        CanaryDeployment
    };
    
    ProductionHotReloadManager(QObject* parent = nullptr) : QObject(parent) {
        setupProductionSafeguards();
    }
    
    bool schedulePluginUpdate(const QString& pluginId, const QString& newVersion,
                            ReloadStrategy strategy = BlueGreenDeployment) {
        UpdateRequest request;
        request.pluginId = pluginId;
        request.newVersion = newVersion;
        request.strategy = strategy;
        request.scheduledTime = QDateTime::currentDateTime().addSecs(60); // 1 minute delay
        
        m_pendingUpdates.append(request);
        
        qDebug() << "Plugin update scheduled:" << pluginId << "Version:" << newVersion;
        return true;
    }
    
    bool executeScheduledUpdates() {
        QDateTime now = QDateTime::currentDateTime();
        
        for (auto it = m_pendingUpdates.begin(); it != m_pendingUpdates.end();) {
            if (it->scheduledTime <= now) {
                bool success = executeUpdate(*it);
                if (success) {
                    qDebug() << "Plugin update completed:" << it->pluginId;
                } else {
                    qWarning() << "Plugin update failed:" << it->pluginId;
                }
                
                it = m_pendingUpdates.erase(it);
            } else {
                ++it;
            }
        }
        
        return true;
    }
    
private:
    struct UpdateRequest {
        QString pluginId;
        QString newVersion;
        ReloadStrategy strategy;
        QDateTime scheduledTime;
    };
    
    void setupProductionSafeguards() {
        // Set conservative hot reload settings
        PluginHotReloadManager* manager = PluginHotReloadManager::instance();
        manager->setReloadStrategy(PluginHotReloadManager::SafeReload);
        manager->setValidationEnabled(true);
        manager->setRollbackOnFailure(true);
        manager->setHealthCheckEnabled(true);
        
        // Schedule periodic update execution
        QTimer* updateTimer = new QTimer(this);
        connect(updateTimer, &QTimer::timeout,
                this, &ProductionHotReloadManager::executeScheduledUpdates);
        updateTimer->start(30000); // Check every 30 seconds
    }
    
    bool executeUpdate(const UpdateRequest& request) {
        switch (request.strategy) {
        case BlueGreenDeployment:
            return executeBlueGreenUpdate(request);
        case RollingUpdate:
            return executeRollingUpdate(request);
        case CanaryDeployment:
            return executeCanaryUpdate(request);
        }
        return false;
    }
    
    bool executeBlueGreenUpdate(const UpdateRequest& request) {
        // Create new plugin instance (green)
        QString tempPluginId = request.pluginId + "_temp";
        
        if (!loadPluginVersion(tempPluginId, request.newVersion)) {
            return false;
        }
        
        // Validate new plugin
        if (!validatePlugin(tempPluginId)) {
            unloadPlugin(tempPluginId);
            return false;
        }
        
        // Switch traffic to new plugin
        if (!switchPluginTraffic(request.pluginId, tempPluginId)) {
            unloadPlugin(tempPluginId);
            return false;
        }
        
        // Unload old plugin (blue)
        unloadPlugin(request.pluginId);
        
        // Rename new plugin to original ID
        renamePlugin(tempPluginId, request.pluginId);
        
        return true;
    }
    
    bool executeRollingUpdate(const UpdateRequest& request) {
        // Implement rolling update strategy
        // Gradually replace plugin instances
        return true; // Simplified
    }
    
    bool executeCanaryUpdate(const UpdateRequest& request) {
        // Implement canary deployment
        // Route small percentage of traffic to new version
        return true; // Simplified
    }
    
    bool loadPluginVersion(const QString& pluginId, const QString& version);
    bool validatePlugin(const QString& pluginId);
    bool switchPluginTraffic(const QString& oldPluginId, const QString& newPluginId);
    void unloadPlugin(const QString& pluginId);
    void renamePlugin(const QString& oldId, const QString& newId);
    
    QList<UpdateRequest> m_pendingUpdates;
};
```

## Hot Reload Testing

### Automated Hot Reload Testing

```cpp
class HotReloadTestSuite : public QObject {
    Q_OBJECT
    
private slots:
    void testBasicHotReload() {
        // Create test plugin
        QString pluginId = "test_plugin";
        createTestPlugin(pluginId, "1.0.0");
        
        // Load plugin
        PluginManager* manager = PluginManager::instance();
        QVERIFY(manager->loadPlugin(pluginId));
        
        // Verify plugin is loaded
        auto* plugin = manager->getPlugin(pluginId);
        QVERIFY(plugin != nullptr);
        QCOMPARE(plugin->getVersion(), QString("1.0.0"));
        
        // Update plugin
        createTestPlugin(pluginId, "1.1.0");
        
        // Perform hot reload
        PluginHotReloadManager* reloadManager = PluginHotReloadManager::instance();
        QVERIFY(reloadManager->reloadPlugin(pluginId));
        
        // Verify plugin is updated
        plugin = manager->getPlugin(pluginId);
        QVERIFY(plugin != nullptr);
        QCOMPARE(plugin->getVersion(), QString("1.1.0"));
    }
    
    void testHotReloadWithStatePreservation() {
        QString pluginId = "stateful_test_plugin";
        createStatefulTestPlugin(pluginId);
        
        PluginManager* manager = PluginManager::instance();
        QVERIFY(manager->loadPlugin(pluginId));
        
        auto* plugin = qobject_cast<StatefulPlugin*>(manager->getPlugin(pluginId));
        QVERIFY(plugin != nullptr);
        
        // Set some state
        QVariantMap testState;
        testState["counter"] = 42;
        testState["message"] = "test message";
        plugin->setState(testState);
        
        // Perform hot reload
        PluginHotReloadManager* reloadManager = PluginHotReloadManager::instance();
        QVERIFY(reloadManager->reloadPlugin(pluginId));
        
        // Verify state is preserved
        plugin = qobject_cast<StatefulPlugin*>(manager->getPlugin(pluginId));
        QVERIFY(plugin != nullptr);
        
        QVariantMap restoredState = plugin->getState();
        QCOMPARE(restoredState["counter"].toInt(), 42);
        QCOMPARE(restoredState["message"].toString(), QString("test message"));
    }
    
    void testHotReloadFailureRollback() {
        QString pluginId = "failing_test_plugin";
        createTestPlugin(pluginId, "1.0.0");
        
        PluginManager* manager = PluginManager::instance();
        QVERIFY(manager->loadPlugin(pluginId));
        
        // Create invalid plugin update
        createInvalidTestPlugin(pluginId, "1.1.0");
        
        // Attempt hot reload (should fail)
        PluginHotReloadManager* reloadManager = PluginHotReloadManager::instance();
        QVERIFY(!reloadManager->reloadPlugin(pluginId));
        
        // Verify original plugin is still loaded
        auto* plugin = manager->getPlugin(pluginId);
        QVERIFY(plugin != nullptr);
        QCOMPARE(plugin->getVersion(), QString("1.0.0"));
    }
    
private:
    void createTestPlugin(const QString& pluginId, const QString& version);
    void createStatefulTestPlugin(const QString& pluginId);
    void createInvalidTestPlugin(const QString& pluginId, const QString& version);
};
```

## Best Practices

### Hot Reload Guidelines

1. **Test Thoroughly**: Test hot reload functionality extensively before production use
2. **Preserve State**: Implement state preservation for stateful plugins
3. **Validate Updates**: Always validate plugin updates before applying them
4. **Plan Rollback**: Have rollback strategies for failed updates
5. **Monitor Performance**: Monitor application performance during hot reloads

### Development Workflow

```cpp
// Development hot reload workflow
class DevelopmentWorkflow {
public:
    void setupDevelopmentEnvironment() {
        // 1. Enable file watching
        auto* watcher = new DebouncedFileWatcher(500);
        watcher->addPath("./plugins");
        
        // 2. Configure hot reload for development
        PluginHotReloadManager* manager = PluginHotReloadManager::instance();
        manager->setReloadStrategy(PluginHotReloadManager::FastReload);
        manager->setValidationEnabled(false); // Skip validation for speed
        manager->setStatePreservation(true);
        
        // 3. Start development server
        auto* server = new DevelopmentHotReloadServer(8080);
        server->startServer();
        
        // 4. Connect signals for immediate feedback
        connect(watcher, &DebouncedFileWatcher::fileChanged,
                this, &DevelopmentWorkflow::onPluginFileChanged);
    }
    
private slots:
    void onPluginFileChanged(const QString& filePath) {
        QString pluginId = extractPluginId(filePath);
        
        // Immediate hot reload for development
        PluginHotReloadManager::instance()->reloadPlugin(pluginId);
        
        // Notify development tools
        notifyDevelopmentTools(pluginId, "reloaded");
    }
};
```

## See Also

- [Plugin Hot Reload Manager](../api/monitoring/plugin-hot-reload-manager.md)
- [Plugin Development](../user-guide/plugin-development.md)
- [State Management](../user-guide/state-management.md)
- [Performance Optimization](../user-guide/performance-optimization.md)
