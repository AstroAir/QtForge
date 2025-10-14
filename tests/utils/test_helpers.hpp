/**
 * @file test_helpers.hpp
 * @brief Common test utilities and helpers for QtForge tests
 */

#pragma once

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTextStream>
#include <QUuid>
#include <QtTest/QtTest>
#include <functional>
#include <memory>

#include <qtplugin/core/plugin_manager.hpp>
#include <qtplugin/interfaces/core/plugin_interface.hpp>
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
        QTemporaryDir& tempDir, const QString& pluginName = "test_plugin",
        const QJsonObject& metadata = QJsonObject()) {
        QString pluginPath = tempDir.path() + "/" + pluginName + ".json";
        QFile file(pluginPath);

        if (file.open(QIODevice::WriteOnly)) {
            QJsonObject finalMetadata = metadata.isEmpty()
                                            ? generatePluginMetadata(pluginName)
                                            : metadata;

            QJsonDocument doc(finalMetadata);
            file.write(doc.toJson());
        }

        return pluginPath;
    }
};

/**
 * @brief Mock plugin interface for testing
 */
class MockPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT

public:
    MockPlugin(QObject* parent = nullptr)
        : QObject(parent), m_state(qtplugin::PluginState::Unloaded) {}
    ~MockPlugin() override = default;

    // IPlugin interface
    qtplugin::expected<void, qtplugin::PluginError> initialize() override {
        if (m_state == qtplugin::PluginState::Running) {
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::AlreadyExists,
                "Plugin already initialized");
        }
        m_state = qtplugin::PluginState::Running;
        m_initializeCalled = true;
        return {};
    }

    void shutdown() noexcept override {
        m_state = qtplugin::PluginState::Stopped;
        m_shutdownCalled = true;
    }

    qtplugin::PluginMetadata metadata() const override {
        qtplugin::PluginMetadata meta;
        meta.name = "MockPlugin";
        meta.version = qtplugin::Version(1, 0, 0);
        meta.description = "Mock plugin for testing";
        meta.author = "Test Suite";
        meta.license = "MIT";
        meta.category = "test";
        meta.capabilities =
            static_cast<uint32_t>(qtplugin::PluginCapability::None);
        meta.priority = qtplugin::PluginPriority::Normal;
        return meta;
    }

    qtplugin::PluginState state() const noexcept override { return m_state; }

    uint32_t capabilities() const noexcept override {
        return static_cast<uint32_t>(qtplugin::PluginCapability::None);
    }

    qtplugin::PluginPriority priority() const noexcept override {
        return qtplugin::PluginPriority::Normal;
    }

    bool is_initialized() const noexcept override {
        return m_state == qtplugin::PluginState::Running;
    }

    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override {
        Q_UNUSED(params);
        Q_UNUSED(command);
        QJsonObject result;
        result["status"] = "success";
        return result;
    }

    std::vector<std::string> available_commands() const override {
        return {"test"};
    }

    qtplugin::expected<void, qtplugin::PluginError> configure(
        const QJsonObject& config) override {
        m_config = config;
        return {};
    }

    QJsonObject get_configuration() const override { return m_config; }

    // Test helpers
    void setInitialized(bool initialized) {
        m_state = initialized ? qtplugin::PluginState::Running
                              : qtplugin::PluginState::Stopped;
    }
    bool wasInitializeCalled() const { return m_initializeCalled; }
    bool wasShutdownCalled() const { return m_shutdownCalled; }

private:
    qtplugin::PluginState m_state;
    QJsonObject m_config;
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
        if (!m_tempDir)
            return QString();
        return TestDataGenerator::createTempPluginFile(*m_tempDir, name,
                                                       metadata);
    }
};

/**
 * @brief Utility macros for common test patterns
 */
#define QTFORGE_VERIFY_SUCCESS(result)                              \
    do {                                                            \
        QVERIFY(result.has_value());                                \
        if (!result.has_value()) {                                  \
            qDebug() << "Error:" << result.error().message.c_str(); \
        }                                                           \
    } while (0)

#define QTFORGE_VERIFY_ERROR(result, expectedCode)       \
    do {                                                 \
        QVERIFY(!result.has_value());                    \
        if (!result.has_value()) {                       \
            QCOMPARE(result.error().code, expectedCode); \
        }                                                \
    } while (0)

#define QTFORGE_VERIFY_ERROR_MESSAGE(result, expectedMessage)      \
    do {                                                           \
        QVERIFY(!result.has_value());                              \
        if (!result.has_value()) {                                 \
            QVERIFY(QString::fromStdString(result.error().message) \
                        .contains(expectedMessage));               \
        }                                                          \
    } while (0)

}  // namespace QtForgeTest
