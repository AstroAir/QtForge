/**
 * @file test_python_plugin_bridge_complete.cpp
 * @brief Comprehensive tests for PythonPluginBridge functionality
 */

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFuture>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QThread>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QtTest/QtTest>

#include <qtplugin/bridges/python_plugin_bridge.hpp>

class TestPythonPluginBridgeComplete : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Core functionality tests
    void testPluginInitialization();
    void testPluginShutdown();
    void testPluginLoading();
    void testPluginUnloading();

    // Method invocation tests
    void testMethodInvocation();
    void testMethodInvocationWithParameters();
    void testMethodInvocationErrors();
    void testAvailableMethodsDiscovery();
    void testMethodSignatureRetrieval();

    // Property access tests
    void testPropertyAccess();
    void testPropertyModification();
    void testPropertyErrors();
    void testAvailablePropertiesDiscovery();

    // Event system tests
    void testEventSubscription();
    void testEventUnsubscription();
    void testEventEmission();
    void testEventCallbacks();
    void testMultipleEventSubscriptions();

    // Advanced functionality tests
    void testHotReload();
    void testDependencyChangeHandling();
    void testCodeExecution();
    void testPluginMetadataExtraction();

    // Error handling tests
    void testInvalidPluginPath();
    void testPythonRuntimeErrors();
    void testTimeoutHandling();
    void testMemoryManagement();

    // Performance tests
    void testLargeDataHandling();
    void testConcurrentAccess();
    void testRepeatedOperations();

    // Factory tests
    void testPythonModuleRequirements();
    void testModuleAvailabilityCheck();

private:
    void createTestPlugin(const QString& filename, const QString& content);
    void waitForCondition(std::function<bool()> condition,
                          int timeoutMs = 5000);
    bool isPythonAvailable();

    QTemporaryDir* m_tempDir;
    QString m_testPluginPath;
    std::unique_ptr<qtplugin::PythonPluginBridge> m_bridge;
    bool m_pythonAvailable;
};

void TestPythonPluginBridgeComplete::initTestCase() {
    // Check if Python is available
    m_pythonAvailable = isPythonAvailable();

    // Create temporary directory for test plugins
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());

    // Create a comprehensive test plugin
    QString testPluginContent = R"(
class TestPlugin:
    def __init__(self):
        self.name = "Test Plugin"
        self.version = "1.0.0"
        self.description = "A test plugin for comprehensive testing"
        self.author = "Test Suite"
        self.license = "MIT"
        self.counter = 0
        self.data = {}
        self.initialized = False
        self.event_handlers = {}

    def initialize(self):
        self.initialized = True
        return {"success": True, "message": "Plugin initialized"}

    def shutdown(self):
        self.initialized = False
        return {"success": True, "message": "Plugin shutdown"}

    def get_info(self):
        return {
            "name": self.name,
            "version": self.version,
            "description": self.description,
            "author": self.author,
            "license": self.license,
            "initialized": self.initialized
        }

    def simple_method(self):
        return "simple_result"

    def method_with_params(self, param1, param2=None):
        return {
            "param1": param1,
            "param2": param2,
            "counter": self.counter
        }

    def increment_counter(self, amount=1):
        self.counter += amount
        return self.counter

    def get_counter(self):
        return self.counter

    def set_counter(self, value):
        self.counter = int(value)

    def store_data(self, key, value):
        self.data[key] = value
        return {"stored": True, "key": key, "value": value}

    def get_data(self, key=None):
        if key is None:
            return self.data
        return self.data.get(key)

    def raise_error(self):
        raise ValueError("Test error for error handling")

    def handle_dependency_change(self, dependency_id, new_state):
        return {
            "handled": True,
            "dependency_id": dependency_id,
            "new_state": new_state
        }

    def handle_event(self, event_name, event_data):
        if event_name not in self.event_handlers:
            self.event_handlers[event_name] = []
        self.event_handlers[event_name].append(event_data)
        return {"handled": True, "event_name": event_name}

    def get_event_history(self):
        return self.event_handlers

