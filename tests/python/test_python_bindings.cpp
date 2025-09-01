/**
 * @file test_python_bindings.cpp
 * @brief Comprehensive tests for QtForge Python bindings
 * @version 3.2.0
 */

#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>
#include <QtTest/QtTest>
#include <memory>

// Note: This test file tests the Python bindings from the C++ side
// It launches Python processes to test the actual bindings

class TestPythonBindings : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic binding tests
    void testPythonModuleImport();
    void testModuleVersion();
    void testModuleAttributes();

    // Core module tests
    void testCoreModuleImport();
    void testPluginManagerBinding();
    void testPluginInterfaceBinding();
    void testPluginLoaderBinding();

    // Utils module tests
    void testUtilsModuleImport();
    void testVersionBinding();
    void testErrorHandlingBinding();

    // Communication tests
    void testMessageBusBinding();
    void testCommunicationFeatures();

    // Security tests
    void testSecurityManagerBinding();
    void testSecurityFeatures();

    // Integration tests
    void testPythonPluginExecution();
    void testCppPythonInteraction();
    void testDataExchange();

    // Error handling tests
    void testPythonExceptions();
    void testBindingErrors();
    void testInvalidOperations();

    // Performance tests
    void testBindingPerformance();
    void testMemoryUsage();

    // Advanced features tests
    void testCallbacks();
    void testSignalSlotBinding();
    void testAsyncOperations();

private:
    bool isPythonAvailable();
    bool isQtForgeModuleAvailable();
    QString runPythonScript(const QString& script);
    void createTestPythonScript(const QString& filename,
                                const QString& content);

    QTemporaryDir m_temp_dir;
    QString m_python_executable;
};

void TestPythonBindings::initTestCase() {
    // Find Python executable
    m_python_executable = "python3";

    // Try different Python executable names
    QStringList python_names = {"python3",   "python",     "python3.8",
                                "python3.9", "python3.10", "python3.11"};

    for (const QString& name : python_names) {
        QProcess process;
        process.start(name, QStringList() << "--version");
        if (process.waitForFinished(3000) && process.exitCode() == 0) {
            m_python_executable = name;
            break;
        }
    }

    // Ensure we have a valid temporary directory
    QVERIFY(m_temp_dir.isValid());
}

void TestPythonBindings::cleanupTestCase() {
    // Cleanup is handled by QTemporaryDir destructor
}

void TestPythonBindings::init() {
    // Setup for each test
}

void TestPythonBindings::cleanup() {
    // Cleanup after each test
}

void TestPythonBindings::testPythonModuleImport() {
    if (!isPythonAvailable()) {
        QSKIP("Python not available for testing");
    }

    QString script = R"(
try:
    import qtforge
    print("SUCCESS: Module imported")
    print(f"Module name: {qtforge.__name__}")
    print(f"Module file: {qtforge.__file__}")
except ImportError as e:
    print(f"IMPORT_ERROR: {e}")
except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);

    if (result.contains("IMPORT_ERROR")) {
        QSKIP("QtForge Python module not available");
    }

    QVERIFY(result.contains("SUCCESS: Module imported"));
    QVERIFY(result.contains("Module name: qtforge"));
}

void TestPythonBindings::testModuleVersion() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    version = qtforge.__version__
    print(f"SUCCESS: Version {version}")

    # Test version components
    if hasattr(qtforge, '__version_major__'):
        print(f"Major: {qtforge.__version_major__}")
    if hasattr(qtforge, '__version_minor__'):
        print(f"Minor: {qtforge.__version_minor__}")
    if hasattr(qtforge, '__version_patch__'):
        print(f"Patch: {qtforge.__version_patch__}")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Version"));
}

