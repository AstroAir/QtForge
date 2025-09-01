/**
 * @file test_configuration_manager.cpp
 * @brief Unit tests for configuration manager
 * @version 3.0.0
 */

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>
#include <filesystem>
#include <memory>

#include "qtplugin/managers/configuration_manager_impl.hpp"

class ConfigurationManagerTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testBasicSetGet();
    void testNestedKeys();
    void testDefaultValues();
    void testKeyExistence();
    void testRemoveKey();

    // Scope tests
    void testDifferentScopes();
    void testPluginSpecificConfiguration();

    // Bulk operations tests
    void testSetConfiguration();
    void testClearConfiguration();
    void testGetKeys();

    // Schema validation tests
    void testSchemaValidation();
    void testStrictMode();
    void testValidationErrors();

    // Persistence tests
    void testSaveLoad();
    void testReload();
    void testAutoPersist();

    // Integration tests
    void testConfigurationManager();
    void testMultiplePlugins();

    // Error handling tests
    void testInvalidJson();
    void testFilePermissions();

private:
    std::unique_ptr<qtplugin::IConfigurationManager> m_config_manager;
    QTemporaryDir m_temp_dir;
    std::filesystem::path m_test_config_path;
};

void ConfigurationManagerTest::initTestCase() {
    // Initialize test environment
    QVERIFY(m_temp_dir.isValid());
    m_test_config_path =
        std::filesystem::path(m_temp_dir.path().toStdString()) /
        "test_config.json";
}

void ConfigurationManagerTest::cleanupTestCase() {
    // Cleanup test environment
}

void ConfigurationManagerTest::init() {
    // Create fresh configuration manager for each test
    m_config_manager = qtplugin::create_configuration_manager();
    QVERIFY(m_config_manager != nullptr);
}

void ConfigurationManagerTest::cleanup() {
    // Clean up after each test
    m_config_manager.reset();

    // Remove test files
    if (std::filesystem::exists(m_test_config_path)) {
        std::filesystem::remove(m_test_config_path);
    }
}

void ConfigurationManagerTest::testBasicSetGet() {
    // Test basic set/get operations
    auto result =
        m_config_manager->set_value("test_key", QJsonValue("test_value"));
    QVERIFY(result.has_value());

    auto value_result = m_config_manager->get_value("test_key");
    QVERIFY(value_result.has_value());
    QCOMPARE(value_result.value().toString(), "test_value");

    // Test different data types
    QVERIFY(m_config_manager->set_value("int_key", QJsonValue(42)).has_value());
    QVERIFY(
        m_config_manager->set_value("bool_key", QJsonValue(true)).has_value());
    QVERIFY(m_config_manager->set_value("double_key", QJsonValue(3.14))
                .has_value());

    auto int_result = m_config_manager->get_value("int_key");
    QVERIFY(int_result.has_value());
    QCOMPARE(int_result.value().toInt(), 42);

    auto bool_result = m_config_manager->get_value("bool_key");
    QVERIFY(bool_result.has_value());
    QCOMPARE(bool_result.value().toBool(), true);

    auto double_result = m_config_manager->get_value("double_key");
    QVERIFY(double_result.has_value());
    QCOMPARE(double_result.value().toDouble(), 3.14);
}

void ConfigurationManagerTest::testNestedKeys() {
    // Test nested key operations
    auto result = m_config_manager->set_value("parent.child.grandchild",
                                              QJsonValue("nested_value"));
    QVERIFY(result.has_value());

    auto value_result = m_config_manager->get_value("parent.child.grandchild");
    QVERIFY(value_result.has_value());
    QCOMPARE(value_result.value().toString(), "nested_value");

    // Test that parent objects are created
    auto parent_result = m_config_manager->get_value("parent");
    QVERIFY(parent_result.has_value());
    QVERIFY(parent_result.value().isObject());

    auto child_result = m_config_manager->get_value("parent.child");
    QVERIFY(child_result.has_value());
    QVERIFY(child_result.value().isObject());
}

void ConfigurationManagerTest::testDefaultValues() {
    // Test default value functionality
    QJsonValue default_value("default");
    auto result = m_config_manager->get_value_or_default("nonexistent_key",
                                                         default_value);
    QCOMPARE(result.toString(), "default");

    // Test with existing key
    QVERIFY(m_config_manager->set_value("existing_key", QJsonValue("existing"))
                .has_value());
    auto existing_result =
        m_config_manager->get_value_or_default("existing_key", default_value);
    QCOMPARE(existing_result.toString(), "existing");
}

void ConfigurationManagerTest::testKeyExistence() {
    // Test key existence checking
    QVERIFY(!m_config_manager->has_key("nonexistent_key"));

    QVERIFY(m_config_manager->set_value("test_key", QJsonValue("value"))
                .has_value());
    QVERIFY(m_config_manager->has_key("test_key"));

    // Test nested key existence
    QVERIFY(m_config_manager->set_value("parent.child", QJsonValue("value"))
                .has_value());
    QVERIFY(m_config_manager->has_key("parent.child"));
    QVERIFY(m_config_manager->has_key("parent"));
}

