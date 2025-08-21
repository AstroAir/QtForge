/**
 * @file ui_plugin_interface.hpp
 * @brief UI plugin interface for C++ widget-based user interface components
 * @version 3.0.0
 */

#pragma once

#include "../core/plugin_interface.hpp"
#include <QObject>
#include <QWidget>
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QDockWidget>
#include <QDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QIcon>
#include <QKeySequence>
#include <QMetaType>
#include <functional>
#include <memory>
#include <string_view>
#include <vector>
#include <optional>

namespace qtplugin {

/**
 * @brief UI component types
 */
enum class UIComponentType : uint32_t {
    None = 0x0000,
    Widget = 0x0001,         ///< Custom widget
    Dialog = 0x0002,         ///< Dialog window
    DockWidget = 0x0004,     ///< Dockable widget
    ToolBar = 0x0008,        ///< Toolbar
    MenuBar = 0x0010,        ///< Menu bar
    ContextMenu = 0x0020,    ///< Context menu
    StatusBar = 0x0040,      ///< Status bar widget
    PropertyEditor = 0x0080, ///< Property editor
    TreeView = 0x0100,       ///< Tree view component
    ListView = 0x0200,       ///< List view component
    TableView = 0x0400,      ///< Table view component
    GraphicsView = 0x0800,   ///< Graphics view component
    CustomControl = 0x1000,  ///< Custom control
    Wizard = 0x2000,         ///< Wizard dialog
    Settings = 0x4000        ///< Settings interface
};

using UIComponentTypes = std::underlying_type_t<UIComponentType>;

/**
 * @brief UI integration points
 */
enum class UIIntegrationPoint {
    MainWindow,              ///< Main application window
    MenuBar,                 ///< Application menu bar
    ToolBar,                 ///< Application toolbar
    StatusBar,               ///< Application status bar
    DockArea,                ///< Dockable area
    CentralWidget,           ///< Central widget area
    ContextMenu,             ///< Context menus
    SettingsDialog,          ///< Settings/preferences dialog
    AboutDialog,             ///< About dialog
    CustomArea               ///< Custom integration area
};

/**
 * @brief UI action information
 */
struct UIActionInfo {
    QString id;                             ///< Action identifier
    QString text;                           ///< Action text
    QString tooltip;                        ///< Action tooltip
    QString status_tip;                     ///< Status bar tip
    QIcon icon;                             ///< Action icon
    QKeySequence shortcut;                  ///< Keyboard shortcut
    bool checkable = false;                 ///< Whether action is checkable
    bool checked = false;                   ///< Initial checked state
    bool enabled = true;                    ///< Whether action is enabled
    bool visible = true;                    ///< Whether action is visible
    QString menu_path;                      ///< Menu path (e.g., "File/Export")
    int priority = 0;                       ///< Display priority
    QJsonObject custom_data;                ///< Custom action data
    
    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON object
     */
    static UIActionInfo from_json(const QJsonObject& json);
};

/**
 * @brief UI widget information
 */
struct UIWidgetInfo {
    QString id;                             ///< Widget identifier
    QString title;                          ///< Widget title
    QString description;                    ///< Widget description
    QIcon icon;                             ///< Widget icon
    UIComponentType type;                   ///< Widget type
    UIIntegrationPoint integration_point;   ///< Integration point
    QSize preferred_size;                   ///< Preferred size
    QSize minimum_size;                     ///< Minimum size
    QSize maximum_size;                     ///< Maximum size
    bool resizable = true;                  ///< Whether widget is resizable
    bool closable = true;                   ///< Whether widget is closable
    bool floatable = true;                  ///< Whether widget can float
    Qt::DockWidgetAreas allowed_areas = Qt::AllDockWidgetAreas; ///< Allowed dock areas
    QJsonObject custom_properties;         ///< Custom widget properties
    
    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON object
     */
    static UIWidgetInfo from_json(const QJsonObject& json);
};

/**
 * @brief UI theme information
 */
struct UIThemeInfo {
    QString name;                           ///< Theme name
    QString description;                    ///< Theme description
    QString stylesheet;                     ///< CSS stylesheet
    QJsonObject color_scheme;               ///< Color scheme
    QJsonObject font_settings;             ///< Font settings
    QJsonObject icon_theme;                 ///< Icon theme settings
    bool dark_mode = false;                 ///< Whether it's a dark theme
    
    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief UI event callback types
 */
using UIActionCallback = std::function<void(const QString& action_id, bool checked)>;
using UIWidgetCallback = std::function<void(const QString& widget_id, const QString& event)>;

/**
 * @brief UI plugin interface
 * 
 * This interface extends the base plugin interface with user interface
 * capabilities for creating C++ widget-based UI components.
 * 
 * Note: This interface is designed for pure C++ applications and does not
 * depend on QML. All UI components are Qt Widget-based.
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
        return (supported_components() & static_cast<UIComponentTypes>(component)) != 0;
    }
    
    /**
     * @brief Get supported integration points
     * @return Vector of supported integration points
     */
    virtual std::vector<UIIntegrationPoint> supported_integration_points() const = 0;
    
