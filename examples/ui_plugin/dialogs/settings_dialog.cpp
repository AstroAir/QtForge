/**
 * @file settings_dialog.cpp
 * @brief Settings dialog implementation
 * @version 3.0.0
 */

#include "settings_dialog.hpp"

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
