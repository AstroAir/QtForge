/**
 * @file demo_widget.cpp
 * @brief Demo widget implementation
 * @version 3.0.0
 */

#include "demo_widget.hpp"
#include <QApplication>
#include <QHeaderView>

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
