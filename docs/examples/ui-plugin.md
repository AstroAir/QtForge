# UI Plugin Example

This example demonstrates how to create user interface plugins using Qt widgets and QML, including custom controls, data binding, and user interaction handling.

## Overview

UI plugins in QtForge enable:
- Custom widget creation and integration
- QML-based user interfaces
- Data binding and model-view patterns
- Event handling and user interaction
- Theme and styling support
- Responsive design patterns

## Qt Widgets UI Plugin

### Custom Widget Plugin

```cpp
// include/custom_widget_plugin.hpp
#pragma once

#include <qtforge/core/plugin_interface.hpp>
#include <qtforge/communication/message_bus.hpp>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QProgressBar>
#include <QTimer>
#include <memory>

class DataVisualizationWidget;
class ControlPanelWidget;

class CustomWidgetPlugin : public QObject, public qtforge::IPlugin {
    Q_OBJECT

public:
    CustomWidgetPlugin(QObject* parent = nullptr);
    ~CustomWidgetPlugin() override;

    // Plugin interface
    std::string name() const override { return "CustomWidgetPlugin"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "Custom Qt widget plugin with data visualization and controls";
    }
    
    std::vector<std::string> dependencies() const override {
        return {"CorePlugin >= 1.0.0"};
    }

    // Lifecycle
    qtforge::expected<void, qtforge::Error> initialize() override;
    qtforge::expected<void, qtforge::Error> activate() override;
    qtforge::expected<void, qtforge::Error> deactivate() override;
    void cleanup() override;

    qtforge::PluginState state() const override { return currentState_; }
    bool isCompatible(const std::string& version) const override;

    // UI access
    QWidget* getMainWidget() const { return mainWidget_.get(); }
    QWidget* createDataVisualizationWidget(QWidget* parent = nullptr);
    QWidget* createControlPanelWidget(QWidget* parent = nullptr);

private slots:
    void onDataReceived(const QVariantMap& data);
    void onControlValueChanged(const QString& control, const QVariant& value);
    void onRefreshRequested();
    void onExportRequested();

private:
    void setupUI();
    void setupMessageHandlers();
    void updateVisualization(const QVariantMap& data);
    void applyTheme(const QString& themeName);

    qtforge::PluginState currentState_;
    std::unique_ptr<QWidget> mainWidget_;
    std::unique_ptr<DataVisualizationWidget> dataWidget_;
    std::unique_ptr<ControlPanelWidget> controlWidget_;
    std::vector<qtforge::SubscriptionHandle> subscriptions_;
    QTimer* refreshTimer_;
};

// Custom data visualization widget
class DataVisualizationWidget : public QWidget {
    Q_OBJECT

public:
    explicit DataVisualizationWidget(QWidget* parent = nullptr);
    ~DataVisualizationWidget() override;

    void setData(const QVariantMap& data);
    void setVisualizationType(const QString& type);
    void exportToImage(const QString& filename);

signals:
    void dataPointClicked(const QVariant& value);
    void visualizationChanged(const QString& type);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void drawChart(QPainter& painter);
    void drawBarChart(QPainter& painter);
    void drawLineChart(QPainter& painter);
    void drawPieChart(QPainter& painter);
    QRect getChartArea() const;
    QPoint dataToScreen(const QPointF& dataPoint) const;

    QString visualizationType_;
    QVariantMap data_;
    QList<QColor> colors_;
    QFont titleFont_;
    QFont labelFont_;
    bool animated_;
    QTimer* animationTimer_;
    double animationProgress_;
};

// Custom control panel widget
class ControlPanelWidget : public QWidget {
    Q_OBJECT

public:
    explicit ControlPanelWidget(QWidget* parent = nullptr);
    ~ControlPanelWidget() override;

    void addControl(const QString& name, const QString& type, const QVariant& defaultValue);
    void removeControl(const QString& name);
    QVariant getControlValue(const QString& name) const;
    void setControlValue(const QString& name, const QVariant& value);

signals:
    void controlValueChanged(const QString& name, const QVariant& value);
    void refreshRequested();
    void exportRequested();

private slots:
    void onControlChanged();

private:
    void setupLayout();
    QWidget* createControl(const QString& type, const QVariant& defaultValue);

    QVBoxLayout* mainLayout_;
    QMap<QString, QWidget*> controls_;
    QPushButton* refreshButton_;
    QPushButton* exportButton_;
};
```

