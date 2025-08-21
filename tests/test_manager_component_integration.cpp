/**
 * @file test_manager_component_integration.cpp
 * @brief Tests for integration between managers and their components
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <memory>

// Manager headers
#include "qtplugin/core/plugin_manager.hpp"
#include "qtplugin/security/security_manager.hpp"
#include "qtplugin/managers/configuration_manager.hpp"
#include "qtplugin/managers/resource_manager.hpp"

// Component headers for verification
#include "qtplugin/core/plugin_registry.hpp"
#include "qtplugin/security/components/security_validator.hpp"
#include "qtplugin/managers/components/configuration_storage.hpp"
#include "qtplugin/managers/components/resource_pool.hpp"

using namespace qtplugin;

class TestManagerComponentIntegration : public QObject
{
    Q_OBJECT

public:
    TestManagerComponentIntegration() = default;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Manager-Component Integration Tests
    void testPluginManagerWithComponents();
    void testSecurityManagerWithComponents();
    void testConfigurationManagerWithComponents();
    void testResourceManagerWithComponents();
    
    // Cross-Manager Integration Tests
    void testManagerInteraction();
    void testComponentSharing();
    void testErrorPropagation();
    
    // Performance Integration Tests
    void testIntegratedPerformance();
    void testConcurrentManagerOperations();
    
    // Backward Compatibility Tests
    void testBackwardCompatibility();
    void testAPIStability();

private:
    void createTestPlugin(const QString& filename, const QString& plugin_id = "com.test.plugin");
    void createTestConfiguration(const QString& filename);

private:
    std::unique_ptr<QTemporaryDir> m_temp_dir;
    QString m_test_dir;
};

void TestManagerComponentIntegration::initTestCase()
{
    qDebug() << "Starting manager-component integration tests";
    m_temp_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_temp_dir->isValid());
    m_test_dir = m_temp_dir->path();
}

void TestManagerComponentIntegration::cleanupTestCase()
{
    qDebug() << "Manager-component integration tests completed";
}

void TestManagerComponentIntegration::init()
{
    // Setup for each test
}

void TestManagerComponentIntegration::cleanup()
{
    // Cleanup after each test
}

void TestManagerComponentIntegration::testPluginManagerWithComponents()
{
    // Test that PluginManager properly uses its components
    auto manager = std::make_unique<PluginManager>();
    QVERIFY(manager != nullptr);
    
    // Test that manager is initialized and components are working
    auto loaded_plugins = manager->get_loaded_plugins();
    QVERIFY(loaded_plugins.empty()); // Should start empty
    
    // Test plugin discovery (should work through components)
    auto discovery_result = manager->discover_plugins(m_test_dir.toStdString());
    QVERIFY(discovery_result.has_value());
    
    // Test that manager handles component errors gracefully
    auto load_result = manager->load_plugin("nonexistent_plugin.so");
    QVERIFY(!load_result.has_value());
    QVERIFY(load_result.error().code != PluginErrorCode::Success);
}

void TestManagerComponentIntegration::testSecurityManagerWithComponents()
{
    // Test that SecurityManager properly uses its security components
    auto security_manager = std::make_unique<SecurityManager>();
    QVERIFY(security_manager != nullptr);
    
    // Create a test file for validation
    QString test_file = m_test_dir + "/security_test.so";
    QFile file(test_file);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("dummy plugin content for security testing");
    file.close();
    
    // Test validation through manager (should use components internally)
    auto validation_result = security_manager->validate_plugin(
        test_file.toStdString(), 
        SecurityLevel::Basic
    );
    
    // Should get a result (pass or fail, but not crash)
    QVERIFY(validation_result.validated_level != SecurityLevel::Maximum || 
            !validation_result.errors.empty());
    
    // Test security level management
    security_manager->set_security_level(SecurityLevel::Standard);
    QCOMPARE(security_manager->security_level(), SecurityLevel::Standard);
    
    // Test trusted plugin management
    security_manager->add_trusted_plugin("test.plugin", SecurityLevel::Basic);
    QVERIFY(security_manager->is_trusted("test.plugin"));
    
    security_manager->remove_trusted_plugin("test.plugin");
    QVERIFY(!security_manager->is_trusted("test.plugin"));
}

void TestManagerComponentIntegration::testConfigurationManagerWithComponents()
{
    // Test that ConfigurationManager properly uses its configuration components
    auto config_manager = std::make_unique<ConfigurationManager>();
    QVERIFY(config_manager != nullptr);
    
    // Test configuration storage through manager
    auto set_result = config_manager->set_value(
        "test.key", 
        QJsonValue("test_value"), 
        ConfigurationScope::Global
    );
    QVERIFY(set_result.has_value());
    
    // Test configuration retrieval
    auto get_result = config_manager->get_value("test.key", ConfigurationScope::Global);
    QVERIFY(get_result.has_value());
    QCOMPARE(get_result->toString(), "test_value");
    
    // Test configuration file operations
    QString config_file = m_test_dir + "/test_config.json";
    createTestConfiguration(config_file);
    
    auto load_result = config_manager->load_from_file(
        config_file.toStdString(), 
        ConfigurationScope::Global
    );
    QVERIFY(load_result.has_value());
    
    // Test configuration validation through components
    ConfigurationSchema schema;
    schema.required_keys = {"name", "version"};
    schema.key_types["name"] = QJsonValue::String;
    schema.key_types["version"] = QJsonValue::String;
    
    auto schema_result = config_manager->set_schema(schema, ConfigurationScope::Global);
    QVERIFY(schema_result.has_value());
}

void TestManagerComponentIntegration::testResourceManagerWithComponents()
{
    // Test that ResourceManager properly uses its resource components
    auto resource_manager = std::make_unique<ResourceManager>();
    QVERIFY(resource_manager != nullptr);
    
    // Test resource pool creation through manager
    ResourceQuota quota;
    quota.max_instances = 5;
    quota.max_memory_bytes = 1024;
    quota.max_lifetime = std::chrono::minutes(10);
    
    auto pool_result = resource_manager->create_pool(
        ResourceType::Thread, 
        "test_integration_pool", 
        quota
    );
    QVERIFY(pool_result.has_value());
    
    // Test resource allocation through manager
    auto alloc_result = resource_manager->allocate_resource(
        ResourceType::Thread,
        "test_plugin",
        ResourcePriority::Normal
    );
    QVERIFY(alloc_result.has_value());
    
    auto allocation = alloc_result.value();
    QVERIFY(!allocation.allocation_id.empty());
    
    // Test resource deallocation
    auto dealloc_result = resource_manager->deallocate_resource(allocation.allocation_id);
    QVERIFY(dealloc_result.has_value());
    
    // Test resource monitoring through manager
    auto stats = resource_manager->get_statistics();
    QVERIFY(!stats.isEmpty());
    QVERIFY(stats.contains("total_pools"));
}

void TestManagerComponentIntegration::testManagerInteraction()
{
    // Test interaction between multiple managers using components
    auto plugin_manager = std::make_unique<PluginManager>();
    auto security_manager = std::make_unique<SecurityManager>();
    auto config_manager = std::make_unique<ConfigurationManager>();
    auto resource_manager = std::make_unique<ResourceManager>();
    
    QVERIFY(plugin_manager != nullptr);
    QVERIFY(security_manager != nullptr);
    QVERIFY(config_manager != nullptr);
    QVERIFY(resource_manager != nullptr);
    
    // Test that managers can work together
    // 1. Configure security level
    security_manager->set_security_level(SecurityLevel::Standard);
    
    // 2. Set up configuration
    auto config_result = config_manager->set_value(
        "plugin.security_level", 
        QJsonValue(static_cast<int>(SecurityLevel::Standard)),
        ConfigurationScope::Global
    );
    QVERIFY(config_result.has_value());
    
    // 3. Create resource pool
    ResourceQuota quota;
    quota.max_instances = 10;
    auto pool_result = resource_manager->create_pool(
        ResourceType::Memory, 
        "plugin_pool", 
        quota
    );
    QVERIFY(pool_result.has_value());
    
    // 4. Test plugin manager with configured environment
    auto discovery_result = plugin_manager->discover_plugins(m_test_dir.toStdString());
    QVERIFY(discovery_result.has_value());
}

void TestManagerComponentIntegration::testComponentSharing()
{
    // Test that components can be shared or work independently
    auto config_manager1 = std::make_unique<ConfigurationManager>();
    auto config_manager2 = std::make_unique<ConfigurationManager>();
    
    // Test that managers maintain separate state
    auto set1_result = config_manager1->set_value(
        "manager1.key", 
        QJsonValue("value1"), 
        ConfigurationScope::Global
    );
    QVERIFY(set1_result.has_value());
    
    auto set2_result = config_manager2->set_value(
        "manager2.key", 
        QJsonValue("value2"), 
        ConfigurationScope::Global
    );
    QVERIFY(set2_result.has_value());
    
    // Both managers should be able to access their own data
    auto get1_result = config_manager1->get_value("manager1.key", ConfigurationScope::Global);
    auto get2_result = config_manager2->get_value("manager2.key", ConfigurationScope::Global);
    
    QVERIFY(get1_result.has_value());
    QVERIFY(get2_result.has_value());
    QCOMPARE(get1_result->toString(), "value1");
    QCOMPARE(get2_result->toString(), "value2");
}

void TestManagerComponentIntegration::testErrorPropagation()
{
    // Test that errors from components are properly propagated through managers
    auto plugin_manager = std::make_unique<PluginManager>();
    
    // Test loading a non-existent plugin (should propagate error from components)
    auto load_result = plugin_manager->load_plugin("definitely_does_not_exist.so");
    QVERIFY(!load_result.has_value());
    
    auto error = load_result.error();
    QVERIFY(error.code != PluginErrorCode::Success);
    QVERIFY(!error.message.empty());
    QVERIFY(!error.details.empty());
}

void TestManagerComponentIntegration::testIntegratedPerformance()
{
    // Test performance of integrated manager-component operations
    auto resource_manager = std::make_unique<ResourceManager>();
    
    const int num_operations = 100;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Perform multiple resource operations
    std::vector<ResourceAllocation> allocations;
    for (int i = 0; i < num_operations; ++i) {
        auto alloc_result = resource_manager->allocate_resource(
            ResourceType::Memory,
            QString("test_plugin_%1").arg(i).toStdString(),
            ResourcePriority::Normal
        );
        
        if (alloc_result.has_value()) {
            allocations.push_back(alloc_result.value());
        }
    }
    
    // Clean up allocations
    for (const auto& allocation : allocations) {
        resource_manager->deallocate_resource(allocation.allocation_id);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Performance should be reasonable (less than 1 second for 100 operations)
    QVERIFY(duration.count() < 1000);
    qDebug() << "Integrated performance test completed in" << duration.count() << "ms";
}

void TestManagerComponentIntegration::testBackwardCompatibility()
{
    // Test that the new component architecture maintains backward compatibility
    auto plugin_manager = std::make_unique<PluginManager>();
    
    // Test that all old API methods still work
    auto loaded_plugins = plugin_manager->get_loaded_plugins();
    QVERIFY(loaded_plugins.empty());
    
    auto all_info = plugin_manager->all_plugin_info();
    QVERIFY(all_info.empty());
    
    // Test that error handling is consistent
    auto load_result = plugin_manager->load_plugin("nonexistent.so");
    QVERIFY(!load_result.has_value());
    
    // Test that the manager can still be used in the old way
    QVERIFY(plugin_manager != nullptr);
}

void TestManagerComponentIntegration::testAPIStability()
{
    // Test that public APIs remain stable
    auto security_manager = std::make_unique<SecurityManager>();
    
    // Test that all public methods are still available and work
    QCOMPARE(security_manager->security_level(), SecurityLevel::Basic);
    
    security_manager->set_security_level(SecurityLevel::Standard);
    QCOMPARE(security_manager->security_level(), SecurityLevel::Standard);
    
    auto stats = security_manager->security_statistics();
    QVERIFY(!stats.isEmpty());
}

void TestManagerComponentIntegration::createTestPlugin(const QString& filename, const QString& plugin_id)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write("dummy plugin content");
        file.close();
    }
    
    // Create metadata file
    QJsonObject metadata;
    metadata["id"] = plugin_id;
    metadata["name"] = "Test Plugin";
    metadata["version"] = "1.0.0";
    metadata["api_version"] = "3.0.0";
    
    QJsonDocument doc(metadata);
    QFile metadata_file(filename + ".json");
    if (metadata_file.open(QIODevice::WriteOnly)) {
        metadata_file.write(doc.toJson());
        metadata_file.close();
    }
}

void TestManagerComponentIntegration::createTestConfiguration(const QString& filename)
{
    QJsonObject config;
    config["name"] = "Test Configuration";
    config["version"] = "1.0.0";
    config["settings"] = QJsonObject{
        {"debug", true},
        {"timeout", 30},
        {"max_connections", 100}
    };
    
    QJsonDocument doc(config);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

#include "test_manager_component_integration.moc"
QTEST_MAIN(TestManagerComponentIntegration)
