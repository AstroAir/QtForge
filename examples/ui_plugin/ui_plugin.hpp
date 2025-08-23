/**
 * @file ui_plugin.hpp
 * @brief Comprehensive UI plugin demonstrating Qt Widgets integration and theme
 * support
 * @version 3.0.0
 *
 * This UI plugin demonstrates advanced QtPlugin system UI capabilities
 * including:
 * - Qt Widgets integration with custom controls
 * - Dialog creation and management
 * - Theme support and customization
 * - Action and menu management
 * - Settings integration
 * - Event handling and callbacks
 */

#pragma once

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDockWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMutex>
#include <QObject>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QReadWriteLock>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <qtplugin/interfaces/ui_plugin_interface.hpp>
#include <qtplugin/qtplugin.hpp>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

/**
 * @brief Custom demo widget showcasing various Qt controls
 */
class DemoWidget : public QWidget {
    Q_OBJECT

public:
    explicit DemoWidget(QWidget* parent = nullptr);
    ~DemoWidget() override;

    void setTheme(const QString& theme_name);
    QJsonObject getWidgetData() const;
    void setWidgetData(const QJsonObject& data);

signals:
    void dataChanged(const QJsonObject& data);
    void actionTriggered(const QString& action);

private slots:
    void on_button_clicked();
    void on_text_changed();
    void on_value_changed();
    void on_selection_changed();

private:
    void setupUI();
    void setupConnections();
    void applyThemeStyles(const QString& theme_name);

    // UI Components
    QVBoxLayout* m_main_layout;
    QTabWidget* m_tab_widget;

    // Basic Controls Tab
    QWidget* m_basic_tab;
    QLineEdit* m_line_edit;
    QTextEdit* m_text_edit;
    QPushButton* m_push_button;
    QCheckBox* m_check_box;
    QRadioButton* m_radio_button1;
    QRadioButton* m_radio_button2;
    QComboBox* m_combo_box;
    QSpinBox* m_spin_box;
    QSlider* m_slider;
    QProgressBar* m_progress_bar;

    // Advanced Controls Tab
    QWidget* m_advanced_tab;
    QTreeWidget* m_tree_widget;
    QListWidget* m_list_widget;
    QTableWidget* m_table_widget;

    // Settings Tab
    QWidget* m_settings_tab;
    QGroupBox* m_theme_group;
    QComboBox* m_theme_combo;
    QGroupBox* m_options_group;

    QString m_current_theme = "default";
    QTimer* m_update_timer;
};

/**
 * @brief Custom settings dialog
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;

    QJsonObject getSettings() const;
    void setSettings(const QJsonObject& settings);

signals:
    void settingsChanged(const QJsonObject& settings);

private slots:
    void on_apply_clicked();
    void on_reset_clicked();

private:
    void setupUI();
    void setupConnections();

    QVBoxLayout* m_main_layout;
    QTabWidget* m_tab_widget;

    // General Settings
    QWidget* m_general_tab;
    QLineEdit* m_name_edit;
    QComboBox* m_theme_combo;
    QCheckBox* m_auto_save_check;
    QSpinBox* m_refresh_interval_spin;

    // Advanced Settings
    QWidget* m_advanced_tab;
    QCheckBox* m_debug_mode_check;
    QCheckBox* m_verbose_logging_check;
    QLineEdit* m_custom_path_edit;

    QPushButton* m_apply_button;
    QPushButton* m_reset_button;
    QPushButton* m_cancel_button;
};

/**
 * @brief About dialog
 */
class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog() override;

private:
    void setupUI();
};

/**
 * @brief Comprehensive UI plugin
 *
 * This plugin demonstrates advanced UI patterns including:
 * - Custom widget creation and management
 * - Dialog system with settings integration
 * - Theme support with multiple themes
 * - Action and menu management
 * - Event handling and callbacks
 * - Qt Widgets best practices
 */
class UIPlugin : public QObject, public qtplugin::IUIPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IUIPlugin/3.0" FILE "ui_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin qtplugin::IUIPlugin)