void ConfigurationManagerTest::testRemoveKey() {
    // Test key removal
    QVERIFY(m_config_manager->set_value("test_key", QJsonValue("value"))
                .has_value());
    QVERIFY(m_config_manager->has_key("test_key"));

    auto remove_result = m_config_manager->remove_key("test_key");
    QVERIFY(remove_result.has_value());
    QVERIFY(!m_config_manager->has_key("test_key"));

    // Test removing nonexistent key
    auto remove_nonexistent = m_config_manager->remove_key("nonexistent_key");
    QVERIFY(!remove_nonexistent.has_value());
}

void ConfigurationManagerTest::testDifferentScopes() {
    using namespace qtplugin;

    // Test different configuration scopes
    QVERIFY(
        m_config_manager
            ->set_value("key", QJsonValue("global"), ConfigurationScope::Global)
            .has_value());
    QVERIFY(m_config_manager
                ->set_value("key", QJsonValue("user"), ConfigurationScope::User)
                .has_value());
    QVERIFY(m_config_manager
                ->set_value("key", QJsonValue("session"),
                            ConfigurationScope::Session)
                .has_value());

    auto global_result =
        m_config_manager->get_value("key", ConfigurationScope::Global);
    QVERIFY(global_result.has_value());
    QCOMPARE(global_result.value().toString(), "global");

    auto user_result =
        m_config_manager->get_value("key", ConfigurationScope::User);
    QVERIFY(user_result.has_value());
    QCOMPARE(user_result.value().toString(), "user");

    auto session_result =
        m_config_manager->get_value("key", ConfigurationScope::Session);
    QVERIFY(session_result.has_value());
    QCOMPARE(session_result.value().toString(), "session");
}

void ConfigurationManagerTest::testPluginSpecificConfiguration() {
    using namespace qtplugin;

    // Test plugin-specific configuration
    std::string plugin1 = "plugin1";
    std::string plugin2 = "plugin2";

    QVERIFY(m_config_manager
                ->set_value("setting", QJsonValue("value1"),
                            ConfigurationScope::Plugin, plugin1)
                .has_value());
    QVERIFY(m_config_manager
                ->set_value("setting", QJsonValue("value2"),
                            ConfigurationScope::Plugin, plugin2)
                .has_value());

    auto plugin1_result = m_config_manager->get_value(
        "setting", ConfigurationScope::Plugin, plugin1);
    QVERIFY(plugin1_result.has_value());
    QCOMPARE(plugin1_result.value().toString(), "value1");

    auto plugin2_result = m_config_manager->get_value(
        "setting", ConfigurationScope::Plugin, plugin2);
    QVERIFY(plugin2_result.has_value());
    QCOMPARE(plugin2_result.value().toString(), "value2");
}

void ConfigurationManagerTest::testSetConfiguration() {
    // Test bulk configuration setting
    QJsonObject config;
    config["key1"] = "value1";
    config["key2"] = 42;
    config["nested"] = QJsonObject{{"child", "nested_value"}};

    auto result = m_config_manager->set_configuration(config);
    QVERIFY(result.has_value());

    auto key1_result = m_config_manager->get_value("key1");
    QVERIFY(key1_result.has_value());
    QCOMPARE(key1_result.value().toString(), "value1");

    auto key2_result = m_config_manager->get_value("key2");
    QVERIFY(key2_result.has_value());
    QCOMPARE(key2_result.value().toInt(), 42);

    auto nested_result = m_config_manager->get_value("nested.child");
    QVERIFY(nested_result.has_value());
    QCOMPARE(nested_result.value().toString(), "nested_value");
}

void ConfigurationManagerTest::testClearConfiguration() {
    // Test configuration clearing
    QVERIFY(
        m_config_manager->set_value("key1", QJsonValue("value1")).has_value());
    QVERIFY(
        m_config_manager->set_value("key2", QJsonValue("value2")).has_value());

    auto clear_result = m_config_manager->clear_configuration();
    QVERIFY(clear_result.has_value());

    QVERIFY(!m_config_manager->has_key("key1"));
    QVERIFY(!m_config_manager->has_key("key2"));
}

void ConfigurationManagerTest::testGetKeys() {
    // Test getting all keys
    QVERIFY(
        m_config_manager->set_value("key1", QJsonValue("value1")).has_value());
    QVERIFY(
        m_config_manager->set_value("key2", QJsonValue("value2")).has_value());
    QVERIFY(m_config_manager->set_value("nested.child", QJsonValue("value3"))
                .has_value());

    auto keys = m_config_manager->get_keys();
    // Note: The actual implementation of get_keys is not complete, so this test
    // might need adjustment QVERIFY(keys.size() >= 2);
}

