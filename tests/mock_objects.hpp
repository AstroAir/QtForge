/**
 * @file mock_objects.hpp
 * @brief Mock objects for QtForge testing
 */

#pragma once

#include <QtTest/QtTest>
#include <QObject>
#include <QJsonObject>
#include <QTimer>
#include <memory>
#include <functional>

#include <qtplugin/core/plugin_interface.hpp>
#include <qtplugin/core/plugin_manager.hpp>
#include <qtplugin/communication/message_bus.hpp>
#include <qtplugin/security/security_manager.hpp>
#include <qtplugin/managers/configuration_manager.hpp>
#include <qtplugin/utils/error_handling.hpp>

namespace QtForgeTest {

/**
 * @brief Mock Configuration Manager for testing
 */
class MockConfigurationManager : public QObject {
    Q_OBJECT

public:
    explicit MockConfigurationManager(QObject* parent = nullptr)
        : QObject(parent) {}

    // Mock configuration operations
    qtplugin::expected<QJsonValue, qtplugin::PluginError> get_value(
        std::string_view key, qtplugin::ConfigurationScope scope = qtplugin::ConfigurationScope::Global,
        std::string_view plugin_id = "") {
        Q_UNUSED(scope)
        Q_UNUSED(plugin_id)
        
        QString keyStr = QString::fromUtf8(key.data(), key.size());
        if (m_config.contains(keyStr)) {
            return m_config[keyStr];
        }
        
        return qtplugin::make_error<QJsonValue>(
            qtplugin::PluginErrorCode::ConfigurationError,
            "Configuration key not found: " + keyStr.toStdString());
    }

    qtplugin::expected<void, qtplugin::PluginError> set_value(
        std::string_view key, const QJsonValue& value,
        qtplugin::ConfigurationScope scope = qtplugin::ConfigurationScope::Global,
        std::string_view plugin_id = "") {
        Q_UNUSED(scope)
        Q_UNUSED(plugin_id)
        
        QString keyStr = QString::fromUtf8(key.data(), key.size());
        m_config[keyStr] = value;
        return qtplugin::make_success();
    }

    void clear() { m_config = QJsonObject(); }
    QJsonObject getConfig() const { return m_config; }

private:
    QJsonObject m_config;
};

/**
 * @brief Mock Security Manager for testing
 */
class MockSecurityManager : public QObject {
    Q_OBJECT

public:
    explicit MockSecurityManager(QObject* parent = nullptr)
        : QObject(parent), m_validation_result(true) {}

    // Mock security operations
    qtplugin::expected<bool, qtplugin::PluginError> validate_plugin(
        const QString& plugin_path) {
        Q_UNUSED(plugin_path)
        
        if (m_validation_result) {
            return true;
        } else {
            return qtplugin::make_error<bool>(
                qtplugin::PluginErrorCode::SecurityValidationFailed,
                "Mock validation failed");
        }
    }

    qtplugin::expected<void, qtplugin::PluginError> apply_security_policy(
        const QString& plugin_id, const QJsonObject& policy) {
        Q_UNUSED(plugin_id)
        Q_UNUSED(policy)
        return qtplugin::make_success();
    }

    // Test control methods
    void setValidationResult(bool result) { m_validation_result = result; }
    bool getValidationResult() const { return m_validation_result; }

private:
    bool m_validation_result;
};

/**
 * @brief Mock Message Bus for testing
 */
class MockMessageBus : public QObject {
    Q_OBJECT

public:
    explicit MockMessageBus(QObject* parent = nullptr)
        : QObject(parent) {}

    // Mock message operations
    qtplugin::expected<void, qtplugin::PluginError> publish(
        const QString& topic, const QJsonObject& message) {
        
        m_published_messages[topic].append(message);
        emit messagePublished(topic, message);
        return qtplugin::make_success();
    }

    qtplugin::expected<void, qtplugin::PluginError> subscribe(
        const QString& topic, std::function<void(const QJsonObject&)> callback) {
        
        m_subscriptions[topic].append(callback);
        return qtplugin::make_success();
    }

