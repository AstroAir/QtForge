/**
 * @file settings_dialog.hpp
 * @brief Settings dialog for UI plugin configuration
 * @version 3.0.0
 */

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

/**
 * @brief Settings dialog for plugin configuration
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;

    /**
     * @brief Get current settings as JSON
     */
    QJsonObject getSettings() const;

    /**
     * @brief Set settings from JSON
     */
    void setSettings(const QJsonObject& settings);

signals:
    /**
     * @brief Emitted when settings are changed and applied
     */
    void settingsChanged(const QJsonObject& settings);

private slots:
    void on_apply_clicked();
    void on_reset_clicked();

private:
    void setupUI();
    void setupConnections();

    // Layout components
    QVBoxLayout* m_main_layout;
    QTabWidget* m_tab_widget;

    // General settings tab
    QWidget* m_general_tab;
    QLineEdit* m_name_edit;
    QComboBox* m_theme_combo;
    QCheckBox* m_auto_save_check;
    QSpinBox* m_refresh_interval_spin;

    // Advanced settings tab
    QWidget* m_advanced_tab;
    QCheckBox* m_debug_mode_check;
    QCheckBox* m_verbose_logging_check;
    QLineEdit* m_custom_path_edit;

    // Buttons
    QPushButton* m_apply_button;
    QPushButton* m_reset_button;
    QPushButton* m_cancel_button;
};
