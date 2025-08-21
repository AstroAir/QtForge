/**
 * @file test_component_performance.cpp
 * @brief Performance tests for QtPlugin v3.0.0 component architecture
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <QElapsedTimer>
#include <QDebug>
#include <QTemporaryDir>
#include <QThread>
#include <QConcurrentRun>
#include <memory>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#endif

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

// Manager headers for comparison
#include "qtplugin/core/plugin_manager.hpp"
#include "qtplugin/security/security_manager.hpp"
#include "qtplugin/managers/configuration_manager.hpp"
#include "qtplugin/managers/resource_manager.hpp"

using namespace qtplugin;

class ComponentPerformanceTests : public QObject
{
    Q_OBJECT

public:
    ComponentPerformanceTests() = default;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Component instantiation performance
    void testComponentInstantiationPerformance();
    void testManagerInstantiationPerformance();
    void testComponentVsManagerInstantiation();

    // Component operation performance
    void testPluginRegistryPerformance();
    void testDependencyResolverPerformance();
    void testSecurityValidatorPerformance();
    void testResourcePoolPerformance();
    void testConfigurationStoragePerformance();

    // Memory usage tests
    void testComponentMemoryFootprint();
    void testManagerMemoryFootprint();
    void testMemoryUsageComparison();

    // Concurrent operation tests
    void testConcurrentComponentOperations();
    void testComponentThreadSafety();

    // Integration performance tests
    void testManagerComponentDelegationOverhead();
    void testComponentCompositionPerformance();

private:
    // Performance measurement helpers
    void measureExecutionTime(const QString& testName, std::function<void()> testFunction);
    void logPerformanceResult(const QString& testName, qint64 elapsedMs, const QString& details = QString());
    size_t getCurrentMemoryUsage() const;
    void createTestPlugins(int count);

private:
    std::unique_ptr<QTemporaryDir> m_temp_dir;
    QString m_test_dir;
    std::vector<PluginInfo> m_test_plugins;
};

void ComponentPerformanceTests::initTestCase()
{
    qDebug() << "Starting component performance tests";
    m_temp_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_temp_dir->isValid());
    m_test_dir = m_temp_dir->path();
    
    // Create test plugins for performance testing
    createTestPlugins(100);
}

void ComponentPerformanceTests::cleanupTestCase()
{
    qDebug() << "Component performance tests completed";
}

void ComponentPerformanceTests::init()
{
    // Setup for each test
}

void ComponentPerformanceTests::cleanup()
{
    // Cleanup after each test
}

void ComponentPerformanceTests::testComponentInstantiationPerformance()
{
    const int iterations = 1000;
    
    measureExecutionTime("Component Instantiation", [iterations]() {
        for (int i = 0; i < iterations; ++i) {
            // Test core components
            auto registry = std::make_unique<PluginRegistry>();
            auto resolver = std::make_unique<PluginDependencyResolver>();
            
            // Test security components
            auto validator = std::make_unique<SecurityValidator>();
            auto verifier = std::make_unique<SignatureVerifier>();
            
            // Test configuration components
            auto storage = std::make_unique<ConfigurationStorage>();
            auto config_validator = std::make_unique<ConfigurationValidator>();
            
            // Test resource components
            auto allocator = std::make_unique<ResourceAllocator>();
            auto monitor = std::make_unique<ResourceMonitor>();
            
            // Components should be created quickly
            Q_UNUSED(registry);
            Q_UNUSED(resolver);
            Q_UNUSED(validator);
            Q_UNUSED(verifier);
            Q_UNUSED(storage);
            Q_UNUSED(config_validator);
            Q_UNUSED(allocator);
            Q_UNUSED(monitor);
        }
    });
}

void ComponentPerformanceTests::testManagerInstantiationPerformance()
{
    const int iterations = 1000;
    
    measureExecutionTime("Manager Instantiation", [iterations]() {
        for (int i = 0; i < iterations; ++i) {
            auto plugin_manager = std::make_unique<PluginManager>();
            auto security_manager = std::make_unique<SecurityManager>();
            auto config_manager = std::make_unique<ConfigurationManager>();
            auto resource_manager = std::make_unique<ResourceManager>();
            
            Q_UNUSED(plugin_manager);
            Q_UNUSED(security_manager);
            Q_UNUSED(config_manager);
            Q_UNUSED(resource_manager);
        }
    });
}

void ComponentPerformanceTests::testPluginRegistryPerformance()
{
    auto registry = std::make_unique<PluginRegistry>();
    
    measureExecutionTime("Plugin Registry Operations", [&registry, this]() {
        // Test plugin registration performance
        for (const auto& plugin_info : m_test_plugins) {
            auto result = registry->register_plugin(plugin_info);
            Q_UNUSED(result);
        }
        
        // Test plugin lookup performance
        for (const auto& plugin_info : m_test_plugins) {
            auto result = registry->find_plugin(plugin_info.id);
            Q_UNUSED(result);
        }
        
        // Test plugin listing performance
        auto all_plugins = registry->get_all_plugins();
        Q_UNUSED(all_plugins);
    });
}

void ComponentPerformanceTests::testResourcePoolPerformance()
{
    auto pool = std::make_unique<ResourcePool<std::vector<char>>>(
        "performance_test_pool", 
        ResourceType::Memory
    );
    
    // Configure pool
    ResourceQuota quota;
    quota.max_instances = 1000;
    quota.max_memory_bytes = 100 * 1024 * 1024; // 100MB
    quota.max_lifetime = std::chrono::hours(1);
    pool->set_quota(quota);
    
    pool->set_factory([]() {
        return std::make_unique<std::vector<char>>(1024); // 1KB buffers
    });
    
    measureExecutionTime("Resource Pool Operations", [&pool]() {
        std::vector<ResourceHandle> handles;
        std::vector<std::unique_ptr<std::vector<char>>> resources;
        
        // Test resource acquisition
        for (int i = 0; i < 100; ++i) {
            auto result = pool->acquire_resource("performance_test");
            if (result) {
                auto [handle, resource] = std::move(result.value());
                handles.push_back(handle);
                resources.push_back(std::move(resource));
            }
        }
        
        // Test resource release
        for (size_t i = 0; i < handles.size(); ++i) {
            pool->release_resource(handles[i], std::move(resources[i]));
        }
    });
}

void ComponentPerformanceTests::testComponentMemoryFootprint()
{
    size_t initial_memory = getCurrentMemoryUsage();
    
    // Create all component types
    auto registry = std::make_unique<PluginRegistry>();
    auto resolver = std::make_unique<PluginDependencyResolver>();
    auto hot_reload = std::make_unique<PluginHotReloadManager>();
    auto metrics = std::make_unique<PluginMetricsCollector>();
    auto validator = std::make_unique<SecurityValidator>();
    auto verifier = std::make_unique<SignatureVerifier>();
    auto permission_mgr = std::make_unique<PermissionManager>();
    auto policy_engine = std::make_unique<SecurityPolicyEngine>();
    auto storage = std::make_unique<ConfigurationStorage>();
    auto config_validator = std::make_unique<ConfigurationValidator>();
    auto merger = std::make_unique<ConfigurationMerger>();
    auto watcher = std::make_unique<ConfigurationWatcher>();
    auto allocator = std::make_unique<ResourceAllocator>();
    auto monitor = std::make_unique<ResourceMonitor>();
    
    size_t after_components = getCurrentMemoryUsage();
    size_t component_memory = after_components - initial_memory;
    
    qDebug() << "Component memory footprint:";
    qDebug() << "  Total components memory:" << component_memory << "bytes";
    qDebug() << "  Average per component:" << (component_memory / 14) << "bytes";
    
    // Each component should use less than 5MB
    QVERIFY2(component_memory < 14 * 5 * 1024 * 1024,
             QString("Components use too much memory: %1 bytes").arg(component_memory).toLocal8Bit());
    
    logPerformanceResult("Component Memory Footprint", component_memory, "bytes total");
}

void ComponentPerformanceTests::testConcurrentComponentOperations()
{
    auto registry = std::make_unique<PluginRegistry>();
    auto allocator = std::make_unique<ResourceAllocator>();
    
    const int thread_count = 4;
    const int operations_per_thread = 250;
    
    measureExecutionTime("Concurrent Component Operations", [&registry, &allocator, thread_count, operations_per_thread]() {
        QList<QFuture<void>> futures;
        
        for (int t = 0; t < thread_count; ++t) {
            auto future = QtConcurrent::run([&registry, &allocator, t, operations_per_thread]() {
                for (int i = 0; i < operations_per_thread; ++i) {
                    // Test concurrent registry operations
                    PluginInfo plugin_info;
                    plugin_info.id = QString("thread%1.plugin%2").arg(t).arg(i).toStdString();
                    plugin_info.state = PluginState::Unloaded;
                    
                    auto reg_result = registry->register_plugin(plugin_info);
                    Q_UNUSED(reg_result);
                    
                    // Test concurrent resource allocation
                    auto alloc_result = allocator->allocate_resource(
                        ResourceType::Memory,
                        plugin_info.id,
                        ResourcePriority::Normal
                    );
                    
                    if (alloc_result) {
                        allocator->deallocate_resource(alloc_result->allocation_id);
                    }
                }
            });
            futures.append(future);
        }
        
        // Wait for all threads to complete
        for (auto& future : futures) {
            future.waitForFinished();
        }
    });
}

void ComponentPerformanceTests::measureExecutionTime(const QString& testName, std::function<void()> testFunction)
{
    QElapsedTimer timer;
    timer.start();
    
    testFunction();
    
    qint64 elapsed = timer.elapsed();
    logPerformanceResult(testName, elapsed);
}

void ComponentPerformanceTests::logPerformanceResult(const QString& testName, qint64 elapsedMs, const QString& details)
{
    QString message = QString("Performance Test '%1': %2ms").arg(testName).arg(elapsedMs);
    if (!details.isEmpty()) {
        message += QString(" (%1)").arg(details);
    }
    qDebug() << message;
}

size_t ComponentPerformanceTests::getCurrentMemoryUsage() const
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
#elif defined(__linux__)
    std::ifstream status_file("/proc/self/status");
    std::string line;
    while (std::getline(status_file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::istringstream iss(line);
            std::string key, value, unit;
            iss >> key >> value >> unit;
            return std::stoull(value) * 1024; // Convert KB to bytes
        }
    }
    return 0;
#elif defined(__APPLE__)
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, 
                  (task_info_t)&info, &infoCount) == KERN_SUCCESS) {
        return info.resident_size;
    }
    return 0;
#else
    return 1024 * 1024; // 1MB fallback
#endif
}

void ComponentPerformanceTests::createTestPlugins(int count)
{
    m_test_plugins.clear();
    m_test_plugins.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        PluginInfo plugin_info;
        plugin_info.id = QString("test.plugin.%1").arg(i).toStdString();
        plugin_info.file_path = QString("%1/test_plugin_%2.so").arg(m_test_dir).arg(i).toStdString();
        plugin_info.state = PluginState::Unloaded;
        plugin_info.metadata.name = QString("Test Plugin %1").arg(i).toStdString();
        plugin_info.metadata.version = Version{1, 0, 0};
        plugin_info.metadata.api_version = Version{3, 0, 0};
        
        m_test_plugins.push_back(plugin_info);
    }
}

#include "test_component_performance.moc"
QTEST_MAIN(ComponentPerformanceTests)