    // Test helper methods
    QList<QJsonObject> getPublishedMessages(const QString& topic) const {
        return m_published_messages.value(topic);
    }

    void clearMessages() { m_published_messages.clear(); }
    
    int getSubscriptionCount(const QString& topic) const {
        return m_subscriptions.value(topic).size();
    }

signals:
    void messagePublished(const QString& topic, const QJsonObject& message);

private:
    QHash<QString, QList<QJsonObject>> m_published_messages;
    QHash<QString, QList<std::function<void(const QJsonObject&)>>> m_subscriptions;
};

/**
 * @brief Mock Plugin Loader for testing
 */
class MockPluginLoader : public QObject {
    Q_OBJECT

public:
    explicit MockPluginLoader(QObject* parent = nullptr)
        : QObject(parent), m_load_success(true) {}

    // Mock loading operations
    qtplugin::expected<std::shared_ptr<qtplugin::IPlugin>, qtplugin::PluginError> 
    load_plugin(const QString& plugin_path) {
        
        if (!m_load_success) {
            return qtplugin::make_error<std::shared_ptr<qtplugin::IPlugin>>(
                qtplugin::PluginErrorCode::LoadFailed,
                "Mock load failure");
        }

        // Create a mock plugin
        auto plugin = std::make_shared<MockPlugin>(
            QFileInfo(plugin_path).baseName());
        
        m_loaded_plugins[plugin_path] = plugin;
        return plugin;
    }

    qtplugin::expected<void, qtplugin::PluginError> unload_plugin(
        const QString& plugin_path) {
        
        if (m_loaded_plugins.contains(plugin_path)) {
            m_loaded_plugins.remove(plugin_path);
            return qtplugin::make_success();
        }
        
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::PluginNotFound,
            "Plugin not loaded: " + plugin_path.toStdString());
    }

    // Test control methods
    void setLoadSuccess(bool success) { m_load_success = success; }
    bool isPluginLoaded(const QString& plugin_path) const {
        return m_loaded_plugins.contains(plugin_path);
    }
    
    int getLoadedPluginCount() const { return m_loaded_plugins.size(); }
    void clearLoadedPlugins() { m_loaded_plugins.clear(); }

private:
    bool m_load_success;
    QHash<QString, std::shared_ptr<qtplugin::IPlugin>> m_loaded_plugins;
};

/**
 * @brief Test Environment Setup Helper
 */
class TestEnvironment {
public:
    static void setupTestEnvironment() {
        // Set up test-specific Qt application attributes
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
        
        // Set test-specific logging
        QLoggingCategory::setFilterRules("qtforge.*.debug=true");
    }

    static void cleanupTestEnvironment() {
        // Cleanup any global test state
    }
};

/**
 * @brief Async Test Helper for testing asynchronous operations
 */
class AsyncTestHelper : public QObject {
    Q_OBJECT

public:
    explicit AsyncTestHelper(QObject* parent = nullptr)
        : QObject(parent), m_timeout(5000) {}

    /**
     * @brief Wait for a signal to be emitted within timeout
     */
    bool waitForSignal(QObject* sender, const char* signal, int timeout = -1) {
        if (timeout == -1) timeout = m_timeout;
        
        QSignalSpy spy(sender, signal);
        return spy.wait(timeout);
    }

    /**
     * @brief Execute an async operation and wait for completion
     */
    template<typename Func>
    bool executeAsync(Func&& func, int timeout = -1) {
        if (timeout == -1) timeout = m_timeout;
        
        bool completed = false;
        QTimer::singleShot(0, [&]() {
            func();
            completed = true;
        });
        
        QElapsedTimer timer;
        timer.start();
        
        while (!completed && timer.elapsed() < timeout) {
            QCoreApplication::processEvents();
            QThread::msleep(10);
        }
        
        return completed;
    }

    void setTimeout(int timeout) { m_timeout = timeout; }

private:
    int m_timeout;
};

} // namespace QtForgeTest

#include "mock_objects.moc"