### Widget Plugin Implementation

```cpp
// src/custom_widget_plugin.cpp
#include "custom_widget_plugin.hpp"
#include <qtforge/communication/message_bus.hpp>
#include <qtforge/utils/logger.hpp>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPainter>
#include <QMouseEvent>
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QMessageBox>

CustomWidgetPlugin::CustomWidgetPlugin(QObject* parent)
    : QObject(parent), currentState_(qtforge::PluginState::Unloaded) {
    
    refreshTimer_ = new QTimer(this);
    refreshTimer_->setInterval(5000); // 5 second refresh
    connect(refreshTimer_, &QTimer::timeout, this, &CustomWidgetPlugin::onRefreshRequested);
}

CustomWidgetPlugin::~CustomWidgetPlugin() {
    cleanup();
}

qtforge::expected<void, qtforge::Error> CustomWidgetPlugin::initialize() {
    try {
        qtforge::Logger::info(name(), "Initializing custom widget plugin...");
        
        // Setup UI components
        setupUI();
        
        // Setup message handlers
        setupMessageHandlers();
        
        currentState_ = qtforge::PluginState::Initialized;
        qtforge::Logger::info(name(), "Custom widget plugin initialized successfully");
        
        return {};
        
    } catch (const std::exception& e) {
        currentState_ = qtforge::PluginState::Error;
        return qtforge::Error("Custom widget plugin initialization failed: " + std::string(e.what()));
    }
}

void CustomWidgetPlugin::setupUI() {
    // Create main widget
    mainWidget_ = std::make_unique<QWidget>();
    mainWidget_->setWindowTitle("Data Visualization Dashboard");
    mainWidget_->resize(1200, 800);
    
    // Create splitter layout
    auto* splitter = new QSplitter(Qt::Horizontal, mainWidget_.get());
    
    // Create data visualization widget
    dataWidget_ = std::make_unique<DataVisualizationWidget>();
    splitter->addWidget(dataWidget_.get());
    
    // Create control panel widget
    controlWidget_ = std::make_unique<ControlPanelWidget>();
    splitter->addWidget(controlWidget_.get());
    
    // Set splitter proportions
    splitter->setStretchFactor(0, 3); // Data widget gets 75%
    splitter->setStretchFactor(1, 1); // Control widget gets 25%
    
    // Main layout
    auto* mainLayout = new QVBoxLayout(mainWidget_.get());
    mainLayout->addWidget(splitter);
    
    // Connect signals
    connect(dataWidget_.get(), &DataVisualizationWidget::dataPointClicked,
            this, [this](const QVariant& value) {
                qtforge::Logger::info(name(), "Data point clicked: " + value.toString().toStdString());
            });
    
    connect(controlWidget_.get(), &ControlPanelWidget::controlValueChanged,
            this, &CustomWidgetPlugin::onControlValueChanged);
    
    connect(controlWidget_.get(), &ControlPanelWidget::refreshRequested,
            this, &CustomWidgetPlugin::onRefreshRequested);
    
    connect(controlWidget_.get(), &ControlPanelWidget::exportRequested,
            this, &CustomWidgetPlugin::onExportRequested);
    
    // Setup default controls
    controlWidget_->addControl("Chart Type", "combo", QStringList{"Bar", "Line", "Pie"});
    controlWidget_->addControl("Animation", "checkbox", true);
    controlWidget_->addControl("Refresh Rate", "slider", 5);
    controlWidget_->addControl("Color Scheme", "color", QColor(Qt::blue));
    
    // Apply default theme
    applyTheme("default");
}

void CustomWidgetPlugin::setupMessageHandlers() {
    auto& messageBus = qtforge::MessageBus::instance();
    
    // Subscribe to data updates
    subscriptions_.emplace_back(
        messageBus.subscribe<qtforge::DataUpdateMessage>("ui.data.update",
            [this](const qtforge::DataUpdateMessage& msg) {
                QVariantMap data;
                // Convert message data to QVariantMap
                for (const auto& [key, value] : msg.data) {
                    data[QString::fromStdString(key)] = QVariant::fromStdString(std::any_cast<std::string>(value));
                }
                onDataReceived(data);
            })
    );
    
    // Subscribe to theme changes
    subscriptions_.emplace_back(
        messageBus.subscribe<qtforge::ThemeChangeMessage>("ui.theme.change",
            [this](const qtforge::ThemeChangeMessage& msg) {
                applyTheme(QString::fromStdString(msg.themeName));
            })
    );
}

void CustomWidgetPlugin::onDataReceived(const QVariantMap& data) {
    qtforge::Logger::debug(name(), "Received data update with " + std::to_string(data.size()) + " items");
    
    // Update visualization
    updateVisualization(data);
    
    // Publish data received event
    auto& messageBus = qtforge::MessageBus::instance();
    qtforge::UIEventMessage event;
    event.eventType = "data_received";
    event.source = name();
    event.timestamp = std::chrono::system_clock::now();
    event.data["item_count"] = data.size();
    
    messageBus.publish("ui.events", event);
}

void CustomWidgetPlugin::updateVisualization(const QVariantMap& data) {
    if (dataWidget_) {
        dataWidget_->setData(data);
        
        // Update chart type based on control
        QString chartType = controlWidget_->getControlValue("Chart Type").toString();
        dataWidget_->setVisualizationType(chartType);
    }
}

// DataVisualizationWidget Implementation
DataVisualizationWidget::DataVisualizationWidget(QWidget* parent)
    : QWidget(parent), visualizationType_("Bar"), animated_(true), animationProgress_(0.0) {
    
    setMinimumSize(400, 300);
    
    // Setup colors
    colors_ << QColor(52, 152, 219)   // Blue
            << QColor(46, 204, 113)   // Green
            << QColor(231, 76, 60)    // Red
            << QColor(241, 196, 15)   // Yellow
            << QColor(155, 89, 182)   // Purple
            << QColor(230, 126, 34);  // Orange
    
    // Setup fonts
    titleFont_ = QFont("Arial", 14, QFont::Bold);
    labelFont_ = QFont("Arial", 10);
    
    // Animation timer
    animationTimer_ = new QTimer(this);
    animationTimer_->setInterval(16); // ~60 FPS
    connect(animationTimer_, &QTimer::timeout, [this]() {
        animationProgress_ += 0.02;
        if (animationProgress_ >= 1.0) {
            animationProgress_ = 1.0;
            animationTimer_->stop();
        }
        update();
    });
}

void DataVisualizationWidget::setData(const QVariantMap& data) {
    data_ = data;
    
    if (animated_) {
        animationProgress_ = 0.0;
        animationTimer_->start();
    } else {
        animationProgress_ = 1.0;
        update();
    }
}

void DataVisualizationWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Clear background
    painter.fillRect(rect(), QColor(248, 249, 250));
    
    if (data_.isEmpty()) {
        // Draw "No Data" message
        painter.setFont(titleFont_);
        painter.setPen(QColor(108, 117, 125));
        painter.drawText(rect(), Qt::AlignCenter, "No Data Available");
        return;
    }
    
    // Draw chart based on type
    drawChart(painter);
}

void DataVisualizationWidget::drawChart(QPainter& painter) {
    if (visualizationType_ == "Bar") {
        drawBarChart(painter);
    } else if (visualizationType_ == "Line") {
        drawLineChart(painter);
    } else if (visualizationType_ == "Pie") {
        drawPieChart(painter);
    }
}

void DataVisualizationWidget::drawBarChart(QPainter& painter) {
    QRect chartArea = getChartArea();
    
    if (data_.isEmpty()) return;
    
    // Calculate bar dimensions
    int barCount = data_.size();
    int barWidth = chartArea.width() / barCount * 0.8;
    int barSpacing = chartArea.width() / barCount * 0.2;
    
    // Find max value for scaling
    double maxValue = 0;
    for (auto it = data_.begin(); it != data_.end(); ++it) {
        maxValue = std::max(maxValue, it.value().toDouble());
    }
    
    // Draw bars
    int x = chartArea.left() + barSpacing / 2;
    int colorIndex = 0;
    
    for (auto it = data_.begin(); it != data_.end(); ++it) {
        double value = it.value().toDouble();
        double normalizedValue = maxValue > 0 ? value / maxValue : 0;
        
        // Apply animation
        double animatedValue = normalizedValue * animationProgress_;
        
        int barHeight = static_cast<int>(chartArea.height() * animatedValue);
        int barY = chartArea.bottom() - barHeight;
        
        // Draw bar
        QRect barRect(x, barY, barWidth, barHeight);
        painter.fillRect(barRect, colors_[colorIndex % colors_.size()]);
        
        // Draw label
        painter.setFont(labelFont_);
        painter.setPen(QColor(73, 80, 87));
        QRect labelRect(x, chartArea.bottom() + 5, barWidth, 20);
        painter.drawText(labelRect, Qt::AlignCenter, it.key());
        
        // Draw value
        if (barHeight > 20) {
            painter.setPen(Qt::white);
            painter.drawText(barRect, Qt::AlignCenter, QString::number(value, 'f', 1));
        }
        
        x += barWidth + barSpacing;
        colorIndex++;
    }
    
    // Draw title
    painter.setFont(titleFont_);
    painter.setPen(QColor(33, 37, 41));
    painter.drawText(QRect(0, 10, width(), 30), Qt::AlignCenter, "Data Visualization");
}

QRect DataVisualizationWidget::getChartArea() const {
    return QRect(50, 50, width() - 100, height() - 100);
}

// ControlPanelWidget Implementation
ControlPanelWidget::ControlPanelWidget(QWidget* parent)
    : QWidget(parent) {
    
    setFixedWidth(250);
    setupLayout();
}

void ControlPanelWidget::setupLayout() {
    mainLayout_ = new QVBoxLayout(this);
    
    // Title
    auto* titleLabel = new QLabel("Control Panel");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 10px;");
    mainLayout_->addWidget(titleLabel);
    
    // Controls will be added dynamically
    
    // Spacer
    mainLayout_->addStretch();
    
    // Action buttons
    refreshButton_ = new QPushButton("Refresh Data");
    exportButton_ = new QPushButton("Export Chart");
    
    refreshButton_->setStyleSheet("QPushButton { padding: 8px; margin: 2px; }");
    exportButton_->setStyleSheet("QPushButton { padding: 8px; margin: 2px; }");
    
    connect(refreshButton_, &QPushButton::clicked, this, &ControlPanelWidget::refreshRequested);
    connect(exportButton_, &QPushButton::clicked, this, &ControlPanelWidget::exportRequested);
    
    mainLayout_->addWidget(refreshButton_);
    mainLayout_->addWidget(exportButton_);
}

void ControlPanelWidget::addControl(const QString& name, const QString& type, const QVariant& defaultValue) {
    if (controls_.contains(name)) {
        return; // Control already exists
    }
    
    // Create label
    auto* label = new QLabel(name + ":");
    label->setStyleSheet("font-weight: bold; margin-top: 10px;");
    
    // Create control widget
    QWidget* control = createControl(type, defaultValue);
    if (!control) {
        return;
    }
    
    controls_[name] = control;
    
    // Insert before spacer and buttons
    int insertIndex = mainLayout_->count() - 3; // Before spacer and 2 buttons
    mainLayout_->insertWidget(insertIndex, label);
    mainLayout_->insertWidget(insertIndex + 1, control);
}

QWidget* ControlPanelWidget::createControl(const QString& type, const QVariant& defaultValue) {
    if (type == "combo") {
        auto* combo = new QComboBox();
        QStringList items = defaultValue.toStringList();
        combo->addItems(items);
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &ControlPanelWidget::onControlChanged);
        return combo;
        
    } else if (type == "checkbox") {
        auto* checkbox = new QCheckBox();
        checkbox->setChecked(defaultValue.toBool());
        connect(checkbox, &QCheckBox::toggled,
                this, &ControlPanelWidget::onControlChanged);
        return checkbox;
        
    } else if (type == "slider") {
        auto* slider = new QSlider(Qt::Horizontal);
        slider->setRange(1, 60);
        slider->setValue(defaultValue.toInt());
        connect(slider, &QSlider::valueChanged,
                this, &ControlPanelWidget::onControlChanged);
        return slider;
        
    } else if (type == "color") {
        auto* button = new QPushButton();
        QColor color = defaultValue.value<QColor>();
        button->setStyleSheet(QString("background-color: %1; min-height: 30px;").arg(color.name()));
        button->setProperty("color", color);
        connect(button, &QPushButton::clicked, [this, button]() {
            QColor currentColor = button->property("color").value<QColor>();
            QColor newColor = QColorDialog::getColor(currentColor, this);
            if (newColor.isValid()) {
                button->setStyleSheet(QString("background-color: %1; min-height: 30px;").arg(newColor.name()));
                button->setProperty("color", newColor);
                onControlChanged();
            }
        });
        return button;
    }
    
    return nullptr;
}

void ControlPanelWidget::onControlChanged() {
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (!sender) return;
    
    // Find control name
    QString controlName;
    for (auto it = controls_.begin(); it != controls_.end(); ++it) {
        if (it.value() == sender) {
            controlName = it.key();
            break;
        }
    }
    
    if (controlName.isEmpty()) return;
    
    // Get control value
    QVariant value = getControlValue(controlName);
    
    // Emit signal
    emit controlValueChanged(controlName, value);
}

QVariant ControlPanelWidget::getControlValue(const QString& name) const {
    QWidget* control = controls_.value(name);
    if (!control) return QVariant();
    
    if (auto* combo = qobject_cast<QComboBox*>(control)) {
        return combo->currentText();
    } else if (auto* checkbox = qobject_cast<QCheckBox*>(control)) {
        return checkbox->isChecked();
    } else if (auto* slider = qobject_cast<QSlider*>(control)) {
        return slider->value();
    } else if (auto* button = qobject_cast<QPushButton*>(control)) {
        return button->property("color");
    }
    
    return QVariant();
}
```

