/**
 * @file ui_plugin_interface.hpp
 * @brief UI plugin interface definitions using modern C++ features
 * @version 3.0.0
 */

#pragma once

#include "../core/plugin_interface.hpp"
#include <QWidget>
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QDockWidget>
#include <QKeySequence>
#include <QSize>
#include <memory>
#include <vector>
#include <string_view>
#include <optional>

namespace qtplugin {

/**
 * @brief UI integration modes
 */
enum class UIIntegrationMode {
    Standalone,     ///< Plugin provides standalone widgets
    Integrated,     ///< Plugin integrates with host application UI
    Overlay,        ///< Plugin provides overlay UI elements
    Modal,          ///< Plugin provides modal dialogs
    Embedded        ///< Plugin embeds in existing UI areas
};

/**
 * @brief Dock area preferences
 */
enum class DockArea {
    Left,
    Right,
    Top,
    Bottom,
    Center,
    Floating,
    Tabbed
};

/**
 * @brief UI theme support levels
 */
enum class ThemeSupport {
    None,           ///< No theme support
    Basic,          ///< Basic light/dark theme support
    Advanced,       ///< Full theme customization support
    Custom          ///< Plugin provides its own theming
};

/**
 * @brief UI plugin interface
 * 
 * This interface extends the base plugin interface with UI-specific
 * functionality for creating widgets, handling user interactions,
 * and integrating with host application UI.
 */
class IUIPlugin : public virtual IPlugin {
public:
    ~IUIPlugin() override = default;
    
    // === Widget Creation ===
    
    /**
     * @brief Create the main plugin widget
     * @param parent Parent widget (optional)
     * @return Unique pointer to the created widget
     */
    virtual std::unique_ptr<QWidget> create_widget(QWidget* parent = nullptr) = 0;
    
    /**
     * @brief Create a configuration widget
     * @param parent Parent widget (optional)
     * @return Unique pointer to the configuration widget, or nullptr if not supported
     */
    virtual std::unique_ptr<QWidget> create_configuration_widget(QWidget* parent = nullptr) {
        Q_UNUSED(parent)
        return nullptr;
    }
    
    /**
     * @brief Create a dock widget
     * @param parent Parent widget (optional)
     * @return Unique pointer to the dock widget, or nullptr if not supported
     */
    virtual std::unique_ptr<QDockWidget> create_dock_widget(QWidget* parent = nullptr) {
        Q_UNUSED(parent)
        return nullptr;
    }
    
    /**
     * @brief Create a status widget
     * @param parent Parent widget (optional)
     * @return Unique pointer to the status widget, or nullptr if not supported
     */
    virtual std::unique_ptr<QWidget> create_status_widget(QWidget* parent = nullptr) {
        Q_UNUSED(parent)
        return nullptr;
    }
    
    // === UI Integration ===
    
    /**
     * @brief Get UI integration mode
     * @return Preferred UI integration mode
     */
    virtual UIIntegrationMode integration_mode() const noexcept {
        return UIIntegrationMode::Standalone;
    }
    
    /**
     * @brief Get menu actions for integration
     * @return Vector of menu actions
     */
    virtual std::vector<QAction*> menu_actions() const {
        return {};
    }
    
    /**
     * @brief Get toolbar actions for integration
     * @return Vector of toolbar actions
     */
    virtual std::vector<QAction*> toolbar_actions() const {
        return {};
    }
    
    /**
     * @brief Get context menu actions
     * @return Vector of context menu actions
     */
    virtual std::vector<QAction*> context_menu_actions() const {
        return {};
    }
    
    /**
     * @brief Create a custom toolbar
     * @param parent Parent widget (optional)
     * @return Unique pointer to the toolbar, or nullptr if not supported
     */
    virtual std::unique_ptr<QToolBar> create_toolbar(QWidget* parent = nullptr) {
        Q_UNUSED(parent)
        return nullptr;
    }
    
    /**
     * @brief Create a custom menu
     * @param parent Parent widget (optional)
     * @return Unique pointer to the menu, or nullptr if not supported
     */
    virtual std::unique_ptr<QMenu> create_menu(QWidget* parent = nullptr) {
        Q_UNUSED(parent)
        return nullptr;
    }
    
    // === Layout and Sizing ===
    
    /**
     * @brief Get preferred dock area
     * @return Preferred dock area for the plugin widget
     */
    virtual DockArea preferred_dock_area() const noexcept {
        return DockArea::Center;
    }
    
    /**
     * @brief Get minimum widget size
     * @return Minimum size for the plugin widget
     */
    virtual QSize minimum_size() const noexcept {
        return QSize{200, 150};
    }
    
    /**
     * @brief Get preferred widget size
     * @return Preferred size for the plugin widget
     */
    virtual QSize preferred_size() const noexcept {
        return QSize{400, 300};
    }
    
    /**
     * @brief Get maximum widget size
     * @return Maximum size for the plugin widget, or invalid size for no limit
     */
    virtual QSize maximum_size() const noexcept {
        return QSize{}; // Invalid size means no maximum
    }
    
    /**
     * @brief Check if widget is resizable
     * @return true if the widget can be resized
     */
    virtual bool is_resizable() const noexcept {
        return true;
    }
    
    /**
     * @brief Get size policy
     * @return Size policy for the plugin widget
     */
    virtual std::pair<QSizePolicy::Policy, QSizePolicy::Policy> size_policy() const noexcept {
        return {QSizePolicy::Preferred, QSizePolicy::Preferred};
    }
    
