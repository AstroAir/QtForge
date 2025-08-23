/**
 * @file ui_plugin.cpp
 * @brief Implementation of comprehensive UI plugin
 * @version 3.0.0
 *
 * This implementation demonstrates advanced UI patterns with
 * Qt Widgets integration, theme support, and comprehensive UI management.
 */

#include "ui_plugin.hpp"
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QStyleFactory>
#include <chrono>
#include <thread>

// === DemoWidget Implementation ===

DemoWidget::DemoWidget(QWidget* parent)
    : QWidget(parent),
      m_main_layout(nullptr),
      m_tab_widget(nullptr),
      m_update_timer(new QTimer(this)) {
    setupUI();
    setupConnections();

    // Setup update timer
    m_update_timer->setInterval(1000);
    connect(m_update_timer, &QTimer::timeout, this,
            &DemoWidget::on_value_changed);
    m_update_timer->start();
}

DemoWidget::~DemoWidget() {
    if (m_update_timer) {
        m_update_timer->stop();
    }
}

void DemoWidget::setupUI() {
    setWindowTitle("UI Plugin Demo Widget");
    setMinimumSize(600, 400);

    m_main_layout = new QVBoxLayout(this);
    m_tab_widget = new QTabWidget(this);
    m_main_layout->addWidget(m_tab_widget);

    // Basic Controls Tab
    m_basic_tab = new QWidget();
    auto basic_layout = new QGridLayout(m_basic_tab);

    // Row 0
    basic_layout->addWidget(new QLabel("Text Input:"), 0, 0);
    m_line_edit = new QLineEdit("Sample text");
    basic_layout->addWidget(m_line_edit, 0, 1);

    // Row 1
    basic_layout->addWidget(new QLabel("Multi-line Text:"), 1, 0);
    m_text_edit = new QTextEdit("Sample multi-line text\nLine 2\nLine 3");
    m_text_edit->setMaximumHeight(80);
    basic_layout->addWidget(m_text_edit, 1, 1);

    // Row 2
    m_push_button = new QPushButton("Click Me!");
    basic_layout->addWidget(m_push_button, 2, 0);

    m_check_box = new QCheckBox("Enable Feature");
    m_check_box->setChecked(true);
    basic_layout->addWidget(m_check_box, 2, 1);

    // Row 3
    m_radio_button1 = new QRadioButton("Option 1");
    m_radio_button1->setChecked(true);
    basic_layout->addWidget(m_radio_button1, 3, 0);

    m_radio_button2 = new QRadioButton("Option 2");
    basic_layout->addWidget(m_radio_button2, 3, 1);

    // Row 4
    basic_layout->addWidget(new QLabel("Combo Box:"), 4, 0);
    m_combo_box = new QComboBox();
    m_combo_box->addItems({"Item 1", "Item 2", "Item 3", "Item 4"});
    basic_layout->addWidget(m_combo_box, 4, 1);

    // Row 5
    basic_layout->addWidget(new QLabel("Spin Box:"), 5, 0);
    m_spin_box = new QSpinBox();
    m_spin_box->setRange(0, 100);
    m_spin_box->setValue(50);
    basic_layout->addWidget(m_spin_box, 5, 1);

    // Row 6
    basic_layout->addWidget(new QLabel("Slider:"), 6, 0);
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(0, 100);
    m_slider->setValue(75);
    basic_layout->addWidget(m_slider, 6, 1);

    // Row 7
    basic_layout->addWidget(new QLabel("Progress:"), 7, 0);
    m_progress_bar = new QProgressBar();
    m_progress_bar->setRange(0, 100);
    m_progress_bar->setValue(60);
    basic_layout->addWidget(m_progress_bar, 7, 1);

    m_tab_widget->addTab(m_basic_tab, "Basic Controls");

    // Advanced Controls Tab
    m_advanced_tab = new QWidget();
    auto advanced_layout = new QHBoxLayout(m_advanced_tab);

    // Tree Widget
    m_tree_widget = new QTreeWidget();
    m_tree_widget->setHeaderLabels({"Name", "Value", "Type"});
    auto root_item =
        new QTreeWidgetItem(m_tree_widget, {"Root", "root_value", "container"});
    auto child1 =
        new QTreeWidgetItem(root_item, {"Child 1", "child1_value", "item"});
    new QTreeWidgetItem(root_item, {"Child 2", "child2_value", "item"});
    new QTreeWidgetItem(child1, {"Sub Child", "sub_value", "item"});
    m_tree_widget->expandAll();
    advanced_layout->addWidget(m_tree_widget);

    // List Widget
    m_list_widget = new QListWidget();
    m_list_widget->addItems({"List Item 1", "List Item 2", "List Item 3",
                             "List Item 4", "List Item 5"});
    advanced_layout->addWidget(m_list_widget);

    // Table Widget
    m_table_widget = new QTableWidget(4, 3);
    m_table_widget->setHorizontalHeaderLabels(
        {"Column 1", "Column 2", "Column 3"});
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 3; ++col) {
            m_table_widget->setItem(
                row, col,
                new QTableWidgetItem(
                    QString("Cell %1,%2").arg(row + 1).arg(col + 1)));
        }
    }
    advanced_layout->addWidget(m_table_widget);

    m_tab_widget->addTab(m_advanced_tab, "Advanced Controls");

    // Settings Tab
    m_settings_tab = new QWidget();
    auto settings_layout = new QVBoxLayout(m_settings_tab);

    // Theme Group
    m_theme_group = new QGroupBox("Theme Settings");
    auto theme_layout = new QVBoxLayout(m_theme_group);

    theme_layout->addWidget(new QLabel("Select Theme:"));
    m_theme_combo = new QComboBox();
    m_theme_combo->addItems({"default", "dark", "light", "blue", "green"});
    theme_layout->addWidget(m_theme_combo);

    settings_layout->addWidget(m_theme_group);

    // Options Group
    m_options_group = new QGroupBox("Options");
    auto options_layout = new QVBoxLayout(m_options_group);

    options_layout->addWidget(new QCheckBox("Auto-save enabled"));
    options_layout->addWidget(new QCheckBox("Show tooltips"));
    options_layout->addWidget(new QCheckBox("Enable animations"));

    settings_layout->addWidget(m_options_group);
    settings_layout->addStretch();

    m_tab_widget->addTab(m_settings_tab, "Settings");
}

