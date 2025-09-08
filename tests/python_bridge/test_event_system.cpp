/**
 * @file test_event_system.cpp
 * @brief Comprehensive tests for the Python bridge event system
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QSignalSpy>
#include <QThread>
#include <QDateTime>
#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <atomic>

#include <qtplugin/bridges/python_plugin_bridge.hpp>

class TestEventSystem : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic event functionality
    void testEventSubscription();
    void testEventUnsubscription();
    void testEventEmission();
    void testEventCallbackExecution();

    // Advanced event scenarios
    void testMultipleEventTypes();
    void testMultipleSubscribers();
    void testEventDataTransmission();
    void testEventOrderPreservation();

    // Error handling
    void testInvalidEventSubscription();
    void testCallbackExceptions();
    void testEventEmissionErrors();

    // Performance and stress tests
    void testHighFrequencyEvents();
    void testLargeEventData();
    void testConcurrentEventHandling();

    // Integration tests
    void testPythonToCppEvents();
    void testCppToPythonEvents();
    void testBidirectionalEvents();

private:
    void createEventTestPlugin();
    void waitForEvents(int expectedCount, int timeoutMs = 5000);

    QTemporaryDir* m_tempDir;
    QString m_testPluginPath;
    std::unique_ptr<qtplugin::PythonPluginBridge> m_bridge;

    // Event tracking
    QMutex m_eventMutex;
    QWaitCondition m_eventCondition;
    QList<QPair<QString, QJsonObject>> m_receivedEvents;
    int m_expectedEventCount;
};

void TestEventSystem::initTestCase()
{
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    createEventTestPlugin();
}

void TestEventSystem::cleanupTestCase()
{
    delete m_tempDir;
}

void TestEventSystem::init()
{
    m_bridge = std::make_unique<qtplugin::PythonPluginBridge>(m_testPluginPath);
    m_receivedEvents.clear();
    m_expectedEventCount = 0;
}

void TestEventSystem::cleanup()
{
    if (m_bridge) {
        m_bridge->shutdown();
        m_bridge.reset();
    }
}

void TestEventSystem::createEventTestPlugin()
{
    QString pluginContent = R"(
import time
import threading

class EventTestPlugin:
    def __init__(self):
        self.name = "Event Test Plugin"
        self.version = "1.0.0"
        self.description = "Plugin for testing event system"
        self.event_history = []
        self.event_handlers = {}
        self.event_lock = threading.Lock()

    def initialize(self):
        return {"success": True}

    def shutdown(self):
        return {"success": True}

    def subscribe_events(self, event_names):
        with self.event_lock:
            for event_name in event_names:
                if event_name not in self.event_handlers:
                    self.event_handlers[event_name] = []
        return {"success": True, "subscribed": event_names}

    def unsubscribe_events(self, event_names):
        with self.event_lock:
            for event_name in event_names:
                if event_name in self.event_handlers:
                    del self.event_handlers[event_name]
        return {"success": True, "unsubscribed": event_names}

    def emit_event(self, event_name, event_data):
        with self.event_lock:
            self.event_history.append({
                "event_name": event_name,
                "event_data": event_data,
                "timestamp": time.time()
            })
        return {"success": True, "event_name": event_name}

    def handle_event(self, event_name, event_data):
        with self.event_lock:
            self.event_history.append({
                "event_name": event_name,
                "event_data": event_data,
                "timestamp": time.time(),
                "source": "external"
            })
        return {"handled": True}

    def get_event_history(self):
        with self.event_lock:
            return list(self.event_history)

    def clear_event_history(self):
        with self.event_lock:
            self.event_history.clear()
        return {"cleared": True}

    def trigger_test_event(self, event_name="test_event", data=None):
        if data is None:
            data = {"message": "test", "timestamp": time.time()}

        # Simulate event triggering
        self.emit_event(event_name, data)
        return {"triggered": True, "event_name": event_name}

    def trigger_multiple_events(self, count=5):
        events = []
        for i in range(count):
            event_name = f"multi_event_{i}"
            event_data = {"index": i, "timestamp": time.time()}
            self.emit_event(event_name, event_data)
            events.append(event_name)
        return {"triggered": count, "events": events}

    def trigger_high_frequency_events(self, count=100, delay=0.001):
        events = []
        for i in range(count):
            event_name = "high_freq_event"
            event_data = {"index": i, "timestamp": time.time()}
            self.emit_event(event_name, event_data)
            events.append(event_name)
            if delay > 0:
                time.sleep(delay)
        return {"triggered": count, "events": len(events)}

    def trigger_large_data_event(self, data_size=10000):
        large_data = {
            "large_string": "x" * data_size,
            "timestamp": time.time(),
            "size": data_size
        }
        self.emit_event("large_data_event", large_data)
        return {"triggered": True, "data_size": data_size}

def create_plugin():
    return EventTestPlugin()
)";

    m_testPluginPath = m_tempDir->path() + "/event_test_plugin.py";
    QFile file(m_testPluginPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << pluginContent;
    file.close();
}

void TestEventSystem::testEventSubscription()
{
    QVERIFY(m_bridge->initialize().has_value());

    bool subscriptionSuccessful = false;
    auto callback = [&](const QString& eventName, const QJsonObject& eventData) {
        Q_UNUSED(eventName)
        Q_UNUSED(eventData)
        subscriptionSuccessful = true;
    };

    std::vector<QString> eventTypes = {"test_subscription_event"};
    auto result = m_bridge->subscribe_to_events("", eventTypes, callback);

    QVERIFY(result.has_value());
}

void TestEventSystem::testEventUnsubscription()
{
    QVERIFY(m_bridge->initialize().has_value());

    auto callback = [](const QString&, const QJsonObject&) {};
    std::vector<QString> eventTypes = {"test_unsubscription_event"};

    // Subscribe first
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    // Then unsubscribe
    auto unsub_result = m_bridge->unsubscribe_from_events("", eventTypes);
    QVERIFY(unsub_result.has_value());
}

void TestEventSystem::testEventEmission()
{
    QVERIFY(m_bridge->initialize().has_value());

    QJsonObject eventData;
    eventData["message"] = "test emission";
    eventData["timestamp"] = QDateTime::currentDateTime().toString();

    auto result = m_bridge->emit_event("test_emission_event", eventData);
    QVERIFY(result.has_value());
}

void TestEventSystem::testEventCallbackExecution()
{
    QVERIFY(m_bridge->initialize().has_value());

    bool callbackExecuted = false;
    QString receivedEventName;
    QJsonObject receivedEventData;

    auto callback = [&](const QString& eventName, const QJsonObject& eventData) {
        callbackExecuted = true;
        receivedEventName = eventName;
        receivedEventData = eventData;

        QMutexLocker locker(&m_eventMutex);
        m_receivedEvents.append(qMakePair(eventName, eventData));
        m_eventCondition.wakeAll();
    };

    std::vector<QString> eventTypes = {"callback_test_event"};
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    QJsonObject testData;
    testData["test_key"] = "test_value";
    testData["callback_test"] = true;

    auto emit_result = m_bridge->emit_event("callback_test_event", testData);
    QVERIFY(emit_result.has_value());

    // Wait for callback execution
    waitForEvents(1);

    QVERIFY(callbackExecuted);
    QCOMPARE(receivedEventName, QString("callback_test_event"));
    QCOMPARE(receivedEventData["test_key"].toString(), QString("test_value"));
}

void TestEventSystem::waitForEvents(int expectedCount, int timeoutMs)
{
    QMutexLocker locker(&m_eventMutex);
    m_expectedEventCount = expectedCount;

    QElapsedTimer timer;
    timer.start();

    while (m_receivedEvents.size() < expectedCount && timer.elapsed() < timeoutMs) {
        m_eventCondition.wait(&m_eventMutex, timeoutMs - timer.elapsed());
    }

    QVERIFY2(m_receivedEvents.size() >= expectedCount,
             QString("Expected %1 events, got %2").arg(expectedCount).arg(m_receivedEvents.size()).toUtf8());
}

// === Missing Method Implementations ===

void TestEventSystem::testMultipleEventTypes()
{
    QVERIFY(m_bridge->initialize().has_value());

    QStringList eventTypes = {"event_type_1", "event_type_2", "event_type_3"};
    int eventsReceived = 0;

    auto callback = [&](const QString& eventName, const QJsonObject& eventData) {
        Q_UNUSED(eventData)
        eventsReceived++;

        QMutexLocker locker(&m_eventMutex);
        m_receivedEvents.append(qMakePair(eventName, eventData));
        m_eventCondition.wakeAll();
    };

    // Subscribe to multiple event types
    std::vector<QString> eventTypesVec;
    for (const QString& type : eventTypes) {
        eventTypesVec.push_back(type);
    }

    auto sub_result = m_bridge->subscribe_to_events("", eventTypesVec, callback);
    QVERIFY(sub_result.has_value());

    // Emit events of different types
    for (const QString& eventType : eventTypes) {
        QJsonObject eventData;
        eventData["type"] = eventType;
        eventData["timestamp"] = QDateTime::currentMSecsSinceEpoch();

        auto emit_result = m_bridge->emit_event(eventType, eventData);
        QVERIFY(emit_result.has_value());
    }

    waitForEvents(eventTypes.size());
    QCOMPARE(eventsReceived, eventTypes.size());
}

void TestEventSystem::testMultipleSubscribers()
{
    QVERIFY(m_bridge->initialize().has_value());

    int subscriber1Events = 0;
    int subscriber2Events = 0;

    auto callback1 = [&](const QString&, const QJsonObject&) {
        subscriber1Events++;
    };

    auto callback2 = [&](const QString&, const QJsonObject&) {
        subscriber2Events++;
    };

    std::vector<QString> eventTypes = {"multi_subscriber_event"};

    // Subscribe with both callbacks
    auto sub1_result = m_bridge->subscribe_to_events("subscriber1", eventTypes, callback1);
    QVERIFY(sub1_result.has_value());

    auto sub2_result = m_bridge->subscribe_to_events("subscriber2", eventTypes, callback2);
    QVERIFY(sub2_result.has_value());

    // Emit an event
    QJsonObject eventData;
    eventData["message"] = "multi subscriber test";

    auto emit_result = m_bridge->emit_event("multi_subscriber_event", eventData);
    QVERIFY(emit_result.has_value());

    // Both subscribers should receive the event
    QThread::msleep(100); // Give time for callbacks
    QVERIFY(subscriber1Events > 0);
    QVERIFY(subscriber2Events > 0);
}

void TestEventSystem::testEventDataTransmission()
{
    QVERIFY(m_bridge->initialize().has_value());

    QJsonObject receivedData;
    bool dataReceived = false;

    auto callback = [&](const QString&, const QJsonObject& eventData) {
        receivedData = eventData;
        dataReceived = true;

        QMutexLocker locker(&m_eventMutex);
        m_receivedEvents.append(qMakePair("data_test", eventData));
        m_eventCondition.wakeAll();
    };

    std::vector<QString> eventTypes = {"data_transmission_event"};
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    // Create complex event data
    QJsonObject originalData;
    originalData["string_value"] = "test string";
    originalData["number_value"] = 42;
    originalData["boolean_value"] = true;
    originalData["array_value"] = QJsonArray{1, 2, 3};

    QJsonObject nestedObject;
    nestedObject["nested_key"] = "nested_value";
    originalData["object_value"] = nestedObject;

    auto emit_result = m_bridge->emit_event("data_transmission_event", originalData);
    QVERIFY(emit_result.has_value());

    waitForEvents(1);

    QVERIFY(dataReceived);
    QCOMPARE(receivedData["string_value"].toString(), QString("test string"));
    QCOMPARE(receivedData["number_value"].toInt(), 42);
    QCOMPARE(receivedData["boolean_value"].toBool(), true);
}

void TestEventSystem::testEventOrderPreservation()
{
    QVERIFY(m_bridge->initialize().has_value());

    QStringList receivedOrder;

    auto callback = [&](const QString& eventName, const QJsonObject& eventData) {
        QString eventId = eventData["id"].toString();
        receivedOrder.append(eventId);

        QMutexLocker locker(&m_eventMutex);
        m_receivedEvents.append(qMakePair(eventName, eventData));
        m_eventCondition.wakeAll();
    };

    std::vector<QString> eventTypes = {"order_test_event"};
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    // Emit events in sequence
    QStringList expectedOrder;
    for (int i = 0; i < 5; ++i) {
        QString eventId = QString("event_%1").arg(i);
        expectedOrder.append(eventId);

        QJsonObject eventData;
        eventData["id"] = eventId;
        eventData["sequence"] = i;

        auto emit_result = m_bridge->emit_event("order_test_event", eventData);
        QVERIFY(emit_result.has_value());
    }

    waitForEvents(5);

    QCOMPARE(receivedOrder.size(), expectedOrder.size());
    for (int i = 0; i < expectedOrder.size(); ++i) {
        QCOMPARE(receivedOrder[i], expectedOrder[i]);
    }
}

void TestEventSystem::testInvalidEventSubscription()
{
    QVERIFY(m_bridge->initialize().has_value());

    auto callback = [](const QString&, const QJsonObject&) {};

    // Test with empty event types
    std::vector<QString> emptyEventTypes;
    auto result = m_bridge->subscribe_to_events("", emptyEventTypes, callback);
    // This might succeed or fail depending on implementation
    // Just verify it doesn't crash
}

void TestEventSystem::testCallbackExceptions()
{
    QVERIFY(m_bridge->initialize().has_value());

    auto throwingCallback = [](const QString&, const QJsonObject&) {
        throw std::runtime_error("Test exception in callback");
    };

    std::vector<QString> eventTypes = {"exception_test_event"};
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, throwingCallback);
    QVERIFY(sub_result.has_value());

    QJsonObject eventData;
    eventData["test"] = "exception handling";

    // This should not crash the system
    auto emit_result = m_bridge->emit_event("exception_test_event", eventData);
    QVERIFY(emit_result.has_value());

    // Give time for callback to execute and potentially throw
    QThread::msleep(100);
}

void TestEventSystem::testEventEmissionErrors()
{
    QVERIFY(m_bridge->initialize().has_value());

    // Test emitting to non-existent event type (should still succeed)
    QJsonObject eventData;
    eventData["test"] = "error handling";

    auto result = m_bridge->emit_event("non_existent_event", eventData);
    QVERIFY(result.has_value()); // Should succeed even if no subscribers
}

void TestEventSystem::testHighFrequencyEvents()
{
    QVERIFY(m_bridge->initialize().has_value());

    int eventsReceived = 0;
    const int totalEvents = 100;

    auto callback = [&](const QString&, const QJsonObject&) {
        eventsReceived++;
    };

    std::vector<QString> eventTypes = {"high_freq_event"};
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < totalEvents; ++i) {
        QJsonObject eventData;
        eventData["index"] = i;

        auto emit_result = m_bridge->emit_event("high_freq_event", eventData);
        QVERIFY(emit_result.has_value());
    }

    qint64 elapsed = timer.elapsed();

    // Wait for all events to be processed
    QElapsedTimer waitTimer;
    waitTimer.start();
    while (eventsReceived < totalEvents && waitTimer.elapsed() < 5000) {
        QThread::msleep(10);
    }

    qDebug() << "High frequency test:" << totalEvents << "events in" << elapsed << "ms";
    qDebug() << "Events received:" << eventsReceived;

    QVERIFY(eventsReceived >= totalEvents * 0.9); // Allow some loss under stress
}

void TestEventSystem::testLargeEventData()
{
    QVERIFY(m_bridge->initialize().has_value());

    bool largeDataReceived = false;
    int receivedDataSize = 0;

    auto callback = [&](const QString&, const QJsonObject& eventData) {
        largeDataReceived = true;
        receivedDataSize = eventData["large_data"].toString().size();

        QMutexLocker locker(&m_eventMutex);
        m_receivedEvents.append(qMakePair("large_data", eventData));
        m_eventCondition.wakeAll();
    };

    std::vector<QString> eventTypes = {"large_data_event"};
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    // Create large data
    const int dataSize = 10000;
    QString largeString = QString("x").repeated(dataSize);

    QJsonObject eventData;
    eventData["large_data"] = largeString;
    eventData["size"] = dataSize;

    auto emit_result = m_bridge->emit_event("large_data_event", eventData);
    QVERIFY(emit_result.has_value());

    waitForEvents(1);

    QVERIFY(largeDataReceived);
    QCOMPARE(receivedDataSize, dataSize);
}

void TestEventSystem::testConcurrentEventHandling()
{
    QVERIFY(m_bridge->initialize().has_value());

    std::atomic<int> eventsReceived{0};

    auto callback = [&](const QString&, const QJsonObject&) {
        eventsReceived++;
    };

    std::vector<QString> eventTypes = {"concurrent_event"};
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    const int numThreads = 4;
    const int eventsPerThread = 25;

    QList<QFuture<void>> futures;

    for (int t = 0; t < numThreads; ++t) {
        auto future = QtConcurrent::run([this, t, eventsPerThread]() {
            for (int i = 0; i < eventsPerThread; ++i) {
                QJsonObject eventData;
                eventData["thread"] = t;
                eventData["index"] = i;

                auto result = m_bridge->emit_event("concurrent_event", eventData);
                Q_UNUSED(result) // Don't assert in thread
            }
        });
        futures.append(future);
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.waitForFinished();
    }

    // Wait for all events to be processed
    QElapsedTimer timer;
    timer.start();
    const int expectedEvents = numThreads * eventsPerThread;

    while (eventsReceived < expectedEvents && timer.elapsed() < 5000) {
        QThread::msleep(10);
    }

    qDebug() << "Concurrent test: expected" << expectedEvents << "received" << eventsReceived.load();

    QVERIFY(eventsReceived >= expectedEvents * 0.9); // Allow some loss under concurrent stress
}

void TestEventSystem::testPythonToCppEvents()
{
    QVERIFY(m_bridge->initialize().has_value());

    // This would test events originating from Python plugin
    // For now, just test that the mechanism works

    bool eventReceived = false;
    auto callback = [&](const QString&, const QJsonObject&) {
        eventReceived = true;
    };

    std::vector<QString> eventTypes = {"python_to_cpp_event"};
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    // Simulate Python-originated event
    QJsonObject eventData;
    eventData["source"] = "python";
    eventData["message"] = "Hello from Python";

    auto emit_result = m_bridge->emit_event("python_to_cpp_event", eventData);
    QVERIFY(emit_result.has_value());

    QThread::msleep(100);
    QVERIFY(eventReceived);
}

void TestEventSystem::testCppToPythonEvents()
{
    QVERIFY(m_bridge->initialize().has_value());

    // This would test events sent from C++ to Python
    // For now, just test that the mechanism works

    QJsonObject eventData;
    eventData["source"] = "cpp";
    eventData["message"] = "Hello from C++";

    auto emit_result = m_bridge->emit_event("cpp_to_python_event", eventData);
    QVERIFY(emit_result.has_value());
}

void TestEventSystem::testBidirectionalEvents()
{
    QVERIFY(m_bridge->initialize().has_value());

    // Test bidirectional event communication
    bool responseReceived = false;

    auto callback = [&](const QString& eventName, const QJsonObject& eventData) {
        Q_UNUSED(eventData)
        if (eventName == "response_event") {
            responseReceived = true;
        }
    };

    std::vector<QString> eventTypes = {"response_event"};
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    // Send request event
    QJsonObject requestData;
    requestData["type"] = "request";
    requestData["message"] = "ping";

    auto request_result = m_bridge->emit_event("request_event", requestData);
    QVERIFY(request_result.has_value());

    // Simulate response (in real scenario, Python plugin would respond)
    QJsonObject responseData;
    responseData["type"] = "response";
    responseData["message"] = "pong";

    auto response_result = m_bridge->emit_event("response_event", responseData);
    QVERIFY(response_result.has_value());

    QThread::msleep(100);
    QVERIFY(responseReceived);
}

QTEST_MAIN(TestEventSystem)
#include "test_event_system.moc"