    // === Keyboard Shortcuts ===
    
    /**
     * @brief Get keyboard shortcuts
     * @return Vector of keyboard shortcuts used by the plugin
     */
    virtual std::vector<QKeySequence> keyboard_shortcuts() const {
        return {};
    }
    
    /**
     * @brief Register keyboard shortcuts with parent widget
     * @param parent Parent widget to register shortcuts with
     */
    virtual void register_shortcuts(QWidget* parent) {
        Q_UNUSED(parent)
        // Default implementation does nothing
    }
    
    /**
     * @brief Unregister keyboard shortcuts
     * @param parent Parent widget to unregister shortcuts from
     */
    virtual void unregister_shortcuts(QWidget* parent) {
        Q_UNUSED(parent)
        // Default implementation does nothing
    }
    
    // === Theming ===
    
    /**
     * @brief Get theme support level
     * @return Level of theme support provided by the plugin
     */
    virtual ThemeSupport theme_support() const noexcept {
        return ThemeSupport::Basic;
    }
    
    /**
     * @brief Apply theme to plugin UI
     * @param theme_name Name of the theme to apply
     * @param theme_data Theme configuration data
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> apply_theme(std::string_view theme_name,
                                                        const QJsonObject& theme_data = {}) {
        Q_UNUSED(theme_name)
        Q_UNUSED(theme_data)
        return make_success();
    }
    
    /**
     * @brief Get supported themes
     * @return Vector of supported theme names
     */
    virtual std::vector<std::string> supported_themes() const {
        return {"default", "light", "dark"};
    }
    
    /**
     * @brief Get current theme
     * @return Name of the currently applied theme
     */
    virtual std::string current_theme() const {
        return "default";
    }
    
    // === UI State Management ===
    
    /**
     * @brief Save UI state
     * @return UI state as JSON object
     */
    virtual QJsonObject save_ui_state() const {
        return QJsonObject{};
    }
    
    /**
     * @brief Restore UI state
     * @param state UI state to restore
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> restore_ui_state(const QJsonObject& state) {
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
    
    // === Event Handling ===
    
    /**
     * @brief Handle UI setup completion
     * 
     * Called after the plugin widget has been created and integrated
     * into the host application UI.
     * 
     * @param main_window Pointer to the main application window
     */
    virtual void on_ui_setup_complete(QWidget* main_window) {
        Q_UNUSED(main_window)
        // Default implementation does nothing
    }
    
    /**
     * @brief Handle UI cleanup
     * 
     * Called before the plugin widget is destroyed or removed
     * from the host application UI.
     */
    virtual void on_ui_cleanup() {
        // Default implementation does nothing
    }
    
    /**
     * @brief Handle focus gained
     * 
     * Called when the plugin widget gains focus.
     */
    virtual void on_focus_gained() {
        // Default implementation does nothing
    }
    
    /**
     * @brief Handle focus lost
     * 
     * Called when the plugin widget loses focus.
     */
    virtual void on_focus_lost() {
        // Default implementation does nothing
    }
    
    /**
     * @brief Handle visibility change
     * @param visible true if the widget became visible, false if hidden
     */
    virtual void on_visibility_changed(bool visible) {
        Q_UNUSED(visible)
        // Default implementation does nothing
    }
    
    // === Accessibility ===
    
    /**
     * @brief Get accessibility information
     * @return Accessibility information as JSON object
     */
    virtual QJsonObject accessibility_info() const {
        return QJsonObject{
            {"accessible", true},
            {"screen_reader_compatible", true},
            {"keyboard_navigable", true}
        };
    }
    
    /**
     * @brief Check if plugin supports accessibility features
     * @return true if accessibility features are supported
     */
    virtual bool supports_accessibility() const noexcept {
        return true;
    }
    
    // === Validation ===
    
    /**
     * @brief Validate UI requirements
     * @param parent_widget Parent widget that will host the plugin
     * @return Success or error information with validation details
     */
    virtual qtplugin::expected<void, PluginError> validate_ui_requirements(QWidget* parent_widget) const {
        Q_UNUSED(parent_widget)
        return make_success();
    }
};

/**
 * @brief UI plugin factory interface
 * 
 * Factory interface for creating UI plugin instances with specific
 * UI requirements and constraints.
 */
class IUIPluginFactory {
public:
    virtual ~IUIPluginFactory() = default;
    
    /**
     * @brief Create UI plugin instance
     * @param parent Parent widget for the plugin
     * @param config Plugin configuration
     * @return Unique pointer to the created plugin
     */
    virtual std::unique_ptr<IUIPlugin> create_ui_plugin(QWidget* parent = nullptr,
                                                       const QJsonObject& config = {}) = 0;
    
    /**
     * @brief Check if factory can create plugin with given requirements
     * @param requirements UI requirements
     * @return true if plugin can be created with the requirements
     */
    virtual bool can_create_with_requirements(const QJsonObject& requirements) const = 0;
    
    /**
     * @brief Get supported UI integration modes
     * @return Vector of supported integration modes
     */
    virtual std::vector<UIIntegrationMode> supported_integration_modes() const = 0;
};

} // namespace qtplugin

Q_DECLARE_INTERFACE(qtplugin::IUIPlugin, "qtplugin.IUIPlugin/3.0")
Q_DECLARE_INTERFACE(qtplugin::IUIPluginFactory, "qtplugin.IUIPluginFactory/3.0")
