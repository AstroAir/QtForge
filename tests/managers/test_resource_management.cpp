/**
 * @file test_resource_management.cpp
 * @brief Comprehensive tests for resource management system
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTimer>
#include <QThread>
#include <memory>
#include <chrono>

#include "qtplugin/managers/resource_manager_impl.hpp"
#include "qtplugin/managers/resource_lifecycle_impl.hpp"
#include "qtplugin/managers/resource_monitor_impl.hpp"
#include "qtplugin/managers/resource_pools.hpp"

using namespace qtplugin;

class TestResourceManagement : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Resource Manager Tests
    void testResourceManagerCreation();
    void testResourceFactoryRegistration();
    void testResourcePoolCreation();
    void testResourceAcquisition();
    void testResourceRelease();
    void testResourceQuotas();
    void testResourceStatistics();

    // Resource Lifecycle Tests
    void testLifecycleTracking();
    void testStateTransitions();
    void testDependencyManagement();
    void testAutomaticCleanup();
    void testLifecycleEvents();

    // Resource Monitor Tests
    void testMonitoringStart();
    void testMetricsCollection();
    void testPerformanceAlerts();
    void testQuotaViolations();
    void testEfficiencyReporting();
    void testDataExport();

    // Resource Pools Tests
    void testThreadPool();
    void testTimerPool();
    void testMemoryPool();
    void testNetworkConnectionPool();

    // Integration Tests
    void testResourceManagerIntegration();
    void testCompleteResourceLifecycle();
    void testResourceCleanupOnPluginUnload();

private:
    std::unique_ptr<IResourceManager> m_resource_manager;
    std::unique_ptr<IResourceLifecycleManager> m_lifecycle_manager;
    std::unique_ptr<IResourceMonitor> m_resource_monitor;
};

void TestResourceManagement::initTestCase() {
    qDebug() << "Starting resource management tests";
}

void TestResourceManagement::cleanupTestCase() {
    qDebug() << "Resource management tests completed";
}

void TestResourceManagement::init() {
    // Create fresh instances for each test
    m_resource_manager = create_resource_manager();
    m_lifecycle_manager = create_resource_lifecycle_manager();
    m_resource_monitor = create_resource_monitor();
}

void TestResourceManagement::cleanup() {
    // Clean up instances
    m_resource_manager.reset();
    m_lifecycle_manager.reset();
    m_resource_monitor.reset();
}

void TestResourceManagement::testResourceManagerCreation() {
    QVERIFY(m_resource_manager != nullptr);
    
    // Test initial state
    auto stats = m_resource_manager->get_statistics();
    QVERIFY(stats.contains("pools_count"));
    QCOMPARE(stats["pools_count"].toInt(), 0);
    
    // Test supported resource types
    QVERIFY(m_resource_manager->is_resource_type_supported(ResourceType::Thread));
    QVERIFY(m_resource_manager->is_resource_type_supported(ResourceType::Timer));
    QVERIFY(m_resource_manager->is_resource_type_supported(ResourceType::Memory));
}

void TestResourceManagement::testResourceFactoryRegistration() {
    // Test factory registration
    auto thread_factory = std::make_unique<ThreadResourceFactory>();
    QVERIFY(thread_factory->name() == "thread");
    
    auto timer_factory = std::make_unique<TimerResourceFactory>();
    QVERIFY(timer_factory->name() == "timer");
    
    auto memory_factory = std::make_unique<MemoryResourceFactory>();
    QVERIFY(memory_factory->name() == "memory");
}

void TestResourceManagement::testResourcePoolCreation() {
    // Test pool creation
    ResourceQuota quota;
    quota.max_instances = 10;
    quota.max_lifetime = std::chrono::minutes(30);
    
    auto result = m_resource_manager->create_pool(ResourceType::Thread, "test_thread_pool", quota);
    QVERIFY(result.has_value());
    
    // Test duplicate pool creation
    auto duplicate_result = m_resource_manager->create_pool(ResourceType::Thread, "test_thread_pool", quota);
    QVERIFY(!duplicate_result.has_value());
    QCOMPARE(duplicate_result.error().code, PluginErrorCode::AlreadyExists);
    
    // Test pool removal
    auto remove_result = m_resource_manager->remove_pool("test_thread_pool");
    QVERIFY(remove_result.has_value());
    
    // Test removing non-existent pool
    auto remove_nonexistent = m_resource_manager->remove_pool("nonexistent_pool");
    QVERIFY(!remove_nonexistent.has_value());
    QCOMPARE(remove_nonexistent.error().code, PluginErrorCode::NotFound);
}

void TestResourceManagement::testResourceAcquisition() {
    // This test would require a more complete implementation
    // For now, we test the interface
    QVERIFY(m_resource_manager != nullptr);
    
    // Test getting pool names
    auto pool_names = m_resource_manager->get_pool_names();
    QVERIFY(pool_names.empty()); // No pools created yet
}

void TestResourceManagement::testResourceRelease() {
    // Test resource release interface
    QVERIFY(m_resource_manager != nullptr);
    
    // Test memory usage tracking
    auto memory_usage = m_resource_manager->get_total_memory_usage();
    QCOMPARE(memory_usage, size_t(0)); // No resources allocated yet
}

void TestResourceManagement::testResourceQuotas() {
    // Test quota setting and retrieval
    ResourceQuota quota;
    quota.max_instances = 5;
    quota.max_memory_bytes = 1024 * 1024; // 1MB
    quota.max_lifetime = std::chrono::hours(1);
    
    auto set_result = m_resource_manager->set_plugin_quota("test_plugin", ResourceType::Memory, quota);
    QVERIFY(set_result.has_value());
    
    auto get_result = m_resource_manager->get_plugin_quota("test_plugin", ResourceType::Memory);
    QVERIFY(get_result.has_value());
    
    auto retrieved_quota = get_result.value();
    QCOMPARE(retrieved_quota.max_instances, quota.max_instances);
    QCOMPARE(retrieved_quota.max_memory_bytes, quota.max_memory_bytes);
    QCOMPARE(retrieved_quota.max_lifetime, quota.max_lifetime);
}

void TestResourceManagement::testResourceStatistics() {
    auto stats = m_resource_manager->get_statistics();
    
    // Verify expected fields
    QVERIFY(stats.contains("pools_count"));
    QVERIFY(stats.contains("factories_count"));
    QVERIFY(stats.contains("tracking_enabled"));
    
    // Test tracking enable/disable
    m_resource_manager->set_tracking_enabled(false);
    QVERIFY(!m_resource_manager->is_tracking_enabled());
    
    m_resource_manager->set_tracking_enabled(true);
    QVERIFY(m_resource_manager->is_tracking_enabled());
}

void TestResourceManagement::testLifecycleTracking() {
    QVERIFY(m_lifecycle_manager != nullptr);
    
    // Create a test resource handle
    ResourceHandle handle("test_resource", ResourceType::Thread, "test_plugin");
    
    // Test resource registration
    auto register_result = m_lifecycle_manager->register_resource(handle, LifecycleState::Created);
    QVERIFY(register_result.has_value());
    
    // Test state retrieval
    auto state_result = m_lifecycle_manager->get_state("test_resource");
    QVERIFY(state_result.has_value());
    QCOMPARE(state_result.value(), LifecycleState::Created);
    
    // Test duplicate registration
    auto duplicate_result = m_lifecycle_manager->register_resource(handle, LifecycleState::Created);
    QVERIFY(!duplicate_result.has_value());
    QCOMPARE(duplicate_result.error().code, PluginErrorCode::AlreadyExists);
    
    // Test unregistration
    auto unregister_result = m_lifecycle_manager->unregister_resource("test_resource");
    QVERIFY(unregister_result.has_value());
}

void TestResourceManagement::testStateTransitions() {
    ResourceHandle handle("test_resource", ResourceType::Thread, "test_plugin");
    
    // Register resource
    auto register_result = m_lifecycle_manager->register_resource(handle, LifecycleState::Created);
    QVERIFY(register_result.has_value());
    
    // Test valid state transition
    auto update_result = m_lifecycle_manager->update_state("test_resource", LifecycleState::Initialized);
    QVERIFY(update_result.has_value());
    
    // Verify state change
    auto state_result = m_lifecycle_manager->get_state("test_resource");
    QVERIFY(state_result.has_value());
    QCOMPARE(state_result.value(), LifecycleState::Initialized);
    
    // Test invalid state transition (from Destroyed)
    m_lifecycle_manager->update_state("test_resource", LifecycleState::Destroyed);
    auto invalid_result = m_lifecycle_manager->update_state("test_resource", LifecycleState::Active);
    QVERIFY(!invalid_result.has_value());
    QCOMPARE(invalid_result.error().code, PluginErrorCode::InvalidArgument);
}

void TestResourceManagement::testDependencyManagement() {
    // Create test resources
    ResourceHandle handle1("resource1", ResourceType::Thread, "test_plugin");
    ResourceHandle handle2("resource2", ResourceType::Timer, "test_plugin");
    
    m_lifecycle_manager->register_resource(handle1);
    m_lifecycle_manager->register_resource(handle2);
    
    // Test dependency addition
    ResourceDependency dependency("resource2", "resource1", "parent", true);
    auto add_result = m_lifecycle_manager->add_dependency(dependency);
    QVERIFY(add_result.has_value());
    
    // Test dependency retrieval
    auto deps_result = m_lifecycle_manager->get_dependencies("resource2");
    QVERIFY(deps_result.has_value());
    QCOMPARE(deps_result.value().size(), size_t(1));
    QCOMPARE(deps_result.value()[0].dependency_id, "resource1");
    
    // Test dependents retrieval
    auto dependents_result = m_lifecycle_manager->get_dependents("resource1");
    QVERIFY(dependents_result.has_value());
    QCOMPARE(dependents_result.value().size(), size_t(1));
    
    // Test dependency removal
    auto remove_result = m_lifecycle_manager->remove_dependency("resource2", "resource1");
    QVERIFY(remove_result.has_value());
    
    // Verify dependency removed
    auto empty_deps = m_lifecycle_manager->get_dependencies("resource2");
    QVERIFY(empty_deps.has_value());
    QVERIFY(empty_deps.value().empty());
}

void TestResourceManagement::testAutomaticCleanup() {
    // Test cleanup policy
    CleanupPolicy policy;
    policy.max_idle_time = std::chrono::seconds(1);
    policy.max_lifetime = std::chrono::seconds(5);
    
    m_lifecycle_manager->set_cleanup_policy(policy);
    
    auto retrieved_policy = m_lifecycle_manager->get_cleanup_policy();
    QCOMPARE(retrieved_policy.max_idle_time, policy.max_idle_time);
    QCOMPARE(retrieved_policy.max_lifetime, policy.max_lifetime);
    
    // Test automatic cleanup enable/disable
    m_lifecycle_manager->set_automatic_cleanup_enabled(false);
    QVERIFY(!m_lifecycle_manager->is_automatic_cleanup_enabled());
    
    m_lifecycle_manager->set_automatic_cleanup_enabled(true);
    QVERIFY(m_lifecycle_manager->is_automatic_cleanup_enabled());
}

void TestResourceManagement::testLifecycleEvents() {
    QSignalSpy spy(dynamic_cast<QObject*>(m_lifecycle_manager.get()), 
                   SIGNAL(resource_state_changed(QString, int, int)));
    
    // Register resource to trigger event
    ResourceHandle handle("test_resource", ResourceType::Thread, "test_plugin");
    m_lifecycle_manager->register_resource(handle, LifecycleState::Created);
    
    // Update state to trigger event
    m_lifecycle_manager->update_state("test_resource", LifecycleState::Initialized);
    
    // Verify signal was emitted
    QCOMPARE(spy.count(), 1);
    
    auto arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), "test_resource");
    QCOMPARE(arguments.at(1).toInt(), static_cast<int>(LifecycleState::Created));
    QCOMPARE(arguments.at(2).toInt(), static_cast<int>(LifecycleState::Initialized));
}

void TestResourceManagement::testMonitoringStart() {
    QVERIFY(m_resource_monitor != nullptr);
    
    // Test monitoring enable/disable
    m_resource_monitor->set_monitoring_enabled(false);
    QVERIFY(!m_resource_monitor->is_monitoring_enabled());
    
    m_resource_monitor->set_monitoring_enabled(true);
    QVERIFY(m_resource_monitor->is_monitoring_enabled());
    
    // Test resource monitoring start
    ResourceHandle handle("test_resource", ResourceType::Thread, "test_plugin");
    auto start_result = m_resource_monitor->start_monitoring(handle);
    QVERIFY(start_result.has_value());
    
    // Test duplicate monitoring start
    auto duplicate_result = m_resource_monitor->start_monitoring(handle);
    QVERIFY(!duplicate_result.has_value());
    QCOMPARE(duplicate_result.error().code, PluginErrorCode::AlreadyExists);
    
    // Test monitoring stop
    auto stop_result = m_resource_monitor->stop_monitoring("test_resource");
    QVERIFY(stop_result.has_value());
}

void TestResourceManagement::testMetricsCollection() {
    ResourceHandle handle("test_resource", ResourceType::Thread, "test_plugin");
    m_resource_monitor->start_monitoring(handle);
    
    // Test access recording
    auto access_result = m_resource_monitor->record_access("test_resource", std::chrono::milliseconds(100));
    QVERIFY(access_result.has_value());
    
    // Test error recording
    auto error_result = m_resource_monitor->record_error("test_resource", "Test error message");
    QVERIFY(error_result.has_value());
    
    // Test metrics retrieval
    auto metrics_result = m_resource_monitor->get_metrics("test_resource");
    QVERIFY(metrics_result.has_value());
    
    auto metrics = metrics_result.value();
    QCOMPARE(metrics.resource_id, "test_resource");
    QCOMPARE(metrics.plugin_id, "test_plugin");
    QCOMPARE(metrics.access_count, size_t(1));
    QCOMPARE(metrics.error_count, size_t(1));
    QCOMPARE(metrics.last_error_message, "Test error message");
}

void TestResourceManagement::testPerformanceAlerts() {
    // Test alert configuration
    MonitoringConfiguration config;
    config.cpu_usage_alert_threshold = 50.0;
    config.memory_usage_alert_threshold = 1024;
    
    auto config_result = m_resource_monitor->set_configuration(config);
    QVERIFY(config_result.has_value());
    
    auto retrieved_config = m_resource_monitor->get_configuration();
    QCOMPARE(retrieved_config.cpu_usage_alert_threshold, 50.0);
    QCOMPARE(retrieved_config.memory_usage_alert_threshold, size_t(1024));
    
    // Test alert retrieval
    auto alerts = m_resource_monitor->get_performance_alerts(0.0);
    QVERIFY(alerts.empty()); // No alerts initially
}

void TestResourceManagement::testQuotaViolations() {
    // Test custom quota setting
    auto quota_result = m_resource_monitor->set_custom_quota("test_plugin", ResourceType::Memory, "max_memory", 1024.0);
    QVERIFY(quota_result.has_value());
    
    // Test quota compliance checking
    auto violations = m_resource_monitor->check_quota_compliance("test_plugin", ResourceType::Memory);
    QVERIFY(violations.empty()); // No violations initially
    
    // Test quota violations retrieval
    auto all_violations = m_resource_monitor->get_quota_violations();
    QVERIFY(all_violations.empty()); // No violations initially
}

void TestResourceManagement::testEfficiencyReporting() {
    // Test efficiency report generation
    auto report = m_resource_monitor->get_efficiency_report();
    
    QVERIFY(report.contains("resources"));
    QVERIFY(report.contains("average_efficiency"));
    QVERIFY(report.contains("total_resources"));
    
    // Test top consumers
    auto consumers = m_resource_monitor->get_top_consumers("cpu", 5);
    QVERIFY(consumers.empty()); // No resources being monitored yet
}

void TestResourceManagement::testDataExport() {
    auto now = std::chrono::system_clock::now();
    auto hour_ago = now - std::chrono::hours(1);
    
    // Test JSON export
    auto json_result = m_resource_monitor->export_metrics("json", hour_ago, now);
    QVERIFY(json_result.has_value());
    QVERIFY(!json_result.value().empty());
    
    // Test CSV export
    auto csv_result = m_resource_monitor->export_metrics("csv", hour_ago, now);
    QVERIFY(csv_result.has_value());
    QVERIFY(!csv_result.value().empty());
    
    // Test invalid format
    auto invalid_result = m_resource_monitor->export_metrics("invalid", hour_ago, now);
    QVERIFY(!invalid_result.has_value());
    QCOMPARE(invalid_result.error().code, PluginErrorCode::InvalidArgument);
}

void TestResourceManagement::testThreadPool() {
    ThreadPool pool;
    
    QCOMPARE(pool.name(), "thread_pool");
    
    auto stats = pool.get_statistics();
    QCOMPARE(stats.currently_active, size_t(0));
    QCOMPARE(stats.total_created, size_t(0));
}

void TestResourceManagement::testTimerPool() {
    TimerPool pool;
    
    QCOMPARE(pool.name(), "timer_pool");
    
    auto stats = pool.get_statistics();
    QCOMPARE(stats.currently_active, size_t(0));
    QCOMPARE(stats.total_created, size_t(0));
}

void TestResourceManagement::testMemoryPool() {
    MemoryPool pool;
    
    QCOMPARE(pool.name(), "memory_pool");
    QCOMPARE(pool.get_current_memory_usage(), size_t(0));
    
    auto stats = pool.get_statistics();
    QCOMPARE(stats.currently_active, size_t(0));
    QCOMPARE(stats.total_created, size_t(0));
}

void TestResourceManagement::testNetworkConnectionPool() {
    NetworkConnectionPool pool;
    
    QCOMPARE(pool.name(), "network_pool");
    
    auto stats = pool.get_statistics();
    QCOMPARE(stats.currently_active, size_t(0));
    QCOMPARE(stats.total_created, size_t(0));
}

void TestResourceManagement::testResourceManagerIntegration() {
    // Test integration between all resource management components
    QVERIFY(m_resource_manager != nullptr);
    QVERIFY(m_lifecycle_manager != nullptr);
    QVERIFY(m_resource_monitor != nullptr);
    
    // Test statistics from all components
    auto manager_stats = m_resource_manager->get_statistics();
    auto lifecycle_stats = m_lifecycle_manager->get_lifecycle_statistics();
    auto monitor_stats = m_resource_monitor->get_monitoring_statistics();
    
    QVERIFY(!manager_stats.isEmpty());
    QVERIFY(!lifecycle_stats.isEmpty());
    QVERIFY(!monitor_stats.isEmpty());
}

void TestResourceManagement::testCompleteResourceLifecycle() {
    // Test complete lifecycle from creation to destruction
    ResourceHandle handle("complete_test", ResourceType::Thread, "test_plugin");
    
    // 1. Start monitoring
    auto monitor_result = m_resource_monitor->start_monitoring(handle);
    QVERIFY(monitor_result.has_value());
    
    // 2. Register with lifecycle manager
    auto register_result = m_lifecycle_manager->register_resource(handle, LifecycleState::Created);
    QVERIFY(register_result.has_value());
    
    // 3. Transition through states
    m_lifecycle_manager->update_state("complete_test", LifecycleState::Initialized);
    m_lifecycle_manager->update_state("complete_test", LifecycleState::Active);
    
    // 4. Record some activity
    m_resource_monitor->record_access("complete_test", std::chrono::milliseconds(50));
    
    // 5. Transition to cleanup
    m_lifecycle_manager->update_state("complete_test", LifecycleState::Cleanup);
    
    // 6. Stop monitoring
    m_resource_monitor->stop_monitoring("complete_test");
    
    // 7. Unregister from lifecycle
    m_lifecycle_manager->unregister_resource("complete_test");
    
    // Verify final state
    auto state_result = m_lifecycle_manager->get_state("complete_test");
    QVERIFY(!state_result.has_value()); // Should be unregistered
}

void TestResourceManagement::testResourceCleanupOnPluginUnload() {
    // Create multiple resources for a plugin
    for (int i = 0; i < 5; ++i) {
        ResourceHandle handle("plugin_resource_" + std::to_string(i), ResourceType::Thread, "test_plugin");
        m_lifecycle_manager->register_resource(handle, LifecycleState::Active);
        m_resource_monitor->start_monitoring(handle);
    }
    
    // Verify resources are tracked
    auto resources_in_active = m_lifecycle_manager->get_resources_in_state(LifecycleState::Active);
    QCOMPARE(resources_in_active.size(), size_t(5));
    
    // Cleanup all resources for the plugin
    auto cleaned_count = m_lifecycle_manager->cleanup_plugin_resources("test_plugin");
    QCOMPARE(cleaned_count, size_t(5));
    
    // Verify resources are cleaned up
    auto remaining_resources = m_lifecycle_manager->get_resources_in_state(LifecycleState::Active);
    QVERIFY(remaining_resources.empty());
}

QTEST_MAIN(TestResourceManagement)
#include "test_resource_management.moc"
