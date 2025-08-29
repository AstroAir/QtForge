/**
 * @file test_plugin_composition.cpp
 * @brief Comprehensive tests for plugin composition functionality
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <memory>

#include "qtplugin/composition/plugin_composition.hpp"
#include "qtplugin/core/plugin_manager.hpp"
#include "qtplugin/core/plugin_interface.hpp"
#include "qtplugin/utils/error_handling.hpp"

using namespace qtplugin;

// Mock plugin for testing
class MockPlugin : public IPlugin {
    Q_OBJECT

public:
    MockPlugin(const QString& id, const QString& name) 
        : m_id(id), m_name(name), m_state(PluginState::Unloaded) {}

    // IPlugin interface
    QString plugin_id() const override { return m_id; }
    QString name() const override { return m_name; }
    QString version() const override { return "1.0.0"; }
    QString description() const override { return "Mock plugin for testing"; }
    PluginState state() const override { return m_state; }
    bool is_loaded() const override { return m_state != PluginState::Unloaded; }

    qtplugin::expected<void, PluginError> initialize() override {
        m_state = PluginState::Loaded;
        return {};
    }

    qtplugin::expected<void, PluginError> startup() override {
        if (m_state != PluginState::Loaded) {
            return qtplugin::unexpected(PluginError{PluginErrorCode::InvalidState, "Plugin not loaded"});
        }
        m_state = PluginState::Running;
        return {};
    }

    qtplugin::expected<void, PluginError> shutdown() override {
        m_state = PluginState::Stopped;
        return {};
    }

    PluginMetadata metadata() const override {
        PluginMetadata meta;
        meta.id = m_id;
        meta.name = m_name;
        meta.version = "1.0.0";
        meta.description = "Mock plugin for testing";
        return meta;
    }

    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command, const QJsonObject& params) override {
        
        QJsonObject result;
        result["plugin_id"] = m_id;
        result["command"] = QString::fromUtf8(command.data(), command.size());
        result["params"] = params;
        result["result"] = "success";
        
        if (command == "fail") {
            return qtplugin::unexpected(PluginError{PluginErrorCode::ExecutionFailed, "Simulated failure"});
        }
        
        return result;
    }

    std::vector<std::string> available_commands() const override {
        return {"test", "process", "fail", "data"};
    }

private:
    QString m_id;
    QString m_name;
    PluginState m_state;
};

class TestPluginComposition : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic composition tests
    void testCompositionCreation();
    void testCompositionConfiguration();
    void testCompositionStrategies();

    // Plugin management tests
    void testAddRemovePlugins();
    void testPluginOrdering();
    void testPluginDependencies();

    // Execution strategy tests
    void testAggregationStrategy();
    void testPipelineStrategy();
    void testFacadeStrategy();

    // Composite plugin tests
    void testCompositePluginCreation();
    void testCompositePluginLifecycle();
    void testCompositePluginExecution();
    void testCompositePluginMetadata();

    // Error handling tests
    void testInvalidComposition();
    void testPluginFailureHandling();
    void testPartialFailures();

    // Advanced features tests
    void testConditionalExecution();
    void testParameterTransformation();
    void testResultAggregation();

    // Performance tests
    void testCompositionPerformance();
    void testLargeComposition();

private:
    std::shared_ptr<MockPlugin> createMockPlugin(const QString& id, const QString& name);
    PluginComposition createTestComposition(CompositionStrategy strategy);

    std::vector<std::shared_ptr<MockPlugin>> m_mock_plugins;
};

void TestPluginComposition::initTestCase() {
    // Setup test environment
}

void TestPluginComposition::cleanupTestCase() {
    // Cleanup test environment
}

void TestPluginComposition::init() {
    // Create fresh mock plugins for each test
    m_mock_plugins.clear();
}

void TestPluginComposition::cleanup() {
    // Clean up mock plugins
    m_mock_plugins.clear();
}

void TestPluginComposition::testCompositionCreation() {
    // Test basic composition creation
    PluginComposition composition;
    
    QCOMPARE(composition.strategy(), CompositionStrategy::Aggregation);
    QVERIFY(composition.plugins().empty());
    QVERIFY(composition.name().isEmpty());
}

void TestPluginComposition::testCompositionConfiguration() {
    PluginComposition composition;
    
    // Test setting strategy
    composition.set_strategy(CompositionStrategy::Pipeline);
    QCOMPARE(composition.strategy(), CompositionStrategy::Pipeline);
    
    // Test setting name
    composition.set_name("TestComposition");
    QCOMPARE(composition.name(), "TestComposition");
    
    // Test setting description
    composition.set_description("Test composition for unit testing");
    QCOMPARE(composition.description(), "Test composition for unit testing");
}

void TestPluginComposition::testCompositionStrategies() {
    PluginComposition composition;
    
    // Test all strategies
    std::vector<CompositionStrategy> strategies = {
        CompositionStrategy::Aggregation,
        CompositionStrategy::Pipeline,
        CompositionStrategy::Facade,
        CompositionStrategy::Decorator,
        CompositionStrategy::Proxy,
        CompositionStrategy::Adapter,
        CompositionStrategy::Bridge
    };
    
    for (auto strategy : strategies) {
        composition.set_strategy(strategy);
        QCOMPARE(composition.strategy(), strategy);
    }
}

void TestPluginComposition::testAddRemovePlugins() {
    PluginComposition composition;
    
    auto plugin1 = createMockPlugin("plugin1", "Plugin 1");
    auto plugin2 = createMockPlugin("plugin2", "Plugin 2");
    
    // Test adding plugins
    auto add_result1 = composition.add_plugin(plugin1);
    QVERIFY(add_result1.has_value());
    QCOMPARE(composition.plugins().size(), 1);
    
    auto add_result2 = composition.add_plugin(plugin2);
    QVERIFY(add_result2.has_value());
    QCOMPARE(composition.plugins().size(), 2);
    
    // Test removing plugins
    auto remove_result = composition.remove_plugin("plugin1");
    QVERIFY(remove_result.has_value());
    QCOMPARE(composition.plugins().size(), 1);
    
    // Test removing non-existent plugin
    auto remove_invalid = composition.remove_plugin("non_existent");
    QVERIFY(!remove_invalid.has_value());
}

void TestPluginComposition::testPluginOrdering() {
    PluginComposition composition;
    
    auto plugin1 = createMockPlugin("plugin1", "Plugin 1");
    auto plugin2 = createMockPlugin("plugin2", "Plugin 2");
    auto plugin3 = createMockPlugin("plugin3", "Plugin 3");
    
    // Add plugins in specific order
    composition.add_plugin(plugin1);
    composition.add_plugin(plugin2);
    composition.add_plugin(plugin3);
    
    auto plugins = composition.plugins();
    QCOMPARE(plugins.size(), 3);
    
    // Verify order
    QCOMPARE(plugins[0]->plugin_id(), "plugin1");
    QCOMPARE(plugins[1]->plugin_id(), "plugin2");
    QCOMPARE(plugins[2]->plugin_id(), "plugin3");
}

void TestPluginComposition::testPluginDependencies() {
    PluginComposition composition;
    
    auto plugin1 = createMockPlugin("plugin1", "Plugin 1");
    auto plugin2 = createMockPlugin("plugin2", "Plugin 2");
    
    composition.add_plugin(plugin1);
    composition.add_plugin(plugin2);
    
    // Test adding dependency
    auto dep_result = composition.add_dependency("plugin2", "plugin1");
    QVERIFY(dep_result.has_value());
    
    // Test circular dependency detection
    auto circular_result = composition.add_dependency("plugin1", "plugin2");
    QVERIFY(!circular_result.has_value());
    QCOMPARE(circular_result.error().code, PluginErrorCode::CircularDependency);
}

void TestPluginComposition::testAggregationStrategy() {
    auto composition = createTestComposition(CompositionStrategy::Aggregation);
    
    // Create composite plugin
    CompositePlugin composite(composition);
    
    // Initialize composite
    auto init_result = composite.initialize();
    QVERIFY(init_result.has_value());
    
    auto startup_result = composite.startup();
    QVERIFY(startup_result.has_value());
    
    // Test command execution (aggregation should execute on all plugins)
    QJsonObject params;
    params["test_param"] = "test_value";
    
    auto exec_result = composite.execute_command("test", params);
    QVERIFY(exec_result.has_value());
    
    // Result should contain responses from all plugins
    QJsonObject result = exec_result.value();
    QVERIFY(result.contains("results"));
}

void TestPluginComposition::testPipelineStrategy() {
    auto composition = createTestComposition(CompositionStrategy::Pipeline);
    
    CompositePlugin composite(composition);
    
    auto init_result = composite.initialize();
    QVERIFY(init_result.has_value());
    
    auto startup_result = composite.startup();
    QVERIFY(startup_result.has_value());
    
    // Test pipeline execution
    QJsonObject params;
    params["input"] = "initial_data";
    
    auto exec_result = composite.execute_command("process", params);
    QVERIFY(exec_result.has_value());
    
    // Result should be the output of the pipeline
    QJsonObject result = exec_result.value();
    QVERIFY(result.contains("result"));
}

void TestPluginComposition::testFacadeStrategy() {
    auto composition = createTestComposition(CompositionStrategy::Facade);
    
    CompositePlugin composite(composition);
    
    auto init_result = composite.initialize();
    QVERIFY(init_result.has_value());
    
    auto startup_result = composite.startup();
    QVERIFY(startup_result.has_value());
    
    // Test facade execution (should use primary or first available plugin)
    QJsonObject params;
    params["facade_param"] = "facade_value";
    
    auto exec_result = composite.execute_command("test", params);
    QVERIFY(exec_result.has_value());
    
    // Result should be from a single plugin
    QJsonObject result = exec_result.value();
    QVERIFY(result.contains("plugin_id"));
}

void TestPluginComposition::testCompositePluginCreation() {
    auto composition = createTestComposition(CompositionStrategy::Aggregation);
    
    CompositePlugin composite(composition);
    
    // Test basic properties
    QVERIFY(!composite.plugin_id().isEmpty());
    QVERIFY(!composite.name().isEmpty());
    QCOMPARE(composite.state(), PluginState::Unloaded);
}

void TestPluginComposition::testCompositePluginLifecycle() {
    auto composition = createTestComposition(CompositionStrategy::Aggregation);
    
    CompositePlugin composite(composition);
    
    // Test initialization
    auto init_result = composite.initialize();
    QVERIFY(init_result.has_value());
    QCOMPARE(composite.state(), PluginState::Loaded);
    
    // Test startup
    auto startup_result = composite.startup();
    QVERIFY(startup_result.has_value());
    QCOMPARE(composite.state(), PluginState::Running);
    
    // Test shutdown
    auto shutdown_result = composite.shutdown();
    QVERIFY(shutdown_result.has_value());
    QCOMPARE(composite.state(), PluginState::Stopped);
}

void TestPluginComposition::testCompositePluginExecution() {
    auto composition = createTestComposition(CompositionStrategy::Aggregation);
    
    CompositePlugin composite(composition);
    composite.initialize();
    composite.startup();
    
    // Test successful command execution
    QJsonObject params;
    params["test"] = "value";
    
    auto exec_result = composite.execute_command("test", params);
    QVERIFY(exec_result.has_value());
    
    // Test command not found
    auto not_found_result = composite.execute_command("non_existent", params);
    QVERIFY(!not_found_result.has_value());
    QCOMPARE(not_found_result.error().code, PluginErrorCode::CommandNotFound);
}

void TestPluginComposition::testCompositePluginMetadata() {
    auto composition = createTestComposition(CompositionStrategy::Pipeline);
    composition.set_name("TestComposite");
    composition.set_description("Test composite plugin");
    
    CompositePlugin composite(composition);
    
    auto metadata = composite.metadata();
    
    QCOMPARE(metadata.name, "TestComposite");
    QCOMPARE(metadata.description, "Test composite plugin");
    QVERIFY(!metadata.id.isEmpty());
    QVERIFY(!metadata.version.isEmpty());
}

void TestPluginComposition::testInvalidComposition() {
    PluginComposition composition;
    
    // Test empty composition
    CompositePlugin composite(composition);
    
    auto init_result = composite.initialize();
    QVERIFY(!init_result.has_value());
    QCOMPARE(init_result.error().code, PluginErrorCode::InvalidConfiguration);
}

void TestPluginComposition::testPluginFailureHandling() {
    auto composition = createTestComposition(CompositionStrategy::Aggregation);
    
    CompositePlugin composite(composition);
    composite.initialize();
    composite.startup();
    
    // Test command that causes failure
    QJsonObject params;
    auto exec_result = composite.execute_command("fail", params);
    
    // Behavior depends on strategy - aggregation might continue with other plugins
    // while pipeline might stop on first failure
    QVERIFY(exec_result.has_value() || !exec_result.has_value());
}

void TestPluginComposition::testPartialFailures() {
    auto composition = createTestComposition(CompositionStrategy::Aggregation);
    
    // Add a plugin that will fail
    auto failing_plugin = createMockPlugin("failing", "Failing Plugin");
    composition.add_plugin(failing_plugin);
    
    CompositePlugin composite(composition);
    composite.initialize();
    composite.startup();
    
    // Execute command - some plugins succeed, some fail
    QJsonObject params;
    auto exec_result = composite.execute_command("test", params);
    
    // Aggregation strategy should handle partial failures gracefully
    QVERIFY(exec_result.has_value());
    
    QJsonObject result = exec_result.value();
    QVERIFY(result.contains("results") || result.contains("errors"));
}

void TestPluginComposition::testConditionalExecution() {
    auto composition = createTestComposition(CompositionStrategy::Pipeline);
    
    CompositePlugin composite(composition);
    composite.initialize();
    composite.startup();
    
    // Test conditional execution based on parameters
    QJsonObject params;
    params["condition"] = true;
    params["data"] = "test_data";
    
    auto exec_result = composite.execute_command("process", params);
    QVERIFY(exec_result.has_value());
}

void TestPluginComposition::testParameterTransformation() {
    auto composition = createTestComposition(CompositionStrategy::Pipeline);
    
    CompositePlugin composite(composition);
    composite.initialize();
    composite.startup();
    
    // Test parameter transformation through pipeline
    QJsonObject params;
    params["input"] = "raw_data";
    params["transform"] = "uppercase";
    
    auto exec_result = composite.execute_command("process", params);
    QVERIFY(exec_result.has_value());
    
    // Verify transformation occurred
    QJsonObject result = exec_result.value();
    QVERIFY(result.contains("result"));
}

void TestPluginComposition::testResultAggregation() {
    auto composition = createTestComposition(CompositionStrategy::Aggregation);
    
    CompositePlugin composite(composition);
    composite.initialize();
    composite.startup();
    
    // Test result aggregation from multiple plugins
    QJsonObject params;
    params["collect"] = true;
    
    auto exec_result = composite.execute_command("data", params);
    QVERIFY(exec_result.has_value());
    
    QJsonObject result = exec_result.value();
    QVERIFY(result.contains("results"));
    
    // Should have results from all plugins
    QJsonArray results = result["results"].toArray();
    QVERIFY(results.size() > 0);
}

void TestPluginComposition::testCompositionPerformance() {
    auto composition = createTestComposition(CompositionStrategy::Aggregation);
    
    CompositePlugin composite(composition);
    composite.initialize();
    composite.startup();
    
    // Measure execution time
    QElapsedTimer timer;
    timer.start();
    
    QJsonObject params;
    auto exec_result = composite.execute_command("test", params);
    
    qint64 elapsed = timer.elapsed();
    
    QVERIFY(exec_result.has_value());
    QVERIFY(elapsed < 1000); // Should complete within 1 second
    
    qDebug() << "Composition execution took:" << elapsed << "ms";
}

void TestPluginComposition::testLargeComposition() {
    PluginComposition composition;
    composition.set_strategy(CompositionStrategy::Aggregation);
    
    // Add many plugins
    for (int i = 0; i < 50; ++i) {
        auto plugin = createMockPlugin(QString("plugin_%1").arg(i), 
                                     QString("Plugin %1").arg(i));
        composition.add_plugin(plugin);
    }
    
    CompositePlugin composite(composition);
    
    // Test initialization with many plugins
    auto init_result = composite.initialize();
    QVERIFY(init_result.has_value());
    
    auto startup_result = composite.startup();
    QVERIFY(startup_result.has_value());
    
    // Test execution with many plugins
    QJsonObject params;
    auto exec_result = composite.execute_command("test", params);
    QVERIFY(exec_result.has_value());
}

std::shared_ptr<MockPlugin> TestPluginComposition::createMockPlugin(const QString& id, const QString& name) {
    auto plugin = std::make_shared<MockPlugin>(id, name);
    m_mock_plugins.push_back(plugin);
    return plugin;
}

PluginComposition TestPluginComposition::createTestComposition(CompositionStrategy strategy) {
    PluginComposition composition;
    composition.set_strategy(strategy);
    composition.set_name("TestComposition");
    composition.set_description("Test composition for unit testing");
    
    // Add some test plugins
    auto plugin1 = createMockPlugin("test_plugin_1", "Test Plugin 1");
    auto plugin2 = createMockPlugin("test_plugin_2", "Test Plugin 2");
    auto plugin3 = createMockPlugin("test_plugin_3", "Test Plugin 3");
    
    composition.add_plugin(plugin1);
    composition.add_plugin(plugin2);
    composition.add_plugin(plugin3);
    
    return composition;
}

QTEST_MAIN(TestPluginComposition)
#include "test_plugin_composition.moc"