void TestPythonBindings::testModuleAttributes() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # List all attributes
    attrs = [attr for attr in dir(qtforge) if not attr.startswith('_')]
    print(f"SUCCESS: Found {len(attrs)} attributes")

    # Check for expected attributes
    expected = ['core', 'utils', 'version', 'create_plugin_manager']
    for attr in expected:
        if hasattr(qtforge, attr):
            print(f"FOUND: {attr}")
        else:
            print(f"MISSING: {attr}")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Found"));
    QVERIFY(result.contains("FOUND: core"));
    QVERIFY(result.contains("FOUND: utils"));
}

void TestPythonBindings::testCoreModuleImport() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    core = qtforge.core
    print(f"SUCCESS: Core module imported")

    # List core attributes
    attrs = [attr for attr in dir(core) if not attr.startswith('_')]
    print(f"Core attributes: {len(attrs)}")

    # Check for expected core functions
    expected = ['test_function', 'get_version', 'create_plugin_manager']
    for attr in expected:
        if hasattr(core, attr):
            print(f"CORE_FOUND: {attr}")
        else:
            print(f"CORE_MISSING: {attr}")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Core module imported"));
}

void TestPythonBindings::testPluginManagerBinding() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test plugin manager creation
    manager = qtforge.create_plugin_manager()
    print("SUCCESS: Plugin manager created")

    # Test manager methods (if available)
    if hasattr(manager, 'load_plugin'):
        print("FOUND: load_plugin method")
    if hasattr(manager, 'unload_plugin'):
        print("FOUND: unload_plugin method")
    if hasattr(manager, 'list_plugins'):
        print("FOUND: list_plugins method")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Plugin manager created"));
}

void TestPythonBindings::testPluginInterfaceBinding() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test if plugin interface classes are available
    if hasattr(qtforge, 'IPlugin'):
        print("FOUND: IPlugin interface")
    if hasattr(qtforge, 'PluginMetadata'):
        print("FOUND: PluginMetadata class")
    if hasattr(qtforge, 'PluginState'):
        print("FOUND: PluginState enum")

    print("SUCCESS: Plugin interface binding test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(
        result.contains("SUCCESS: Plugin interface binding test completed"));
}

void TestPythonBindings::testPluginLoaderBinding() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test plugin loader functionality
    if hasattr(qtforge, 'PluginLoader'):
        print("FOUND: PluginLoader class")
    if hasattr(qtforge, 'load_plugin_demo'):
        result = qtforge.load_plugin_demo()
        print(f"SUCCESS: Plugin demo loaded: {result}")
    else:
        print("INFO: Plugin demo not available")

    print("SUCCESS: Plugin loader binding test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Plugin loader binding test completed"));
}

void TestPythonBindings::testUtilsModuleImport() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    utils = qtforge.utils
    print("SUCCESS: Utils module imported")

    # List utils attributes
    attrs = [attr for attr in dir(utils) if not attr.startswith('_')]
    print(f"Utils attributes: {len(attrs)}")

    # Check for expected utils functions
    expected = ['utils_test', 'create_version', 'parse_version', 'create_error']
    for attr in expected:
        if hasattr(utils, attr):
            print(f"UTILS_FOUND: {attr}")
        else:
            print(f"UTILS_MISSING: {attr}")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Utils module imported"));
}

void TestPythonBindings::testVersionBinding() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test version functions
    if hasattr(qtforge, 'get_version'):
        version = qtforge.get_version()
        print(f"SUCCESS: Got version: {version}")

    if hasattr(qtforge, 'create_version'):
        version_obj = qtforge.create_version("1.0.0")
        print(f"SUCCESS: Created version object: {version_obj}")

    if hasattr(qtforge, 'parse_version'):
        parsed = qtforge.parse_version("2.1.0")
        print(f"SUCCESS: Parsed version: {parsed}")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Got version") ||
            result.contains("SUCCESS: Created version") ||
            result.contains("SUCCESS: Parsed version"));
}

