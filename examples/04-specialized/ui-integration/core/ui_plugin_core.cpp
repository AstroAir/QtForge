/**
 * @file ui_plugin_core.cpp
 * @brief Core UI plugin implementation
 * @version 3.0.0
 */

#include "ui_plugin_core.hpp"
#include "../dialogs/about_dialog.hpp"
#include "../dialogs/settings_dialog.hpp"
#include "../widgets/demo_widget.hpp"

#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <chrono>
#include <thread>

UIPluginCore::UIPluginCore(QObject* parent) : QObject(parent) {
    // Initialize dependencies
    m_required_dependencies = {};  // No required dependencies for UI plugin
    m_optional_dependencies = {"qtplugin.ConfigurationManager",
                               "qtplugin.ThemeManager"};

    log_info("UIPluginCore constructed");
}

UIPluginCore::~UIPluginCore() {
    shutdown();
    log_info("UIPluginCore destroyed");
}

// IPlugin interface - Metadata implementations
std::string_view UIPluginCore::name() const noexcept {
    return "UI Plugin Core";
}

std::string_view UIPluginCore::description() const noexcept {
    return "Core UI plugin providing widget-based user interface components";
}

qtplugin::Version UIPluginCore::version() const noexcept { return {3, 0, 0}; }

std::string_view UIPluginCore::author() const noexcept {
    return "QtForge Development Team";
}

std::string UIPluginCore::id() const noexcept { return "qtforge.ui.core"; }

qtplugin::PluginState UIPluginCore::state() const noexcept {
    return is_initialized() ? qtplugin::PluginState::Running
                            : qtplugin::PluginState::Stopped;
}

qtplugin::PluginCapabilities UIPluginCore::capabilities() const noexcept {
    return qtplugin::PluginCapability::UI |
           qtplugin::PluginCapability::Configuration;
}

qtplugin::expected<void, qtplugin::PluginError> UIPluginCore::initialize() {
    std::unique_lock lock(m_state_mutex);

    if (m_state != qtplugin::PluginState::Unloaded) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin is already initialized");
    }

    try {
        m_state = qtplugin::PluginState::Initializing;
        m_initialization_time = std::chrono::system_clock::now();
        lock.unlock();

        log_info("Initializing UIPluginCore...");

        // Initialize themes
        initialize_themes();

        // Initialize widgets
        initialize_widgets();

        // Initialize actions
        initialize_actions();

        lock.lock();
        m_state = qtplugin::PluginState::Running;
        lock.unlock();

        log_info("UIPluginCore initialized successfully");

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to initialize UIPluginCore: " + std::string(e.what());
        log_error(error_msg);

        lock.lock();
        m_state = qtplugin::PluginState::Error;
        lock.unlock();

        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed, error_msg);
    }
}

void UIPluginCore::shutdown() noexcept {
    try {
        std::unique_lock lock(m_state_mutex);
        m_state = qtplugin::PluginState::Stopping;
        lock.unlock();

        log_info("Shutting down UIPluginCore...");

        // Cleanup resources
        cleanup_resources();

        lock.lock();
        m_state = qtplugin::PluginState::Stopped;
        lock.unlock();

        log_info("UIPluginCore shutdown completed");

    } catch (...) {
        // Shutdown must not throw
        std::unique_lock lock(m_state_mutex);
        m_state = qtplugin::PluginState::Error;
    }
}

bool UIPluginCore::is_initialized() const noexcept {
    std::shared_lock lock(m_state_mutex);
    auto current_state = m_state.load();
    return current_state == qtplugin::PluginState::Running ||
           current_state == qtplugin::PluginState::Paused;
}

