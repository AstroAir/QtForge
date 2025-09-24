#include <QLoggingCategory>
#include <sol/sol.hpp>

#include "qtplugin/interfaces/ui_plugin_interface.hpp"
#include "qtplugin/utils/error_handling.hpp"

Q_LOGGING_CATEGORY(uiBindingsLog, "qtforge.lua.ui")

namespace qtplugin {

/**
 * @brief Register UIComponentType enum bindings
 */
void register_ui_component_type_bindings(sol::state_view& lua) {
    qCDebug(uiBindingsLog) << "Registering UIComponentType enum bindings...";

    sol::table qtforge = lua["qtforge"];
    sol::table ui_ns = qtforge["ui"].get_or_create<sol::table>();

    // UIComponentType enum
    ui_ns.new_enum<UIComponentType>(
        "UIComponentType", {{"None", UIComponentType::None},
                            {"Widget", UIComponentType::Widget},
                            {"Dialog", UIComponentType::Dialog},
                            {"DockWidget", UIComponentType::DockWidget},
                            {"ToolBar", UIComponentType::ToolBar},
                            {"MenuBar", UIComponentType::MenuBar},
                            {"ContextMenu", UIComponentType::ContextMenu},
                            {"StatusBar", UIComponentType::StatusBar},
                            {"PropertyEditor", UIComponentType::PropertyEditor},
                            {"TreeView", UIComponentType::TreeView},
                            {"ListView", UIComponentType::ListView},
                            {"TableView", UIComponentType::TableView},
                            {"GraphicsView", UIComponentType::GraphicsView},
                            {"CustomControl", UIComponentType::CustomControl},
                            {"Wizard", UIComponentType::Wizard},
                            {"Settings", UIComponentType::Settings}});

    qCDebug(uiBindingsLog) << "UIComponentType enum bindings registered";
}

/**
 * @brief Register UIIntegrationMode enum bindings
 */
void register_ui_integration_mode_bindings(sol::state_view& lua) {
    qCDebug(uiBindingsLog) << "Registering UIIntegrationMode enum bindings...";

    sol::table qtforge = lua["qtforge"];
    sol::table ui_ns = qtforge["ui"].get_or_create<sol::table>();

    // UIIntegrationMode enum
    ui_ns.new_enum<UIIntegrationMode>(
        "UIIntegrationMode", {{"Standalone", UIIntegrationMode::Standalone},
                              {"Integrated", UIIntegrationMode::Integrated},
                              {"Overlay", UIIntegrationMode::Overlay},
                              {"Modal", UIIntegrationMode::Modal},
                              {"Embedded", UIIntegrationMode::Embedded}});

    qCDebug(uiBindingsLog) << "UIIntegrationMode enum bindings registered";
}

/**
 * @brief Register UIIntegrationPoint enum bindings
 */
void register_ui_integration_point_bindings(sol::state_view& lua) {
    qCDebug(uiBindingsLog) << "Registering UIIntegrationPoint enum bindings...";

    sol::table qtforge = lua["qtforge"];
    sol::table ui_ns = qtforge["ui"].get_or_create<sol::table>();

    // UIIntegrationPoint enum
    ui_ns.new_enum<UIIntegrationPoint>(
        "UIIntegrationPoint",
        {{"MainWindow", UIIntegrationPoint::MainWindow},
         {"MenuBar", UIIntegrationPoint::MenuBar},
         {"ToolBar", UIIntegrationPoint::ToolBar},
         {"StatusBar", UIIntegrationPoint::StatusBar},
         {"DockArea", UIIntegrationPoint::DockArea},
         {"CentralWidget", UIIntegrationPoint::CentralWidget},
         {"ContextMenu", UIIntegrationPoint::ContextMenu},
         {"SettingsDialog", UIIntegrationPoint::SettingsDialog},
         {"AboutDialog", UIIntegrationPoint::AboutDialog},
         {"CustomArea", UIIntegrationPoint::CustomArea}});

    qCDebug(uiBindingsLog) << "UIIntegrationPoint enum bindings registered";
}

/**
 * @brief Register UIActionInfo structure bindings
 */
void register_ui_action_info_bindings(sol::state_view& lua) {
    qCDebug(uiBindingsLog) << "Registering UIActionInfo structure bindings...";

    sol::table qtforge = lua["qtforge"];
    sol::table ui_ns = qtforge["ui"].get_or_create<sol::table>();

    // UIActionInfo structure
    auto ui_action_info_type = ui_ns.new_usertype<UIActionInfo>(
        "UIActionInfo", sol::constructors<UIActionInfo()>(), "id",
        &UIActionInfo::id, "text", &UIActionInfo::text, "tooltip",
        &UIActionInfo::tooltip, "status_tip", &UIActionInfo::status_tip, "icon",
        &UIActionInfo::icon, "shortcut", &UIActionInfo::shortcut, "checkable",
        &UIActionInfo::checkable, "checked", &UIActionInfo::checked, "enabled",
        &UIActionInfo::enabled, "visible", &UIActionInfo::visible, "menu_path",
        &UIActionInfo::menu_path, "priority", &UIActionInfo::priority,
        "custom_data", &UIActionInfo::custom_data);

    qCDebug(uiBindingsLog) << "UIActionInfo structure bindings registered";
}

/**
 * @brief Register UIWidgetInfo structure bindings
 */
void register_ui_widget_info_bindings(sol::state_view& lua) {
    qCDebug(uiBindingsLog) << "Registering UIWidgetInfo structure bindings...";

    sol::table qtforge = lua["qtforge"];
    sol::table ui_ns = qtforge["ui"].get_or_create<sol::table>();

    // UIWidgetInfo structure
    auto ui_widget_info_type = ui_ns.new_usertype<UIWidgetInfo>(
        "UIWidgetInfo", sol::constructors<UIWidgetInfo()>(), "id",
        &UIWidgetInfo::id, "title", &UIWidgetInfo::title, "description",
        &UIWidgetInfo::description, "icon", &UIWidgetInfo::icon, "type",
        &UIWidgetInfo::type, "integration_point",
        &UIWidgetInfo::integration_point, "integration_mode",
        &UIWidgetInfo::integration_mode, "preferred_size",
        &UIWidgetInfo::preferred_size, "minimum_size",
        &UIWidgetInfo::minimum_size, "maximum_size",
        &UIWidgetInfo::maximum_size, "resizable", &UIWidgetInfo::resizable,
        "closable", &UIWidgetInfo::closable, "floatable",
        &UIWidgetInfo::floatable, "accessible", &UIWidgetInfo::accessible,
        "allowed_areas", &UIWidgetInfo::allowed_areas, "custom_properties",
        &UIWidgetInfo::custom_properties, "accessibility_info",
        &UIWidgetInfo::accessibility_info);

    qCDebug(uiBindingsLog) << "UIWidgetInfo structure bindings registered";
}

/**
 * @brief Register UIThemeInfo structure bindings
 */
void register_ui_theme_info_bindings(sol::state_view& lua) {
    qCDebug(uiBindingsLog) << "Registering UIThemeInfo structure bindings...";

    sol::table qtforge = lua["qtforge"];
    sol::table ui_ns = qtforge["ui"].get_or_create<sol::table>();

    // UIThemeInfo structure
    auto ui_theme_info_type = ui_ns.new_usertype<UIThemeInfo>(
        "UIThemeInfo", sol::constructors<UIThemeInfo()>(), "name",
        &UIThemeInfo::name, "description", &UIThemeInfo::description,
        "stylesheet", &UIThemeInfo::stylesheet, "color_scheme",
        &UIThemeInfo::color_scheme, "font_settings",
        &UIThemeInfo::font_settings, "icon_theme", &UIThemeInfo::icon_theme,
        "dark_mode", &UIThemeInfo::dark_mode, "high_contrast",
        &UIThemeInfo::high_contrast, "accessibility_settings",
        &UIThemeInfo::accessibility_settings);

    qCDebug(uiBindingsLog) << "UIThemeInfo structure bindings registered";
}

/**
 * @brief Register IUIPlugin interface bindings
 */
void register_ui_plugin_interface_bindings(sol::state_view& lua) {
    qCDebug(uiBindingsLog) << "Registering IUIPlugin interface bindings...";

    sol::table qtforge = lua["qtforge"];
    sol::table ui_ns = qtforge["ui"].get_or_create<sol::table>();

    // IUIPlugin interface
    auto ui_plugin_type = ui_ns.new_usertype<IUIPlugin>(
        "IUIPlugin", sol::no_constructor,

        // Widget Management
        "create_widget",
        [](IUIPlugin& plugin, const QString& widget_id,
           sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto result = plugin.create_widget(widget_id, nullptr);
            if (result.has_value()) {
                return sol::make_object(lua, result.value());
            } else {
                sol::table error_result = lua.create_table();
                error_result["success"] = false;
                error_result["error"] = result.error();
                return error_result;
            }
        },

        "create_main_widget",
        [](IUIPlugin& plugin, sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto widget = plugin.create_main_widget(nullptr);
            if (widget) {
                return sol::make_object(lua, widget.release());
            } else {
                return sol::nil;
            }
        },

        "create_configuration_widget",
        [](IUIPlugin& plugin, sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto widget = plugin.create_configuration_widget(nullptr);
            if (widget) {
                return sol::make_object(lua, widget.release());
            } else {
                return sol::nil;
            }
        },

        "create_dock_widget",
        [](IUIPlugin& plugin, sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto widget = plugin.create_dock_widget(nullptr);
            if (widget) {
                return sol::make_object(lua, widget.release());
            } else {
                return sol::nil;
            }
        },

        "create_status_widget",
        [](IUIPlugin& plugin, sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto widget = plugin.create_status_widget(nullptr);
            if (widget) {
                return sol::make_object(lua, widget.release());
            } else {
                return sol::nil;
            }
        },

        "get_widget_info",
        [](IUIPlugin& plugin, const QString& widget_id,
           sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto result = plugin.get_widget_info(widget_id);
            if (result.has_value()) {
                return sol::make_object(lua, result.value());
            } else {
                sol::table error_result = lua.create_table();
                error_result["success"] = false;
                error_result["error"] = result.error();
                return error_result;
            }
        },

        "get_available_widgets", &IUIPlugin::get_available_widgets,

        "destroy_widget",
        [](IUIPlugin& plugin, const QString& widget_id,
           sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto result = plugin.destroy_widget(widget_id);
            if (result.has_value()) {
                sol::table success_result = lua.create_table();
                success_result["success"] = true;
                return success_result;
            } else {
                sol::table error_result = lua.create_table();
                error_result["success"] = false;
                error_result["error"] = result.error();
                return error_result;
            }
        },

        // Layout and Sizing
        "minimum_size", &IUIPlugin::minimum_size, "preferred_size",
        &IUIPlugin::preferred_size, "maximum_size", &IUIPlugin::maximum_size,
        "is_resizable", &IUIPlugin::is_resizable,

        // Component Support
        "supported_components", &IUIPlugin::supported_components,
        "supports_component", &IUIPlugin::supports_component,
        "supported_integration_points",
        &IUIPlugin::supported_integration_points, "integration_mode",
        &IUIPlugin::integration_mode,

        // Action Management
        "create_action",
        [](IUIPlugin& plugin, const UIActionInfo& action_info,
           sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto result = plugin.create_action(action_info, nullptr);
            if (result.has_value()) {
                return sol::make_object(lua, result.value());
            } else {
                sol::table error_result = lua.create_table();
                error_result["success"] = false;
                error_result["error"] = result.error();
                return error_result;
            }
        },

        "get_available_actions", &IUIPlugin::get_available_actions,

        "remove_action",
        [](IUIPlugin& plugin, const QString& action_id,
           sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto result = plugin.remove_action(action_id);
            if (result.has_value()) {
                sol::table success_result = lua.create_table();
                success_result["success"] = true;
                return success_result;
            } else {
                sol::table error_result = lua.create_table();
                error_result["success"] = false;
                error_result["error"] = result.error();
                return error_result;
            }
        },

        // Settings Integration
        "create_settings_widget",
        [](IUIPlugin& plugin, sol::this_state s) -> sol::object {
            sol::state_view lua(s);
            auto result = plugin.create_settings_widget(nullptr);
            if (result.has_value()) {
                return sol::make_object(lua, result.value());
            } else {
                sol::table error_result = lua.create_table();
                error_result["success"] = false;
                error_result["error"] = result.error();
                return error_result;
            }
        });

    qCDebug(uiBindingsLog) << "IUIPlugin interface bindings registered";
}

/**
 * @brief Register all UI bindings
 */
void register_ui_bindings(sol::state_view& lua) {
    qCDebug(uiBindingsLog) << "Registering UI bindings...";

    // Create qtforge.ui namespace
    sol::table qtforge = lua["qtforge"];
    sol::table ui_ns = qtforge["ui"].get_or_create<sol::table>();

    // Register all UI types
    register_ui_component_type_bindings(lua);
    register_ui_integration_mode_bindings(lua);
    register_ui_integration_point_bindings(lua);
    register_ui_action_info_bindings(lua);
    register_ui_widget_info_bindings(lua);
    register_ui_theme_info_bindings(lua);
    register_ui_plugin_interface_bindings(lua);

    // Add utility functions to ui namespace
    ui_ns["create_widget_info"] = []() { return UIWidgetInfo{}; };

    ui_ns["create_action_info"] = []() { return UIActionInfo{}; };

    ui_ns["create_theme_info"] = []() { return UIThemeInfo{}; };

    qCDebug(uiBindingsLog) << "UI bindings registration complete";
}

}  // namespace qtplugin