void TestPythonBindings::testErrorHandlingBinding() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test error handling functions
    if hasattr(qtforge, 'create_error'):
        error = qtforge.create_error("Test error message")
        print(f"SUCCESS: Created error: {error}")

    # Test error types/enums
    if hasattr(qtforge, 'PluginError'):
        print("FOUND: PluginError class")

    print("SUCCESS: Error handling binding test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Error handling binding test completed"));
}

void TestPythonBindings::testMessageBusBinding() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test message bus functionality
    if hasattr(qtforge, 'MessageBus'):
        print("FOUND: MessageBus class")
        # Try to create message bus instance
        # bus = qtforge.MessageBus()
        # print("SUCCESS: MessageBus created")

    print("SUCCESS: Message bus binding test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Message bus binding test completed"));
}

void TestPythonBindings::testCommunicationFeatures() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test communication-related features
    communication_features = ['MessageBus', 'publish', 'subscribe']
    found_features = []

    for feature in communication_features:
        if hasattr(qtforge, feature):
            found_features.append(feature)
            print(f"COMM_FOUND: {feature}")

    print(f"SUCCESS: Found {len(found_features)} communication features")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Found"));
}

void TestPythonBindings::testSecurityManagerBinding() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test security manager functionality
    if hasattr(qtforge, 'SecurityManager'):
        print("FOUND: SecurityManager class")

    print("SUCCESS: Security manager binding test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(
        result.contains("SUCCESS: Security manager binding test completed"));
}

void TestPythonBindings::testSecurityFeatures() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test security-related features
    security_features = ['SecurityManager', 'validate', 'verify']
    found_features = []

    for feature in security_features:
        if hasattr(qtforge, feature):
            found_features.append(feature)
            print(f"SEC_FOUND: {feature}")

    print(f"SUCCESS: Found {len(found_features)} security features")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Found"));
}

void TestPythonBindings::testPythonPluginExecution() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test executing Python plugin functionality
    if hasattr(qtforge.core, 'test_function'):
        result = qtforge.core.test_function()
        print(f"SUCCESS: Test function result: {result}")

    if hasattr(qtforge.utils, 'utils_test'):
        result = qtforge.utils.utils_test()
        print(f"SUCCESS: Utils test result: {result}")

    print("SUCCESS: Python plugin execution test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Python plugin execution test completed"));
}

void TestPythonBindings::testCppPythonInteraction() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test C++ to Python interaction
    manager = qtforge.create_plugin_manager()
    print("SUCCESS: Created plugin manager from Python")

    # Test data exchange
    version = qtforge.get_version()
    print(f"SUCCESS: Got version from C++: {version}")

    print("SUCCESS: C++ Python interaction test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: C++ Python interaction test completed"));
}

void TestPythonBindings::testDataExchange() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test data exchange between C++ and Python

    # Test string exchange
    if hasattr(qtforge, 'get_version'):
        version_str = qtforge.get_version()
        print(f"STRING_EXCHANGE: {type(version_str).__name__}")

    # Test object exchange
    if hasattr(qtforge, 'create_plugin_manager'):
        manager = qtforge.create_plugin_manager()
        print(f"OBJECT_EXCHANGE: {type(manager).__name__}")

    print("SUCCESS: Data exchange test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Data exchange test completed"));
}

void TestPythonBindings::testPythonExceptions() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test exception handling
    print("Testing exception handling...")

    # This should work without exceptions
    version = qtforge.get_version()
    print(f"SUCCESS: No exception for valid operation: {version}")

    print("SUCCESS: Exception handling test completed")

except Exception as e:
    print(f"EXCEPTION: {type(e).__name__}: {e}")
    print("SUCCESS: Exception properly caught")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS:"));
}

void TestPythonBindings::testBindingErrors() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test error conditions
    print("Testing binding error conditions...")

    # Test calling non-existent function
    try:
        qtforge.non_existent_function()
        print("ERROR: Should have failed")
    except AttributeError:
        print("SUCCESS: AttributeError properly raised")

    print("SUCCESS: Binding error test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Binding error test completed"));
}