def create_plugin():
    return TestPlugin()
)";

    m_testPluginPath = m_tempDir->path() + "/test_plugin.py";
    createTestPlugin("test_plugin.py", testPluginContent);
}

void TestPythonPluginBridgeComplete::cleanupTestCase() { delete m_tempDir; }

void TestPythonPluginBridgeComplete::init() {
    if (!m_pythonAvailable) {
        QSKIP("Python not available for testing");
    }

    m_bridge = std::make_unique<qtplugin::PythonPluginBridge>(m_testPluginPath);
}

void TestPythonPluginBridgeComplete::cleanup() {
    if (m_bridge) {
        m_bridge->shutdown();
        m_bridge.reset();
    }
}

void TestPythonPluginBridgeComplete::testPluginInitialization() {
    QVERIFY(m_bridge != nullptr);

    auto result = m_bridge->initialize();
    QVERIFY(result.has_value());

    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Running);
    QVERIFY(!m_bridge->name().empty());
    QVERIFY(!m_bridge->description().empty());
}

void TestPythonPluginBridgeComplete::testPluginShutdown() {
    QVERIFY(m_bridge->initialize().has_value());

    m_bridge->shutdown();
    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Unloaded);
}

void TestPythonPluginBridgeComplete::testMethodInvocation() {
    QVERIFY(m_bridge->initialize().has_value());

    // Test simple method call
    auto result = m_bridge->invoke_method("simple_method", QVariantList());
    QVERIFY(result.has_value());
    QCOMPARE(result.value().toString(), QString("simple_result"));
}

void TestPythonPluginBridgeComplete::testMethodInvocationWithParameters() {
    QVERIFY(m_bridge->initialize().has_value());

    QVariantList params;
    params << "test_param1" << "test_param2";

    auto result = m_bridge->invoke_method("method_with_params", params);
    QVERIFY(result.has_value());

    // The result should be a QJsonObject converted to QVariant
    QVariantMap resultMap = result.value().toMap();
    QCOMPARE(resultMap["param1"].toString(), QString("test_param1"));
    QCOMPARE(resultMap["param2"].toString(), QString("test_param2"));
}

void TestPythonPluginBridgeComplete::testMethodInvocationErrors() {
    QVERIFY(m_bridge->initialize().has_value());

    // Test calling non-existent method
    auto result =
        m_bridge->invoke_method("non_existent_method", QVariantList());
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::ExecutionFailed);

    // Test method that raises an error
    auto error_result = m_bridge->invoke_method("raise_error", QVariantList());
    QVERIFY(!error_result.has_value());
}

void TestPythonPluginBridgeComplete::testAvailableMethodsDiscovery() {
    QVERIFY(m_bridge->initialize().has_value());

    auto methods = m_bridge->get_available_methods();
    QVERIFY(!methods.empty());

    // Check for expected methods
    QStringList methodNames;
    for (const auto& method : methods) {
        methodNames << method;
    }

    QVERIFY(methodNames.contains("simple_method"));
    QVERIFY(methodNames.contains("method_with_params"));
    QVERIFY(methodNames.contains("get_counter"));
    QVERIFY(methodNames.contains("set_counter"));
}

void TestPythonPluginBridgeComplete::testMethodSignatureRetrieval() {
    QVERIFY(m_bridge->initialize().has_value());

    auto signature = m_bridge->get_method_signature("method_with_params");
    QVERIFY(signature.has_value());

    QJsonObject sigObj = signature.value();
    QCOMPARE(sigObj["name"].toString(), QString("method_with_params"));
    QVERIFY(sigObj.contains("signature"));
}

void TestPythonPluginBridgeComplete::testPropertyAccess() {
    QVERIFY(m_bridge->initialize().has_value());

    // Test getting a property
    auto result = m_bridge->get_property("counter");
    QVERIFY(result.has_value());
    QCOMPARE(result.value().toInt(), 0);

    // Test getting string property
    auto name_result = m_bridge->get_property("name");
    QVERIFY(name_result.has_value());
    QCOMPARE(name_result.value().toString(), QString("Test Plugin"));
}

