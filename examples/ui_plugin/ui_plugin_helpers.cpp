/**
 * @file ui_plugin_helpers.cpp
 * @brief Helper methods and slot implementations for UI plugin
 * @version 3.0.0
 *
 * This file contains helper methods, slot implementations, and command handlers
 * for the UI plugin.
 */

#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include "ui_plugin.hpp"

// === Slot Implementations ===

void UIPlugin::on_widget_data_changed(const QJsonObject& data) {
    log_info("Widget data changed: " +
             QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString());
}

void UIPlugin::on_action_triggered() {
    auto action = qobject_cast<QAction*>(sender());
    if (action) {
        log_info("Action triggered: " + action->text().toStdString());

        // Find action ID and execute callback if available
        std::lock_guard lock(m_actions_mutex);
        for (const auto& [id, stored_action] : m_actions) {
            if (stored_action == action) {
                auto callback_it = m_action_callbacks.find(id);
                if (callback_it != m_action_callbacks.end()) {
                    callback_it->second(id, action->isChecked());
                }
                break;
            }
        }
    }
}

void UIPlugin::on_settings_changed(const QJsonObject& settings) {
    log_info(
        "Settings changed: " +
        QJsonDocument(settings).toJson(QJsonDocument::Compact).toStdString());

    // Apply settings as configuration
    auto result = apply_settings(settings);
    if (!result) {
        log_error("Failed to apply settings: " + result.error().message);
    }
}

// === Helper Methods ===

void UIPlugin::log_error(const std::string& error) {
    {
        std::lock_guard lock(m_error_mutex);
        m_last_error = error;
        m_error_log.push_back(error);

        // Maintain error log size
        if (m_error_log.size() > MAX_ERROR_LOG_SIZE) {
            m_error_log.erase(m_error_log.begin());
        }
    }

    m_error_count.fetch_add(1);

    if (m_logging_enabled) {
        qWarning() << "UIPlugin Error:" << QString::fromStdString(error);
    }
}

void UIPlugin::log_info(const std::string& message) {
    if (m_logging_enabled) {
        qInfo() << "UIPlugin:" << QString::fromStdString(message);
    }
}

void UIPlugin::initialize_themes() {
    std::lock_guard lock(m_theme_mutex);

    m_available_themes.clear();

    // Default theme
    qtplugin::UIThemeInfo default_theme;
    default_theme.name = "default";
    default_theme.description = "Default system theme";
    default_theme.stylesheet = "";
    default_theme.dark_mode = false;
    m_available_themes.push_back(default_theme);

    // Dark theme
    qtplugin::UIThemeInfo dark_theme;
    dark_theme.name = "dark";
    dark_theme.description = "Dark theme with high contrast";
    dark_theme.stylesheet =
        "QWidget { background-color: #2b2b2b; color: #ffffff; }";
    dark_theme.dark_mode = true;
    m_available_themes.push_back(dark_theme);

    // Light theme
    qtplugin::UIThemeInfo light_theme;
    light_theme.name = "light";
    light_theme.description = "Light theme with soft colors";
    light_theme.stylesheet =
        "QWidget { background-color: #ffffff; color: #000000; }";
    light_theme.dark_mode = false;
    m_available_themes.push_back(light_theme);

    // Blue theme
    qtplugin::UIThemeInfo blue_theme;
    blue_theme.name = "blue";
    blue_theme.description = "Blue-themed interface";
    blue_theme.stylesheet =
        "QWidget { background-color: #f0f8ff; color: #000080; }";
    blue_theme.dark_mode = false;
    m_available_themes.push_back(blue_theme);

    // Green theme
    qtplugin::UIThemeInfo green_theme;
    green_theme.name = "green";
    green_theme.description = "Green-themed interface";
    green_theme.stylesheet =
        "QWidget { background-color: #f0fff0; color: #006400; }";
    green_theme.dark_mode = false;
    m_available_themes.push_back(green_theme);

    log_info("Themes initialized: " +
             std::to_string(m_available_themes.size()) + " themes available");
}

void UIPlugin::initialize_widgets() {
    // Widget initialization is done on-demand in create_widget()
    log_info("Widget system initialized");
}