void DemoWidget::setupConnections() {
    connect(m_push_button, &QPushButton::clicked, this,
            &DemoWidget::on_button_clicked);
    connect(m_line_edit, &QLineEdit::textChanged, this,
            &DemoWidget::on_text_changed);
    connect(m_text_edit, &QTextEdit::textChanged, this,
            &DemoWidget::on_text_changed);
    connect(m_spin_box, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &DemoWidget::on_value_changed);
    connect(m_slider, &QSlider::valueChanged, this,
            &DemoWidget::on_value_changed);
    connect(m_combo_box, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DemoWidget::on_selection_changed);
    connect(m_theme_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() {
                QString theme = m_theme_combo->currentText();
                setTheme(theme);
                emit actionTriggered(QString("theme_changed:%1").arg(theme));
            });

    connect(m_tree_widget, &QTreeWidget::itemSelectionChanged, this,
            &DemoWidget::on_selection_changed);
    connect(m_list_widget, &QListWidget::itemSelectionChanged, this,
            &DemoWidget::on_selection_changed);
    connect(m_table_widget, &QTableWidget::itemSelectionChanged, this,
            &DemoWidget::on_selection_changed);
}

void DemoWidget::setTheme(const QString& theme_name) {
    m_current_theme = theme_name;
    applyThemeStyles(theme_name);
    emit dataChanged(getWidgetData());
}