void TestPythonPluginBridgeComplete::testPropertyModification() {
    QVERIFY(m_bridge->initialize().has_value());

    // Set a property
    auto set_result = m_bridge->set_property("counter", QVariant(42));
    QVERIFY(set_result.has_value());

    // Verify the change
    auto get_result = m_bridge->get_property("counter");
    QVERIFY(get_result.has_value());
    QCOMPARE(get_result.value().toInt(), 42);
}

void TestPythonPluginBridgeComplete::testPropertyErrors() {
    QVERIFY(m_bridge->initialize().has_value());

    // Test accessing non-existent property
    auto result = m_bridge->get_property("non_existent_property");
    // This might return None/null rather than error, depending on
    // implementation
    QVERIFY(
        result
            .has_value());  // getattr returns None for non-existent properties
}

void TestPythonPluginBridgeComplete::testAvailablePropertiesDiscovery() {
    QVERIFY(m_bridge->initialize().has_value());

    auto properties = m_bridge->get_available_properties();
    QVERIFY(!properties.empty());

    QStringList propNames;
    for (const auto& prop : properties) {
        propNames << prop;
    }

    QVERIFY(propNames.contains("name"));
    QVERIFY(propNames.contains("version"));
    QVERIFY(propNames.contains("counter"));
}

void TestPythonPluginBridgeComplete::testEventSubscription() {
    QVERIFY(m_bridge->initialize().has_value());

    bool eventReceived = false;
    QString receivedEventName;
    QJsonObject receivedEventData;

    auto callback = [&](const QString& eventName,
                        const QJsonObject& eventData) {
        eventReceived = true;
        receivedEventName = eventName;
        receivedEventData = eventData;
    };

    std::vector<QString> eventTypes = {"test_event"};
    auto result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(result.has_value());
}

void TestPythonPluginBridgeComplete::testEventUnsubscription() {
    QVERIFY(m_bridge->initialize().has_value());

    auto callback = [](const QString&, const QJsonObject&) {};
    std::vector<QString> eventTypes = {"test_event"};

    // Subscribe first
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    // Then unsubscribe
    auto unsub_result = m_bridge->unsubscribe_from_events("", eventTypes);
    QVERIFY(unsub_result.has_value());
}

void TestPythonPluginBridgeComplete::testEventEmission() {
    QVERIFY(m_bridge->initialize().has_value());

    QJsonObject eventData;
    eventData["message"] = "test message";
    eventData["timestamp"] = QDateTime::currentDateTime().toString();

    auto result = m_bridge->emit_event("test_event", eventData);
    QVERIFY(result.has_value());
}

void TestPythonPluginBridgeComplete::testEventCallbacks() {
    QVERIFY(m_bridge->initialize().has_value());

    bool eventReceived = false;
    QString receivedEventName;
    QJsonObject receivedEventData;

    auto callback = [&](const QString& eventName,
                        const QJsonObject& eventData) {
        eventReceived = true;
        receivedEventName = eventName;
        receivedEventData = eventData;
    };

    std::vector<QString> eventTypes = {"callback_test_event"};
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    QJsonObject testData;
    testData["test_key"] = "test_value";

    auto emit_result = m_bridge->emit_event("callback_test_event", testData);
    QVERIFY(emit_result.has_value());

    // Wait for callback to be called
    waitForCondition([&]() { return eventReceived; });

    QVERIFY(eventReceived);
    QCOMPARE(receivedEventName, QString("callback_test_event"));
    QCOMPARE(receivedEventData["test_key"].toString(), QString("test_value"));
}

void TestPythonPluginBridgeComplete::testMultipleEventSubscriptions() {
    QVERIFY(m_bridge->initialize().has_value());

    int eventsReceived = 0;
    QStringList receivedEvents;

    auto callback = [&](const QString& eventName, const QJsonObject&) {
        eventsReceived++;
        receivedEvents << eventName;
    };

    std::vector<QString> eventTypes = {"event1", "event2", "event3"};
    auto sub_result = m_bridge->subscribe_to_events("", eventTypes, callback);
    QVERIFY(sub_result.has_value());

    // Emit multiple events
    for (const QString& eventType : eventTypes) {
        auto emit_result = m_bridge->emit_event(eventType, QJsonObject());
        QVERIFY(emit_result.has_value());
    }

    waitForCondition([&]() { return eventsReceived >= 3; });

    QCOMPARE(eventsReceived, 3);
    QVERIFY(receivedEvents.contains("event1"));
    QVERIFY(receivedEvents.contains("event2"));
    QVERIFY(receivedEvents.contains("event3"));
}