    // === Widget Management ===
    
    /**
     * @brief Create widget
     * @param widget_id Widget identifier
     * @param parent Parent widget
     * @return Created widget or error
     */
    virtual qtplugin::expected<QWidget*, PluginError>
    create_widget(const QString& widget_id, QWidget* parent = nullptr) = 0;
    
    /**
     * @brief Get widget information
     * @param widget_id Widget identifier
     * @return Widget information or error
     */
    virtual qtplugin::expected<UIWidgetInfo, PluginError>
    get_widget_info(const QString& widget_id) const = 0;
    
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
    virtual qtplugin::expected<void, PluginError>
    destroy_widget(const QString& widget_id) = 0;
    
    // === Action Management ===
    
    /**
     * @brief Create action
     * @param action_info Action information
     * @param parent Parent object
     * @return Created action or error
     */
    virtual qtplugin::expected<QAction*, PluginError>
    create_action(const UIActionInfo& action_info, QObject* parent = nullptr) = 0;
    
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
    virtual qtplugin::expected<void, PluginError>
    set_action_callback(const QString& action_id, UIActionCallback callback) = 0;
    
    /**
     * @brief Remove action
     * @param action_id Action identifier
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError>
    remove_action(const QString& action_id) = 0;
    
    // === Menu and Toolbar Support ===
    
    /**
     * @brief Create menu
     * @param menu_id Menu identifier
     * @param title Menu title
     * @param parent Parent widget
     * @return Created menu or error
     */
    virtual qtplugin::expected<QMenu*, PluginError>
    create_menu(const QString& menu_id, const QString& title, QWidget* parent = nullptr) = 0;
    
    /**
     * @brief Create toolbar
     * @param toolbar_id Toolbar identifier
     * @param title Toolbar title
     * @param parent Parent widget
     * @return Created toolbar or error
     */
    virtual qtplugin::expected<QToolBar*, PluginError>
    create_toolbar(const QString& toolbar_id, const QString& title, QWidget* parent = nullptr) = 0;
    
    // === Dialog Support ===
    
    /**
     * @brief Create dialog
     * @param dialog_id Dialog identifier
     * @param parent Parent widget
     * @return Created dialog or error
     */
    virtual qtplugin::expected<QDialog*, PluginError>
    create_dialog(const QString& dialog_id, QWidget* parent = nullptr) = 0;
    
    /**
     * @brief Show modal dialog
     * @param dialog_id Dialog identifier
     * @return Dialog result or error
     */
    virtual qtplugin::expected<int, PluginError>
    show_modal_dialog(const QString& dialog_id) = 0;
    
    // === Theme Support ===
    
    /**
     * @brief Get available themes
     * @return Vector of available theme information
     */
    virtual std::vector<UIThemeInfo> get_available_themes() const {
        return {};
    }
    
    /**
     * @brief Apply theme
     * @param theme_name Theme name
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError>
    apply_theme(const QString& theme_name) {
        Q_UNUSED(theme_name)
        return make_error<void>(PluginErrorCode::CommandNotFound, "Theme support not implemented");
    }
    
    /**
     * @brief Get current theme
     * @return Current theme name
     */
    virtual QString get_current_theme() const {
        return "default";
    }
    
    // === Event Handling ===
    
    /**
     * @brief Set widget event callback
     * @param widget_id Widget identifier
     * @param callback Widget event callback
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError>
    set_widget_callback(const QString& widget_id, UIWidgetCallback callback) {
        Q_UNUSED(widget_id)
        Q_UNUSED(callback)
        return make_error<void>(PluginErrorCode::CommandNotFound, "Widget callbacks not supported");
    }
    
    // === Settings Integration ===
    
    /**
     * @brief Get settings widget
     * @param parent Parent widget
     * @return Settings widget or error
     */
    virtual qtplugin::expected<QWidget*, PluginError>
    create_settings_widget(QWidget* parent = nullptr) {
        Q_UNUSED(parent)
        return make_error<QWidget*>(PluginErrorCode::CommandNotFound, "Settings widget not supported");
    }
    
    /**
     * @brief Apply settings
     * @param settings Settings data
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError>
    apply_settings(const QJsonObject& settings) {
        Q_UNUSED(settings)
        return make_success();
    }
    
    /**
     * @brief Get current settings
     * @return Current settings data
     */
    virtual QJsonObject get_current_settings() const {
        return QJsonObject{};
    }
};

} // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::UIComponentType)
Q_DECLARE_METATYPE(qtplugin::UIIntegrationPoint)
Q_DECLARE_METATYPE(qtplugin::UIActionInfo)
Q_DECLARE_METATYPE(qtplugin::UIWidgetInfo)
Q_DECLARE_METATYPE(qtplugin::UIThemeInfo)

Q_DECLARE_INTERFACE(qtplugin::IUIPlugin, "qtplugin.IUIPlugin/3.0")