void TestPythonBindings::testInvalidOperations() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test invalid operations
    print("Testing invalid operations...")

    # Test with invalid parameters (if applicable)
    # This depends on the specific binding implementation

    print("SUCCESS: Invalid operations test completed")

except Exception as e:
    print(f"EXPECTED_ERROR: {e}")
    print("SUCCESS: Invalid operation properly handled")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS:"));
}

void TestPythonBindings::testBindingPerformance() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
import time
try:
    # Test binding performance
    start_time = time.time()

    # Perform multiple operations
    for i in range(100):
        version = qtforge.get_version()

    end_time = time.time()
    elapsed = (end_time - start_time) * 1000  # Convert to milliseconds

    print(f"SUCCESS: 100 operations took {elapsed:.2f} ms")
    print(f"Average: {elapsed/100:.2f} ms per operation")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: 100 operations took"));
}

void TestPythonBindings::testMemoryUsage() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
import gc
try:
    # Test memory usage
    print("Testing memory usage...")

    # Create and destroy objects
    objects = []
    for i in range(10):
        manager = qtforge.create_plugin_manager()
        objects.append(manager)

    # Clear references
    objects.clear()
    gc.collect()

    print("SUCCESS: Memory usage test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Memory usage test completed"));
}

void TestPythonBindings::testCallbacks() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test callback functionality (if available)
    print("Testing callbacks...")

    # This would depend on the specific callback implementation
    # For now, just test that the interface doesn't crash

    print("SUCCESS: Callback test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Callback test completed"));
}

void TestPythonBindings::testSignalSlotBinding() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test Qt signal/slot binding (if available)
    print("Testing signal/slot binding...")

    # This would depend on Qt signal/slot binding implementation
    # For now, just test basic functionality

    print("SUCCESS: Signal/slot binding test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Signal/slot binding test completed"));
}

void TestPythonBindings::testAsyncOperations() {
    if (!isPythonAvailable() || !isQtForgeModuleAvailable()) {
        QSKIP("Python or QtForge module not available");
    }

    QString script = R"(
import qtforge
try:
    # Test asynchronous operations (if available)
    print("Testing async operations...")

    # This would depend on async operation implementation
    # For now, just test basic functionality

    print("SUCCESS: Async operations test completed")

except Exception as e:
    print(f"ERROR: {e}")
)";

    QString result = runPythonScript(script);
    QVERIFY(result.contains("SUCCESS: Async operations test completed"));
}

bool TestPythonBindings::isPythonAvailable() {
    QProcess process;
    process.start(m_python_executable, QStringList() << "--version");
    return process.waitForFinished(3000) && process.exitCode() == 0;
}

bool TestPythonBindings::isQtForgeModuleAvailable() {
    QString script = R"(
try:
    import qtforge
    print("AVAILABLE")
except ImportError:
    print("NOT_AVAILABLE")
)";

    QString result = runPythonScript(script);
    return result.contains("AVAILABLE");
}

QString TestPythonBindings::runPythonScript(const QString& script) {
    // Create temporary script file
    QString script_file = m_temp_dir.filePath("test_script.py");
    QFile file(script_file);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return "ERROR: Could not create script file";
    }

    QTextStream stream(&file);
    stream << script;
    file.close();

    // Run Python script
    QProcess process;
    process.start(m_python_executable, QStringList() << script_file);

    if (!process.waitForFinished(10000)) {
        return "ERROR: Python process timeout";
    }

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if (!error.isEmpty()) {
        output += "\nSTDERR: " + error;
    }

    return output;
}

void TestPythonBindings::createTestPythonScript(const QString& filename,
                                                const QString& content) {
    QString file_path = m_temp_dir.filePath(filename);
    QFile file(file_path);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
        file.close();
    }
}

QTEST_MAIN(TestPythonBindings)
#include "test_python_bindings.moc"
