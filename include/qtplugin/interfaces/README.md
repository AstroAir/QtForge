# QtPlugin Interfaces

This directory contains extended plugin interfaces that provide specialized functionality beyond the core `IPlugin` interface.

## Available Interfaces

### UI Plugin Interface (`ui_plugin_interface.hpp`)

**Version**: 3.1.0
**Interface ID**: `qtplugin.IUIPlugin/3.1`

The unified UI plugin interface combines comprehensive enterprise features with modern C++ patterns for creating widget-based UI components. This interface consolidates the best features from both comprehensive and streamlined approaches.

#### Key Features

- **Comprehensive Component Management**: Support for widgets, dialogs, toolbars, menus, and more
- **Modern Widget Creation**: Smart pointer-based widget management with `std::unique_ptr<QWidget>`
- **Accessibility Support**: Built-in accessibility features and high contrast support
- **UI State Management**: Save/restore UI state with JSON serialization
- **Enhanced Integration**: Multiple integration modes (Standalone, Integrated, Overlay, Modal, Embedded)
- **Theme Support**: Comprehensive theming with dark mode and accessibility settings
- **Layout Management**: Size policies, minimum/maximum sizes, and resizing capabilities
- **Event Handling**: Lifecycle events (setup, cleanup, focus, visibility changes)

#### Integration Modes

```cpp
enum class UIIntegrationMode {
    Standalone,  ///< Plugin provides standalone widgets
    Integrated,  ///< Plugin integrates with host application UI
    Overlay,     ///< Plugin provides overlay UI elements
    Modal,       ///< Plugin provides modal dialogs
    Embedded     ///< Plugin embeds in existing UI areas
};
```

#### Basic Usage

```cpp
class MyUIPlugin : public QObject, public qtplugin::IUIPlugin {
    Q_OBJECT
    Q_INTERFACES(qtplugin::IUIPlugin)
    Q_PLUGIN_METADATA(IID "qtplugin.IUIPlugin/3.1" FILE "plugin.json")

public:
    // Required: Create widget with identifier
    qtplugin::expected<QWidget*, PluginError> create_widget(
        const QString& widget_id, QWidget* parent = nullptr) override {

        if (widget_id == "main_widget") {
            return new MyMainWidget(parent);
        }
        return qtplugin::make_error<QWidget*>(
            qtplugin::PluginErrorCode::CommandNotFound,
            "Unknown widget ID");
    }

    // Optional: Specify integration mode
    UIIntegrationMode integration_mode() const noexcept override {
        return UIIntegrationMode::Integrated;
    }

    // Optional: Handle UI lifecycle events
    void on_ui_setup_complete(QWidget* main_window) override {
        // Initialize UI after integration
    }
};
```

#### Data Structures

The interface includes comprehensive data structures with JSON serialization:

- **`UIActionInfo`**: Action metadata with shortcuts, icons, and menu paths
- **`UIWidgetInfo`**: Widget metadata with sizing, accessibility, and integration settings
- **`UIThemeInfo`**: Theme information with dark mode and accessibility support

All structures include `to_json()` and `from_json()` methods for persistence.

### Data Processor Plugin Interface (`data_processor_plugin_interface.hpp`)

Provides data processing capabilities with support for various data formats and transformation operations.

### Network Plugin Interface (`network_plugin_interface.hpp`)

Enables network communication capabilities including HTTP requests, WebSocket connections, and custom protocols.

### Scripting Plugin Interface (`scripting_plugin_interface.hpp`)

Allows plugins to execute scripts in various languages (JavaScript, Python, Lua) with secure sandboxing.

## Interface Versioning

All interfaces use semantic versioning in their Qt interface IDs:

- **Major version**: Breaking changes to the interface
- **Minor version**: New features, backward compatible
- **Patch version**: Bug fixes, fully backward compatible

Example: `qtplugin.IUIPlugin/3.1` indicates major version 3, minor version 1.

## Migration Guide

### From UI Interface 3.0 to 3.1

The unified interface (3.1) consolidates two previous interface versions:

1. **Enhanced Features**: New accessibility support, UI state management, and layout methods
2. **Updated Interface ID**: Change from `qtplugin.IUIPlugin/3.0` to `qtplugin.IUIPlugin/3.1`
3. **New Integration Modes**: `UIIntegrationMode` enum for better host integration
4. **Lifecycle Events**: New event handlers for UI setup, cleanup, focus, and visibility

#### Required Changes

```cpp
// Update interface ID in Q_PLUGIN_METADATA
Q_PLUGIN_METADATA(IID "qtplugin.IUIPlugin/3.1" FILE "plugin.json")

// Optional: Implement new lifecycle methods
void on_ui_setup_complete(QWidget* main_window) override;
void on_ui_cleanup() override;
void on_focus_gained() override;
void on_focus_lost() override;
void on_visibility_changed(bool visible) override;
```

## Best Practices

1. **Interface Versioning**: Always specify the exact interface version in `Q_PLUGIN_METADATA`
2. **Error Handling**: Use `qtplugin::expected<T, PluginError>` for all operations that can fail
3. **Resource Management**: Use smart pointers for widget management
4. **Accessibility**: Implement accessibility features for inclusive design
5. **State Persistence**: Implement `save_ui_state()` and `restore_ui_state()` for better UX
6. **Documentation**: Document all custom widgets and their capabilities

## See Also

- [Core Plugin Interface](../core/plugin_interface.hpp)
- [Error Handling Utilities](../utils/error_handling.hpp)
- [Version Management](../utils/version.hpp)
- [UI Plugin Examples](../../../examples/04-specialized/ui-integration/)
