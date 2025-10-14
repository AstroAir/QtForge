/**
 * @file mock_objects.hpp
 * @brief Mock objects for QtForge testing
 * @version 3.2.0
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <qtplugin/interfaces/core/plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>
#include <qtplugin/utils/version.hpp>

namespace QtForgeTest {

/**
 * @brief Base mock plugin implementation for testing
 */
class MockPluginBase : public qtplugin::IPlugin {
public:
    MockPluginBase() : m_state(qtplugin::PluginState::Unloaded) {}

    ~MockPluginBase() override = default;

    // IPlugin interface implementation
    qtplugin::expected<void, qtplugin::PluginError> initialize() override {
        if (m_state == qtplugin::PluginState::Running) {
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::AlreadyExists,
                "Plugin already initialized");
        }
        m_state = qtplugin::PluginState::Running;
        return {};
    }

    void shutdown() noexcept override {
        m_state = qtplugin::PluginState::Stopped;
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

        if (command == "test") {
            QJsonObject result;
            result["status"] = "success";
            return result;
        }

        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound,
            "Unknown command: " + std::string(command));
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
    void setState(qtplugin::PluginState state) { m_state = state; }

protected:
    qtplugin::PluginState m_state;
    QJsonObject m_config;
};

/**
 * @brief Advanced mock plugin with service contracts
 */
class AdvancedPluginBase : public MockPluginBase {
public:
    AdvancedPluginBase() : MockPluginBase() {}

    ~AdvancedPluginBase() override = default;

    qtplugin::PluginMetadata metadata() const override {
        auto meta = MockPluginBase::metadata();
        meta.name = "AdvancedMockPlugin";
        meta.capabilities =
            static_cast<uint32_t>(qtplugin::PluginCapability::Service) |
            static_cast<uint32_t>(qtplugin::PluginCapability::Configuration);
        return meta;
    }

    uint32_t capabilities() const noexcept override {
        return static_cast<uint32_t>(qtplugin::PluginCapability::Service) |
               static_cast<uint32_t>(qtplugin::PluginCapability::Configuration);
    }

    std::vector<std::string> available_commands() const override {
        return {"test", "advanced_test", "service_call"};
    }

    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override {
        if (command == "advanced_test") {
            QJsonObject result;
            result["status"] = "advanced_success";
            result["params"] = params;
            return result;
        }

        if (command == "service_call") {
            QJsonObject result;
            result["service"] = "mock_service";
            result["response"] = "service_data";
            return result;
        }

        return MockPluginBase::execute_command(command, params);
    }
};

}  // namespace QtForgeTest
