# Advanced Plugin Examples

This document provides comprehensive examples of advanced plugin development patterns and techniques in QtForge.

## Overview

These examples demonstrate sophisticated plugin architectures and advanced features:

- **Multi-threaded Plugins**: Background processing and thread management
- **Service-oriented Plugins**: Complex service implementations
- **Event-driven Plugins**: Advanced event handling and messaging
- **Data Processing Plugins**: Stream processing and transformation
- **Integration Plugins**: External system integration

## Multi-threaded Plugin Example

### Background Processing Plugin

```cpp
#include <QtForge/PluginInterface>
#include <QThread>
#include <QTimer>
#include <QMutex>

class BackgroundProcessorPlugin : public PluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.BackgroundProcessor" FILE "metadata.json")
    Q_INTERFACES(QtForge::PluginInterface)

public:
    BackgroundProcessorPlugin(QObject* parent = nullptr);
    ~BackgroundProcessorPlugin();

    // PluginInterface implementation
    bool initialize(PluginContext* context) override;
    bool shutdown() override;
    PluginMetadata getMetadata() const override;

public slots:
    void processData(const QVariant& data);
    void pauseProcessing();
    void resumeProcessing();

signals:
    void dataProcessed(const QVariant& result);
    void processingProgress(int percentage);
    void processingError(const QString& error);

private slots:
    void onWorkerFinished();
    void onProgressUpdate(int progress);

private:
    class WorkerThread;
    WorkerThread* m_workerThread;
    QMutex m_dataMutex;
    QQueue<QVariant> m_dataQueue;
    bool m_processing;
};

class BackgroundProcessorPlugin::WorkerThread : public QThread {
    Q_OBJECT

public:
    WorkerThread(BackgroundProcessorPlugin* parent);
    void addData(const QVariant& data);
    void pauseProcessing();
    void resumeProcessing();

protected:
    void run() override;

signals:
    void dataProcessed(const QVariant& result);
    void progressUpdate(int percentage);
    void errorOccurred(const QString& error);

private:
    BackgroundProcessorPlugin* m_plugin;
    QMutex m_queueMutex;
    QQueue<QVariant> m_dataQueue;
    QWaitCondition m_dataAvailable;
    bool m_paused;
    bool m_running;

    QVariant processItem(const QVariant& data);
};

// Implementation
BackgroundProcessorPlugin::BackgroundProcessorPlugin(QObject* parent)
    : PluginInterface(parent)
    , m_workerThread(nullptr)
    , m_processing(false) {
}

bool BackgroundProcessorPlugin::initialize(PluginContext* context) {
    m_workerThread = new WorkerThread(this);

    connect(m_workerThread, &WorkerThread::dataProcessed,
            this, &BackgroundProcessorPlugin::dataProcessed);
    connect(m_workerThread, &WorkerThread::progressUpdate,
            this, &BackgroundProcessorPlugin::processingProgress);
    connect(m_workerThread, &WorkerThread::errorOccurred,
            this, &BackgroundProcessorPlugin::processingError);

    m_workerThread->start();
    m_processing = true;

    return true;
}

void BackgroundProcessorPlugin::processData(const QVariant& data) {
    if (m_workerThread && m_processing) {
        m_workerThread->addData(data);
    }
}
```

## Service-oriented Plugin Example

### Microservice Plugin Architecture