qtplugin::expected<void, qtplugin::PluginError> UIPluginCore::pause() {
    std::unique_lock lock(m_state_mutex);

    if (m_state != qtplugin::PluginState::Running) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be running to pause");
    }

    try {
        // Disable all widgets
        std::lock_guard widgets_lock(m_widgets_mutex);
        for (auto& [id, widget] : m_widgets) {
            if (widget) {
                widget->setEnabled(false);
            }
        }

        m_state = qtplugin::PluginState::Paused;
        log_info("UIPluginCore paused successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to pause UIPluginCore: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> UIPluginCore::resume() {
    std::unique_lock lock(m_state_mutex);

    if (m_state != qtplugin::PluginState::Paused) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be paused to resume");
    }

    try {
        // Re-enable all widgets
        std::lock_guard widgets_lock(m_widgets_mutex);
        for (auto& [id, widget] : m_widgets) {
            if (widget) {
                widget->setEnabled(true);
            }
        }

        m_state = qtplugin::PluginState::Running;
        log_info("UIPluginCore resumed successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to resume UIPluginCore: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> UIPluginCore::restart() {
    log_info("Restarting UIPluginCore...");

    // Shutdown first
    shutdown();

    // Wait a brief moment for cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Initialize again
    return initialize();
}

std::optional<QJsonObject> UIPluginCore::default_configuration() const {
    return QJsonObject{
        {"default_theme", "default"}, {"logging_enabled", true},
        {"auto_save_enabled", true},  {"refresh_interval", 1000},
        {"show_tooltips", true},      {"enable_animations", true},
        {"window_opacity", 1.0}};
}

qtplugin::expected<void, qtplugin::PluginError> UIPluginCore::configure(
    const QJsonObject& config) {
    if (!validate_configuration(config)) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError,
            "Invalid configuration provided");
    }

    try {
        std::lock_guard lock(m_config_mutex);

        // Update configuration
        m_configuration = config;

        // Apply configuration
        m_default_theme = config.value("default_theme").toString("default");
        m_logging_enabled = config.value("logging_enabled").toBool(true);
        m_auto_save_enabled = config.value("auto_save_enabled").toBool(true);
        m_refresh_interval = config.value("refresh_interval").toInt(1000);

        // Apply theme if changed
        if (config.contains("default_theme")) {
            QString theme = config.value("default_theme").toString();
            if (theme != m_current_theme) {
                auto theme_result = apply_theme(theme);
                if (!theme_result) {
                    log_error("Failed to apply theme: " +
                              theme_result.error().message);
                }
            }
        }

        log_info("UIPluginCore configured successfully");

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to configure UIPluginCore: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError, error_msg);
    }
}

QJsonObject UIPluginCore::current_configuration() const {
    std::lock_guard lock(m_config_mutex);
    return m_configuration;
}

bool UIPluginCore::validate_configuration(const QJsonObject& config) const {
    // Validate refresh interval
    if (config.contains("refresh_interval")) {
        int interval = config.value("refresh_interval").toInt(-1);
        if (interval < 100 || interval > 10000) {
            return false;
        }
    }

    // Validate window opacity
    if (config.contains("window_opacity")) {
        double opacity = config.value("window_opacity").toDouble(-1.0);
        if (opacity < 0.0 || opacity > 1.0) {
            return false;
        }
    }

    // Validate theme
    if (config.contains("default_theme")) {
        QString theme = config.value("default_theme").toString();
        if (theme.isEmpty()) {
            return false;
        }
    }

    return true;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
UIPluginCore::execute_command(std::string_view command,
                              const QJsonObject& params) {
    m_command_count.fetch_add(1);

    if (command == "widget") {
        return handle_widget_command(params);
    } else if (command == "action") {
        return handle_action_command(params);
    } else if (command == "dialog") {
        return handle_dialog_command(params);
    } else if (command == "theme") {
        return handle_theme_command(params);
    } else if (command == "settings") {
        return handle_settings_command(params);
    } else if (command == "status") {
        return handle_status_command(params);
    } else {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound,
            "Unknown command: " + std::string(command));
    }
}

