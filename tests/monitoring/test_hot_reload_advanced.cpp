/**
 * @file test_hot_reload_advanced.cpp
 * @brief Advanced tests for hot reload functionality
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFileSystemWatcher>
#include <QSignalSpy>
#include <QTimer>
#include <QThread>
#include <QElapsedTimer>

#include <qtplugin/monitoring/plugin_hot_reload_manager.hpp>
#include <qtplugin/core/plugin_manager.hpp>
#include <qtplugin/core/plugin_interface.hpp>

#include <memory>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <atomic>

using namespace qtplugin;

class HotReloadComprehensiveTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic hot reload functionality
    void testHotReloadManagerCreation();
    void testEnableHotReload();
    void testDisableHotReload();
    void testFileWatchingSetup();

    // File change detection
    void testFileChangeDetection();
    void testMultipleFileChanges();
    void testRapidFileChanges();
    void testFileCreationAndDeletion();

    // Plugin reload scenarios
    void testPluginReloadCallback();
    void testReloadCallbackErrorHandling();
    void testConcurrentReloadRequests();
    void testReloadDuringPluginExecution();

    // Performance and reliability
    void testHotReloadPerformance();
    void testMemoryUsageDuringReload();
    void testLongRunningReloadOperations();
    void testReloadStressTest();

    // Error handling and edge cases
    void testInvalidFilePathHandling();
    void testMissingFileHandling();
    void testPermissionDeniedHandling();
    void testFileSystemErrorRecovery();

    // Integration scenarios
    void testHotReloadWithPluginManager();
    void testHotReloadWithMetricsCollection();
    void testHotReloadWithSecurityValidation();

private:
    std::unique_ptr<QTemporaryDir> m_temp_dir;
    std::unique_ptr<PluginHotReloadManager> m_hot_reload_manager;
    std::unique_ptr<PluginManager> m_plugin_manager;

    QString createTestPlugin(const QString& plugin_name, const QString& content = QString());
    void modifyTestPlugin(const QString& plugin_path, const QString& new_content);
    bool waitForSignal(QObject* sender, const char* signal, int timeout_ms = 5000);
};

void HotReloadComprehensiveTest::initTestCase()
{
    qDebug() << "Starting comprehensive hot reload tests";

    // Create temporary directory for test files
    m_temp_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_temp_dir->isValid());

    qDebug() << "Test directory:" << m_temp_dir->path();
}

void HotReloadComprehensiveTest::cleanupTestCase()
{
    qDebug() << "Comprehensive hot reload tests completed";
}

void HotReloadComprehensiveTest::init()
{
    // Create fresh instances for each test
    m_hot_reload_manager = std::make_unique<PluginHotReloadManager>();
    m_plugin_manager = std::make_unique<PluginManager>();
}

void HotReloadComprehensiveTest::cleanup()
{
    // Clean up after each test
    m_hot_reload_manager.reset();
    m_plugin_manager.reset();
}

void HotReloadComprehensiveTest::testHotReloadManagerCreation()
{
    // Test basic creation and initialization
    QVERIFY(m_hot_reload_manager != nullptr);

    // Test initial state
    QVERIFY(!m_hot_reload_manager->is_hot_reload_enabled());
    QCOMPARE(m_hot_reload_manager->get_watched_plugin_count(), 0);
}

void HotReloadComprehensiveTest::testEnableHotReload()
{
    // Create a test plugin file
    QString plugin_path = createTestPlugin("test_plugin");
    QVERIFY(!plugin_path.isEmpty());

    // Enable hot reload for the plugin
    std::string plugin_id = "test_plugin";
    std::filesystem::path fs_path(plugin_path.toStdString());

    auto result = m_hot_reload_manager->enable_hot_reload(plugin_id, fs_path);
    QVERIFY(result.has_value());

    // Verify hot reload is enabled
    QVERIFY(m_hot_reload_manager->is_hot_reload_enabled());
    QCOMPARE(m_hot_reload_manager->get_watched_plugin_count(), 1);
}

void HotReloadComprehensiveTest::testDisableHotReload()
{
    // First enable hot reload
    QString plugin_path = createTestPlugin("test_plugin");
    std::string plugin_id = "test_plugin";
    std::filesystem::path fs_path(plugin_path.toStdString());

    auto enable_result = m_hot_reload_manager->enable_hot_reload(plugin_id, fs_path);
    QVERIFY(enable_result.has_value());
    QCOMPARE(m_hot_reload_manager->get_watched_plugin_count(), 1);

    // Now disable hot reload
    auto disable_result = m_hot_reload_manager->disable_hot_reload(plugin_id);
    QVERIFY(disable_result.has_value());
    QCOMPARE(m_hot_reload_manager->get_watched_plugin_count(), 0);
}

void HotReloadComprehensiveTest::testFileChangeDetection()
{
    // Create test plugin and enable hot reload
    QString plugin_path = createTestPlugin("test_plugin", "// Original content");
    std::string plugin_id = "test_plugin";
    std::filesystem::path fs_path(plugin_path.toStdString());

    auto result = m_hot_reload_manager->enable_hot_reload(plugin_id, fs_path);
    QVERIFY(result.has_value());

    // Set up signal spy to detect file changes
    QSignalSpy spy(m_hot_reload_manager.get(), &PluginHotReloadManager::plugin_file_changed);

    // Modify the file
    modifyTestPlugin(plugin_path, "// Modified content");

    // Wait for file change signal
    QVERIFY(spy.wait(5000));
    QCOMPARE(spy.count(), 1);

    // Verify signal parameters
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QString::fromStdString(plugin_id));
}

void HotReloadComprehensiveTest::testPluginReloadCallback()
{
    // Create test plugin
    QString plugin_path = createTestPlugin("test_plugin");
    std::string plugin_id = "test_plugin";
    std::filesystem::path fs_path(plugin_path.toStdString());

    // Set up reload callback
    std::atomic<bool> callback_called{false};
    std::string callback_plugin_id;

    m_hot_reload_manager->set_reload_callback([&](const std::string& id) {
        callback_called = true;
        callback_plugin_id = id;
    });

    // Enable hot reload
    auto result = m_hot_reload_manager->enable_hot_reload(plugin_id, fs_path);
    QVERIFY(result.has_value());

    // Modify the file to trigger reload
    modifyTestPlugin(plugin_path, "// Trigger reload");

    // Wait for callback to be called
    QElapsedTimer timer;
    timer.start();
    while (!callback_called && timer.elapsed() < 5000) {
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }

    QVERIFY(callback_called);
    QCOMPARE(callback_plugin_id, plugin_id);
}

void HotReloadComprehensiveTest::testHotReloadPerformance()
{
    // Create multiple test plugins
    const int plugin_count = 10;
    std::vector<QString> plugin_paths;

    for (int i = 0; i < plugin_count; ++i) {
        QString plugin_path = createTestPlugin(QString("test_plugin_%1").arg(i));
        plugin_paths.push_back(plugin_path);

        std::string plugin_id = QString("test_plugin_%1").arg(i).toStdString();
        std::filesystem::path fs_path(plugin_path.toStdString());

        auto result = m_hot_reload_manager->enable_hot_reload(plugin_id, fs_path);
        QVERIFY(result.has_value());
    }

    // Measure performance of file change detection
    QElapsedTimer timer;
    timer.start();

    // Modify all plugins simultaneously
    for (int i = 0; i < plugin_count; ++i) {
        modifyTestPlugin(plugin_paths[i], QString("// Modified %1").arg(i));
    }

    // Wait for all changes to be detected
    QSignalSpy spy(m_hot_reload_manager.get(), &PluginHotReloadManager::plugin_file_changed);
    while (spy.count() < plugin_count && timer.elapsed() < 10000) {
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }

    qint64 elapsed = timer.elapsed();

    // Performance expectations
    QVERIFY(elapsed < 5000); // Should complete within 5 seconds
    QCOMPARE(spy.count(), plugin_count);

    qDebug() << "Hot reload performance:" << elapsed << "ms for" << plugin_count << "plugins";
    qDebug() << "Average per plugin:" << (elapsed / plugin_count) << "ms";
}

void HotReloadComprehensiveTest::testErrorHandlingAndRecovery()
{
    // Test invalid plugin ID
    auto result1 = m_hot_reload_manager->enable_hot_reload("", std::filesystem::path("dummy"));
    QVERIFY(!result1.has_value());
    QCOMPARE(result1.error().code, PluginErrorCode::InvalidParameters);

    // Test non-existent file
    auto result2 = m_hot_reload_manager->enable_hot_reload("test", std::filesystem::path("/non/existent/file"));
    QVERIFY(!result2.has_value());
    QCOMPARE(result2.error().code, PluginErrorCode::FileNotFound);

    // Test disabling non-existent plugin
    auto result3 = m_hot_reload_manager->disable_hot_reload("non_existent_plugin");
    QVERIFY(!result3.has_value());
    QCOMPARE(result3.error().code, PluginErrorCode::PluginNotFound);
}

QString HotReloadComprehensiveTest::createTestPlugin(const QString& plugin_name, const QString& content)
{
    QString plugin_path = m_temp_dir->path() + "/" + plugin_name + ".cpp";
    QFile plugin_file(plugin_path);

    if (!plugin_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return QString();
    }

    QTextStream stream(&plugin_file);
    if (content.isEmpty()) {
        stream << "// Test plugin: " << plugin_name << "\n";
        stream << "#include <qtplugin/core/plugin_interface.hpp>\n";
        stream << "class " << plugin_name << " : public qtplugin::IPlugin {\n";
        stream << "    // Plugin implementation\n";
        stream << "};\n";
    } else {
        stream << content;
    }

    plugin_file.close();
    return plugin_path;
}

void HotReloadComprehensiveTest::modifyTestPlugin(const QString& plugin_path, const QString& new_content)
{
    QFile plugin_file(plugin_path);
    if (plugin_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&plugin_file);
        stream << new_content;
        plugin_file.close();

        // Force file system to recognize the change
        plugin_file.flush();
        QCoreApplication::processEvents();
    }
}

bool HotReloadComprehensiveTest::waitForSignal(QObject* sender, const char* signal, int timeout_ms)
{
    QSignalSpy spy(sender, signal);
    return spy.wait(timeout_ms);
}

QTEST_MAIN(HotReloadComprehensiveTest)
#include "test_hot_reload_comprehensive.moc"