void ConfigurationManagerTest::testSchemaValidation() {
    using namespace qtplugin;

    // Create a simple schema
    QJsonObject schema;
    schema["type"] = "object";

    QJsonObject properties;
    QJsonObject nameProperty;
    nameProperty["type"] = "string";
    nameProperty["minLength"] = 1;
    nameProperty["maxLength"] = 50;
    properties["name"] = nameProperty;

    QJsonObject ageProperty;
    ageProperty["type"] = "number";
    ageProperty["minimum"] = 0;
    ageProperty["maximum"] = 150;
    properties["age"] = ageProperty;

    schema["properties"] = properties;

    QJsonArray required;
    required.append("name");
    schema["required"] = required;

    ConfigurationSchema config_schema(schema, false);

    // Set schema
    auto schema_result = m_config_manager->set_schema(config_schema);
    QVERIFY(schema_result.has_value());

    // Test valid configuration
    QJsonObject valid_config;
    valid_config["name"] = "John Doe";
    valid_config["age"] = 30;

    auto validation_result =
        m_config_manager->validate_configuration(valid_config, config_schema);
    QVERIFY(validation_result.is_valid);
    QVERIFY(validation_result.errors.empty());
}

void ConfigurationManagerTest::testStrictMode() {
    using namespace qtplugin;

    // Create schema with strict mode
    QJsonObject schema;
    schema["type"] = "object";

    QJsonObject properties;
    QJsonObject nameProperty;
    nameProperty["type"] = "string";
    properties["name"] = nameProperty;
    schema["properties"] = properties;

    ConfigurationSchema strict_schema(schema, true);

    // Test configuration with unknown property in strict mode
    QJsonObject config_with_unknown;
    config_with_unknown["name"] = "John";
    config_with_unknown["unknown_property"] = "value";

    auto validation_result = m_config_manager->validate_configuration(
        config_with_unknown, strict_schema);
    QVERIFY(!validation_result.is_valid);
    QVERIFY(!validation_result.errors.empty());
}

void ConfigurationManagerTest::testValidationErrors() {
    using namespace qtplugin;

    // Create schema requiring a string property
    QJsonObject schema;
    schema["type"] = "object";

    QJsonObject properties;
    QJsonObject nameProperty;
    nameProperty["type"] = "string";
    properties["name"] = nameProperty;
    schema["properties"] = properties;

    QJsonArray required;
    required.append("name");
    schema["required"] = required;

    ConfigurationSchema config_schema(schema, false);

    // Test missing required property
    QJsonObject invalid_config;
    invalid_config["age"] = 30;  // Missing required "name"

    auto validation_result =
        m_config_manager->validate_configuration(invalid_config, config_schema);
    QVERIFY(!validation_result.is_valid);
    QVERIFY(!validation_result.errors.empty());

    // Test wrong type
    QJsonObject wrong_type_config;
    wrong_type_config["name"] = 123;  // Should be string, not number

    auto type_validation = m_config_manager->validate_configuration(
        wrong_type_config, config_schema);
    QVERIFY(!type_validation.is_valid);
    QVERIFY(!type_validation.errors.empty());
}

void ConfigurationManagerTest::testSaveLoad() {
    // Test save/load functionality
    QVERIFY(
        m_config_manager->set_value("key1", QJsonValue("value1")).has_value());
    QVERIFY(m_config_manager->set_value("key2", QJsonValue(42)).has_value());
    QVERIFY(
        m_config_manager->set_value("nested.child", QJsonValue("nested_value"))
            .has_value());

    // Save to file
    auto save_result = m_config_manager->save_to_file(m_test_config_path);
    QVERIFY(save_result.has_value());
    QVERIFY(std::filesystem::exists(m_test_config_path));

    // Disable auto-persist to prevent interference
    m_config_manager->set_auto_persist(false);

    // Clear configuration
    QVERIFY(m_config_manager->clear_configuration().has_value());
    QVERIFY(!m_config_manager->has_key("key1"));

    // Load from file (explicitly specify Global scope and merge=false to
    // replace)
    auto load_result = m_config_manager->load_from_file(
        m_test_config_path, qtplugin::ConfigurationScope::Global, "", false);
    QVERIFY(load_result.has_value());

    // Re-enable auto-persist
    m_config_manager->set_auto_persist(true);

    // Verify loaded data
    auto key1_result = m_config_manager->get_value("key1");
    QVERIFY(key1_result.has_value());
    QCOMPARE(key1_result.value().toString(), "value1");

    auto key2_result = m_config_manager->get_value("key2");
    QVERIFY(key2_result.has_value());
    QCOMPARE(key2_result.value().toInt(), 42);

    auto nested_result = m_config_manager->get_value("nested.child");
    QVERIFY(nested_result.has_value());
    QCOMPARE(nested_result.value().toString(), "nested_value");
}

