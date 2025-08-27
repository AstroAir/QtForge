/**
 * @file python_plugin_bridge.hpp
 * @brief Python plugin bridge for executing Python-based plugins
 * @version 3.2.0
 * @author QtPlugin Development Team
 *
 * This file provides a bridge between the QtForge plugin system and Python plugins,
 * allowing Python scripts to be loaded and executed as plugins with full integration
 * into the QtForge ecosystem.
 */

#pragma once

#include "../core/dynamic_plugin_interface.hpp"
#include <QObject>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include <memory>
#include <functional>

namespace qtplugin {

/**
 * @brief Python plugin execution environment
 */
class PythonExecutionEnvironment {
public:
    explicit PythonExecutionEnvironment(const QString& python_path = "python");
    ~PythonExecutionEnvironment();

    /**
     * @brief Initialize the Python environment
     */
    qtplugin::expected<void, PluginError> initialize();

    /**
     * @brief Shutdown the Python environment
     */
    void shutdown();

    /**
     * @brief Execute Python code
     * @param code Python code to execute
     * @param context Execution context
     * @return Execution result
     */
    qtplugin::expected<QJsonObject, PluginError> execute_code(
        const QString& code, const QJsonObject& context = {});

    /**
     * @brief Load a Python plugin module
     * @param plugin_path Path to the Python plugin file
     * @param plugin_class Name of the plugin class
     * @return Success or error
     */
    qtplugin::expected<QString, PluginError> load_plugin_module(
        const QString& plugin_path, const QString& plugin_class);

    /**
     * @brief Call a method on a loaded plugin
     * @param plugin_id Plugin identifier
     * @param method_name Method name
     * @param parameters Method parameters
     * @return Method result
     */
    qtplugin::expected<QJsonObject, PluginError> call_plugin_method(
        const QString& plugin_id, const QString& method_name,
        const QJsonArray& parameters = {});

    /**
     * @brief Check if environment is running
     */
    bool is_running() const { return m_process && m_process->state() == QProcess::Running; }

    /**
     * @brief Get Python interpreter path
     */
    QString python_path() const { return m_python_path; }

private:
    QString m_python_path;
    std::unique_ptr<QProcess> m_process;
    QMutex m_mutex;
    QWaitCondition m_response_condition;
    QJsonObject m_last_response;
    bool m_waiting_for_response = false;
    int m_request_id = 0;

    void setup_process();
    void handle_process_output();
    void handle_process_error();
    qtplugin::expected<QJsonObject, PluginError> send_request(const QJsonObject& request);
};

/**
 * @brief Python plugin bridge implementation
 */
class PythonPluginBridge : public QObject, public IDynamicPlugin {
    Q_OBJECT

public:
    explicit PythonPluginBridge(const QString& plugin_path, QObject* parent = nullptr);
    ~PythonPluginBridge() override;

    // === IPlugin Implementation ===
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    qtplugin::Version version() const noexcept override;
    std::string_view author() const noexcept override;
    std::string id() const noexcept override;

    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override;

    qtplugin::PluginCapabilities capabilities() const noexcept override;

    qtplugin::expected<void, qtplugin::PluginError> configure(
        const QJsonObject& config) override;
    QJsonObject current_configuration() const override;

    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override;
    std::vector<std::string> available_commands() const override;

    // === IEnhancedPlugin Implementation ===
    qtplugin::expected<void, qtplugin::PluginError> hot_reload();
    bool validate_configuration(const QJsonObject& config) const override;
    QJsonObject get_configuration_schema() const;
    qtplugin::expected<void, qtplugin::PluginError> handle_dependency_change(
        const QString& dependency_id, qtplugin::PluginState new_state);

    // === IDynamicPlugin Implementation ===
    std::vector<qtplugin::InterfaceDescriptor> get_interface_descriptors() const override;
    bool supports_interface(const QString& interface_id,
                           const qtplugin::Version& min_version = qtplugin::Version{}) const override;
    std::optional<qtplugin::InterfaceDescriptor> get_interface_descriptor(
        const QString& interface_id) const override;

