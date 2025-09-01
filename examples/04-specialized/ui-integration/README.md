# UI Plugin Example

This comprehensive UI plugin demonstrates advanced QtPlugin system UI capabilities including Qt Widgets integration, theme support, dialog management, and complete UI component lifecycle management.

## Overview

The UI Plugin showcases **complete UI integration** patterns including:

- ✅ **Qt Widgets Integration** with custom controls and layouts
- ✅ **Theme Support** with 5 built-in themes and dynamic theme switching
- ✅ **Dialog Management** with modal dialogs and settings integration
- ✅ **Action Management** with menu integration and callback handling
- ✅ **Widget Lifecycle** with creation, management, and cleanup
- ✅ **Settings Integration** with persistent configuration
- ✅ **Comprehensive UI Components** (TreeView, ListView, TableView, etc.)
- ✅ **Thread Safety** with main-thread UI operations

## Architecture

### Core Components

1. **UIPlugin** - Main plugin class implementing both IPlugin and IUIPlugin interfaces
2. **DemoWidget** - Comprehensive demo widget showcasing various Qt controls
3. **SettingsDialog** - Configuration dialog with tabbed interface
4. **AboutDialog** - Information dialog with plugin details

### UI Component Hierarchy

```
UIPlugin (IPlugin + IUIPlugin)
├── Widget Management
│   ├── DemoWidget (Tabbed interface with controls)
│   └── SettingsDialog (Configuration interface)
├── Theme Management
│   ├── Default Theme
│   ├── Dark Theme
│   ├── Light Theme
│   ├── Blue Theme
│   └── Green Theme
├── Action Management
│   ├── Show Demo Action
│   ├── Show Settings Action
│   └── Show About Action
└── Dialog Management
    ├── Settings Dialog
    └── About Dialog
```

### Threading Model

- **Main Thread Only**: All UI operations run on the main Qt thread
- **Thread Safety**: Full synchronization for plugin state management
- **UI Thread Required**: Plugin requires UI thread for proper operation

## Features

### Widget Management

The plugin demonstrates comprehensive widget management:

- **Dynamic Widget Creation**: On-demand widget instantiation
- **Widget Information**: Metadata tracking for all widgets
- **Widget Lifecycle**: Proper creation, management, and cleanup
- **Parent-Child Relationships**: Correct Qt object hierarchy

### Theme Support

Advanced theme system with 5 built-in themes:

- **Default Theme**: System default appearance
- **Dark Theme**: High-contrast dark interface
- **Light Theme**: Soft light colors
- **Blue Theme**: Blue-themed interface with custom styling
- **Green Theme**: Green-themed interface with nature colors

Each theme includes:

- Custom color schemes
- Styled controls (buttons, inputs, groups)
- Consistent visual language
- Dynamic theme switching

### Dialog System

Comprehensive dialog management:

- **Modal Dialogs**: Proper modal dialog handling
- **Settings Dialog**: Multi-tab configuration interface
- **About Dialog**: Plugin information display
- **Dialog Lifecycle**: Creation, display, and cleanup

### Action System

Complete action management with:

- **Action Creation**: Dynamic action instantiation
- **Callback System**: Custom callback handling
- **Menu Integration**: Menu path and priority support
- **Action Metadata**: Comprehensive action information

## Available Commands

### Widget Management Commands

- **`widget`** - Widget lifecycle management
  - `create` - Create new widget instance
  - `list` - List available widget types
  - `destroy` - Destroy widget instance

### Theme Management Commands

- **`theme`** - Theme system operations
  - `list` - List available themes with metadata
  - `apply` - Apply specific theme
  - `current` - Get current theme name

### Dialog Management Commands

- **`dialog`** - Dialog system operations
  - `create` - Create dialog instance
  - `show` - Show modal dialog
  - `list` - List available dialog types

### Action Management Commands

- **`action`** - Action system operations
  - `list` - List available actions with metadata
  - `trigger` - Trigger specific action

### Settings Management Commands

- **`settings`** - Configuration management
  - `get` - Get current settings
  - `set` - Apply new settings
  - `reset` - Reset to default settings

### Status Command

- **`status`** - Get comprehensive plugin status

## Configuration

The UI plugin supports extensive configuration:

```json
{
  "default_theme": "default", // Default theme (default|dark|light|blue|green)
  "logging_enabled": true, // Enable/disable logging
  "auto_save_enabled": true, // Enable automatic state saving
  "refresh_interval": 1000, // Widget refresh interval (100-10000ms)
  "show_tooltips": true, // Show tooltips on UI elements
  "enable_animations": true, // Enable UI animations
  "window_opacity": 1.0 // Window opacity (0.0-1.0)
}
```

## Dependencies

- **Required**: None (standalone UI plugin)
- **Optional**:
  - `qtplugin.ConfigurationManager` - For centralized configuration
  - `qtplugin.ThemeManager` - For advanced theme management

