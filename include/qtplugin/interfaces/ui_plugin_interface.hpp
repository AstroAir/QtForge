/**
 * @file ui_plugin_interface.hpp
 * @brief Unified UI plugin interface combining comprehensive features with modern C++ patterns
 * @version 3.1.0
 *
 * This interface consolidates the best features from both the comprehensive enterprise-focused
 * UI interface and the streamlined developer-friendly interface, providing a complete solution
 * for C++ widget-based user interface components with modern accessibility and state management.
 */

#pragma once

#include <QAction>
#include <QDialog>
#include <QDockWidget>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QMenu>
#include <QMetaType>
#include <QObject>
#include <QSize>
#include <QSizePolicy>
#include <QToolBar>
#include <QWidget>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>
#include "core/plugin_interface.hpp"

// Ensure Qt types are properly included
#include <QtCore/QString>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtGui/QKeySequence>
#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QSizePolicy>

namespace qtplugin {

/**
 * @brief UI component types
 */
enum class UIComponentType : uint32_t {
    None = 0x0000,
    Widget = 0x0001,          ///< Custom widget
    Dialog = 0x0002,          ///< Dialog window
    DockWidget = 0x0004,      ///< Dockable widget
    ToolBar = 0x0008,         ///< Toolbar
    MenuBar = 0x0010,         ///< Menu bar
    ContextMenu = 0x0020,     ///< Context menu
    StatusBar = 0x0040,       ///< Status bar widget
    PropertyEditor = 0x0080,  ///< Property editor
    TreeView = 0x0100,        ///< Tree view component
    ListView = 0x0200,        ///< List view component
    TableView = 0x0400,       ///< Table view component
    GraphicsView = 0x0800,    ///< Graphics view component
    CustomControl = 0x1000,   ///< Custom control
    Wizard = 0x2000,          ///< Wizard dialog
    Settings = 0x4000         ///< Settings interface
};

using UIComponentTypes = std::underlying_type_t<UIComponentType>;

/**
 * @brief UI integration modes (enhanced from streamlined interface)
 */
enum class UIIntegrationMode {
    Standalone,  ///< Plugin provides standalone widgets
    Integrated,  ///< Plugin integrates with host application UI
    Overlay,     ///< Plugin provides overlay UI elements
    Modal,       ///< Plugin provides modal dialogs
    Embedded     ///< Plugin embeds in existing UI areas
};

/**
 * @brief UI integration points
 */
enum class UIIntegrationPoint {
    MainWindow,      ///< Main application window
    MenuBar,         ///< Application menu bar
    ToolBar,         ///< Application toolbar
    StatusBar,       ///< Application status bar
    DockArea,        ///< Dockable area
    CentralWidget,   ///< Central widget area
    ContextMenu,     ///< Context menus
    SettingsDialog,  ///< Settings/preferences dialog
    AboutDialog,     ///< About dialog
    CustomArea       ///< Custom integration area
};

/**
 * @brief UI action information (enhanced with modern patterns)
 */
struct UIActionInfo {
    QString id;               ///< Action identifier
    QString text;             ///< Action text
    QString tooltip;          ///< Action tooltip
    QString status_tip;       ///< Status bar tip
    QIcon icon;               ///< Action icon
    QKeySequence shortcut;    ///< Keyboard shortcut
    bool checkable = false;   ///< Whether action is checkable
    bool checked = false;     ///< Initial checked state
    bool enabled = true;      ///< Whether action is enabled
    bool visible = true;      ///< Whether action is visible
    QString menu_path;        ///< Menu path (e.g., "File/Export")
    int priority = 0;         ///< Display priority
    QJsonObject custom_data;  ///< Custom action data

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const {
        QJsonObject json;
        json["id"] = id;
        json["text"] = text;
        json["tooltip"] = tooltip;
        json["status_tip"] = status_tip;
        json["shortcut"] = shortcut.toString();
        json["checkable"] = checkable;
        json["checked"] = checked;
        json["enabled"] = enabled;
        json["visible"] = visible;
        json["menu_path"] = menu_path;
        json["priority"] = priority;
        json["custom_data"] = custom_data;
        return json;
    }

