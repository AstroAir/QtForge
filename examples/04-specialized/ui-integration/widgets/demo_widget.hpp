/**
 * @file demo_widget.hpp
 * @brief Demo widget with comprehensive UI controls
 * @version 3.0.0
 */

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

/**
 * @brief Comprehensive demo widget showcasing various Qt controls
 */
class DemoWidget : public QWidget {
    Q_OBJECT

public:
    explicit DemoWidget(QWidget* parent = nullptr);
    ~DemoWidget() override;

    /**
     * @brief Apply a theme to the widget
     */
    void setTheme(const QString& theme_name);

    /**
     * @brief Get current widget data as JSON
     */
    QJsonObject getWidgetData() const;

    /**
     * @brief Set widget data from JSON
     */
    void setWidgetData(const QJsonObject& data);

signals:
    /**
     * @brief Emitted when widget data changes
     */
    void dataChanged(const QJsonObject& data);

    /**
     * @brief Emitted when an action is triggered
     */
    void actionTriggered(const QString& action);

private slots:
    void on_button_clicked();
    void on_text_changed();
    void on_value_changed();
    void on_selection_changed();

private:
    void setupUI();
    void setupConnections();
    void applyThemeStyles(const QString& theme_name);

    // Layout components
    QVBoxLayout* m_main_layout;
    QTabWidget* m_tab_widget;

    // Basic controls tab
    QWidget* m_basic_tab;
    QLineEdit* m_line_edit;
    QTextEdit* m_text_edit;
    QPushButton* m_push_button;
    QCheckBox* m_check_box;
    QRadioButton* m_radio_button1;
    QRadioButton* m_radio_button2;
    QComboBox* m_combo_box;
    QSpinBox* m_spin_box;
    QSlider* m_slider;
    QProgressBar* m_progress_bar;

    // Advanced controls tab
    QWidget* m_advanced_tab;
    QTreeWidget* m_tree_widget;
    QListWidget* m_list_widget;
    QTableWidget* m_table_widget;

    // Settings tab
    QWidget* m_settings_tab;
    QGroupBox* m_theme_group;
    QGroupBox* m_options_group;
    QComboBox* m_theme_combo;

    // State
    QString m_current_theme = "default";
    QTimer* m_update_timer;
};
