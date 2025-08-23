/**
 * @file ui_plugin_commands.cpp
 * @brief Command handlers for UI plugin
 * @version 3.0.0
 *
 * This file contains the remaining command handlers for the UI plugin.
 */

#include <QJsonArray>
#include <QJsonDocument>
#include "ui_plugin.hpp"

// === Remaining Command Handlers ===

QJsonObject UIPlugin::handle_dialog_command(const QJsonObject& params) {
    QString action = params.value("action").toString();

    if (action == "create") {
        QString dialog_id = params.value("dialog_id").toString();
        if (dialog_id.isEmpty()) {
            return QJsonObject{{"success", false},
                               {"error", "dialog_id is required"}};
        }

        auto result = create_dialog(dialog_id);
        if (result) {
            return QJsonObject{{"success", true},
                               {"dialog_id", dialog_id},
                               {"dialog_created", true}};
        } else {
            return QJsonObject{
                {"success", false},
                {"error", QString::fromStdString(result.error().message)}};
        }
    } else if (action == "show") {
        QString dialog_id = params.value("dialog_id").toString();
        if (dialog_id.isEmpty()) {
            return QJsonObject{{"success", false},
                               {"error", "dialog_id is required"}};
        }

        auto result = show_modal_dialog(dialog_id);
        if (result) {
            return QJsonObject{{"success", true},
                               {"dialog_id", dialog_id},
                               {"result", result.value()}};
        } else {
            return QJsonObject{
                {"success", false},
                {"error", QString::fromStdString(result.error().message)}};
        }
    } else if (action == "list") {
        QJsonArray dialogs;
        dialogs.append("settings");
        dialogs.append("about");

        return QJsonObject{
            {"success", true},
            {"available_dialogs", dialogs},
            {"active_dialogs", static_cast<int>(m_dialogs.size())}};
    } else {
        return QJsonObject{
            {"success", false},
            {"error", "Invalid action. Supported: create, show, list"}};
    }
}

QJsonObject UIPlugin::handle_theme_command(const QJsonObject& params) {
    QString action = params.value("action").toString();

    if (action == "list") {
        QJsonArray themes;
        auto available_themes = get_available_themes();
        for (const auto& theme : available_themes) {
            themes.append(QJsonObject{{"name", theme.name},
                                      {"description", theme.description},
                                      {"stylesheet", theme.stylesheet},
                                      {"dark_mode", theme.dark_mode}});
        }

        return QJsonObject{
            {"success", true},
            {"themes", themes},
            {"current_theme", get_current_theme()},
            {"theme_count", static_cast<int>(available_themes.size())}};
    } else if (action == "apply") {
        QString theme_name = params.value("theme_name").toString();
        if (theme_name.isEmpty()) {
            return QJsonObject{{"success", false},
                               {"error", "theme_name is required"}};
        }

        auto result = apply_theme(theme_name);
        return QJsonObject{
            {"success", result.has_value()},
            {"theme_name", theme_name},
            {"current_theme", get_current_theme()},
            {"error",
             result ? "" : QString::fromStdString(result.error().message)}};
    } else if (action == "current") {
        return QJsonObject{{"success", true},
                           {"current_theme", get_current_theme()}};
    } else {
        return QJsonObject{
            {"success", false},
            {"error", "Invalid action. Supported: list, apply, current"}};
    }
}

QJsonObject UIPlugin::handle_settings_command(const QJsonObject& params) {
    QString action = params.value("action").toString();

    if (action == "get") {
        return QJsonObject{{"success", true},
                           {"settings", get_current_settings()}};
    } else if (action == "set") {
        QJsonObject settings = params.value("settings").toObject();
        if (settings.isEmpty()) {
            return QJsonObject{{"success", false},
                               {"error", "settings object is required"}};
        }

        auto result = apply_settings(settings);
        return QJsonObject{
            {"success", result.has_value()},
            {"error",
             result ? "" : QString::fromStdString(result.error().message)},
            {"current_settings", get_current_settings()}};
    } else if (action == "reset") {
        auto default_config = default_configuration();
        if (default_config) {
            auto result = configure(default_config.value());
            return QJsonObject{
                {"success", result.has_value()},
                {"error",
                 result ? "" : QString::fromStdString(result.error().message)},
                {"settings", get_current_settings()}};
        } else {
            return QJsonObject{{"success", false},
                               {"error", "No default configuration available"}};
        }
    } else {
        return QJsonObject{
            {"success", false},
            {"error", "Invalid action. Supported: get, set, reset"}};
    }
}

QJsonObject UIPlugin::handle_status_command(const QJsonObject& params) {
    Q_UNUSED(params)

    return QJsonObject{
        {"plugin_name", QString::fromStdString(std::string(name()))},
        {"plugin_id", QString::fromStdString(id())},
        {"state", static_cast<int>(m_state.load())},
        {"uptime_ms", static_cast<qint64>(uptime().count())},
        {"current_theme", get_current_theme()},
        {"widget_count", static_cast<int>(m_widgets.size())},
        {"action_count", static_cast<int>(m_actions.size())},
        {"dialog_count", static_cast<int>(m_dialogs.size())},
        {"theme_count", static_cast<int>(m_available_themes.size())},
        {"dependencies_satisfied", dependencies_satisfied()},
        {"supported_components", static_cast<int>(supported_components())},
        {"integration_points",
         static_cast<int>(supported_integration_points().size())},
        {"logging_enabled", m_logging_enabled},
        {"auto_save_enabled", m_auto_save_enabled},
        {"refresh_interval", m_refresh_interval}};
}
