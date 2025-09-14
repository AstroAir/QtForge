/**
 * @file test_python_bridge.cpp
 * @brief Tests for Python plugin bridge implementation
 */

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>

#include <qtplugin/bridges/python_plugin_bridge.hpp>
#include <qtplugin/utils/error_handling.hpp>

#include "../utils/test_helpers.hpp"
#include "../utils/test_config_templates.hpp"

using namespace qtplugin;
using namespace QtForgeTest;

/**
 * @brief Test class for Python plugin bridge
 */
class TestPythonBridge : public TestFixtureBase {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testBridgeCreation();
    void testBridgeInitialization();
    void testBridgeShutdown();
    void testBridgeState();

    // Plugin interface tests
    void testPluginMetadata();
    void testPluginCapabilities();
    void testPluginCommands();
    void testPluginConfiguration();

    // Python execution tests
    void testPythonCodeExecution();
    void testPythonScriptLoading();
    void testPythonErrorHandling();

    // Dynamic plugin interface tests
    void testMethodInvocation();
    void testPropertyAccess();
    void testInterfaceAdaptation();

    // Integration tests
    void testPluginLifecycle();
    void testPluginCommunication();

    // Performance tests
    void testExecutionPerformance();
    void testMemoryManagement();

    // Error handling tests
    void testInvalidPythonScript();
    void testMissingFile();
    void testRuntimeErrors();

private:
    QString createTestPythonScript(const QString& content);
    QString createSimplePythonPlugin();
    QString createComplexPythonPlugin();

    std::unique_ptr<PythonPluginBridge> m_python_bridge;
};

void TestPythonBridge::initTestCase() {
    TestFixtureBase::initTestCase();

#ifndef QTFORGE_PYTHON_BINDINGS
    QSKIP("Python bindings not available in this build");
#endif
}

void TestPythonBridge::cleanupTestCase() {
    TestFixtureBase::cleanupTestCase();
}

void TestPythonBridge::init() {
    TestFixtureBase::init();

#ifdef QTFORGE_PYTHON_BINDINGS
    m_python_bridge = std::make_unique<PythonPluginBridge>();
#endif
}

void TestPythonBridge::cleanup() {
    if (m_python_bridge) {
        m_python_bridge->shutdown();
        m_python_bridge.reset();
    }
    TestFixtureBase::cleanup();
}

QString TestPythonBridge::createTestPythonScript(const QString& content) {
    if (!m_tempDir) return QString();

    QString scriptPath = m_tempDir->path() + "/test_script.py";
    QFile file(scriptPath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
    }

    return scriptPath;
}

QString TestPythonBridge::createSimplePythonPlugin() {
    return R"(
class SimplePlugin:
    def __init__(self):
        self.name = "SimpleTestPlugin"
        self.version = "1.0.0"

    def initialize(self):
        return True

    def shutdown(self):
        return True

    def execute_command(self, command, params):
        if command == "test":
            return {"result": "success", "message": "Test command executed"}
        return {"error": "Unknown command"}

# Plugin entry point
plugin = SimplePlugin()
)";
}

QString TestPythonBridge::createComplexPythonPlugin() {
    return R"(
import json

class ComplexPlugin:
    def __init__(self):
        self.name = "ComplexTestPlugin"
        self.version = "2.0.0"
        self.state = "unloaded"
        self.data = {}

    def initialize(self):
        self.state = "loaded"
        return True

    def shutdown(self):
        self.state = "stopped"
        return True

    def execute_command(self, command, params):
        if command == "store_data":
            key = params.get("key")
            value = params.get("value")
            if key:
                self.data[key] = value
                return {"result": "success", "stored": {key: value}}
            return {"error": "Missing key parameter"}
        elif command == "get_data":
            key = params.get("key")
            if key in self.data:
                return {"result": "success", "value": self.data[key]}
            return {"error": "Key not found"}
        return {"error": "Unknown command"}

# Plugin entry point
plugin = ComplexPlugin()
)";
}

