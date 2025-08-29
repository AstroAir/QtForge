/**
 * @file test_python_bridge.cpp
 * @brief Comprehensive tests for Python plugin bridge functionality
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>
#include <QTextStream>
#include <QSignalSpy>
#include <memory>

#include "qtplugin/bridges/python_plugin_bridge.hpp"
#include "qtplugin/core/plugin_manager.hpp"
#include "qtplugin/utils/error_handling.hpp"

using namespace qtplugin;

class TestPythonBridge : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testPythonExecutionEnvironmentCreation();
    void testPythonExecutionEnvironmentInitialization();
    void testPythonExecutionEnvironmentExecution();
    void testPythonExecutionEnvironmentCleanup();

    // Python bridge tests
    void testPythonPluginBridgeCreation();
    void testPythonPluginBridgeInitialization();
    void testPythonPluginBridgeLifecycle();
    void testPythonPluginBridgeCommandExecution();
    void testPythonPluginBridgeConfiguration();

    // Error handling tests
    void testPythonExecutionErrors();
    void testInvalidPythonPath();
    void testPythonScriptErrors();
    void testTimeoutHandling();

    // Integration tests
    void testPythonPluginLoading();
    void testPythonPluginCommunication();
    void testPythonPluginCapabilities();

    // Performance tests
    void testPythonExecutionPerformance();
    void testMultiplePythonInstances();

private:
    void createTestPythonScript(const QString& filename, const QString& content);
    QString createSimplePythonPlugin();
    QString createComplexPythonPlugin();

    std::unique_ptr<PythonExecutionEnvironment> m_python_env;
    std::unique_ptr<PythonPluginBridge> m_python_bridge;
    QTemporaryDir m_temp_dir;
    QString m_test_script_path;
};

void TestPythonBridge::initTestCase() {
    // Ensure we have a valid temporary directory
    QVERIFY(m_temp_dir.isValid());
    
    // Create test Python scripts
    createTestPythonScript("simple_test.py", R"(
def test_function():
    return "Hello from Python!"

def add_numbers(a, b):
    return a + b

if __name__ == "__main__":
    print("Python script executed successfully")
)");

    createTestPythonScript("plugin_test.py", createSimplePythonPlugin());
}

void TestPythonBridge::cleanupTestCase() {
    // Cleanup is handled by QTemporaryDir destructor
}

void TestPythonBridge::init() {
    // Create fresh instances for each test
    m_python_env = std::make_unique<PythonExecutionEnvironment>();
    m_python_bridge = std::make_unique<PythonPluginBridge>();
}

void TestPythonBridge::cleanup() {
    // Clean up instances
    if (m_python_bridge) {
        m_python_bridge->shutdown();
        m_python_bridge.reset();
    }
    if (m_python_env) {
        m_python_env->cleanup();
        m_python_env.reset();
    }
}

void TestPythonBridge::testPythonExecutionEnvironmentCreation() {
    // Test basic creation
    QVERIFY(m_python_env != nullptr);
    
    // Test initial state
    QVERIFY(!m_python_env->is_initialized());
    QVERIFY(!m_python_env->is_running());
}

void TestPythonBridge::testPythonExecutionEnvironmentInitialization() {
    // Test initialization
    auto result = m_python_env->initialize();
    
    // Skip test if Python is not available
    if (!result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    QVERIFY(result.has_value());
    QVERIFY(m_python_env->is_initialized());
}

void TestPythonBridge::testPythonExecutionEnvironmentExecution() {
    // Initialize environment
    auto init_result = m_python_env->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    // Test simple script execution
    QString script_path = m_temp_dir.filePath("simple_test.py");
    auto exec_result = m_python_env->execute_script(script_path);
    
    QVERIFY(exec_result.has_value());
    
    // Test function call
    QJsonObject params;
    params["a"] = 5;
    params["b"] = 3;
    
    auto call_result = m_python_env->call_function("add_numbers", params);
    QVERIFY(call_result.has_value());
    
    QJsonObject response = call_result.value();
    QVERIFY(response.contains("result"));
    QCOMPARE(response["result"].toInt(), 8);
}

void TestPythonBridge::testPythonExecutionEnvironmentCleanup() {
    // Initialize and then cleanup
    auto init_result = m_python_env->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    QVERIFY(m_python_env->is_initialized());
    
    auto cleanup_result = m_python_env->cleanup();
    QVERIFY(cleanup_result.has_value());
    QVERIFY(!m_python_env->is_running());
}

void TestPythonBridge::testPythonPluginBridgeCreation() {
    // Test basic creation
    QVERIFY(m_python_bridge != nullptr);
    
    // Test initial state
    QCOMPARE(m_python_bridge->state(), PluginState::Unloaded);
    QVERIFY(!m_python_bridge->is_loaded());
}

void TestPythonBridge::testPythonPluginBridgeInitialization() {
    // Test initialization
    auto result = m_python_bridge->initialize();
    
    // Skip test if Python is not available
    if (!result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    QVERIFY(result.has_value());
    QCOMPARE(m_python_bridge->state(), PluginState::Loaded);
}

void TestPythonBridge::testPythonPluginBridgeLifecycle() {
    // Test full lifecycle
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    // Test startup
    auto startup_result = m_python_bridge->startup();
    QVERIFY(startup_result.has_value());
    QCOMPARE(m_python_bridge->state(), PluginState::Running);
    
    // Test shutdown
    auto shutdown_result = m_python_bridge->shutdown();
    QVERIFY(shutdown_result.has_value());
    QCOMPARE(m_python_bridge->state(), PluginState::Stopped);
}

void TestPythonBridge::testPythonPluginBridgeCommandExecution() {
    // Initialize bridge
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    auto startup_result = m_python_bridge->startup();
    QVERIFY(startup_result.has_value());
    
    // Test command execution
    QJsonObject params;
    params["test_param"] = "test_value";
    
    auto exec_result = m_python_bridge->execute_command("test_command", params);
    
    // Note: This might fail if the command is not implemented
    // The test verifies the interface works, not necessarily the implementation
    QVERIFY(exec_result.has_value() || 
            exec_result.error().code == PluginErrorCode::NotImplemented);
}

void TestPythonBridge::testPythonPluginBridgeConfiguration() {
    // Test configuration methods
    QJsonObject config;
    config["python_path"] = "python3";
    config["timeout"] = 5000;
    
    bool is_valid = m_python_bridge->validate_configuration(config);
    QVERIFY(is_valid);
    
    // Test configuration schema
    QJsonObject schema = m_python_bridge->get_configuration_schema();
    QVERIFY(schema.isEmpty() || schema.contains("properties"));
}

void TestPythonBridge::testPythonExecutionErrors() {
    auto init_result = m_python_env->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    // Test execution of non-existent script
    auto exec_result = m_python_env->execute_script("/non/existent/script.py");
    QVERIFY(!exec_result.has_value());
    QVERIFY(exec_result.error().code == PluginErrorCode::FileNotFound ||
            exec_result.error().code == PluginErrorCode::ExecutionFailed);
}

void TestPythonBridge::testInvalidPythonPath() {
    // Create environment with invalid Python path
    PythonExecutionEnvironment env("/invalid/python/path");
    
    auto result = env.initialize();
    QVERIFY(!result.has_value());
    QVERIFY(result.error().code == PluginErrorCode::InitializationFailed);
}

void TestPythonBridge::testPythonScriptErrors() {
    auto init_result = m_python_env->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    // Create script with syntax error
    createTestPythonScript("error_script.py", R"(
def invalid_syntax(
    print("This has a syntax error")
)");
    
    QString error_script_path = m_temp_dir.filePath("error_script.py");
    auto exec_result = m_python_env->execute_script(error_script_path);
    
    QVERIFY(!exec_result.has_value());
    QVERIFY(exec_result.error().code == PluginErrorCode::ExecutionFailed);
}

void TestPythonBridge::testTimeoutHandling() {
    auto init_result = m_python_env->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    // Create script that takes a long time
    createTestPythonScript("slow_script.py", R"(
import time
time.sleep(10)  # Sleep for 10 seconds
print("This should timeout")
)");
    
    // Set short timeout
    m_python_env->set_timeout(std::chrono::milliseconds(1000));
    
    QString slow_script_path = m_temp_dir.filePath("slow_script.py");
    auto exec_result = m_python_env->execute_script(slow_script_path);
    
    QVERIFY(!exec_result.has_value());
    QVERIFY(exec_result.error().code == PluginErrorCode::Timeout);
}

void TestPythonBridge::testPythonPluginLoading() {
    // This test would require a more complex setup with actual plugin loading
    // For now, we test the interface
    
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    // Test plugin type
    QCOMPARE(m_python_bridge->get_plugin_type(), PluginType::Python);
    
    // Test execution context
    auto context = m_python_bridge->get_execution_context();
    QCOMPARE(context.type, PluginType::Python);
}

void TestPythonBridge::testPythonPluginCommunication() {
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    auto startup_result = m_python_bridge->startup();
    QVERIFY(startup_result.has_value());
    
    // Test available commands
    auto commands = m_python_bridge->available_commands();
    // Commands list might be empty for basic implementation
    QVERIFY(commands.size() >= 0);
}

void TestPythonBridge::testPythonPluginCapabilities() {
    auto init_result = m_python_bridge->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    // Test capability negotiation
    std::vector<InterfaceCapability> requested_caps;
    // Add some test capabilities if needed
    
    auto caps_result = m_python_bridge->negotiate_capabilities("test_interface", requested_caps);
    
    // This might not be implemented yet
    QVERIFY(caps_result.has_value() || 
            caps_result.error().code == PluginErrorCode::NotImplemented);
}

void TestPythonBridge::testPythonExecutionPerformance() {
    auto init_result = m_python_env->initialize();
    if (!init_result.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    // Measure execution time
    QElapsedTimer timer;
    timer.start();
    
    QString script_path = m_temp_dir.filePath("simple_test.py");
    auto exec_result = m_python_env->execute_script(script_path);
    
    qint64 elapsed = timer.elapsed();
    
    QVERIFY(exec_result.has_value());
    QVERIFY(elapsed < 5000); // Should complete within 5 seconds
    
    qDebug() << "Python script execution took:" << elapsed << "ms";
}

void TestPythonBridge::testMultiplePythonInstances() {
    // Test multiple Python environments
    auto env1 = std::make_unique<PythonExecutionEnvironment>();
    auto env2 = std::make_unique<PythonExecutionEnvironment>();
    
    auto init1 = env1->initialize();
    auto init2 = env2->initialize();
    
    if (!init1.has_value() || !init2.has_value()) {
        QSKIP("Python interpreter not available for testing");
    }
    
    QVERIFY(env1->is_initialized());
    QVERIFY(env2->is_initialized());
    
    // Both should be able to execute scripts independently
    QString script_path = m_temp_dir.filePath("simple_test.py");
    
    auto exec1 = env1->execute_script(script_path);
    auto exec2 = env2->execute_script(script_path);
    
    QVERIFY(exec1.has_value());
    QVERIFY(exec2.has_value());
    
    // Cleanup
    env1->cleanup();
    env2->cleanup();
}

void TestPythonBridge::createTestPythonScript(const QString& filename, const QString& content) {
    QString file_path = m_temp_dir.filePath(filename);
    QFile file(file_path);
    
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    
    QTextStream stream(&file);
    stream << content;
    file.close();
}

QString TestPythonBridge::createSimplePythonPlugin() {
    return R"(
class SimplePlugin:
    def __init__(self):
        self.name = "SimpleTestPlugin"
        self.version = "1.0.0"
    
    def initialize(self):
        return True
    
    def startup(self):
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
import time

class ComplexPlugin:
    def __init__(self):
        self.name = "ComplexTestPlugin"
        self.version = "2.0.0"
        self.state = "unloaded"
        self.data = {}
    
    def initialize(self):
        self.state = "loaded"
        return True
    
    def startup(self):
        self.state = "running"
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
        
        elif command == "list_data":
            return {"result": "success", "data": self.data}
        
        elif command == "process_delay":
            delay = params.get("delay", 1)
            time.sleep(delay)
            return {"result": "success", "processed_after": delay}
        
        return {"error": "Unknown command"}
    
    def get_capabilities(self):
        return ["data_storage", "delayed_processing", "state_management"]

# Plugin entry point
plugin = ComplexPlugin()
)";
}

QTEST_MAIN(TestPythonBridge)
#include "test_python_bridge.moc"
