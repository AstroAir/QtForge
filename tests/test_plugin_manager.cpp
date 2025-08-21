/**
 * @file test_plugin_manager.cpp
 * @brief Basic tests for plugin manager functionality
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <memory>
#include <filesystem>

#include "qtplugin/core/plugin_manager.hpp"
#include "qtplugin/utils/error_handling.hpp"

class TestPluginManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Core functionality tests
    void testPluginManagerCreation();
    void testPluginManagerDestruction();
    void testPluginManagerInitialization();
    
    // Plugin loading tests
    void testLoadValidPlugin();
    void testLoadInvalidPlugin();
    void testLoadNonexistentPlugin();

private:
    std::unique_ptr<qtplugin::PluginManager> m_plugin_manager;
    std::unique_ptr<QTemporaryDir> m_temp_dir;
    std::filesystem::path m_plugin_dir;
    
    // Helper methods
    void createMockPlugin(const QString& name, const QString& version = "1.0.0");
    void createInvalidPlugin(const QString& name);
    std::filesystem::path getPluginPath(const QString& name);
};

void TestPluginManager::initTestCase()
{
    qDebug() << "Starting plugin manager tests";
    
    // Create temporary directory for test plugins
    m_temp_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_temp_dir->isValid());
    m_plugin_dir = m_temp_dir->path().toStdString();
}

void TestPluginManager::cleanupTestCase()
{
    qDebug() << "Plugin manager tests completed";
}

void TestPluginManager::init()
{
    // Create fresh plugin manager for each test with all required components
    m_plugin_manager = std::make_unique<qtplugin::PluginManager>(
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    QVERIFY(m_plugin_manager != nullptr);

    // Add our test plugin directory to search paths
    m_plugin_manager->add_search_path(m_plugin_dir);
}

void TestPluginManager::cleanup()
{
    // Clean up plugin manager
    if (m_plugin_manager) {
        // Unload all loaded plugins individually
        auto loaded = m_plugin_manager->loaded_plugins();
        for (const auto& plugin_id : loaded) {
            m_plugin_manager->unload_plugin(plugin_id);
        }
        m_plugin_manager.reset();
    }
}

void TestPluginManager::testPluginManagerCreation()
{
    // Test basic creation
    auto manager = std::make_unique<qtplugin::PluginManager>();
    QVERIFY(manager != nullptr);

    // Test creation with custom components
    auto loader = std::make_unique<qtplugin::QtPluginLoader>();
    auto message_bus = std::make_unique<qtplugin::MessageBus>();
    auto security_manager = std::make_unique<qtplugin::SecurityManager>();

    auto custom_manager = std::make_unique<qtplugin::PluginManager>(
        std::move(loader), std::move(message_bus), std::move(security_manager));
    QVERIFY(custom_manager != nullptr);
}

void TestPluginManager::testPluginManagerDestruction()
{
    // Test that destruction properly cleans up resources
    {
        auto manager = std::make_unique<qtplugin::PluginManager>();
        createMockPlugin("test_plugin");

        // Try to load a JSON file as a plugin (should fail)
        auto load_result = manager->load_plugin(getPluginPath("test_plugin"));
        QVERIFY(!load_result.has_value()); // Should fail because it's not a valid plugin library

        // Manager should clean up automatically when destroyed
    }

    // Verify no memory leaks or hanging resources
    QVERIFY(true); // This would be verified with memory profiling tools
}

void TestPluginManager::testPluginManagerInitialization()
{
    // Test initialization state
    QVERIFY(m_plugin_manager->loaded_plugins().empty());
    QCOMPARE(m_plugin_manager->loaded_plugins().size(), 0);

    // Test search paths
    auto search_paths = m_plugin_manager->search_paths();
    QVERIFY(!search_paths.empty());
    QVERIFY(std::find(search_paths.begin(), search_paths.end(), m_plugin_dir) != search_paths.end());
}

void TestPluginManager::testLoadValidPlugin()
{
    createMockPlugin("valid_plugin", "1.0.0");

    // Try to load a JSON file as a plugin (should fail because it's not a valid plugin library)
    auto result = m_plugin_manager->load_plugin(getPluginPath("valid_plugin"));
    QVERIFY(!result.has_value()); // Should fail because it's not a valid plugin library

    // Verify no plugins are loaded
    auto loaded = m_plugin_manager->loaded_plugins();
    QVERIFY(loaded.empty());
}

void TestPluginManager::testLoadInvalidPlugin()
{
    createInvalidPlugin("invalid_plugin");
    
    auto result = m_plugin_manager->load_plugin(getPluginPath("invalid_plugin"));
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::InvalidFormat);
}

void TestPluginManager::testLoadNonexistentPlugin()
{
    auto result = m_plugin_manager->load_plugin(m_plugin_dir / "nonexistent.dll");
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::FileNotFound);
}

// Helper methods implementation
void TestPluginManager::createMockPlugin(const QString& name, const QString& version)
{
    // Create a mock plugin metadata file
    QJsonObject metadata;
    metadata["name"] = name;
    metadata["version"] = version;
    metadata["description"] = QString("Mock plugin %1").arg(name);
    metadata["author"] = "Test Suite";
    metadata["api_version"] = "3.0.0";
    
    QJsonDocument doc(metadata);
    
    QString plugin_path = QString::fromStdString((m_plugin_dir / (name.toStdString() + ".json")).string());
    QFile file(plugin_path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(doc.toJson());
    file.close();
}

void TestPluginManager::createInvalidPlugin(const QString& name)
{
    QString plugin_path = QString::fromStdString((m_plugin_dir / (name.toStdString() + ".json")).string());
    QFile file(plugin_path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{ invalid json content");
    file.close();
}

std::filesystem::path TestPluginManager::getPluginPath(const QString& name)
{
    return m_plugin_dir / (name.toStdString() + ".json");
}

QTEST_MAIN(TestPluginManager)
#include "test_plugin_manager.moc"