```cpp
#include <QtForge/ServicePlugin>
#include <QNetworkAccessManager>
#include <QJsonDocument>

class MicroservicePlugin : public ServicePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.Microservice" FILE "metadata.json")

public:
    // Service interface
    class MicroserviceInterface : public QObject {
        Q_OBJECT

    public:
        Q_INVOKABLE QVariant callService(const QString& endpoint, const QVariantMap& params);
        Q_INVOKABLE bool isServiceAvailable() const;
        Q_INVOKABLE QStringList getAvailableEndpoints() const;
        Q_INVOKABLE void setServiceConfiguration(const QVariantMap& config);

    signals:
        void serviceResponse(const QString& endpoint, const QVariant& response);
        void serviceError(const QString& endpoint, const QString& error);
        void serviceStatusChanged(bool available);

    private:
        QNetworkAccessManager* m_networkManager;
        QString m_baseUrl;
        QStringList m_endpoints;
        QVariantMap m_configuration;
        bool m_available;

        void checkServiceHealth();
        QNetworkRequest createRequest(const QString& endpoint) const;
    };

    MicroservicePlugin(QObject* parent = nullptr);

    // ServicePlugin implementation
    QObject* getService() override;
    bool startService() override;
    bool stopService() override;
    bool isServiceRunning() const override;

private:
    MicroserviceInterface* m_service;
    QTimer* m_healthCheckTimer;
};

// Advanced service implementation
QVariant MicroservicePlugin::MicroserviceInterface::callService(
    const QString& endpoint, const QVariantMap& params) {

    if (!m_available) {
        emit serviceError(endpoint, "Service not available");
        return QVariant();
    }

    QNetworkRequest request = createRequest(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc = QJsonDocument::fromVariant(params);
    QByteArray data = doc.toJson();

    QNetworkReply* reply = m_networkManager->post(request, data);

    // Handle response asynchronously
    connect(reply, &QNetworkReply::finished, [this, endpoint, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
            emit serviceResponse(endpoint, response.toVariant());
        } else {
            emit serviceError(endpoint, reply->errorString());
        }
        reply->deleteLater();
    });

    return QVariant(); // Async operation
}
```

## Event-driven Plugin Example

### Event Processing Pipeline

```cpp
#include <QtForge/PluginInterface>
#include <QStateMachine>
#include <QState>

class EventDrivenPlugin : public PluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.EventDriven" FILE "metadata.json")

public:
    enum EventType {
        DataReceived,
        ProcessingStarted,
        ProcessingCompleted,
        ErrorOccurred,
        SystemShutdown
    };

    struct Event {
        EventType type;
        QVariantMap data;
        QDateTime timestamp;
        QString source;
    };

    EventDrivenPlugin(QObject* parent = nullptr);

    // PluginInterface implementation
    bool initialize(PluginContext* context) override;
    bool shutdown() override;

public slots:
    void handleEvent(const Event& event);
    void processEventQueue();

signals:
    void eventProcessed(const Event& event, const QVariant& result);
    void eventFailed(const Event& event, const QString& error);

private:
    void setupStateMachine();
    void processDataEvent(const Event& event);
    void handleErrorEvent(const Event& event);

    QStateMachine* m_stateMachine;
    QState* m_idleState;
    QState* m_processingState;
    QState* m_errorState;

    QQueue<Event> m_eventQueue;
    QMutex m_queueMutex;
    QTimer* m_processingTimer;

    // Event handlers
    QMap<EventType, std::function<void(const Event&)>> m_eventHandlers;
};

void EventDrivenPlugin::setupStateMachine() {
    m_stateMachine = new QStateMachine(this);

    m_idleState = new QState(m_stateMachine);
    m_processingState = new QState(m_stateMachine);
    m_errorState = new QState(m_stateMachine);

    // State transitions
    m_idleState->addTransition(this, &EventDrivenPlugin::eventProcessed, m_processingState);
    m_processingState->addTransition(this, &EventDrivenPlugin::eventProcessed, m_idleState);
    m_processingState->addTransition(this, &EventDrivenPlugin::eventFailed, m_errorState);
    m_errorState->addTransition(this, &EventDrivenPlugin::eventProcessed, m_idleState);

    // State actions
    connect(m_idleState, &QState::entered, [this]() {
        qDebug() << "Plugin entered idle state";
        processEventQueue();
    });

    connect(m_processingState, &QState::entered, [this]() {
        qDebug() << "Plugin entered processing state";
    });

    connect(m_errorState, &QState::entered, [this]() {
        qDebug() << "Plugin entered error state";
        // Implement error recovery logic
    });

    m_stateMachine->setInitialState(m_idleState);
    m_stateMachine->start();
}
```

## Data Processing Plugin Example

