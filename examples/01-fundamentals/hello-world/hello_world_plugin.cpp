/**
 * @file hello_world_plugin.cpp
 * @brief Implementation of minimal QtForge plugin
 */

#include "hello_world_plugin.hpp"
#include <QDebug>
#include <QDateTime>

HelloWorldPlugin::HelloWorldPlugin(QObject* parent)
    : QObject(parent) {
    // Minimal constructor - just set parent
}

std::string_view HelloWorldPlugin::name() const noexcept {
    return "HelloWorldPlugin";
}

std::string_view HelloWorldPlugin::description() const noexcept {
    return "Minimal QtForge plugin for beginners";
}

qtplugin::Version HelloWorldPlugin::version() const noexcept {
    return qtplugin::Version{1, 0, 0};
}

std::string_view HelloWorldPlugin::author() const noexcept {
    return "QtForge Examples";
}

std::string HelloWorldPlugin::id() const noexcept {
    return "com.qtforge.examples.hello_world";
}

qtplugin::PluginCapabilities HelloWorldPlugin::capabilities() const noexcept {
    return static_cast<qtplugin::PluginCapabilities>(qtplugin::PluginCapability::None);
}

qtplugin::expected<void, qtplugin::PluginError> HelloWorldPlugin::initialize() {
    qDebug() << "HelloWorldPlugin: Initializing...";

    m_state = qtplugin::PluginState::Loaded;

    qDebug() << "HelloWorldPlugin: Initialized successfully!";
    return {};
}

void HelloWorldPlugin::shutdown() noexcept {
    qDebug() << "HelloWorldPlugin: Shutting down...";
    m_state = qtplugin::PluginState::Unloaded;
    qDebug() << "HelloWorldPlugin: Shutdown complete.";
}

qtplugin::expected<QJsonObject, qtplugin::PluginError> HelloWorldPlugin::execute_command(
    std::string_view command, const QJsonObject& params) {

    if (m_state != qtplugin::PluginState::Loaded) {
        return qtplugin::unexpected(qtplugin::PluginError{
            qtplugin::PluginErrorCode::InvalidState,
            "Plugin not initialized"
        });
    }

    if (command == "hello") {
        QString name = params.value("name").toString("World");

        QJsonObject result;
        result["message"] = QString("Hello, %1!").arg(name);
        result["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        result["plugin"] = "HelloWorldPlugin";

        qDebug() << "HelloWorldPlugin: Executed 'hello' command for" << name;
        return result;
    }

    return qtplugin::unexpected(qtplugin::PluginError{
        qtplugin::PluginErrorCode::CommandNotFound,
        std::string("Unknown command: ") + std::string(command)
    });
}

std::vector<std::string> HelloWorldPlugin::available_commands() const {
    return {"hello"};
}

qtplugin::PluginMetadata HelloWorldPlugin::metadata() const {
    qtplugin::PluginMetadata meta;
    meta.name = "HelloWorldPlugin";
    meta.description = "Minimal QtForge plugin for beginners";
    meta.version = qtplugin::Version{1, 0, 0};
    meta.author = "QtForge Examples";
    meta.category = "Example";
    meta.license = "MIT";
    meta.homepage = "https://github.com/qtforge/examples";
    return meta;
}

qtplugin::PluginState HelloWorldPlugin::state() const noexcept {
    return m_state;
}