    /**
     * @brief Create from JSON object
     */
    static UIActionInfo from_json(const QJsonObject& json) {
        UIActionInfo info;
        info.id = json["id"].toString();
        info.text = json["text"].toString();
        info.tooltip = json["tooltip"].toString();
        info.status_tip = json["status_tip"].toString();
        info.shortcut = QKeySequence::fromString(json["shortcut"].toString());
        info.checkable = json["checkable"].toBool();
        info.checked = json["checked"].toBool();
        info.enabled = json["enabled"].toBool(true);
        info.visible = json["visible"].toBool(true);
        info.menu_path = json["menu_path"].toString();
        info.priority = json["priority"].toInt();
        info.custom_data = json["custom_data"].toObject();
        return info;
    }

    /**
     * @brief Equality comparison
     */
    bool operator==(const UIActionInfo& other) const noexcept {
        return id == other.id && text == other.text &&
               tooltip == other.tooltip && status_tip == other.status_tip;
    }

    /**
     * @brief Inequality comparison
     */
    bool operator!=(const UIActionInfo& other) const noexcept {
        return !(*this == other);
    }
};

/**
 * @brief UI widget information (enhanced with accessibility and state management)
 */
struct UIWidgetInfo {
    QString id;                            ///< Widget identifier
    QString title;                         ///< Widget title
    QString description;                   ///< Widget description
    QIcon icon;                            ///< Widget icon
    UIComponentType type;                  ///< Widget type
    UIIntegrationPoint integration_point;  ///< Integration point
    UIIntegrationMode integration_mode;    ///< Integration mode
    QSize preferred_size;                  ///< Preferred size
    QSize minimum_size;                    ///< Minimum size
    QSize maximum_size;                    ///< Maximum size
    bool resizable = true;                 ///< Whether widget is resizable
    bool closable = true;                  ///< Whether widget is closable
    bool floatable = true;                 ///< Whether widget can float
    bool accessible = true;                ///< Whether widget supports accessibility
    Qt::DockWidgetAreas allowed_areas =
        Qt::AllDockWidgetAreas;     ///< Allowed dock areas
    QJsonObject custom_properties;  ///< Custom widget properties
    QJsonObject accessibility_info; ///< Accessibility information

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const {
        QJsonObject json;
        json["id"] = id;
        json["title"] = title;
        json["description"] = description;
        json["type"] = static_cast<int>(type);
        json["integration_point"] = static_cast<int>(integration_point);
        json["integration_mode"] = static_cast<int>(integration_mode);

        QJsonObject size_obj;
        size_obj["preferred_width"] = preferred_size.width();
        size_obj["preferred_height"] = preferred_size.height();
        size_obj["minimum_width"] = minimum_size.width();
        size_obj["minimum_height"] = minimum_size.height();
        size_obj["maximum_width"] = maximum_size.width();
        size_obj["maximum_height"] = maximum_size.height();
        json["sizes"] = size_obj;

        json["resizable"] = resizable;
        json["closable"] = closable;
        json["floatable"] = floatable;
        json["accessible"] = accessible;
        json["allowed_areas"] = static_cast<int>(allowed_areas);
        json["custom_properties"] = custom_properties;
        json["accessibility_info"] = accessibility_info;

        return json;
    }