## QML UI Plugin

### QML-Based Plugin

```cpp
// include/qml_ui_plugin.hpp
#pragma once

#include <qtforge/core/plugin_interface.hpp>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <memory>

class DataModel;
class UIController;

class QmlUIPlugin : public QObject, public qtforge::IPlugin {
    Q_OBJECT

public:
    QmlUIPlugin(QObject* parent = nullptr);
    ~QmlUIPlugin() override;

    // Plugin interface
    std::string name() const override { return "QmlUIPlugin"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "QML-based user interface plugin with modern UI components";
    }

    // Lifecycle
    qtforge::expected<void, qtforge::Error> initialize() override;
    qtforge::expected<void, qtforge::Error> activate() override;
    qtforge::expected<void, qtforge::Error> deactivate() override;
    void cleanup() override;

    qtforge::PluginState state() const override { return currentState_; }

    // QML access
    QQmlApplicationEngine* getQmlEngine() const { return qmlEngine_.get(); }
    Q_INVOKABLE void showMainWindow();
    Q_INVOKABLE void hideMainWindow();

public slots:
    void onDataChanged(const QVariantMap& data);
    void onUserAction(const QString& action, const QVariant& data);

private:
    void setupQmlEngine();
    void registerQmlTypes();
    void loadQmlComponents();

    qtforge::PluginState currentState_;
    std::unique_ptr<QQmlApplicationEngine> qmlEngine_;
    std::unique_ptr<DataModel> dataModel_;
    std::unique_ptr<UIController> uiController_;
    std::vector<qtforge::SubscriptionHandle> subscriptions_;
};
```