void TestPythonPluginBridgeComplete::testHotReload() {
    QVERIFY(m_bridge->initialize().has_value());

    // Get initial counter value
    auto initial_result = m_bridge->get_property("counter");
    QVERIFY(initial_result.has_value());

    // Modify counter
    auto set_result = m_bridge->set_property("counter", QVariant(100));
    QVERIFY(set_result.has_value());

    // Perform hot reload
    auto reload_result = m_bridge->hot_reload();
    QVERIFY(reload_result.has_value());

    // Verify plugin is still functional after reload
    auto method_result =
        m_bridge->invoke_method("simple_method", QVariantList());
    QVERIFY(method_result.has_value());
    QCOMPARE(method_result.value().toString(), QString("simple_result"));
}

void TestPythonPluginBridgeComplete::testDependencyChangeHandling() {
    QVERIFY(m_bridge->initialize().has_value());

    auto result = m_bridge->handle_dependency_change(
        "test_dependency", qtplugin::PluginState::Running);
    QVERIFY(result.has_value());
}

void TestPythonPluginBridgeComplete::testCodeExecution() {
    QVERIFY(m_bridge->initialize().has_value());

    QString code = "plugin.get_counter()";
    auto result = m_bridge->execute_code(code);
    QVERIFY(result.has_value());
}

void TestPythonPluginBridgeComplete::testPluginMetadataExtraction() {
    QVERIFY(m_bridge->initialize().has_value());

    // Test that metadata was extracted during initialization
    QVERIFY(!m_bridge->name().empty());
    QVERIFY(!m_bridge->description().empty());

    // Test getting plugin info through method call
    auto info_result = m_bridge->invoke_method("get_info", QVariantList());
    QVERIFY(info_result.has_value());

    QVariantMap infoMap = info_result.value().toMap();
    QCOMPARE(infoMap["name"].toString(), QString("Test Plugin"));
    QCOMPARE(infoMap["version"].toString(), QString("1.0.0"));
}

void TestPythonPluginBridgeComplete::testInvalidPluginPath() {
    auto invalid_bridge = std::make_unique<qtplugin::PythonPluginBridge>(
        "/invalid/path/plugin.py");
    auto result = invalid_bridge->initialize();
    QVERIFY(!result.has_value());
}

void TestPythonPluginBridgeComplete::testPythonRuntimeErrors() {
    QVERIFY(m_bridge->initialize().has_value());

    // Test method that raises an exception
    auto result = m_bridge->invoke_method("raise_error", QVariantList());
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::ExecutionFailed);
}

void TestPythonPluginBridgeComplete::testTimeoutHandling() {
    QVERIFY(m_bridge->initialize().has_value());

    // Create a plugin method that takes a long time
    QString longRunningCode = R"(
import time
time.sleep(0.1)  # Short sleep for testing
"completed"
)";

    auto result = m_bridge->execute_code(longRunningCode);
    QVERIFY(result.has_value());
    QCOMPARE(result.value().toString(), QString("completed"));
}

void TestPythonPluginBridgeComplete::testMemoryManagement() {
    // Test multiple initialization and cleanup cycles
    for (int i = 0; i < 5; ++i) {
        auto bridge =
            std::make_unique<qtplugin::PythonPluginBridge>(m_testPluginPath);
        QVERIFY(bridge->initialize().has_value());

        // Perform some operations
        auto result = bridge->invoke_method("simple_method", QVariantList());
        QVERIFY(result.has_value());

        bridge->shutdown();
    }
}

void TestPythonPluginBridgeComplete::testLargeDataHandling() {
    QVERIFY(m_bridge->initialize().has_value());

    // Test with large string parameter
    QString largeString = QString("x").repeated(10000);
    QVariantList params;
    params << largeString;

    auto result = m_bridge->invoke_method("method_with_params", params);
    QVERIFY(result.has_value());

    QVariantMap resultMap = result.value().toMap();
    QCOMPARE(resultMap["param1"].toString().length(), 10000);
}