    /**
     * @brief Create from JSON object
     */
    static UIWidgetInfo from_json(const QJsonObject& json) {
        UIWidgetInfo info;
        info.id = json["id"].toString();
        info.title = json["title"].toString();
        info.description = json["description"].toString();
        info.type = static_cast<UIComponentType>(json["type"].toInt());
        info.integration_point = static_cast<UIIntegrationPoint>(json["integration_point"].toInt());
        info.integration_mode = static_cast<UIIntegrationMode>(json["integration_mode"].toInt());

        QJsonObject size_obj = json["sizes"].toObject();
        info.preferred_size = QSize(size_obj["preferred_width"].toInt(400),
                                   size_obj["preferred_height"].toInt(300));
        info.minimum_size = QSize(size_obj["minimum_width"].toInt(200),
                                 size_obj["minimum_height"].toInt(150));
        info.maximum_size = QSize(size_obj["maximum_width"].toInt(),
                                 size_obj["maximum_height"].toInt());

        info.resizable = json["resizable"].toBool(true);
        info.closable = json["closable"].toBool(true);
        info.floatable = json["floatable"].toBool(true);
        info.accessible = json["accessible"].toBool(true);
        info.allowed_areas = static_cast<Qt::DockWidgetAreas>(json["allowed_areas"].toInt(Qt::AllDockWidgetAreas));
        info.custom_properties = json["custom_properties"].toObject();
        info.accessibility_info = json["accessibility_info"].toObject();

        return info;
    }

    /**
     * @brief Equality comparison
     */
    bool operator==(const UIWidgetInfo& other) const noexcept {
        return id == other.id && title == other.title && type == other.type;
    }

    /**
     * @brief Inequality comparison
     */
    bool operator!=(const UIWidgetInfo& other) const noexcept {
        return !(*this == other);
    }
};

/**
 * @brief UI theme information (enhanced with accessibility and validation)
 */
struct UIThemeInfo {
    QString name;               ///< Theme name
    QString description;        ///< Theme description
    QString stylesheet;         ///< CSS stylesheet
    QJsonObject color_scheme;   ///< Color scheme
    QJsonObject font_settings;  ///< Font settings
    QJsonObject icon_theme;     ///< Icon theme settings
    bool dark_mode = false;     ///< Whether it's a dark theme
    bool high_contrast = false; ///< Whether it's a high contrast theme
    QJsonObject accessibility_settings; ///< Accessibility-specific settings

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const {
        QJsonObject json;
        json["name"] = name;
        json["description"] = description;
        json["stylesheet"] = stylesheet;
        json["color_scheme"] = color_scheme;
        json["font_settings"] = font_settings;
        json["icon_theme"] = icon_theme;
        json["dark_mode"] = dark_mode;
        json["high_contrast"] = high_contrast;
        json["accessibility_settings"] = accessibility_settings;
        return json;
    }

    /**
     * @brief Create from JSON object
     */
    static UIThemeInfo from_json(const QJsonObject& json) {
        UIThemeInfo info;
        info.name = json["name"].toString();
        info.description = json["description"].toString();
        info.stylesheet = json["stylesheet"].toString();
        info.color_scheme = json["color_scheme"].toObject();
        info.font_settings = json["font_settings"].toObject();
        info.icon_theme = json["icon_theme"].toObject();
        info.dark_mode = json["dark_mode"].toBool();
        info.high_contrast = json["high_contrast"].toBool();
        info.accessibility_settings = json["accessibility_settings"].toObject();
        return info;
    }

    /**
     * @brief Equality comparison
     */
    bool operator==(const UIThemeInfo& other) const noexcept {
        return name == other.name && dark_mode == other.dark_mode;
    }

    /**
     * @brief Inequality comparison
     */
    bool operator!=(const UIThemeInfo& other) const noexcept {
        return !(*this == other);
    }
};

/**
 * @brief UI event callback types
 */
using UIActionCallback =
    std::function<void(const QString& action_id, bool checked)>;
using UIWidgetCallback =
    std::function<void(const QString& widget_id, const QString& event)>;

/**
 * @brief Unified UI plugin interface
 *
 * This interface combines comprehensive enterprise features with modern C++ patterns
 * for creating widget-based UI components. It includes accessibility support,
 * state management, and enhanced integration capabilities.
 *
 * Features from both interfaces:
 * - Comprehensive component and action management
 * - Modern widget creation with smart pointers
 * - Accessibility and theme support
 * - UI state management and lifecycle events
 * - Enhanced integration modes and layout management
 */
class IUIPlugin : public virtual IPlugin {
public:
    ~IUIPlugin() override = default;

    // === UI Component Support ===