## Building

```bash
cd examples/ui_plugin
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DQTPLUGIN_BUILD_UI=ON
cmake --build .
```

**Note**: Requires `QTPLUGIN_BUILD_UI=ON` to enable UI plugin support.

## Testing

```bash
# Run the comprehensive test application
./UIPluginTest

# Run unit tests
ctest --output-on-failure
```

The test application provides a complete UI for testing all plugin functionality.

## Usage Examples

### Basic UI Plugin Usage

```cpp
#include "ui_plugin.hpp"

// Create and initialize UI plugin
auto plugin = std::make_unique<UIPlugin>();
auto init_result = plugin->initialize();

if (init_result) {
    // Create demo widget
    auto widget_result = plugin->execute_command("widget",
        QJsonObject{{"action", "create"}, {"widget_id", "demo_widget"}});

    // Apply dark theme
    auto theme_result = plugin->execute_command("theme",
        QJsonObject{{"action", "apply"}, {"theme_name", "dark"}});

    // Show settings dialog
    auto dialog_result = plugin->execute_command("dialog",
        QJsonObject{{"action", "show"}, {"dialog_id", "settings"}});
}
```

### Advanced Widget Integration

```cpp
// Using IUIPlugin interface directly
auto ui_plugin = dynamic_cast<qtplugin::IUIPlugin*>(plugin.get());
if (ui_plugin) {
    // Create widget using interface
    auto widget = ui_plugin->create_widget("demo_widget", parent_widget);
    if (widget) {
        widget.value()->show();
    }

    // Get widget information
    auto widget_info = ui_plugin->get_widget_info("demo_widget");
    if (widget_info) {
        qDebug() << "Widget title:" << widget_info.value().title;
        qDebug() << "Widget type:" << static_cast<int>(widget_info.value().type);
    }
}
```

### Theme Management Integration

```cpp
// Get available themes
auto themes = ui_plugin->get_available_themes();
for (const auto& theme : themes) {
    qDebug() << "Theme:" << theme.name
             << "Display:" << theme.display_name
             << "Dark:" << theme.is_dark;
}

// Apply theme with error handling
auto theme_result = ui_plugin->apply_theme("dark");
if (!theme_result) {
    qWarning() << "Failed to apply theme:"
               << QString::fromStdString(theme_result.error().message);
}

// Get current theme
QString current_theme = ui_plugin->get_current_theme();
```

### Action System Integration

```cpp
// Create custom action
qtplugin::UIActionInfo action_info;
action_info.id = "custom_action";
action_info.text = "Custom Action";
action_info.tooltip = "Perform custom operation";
action_info.menu_path = "Tools/Custom";

auto action = ui_plugin->create_action(action_info, parent);
if (action) {
    // Set callback
    ui_plugin->set_action_callback("custom_action", []() {
        qDebug() << "Custom action triggered!";
    });
}
```

## Performance Characteristics

- **Memory Usage**: ~2MB base + widget overhead
- **CPU Usage**: ~0.2% idle (UI operations are event-driven)
- **Thread Count**: 1 (main thread only)
- **UI Responsiveness**: High-performance Qt Widgets integration
- **Theme Switching**: Instant theme application

## Supported UI Components

The plugin supports all major Qt Widgets components:

- **Basic Controls**: QLineEdit, QPushButton, QCheckBox, QRadioButton
- **Input Controls**: QComboBox, QSpinBox, QSlider, QTextEdit
- **Display Controls**: QLabel, QProgressBar, QGroupBox
- **Container Controls**: QTabWidget, QSplitter, QScrollArea
- **Item Views**: QTreeWidget, QListWidget, QTableWidget
- **Dialogs**: QDialog, QMessageBox, custom dialogs
- **Menus & Toolbars**: QMenu, QToolBar, QAction

## Integration Points

The plugin integrates with:

- **MainWindow**: Central widget and dock area integration
- **MenuBar**: Menu and action integration
- **ToolBar**: Toolbar and action integration
- **ContextMenu**: Right-click menu integration
- **SettingsDialog**: Configuration interface integration
- **DockArea**: Dockable widget support

## Real-World Applications

This UI plugin pattern is suitable for:

- **IDE Plugins** - Code editors, debuggers, project explorers
- **CAD Applications** - Tool palettes, property editors
- **Media Applications** - Control panels, timeline editors
- **Scientific Software** - Data visualization, analysis tools
- **Business Applications** - Dashboard widgets, report generators

## Integration with Other Examples

The UI plugin demonstrates patterns used by:

- **Enhanced Basic Plugin** - Foundation plugin patterns
- **Service Plugin** - Background processing with UI feedback
- **Configuration Example** - Settings management integration
- **Theme Management** - Visual customization patterns

## License

MIT License - Same as QtPlugin library