void UIPlugin::initialize_actions() {
    // Create default actions (only if QApplication is available)
    if (!QApplication::instance()) {
        log_info(
            "Skipping action initialization - no QApplication instance "
            "available");
        return;
    }

    try {
        qtplugin::UIActionInfo show_demo_action;
        show_demo_action.id = "show_demo";
        show_demo_action.text = "Show Demo Widget";
        show_demo_action.tooltip = "Show the demo widget with various controls";
        show_demo_action.status_tip = "Display the demo widget";
        show_demo_action.menu_path = "View/Demo";
        show_demo_action.priority = 100;

        auto demo_action_result = create_action(show_demo_action, this);
        if (demo_action_result) {
            set_action_callback(
                "show_demo", [this](const QString& action_id, bool checked) {
                    Q_UNUSED(action_id)
                    Q_UNUSED(checked)
                    auto widget_result = create_widget("demo_widget");
                    if (widget_result) {
                        widget_result.value()->show();
                    }
                });
        }

        qtplugin::UIActionInfo show_settings_action;
        show_settings_action.id = "show_settings";
        show_settings_action.text = "Settings...";
        show_settings_action.tooltip = "Open plugin settings dialog";
        show_settings_action.status_tip = "Configure plugin settings";
        show_settings_action.menu_path = "Tools/Settings";
        show_settings_action.priority = 200;

        auto settings_action_result = create_action(show_settings_action, this);
        if (settings_action_result) {
            set_action_callback("show_settings",
                                [this](const QString& action_id, bool checked) {
                                    Q_UNUSED(action_id)
                                    Q_UNUSED(checked)
                                    auto dialog_result =
                                        create_dialog("settings");
                                    if (dialog_result) {
                                        show_modal_dialog("settings");
                                    }
                                });
        }

        qtplugin::UIActionInfo show_about_action;
        show_about_action.id = "show_about";
        show_about_action.text = "About UI Plugin...";
        show_about_action.tooltip = "Show information about the UI plugin";
        show_about_action.status_tip = "Display plugin information";
        show_about_action.menu_path = "Help/About";
        show_about_action.priority = 300;

        auto about_action_result = create_action(show_about_action, this);
        if (about_action_result) {
            set_action_callback("show_about",
                                [this](const QString& action_id, bool checked) {
                                    Q_UNUSED(action_id)
                                    Q_UNUSED(checked)
                                    auto dialog_result = create_dialog("about");
                                    if (dialog_result) {
                                        show_modal_dialog("about");
                                    }
                                });
        }

        log_info("Default actions initialized");

    } catch (const std::exception& e) {
        log_error("Failed to initialize actions: " + std::string(e.what()));
    }
}

void UIPlugin::cleanup_resources() {
    try {
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

        // Cleanup actions
        {
            std::lock_guard lock(m_actions_mutex);
            for (auto& [id, action] : m_actions) {
                if (action) {
                    action->deleteLater();
                }
            }
            m_actions.clear();
            m_action_info.clear();
            m_action_callbacks.clear();
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

        log_info("Resources cleaned up");

    } catch (const std::exception& e) {
        log_error("Error during resource cleanup: " + std::string(e.what()));
    }
}

// === Command Handlers ===

QJsonObject UIPlugin::handle_widget_command(const QJsonObject& params) {
    QString action = params.value("action").toString();

    if (action == "create") {
        QString widget_id = params.value("widget_id").toString();
        if (widget_id.isEmpty()) {
            return QJsonObject{{"success", false},
                               {"error", "widget_id is required"}};
        }

        auto result = create_widget(widget_id);
        if (result) {
            return QJsonObject{{"success", true},
                               {"widget_id", widget_id},
                               {"widget_created", true}};
        } else {
            return QJsonObject{
                {"success", false},
                {"error", QString::fromStdString(result.error().message)}};
        }
    } else if (action == "list") {
        QJsonArray widgets;
        for (const QString& widget_id : get_available_widgets()) {
            widgets.append(widget_id);
        }
        return QJsonObject{
            {"success", true},
            {"available_widgets", widgets},
            {"active_widgets", static_cast<int>(m_widgets.size())}};
    } else if (action == "destroy") {
        QString widget_id = params.value("widget_id").toString();
        if (widget_id.isEmpty()) {
            return QJsonObject{{"success", false},
                               {"error", "widget_id is required"}};
        }

        auto result = destroy_widget(widget_id);
        return QJsonObject{
            {"success", result.has_value()},
            {"error",
             result ? "" : QString::fromStdString(result.error().message)}};
    } else {
        return QJsonObject{
            {"success", false},
            {"error", "Invalid action. Supported: create, list, destroy"}};
    }
}

QJsonObject UIPlugin::handle_action_command(const QJsonObject& params) {
    QString action = params.value("action").toString();

    if (action == "list") {
        QJsonArray actions;
        auto available_actions = get_available_actions();
        for (const auto& action_info : available_actions) {
            actions.append(QJsonObject{{"id", action_info.id},
                                       {"text", action_info.text},
                                       {"tooltip", action_info.tooltip},
                                       {"menu_path", action_info.menu_path},
                                       {"enabled", action_info.enabled},
                                       {"visible", action_info.visible}});
        }
        return QJsonObject{
            {"success", true},
            {"actions", actions},
            {"action_count", static_cast<int>(m_actions.size())}};
    } else if (action == "trigger") {
        QString action_id = params.value("action_id").toString();
        if (action_id.isEmpty()) {
            return QJsonObject{{"success", false},
                               {"error", "action_id is required"}};
        }

        std::lock_guard lock(m_actions_mutex);
        auto it = m_actions.find(action_id);
        if (it != m_actions.end() && it->second) {
            it->second->trigger();
            return QJsonObject{{"success", true},
                               {"action_id", action_id},
                               {"triggered", true}};
        } else {
            return QJsonObject{{"success", false},
                               {"error", "Action not found: " + action_id}};
        }
    } else {
        return QJsonObject{
            {"success", false},
            {"error", "Invalid action. Supported: list, trigger"}};
    }
}