    qtplugin::expected<void, qtplugin::PluginError> adapt_to_interface(
        const QString& interface_id, const qtplugin::Version& target_version) override;

    qtplugin::expected<std::vector<qtplugin::InterfaceCapability>, qtplugin::PluginError>
    negotiate_capabilities(const QString& other_plugin_id,
                          const std::vector<qtplugin::InterfaceCapability>& requested_capabilities) override;

    qtplugin::PluginType get_plugin_type() const override;
    qtplugin::PluginExecutionContext get_execution_context() const override;

    qtplugin::expected<QVariant, qtplugin::PluginError> execute_code(
        const QString& code, const QJsonObject& context = {}) override;

    qtplugin::expected<QVariant, qtplugin::PluginError> invoke_method(
        const QString& method_name, const QVariantList& parameters = {},
        const QString& interface_id = {}) override;

    std::vector<QString> get_available_methods(const QString& interface_id = {}) const override;
    std::optional<QJsonObject> get_method_signature(
        const QString& method_name, const QString& interface_id = {}) const override;

    qtplugin::expected<QVariant, qtplugin::PluginError> get_property(
        const QString& property_name, const QString& interface_id = {}) override;
    qtplugin::expected<void, qtplugin::PluginError> set_property(
        const QString& property_name, const QVariant& value,
        const QString& interface_id = {}) override;
    std::vector<QString> get_available_properties(const QString& interface_id = {}) const override;

    qtplugin::expected<void, qtplugin::PluginError> subscribe_to_events(
        const QString& source_plugin_id, const std::vector<QString>& event_types,
        std::function<void(const QString&, const QJsonObject&)> callback) override;
    qtplugin::expected<void, qtplugin::PluginError> unsubscribe_from_events(
        const QString& source_plugin_id, const std::vector<QString>& event_types = {}) override;
    qtplugin::expected<void, qtplugin::PluginError> emit_event(
        const QString& event_type, const QJsonObject& event_data) override;

private slots:
    void handle_environment_error();

private:
    QString m_plugin_path;
    QString m_plugin_id;
    QJsonObject m_metadata;
    qtplugin::PluginState m_state{qtplugin::PluginState::Unloaded};
    std::unique_ptr<PythonExecutionEnvironment> m_environment;
    std::vector<qtplugin::InterfaceDescriptor> m_interfaces;
    std::vector<QString> m_available_methods;
    std::vector<QString> m_available_properties;
    QJsonObject m_configuration;

    qtplugin::expected<void, qtplugin::PluginError> load_metadata();
    qtplugin::expected<void, qtplugin::PluginError> setup_interfaces();
    qtplugin::expected<void, qtplugin::PluginError> discover_methods_and_properties();
    QJsonArray convert_variant_list_to_json(const QVariantList& list) const;
    QVariant convert_json_to_variant(const QJsonValue& value) const;
};

/**
 * @brief Python plugin factory for creating Python plugin bridges
 */
class PythonPluginFactory {
public:
    /**
     * @brief Create a Python plugin bridge
     * @param plugin_path Path to the Python plugin file
     * @param parent Parent QObject
     * @return Plugin bridge or error
     */
    static qtplugin::expected<std::unique_ptr<PythonPluginBridge>, PluginError>
    create_plugin(const QString& plugin_path, QObject* parent = nullptr);

    /**
     * @brief Validate Python plugin
     * @param plugin_path Path to the Python plugin file
     * @return Validation result
     */
    static qtplugin::expected<void, PluginError> validate_plugin(const QString& plugin_path);

    /**
     * @brief Check if Python is available
     * @param python_path Path to Python interpreter
     * @return True if Python is available
     */
    static bool is_python_available(const QString& python_path = "python");

    /**
     * @brief Get Python version
     * @param python_path Path to Python interpreter
     * @return Python version string
     */
    static QString get_python_version(const QString& python_path = "python");
};

} // namespace qtplugin

Q_DECLARE_INTERFACE(qtplugin::PythonPluginBridge, "qtplugin.PythonPluginBridge/3.2")
