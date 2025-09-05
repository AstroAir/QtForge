/**
 * @file test_lua_plugin_loader.cpp
 * @brief Test suite for LuaPluginLoader
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <filesystem>

#include "qtplugin/core/lua_plugin_loader.hpp"

class TestLuaPluginLoader : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testLoaderCreation();
    void testLoaderInfo();
    void testSupportedExtensions();
    void testLuaAvailability();

    // File validation tests
    void testCanLoad();
    void testCanLoadInvalidFiles();
    void testFileValidation();

    // Plugin loading tests
    void testLoadValidPlugin();
    void testLoadInvalidPlugin();
    void testLoadNonexistentPlugin();
    void testLoadMultiplePlugins();

    // Plugin unloading tests
    void testUnloadPlugin();
    void testUnloadNonexistentPlugin();
    void testUnloadMultiplePlugins();

    // Plugin management tests
    void testPluginCount();
    void testPluginIds();
    void testGetLuaBridge();

    // Metadata extraction tests
    void testMetadataExtraction();
    void testMetadataDefaults();
    void testInvalidMetadata();

    // Factory tests
    void testLoaderFactory();
    void testFactoryAvailability();
    void testFactoryRegistration();

    // Composite loader tests
    void testCompositeLoader();
    void testCompositeLoaderSelection();
    void testCompositeLoaderFallback();

    // Error handling tests
    void testErrorConditions();
    void testResourceCleanup();
    void testConcurrentAccess();

private:
    void createTestLuaPlugin(const QString& filename, const QString& content);
    std::filesystem::path getTestPluginPath(const QString& filename);

    QTemporaryDir m_temp_dir;
    std::unique_ptr<qtplugin::LuaPluginLoader> m_loader;
};

void TestLuaPluginLoader::initTestCase()
{
    qDebug() << "Starting LuaPluginLoader test suite";
    QVERIFY(m_temp_dir.isValid());
}

void TestLuaPluginLoader::cleanupTestCase()
{
    qDebug() << "LuaPluginLoader test suite completed";
}

void TestLuaPluginLoader::init()
{
    if (qtplugin::LuaPluginLoader::is_lua_available()) {
        m_loader = std::make_unique<qtplugin::LuaPluginLoader>();
    }
}

void TestLuaPluginLoader::cleanup()
{
    m_loader.reset();
}

void TestLuaPluginLoader::testLoaderCreation()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }

    QVERIFY(m_loader != nullptr);
    QVERIFY(!m_loader->name().empty());
    QVERIFY(!m_loader->description().empty());
    QVERIFY(m_loader->version().major >= 3);
}

void TestLuaPluginLoader::testSupportedExtensions()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }

    auto extensions = m_loader->supported_extensions();
    QVERIFY(!extensions.empty());
    QVERIFY(std::find(extensions.begin(), extensions.end(), ".lua") != extensions.end());
}

void TestLuaPluginLoader::testCanLoad()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }

    // Create test Lua file
    createTestLuaPlugin("test.lua", "-- Test Lua plugin");
    std::filesystem::path lua_path = getTestPluginPath("test.lua");

    QVERIFY(m_loader->can_load(lua_path));

    // Test non-Lua file
    createTestLuaPlugin("test.txt", "Not a Lua file");
    std::filesystem::path txt_path = getTestPluginPath("test.txt");

    QVERIFY(!m_loader->can_load(txt_path));

    // Test nonexistent file
    std::filesystem::path nonexistent = getTestPluginPath("nonexistent.lua");
    QVERIFY(!m_loader->can_load(nonexistent));
}

void TestLuaPluginLoader::testLoadValidPlugin()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }

    QString plugin_content = R"(
--[[
@plugin_name: Test Plugin
@plugin_description: A test Lua plugin
@plugin_version: 1.0.0
@plugin_author: Test Author
]]

plugin = {}

function plugin.initialize()
    return true
end

function plugin.get_name()
    return "Test Plugin"
end

return plugin
)";

    createTestLuaPlugin("valid_plugin.lua", plugin_content);
    std::filesystem::path plugin_path = getTestPluginPath("valid_plugin.lua");

    auto result = m_loader->load(plugin_path);
    QVERIFY(result.has_value());

    auto plugin = result.value();
    QVERIFY(plugin != nullptr);
    QVERIFY(!plugin->id().empty());

    // Verify plugin count increased
    QCOMPARE(m_loader->loaded_plugin_count(), 1u);

    // Verify plugin is in the list
    auto plugin_ids = m_loader->loaded_plugin_ids();
    QCOMPARE(plugin_ids.size(), 1u);
    QCOMPARE(plugin_ids[0], plugin->id());
}

void TestLuaPluginLoader::testLoadInvalidPlugin()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }

    // Test invalid Lua syntax
    createTestLuaPlugin("invalid_syntax.lua", "invalid lua syntax !!!");
    std::filesystem::path invalid_path = getTestPluginPath("invalid_syntax.lua");

    auto result = m_loader->load(invalid_path);
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::LoadFailed);

    // Verify plugin count didn't change
    QCOMPARE(m_loader->loaded_plugin_count(), 0u);
}

void TestLuaPluginLoader::testLoadNonexistentPlugin()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }

    std::filesystem::path nonexistent = getTestPluginPath("nonexistent.lua");

    auto result = m_loader->load(nonexistent);
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::FileNotFound);
}

void TestLuaPluginLoader::testUnloadPlugin()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }

    // First load a plugin
    createTestLuaPlugin("unload_test.lua", "return {}");
    std::filesystem::path plugin_path = getTestPluginPath("unload_test.lua");

    auto load_result = m_loader->load(plugin_path);
    QVERIFY(load_result.has_value());

    auto plugin = load_result.value();
    std::string plugin_id = plugin->id();

    QCOMPARE(m_loader->loaded_plugin_count(), 1u);

    // Now unload it
    auto unload_result = m_loader->unload(plugin_id);
    QVERIFY(unload_result.has_value());

    QCOMPARE(m_loader->loaded_plugin_count(), 0u);

    // Verify plugin is no longer in the list
    auto plugin_ids = m_loader->loaded_plugin_ids();
    QVERIFY(plugin_ids.empty());
}

void TestLuaPluginLoader::testUnloadNonexistentPlugin()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }

    auto result = m_loader->unload("nonexistent_plugin_id");
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::NotFound);
}

void TestLuaPluginLoader::testGetLuaBridge()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }

    // Load a plugin
    createTestLuaPlugin("bridge_test.lua", "return {}");
    std::filesystem::path plugin_path = getTestPluginPath("bridge_test.lua");

    auto load_result = m_loader->load(plugin_path);
    QVERIFY(load_result.has_value());

    auto plugin = load_result.value();
    std::string plugin_id = plugin->id();

    // Get the Lua bridge
    auto bridge = m_loader->get_lua_bridge(plugin_id);
    QVERIFY(bridge != nullptr);
    QCOMPARE(bridge->id(), plugin_id);

    // Test nonexistent plugin
    auto null_bridge = m_loader->get_lua_bridge("nonexistent");
    QVERIFY(null_bridge == nullptr);
}

void TestLuaPluginLoader::testMetadataExtraction()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }

    QString plugin_content = R"(
--[[
@plugin_name: Metadata Test Plugin
@plugin_description: Testing metadata extraction
@plugin_version: 2.1.0
@plugin_author: Metadata Author
]]

return {}
)";

    createTestLuaPlugin("metadata_test.lua", plugin_content);
    std::filesystem::path plugin_path = getTestPluginPath("metadata_test.lua");

    auto result = m_loader->load(plugin_path);
    QVERIFY(result.has_value());

    auto plugin = result.value();
    QVERIFY(plugin != nullptr);

    // Note: Actual metadata verification would require access to the plugin's metadata
    // This would be implementation-specific based on how metadata is exposed
}

void TestLuaPluginLoader::testLoaderFactory()
{
    // Test factory creation
    auto loader = qtplugin::LuaPluginLoaderFactory::create();

    if (qtplugin::LuaPluginLoaderFactory::is_available()) {
        QVERIFY(loader != nullptr);
        QVERIFY(!loader->name().empty());
    } else {
        QVERIFY(loader == nullptr);
    }

    // Test availability check
    bool available = qtplugin::LuaPluginLoaderFactory::is_available();
    QCOMPARE(available, qtplugin::LuaPluginLoader::is_lua_available());
}

void TestLuaPluginLoader::testCompositeLoader()
{
    qtplugin::CompositePluginLoader composite_loader;

    QVERIFY(!composite_loader.name().empty());
    QVERIFY(!composite_loader.description().empty());

    // Test supported extensions
    auto extensions = composite_loader.supported_extensions();
    QVERIFY(!extensions.empty());

    // Should include .lua if Lua is available
    if (qtplugin::LuaPluginLoader::is_lua_available()) {
        QVERIFY(std::find(extensions.begin(), extensions.end(), ".lua") != extensions.end());
        QVERIFY(composite_loader.has_lua_support());
    } else {
        QVERIFY(!composite_loader.has_lua_support());
    }
}

void TestLuaPluginLoader::createTestLuaPlugin(const QString& filename, const QString& content)
{
    QFile file(m_temp_dir.filePath(filename));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << content;
    file.close();
}

std::filesystem::path TestLuaPluginLoader::getTestPluginPath(const QString& filename)
{
    return std::filesystem::path(m_temp_dir.filePath(filename).toStdString());
}

QTEST_MAIN(TestLuaPluginLoader)
#include "test_lua_plugin_loader.moc"