void DemoWidget::applyThemeStyles(const QString& theme_name) {
    QString style_sheet;

    if (theme_name == "dark") {
        style_sheet = R"(
            QWidget { background-color: #2b2b2b; color: #ffffff; }
            QLineEdit, QTextEdit, QComboBox, QSpinBox {
                background-color: #3c3c3c; border: 1px solid #555555;
                border-radius: 4px; padding: 4px; color: #ffffff;
            }
            QPushButton {
                background-color: #0078d4; border: none; border-radius: 4px;
                padding: 8px 16px; color: white; font-weight: bold;
            }
            QPushButton:hover { background-color: #106ebe; }
            QPushButton:pressed { background-color: #005a9e; }
            QGroupBox { font-weight: bold; border: 2px solid #555555; border-radius: 8px; margin: 8px 0px; color: #ffffff; }
            QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px 0 5px; }
            QTabWidget::pane { border: 1px solid #555555; background-color: #2b2b2b; }
            QTabBar::tab { background-color: #3c3c3c; color: #ffffff; padding: 8px 16px; margin-right: 2px; }
            QTabBar::tab:selected { background-color: #0078d4; }
            QTreeWidget, QListWidget, QTableWidget { background-color: #3c3c3c; color: #ffffff; border: 1px solid #555555; }
        )";
    } else if (theme_name == "light") {
        style_sheet = R"(
            QWidget { background-color: #ffffff; color: #000000; }
            QLineEdit, QTextEdit, QComboBox, QSpinBox {
                background-color: #ffffff; border: 1px solid #cccccc;
                border-radius: 4px; padding: 4px; color: #000000;
            }
            QPushButton {
                background-color: #e1e1e1; border: 1px solid #adadad; border-radius: 4px;
                padding: 8px 16px; color: black; font-weight: bold;
            }
            QPushButton:hover { background-color: #d4d4d4; }
            QPushButton:pressed { background-color: #c7c7c7; }
            QGroupBox { font-weight: bold; border: 2px solid #cccccc; border-radius: 8px; margin: 8px 0px; color: #000000; }
            QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px 0 5px; }
            QTabWidget::pane { border: 1px solid #cccccc; background-color: #ffffff; }
            QTabBar::tab { background-color: #f0f0f0; color: #000000; padding: 8px 16px; margin-right: 2px; }
            QTabBar::tab:selected { background-color: #e1e1e1; }
            QTreeWidget, QListWidget, QTableWidget { background-color: #ffffff; color: #000000; border: 1px solid #cccccc; }
        )";
    } else if (theme_name == "blue") {
        style_sheet = R"(
            QWidget { background-color: #f0f8ff; color: #000080; }
            QLineEdit, QTextEdit, QComboBox, QSpinBox {
                background-color: #ffffff; border: 2px solid #4169e1;
                border-radius: 6px; padding: 4px; color: #000080;
            }
            QPushButton {
                background-color: #4169e1; border: none; border-radius: 6px;
                padding: 8px 16px; color: white; font-weight: bold;
            }
            QPushButton:hover { background-color: #6495ed; }
            QPushButton:pressed { background-color: #1e90ff; }
            QGroupBox { font-weight: bold; border: 2px solid #4169e1; border-radius: 8px; margin: 8px 0px; color: #000080; }
            QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px 0 5px; color: #4169e1; }
            QTabWidget::pane { border: 2px solid #4169e1; background-color: #f0f8ff; }
            QTabBar::tab { background-color: #e6f2ff; color: #000080; padding: 8px 16px; margin-right: 2px; }
            QTabBar::tab:selected { background-color: #4169e1; color: white; }
            QTreeWidget, QListWidget, QTableWidget { background-color: #ffffff; color: #000080; border: 2px solid #4169e1; }
        )";
    } else if (theme_name == "green") {
        style_sheet = R"(
            QWidget { background-color: #f0fff0; color: #006400; }
            QLineEdit, QTextEdit, QComboBox, QSpinBox {
                background-color: #ffffff; border: 2px solid #32cd32;
                border-radius: 6px; padding: 4px; color: #006400;
            }
            QPushButton {
                background-color: #32cd32; border: none; border-radius: 6px;
                padding: 8px 16px; color: white; font-weight: bold;
            }
            QPushButton:hover { background-color: #90ee90; color: #006400; }
            QPushButton:pressed { background-color: #228b22; }
            QGroupBox { font-weight: bold; border: 2px solid #32cd32; border-radius: 8px; margin: 8px 0px; color: #006400; }
            QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px 0 5px; color: #32cd32; }
            QTabWidget::pane { border: 2px solid #32cd32; background-color: #f0fff0; }
            QTabBar::tab { background-color: #e6ffe6; color: #006400; padding: 8px 16px; margin-right: 2px; }
            QTabBar::tab:selected { background-color: #32cd32; color: white; }
            QTreeWidget, QListWidget, QTableWidget { background-color: #ffffff; color: #006400; border: 2px solid #32cd32; }
        )";
    } else {
        // Default theme - clear stylesheet to use system default
        style_sheet = "";
    }

    setStyleSheet(style_sheet);

    // Force update of all child widgets
    update();
    repaint();
}

QJsonObject DemoWidget::getWidgetData() const {
    return QJsonObject{{"theme", m_current_theme},
                       {"line_edit_text", m_line_edit->text()},
                       {"text_edit_text", m_text_edit->toPlainText()},
                       {"check_box_checked", m_check_box->isChecked()},
                       {"radio_button1_checked", m_radio_button1->isChecked()},
                       {"radio_button2_checked", m_radio_button2->isChecked()},
                       {"combo_box_index", m_combo_box->currentIndex()},
                       {"combo_box_text", m_combo_box->currentText()},
                       {"spin_box_value", m_spin_box->value()},
                       {"slider_value", m_slider->value()},
                       {"progress_value", m_progress_bar->value()},
                       {"current_tab", m_tab_widget->currentIndex()}};
}

void DemoWidget::setWidgetData(const QJsonObject& data) {
    if (data.contains("theme")) {
        QString theme = data.value("theme").toString();
        m_theme_combo->setCurrentText(theme);
        setTheme(theme);
    }

    if (data.contains("line_edit_text")) {
        m_line_edit->setText(data.value("line_edit_text").toString());
    }

    if (data.contains("text_edit_text")) {
        m_text_edit->setPlainText(data.value("text_edit_text").toString());
    }

    if (data.contains("check_box_checked")) {
        m_check_box->setChecked(data.value("check_box_checked").toBool());
    }

    if (data.contains("combo_box_index")) {
        m_combo_box->setCurrentIndex(data.value("combo_box_index").toInt());
    }

    if (data.contains("spin_box_value")) {
        m_spin_box->setValue(data.value("spin_box_value").toInt());
    }

    if (data.contains("slider_value")) {
        m_slider->setValue(data.value("slider_value").toInt());
    }

    if (data.contains("current_tab")) {
        m_tab_widget->setCurrentIndex(data.value("current_tab").toInt());
    }
}

void DemoWidget::on_button_clicked() {
    emit actionTriggered("button_clicked");
    emit dataChanged(getWidgetData());
}

void DemoWidget::on_text_changed() {
    emit actionTriggered("text_changed");
    emit dataChanged(getWidgetData());
}

void DemoWidget::on_value_changed() {
    // Update progress bar based on slider value
    m_progress_bar->setValue(m_slider->value());
    emit actionTriggered("value_changed");
    emit dataChanged(getWidgetData());
}

void DemoWidget::on_selection_changed() {
    emit actionTriggered("selection_changed");
    emit dataChanged(getWidgetData());
}

// === SettingsDialog Implementation ===

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent), m_main_layout(nullptr), m_tab_widget(nullptr) {
    setupUI();
    setupConnections();
    setModal(true);
    setWindowTitle("Plugin Settings");
    resize(400, 300);
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::setupUI() {
    m_main_layout = new QVBoxLayout(this);
    m_tab_widget = new QTabWidget(this);
    m_main_layout->addWidget(m_tab_widget);

    // General Settings Tab
    m_general_tab = new QWidget();
    auto general_layout = new QGridLayout(m_general_tab);

    // Row 0
    general_layout->addWidget(new QLabel("Plugin Name:"), 0, 0);
    m_name_edit = new QLineEdit("UI Plugin");
    general_layout->addWidget(m_name_edit, 0, 1);

    // Row 1
    general_layout->addWidget(new QLabel("Theme:"), 1, 0);
    m_theme_combo = new QComboBox();
    m_theme_combo->addItems({"default", "dark", "light", "blue", "green"});
    general_layout->addWidget(m_theme_combo, 1, 1);

    // Row 2
    m_auto_save_check = new QCheckBox("Enable Auto-save");
    m_auto_save_check->setChecked(true);
    general_layout->addWidget(m_auto_save_check, 2, 0, 1, 2);

    // Row 3
    general_layout->addWidget(new QLabel("Refresh Interval (ms):"), 3, 0);
    m_refresh_interval_spin = new QSpinBox();
    m_refresh_interval_spin->setRange(100, 10000);
    m_refresh_interval_spin->setValue(1000);
    general_layout->addWidget(m_refresh_interval_spin, 3, 1);

    general_layout->setRowStretch(4, 1);
    m_tab_widget->addTab(m_general_tab, "General");

    // Advanced Settings Tab
    m_advanced_tab = new QWidget();
    auto advanced_layout = new QVBoxLayout(m_advanced_tab);

    m_debug_mode_check = new QCheckBox("Enable Debug Mode");
    advanced_layout->addWidget(m_debug_mode_check);

    m_verbose_logging_check = new QCheckBox("Verbose Logging");
    advanced_layout->addWidget(m_verbose_logging_check);

    auto path_layout = new QHBoxLayout();
    path_layout->addWidget(new QLabel("Custom Path:"));
    m_custom_path_edit = new QLineEdit();
    path_layout->addWidget(m_custom_path_edit);
    auto browse_button = new QPushButton("Browse...");
    path_layout->addWidget(browse_button);
    advanced_layout->addLayout(path_layout);

    advanced_layout->addStretch();
    m_tab_widget->addTab(m_advanced_tab, "Advanced");

    // Button layout
    auto button_layout = new QHBoxLayout();
    button_layout->addStretch();

    m_apply_button = new QPushButton("Apply");
    button_layout->addWidget(m_apply_button);

    m_reset_button = new QPushButton("Reset");
    button_layout->addWidget(m_reset_button);

    m_cancel_button = new QPushButton("Cancel");
    button_layout->addWidget(m_cancel_button);

    m_main_layout->addLayout(button_layout);
}

void SettingsDialog::setupConnections() {
    connect(m_apply_button, &QPushButton::clicked, this,
            &SettingsDialog::on_apply_clicked);
    connect(m_reset_button, &QPushButton::clicked, this,
            &SettingsDialog::on_reset_clicked);
    connect(m_cancel_button, &QPushButton::clicked, this, &QDialog::reject);
}

QJsonObject SettingsDialog::getSettings() const {
    return QJsonObject{
        {"plugin_name", m_name_edit->text()},
        {"theme", m_theme_combo->currentText()},
        {"auto_save_enabled", m_auto_save_check->isChecked()},
        {"refresh_interval", m_refresh_interval_spin->value()},
        {"debug_mode", m_debug_mode_check->isChecked()},
        {"verbose_logging", m_verbose_logging_check->isChecked()},
        {"custom_path", m_custom_path_edit->text()}};
}

void SettingsDialog::setSettings(const QJsonObject& settings) {
    if (settings.contains("plugin_name")) {
        m_name_edit->setText(settings.value("plugin_name").toString());
    }
    if (settings.contains("theme")) {
        m_theme_combo->setCurrentText(settings.value("theme").toString());
    }
    if (settings.contains("auto_save_enabled")) {
        m_auto_save_check->setChecked(
            settings.value("auto_save_enabled").toBool());
    }
    if (settings.contains("refresh_interval")) {
        m_refresh_interval_spin->setValue(
            settings.value("refresh_interval").toInt());
    }
    if (settings.contains("debug_mode")) {
        m_debug_mode_check->setChecked(settings.value("debug_mode").toBool());
    }
    if (settings.contains("verbose_logging")) {
        m_verbose_logging_check->setChecked(
            settings.value("verbose_logging").toBool());
    }
    if (settings.contains("custom_path")) {
        m_custom_path_edit->setText(settings.value("custom_path").toString());
    }
}

void SettingsDialog::on_apply_clicked() {
    emit settingsChanged(getSettings());
    accept();
}

void SettingsDialog::on_reset_clicked() {
    // Reset to default values
    m_name_edit->setText("UI Plugin");
    m_theme_combo->setCurrentText("default");
    m_auto_save_check->setChecked(true);
    m_refresh_interval_spin->setValue(1000);
    m_debug_mode_check->setChecked(false);
    m_verbose_logging_check->setChecked(false);
    m_custom_path_edit->clear();
}

// === AboutDialog Implementation ===

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setupUI();
    setModal(true);
    setWindowTitle("About UI Plugin");
    setFixedSize(350, 250);
}

AboutDialog::~AboutDialog() = default;

void AboutDialog::setupUI() {
    auto layout = new QVBoxLayout(this);

    // Title
    auto title_label = new QLabel("<h2>UI Plugin Example</h2>");
    title_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(title_label);

    // Version info
    auto version_label = new QLabel("Version 1.0.0");
    version_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(version_label);

    // Description
    auto desc_label = new QLabel(
        "A comprehensive UI plugin demonstrating Qt Widgets integration, "
        "theme support, and advanced UI management patterns.");
    desc_label->setWordWrap(true);
    desc_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(desc_label);

    // Author info
    auto author_label = new QLabel("Author: QtPlugin Development Team");
    author_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(author_label);

    // License
    auto license_label = new QLabel("License: MIT");
    license_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(license_label);

    layout->addStretch();

    // Close button
    auto close_button = new QPushButton("Close");
    connect(close_button, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(close_button);
}

// === UIPlugin Implementation ===

UIPlugin::UIPlugin(QObject* parent) : QObject(parent) {
    // Initialize dependencies
    m_required_dependencies = {};  // No required dependencies for UI plugin
    m_optional_dependencies = {"qtplugin.ConfigurationManager",
                               "qtplugin.ThemeManager"};

    log_info("UIPlugin constructed");
}

UIPlugin::~UIPlugin() {
    shutdown();
    log_info("UIPlugin destroyed");
}

qtplugin::expected<void, qtplugin::PluginError> UIPlugin::initialize() {
    std::unique_lock lock(m_state_mutex);

    if (m_state != qtplugin::PluginState::Unloaded) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin is already initialized");
    }

    try {
        m_state = qtplugin::PluginState::Initializing;
        m_initialization_time = std::chrono::system_clock::now();
        lock.unlock();

        log_info("Initializing UIPlugin...");

        // Initialize themes
        initialize_themes();

        // Initialize widgets
        initialize_widgets();

        // Initialize actions
        initialize_actions();

        lock.lock();
        m_state = qtplugin::PluginState::Running;
        lock.unlock();

        log_info("UIPlugin initialized successfully");

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to initialize UIPlugin: " + std::string(e.what());
        log_error(error_msg);

        lock.lock();
        m_state = qtplugin::PluginState::Error;
        lock.unlock();

        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed, error_msg);
    }
}

void UIPlugin::shutdown() noexcept {
    try {
        std::unique_lock lock(m_state_mutex);
        m_state = qtplugin::PluginState::Stopping;
        lock.unlock();

        log_info("Shutting down UIPlugin...");

        // Cleanup resources
        cleanup_resources();

        lock.lock();
        m_state = qtplugin::PluginState::Stopped;
        lock.unlock();

        log_info("UIPlugin shutdown completed");

    } catch (...) {
        // Shutdown must not throw
        std::unique_lock lock(m_state_mutex);
        m_state = qtplugin::PluginState::Error;
    }
}

bool UIPlugin::is_initialized() const noexcept {
    std::shared_lock lock(m_state_mutex);
    auto current_state = m_state.load();
    return current_state == qtplugin::PluginState::Running ||
           current_state == qtplugin::PluginState::Paused;
}

qtplugin::expected<void, qtplugin::PluginError> UIPlugin::pause() {
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
        log_info("UIPlugin paused successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to pause UIPlugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> UIPlugin::resume() {
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
        log_info("UIPlugin resumed successfully");

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to resume UIPlugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> UIPlugin::restart() {
    log_info("Restarting UIPlugin...");

    // Shutdown first
    shutdown();

    // Wait a brief moment for cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Initialize again
    return initialize();
}

// === Configuration ===

std::optional<QJsonObject> UIPlugin::default_configuration() const {
    return QJsonObject{
        {"default_theme", "default"}, {"logging_enabled", true},
        {"auto_save_enabled", true},  {"refresh_interval", 1000},
        {"show_tooltips", true},      {"enable_animations", true},
        {"window_opacity", 1.0}};
}

qtplugin::expected<void, qtplugin::PluginError> UIPlugin::configure(
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

        log_info("UIPlugin configured successfully");

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg =
            "Failed to configure UIPlugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError, error_msg);
    }
}

QJsonObject UIPlugin::current_configuration() const {
    std::lock_guard lock(m_config_mutex);
    return m_configuration;
}

bool UIPlugin::validate_configuration(const QJsonObject& config) const {
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

// === Commands ===

qtplugin::expected<QJsonObject, qtplugin::PluginError>
UIPlugin::execute_command(std::string_view command, const QJsonObject& params) {
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

std::vector<std::string> UIPlugin::available_commands() const {
    return {"widget", "action", "dialog", "theme", "settings", "status"};
}

// === Dependencies ===

std::vector<std::string> UIPlugin::dependencies() const {
    return m_required_dependencies;
}

std::vector<std::string> UIPlugin::optional_dependencies() const {
    return m_optional_dependencies;
}

bool UIPlugin::dependencies_satisfied() const {
    return m_dependencies_satisfied.load();
}

// === Error Handling ===

void UIPlugin::clear_errors() {
    std::lock_guard lock(m_error_mutex);
    m_error_log.clear();
    m_last_error.clear();
}

// === Monitoring ===

std::chrono::milliseconds UIPlugin::uptime() const {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_initialization_time);
    return duration;
}

QJsonObject UIPlugin::performance_metrics() const {
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

QJsonObject UIPlugin::resource_usage() const {
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

// === IUIPlugin Interface Implementation ===

qtplugin::UIComponentTypes UIPlugin::supported_components() const noexcept {
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
UIPlugin::supported_integration_points() const {
    return {qtplugin::UIIntegrationPoint::MainWindow,
            qtplugin::UIIntegrationPoint::MenuBar,
            qtplugin::UIIntegrationPoint::ToolBar,
            qtplugin::UIIntegrationPoint::DockArea,
            qtplugin::UIIntegrationPoint::CentralWidget,
            qtplugin::UIIntegrationPoint::ContextMenu,
            qtplugin::UIIntegrationPoint::SettingsDialog};
}

// === Widget Management ===

qtplugin::expected<QWidget*, qtplugin::PluginError> UIPlugin::create_widget(
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
                    &UIPlugin::on_widget_data_changed);
            connect(demo_widget, &DemoWidget::actionTriggered, this,
                    &UIPlugin::on_action_triggered);
            widget = demo_widget;
        } else if (widget_id == "settings_widget") {
            auto settings_dialog = new SettingsDialog(parent);
            connect(settings_dialog, &SettingsDialog::settingsChanged, this,
                    &UIPlugin::on_settings_changed);
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
UIPlugin::get_widget_info(const QString& widget_id) const {
    std::lock_guard lock(m_widgets_mutex);

    auto it = m_widget_info.find(widget_id);
    if (it == m_widget_info.end()) {
        return qtplugin::make_error<qtplugin::UIWidgetInfo>(
            qtplugin::PluginErrorCode::NotFound,
            "Widget not found: " + widget_id.toStdString());
    }

    return it->second;
}

std::vector<QString> UIPlugin::get_available_widgets() const {
    return {"demo_widget", "settings_widget"};
}

qtplugin::expected<void, qtplugin::PluginError> UIPlugin::destroy_widget(
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

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        std::string error_msg = "Failed to destroy widget '" +
                                widget_id.toStdString() + "': " + e.what();
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}
