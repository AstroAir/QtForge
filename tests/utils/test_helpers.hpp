/**
 * @file test_helpers.hpp
 * @brief Common test utilities and helpers for QtForge tests
 */

#pragma once

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include <QUuid>
#include <memory>
#include <functional>

#include <qtplugin/core/plugin_interface.hpp>
#include <qtplugin/core/plugin_manager.hpp>
#include <qtplugin/utils/error_handling.hpp>

namespace QtForgeTest {

/**
 * @brief Test data generator for creating mock plugin data
 */
class TestDataGenerator {
public:
    /**
     * @brief Generate plugin metadata for testing
     */
    static QJsonObject generatePluginMetadata(
        const QString& pluginName = "TestPlugin",
        const QString& version = "1.0.0") {

        QJsonObject metadata;
        metadata["name"] = pluginName;
        metadata["version"] = version;
        metadata["description"] = "A test plugin for QtForge testing";
        metadata["author"] = "QtForge Test Suite";
        metadata["license"] = "MIT";
        metadata["category"] = "test";

        QJsonObject capabilities;
        capabilities["supports_hot_reload"] = true;
        capabilities["thread_safe"] = true;
        metadata["capabilities"] = capabilities;

        return metadata;
    }

    /**
     * @brief Generate test configuration data
     */
    static QJsonObject generateTestConfiguration(
        const QString& configName = "test_config") {

        QJsonObject config;
        config["name"] = configName;
        config["enabled"] = true;
        config["log_level"] = "debug";
        config["timeout"] = 30000;

        QJsonObject settings;
        settings["test_mode"] = true;
        settings["mock_data"] = true;
        config["settings"] = settings;

        return config;
    }

    /**
     * @brief Create a temporary plugin file with metadata
     */
    static QString createTempPluginFile(
        QTemporaryDir& tempDir,
        const QString& pluginName = "test_plugin",
        const QJsonObject& metadata = QJsonObject()) {

        QString pluginPath = tempDir.path() + "/" + pluginName + ".json";
        QFile file(pluginPath);

        if (file.open(QIODevice::WriteOnly)) {
            QJsonObject finalMetadata = metadata.isEmpty() ?
                generatePluginMetadata(pluginName) : metadata;

            QJsonDocument doc(finalMetadata);
            file.write(doc.toJson());
        }

        return pluginPath;
    }
};

/**
 * @brief Mock plugin interface for testing
 */
class MockPlugin : public qtplugin::IPlugin {
    Q_OBJECT

public:
    MockPlugin() = default;
    ~MockPlugin() override = default;

    // IPlugin interface
    QString name() const override { return "MockPlugin"; }
    QString version() const override { return "1.0.0"; }
    QString description() const override { return "Mock plugin for testing"; }
    QString author() const override { return "Test Suite"; }
    QString license() const override { return "MIT"; }

    bool initialize() override {
        m_initialized = true;
        return true;
    }

    void shutdown() override {
        m_initialized = false;
    }

    bool isInitialized() const override {
        return m_initialized;
    }

    QJsonObject metadata() const override {
        return TestDataGenerator::generatePluginMetadata(name(), version());
    }

    // Test helpers
    void setInitialized(bool initialized) { m_initialized = initialized; }
    bool wasInitializeCalled() const { return m_initializeCalled; }
    bool wasShutdownCalled() const { return m_shutdownCalled; }

private:
    bool m_initialized = false;
    bool m_initializeCalled = false;
    bool m_shutdownCalled = false;
};

/**
 * @brief Base test fixture with common setup/teardown
 */
class TestFixtureBase : public QObject {
    Q_OBJECT

public:
    TestFixtureBase() = default;
    virtual ~TestFixtureBase() = default;

protected slots:
    virtual void initTestCase() {
        // Create temporary directory for test files
        m_tempDir = std::make_unique<QTemporaryDir>();
        QVERIFY(m_tempDir->isValid());
    }

    virtual void cleanupTestCase() {
        // Cleanup is automatic with QTemporaryDir
        m_tempDir.reset();
    }

    virtual void init() {
        // Per-test setup
    }

    virtual void cleanup() {
        // Per-test cleanup
    }

protected:
    std::unique_ptr<QTemporaryDir> m_tempDir;

    /**
     * @brief Get temporary directory path
     */
    QString tempPath() const {
        return m_tempDir ? m_tempDir->path() : QString();
    }

    /**
     * @brief Create a test plugin in temporary directory
     */
    QString createTestPlugin(const QString& name = "test_plugin",
                           const QJsonObject& metadata = QJsonObject()) {
        if (!m_tempDir) return QString();
        return TestDataGenerator::createTempPluginFile(*m_tempDir, name, metadata);
    }
};

/**
 * @brief Utility macros for common test patterns
 */
#define QTFORGE_VERIFY_SUCCESS(result) \
    do { \
        QVERIFY(result.has_value()); \
        if (!result.has_value()) { \
            qDebug() << "Error:" << result.error().message.c_str(); \
        } \
    } while(0)

#define QTFORGE_VERIFY_ERROR(result, expectedCode) \
    do { \
        QVERIFY(!result.has_value()); \
        if (!result.has_value()) { \
            QCOMPARE(result.error().code, expectedCode); \
        } \
    } while(0)

#define QTFORGE_VERIFY_ERROR_MESSAGE(result, expectedMessage) \
    do { \
        QVERIFY(!result.has_value()); \
        if (!result.has_value()) { \
            QVERIFY(QString::fromStdString(result.error().message).contains(expectedMessage)); \
        } \
    } while(0)

} // namespace QtForgeTest

#include "test_helpers.moc"