### Stream Processing Plugin

```cpp
#include <QtForge/PluginInterface>
#include <QDataStream>
#include <QBuffer>

class StreamProcessorPlugin : public PluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.StreamProcessor" FILE "metadata.json")

public:
    class DataStream {
    public:
        virtual ~DataStream() = default;
        virtual QByteArray read(qint64 maxSize) = 0;
        virtual qint64 write(const QByteArray& data) = 0;
        virtual bool isOpen() const = 0;
        virtual void close() = 0;
    };

    class ProcessingPipeline {
    public:
        class Stage {
        public:
            virtual ~Stage() = default;
            virtual QByteArray process(const QByteArray& input) = 0;
            virtual QString getName() const = 0;
        };

        void addStage(std::unique_ptr<Stage> stage);
        QByteArray process(const QByteArray& input);
        void clearStages();

    private:
        QList<std::unique_ptr<Stage>> m_stages;
    };

    StreamProcessorPlugin(QObject* parent = nullptr);

    // Stream processing methods
    Q_INVOKABLE void processStream(DataStream* input, DataStream* output);
    Q_INVOKABLE void addProcessingStage(const QString& stageName, const QVariantMap& config);
    Q_INVOKABLE void removeProcessingStage(const QString& stageName);
    Q_INVOKABLE QStringList getAvailableStages() const;

signals:
    void streamProcessingStarted();
    void streamProcessingProgress(qint64 bytesProcessed, qint64 totalBytes);
    void streamProcessingCompleted(qint64 totalBytesProcessed);
    void streamProcessingError(const QString& error);

private:
    ProcessingPipeline m_pipeline;
    QMap<QString, std::function<std::unique_ptr<ProcessingPipeline::Stage>(const QVariantMap&)>> m_stageFactories;

    void registerBuiltInStages();
    void processStreamChunk(const QByteArray& chunk, DataStream* output);
};

// Built-in processing stages
class CompressionStage : public StreamProcessorPlugin::ProcessingPipeline::Stage {
public:
    CompressionStage(int compressionLevel = 6) : m_compressionLevel(compressionLevel) {}

    QByteArray process(const QByteArray& input) override {
        return qCompress(input, m_compressionLevel);
    }

    QString getName() const override { return "compression"; }

private:
    int m_compressionLevel;
};

class EncryptionStage : public StreamProcessorPlugin::ProcessingPipeline::Stage {
public:
    EncryptionStage(const QByteArray& key) : m_key(key) {}

    QByteArray process(const QByteArray& input) override {
        // Implement encryption logic
        QByteArray encrypted = input;
        for (int i = 0; i < encrypted.size(); ++i) {
            encrypted[i] = encrypted[i] ^ m_key[i % m_key.size()];
        }
        return encrypted;
    }

    QString getName() const override { return "encryption"; }

private:
    QByteArray m_key;
};
```

## Integration Plugin Example

### External System Integration