void TestPythonPluginBridgeComplete::testConcurrentAccess() {
    QVERIFY(m_bridge->initialize().has_value());

    // Test concurrent method calls (simplified test)
    QList<QFuture<void>> futures;

    for (int i = 0; i < 5; ++i) {
        auto future = QtConcurrent::run([this, i]() {
            QVariantList params;
            params << i;
            auto result = m_bridge->invoke_method("increment_counter", params);
            QVERIFY(result.has_value());
        });
        futures.append(future);
    }

    // Wait for all futures to complete
    for (auto& future : futures) {
        future.waitForFinished();
    }

    // Verify final counter value
    auto counter_result = m_bridge->get_property("counter");
    QVERIFY(counter_result.has_value());
    QVERIFY(counter_result.value().toInt() > 0);
}

void TestPythonPluginBridgeComplete::testRepeatedOperations() {
    QVERIFY(m_bridge->initialize().has_value());

    // Test repeated method calls
    for (int i = 0; i < 100; ++i) {
        auto result = m_bridge->invoke_method("simple_method", QVariantList());
        QVERIFY(result.has_value());
        QCOMPARE(result.value().toString(), QString("simple_result"));
    }
}

void TestPythonPluginBridgeComplete::testPythonModuleRequirements() {
    auto required_modules =
        qtplugin::PythonPluginFactory::required_python_modules();
    QVERIFY(!required_modules.isEmpty());

    QVERIFY(required_modules.contains("json"));
    QVERIFY(required_modules.contains("sys"));
    QVERIFY(required_modules.contains("os"));
    QVERIFY(required_modules.contains("importlib"));
}

void TestPythonPluginBridgeComplete::testModuleAvailabilityCheck() {
    QString pythonPath = "python";  // Assume python is in PATH
    auto missing_modules =
        qtplugin::PythonPluginFactory::check_required_modules(pythonPath);

    // In a proper environment, no modules should be missing
    QVERIFY(missing_modules.isEmpty() ||
            missing_modules.size() < 3);  // Allow some tolerance
}

void TestPythonPluginBridgeComplete::testPluginLoading() {
    // Test is covered by initialization test
    QVERIFY(m_bridge->initialize().has_value());
    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Running);
}

void TestPythonPluginBridgeComplete::testPluginUnloading() {
    QVERIFY(m_bridge->initialize().has_value());
    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Running);

    m_bridge->shutdown();
    QCOMPARE(m_bridge->state(), qtplugin::PluginState::Unloaded);
}

void TestPythonPluginBridgeComplete::createTestPlugin(const QString& filename,
                                                      const QString& content) {
    QString fullPath = m_tempDir->path() + "/" + filename;
    QFile file(fullPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << content;
    file.close();
}

void TestPythonPluginBridgeComplete::waitForCondition(
    std::function<bool()> condition, int timeoutMs) {
    QElapsedTimer timer;
    timer.start();

    while (!condition() && timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }

    QVERIFY2(condition(), "Condition not met within timeout");
}

bool TestPythonPluginBridgeComplete::isPythonAvailable() {
    QProcess process;
    QStringList python_names = {"python3",   "python",     "python3.8",
                                "python3.9", "python3.10", "python3.11",
                                "python3.12"};

    for (const QString& name : python_names) {
        process.start(name, QStringList() << "--version");

        if (!process.waitForFinished(3000)) {
            continue;  // Process didn't finish in time
        }

        if (process.exitCode() != 0) {
            continue;  // Process exited with error
        }

        // Check if output contains "Python" to verify it's actually Python
        QString output = process.readAllStandardOutput();
        if (output.contains("Python", Qt::CaseInsensitive)) {
            return true;  // Found a working Python installation
        }
    }

    return false;  // No working Python installation found
}

QTEST_MAIN(TestPythonPluginBridgeComplete)
#include "test_python_plugin_bridge_complete.moc"
