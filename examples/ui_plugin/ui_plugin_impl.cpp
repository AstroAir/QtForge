/**
 * @file ui_plugin_impl.cpp
 * @brief Additional implementation methods for UI plugin
 * @version 3.0.0
 *
 * This file contains the remaining implementation methods for the UI plugin
 * including action management, dialog support, theme support, and helper
 * methods.
 */

#include <QDebug>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>
#include <QPixmap>
#include "ui_plugin.hpp"

// === Action Management ===

qtplugin::expected<QAction*, qtplugin::PluginError> UIPlugin::create_action(
    const qtplugin::UIActionInfo& action_info, QObject* parent) {
    try {
        std::lock_guard lock(m_actions_mutex);

        // Check if action already exists
        if (m_actions.find(action_info.id) != m_actions.end()) {
            return qtplugin::make_error<QAction*>(
                qtplugin::PluginErrorCode::AlreadyExists,
                "Action with ID '" + action_info.id.toStdString() +
                    "' already exists");
        }

        auto action = new QAction(parent);
        action->setText(action_info.text);
        action->setToolTip(action_info.tooltip);
        action->setStatusTip(action_info.status_tip);
        action->setIcon(action_info.icon);
        action->setShortcut(action_info.shortcut);
        action->setCheckable(action_info.checkable);
        action->setChecked(action_info.checked);
        action->setEnabled(action_info.enabled);
        action->setVisible(action_info.visible);

        // Connect action triggered signal
        connect(action, &QAction::triggered, this,
                &UIPlugin::on_action_triggered);

        m_actions[action_info.id] = action;
        m_action_info[action_info.id] = action_info;
        m_action_count.fetch_add(1);

        log_info("Action created: " + action_info.id.toStdString());

        return action;

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to create action '" +
                                action_info.id.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<QAction*>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

std::vector<qtplugin::UIActionInfo> UIPlugin::get_available_actions() const {
    std::lock_guard lock(m_actions_mutex);

    std::vector<qtplugin::UIActionInfo> actions;
    for (const auto& [id, info] : m_action_info) {
        actions.push_back(info);
    }

    return actions;
}

qtplugin::expected<void, qtplugin::PluginError> UIPlugin::set_action_callback(
    const QString& action_id, qtplugin::UIActionCallback callback) {
    try {
        std::lock_guard lock(m_actions_mutex);

        auto it = m_actions.find(action_id);
        if (it == m_actions.end()) {
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::NotFound,
                "Action not found: " + action_id.toStdString());
        }

        m_action_callbacks[action_id] = callback;

        log_info("Action callback set: " + action_id.toStdString());

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to set action callback '" +
                                action_id.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> UIPlugin::remove_action(
    const QString& action_id) {
    try {
        std::lock_guard lock(m_actions_mutex);

        auto it = m_actions.find(action_id);
        if (it == m_actions.end()) {
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::NotFound,
                "Action not found: " + action_id.toStdString());
        }

        QAction* action = it->second;
        if (action) {
            action->deleteLater();
            m_action_count.fetch_sub(1);
        }

        m_actions.erase(it);
        m_action_info.erase(action_id);
        m_action_callbacks.erase(action_id);

        log_info("Action removed: " + action_id.toStdString());

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to remove action '" +
                                action_id.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

// === Menu and Toolbar Support ===

qtplugin::expected<QMenu*, qtplugin::PluginError> UIPlugin::create_menu(
    const QString& menu_id, const QString& title, QWidget* parent) {
    try {
        auto menu = new QMenu(title, parent);

        log_info("Menu created: " + menu_id.toStdString() + " (" +
                 title.toStdString() + ")");

        return menu;

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to create menu '" +
                                menu_id.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<QMenu*>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<QToolBar*, qtplugin::PluginError> UIPlugin::create_toolbar(
    const QString& toolbar_id, const QString& title, QWidget* parent) {
    try {
        auto toolbar = new QToolBar(title, parent);
        toolbar->setObjectName(toolbar_id);

        log_info("Toolbar created: " + toolbar_id.toStdString() + " (" +
                 title.toStdString() + ")");

        return toolbar;

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to create toolbar '" +
                                toolbar_id.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<QToolBar*>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

// === Dialog Support ===

qtplugin::expected<QDialog*, qtplugin::PluginError> UIPlugin::create_dialog(
    const QString& dialog_id, QWidget* parent) {
    try {
        std::lock_guard lock(m_dialogs_mutex);

        // Check if dialog already exists
        if (m_dialogs.find(dialog_id) != m_dialogs.end()) {
            return qtplugin::make_error<QDialog*>(
                qtplugin::PluginErrorCode::AlreadyExists,
                "Dialog with ID '" + dialog_id.toStdString() +
                    "' already exists");
        }

        QDialog* dialog = nullptr;

        if (dialog_id == "settings") {
            auto settings_dialog = new SettingsDialog(parent);
            connect(settings_dialog, &SettingsDialog::settingsChanged, this,
                    &UIPlugin::on_settings_changed);
            dialog = settings_dialog;
        } else if (dialog_id == "about") {
            dialog = new AboutDialog(parent);
        } else {
            return qtplugin::make_error<QDialog*>(
                qtplugin::PluginErrorCode::NotFound,
                "Unknown dialog ID: " + dialog_id.toStdString());
        }

        if (dialog) {
            m_dialogs[dialog_id] = dialog;
            log_info("Dialog created: " + dialog_id.toStdString());
        }

        return dialog;

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to create dialog '" +
                                dialog_id.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<QDialog*>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<int, qtplugin::PluginError> UIPlugin::show_modal_dialog(
    const QString& dialog_id) {
    try {
        std::lock_guard lock(m_dialogs_mutex);

        auto it = m_dialogs.find(dialog_id);
        if (it == m_dialogs.end()) {
            return qtplugin::make_error<int>(
                qtplugin::PluginErrorCode::NotFound,
                "Dialog not found: " + dialog_id.toStdString());
        }

        QDialog* dialog = it->second;
        if (!dialog) {
            return qtplugin::make_error<int>(
                qtplugin::PluginErrorCode::NotFound,
                "Dialog is null: " + dialog_id.toStdString());
        }

        int result = dialog->exec();

        log_info("Modal dialog shown: " + dialog_id.toStdString() +
                 " (result: " + std::to_string(result) + ")");

        return result;

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to show modal dialog '" +
                                dialog_id.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<int>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

// === Theme Support ===

std::vector<qtplugin::UIThemeInfo> UIPlugin::get_available_themes() const {
    std::lock_guard lock(m_theme_mutex);
    return m_available_themes;
}

qtplugin::expected<void, qtplugin::PluginError> UIPlugin::apply_theme(
    const QString& theme_name) {
    try {
        std::lock_guard lock(m_theme_mutex);

        // Check if theme exists
        bool theme_found = false;
        for (const auto& theme : m_available_themes) {
            if (theme.name == theme_name) {
                theme_found = true;
                break;
            }
        }

        if (!theme_found && theme_name != "default") {
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::NotFound,
                "Theme not found: " + theme_name.toStdString());
        }

        // Apply theme to all widgets
        std::lock_guard widgets_lock(m_widgets_mutex);
        for (auto& [id, widget] : m_widgets) {
            if (widget) {
                auto demo_widget = qobject_cast<DemoWidget*>(widget);
                if (demo_widget) {
                    demo_widget->setTheme(theme_name);
                }
            }
        }

        m_current_theme = theme_name;

        log_info("Theme applied: " + theme_name.toStdString());

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to apply theme '" +
                                theme_name.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

QString UIPlugin::get_current_theme() const {
    std::lock_guard lock(m_theme_mutex);
    return m_current_theme;
}

// === Settings Integration ===

qtplugin::expected<QWidget*, qtplugin::PluginError>
UIPlugin::create_settings_widget(QWidget* parent) {
    try {
        auto settings_dialog = new SettingsDialog(parent);
        connect(settings_dialog, &SettingsDialog::settingsChanged, this,
                &UIPlugin::on_settings_changed);

        // Set current settings
        settings_dialog->setSettings(get_current_settings());

        log_info("Settings widget created");

        return settings_dialog;

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to create settings widget: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<QWidget*>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> UIPlugin::apply_settings(
    const QJsonObject& settings) {
    try {
        // Apply settings as configuration
        return configure(settings);

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to apply settings: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

QJsonObject UIPlugin::get_current_settings() const {
    std::lock_guard lock(m_config_mutex);

    QJsonObject settings = m_configuration;
    settings["current_theme"] = m_current_theme;
    settings["widget_count"] = static_cast<int>(m_widgets.size());
    settings["action_count"] = static_cast<int>(m_actions.size());

    return settings;
}