```cpp
#include <QtForge/PluginInterface>
#include <QWebSocket>
#include <QSqlDatabase>

class IntegrationPlugin : public PluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.Integration" FILE "metadata.json")

public:
    class ExternalSystemConnector : public QObject {
        Q_OBJECT

    public:
        virtual ~ExternalSystemConnector() = default;
        virtual bool connect() = 0;
        virtual void disconnect() = 0;
        virtual bool isConnected() const = 0;
        virtual QVariant sendRequest(const QVariantMap& request) = 0;

    signals:
        void connected();
        void disconnected();
        void dataReceived(const QVariant& data);
        void errorOccurred(const QString& error);
    };

    class WebSocketConnector : public ExternalSystemConnector {
        Q_OBJECT

    public:
        WebSocketConnector(const QUrl& url, QObject* parent = nullptr);

        bool connect() override;
        void disconnect() override;
        bool isConnected() const override;
        QVariant sendRequest(const QVariantMap& request) override;

    private slots:
        void onConnected();
        void onDisconnected();
        void onTextMessageReceived(const QString& message);
        void onError(QAbstractSocket::SocketError error);

    private:
        QWebSocket* m_webSocket;
        QUrl m_url;
        QMap<QString, QVariantMap> m_pendingRequests;
    };

    class DatabaseConnector : public ExternalSystemConnector {
        Q_OBJECT

    public:
        DatabaseConnector(const QString& connectionString, QObject* parent = nullptr);

        bool connect() override;
        void disconnect() override;
        bool isConnected() const override;
        QVariant sendRequest(const QVariantMap& request) override;

    private:
        QSqlDatabase m_database;
        QString m_connectionString;
        QString m_connectionName;
    };

    IntegrationPlugin(QObject* parent = nullptr);

    // Integration methods
    Q_INVOKABLE bool addConnector(const QString& name, const QString& type, const QVariantMap& config);
    Q_INVOKABLE bool removeConnector(const QString& name);
    Q_INVOKABLE QVariant sendToSystem(const QString& connectorName, const QVariantMap& request);
    Q_INVOKABLE QStringList getAvailableConnectors() const;
    Q_INVOKABLE bool isSystemConnected(const QString& connectorName) const;

signals:
    void systemConnected(const QString& connectorName);
    void systemDisconnected(const QString& connectorName);
    void dataFromSystem(const QString& connectorName, const QVariant& data);
    void systemError(const QString& connectorName, const QString& error);

private:
    QMap<QString, std::unique_ptr<ExternalSystemConnector>> m_connectors;
    QMap<QString, std::function<std::unique_ptr<ExternalSystemConnector>(const QVariantMap&)>> m_connectorFactories;

    void registerConnectorTypes();
    void setupConnectorSignals(const QString& name, ExternalSystemConnector* connector);
};
```

## Usage Examples

### Using the Background Processor

```cpp
// In your application
auto* processor = pluginManager->getPlugin<BackgroundProcessorPlugin>("com.example.BackgroundProcessor");

connect(processor, &BackgroundProcessorPlugin::dataProcessed,
        [](const QVariant& result) {
    qDebug() << "Processing completed:" << result;
});

// Process data in background
processor->processData(QVariantMap{{"input", "large_dataset.csv"}});
```

### Using the Stream Processor

```cpp
auto* streamProcessor = pluginManager->getPlugin<StreamProcessorPlugin>("com.example.StreamProcessor");

// Configure processing pipeline
streamProcessor->addProcessingStage("compression", QVariantMap{{"level", 9}});
streamProcessor->addProcessingStage("encryption", QVariantMap{{"key", "secret_key"}});

// Process stream
FileDataStream input("input.dat");
FileDataStream output("output.dat");
streamProcessor->processStream(&input, &output);
```

### Using the Integration Plugin

```cpp
auto* integration = pluginManager->getPlugin<IntegrationPlugin>("com.example.Integration");

// Add WebSocket connector
integration->addConnector("websocket", "websocket", QVariantMap{
    {"url", "wss://api.example.com/ws"},
    {"headers", QVariantMap{{"Authorization", "Bearer token"}}}
});

// Send request to external system
QVariant response = integration->sendToSystem("websocket", QVariantMap{
    {"action", "getData"},
    {"params", QVariantMap{{"id", 123}}}
});
```

## Best Practices

### Performance Optimization

1. **Use thread pools** for CPU-intensive operations
2. **Implement proper caching** for frequently accessed data
3. **Use asynchronous operations** for I/O operations
4. **Monitor memory usage** and implement cleanup strategies

### Error Handling

1. **Implement comprehensive error recovery**
2. **Use proper exception handling**
3. **Log errors with sufficient context**
4. **Provide meaningful error messages to users**

### Testing

1. **Write unit tests** for all plugin components
2. **Test error conditions** and edge cases
3. **Use mock objects** for external dependencies
4. **Implement integration tests** for complete workflows

## See Also

- [Basic Plugin Examples](basic-plugin-examples.md)
- [Composition Examples](composition-examples.md)
- [Plugin Development Guide](../user-guide/plugin-development.md)
- [API Reference](../api/index.md)