    /**
     * @brief Get supported UI component types
     * @return Bitfield of supported component types
     */
    virtual UIComponentTypes supported_components() const noexcept = 0;

    /**
     * @brief Check if component type is supported
     * @param component Component type to check
     * @return true if component is supported
     */
    bool supports_component(UIComponentType component) const noexcept {
        return (supported_components() &
                static_cast<UIComponentTypes>(component)) != 0;
    }

    /**
     * @brief Get supported integration points
     * @return Vector of supported integration points
     */
    virtual std::vector<UIIntegrationPoint> supported_integration_points()
        const = 0;

    /**
     * @brief Get UI integration mode (enhanced from streamlined interface)
     * @return Preferred UI integration mode
     */
    virtual UIIntegrationMode integration_mode() const noexcept {
        return UIIntegrationMode::Standalone;
    }

    // === Enhanced Widget Management (combining both interfaces) ===

    /**
     * @brief Create widget with identifier (comprehensive interface)
     * @param widget_id Widget identifier
     * @param parent Parent widget
     * @return Created widget or error
     */
    virtual qtplugin::expected<QWidget*, PluginError> create_widget(
        const QString& widget_id, QWidget* parent = nullptr) = 0;

    /**
     * @brief Create main plugin widget (streamlined interface)
     * @param parent Parent widget (optional)
     * @return Unique pointer to the created widget
     */
    virtual std::unique_ptr<QWidget> create_main_widget(
        QWidget* parent = nullptr) = 0;

    /**
     * @brief Create configuration widget (enhanced from streamlined)
     * @param parent Parent widget (optional)
     * @return Unique pointer to the configuration widget, or nullptr if not supported
     */
    virtual std::unique_ptr<QWidget> create_configuration_widget(
        QWidget* parent = nullptr) {
        Q_UNUSED(parent)
        return nullptr;
    }

    /**
     * @brief Create dock widget (enhanced from streamlined)
     * @param parent Parent widget (optional)
     * @return Unique pointer to the dock widget, or nullptr if not supported
     */
    virtual std::unique_ptr<QDockWidget> create_dock_widget(
        QWidget* parent = nullptr) {
        Q_UNUSED(parent)
        return nullptr;
    }

    /**
     * @brief Create status widget (enhanced from streamlined)
     * @param parent Parent widget (optional)
     * @return Unique pointer to the status widget, or nullptr if not supported
     */
    virtual std::unique_ptr<QWidget> create_status_widget(
        QWidget* parent = nullptr) {
        Q_UNUSED(parent)
        return nullptr;
    }

    /**
     * @brief Get widget information
     * @param widget_id Widget identifier
     * @return Widget information or error
     */
    virtual qtplugin::expected<UIWidgetInfo, PluginError> get_widget_info(
        const QString& widget_id) const = 0;

    /**
     * @brief Get available widgets
     * @return Vector of available widget IDs
     */
    virtual std::vector<QString> get_available_widgets() const = 0;

    /**
     * @brief Destroy widget
     * @param widget_id Widget identifier
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> destroy_widget(
        const QString& widget_id) = 0;

    // === Layout and Sizing (enhanced from streamlined interface) ===

    /**
     * @brief Get minimum widget size
     * @return Minimum size for the plugin widget
     */
    virtual QSize minimum_size() const noexcept { return QSize{200, 150}; }

    /**
     * @brief Get preferred widget size
     * @return Preferred size for the plugin widget
     */
    virtual QSize preferred_size() const noexcept { return QSize{400, 300}; }

    /**
     * @brief Get maximum widget size
     * @return Maximum size for the plugin widget, or invalid size for no limit
     */
    virtual QSize maximum_size() const noexcept {
        return QSize{};  // Invalid size means no maximum
    }

    /**
     * @brief Check if widget is resizable
     * @return true if the widget can be resized
     */
    virtual bool is_resizable() const noexcept { return true; }

    /**
     * @brief Get size policy
     * @return Size policy for the plugin widget
     */
    virtual std::pair<QSizePolicy::Policy, QSizePolicy::Policy> size_policy()
        const noexcept {
        return {QSizePolicy::Preferred, QSizePolicy::Preferred};
    }