### QML Files

```qml
// qml/MainWindow.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

ApplicationWindow {
    id: mainWindow
    width: 1200
    height: 800
    visible: true
    title: "QtForge QML Dashboard"
    
    Material.theme: Material.Light
    Material.primary: Material.Blue
    Material.accent: Material.Orange
    
    property alias dataModel: dataView.model
    
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            
            Label {
                text: "QtForge Dashboard"
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }
            
            ToolButton {
                text: "⚙️"
                onClicked: settingsDrawer.open()
            }
            
            ToolButton {
                text: "📊"
                onClicked: chartDialog.open()
            }
        }
    }
    
    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        
        // Data Panel
        Rectangle {
            SplitView.minimumWidth: 300
            SplitView.preferredWidth: 400
            color: "#f8f9fa"
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                
                Label {
                    text: "Data Overview"
                    font.pixelSize: 16
                    font.bold: true
                }
                
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    ListView {
                        id: dataView
                        model: dataModel
                        
                        delegate: ItemDelegate {
                            width: dataView.width
                            height: 60
                            
                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 2
                                color: parent.hovered ? "#e9ecef" : "transparent"
                                radius: 4
                                
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    
                                    Rectangle {
                                        width: 40
                                        height: 40
                                        radius: 20
                                        color: model.color || Material.primary
                                        
                                        Label {
                                            anchors.centerIn: parent
                                            text: model.icon || "📊"
                                            font.pixelSize: 16
                                        }
                                    }
                                    
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        
                                        Label {
                                            text: model.name || "Unknown"
                                            font.bold: true
                                        }
                                        
                                        Label {
                                            text: model.value || "N/A"
                                            color: "#6c757d"
                                            font.pixelSize: 12
                                        }
                                    }
                                    
                                    Label {
                                        text: model.trend || ""
                                        font.pixelSize: 20
                                    }
                                }
                            }
                            
                            onClicked: {
                                uiController.onItemSelected(model.id)
                            }
                        }
                    }
                }
                
                Button {
                    text: "Refresh Data"
                    Layout.fillWidth: true
                    onClicked: uiController.refreshData()
                }
            }
        }
        
        // Visualization Panel
        Rectangle {
            SplitView.fillWidth: true
            color: "white"
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                
                Label {
                    text: "Data Visualization"
                    font.pixelSize: 18
                    font.bold: true
                }
                
                ChartView {
                    id: chartView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    property var chartData: dataModel.chartData
                    
                    onChartDataChanged: {
                        updateChart()
                    }
                    
                    function updateChart() {
                        // Update chart with new data
                        // Implementation depends on chart type
                    }
                }
                
                RowLayout {
                    Button {
                        text: "Export"
                        onClicked: uiController.exportChart()
                    }
                    
                    Button {
                        text: "Settings"
                        onClicked: settingsDrawer.open()
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    ComboBox {
                        model: ["Bar Chart", "Line Chart", "Pie Chart"]
                        onCurrentTextChanged: {
                            uiController.setChartType(currentText)
                        }
                    }
                }
            }
        }
    }
    
    // Settings Drawer
    Drawer {
        id: settingsDrawer
        width: 300
        height: parent.height
        edge: Qt.RightEdge
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            
            Label {
                text: "Settings"
                font.pixelSize: 18
                font.bold: true
            }
            
            GroupBox {
                title: "Appearance"
                Layout.fillWidth: true
                
                ColumnLayout {
                    anchors.fill: parent
                    
                    Switch {
                        text: "Dark Theme"
                        onToggled: {
                            Material.theme = checked ? Material.Dark : Material.Light
                        }
                    }
                    
                    RowLayout {
                        Label { text: "Primary Color:" }
                        Rectangle {
                            width: 30
                            height: 30
                            color: Material.primary
                            radius: 4
                            
                            MouseArea {
                                anchors.fill: parent
                                onClicked: colorDialog.open()
                            }
                        }
                    }
                }
            }
            
            GroupBox {
                title: "Data"
                Layout.fillWidth: true
                
                ColumnLayout {
                    anchors.fill: parent
                    
                    RowLayout {
                        Label { text: "Refresh Rate:" }
                        SpinBox {
                            from: 1
                            to: 60
                            value: 5
                            suffix: "s"
                            onValueChanged: {
                                uiController.setRefreshRate(value)
                            }
                        }
                    }
                    
                    Switch {
                        text: "Auto Refresh"
                        checked: true
                        onToggled: {
                            uiController.setAutoRefresh(checked)
                        }
                    }
                }
            }
            
            Item { Layout.fillHeight: true }
            
            Button {
                text: "Close"
                Layout.fillWidth: true
                onClicked: settingsDrawer.close()
            }
        }
    }
}
```

## Key Features Demonstrated

1. **Qt Widgets Integration**: Custom widgets with painting and event handling
2. **QML Modern UI**: Declarative UI with Material Design
3. **Data Binding**: Model-view patterns with automatic updates
4. **Custom Controls**: Specialized UI components for specific needs
5. **Theme Support**: Dynamic theming and styling
6. **User Interaction**: Event handling and user input processing
7. **Responsive Design**: Adaptive layouts and resizing
8. **Animation**: Smooth transitions and visual feedback

## Best Practices

1. **Separation of Concerns**: Keep UI logic separate from business logic
2. **Model-View Patterns**: Use proper data binding and models
3. **Performance**: Optimize painting and updates for smooth UI
4. **Accessibility**: Support keyboard navigation and screen readers
5. **Responsive Design**: Handle different screen sizes and orientations
6. **Theme Consistency**: Follow platform design guidelines

## Next Steps

- **[Advanced Examples](advanced.md)**: More complex UI patterns
- **[Python Examples](python-examples.md)**: Python UI integration
- **[Plugin Architecture](../user-guide/plugin-architecture.md)**: UI plugin architecture
- **[Performance Optimization](../user-guide/performance-optimization.md)**: UI performance tuning