public:
    explicit UIPlugin(QObject* parent = nullptr);
    ~UIPlugin() override;

    // === IPlugin Interface ===

    // Metadata
    std::string_view name() const noexcept override {
        return "UI Example Plugin";
    }

    std::string_view description() const noexcept override {
        return "A comprehensive UI plugin demonstrating Qt Widgets integration "
               "and theme support";
    }

    qtplugin::Version version() const noexcept override { return {1, 0, 0}; }

    std::string_view author() const noexcept override {
        return "QtPlugin Development Team";
    }

    std::string id() const noexcept override { return "com.example.ui_plugin"; }

    std::string_view category() const noexcept override { return "UI"; }

    std::string_view license() const noexcept override { return "MIT"; }

    std::string_view homepage() const noexcept override {
        return "https://github.com/example/qtplugin";
    }

    // === Lifecycle Management ===
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override { return m_state; }
    bool is_initialized() const noexcept override;

    qtplugin::expected<void, qtplugin::PluginError> pause() override;
    qtplugin::expected<void, qtplugin::PluginError> resume() override;
    qtplugin::expected<void, qtplugin::PluginError> restart() override;

    // === Capabilities ===
    qtplugin::PluginCapabilities capabilities() const noexcept override {
        return qtplugin::PluginCapability::UI |
               qtplugin::PluginCapability::Configuration |
               qtplugin::PluginCapability::Logging |
               qtplugin::PluginCapability::Monitoring;
    }

    qtplugin::PluginPriority priority() const noexcept override {
        return qtplugin::PluginPriority::Normal;
    }

    // === Configuration ===
    std::optional<QJsonObject> default_configuration() const override;
    qtplugin::expected<void, qtplugin::PluginError> configure(
        const QJsonObject& config) override;
    QJsonObject current_configuration() const override;
    bool validate_configuration(const QJsonObject& config) const override;

    // === Commands ===
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override;

    std::vector<std::string> available_commands() const override;

    // === Dependencies ===
    std::vector<std::string> dependencies() const override;
    std::vector<std::string> optional_dependencies() const override;
    bool dependencies_satisfied() const override;

    // === Error Handling ===
    std::string last_error() const override { return m_last_error; }
    std::vector<std::string> error_log() const override { return m_error_log; }
    void clear_errors() override;

    // === Monitoring ===
    std::chrono::milliseconds uptime() const override;
    QJsonObject performance_metrics() const override;
    QJsonObject resource_usage() const override;

    // === Threading ===
    bool is_thread_safe() const noexcept override { return true; }
    std::string_view thread_model() const noexcept override {
        return "main-thread";
    }

    // === IUIPlugin Interface ===

    // Component Support
    qtplugin::UIComponentTypes supported_components() const noexcept override;
    std::vector<qtplugin::UIIntegrationPoint> supported_integration_points()
        const override;

    // Widget Management
    qtplugin::expected<QWidget*, qtplugin::PluginError> create_widget(
        const QString& widget_id, QWidget* parent = nullptr) override;
    qtplugin::expected<qtplugin::UIWidgetInfo, qtplugin::PluginError>
    get_widget_info(const QString& widget_id) const override;
    std::vector<QString> get_available_widgets() const override;
    qtplugin::expected<void, qtplugin::PluginError> destroy_widget(
        const QString& widget_id) override;

    // Action Management
    qtplugin::expected<QAction*, qtplugin::PluginError> create_action(
        const qtplugin::UIActionInfo& action_info,
        QObject* parent = nullptr) override;
    std::vector<qtplugin::UIActionInfo> get_available_actions() const override;
    qtplugin::expected<void, qtplugin::PluginError> set_action_callback(
        const QString& action_id, qtplugin::UIActionCallback callback) override;
    qtplugin::expected<void, qtplugin::PluginError> remove_action(
        const QString& action_id) override;

    // Menu and Toolbar Support
    qtplugin::expected<QMenu*, qtplugin::PluginError> create_menu(
        const QString& menu_id, const QString& title,
        QWidget* parent = nullptr) override;
    qtplugin::expected<QToolBar*, qtplugin::PluginError> create_toolbar(
        const QString& toolbar_id, const QString& title,
        QWidget* parent = nullptr) override;

    // Dialog Support
    qtplugin::expected<QDialog*, qtplugin::PluginError> create_dialog(
        const QString& dialog_id, QWidget* parent = nullptr) override;
    qtplugin::expected<int, qtplugin::PluginError> show_modal_dialog(
        const QString& dialog_id) override;

    // Theme Support
    std::vector<qtplugin::UIThemeInfo> get_available_themes() const override;
    qtplugin::expected<void, qtplugin::PluginError> apply_theme(
        const QString& theme_name) override;
    QString get_current_theme() const override;

    // Settings Integration
    qtplugin::expected<QWidget*, qtplugin::PluginError> create_settings_widget(
        QWidget* parent = nullptr) override;
    qtplugin::expected<void, qtplugin::PluginError> apply_settings(
        const QJsonObject& settings) override;
    QJsonObject get_current_settings() const override;