    // === Action Management ===

    /**
     * @brief Create action
     * @param action_info Action information
     * @param parent Parent object
     * @return Created action or error
     */
    virtual qtplugin::expected<QAction*, PluginError> create_action(
        const UIActionInfo& action_info, QObject* parent = nullptr) = 0;

    /**
     * @brief Get available actions
     * @return Vector of available action information
     */
    virtual std::vector<UIActionInfo> get_available_actions() const = 0;

    /**
     * @brief Set action callback
     * @param action_id Action identifier
     * @param callback Action callback function
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> set_action_callback(
        const QString& action_id, UIActionCallback callback) = 0;

    /**
     * @brief Remove action
     * @param action_id Action identifier
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> remove_action(
        const QString& action_id) = 0;

    // === Menu and Toolbar Support ===

    /**
     * @brief Create menu
     * @param menu_id Menu identifier
     * @param title Menu title
     * @param parent Parent widget
     * @return Created menu or error
     */
    virtual qtplugin::expected<QMenu*, PluginError> create_menu(
        const QString& menu_id, const QString& title,
        QWidget* parent = nullptr) = 0;

    /**
     * @brief Create toolbar
     * @param toolbar_id Toolbar identifier
     * @param title Toolbar title
     * @param parent Parent widget
     * @return Created toolbar or error
     */
    virtual qtplugin::expected<QToolBar*, PluginError> create_toolbar(
        const QString& toolbar_id, const QString& title,
        QWidget* parent = nullptr) = 0;

    // === Dialog Support ===

    /**
     * @brief Create dialog
     * @param dialog_id Dialog identifier
     * @param parent Parent widget
     * @return Created dialog or error
     */
    virtual qtplugin::expected<QDialog*, PluginError> create_dialog(
        const QString& dialog_id, QWidget* parent = nullptr) = 0;

    /**
     * @brief Show modal dialog
     * @param dialog_id Dialog identifier
     * @return Dialog result or error
     */
    virtual qtplugin::expected<int, PluginError> show_modal_dialog(
        const QString& dialog_id) = 0;

    // === Theme Support ===

    /**
     * @brief Get available themes
     * @return Vector of available theme information
     */
    virtual std::vector<UIThemeInfo> get_available_themes() const { return {}; }

    /**
     * @brief Apply theme
     * @param theme_name Theme name
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> apply_theme(
        const QString& theme_name) {
        Q_UNUSED(theme_name)
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "Theme support not implemented");
    }

    /**
     * @brief Get current theme
     * @return Current theme name
     */
    virtual QString get_current_theme() const { return "default"; }

    // === Accessibility Support (enhanced from streamlined interface) ===

    /**
     * @brief Get accessibility information
     * @return Accessibility information as JSON object
     */
    virtual QJsonObject accessibility_info() const {
        return QJsonObject{{"accessible", true},
                           {"screen_reader_compatible", true},
                           {"keyboard_navigable", true},
                           {"high_contrast_support", true}};
    }

    /**
     * @brief Check if plugin supports accessibility features
     * @return true if accessibility features are supported
     */
    virtual bool supports_accessibility() const noexcept { return true; }

    /**
     * @brief Validate UI requirements
     * @param parent_widget Parent widget that will host the plugin
     * @return Success or error information with validation details
     */
    virtual qtplugin::expected<void, PluginError> validate_ui_requirements(
        QWidget* parent_widget) const {
        Q_UNUSED(parent_widget)
        return make_success();
    }

    // === UI State Management (enhanced from streamlined interface) ===

    /**
     * @brief Save UI state
     * @return UI state as JSON object
     */
    virtual QJsonObject save_ui_state() const { return QJsonObject{}; }