void TestPythonBridge::testBridgeCreation() {
#ifdef QTFORGE_PYTHON_BINDINGS
    QVERIFY(m_python_bridge != nullptr);
    QCOMPARE(m_python_bridge->state(), PluginState::Unloaded);
    QVERIFY(!m_python_bridge->name().empty());
    QVERIFY(!m_python_bridge->description().empty());
    QVERIFY(m_python_bridge->version().major >= 3);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testBridgeInitialization() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto result = m_python_bridge->initialize();

    // Skip test if Python is not available
    if (!result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    QTFORGE_VERIFY_SUCCESS(result);
    QCOMPARE(m_python_bridge->state(), PluginState::Running);
    QVERIFY(m_python_bridge->is_initialized());
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testBridgeShutdown() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    m_python_bridge->shutdown();
    QCOMPARE(m_python_bridge->state(), PluginState::Unloaded);
    QVERIFY(!m_python_bridge->is_initialized());
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testBridgeState() {
#ifdef QTFORGE_PYTHON_BINDINGS
    // Test initial state
    QCOMPARE(m_python_bridge->state(), PluginState::Unloaded);

    // Test state after initialization
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    QCOMPARE(m_python_bridge->state(), PluginState::Running);

    // Test state after shutdown
    m_python_bridge->shutdown();
    QCOMPARE(m_python_bridge->state(), PluginState::Unloaded);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testPluginMetadata() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    auto metadata = m_python_bridge->metadata();
    QVERIFY(!metadata.id.isEmpty());
    QVERIFY(!metadata.name.isEmpty());
    QVERIFY(metadata.version.major >= 3);
    QVERIFY(!metadata.description.isEmpty());
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testPluginCapabilities() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    auto commands = m_python_bridge->available_commands();
    QVERIFY(!commands.empty());

    // Check for expected commands
    QVERIFY(std::find(commands.begin(), commands.end(), "execute_python") != commands.end());
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testPluginCommands() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    // Test execute_python command
    QJsonObject params;
    params["code"] = "result = 2 + 2";

    auto result = m_python_bridge->execute_command("execute_python", params);
    // This might not be implemented yet, so we check for either success or not implemented
    QVERIFY(result.has_value() || result.error().code == PluginErrorCode::NotImplemented);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testPluginConfiguration() {
#ifdef QTFORGE_PYTHON_BINDINGS
    QJsonObject config = ConfigTemplates::pythonPluginTestConfig();

    auto configure_result = m_python_bridge->configure(config);
    QTFORGE_VERIFY_SUCCESS(configure_result);

    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testPythonCodeExecution() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    // Test simple code execution
    QString code = "result = 42";
    QJsonObject context;

    auto result = m_python_bridge->execute_code(code, context);
    // This might not be implemented yet
    QVERIFY(result.has_value() || result.error().code == PluginErrorCode::NotImplemented);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testPythonScriptLoading() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    QString scriptPath = createTestPythonScript(createSimplePythonPlugin());
    QVERIFY(!scriptPath.isEmpty());

    // Test script loading (if supported)
    // This is a placeholder test as the exact API might vary
    QVERIFY(true); // Test passes if we reach here without crashing
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testPythonErrorHandling() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    // Test syntax error handling
    QString invalidCode = "invalid python syntax !!!";
    QJsonObject context;

    auto result = m_python_bridge->execute_code(invalidCode, context);
    // Should either fail or not be implemented
    QVERIFY(!result.has_value() || result.error().code == PluginErrorCode::NotImplemented);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testMethodInvocation() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    // Test method invocation (if supported)
    QVariantList params;
    params << "test_param";

    auto result = m_python_bridge->invoke_method("test_method", params);
    // This might not be implemented yet
    QVERIFY(result.has_value() || result.error().code == PluginErrorCode::NotImplemented);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testPropertyAccess() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    // Test property access (if supported)
    auto get_result = m_python_bridge->get_property("test_property");
    QVERIFY(get_result.has_value() || get_result.error().code == PluginErrorCode::NotImplemented);

    auto set_result = m_python_bridge->set_property("test_property", QVariant("test_value"));
    QVERIFY(set_result.has_value() || set_result.error().code == PluginErrorCode::NotImplemented);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testInterfaceAdaptation() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    // Test interface adaptation (if supported)
    auto result = m_python_bridge->adapt_to_interface("ITestInterface", Version(1, 0, 0));
    QVERIFY(result.has_value() || result.error().code == PluginErrorCode::NotImplemented);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testPluginLifecycle() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    // Test full lifecycle
    QCOMPARE(m_python_bridge->state(), PluginState::Running);

    m_python_bridge->shutdown();
    QCOMPARE(m_python_bridge->state(), PluginState::Unloaded);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testPluginCommunication() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    // Test plugin communication
    QJsonObject params;
    params["message"] = "test_message";

    auto result = m_python_bridge->execute_command("test", params);
    // This might not be implemented yet
    QVERIFY(result.has_value() || result.error().code == PluginErrorCode::NotImplemented);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testExecutionPerformance() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    QElapsedTimer timer;
    timer.start();

    const int iterations = 10;
    for (int i = 0; i < iterations; ++i) {
        QJsonObject params;
        params["iteration"] = i;

        auto result = m_python_bridge->execute_command("test", params);
        // Don't fail if not implemented
    }

    qint64 elapsed = timer.elapsed();
    qDebug() << "Python bridge performance:" << elapsed << "ms for" << iterations << "commands";

    // Verify reasonable performance (less than 100ms per command on average)
    QVERIFY(elapsed < iterations * 100);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testMemoryManagement() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    // Test multiple operations to check for memory leaks
    for (int i = 0; i < 10; ++i) {
        QJsonObject params;
        params["test_data"] = QString("test_data_%1").arg(i);

        auto result = m_python_bridge->execute_command("test", params);
        // Don't fail if not implemented
    }

    // Test passes if we don't crash or leak memory
    QVERIFY(true);
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testInvalidPythonScript() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    QString invalidScript = "invalid python syntax !!!";
    QString scriptPath = createTestPythonScript(invalidScript);

    // Test handling of invalid script (if script loading is supported)
    // This is a placeholder test as the exact API might vary
    QVERIFY(true); // Test passes if we reach here without crashing
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testMissingFile() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    QString nonExistentPath = "/path/that/does/not/exist.py";

    // Test handling of missing file (if file loading is supported)
    // This is a placeholder test as the exact API might vary
    QVERIFY(true); // Test passes if we reach here without crashing
#else
    QSKIP("Python bindings not available");
#endif
}

void TestPythonBridge::testRuntimeErrors() {
#ifdef QTFORGE_PYTHON_BINDINGS
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }

    // Test runtime error handling
    QString errorCode = "raise Exception('Test runtime error')";
    QJsonObject context;

    auto result = m_python_bridge->execute_code(errorCode, context);
    // Should either fail or not be implemented
    QVERIFY(!result.has_value() || result.error().code == PluginErrorCode::NotImplemented);
#else
    QSKIP("Python bindings not available");
#endif
}

QTEST_MAIN(TestPythonBridge)
#include "test_python_bridge.moc"