private slots:
    void on_widget_data_changed(const QJsonObject& data);
    void on_action_triggered();
    void on_settings_changed(const QJsonObject& settings);

private:
    // === State Management ===
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    std::chrono::system_clock::time_point m_initialization_time;
    mutable std::shared_mutex m_state_mutex;

    // === Configuration ===
    QJsonObject m_configuration;
    mutable std::mutex m_config_mutex;
    QString m_default_theme = "default";
    bool m_logging_enabled = true;
    bool m_auto_save_enabled = true;
    int m_refresh_interval = 1000;

    // === Widget Management ===
    std::unordered_map<QString, QWidget*> m_widgets;
    std::unordered_map<QString, qtplugin::UIWidgetInfo> m_widget_info;
    mutable std::mutex m_widgets_mutex;

    // === Action Management ===
    std::unordered_map<QString, QAction*> m_actions;
    std::unordered_map<QString, qtplugin::UIActionInfo> m_action_info;
    std::unordered_map<QString, qtplugin::UIActionCallback> m_action_callbacks;
    mutable std::mutex m_actions_mutex;

    // === Dialog Management ===
    std::unordered_map<QString, QDialog*> m_dialogs;
    mutable std::mutex m_dialogs_mutex;

    // === Theme Management ===
    QString m_current_theme = "default";
    std::vector<qtplugin::UIThemeInfo> m_available_themes;
    mutable std::mutex m_theme_mutex;

    // === Error Handling ===
    mutable std::string m_last_error;
    mutable std::vector<std::string> m_error_log;
    mutable std::mutex m_error_mutex;
    static constexpr size_t MAX_ERROR_LOG_SIZE = 100;

    // === Monitoring ===
    std::atomic<uint64_t> m_command_count{0};
    std::atomic<uint64_t> m_widget_count{0};
    std::atomic<uint64_t> m_action_count{0};
    std::atomic<uint64_t> m_error_count{0};
    mutable std::mutex m_metrics_mutex;

    // === Dependencies ===
    std::vector<std::string> m_required_dependencies;
    std::vector<std::string> m_optional_dependencies;
    std::atomic<bool> m_dependencies_satisfied{true};

    // === Helper Methods ===
    void log_error(const std::string& error);
    void log_info(const std::string& message);
    void initialize_themes();
    void initialize_widgets();
    void initialize_actions();
    void cleanup_resources();

    // === Command Handlers ===
    QJsonObject handle_widget_command(const QJsonObject& params);
    QJsonObject handle_action_command(const QJsonObject& params);
    QJsonObject handle_dialog_command(const QJsonObject& params);
    QJsonObject handle_theme_command(const QJsonObject& params);
    QJsonObject handle_settings_command(const QJsonObject& params);
    QJsonObject handle_status_command(const QJsonObject& params);
};