    /**
     * @brief Restore UI state
     * @param state UI state to restore
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> restore_ui_state(
        const QJsonObject& state) {
        Q_UNUSED(state)
        return make_success();
    }

    /**
     * @brief Reset UI to default state
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> reset_ui_state() {
        return make_success();
    }

    // === Enhanced Event Handling (combining both interfaces) ===

    /**
     * @brief Handle UI setup completion (from streamlined interface)
     * Called after the plugin widget has been created and integrated
     * into the host application UI.
     * @param main_window Pointer to the main application window
     */
    virtual void on_ui_setup_complete(QWidget* main_window) {
        Q_UNUSED(main_window)
        // Default implementation does nothing
    }

    /**
     * @brief Handle UI cleanup (from streamlined interface)
     * Called before the plugin widget is destroyed or removed
     * from the host application UI.
     */
    virtual void on_ui_cleanup() {
        // Default implementation does nothing
    }

    /**
     * @brief Handle focus gained (from streamlined interface)
     * Called when the plugin widget gains focus.
     */
    virtual void on_focus_gained() {
        // Default implementation does nothing
    }

    /**
     * @brief Handle focus lost (from streamlined interface)
     * Called when the plugin widget loses focus.
     */
    virtual void on_focus_lost() {
        // Default implementation does nothing
    }

    /**
     * @brief Handle visibility change (from streamlined interface)
     * @param visible true if the widget became visible, false if hidden
     */
    virtual void on_visibility_changed(bool visible) {
        Q_UNUSED(visible)
        // Default implementation does nothing
    }

    /**
     * @brief Set widget event callback
     * @param widget_id Widget identifier
     * @param callback Widget event callback
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> set_widget_callback(
        const QString& widget_id, UIWidgetCallback callback) {
        Q_UNUSED(widget_id)
        Q_UNUSED(callback)
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "Widget callbacks not supported");
    }

    // === Settings Integration ===

    /**
     * @brief Get settings widget
     * @param parent Parent widget
     * @return Settings widget or error
     */
    virtual qtplugin::expected<QWidget*, PluginError> create_settings_widget(
        QWidget* parent = nullptr) {
        Q_UNUSED(parent)
        return make_error<QWidget*>(PluginErrorCode::CommandNotFound,
                                    "Settings widget not supported");
    }

    /**
     * @brief Apply settings
     * @param settings Settings data
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> apply_settings(
        const QJsonObject& settings) {
        Q_UNUSED(settings)
        return make_success();
    }

    /**
     * @brief Get current settings
     * @return Current settings data
     */
    virtual QJsonObject get_current_settings() const { return QJsonObject{}; }
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::UIComponentType)
Q_DECLARE_METATYPE(qtplugin::UIIntegrationPoint)
Q_DECLARE_METATYPE(qtplugin::UIIntegrationMode)
Q_DECLARE_METATYPE(qtplugin::UIActionInfo)
Q_DECLARE_METATYPE(qtplugin::UIWidgetInfo)
Q_DECLARE_METATYPE(qtplugin::UIThemeInfo)

Q_DECLARE_INTERFACE(qtplugin::IUIPlugin, "qtplugin.IUIPlugin/3.1")



// Register interface with validator
#include "interface_validator.hpp"

namespace {
    // Auto-register UI interface metadata
    struct UIInterfaceRegistrar {
        UIInterfaceRegistrar() {
            qtplugin::InterfaceMetadata metadata;
            metadata.interface_id = "qtplugin.IUIPlugin/3.1";
            metadata.version = qtplugin::Version{3, 1, 0};
            metadata.name = "UI Plugin Interface";
            metadata.description = "Unified UI plugin interface with comprehensive widget management";
            metadata.required_methods = {
                "create_widget(const QString&, QWidget*)",
                "supported_components()",
                "get_widget_info(const QString&)"
            };
            metadata.optional_methods = {
                "integration_mode()",
                "save_ui_state()",
                "restore_ui_state(const QJsonObject&)",
                "on_ui_setup_complete(QWidget*)",
                "on_ui_cleanup()"
            };
            metadata.dependencies = {"qtplugin.IPlugin/3.0"};

            qtplugin::global_interface_validator().register_interface(metadata);
        }
    };

    static UIInterfaceRegistrar ui_interface_registrar;
}
