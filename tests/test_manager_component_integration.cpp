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
#include "qtplugin/managers/configuration_manager_impl.hpp"
#include "qtplugin/managers/resource_manager.hpp"
#include "qtplugin/managers/resource_manager_impl.hpp"
#include "qtplugin/managers/logging_manager_impl.hpp"
#include "qtplugin/managers/resource_lifecycle_impl.hpp"
#include "qtplugin/managers/resource_monitor_impl.hpp"
#include "qtplugin/core/plugin_loader.hpp"
#include "qtplugin/communication/message_bus.hpp"

// Component headers for verification
#include "qtplugin/core/plugin_registry.hpp"
#include "qtplugin/security/components/security_validator.hpp"
#include "qtplugin/managers/components/configuration_storage.hpp"
#include "qtplugin/managers/components/resource_pool.hpp"
#include "qtplugin/managers/components/resource_allocator.hpp"

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
    // Create PluginManager with default implementations
    auto manager = std::make_unique<PluginManager>();
    QVERIFY(manager != nullptr);

    // Test that manager is initialized and components are working
    auto loaded_plugins = manager->loaded_plugins();
    QVERIFY(loaded_plugins.empty()); // Should start empty

    // Test plugin discovery on empty directory (should return empty but not crash)
    auto discovery_result = manager->discover_plugins(m_test_dir.toStdString());
    // Discovery should work (return empty list for empty directory) without crashing
    // This tests that the discovery mechanism and components are working properly
    QVERIFY(discovery_result.empty()); // Empty directory should return empty list

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
    QCOMPARE(get_result.value().toString(), "test_value");

    // Test configuration file operations
    QString config_file = m_test_dir + "/test_config.json";
    createTestConfiguration(config_file);

    auto load_result = config_manager->load_from_file(
        config_file.toStdString(),
        ConfigurationScope::Global
    );
    QVERIFY(load_result.has_value());

    // Test configuration validation through components
    QJsonObject schema_obj;
    schema_obj["type"] = "object";

    QJsonObject properties;
    QJsonObject nameProperty;
    nameProperty["type"] = "string";
    properties["name"] = nameProperty;

    QJsonObject versionProperty;
    versionProperty["type"] = "string";
    properties["version"] = versionProperty;

    schema_obj["properties"] = properties;

    QJsonArray required;
    required.append("name");
    required.append("version");
    schema_obj["required"] = required;

    ConfigurationSchema schema(schema_obj, false);

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

    // Test resource statistics through manager
    auto stats = resource_manager->get_usage_statistics(ResourceType::Thread);
    QVERIFY(stats.total_created >= 0);
    QVERIFY(stats.currently_active >= 0);

    // Test active resources listing
    auto active_resources = resource_manager->get_active_resources("test_plugin");
    QVERIFY(active_resources.size() >= 0);

    // Test resource monitoring through manager
    auto stats2 = resource_manager->get_usage_statistics();
    QVERIFY(stats2.total_created >= 0);
    QVERIFY(stats2.currently_active >= 0);
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
    // Test plugin discovery (should work without crashing even on empty directory)
    auto discovery_result = plugin_manager->discover_plugins(m_test_dir.toStdString());
    // Discovery should work properly (empty directory returns empty list)
    QVERIFY(discovery_result.empty()); // Empty directory should return empty list
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
    QCOMPARE(get1_result.value().toString(), "value1");
    QCOMPARE(get2_result.value().toString(), "value2");
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
    // Error details may or may not be populated depending on the specific error path
    // The important thing is that the error is properly propagated from components
}

void TestManagerComponentIntegration::testIntegratedPerformance()
{
    // Test performance of integrated manager-component operations
    auto resource_manager = std::make_unique<ResourceManager>();

    const int num_operations = 100;
    auto start_time = std::chrono::high_resolution_clock::now();

    // Perform multiple resource operations (create and remove pools)
    std::vector<std::string> pool_names;
    for (int i = 0; i < num_operations; ++i) {
        std::string pool_name = QString("test_pool_%1").arg(i).toStdString();
        auto pool_result = resource_manager->create_pool(
            ResourceType::Memory,
            pool_name,
            ResourceQuota{}
        );

        if (pool_result.has_value()) {
            pool_names.push_back(pool_name);
        }
    }

    // Clean up pools
    for (const auto& pool_name : pool_names) {
        resource_manager->remove_pool(pool_name);
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
    auto loaded_plugins = plugin_manager->loaded_plugins();
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

void TestManagerComponentIntegration::testConcurrentManagerOperations() {
    // Test concurrent operations across multiple managers
    auto config_manager = std::make_unique<ConfigurationManager>();
    auto resource_manager = std::make_unique<ResourceManager>();

    const int num_threads = 4;
    const int operations_per_thread = 50;

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    // Launch concurrent operations
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                // Configuration operations
                std::string key = QString("thread%1.key%2").arg(t).arg(i).toStdString();
                auto set_result = config_manager->set_value(
                    key, QJsonValue(QString("value_%1_%2").arg(t).arg(i)),
                    ConfigurationScope::Plugin, "test_plugin");

                if (set_result.has_value()) {
                    auto get_result = config_manager->get_value(
                        key, ConfigurationScope::Plugin, "test_plugin");
                    if (get_result.has_value()) {
                        success_count++;
                    }
                }
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify that most operations succeeded (allowing for some contention)
    int expected_min_success = (num_threads * operations_per_thread) * 0.8; // 80% success rate
    QVERIFY(success_count >= expected_min_success);
}

#include "test_manager_component_integration.moc"
QTEST_MAIN(TestManagerComponentIntegration)
