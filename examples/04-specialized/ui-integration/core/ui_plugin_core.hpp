/**
 * @file ui_plugin_core.hpp
 * @brief Core UI plugin implementation
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QWidget>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "qtplugin/core/plugin_interface.hpp"
#include "qtplugin/interfaces/ui_plugin_interface.hpp"
#include "qtplugin/utils/error_handling.hpp"

// Forward declarations
class DemoWidget;
class SettingsDialog;
class AboutDialog;

/**
 * @brief Core UI plugin implementation
 */
class UIPluginCore : public QObject, public qtplugin::IUIPlugin {
    Q_OBJECT
    Q_INTERFACES(qtplugin::IUIPlugin)
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/1.0" FILE "../ui_plugin.json")

public:
    explicit UIPluginCore(QObject* parent = nullptr);
    ~UIPluginCore() override;

    // IPlugin interface - Metadata
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    qtplugin::Version version() const noexcept override;
    std::string_view author() const noexcept override;
    std::string id() const noexcept override;
    qtplugin::PluginState state() const noexcept override;
    qtplugin::PluginCapabilities capabilities() const noexcept override;

    // IPlugin interface - Lifecycle
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    bool is_initialized() const noexcept override;
    qtplugin::expected<void, qtplugin::PluginError> pause() override;
    qtplugin::expected<void, qtplugin::PluginError> resume() override;
    qtplugin::expected<void, qtplugin::PluginError> restart() override;

    // Configuration
    std::optional<QJsonObject> default_configuration() const override;
    qtplugin::expected<void, qtplugin::PluginError> configure(
        const QJsonObject& config) override;
    QJsonObject current_configuration() const override;

    // Commands
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params) override;
    std::vector<std::string> available_commands() const override;

    // Dependencies
    std::vector<std::string> dependencies() const override;
    std::vector<std::string> optional_dependencies() const override;
    bool dependencies_satisfied() const override;

    // Error handling
    void clear_errors() override;

    // Monitoring
    std::chrono::milliseconds uptime() const override;
    QJsonObject performance_metrics() const override;
    QJsonObject resource_usage() const override;

    // IUIPlugin interface
    qtplugin::UIComponentTypes supported_components() const noexcept override;
    std::vector<qtplugin::UIIntegrationPoint> supported_integration_points()
        const override;

    // Widget management
    qtplugin::expected<QWidget*, qtplugin::PluginError> create_widget(
        const QString& widget_id, QWidget* parent = nullptr) override;
    qtplugin::expected<qtplugin::UIWidgetInfo, qtplugin::PluginError>
    get_widget_info(const QString& widget_id) const override;
    std::vector<QString> get_available_widgets() const override;
    qtplugin::expected<void, qtplugin::PluginError> destroy_widget(
        const QString& widget_id) override;

    // IUIPlugin interface - Action Management
    qtplugin::expected<QAction*, qtplugin::PluginError> create_action(
        const qtplugin::UIActionInfo& action_info,
        QObject* parent = nullptr) override;
    std::vector<qtplugin::UIActionInfo> get_available_actions() const override;
    qtplugin::expected<void, qtplugin::PluginError> set_action_callback(
        const QString& action_id, qtplugin::UIActionCallback callback) override;
    qtplugin::expected<void, qtplugin::PluginError> remove_action(
        const QString& action_id) override;

    // IUIPlugin interface - Menu and Toolbar Support
    qtplugin::expected<QMenu*, qtplugin::PluginError> create_menu(
        const QString& menu_id, const QString& title,
        QWidget* parent = nullptr) override;
    qtplugin::expected<QToolBar*, qtplugin::PluginError> create_toolbar(
        const QString& toolbar_id, const QString& title,
        QWidget* parent = nullptr) override;
    qtplugin::expected<QDialog*, qtplugin::PluginError> create_dialog(
        const QString& dialog_id, QWidget* parent = nullptr) override;
    qtplugin::expected<int, qtplugin::PluginError> show_modal_dialog(
        const QString& dialog_id) override;

    // Theme management
    qtplugin::expected<void, qtplugin::PluginError> apply_theme(
        const QString& theme_name);
    std::vector<qtplugin::UIThemeInfo> get_available_themes() const override;
    QString get_current_theme() const;

signals:
    void widget_created(const QString& widget_id);
    void widget_destroyed(const QString& widget_id);
    void theme_changed(const QString& theme_name);

private slots:
    void on_widget_data_changed(const QJsonObject& data);
    void on_action_triggered(const QString& action);
    void on_settings_changed(const QJsonObject& settings);

private:
    // Command handlers
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    handle_widget_command(const QJsonObject& params);
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    handle_action_command(const QJsonObject& params);
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    handle_dialog_command(const QJsonObject& params);
    qtplugin::expected<QJsonObject, qtplugin::PluginError> handle_theme_command(
        const QJsonObject& params);
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    handle_settings_command(const QJsonObject& params);
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    handle_status_command(const QJsonObject& params);

    // Initialization helpers
    void initialize_themes();
    void initialize_widgets();
    void initialize_actions();

    // Cleanup
    void cleanup_resources();

    // Configuration validation
    bool validate_configuration(const QJsonObject& config) const;

    // Logging
    void log_info(const std::string& message);
    void log_error(const std::string& message);

    // State management
    mutable std::shared_mutex m_state_mutex;
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};

    // Configuration
    mutable std::mutex m_config_mutex;
    QJsonObject m_configuration;
    QString m_default_theme = "default";
    bool m_logging_enabled = true;
    bool m_auto_save_enabled = true;
    int m_refresh_interval = 1000;

    // Widget management
    mutable std::mutex m_widgets_mutex;
    std::unordered_map<QString, QWidget*> m_widgets;
    std::unordered_map<QString, qtplugin::UIWidgetInfo> m_widget_info;

    // Dialog management
    mutable std::mutex m_dialogs_mutex;
    std::unordered_map<QString, QWidget*> m_dialogs;

    // Action management
    mutable std::mutex m_actions_mutex;
    std::unordered_map<QString, std::function<void()>> m_actions;

    // Theme management
    QString m_current_theme = "default";
    std::vector<QString> m_available_themes = {"default", "dark", "light",
                                               "blue", "green"};

    // Dependencies
    std::vector<std::string> m_required_dependencies;
    std::vector<std::string> m_optional_dependencies;
    std::atomic<bool> m_dependencies_satisfied{true};

    // Error handling
    mutable std::mutex m_error_mutex;
    std::vector<std::string> m_error_log;
    std::string m_last_error;

    // Performance metrics
    mutable std::mutex m_metrics_mutex;
    std::chrono::system_clock::time_point m_initialization_time;
    std::atomic<size_t> m_command_count{0};
    std::atomic<size_t> m_widget_count{0};
    std::atomic<size_t> m_action_count{0};
    std::atomic<size_t> m_error_count{0};
};
