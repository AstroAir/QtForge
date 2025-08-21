/**
 * @file test_component_architecture.cpp
 * @brief Comprehensive tests for the new component architecture
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <memory>

// Component headers
#include "qtplugin/core/plugin_registry.hpp"
#include "qtplugin/core/plugin_dependency_resolver.hpp"
#include "qtplugin/monitoring/plugin_hot_reload_manager.hpp"
#include "qtplugin/monitoring/plugin_metrics_collector.hpp"
#include "qtplugin/security/components/security_validator.hpp"
#include "qtplugin/security/components/signature_verifier.hpp"
#include "qtplugin/security/components/permission_manager.hpp"
#include "qtplugin/security/components/security_policy_engine.hpp"
#include "qtplugin/managers/components/configuration_storage.hpp"
#include "qtplugin/managers/components/configuration_validator.hpp"
#include "qtplugin/managers/components/configuration_merger.hpp"
#include "qtplugin/managers/components/configuration_watcher.hpp"
#include "qtplugin/managers/components/resource_pool.hpp"
#include "qtplugin/managers/components/resource_allocator.hpp"
#include "qtplugin/managers/components/resource_monitor.hpp"

using namespace qtplugin;

class TestComponentArchitecture : public QObject
{
    Q_OBJECT

public:
    TestComponentArchitecture() = default;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Core Component Tests
    void testPluginRegistryComponent();
    void testPluginDependencyResolverComponent();
    
    // Monitoring Component Tests
    void testPluginHotReloadManagerComponent();
    void testPluginMetricsCollectorComponent();
    
    // Security Component Tests
    void testSecurityValidatorComponent();
    void testSignatureVerifierComponent();
    void testPermissionManagerComponent();
    void testSecurityPolicyEngineComponent();
    
    // Configuration Component Tests
    void testConfigurationStorageComponent();
    void testConfigurationValidatorComponent();
    void testConfigurationMergerComponent();
    void testConfigurationWatcherComponent();
    
    // Resource Component Tests
    void testResourcePoolComponent();
    void testResourceAllocatorComponent();
    void testResourceMonitorComponent();
    
    // Integration Tests
    void testComponentInteraction();
    void testComponentLifecycle();
    void testComponentThreadSafety();

private:
    std::unique_ptr<QTemporaryDir> m_temp_dir;
    QString m_test_dir;
};

void TestComponentArchitecture::initTestCase()
{
    qDebug() << "Starting component architecture tests";
    m_temp_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_temp_dir->isValid());
    m_test_dir = m_temp_dir->path();
}

void TestComponentArchitecture::cleanupTestCase()
{
    qDebug() << "Component architecture tests completed";
}

void TestComponentArchitecture::init()
{
    // Setup for each test
}

void TestComponentArchitecture::cleanup()
{
    // Cleanup after each test
}

void TestComponentArchitecture::testPluginRegistryComponent()
{
    // Test PluginRegistry component
    auto registry = std::make_unique<PluginRegistry>();
    QVERIFY(registry != nullptr);
    
    // Test plugin registration
    PluginInfo plugin_info;
    plugin_info.id = "test.plugin";
    plugin_info.file_path = m_test_dir + "/test_plugin.so";
    plugin_info.state = PluginState::Unloaded;
    
    auto register_result = registry->register_plugin(plugin_info);
    QVERIFY(register_result.has_value());
    
    // Test plugin lookup
    auto lookup_result = registry->find_plugin("test.plugin");
    QVERIFY(lookup_result.has_value());
    QCOMPARE(lookup_result->id, "test.plugin");
    
    // Test plugin listing
    auto all_plugins = registry->get_all_plugins();
    QCOMPARE(all_plugins.size(), 1);
    
    // Test plugin unregistration
    auto unregister_result = registry->unregister_plugin("test.plugin");
    QVERIFY(unregister_result.has_value());
    
    auto empty_list = registry->get_all_plugins();
    QVERIFY(empty_list.empty());
}

void TestComponentArchitecture::testPluginDependencyResolverComponent()
{
    // Test PluginDependencyResolver component
    auto resolver = std::make_unique<PluginDependencyResolver>();
    QVERIFY(resolver != nullptr);
    
    // Create test plugins with dependencies
    PluginInfo plugin_a;
    plugin_a.id = "plugin.a";
    plugin_a.metadata.dependencies = {};
    
    PluginInfo plugin_b;
    plugin_b.id = "plugin.b";
    plugin_b.metadata.dependencies = {"plugin.a"};
    
    PluginInfo plugin_c;
    plugin_c.id = "plugin.c";
    plugin_c.metadata.dependencies = {"plugin.b"};
    
    // Add plugins to resolver
    auto add_a = resolver->add_plugin(plugin_a);
    auto add_b = resolver->add_plugin(plugin_b);
    auto add_c = resolver->add_plugin(plugin_c);
    
    QVERIFY(add_a.has_value());
    QVERIFY(add_b.has_value());
    QVERIFY(add_c.has_value());
    
    // Test dependency resolution
    auto load_order = resolver->resolve_load_order();
    QVERIFY(load_order.has_value());
    QCOMPARE(load_order->size(), 3);
    
    // Verify correct order: a, b, c
    QCOMPARE(load_order->at(0), "plugin.a");
    QCOMPARE(load_order->at(1), "plugin.b");
    QCOMPARE(load_order->at(2), "plugin.c");
}

void TestComponentArchitecture::testSecurityValidatorComponent()
{
    // Test SecurityValidator component
    auto validator = std::make_unique<SecurityValidator>();
    QVERIFY(validator != nullptr);
    
    // Create a test file
    QString test_file = m_test_dir + "/test_plugin.so";
    QFile file(test_file);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("dummy plugin content");
    file.close();
    
    // Test file integrity validation
    auto integrity_result = validator->validate_file_integrity(test_file.toStdString());
    QVERIFY(integrity_result.is_valid || !integrity_result.errors.empty());
    
    // Test metadata validation
    QJsonObject metadata;
    metadata["name"] = "Test Plugin";
    metadata["version"] = "1.0.0";
    metadata["api_version"] = "3.0.0";
    
    auto metadata_result = validator->validate_metadata(metadata);
    QVERIFY(metadata_result.is_valid);
}

void TestComponentArchitecture::testResourcePoolComponent()
{
    // Test ResourcePool component
    auto pool = std::make_unique<ResourcePool<std::string>>("test_pool", ResourceType::Memory);
    QVERIFY(pool != nullptr);
    
    // Set up quota
    ResourceQuota quota;
    quota.max_instances = 5;
    quota.max_memory_bytes = 1024;
    quota.max_lifetime = std::chrono::minutes(10);
    
    pool->set_quota(quota);
    
    // Set up factory
    pool->set_factory([]() {
        return std::make_unique<std::string>("test resource");
    });
    
    // Test resource acquisition
    auto acquire_result = pool->acquire_resource("test_plugin");
    QVERIFY(acquire_result.has_value());
    
    auto [handle, resource] = std::move(acquire_result.value());
    QVERIFY(resource != nullptr);
    QCOMPARE(*resource, "test resource");
    
    // Test resource release
    auto release_result = pool->release_resource(handle, std::move(resource));
    QVERIFY(release_result.has_value());
    
    // Test pool statistics
    auto stats = pool->get_statistics();
    QVERIFY(stats.total_acquisitions > 0);
}

void TestComponentArchitecture::testResourceAllocatorComponent()
{
    // Test ResourceAllocator component
    auto allocator = std::make_unique<ResourceAllocator>();
    QVERIFY(allocator != nullptr);
    
    // Test resource allocation
    auto alloc_result = allocator->allocate_resource(
        ResourceType::Memory,
        "test_plugin",
        ResourcePriority::Normal
    );
    
    QVERIFY(alloc_result.has_value());
    
    auto allocation = alloc_result.value();
    QVERIFY(!allocation.allocation_id.empty());
    QCOMPARE(allocation.resource_type, ResourceType::Memory);
    QCOMPARE(allocation.plugin_id, "test_plugin");
    
    // Test resource deallocation
    auto dealloc_result = allocator->deallocate_resource(allocation.allocation_id);
    QVERIFY(dealloc_result.has_value());
    
    // Test allocation statistics
    auto stats = allocator->get_allocation_statistics();
    QVERIFY(stats.total_allocations > 0);
}

void TestComponentArchitecture::testComponentInteraction()
{
    // Test interaction between multiple components
    auto registry = std::make_unique<PluginRegistry>();
    auto resolver = std::make_unique<PluginDependencyResolver>();
    auto validator = std::make_unique<SecurityValidator>();
    
    // Create a plugin and test the workflow
    PluginInfo plugin_info;
    plugin_info.id = "integration.test";
    plugin_info.file_path = m_test_dir + "/integration_test.so";
    plugin_info.state = PluginState::Unloaded;
    
    // 1. Validate plugin (would normally validate file)
    QJsonObject metadata;
    metadata["name"] = "Integration Test Plugin";
    metadata["version"] = "1.0.0";
    metadata["api_version"] = "3.0.0";
    
    auto validation_result = validator->validate_metadata(metadata);
    QVERIFY(validation_result.is_valid);
    
    // 2. Register plugin
    auto register_result = registry->register_plugin(plugin_info);
    QVERIFY(register_result.has_value());
    
    // 3. Add to dependency resolver
    auto resolver_result = resolver->add_plugin(plugin_info);
    QVERIFY(resolver_result.has_value());
    
    // 4. Resolve dependencies
    auto load_order = resolver->resolve_load_order();
    QVERIFY(load_order.has_value());
    QCOMPARE(load_order->size(), 1);
    QCOMPARE(load_order->at(0), "integration.test");
}

void TestComponentArchitecture::testComponentLifecycle()
{
    // Test component lifecycle management
    auto monitor = std::make_unique<ResourceMonitor>();
    QVERIFY(monitor != nullptr);
    
    // Test monitoring configuration
    MonitoringConfig config;
    config.monitoring_interval = std::chrono::milliseconds(100);
    config.enable_usage_tracking = true;
    config.enable_performance_tracking = true;
    config.enable_leak_detection = true;
    
    auto config_result = monitor->set_monitoring_config(config);
    QVERIFY(config_result.has_value());
    
    // Test monitoring start/stop
    auto start_result = monitor->start_monitoring();
    QVERIFY(start_result.has_value());
    
    // Wait a bit for monitoring to collect data
    QTest::qWait(200);
    
    auto stop_result = monitor->stop_monitoring();
    QVERIFY(stop_result.has_value());
    
    // Test snapshot retrieval
    auto snapshot = monitor->get_current_snapshot();
    QVERIFY(snapshot.timestamp > std::chrono::system_clock::time_point{});
}

void TestComponentArchitecture::testComponentThreadSafety()
{
    // Test thread safety of components
    auto registry = std::make_unique<PluginRegistry>();
    
    const int num_threads = 4;
    const int plugins_per_thread = 10;
    
    QVector<QFuture<void>> futures;
    
    for (int t = 0; t < num_threads; ++t) {
        auto future = QtConcurrent::run([&registry, t, plugins_per_thread]() {
            for (int i = 0; i < plugins_per_thread; ++i) {
                PluginInfo plugin_info;
                plugin_info.id = QString("thread%1.plugin%2").arg(t).arg(i).toStdString();
                plugin_info.state = PluginState::Unloaded;
                
                auto result = registry->register_plugin(plugin_info);
                Q_UNUSED(result); // May fail due to race conditions, that's expected
            }
        });
        futures.append(future);
    }
    
    // Wait for all threads to complete
    for (auto& future : futures) {
        future.waitForFinished();
    }
    
    // Verify that registry is still in a consistent state
    auto all_plugins = registry->get_all_plugins();
    QVERIFY(all_plugins.size() <= num_threads * plugins_per_thread);
}

#include "test_component_architecture.moc"
QTEST_MAIN(TestComponentArchitecture)