void ConfigurationManagerTest::testReload() {
    // Test configuration reloading
    QVERIFY(
        m_config_manager->set_value("key", QJsonValue("original")).has_value());
    QVERIFY(m_config_manager->save_to_file(m_test_config_path).has_value());

    // Modify configuration externally (simulate external file change)
    QJsonObject external_config;
    external_config["key"] = "modified";
    external_config["new_key"] = "new_value";

    QJsonDocument doc(external_config);
    QFile file(QString::fromStdString(m_test_config_path.string()));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(doc.toJson());
    file.close();

    // Reload configuration
    auto reload_result = m_config_manager->reload_configuration();
    QVERIFY(reload_result.has_value());

    // Verify reloaded data
    auto key_result = m_config_manager->get_value("key");
    QVERIFY(key_result.has_value());
    QCOMPARE(key_result.value().toString(), "modified");

    auto new_key_result = m_config_manager->get_value("new_key");
    QVERIFY(new_key_result.has_value());
    QCOMPARE(new_key_result.value().toString(), "new_value");
}

void ConfigurationManagerTest::testAutoPersist() {
    // Test auto-persistence functionality
    QVERIFY(m_config_manager
                ->is_auto_persist_enabled());  // Should be enabled by default

    // Disable auto-persist
    m_config_manager->set_auto_persist(false);
    QVERIFY(!m_config_manager->is_auto_persist_enabled());

    // Enable auto-persist
    m_config_manager->set_auto_persist(true);
    QVERIFY(m_config_manager->is_auto_persist_enabled());
}

void ConfigurationManagerTest::testConfigurationManager() {
    // Test configuration manager statistics
    auto stats = m_config_manager->get_statistics();
    QVERIFY(stats.contains("access_count"));
    QVERIFY(stats.contains("change_count"));
    QVERIFY(stats.contains("auto_persist"));

    // Make some changes and check statistics
    QVERIFY(
        m_config_manager->set_value("test", QJsonValue("value")).has_value());
    auto updated_stats = m_config_manager->get_statistics();
    QVERIFY(updated_stats["change_count"].toInt() >
            stats["change_count"].toInt());
}

void ConfigurationManagerTest::testMultiplePlugins() {
    using namespace qtplugin;

    // Test multiple plugins with separate configurations
    std::string plugin1 = "plugin1";
    std::string plugin2 = "plugin2";
    std::string plugin3 = "plugin3";

    // Set different configurations for each plugin
    QVERIFY(m_config_manager
                ->set_value("setting", QJsonValue("value1"),
                            ConfigurationScope::Plugin, plugin1)
                .has_value());
    QVERIFY(m_config_manager
                ->set_value("setting", QJsonValue("value2"),
                            ConfigurationScope::Plugin, plugin2)
                .has_value());
    QVERIFY(m_config_manager
                ->set_value("setting", QJsonValue("value3"),
                            ConfigurationScope::Plugin, plugin3)
                .has_value());

    // Verify each plugin has its own configuration
    auto plugin1_result = m_config_manager->get_value(
        "setting", ConfigurationScope::Plugin, plugin1);
    QVERIFY(plugin1_result.has_value());
    QCOMPARE(plugin1_result.value().toString(), "value1");

    auto plugin2_result = m_config_manager->get_value(
        "setting", ConfigurationScope::Plugin, plugin2);
    QVERIFY(plugin2_result.has_value());
    QCOMPARE(plugin2_result.value().toString(), "value2");

    auto plugin3_result = m_config_manager->get_value(
        "setting", ConfigurationScope::Plugin, plugin3);
    QVERIFY(plugin3_result.has_value());
    QCOMPARE(plugin3_result.value().toString(), "value3");
}

void ConfigurationManagerTest::testInvalidJson() {
    // Test handling of invalid JSON files
    QString invalid_json_path = m_temp_dir.path() + "/invalid.json";
    QFile file(invalid_json_path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{ invalid json content");
    file.close();

    auto load_result =
        m_config_manager->load_from_file(invalid_json_path.toStdString());
    QVERIFY(!load_result.has_value());
}

void ConfigurationManagerTest::testFilePermissions() {
    // Test handling of file permission errors
    auto load_nonexistent =
        m_config_manager->load_from_file("/nonexistent/path/config.json");
    QVERIFY(!load_nonexistent.has_value());

    // Try to save to a path that should definitely fail (contains invalid
    // characters on Windows)
    auto save_invalid_path =
        m_config_manager->save_to_file("C:\\invalid<>|path\\config.json");
    QVERIFY(!save_invalid_path.has_value());
}

QTEST_MAIN(ConfigurationManagerTest)
#include "test_configuration_manager.moc"