std::vector<std::string> UIPluginCore::available_commands() const {
    return {"widget", "action", "dialog", "theme", "settings", "status"};
}

std::vector<std::string> UIPluginCore::dependencies() const {
    return m_required_dependencies;
}

std::vector<std::string> UIPluginCore::optional_dependencies() const {
    return m_optional_dependencies;
}

bool UIPluginCore::dependencies_satisfied() const {
    return m_dependencies_satisfied.load();
}

void UIPluginCore::clear_errors() {
    std::lock_guard lock(m_error_mutex);
    m_error_log.clear();
    m_last_error.clear();
}

std::chrono::milliseconds UIPluginCore::uptime() const {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_initialization_time);
    return duration;
}

void UIPluginCore::log_info(const std::string& message) {
    if (m_logging_enabled) {
        qDebug() << QString::fromStdString("[UIPluginCore] " + message);
    }
}

void UIPluginCore::log_error(const std::string& message) {
    std::lock_guard lock(m_error_mutex);
    m_error_log.push_back(message);
    m_last_error = message;
    m_error_count.fetch_add(1);

    qDebug() << QString::fromStdString("[UIPluginCore ERROR] " + message);
}

QJsonObject UIPluginCore::performance_metrics() const {
    std::lock_guard lock(m_metrics_mutex);

    auto current_uptime = uptime();
    auto commands_per_second =
        current_uptime.count() > 0
            ? (m_command_count.load() * 1000.0) / current_uptime.count()
            : 0.0;

    return QJsonObject{
        {"uptime_ms", static_cast<qint64>(current_uptime.count())},
        {"command_count", static_cast<qint64>(m_command_count.load())},
        {"widget_count", static_cast<qint64>(m_widget_count.load())},
        {"action_count", static_cast<qint64>(m_action_count.load())},
        {"error_count", static_cast<qint64>(m_error_count.load())},
        {"commands_per_second", commands_per_second},
        {"state", static_cast<int>(m_state.load())},
        {"current_theme", m_current_theme},
        {"available_themes", static_cast<int>(m_available_themes.size())},
        {"active_widgets", static_cast<int>(m_widgets.size())},
        {"active_actions", static_cast<int>(m_actions.size())},
        {"active_dialogs", static_cast<int>(m_dialogs.size())}};
}

QJsonObject UIPluginCore::resource_usage() const {
    std::lock_guard lock(m_metrics_mutex);

    // Enhanced resource usage tracking for UI plugin
    auto memory_estimate = 2048 + (m_widgets.size() * 100) +
                           (m_actions.size() * 50) + (m_error_log.size() * 50);
    auto cpu_estimate = 0.2;  // UI plugins typically use minimal CPU when idle

    return QJsonObject{
        {"estimated_memory_kb", static_cast<qint64>(memory_estimate)},
        {"estimated_cpu_percent", cpu_estimate},
        {"thread_count", 1},  // UI plugins run on main thread
        {"widget_count", static_cast<qint64>(m_widgets.size())},
        {"action_count", static_cast<qint64>(m_actions.size())},
        {"dialog_count", static_cast<qint64>(m_dialogs.size())},
        {"theme_count", static_cast<qint64>(m_available_themes.size())},
        {"error_log_size", static_cast<qint64>(m_error_log.size())},
        {"dependencies_satisfied", dependencies_satisfied()}};
}

qtplugin::UIComponentTypes UIPluginCore::supported_components() const noexcept {
    return static_cast<qtplugin::UIComponentTypes>(
        static_cast<uint32_t>(qtplugin::UIComponentType::Widget) |
        static_cast<uint32_t>(qtplugin::UIComponentType::Dialog) |
        static_cast<uint32_t>(qtplugin::UIComponentType::ToolBar) |
        static_cast<uint32_t>(qtplugin::UIComponentType::MenuBar) |
        static_cast<uint32_t>(qtplugin::UIComponentType::ContextMenu) |
        static_cast<uint32_t>(qtplugin::UIComponentType::Settings) |
        static_cast<uint32_t>(qtplugin::UIComponentType::PropertyEditor) |
        static_cast<uint32_t>(qtplugin::UIComponentType::TreeView) |
        static_cast<uint32_t>(qtplugin::UIComponentType::ListView) |
        static_cast<uint32_t>(qtplugin::UIComponentType::TableView));
}

std::vector<qtplugin::UIIntegrationPoint>
UIPluginCore::supported_integration_points() const {
    return {qtplugin::UIIntegrationPoint::MainWindow,
            qtplugin::UIIntegrationPoint::MenuBar,
            qtplugin::UIIntegrationPoint::ToolBar,
            qtplugin::UIIntegrationPoint::DockArea,
            qtplugin::UIIntegrationPoint::CentralWidget,
            qtplugin::UIIntegrationPoint::ContextMenu,
            qtplugin::UIIntegrationPoint::SettingsDialog};
}

qtplugin::expected<QWidget*, qtplugin::PluginError> UIPluginCore::create_widget(
    const QString& widget_id, QWidget* parent) {
    try {
        std::lock_guard lock(m_widgets_mutex);

        // Check if widget already exists
        if (m_widgets.find(widget_id) != m_widgets.end()) {
            return qtplugin::make_error<QWidget*>(
                qtplugin::PluginErrorCode::AlreadyExists,
                "Widget with ID '" + widget_id.toStdString() +
                    "' already exists");
        }

        QWidget* widget = nullptr;

        if (widget_id == "demo_widget") {
            auto demo_widget = new DemoWidget(parent);
            connect(demo_widget, &DemoWidget::dataChanged, this,
                    &UIPluginCore::on_widget_data_changed);
            connect(demo_widget, &DemoWidget::actionTriggered, this,
                    &UIPluginCore::on_action_triggered);
            widget = demo_widget;
        } else if (widget_id == "settings_widget") {
            auto settings_dialog = new SettingsDialog(parent);
            connect(settings_dialog, &SettingsDialog::settingsChanged, this,
                    &UIPluginCore::on_settings_changed);
            widget = settings_dialog;
        } else {
            return qtplugin::make_error<QWidget*>(
                qtplugin::PluginErrorCode::NotFound,
                "Unknown widget ID: " + widget_id.toStdString());
        }

        if (widget) {
            m_widgets[widget_id] = widget;
            m_widget_count.fetch_add(1);

            // Store widget info
            qtplugin::UIWidgetInfo info;
            info.id = widget_id;
            info.title = widget->windowTitle();
            info.type = qtplugin::UIComponentType::Widget;
            info.integration_point =
                qtplugin::UIIntegrationPoint::CentralWidget;
            info.resizable = true;
            info.closable = true;
            info.floatable = true;
            m_widget_info[widget_id] = info;

            log_info("Widget created: " + widget_id.toStdString());
            emit widget_created(widget_id);
        }

        return widget;

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to create widget '" +
                                widget_id.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<QWidget*>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<qtplugin::UIWidgetInfo, qtplugin::PluginError>
UIPluginCore::get_widget_info(const QString& widget_id) const {
    std::lock_guard lock(m_widgets_mutex);

    auto it = m_widget_info.find(widget_id);
    if (it == m_widget_info.end()) {
        return qtplugin::make_error<qtplugin::UIWidgetInfo>(
            qtplugin::PluginErrorCode::NotFound,
            "Widget not found: " + widget_id.toStdString());
    }

    return it->second;
}

std::vector<QString> UIPluginCore::get_available_widgets() const {
    return {"demo_widget", "settings_widget"};
}

qtplugin::expected<void, qtplugin::PluginError> UIPluginCore::destroy_widget(
    const QString& widget_id) {
    try {
        std::lock_guard lock(m_widgets_mutex);

        auto it = m_widgets.find(widget_id);
        if (it == m_widgets.end()) {
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::NotFound,
                "Widget not found: " + widget_id.toStdString());
        }

        QWidget* widget = it->second;
        if (widget) {
            widget->deleteLater();
            m_widget_count.fetch_sub(1);
        }

        m_widgets.erase(it);
        m_widget_info.erase(widget_id);

        log_info("Widget destroyed: " + widget_id.toStdString());
        emit widget_destroyed(widget_id);

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to destroy widget '" +
                                widget_id.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

// Theme management
qtplugin::expected<void, qtplugin::PluginError> UIPluginCore::apply_theme(
    const QString& theme_name) {
    try {
        if (std::find(m_available_themes.begin(), m_available_themes.end(),
                      theme_name) == m_available_themes.end()) {
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::NotFound,
                "Theme not found: " + theme_name.toStdString());
        }

        m_current_theme = theme_name;

        // Apply theme to all widgets
        std::lock_guard lock(m_widgets_mutex);
        for (auto& [id, widget] : m_widgets) {
            if (auto demo_widget = qobject_cast<DemoWidget*>(widget)) {
                demo_widget->setTheme(theme_name);
            }
        }

        log_info("Theme applied: " + theme_name.toStdString());
        emit theme_changed(theme_name);

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to apply theme '" +
                                theme_name.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

std::vector<qtplugin::UIThemeInfo> UIPluginCore::get_available_themes() const {
    std::vector<qtplugin::UIThemeInfo> themes;

    for (const QString& theme_name : m_available_themes) {
        qtplugin::UIThemeInfo theme_info;
        theme_info.name = theme_name;
        theme_info.description = QString("Theme: %1").arg(theme_name);
        theme_info.dark_mode = (theme_name == QLatin1String("dark"));
        themes.push_back(theme_info);
    }

    return themes;
}

QString UIPluginCore::get_current_theme() const { return m_current_theme; }

// IUIPlugin interface - Action Management implementations
qtplugin::expected<QAction*, qtplugin::PluginError> UIPluginCore::create_action(
    const qtplugin::UIActionInfo& action_info, QObject* parent) {
    Q_UNUSED(action_info)
    Q_UNUSED(parent)
    return qtplugin::make_error<QAction*>(
        qtplugin::PluginErrorCode::CommandNotFound,
        "Action creation not implemented");
}

std::vector<qtplugin::UIActionInfo> UIPluginCore::get_available_actions()
    const {
    return {};  // Return empty vector for now
}

qtplugin::expected<void, qtplugin::PluginError>
UIPluginCore::set_action_callback(const QString& action_id,
                                  qtplugin::UIActionCallback callback) {
    Q_UNUSED(action_id)
    Q_UNUSED(callback)
    return qtplugin::make_error<void>(
        qtplugin::PluginErrorCode::CommandNotFound,
        "Action callback setting not implemented");
}

qtplugin::expected<void, qtplugin::PluginError> UIPluginCore::remove_action(
    const QString& action_id) {
    Q_UNUSED(action_id)
    return qtplugin::make_error<void>(
        qtplugin::PluginErrorCode::CommandNotFound,
        "Action removal not implemented");
}

// IUIPlugin interface - Menu and Toolbar Support implementations
qtplugin::expected<QMenu*, qtplugin::PluginError> UIPluginCore::create_menu(
    const QString& menu_id, const QString& title, QWidget* parent) {
    Q_UNUSED(menu_id)
    Q_UNUSED(title)
    Q_UNUSED(parent)
    return qtplugin::make_error<QMenu*>(
        qtplugin::PluginErrorCode::CommandNotFound,
        "Menu creation not implemented");
}

qtplugin::expected<QToolBar*, qtplugin::PluginError>
UIPluginCore::create_toolbar(const QString& toolbar_id, const QString& title,
                             QWidget* parent) {
    Q_UNUSED(toolbar_id)
    Q_UNUSED(title)
    Q_UNUSED(parent)
    return qtplugin::make_error<QToolBar*>(
        qtplugin::PluginErrorCode::CommandNotFound,
        "Toolbar creation not implemented");
}

qtplugin::expected<QDialog*, qtplugin::PluginError> UIPluginCore::create_dialog(
    const QString& dialog_id, QWidget* parent) {
    Q_UNUSED(dialog_id)
    Q_UNUSED(parent)
    return qtplugin::make_error<QDialog*>(
        qtplugin::PluginErrorCode::CommandNotFound,
        "Dialog creation not implemented");
}

qtplugin::expected<int, qtplugin::PluginError> UIPluginCore::show_modal_dialog(
    const QString& dialog_id) {
    Q_UNUSED(dialog_id)
    return qtplugin::make_error<int>(qtplugin::PluginErrorCode::CommandNotFound,
                                     "Modal dialog showing not implemented");
}

// Slot implementations
void UIPluginCore::on_widget_data_changed(const QJsonObject& data) {
    Q_UNUSED(data)
    log_info("Widget data changed");
    // Handle widget data changes if needed
}

void UIPluginCore::on_action_triggered(const QString& action) {
    m_action_count.fetch_add(1);
    log_info("Action triggered: " + action.toStdString());

    // Handle specific actions
    if (action.startsWith("theme_changed:")) {
        QString theme = action.mid(14);  // Remove "theme_changed:" prefix
        apply_theme(theme);
    }
}

void UIPluginCore::on_settings_changed(const QJsonObject& settings) {
    log_info("Settings changed");

    // Apply new settings
    if (settings.contains("theme")) {
        QString theme = settings.value("theme").toString();
        apply_theme(theme);
    }
}

// Command handlers (simplified implementations)
qtplugin::expected<QJsonObject, qtplugin::PluginError>
UIPluginCore::handle_widget_command(const QJsonObject& params) {
    QString action = params.value(QLatin1String("action")).toString();
    QString widget_id = params.value(QLatin1String("widget_id")).toString();

    if (action == QLatin1String("create")) {
        auto result = create_widget(widget_id);
        if (result) {
            return QJsonObject{{QLatin1String("success"), true},
                               {QLatin1String("widget_id"), widget_id}};
        } else {
            return qtplugin::make_error<QJsonObject>(result.error().code,
                                                     result.error().message);
        }
    } else if (action == QLatin1String("destroy")) {
        auto result = destroy_widget(widget_id);
        if (result) {
            return QJsonObject{{QLatin1String("success"), true},
                               {QLatin1String("widget_id"), widget_id}};
        } else {
            return qtplugin::make_error<QJsonObject>(result.error().code,
                                                     result.error().message);
        }
    } else if (action == QLatin1String("list")) {
        auto widgets = get_available_widgets();
        QJsonArray widget_array;
        for (const auto& widget : widgets) {
            widget_array.append(widget);
        }
        return QJsonObject{{QLatin1String("widgets"), widget_array}};
    }

    return qtplugin::make_error<QJsonObject>(
        qtplugin::PluginErrorCode::InvalidParameters, "Unknown widget action");
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
UIPluginCore::handle_action_command(const QJsonObject& params) {
    QString action = params.value(QLatin1String("action")).toString();

    if (action == QLatin1String("trigger")) {
        QString action_name =
            params.value(QLatin1String("action_name")).toString();
        on_action_triggered(action_name);
        return QJsonObject{{QLatin1String("success"), true},
                           {QLatin1String("action"), action_name}};
    }

    return qtplugin::make_error<QJsonObject>(
        qtplugin::PluginErrorCode::InvalidParameters, "Unknown action command");
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
UIPluginCore::handle_dialog_command(const QJsonObject& params) {
    QString dialog_type = params.value(QLatin1String("type")).toString();

    if (dialog_type == QLatin1String("about")) {
        auto about_dialog = new AboutDialog();
        about_dialog->show();
        return QJsonObject{{QLatin1String("success"), true},
                           {QLatin1String("dialog"), QLatin1String("about")}};
    } else if (dialog_type == QLatin1String("settings")) {
        auto settings_dialog = new SettingsDialog();
        connect(settings_dialog, &SettingsDialog::settingsChanged, this,
                &UIPluginCore::on_settings_changed);
        settings_dialog->show();
        return QJsonObject{
            {QLatin1String("success"), true},
            {QLatin1String("dialog"), QLatin1String("settings")}};
    }

    return qtplugin::make_error<QJsonObject>(
        qtplugin::PluginErrorCode::InvalidParameters, "Unknown dialog type");
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
UIPluginCore::handle_theme_command(const QJsonObject& params) {
    QString action = params.value(QLatin1String("action")).toString();

    if (action == QLatin1String("apply")) {
        QString theme = params.value(QLatin1String("theme")).toString();
        auto result = apply_theme(theme);
        if (result) {
            return QJsonObject{{QLatin1String("success"), true},
                               {QLatin1String("theme"), theme}};
        } else {
            return qtplugin::make_error<QJsonObject>(result.error().code,
                                                     result.error().message);
        }
    } else if (action == QLatin1String("list")) {
        auto themes = get_available_themes();
        QJsonArray theme_array;
        for (const auto& theme : themes) {
            theme_array.append(theme.name);
        }
        return QJsonObject{{QLatin1String("themes"), theme_array},
                           {QLatin1String("current"), get_current_theme()}};
    }

    return qtplugin::make_error<QJsonObject>(
        qtplugin::PluginErrorCode::InvalidParameters, "Unknown theme action");
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
UIPluginCore::handle_settings_command(const QJsonObject& params) {
    QString action = params.value(QLatin1String("action")).toString();

    if (action == QLatin1String("get")) {
        return current_configuration();
    } else if (action == QLatin1String("set")) {
        QJsonObject config = params.value(QLatin1String("config")).toObject();
        auto result = configure(config);
        if (result) {
            return QJsonObject{{QLatin1String("success"), true}};
        } else {
            return qtplugin::make_error<QJsonObject>(result.error().code,
                                                     result.error().message);
        }
    }

    return qtplugin::make_error<QJsonObject>(
        qtplugin::PluginErrorCode::InvalidParameters,
        "Unknown settings action");
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
UIPluginCore::handle_status_command(const QJsonObject& params) {
    Q_UNUSED(params)

    QJsonObject status;
    status["initialized"] = is_initialized();
    status["state"] = static_cast<int>(m_state.load());
    status["uptime_ms"] = static_cast<qint64>(uptime().count());
    status["performance"] = performance_metrics();
    status["resources"] = resource_usage();

    return status;
}

// Initialization helpers
void UIPluginCore::initialize_themes() {
    log_info("Initializing themes...");
    // Themes are already initialized in constructor
}

void UIPluginCore::initialize_widgets() {
    log_info("Initializing widget system...");
    // Widget system is ready for on-demand creation
}

void UIPluginCore::initialize_actions() {
    log_info("Initializing actions...");
    // Actions are handled dynamically
}

void UIPluginCore::cleanup_resources() {
    log_info("Cleaning up resources...");

    // Cleanup widgets
    {
        std::lock_guard lock(m_widgets_mutex);
        for (auto& [id, widget] : m_widgets) {
            if (widget) {
                widget->deleteLater();
            }
        }
        m_widgets.clear();
        m_widget_info.clear();
    }

    // Cleanup dialogs
    {
        std::lock_guard lock(m_dialogs_mutex);
        for (auto& [id, dialog] : m_dialogs) {
            if (dialog) {
                dialog->deleteLater();
            }
        }
        m_dialogs.clear();
    }

    // Cleanup actions
    {
        std::lock_guard lock(m_actions_mutex);
        m_actions.clear();
    }
}
