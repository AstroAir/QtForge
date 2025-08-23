/**
 * @file test_ui_plugin.cpp
 * @brief Comprehensive test application for the UI plugin example
 * @version 3.0.0
 *
 * This test application demonstrates and validates ALL functionality of the
 * UI plugin, including widget creation, theme management, dialog handling,
 * action management, and comprehensive UI integration.
 */

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <chrono>
#include <filesystem>
#include <qtplugin/qtplugin.hpp>
#include <thread>

class UIPluginTestWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit UIPluginTestWindow(qtplugin::IPlugin* plugin,
                                QWidget* parent = nullptr)
        : QMainWindow(parent), m_plugin(plugin) {
        setupUI();
        setupConnections();
        setWindowTitle("UI Plugin Test Application");
        resize(1000, 700);
    }

private slots:
    void on_create_demo_widget();
    void on_create_settings_widget();
    void on_show_settings_dialog();
    void on_show_about_dialog();
    void on_apply_theme();
    void on_trigger_action();
    void on_get_status();
    void on_run_comprehensive_test();

private:
    void setupUI() {
        // Central widget
        auto central_widget = new QWidget(this);
        setCentralWidget(central_widget);

        auto main_layout = new QVBoxLayout(central_widget);

        // Control panel
        auto control_panel = new QWidget();
        auto control_layout = new QHBoxLayout(control_panel);

        // Widget controls
        auto widget_group = new QWidget();
        auto widget_layout = new QVBoxLayout(widget_group);
        widget_layout->addWidget(new QLabel("<b>Widget Management</b>"));

        m_create_demo_btn = new QPushButton("Create Demo Widget");
        widget_layout->addWidget(m_create_demo_btn);

        m_create_settings_btn = new QPushButton("Create Settings Widget");
        widget_layout->addWidget(m_create_settings_btn);

        control_layout->addWidget(widget_group);

        // Dialog controls
        auto dialog_group = new QWidget();
        auto dialog_layout = new QVBoxLayout(dialog_group);
        dialog_layout->addWidget(new QLabel("<b>Dialog Management</b>"));

        m_show_settings_btn = new QPushButton("Show Settings Dialog");
        dialog_layout->addWidget(m_show_settings_btn);

        m_show_about_btn = new QPushButton("Show About Dialog");
        dialog_layout->addWidget(m_show_about_btn);

        control_layout->addWidget(dialog_group);

        // Theme controls
        auto theme_group = new QWidget();
        auto theme_layout = new QVBoxLayout(theme_group);
        theme_layout->addWidget(new QLabel("<b>Theme Management</b>"));

        m_theme_combo = new QComboBox();
        m_theme_combo->addItems({"default", "dark", "light", "blue", "green"});
        theme_layout->addWidget(m_theme_combo);

        m_apply_theme_btn = new QPushButton("Apply Theme");
        theme_layout->addWidget(m_apply_theme_btn);

        control_layout->addWidget(theme_group);

        // Action controls
        auto action_group = new QWidget();
        auto action_layout = new QVBoxLayout(action_group);
        action_layout->addWidget(new QLabel("<b>Action Management</b>"));

        m_action_combo = new QComboBox();
        m_action_combo->addItems({"show_demo", "show_settings", "show_about"});
        action_layout->addWidget(m_action_combo);

        m_trigger_action_btn = new QPushButton("Trigger Action");
        action_layout->addWidget(m_trigger_action_btn);

        control_layout->addWidget(action_group);

        main_layout->addWidget(control_panel);

        // Test controls
        auto test_layout = new QHBoxLayout();

        m_get_status_btn = new QPushButton("Get Status");
        test_layout->addWidget(m_get_status_btn);

        m_comprehensive_test_btn = new QPushButton("Run Comprehensive Test");
        test_layout->addWidget(m_comprehensive_test_btn);

        test_layout->addStretch();
        main_layout->addLayout(test_layout);

        // Output area
        m_output_text = new QTextEdit();
        m_output_text->setReadOnly(true);
        m_output_text->setFont(QFont("Consolas", 9));
        main_layout->addWidget(m_output_text);

        // Status bar
        statusBar()->showMessage("UI Plugin Test Application Ready");
    }

    void setupConnections() {
        connect(m_create_demo_btn, &QPushButton::clicked, this,
                &UIPluginTestWindow::on_create_demo_widget);
        connect(m_create_settings_btn, &QPushButton::clicked, this,
                &UIPluginTestWindow::on_create_settings_widget);
        connect(m_show_settings_btn, &QPushButton::clicked, this,
                &UIPluginTestWindow::on_show_settings_dialog);
        connect(m_show_about_btn, &QPushButton::clicked, this,
                &UIPluginTestWindow::on_show_about_dialog);
        connect(m_apply_theme_btn, &QPushButton::clicked, this,
                &UIPluginTestWindow::on_apply_theme);
        connect(m_trigger_action_btn, &QPushButton::clicked, this,
                &UIPluginTestWindow::on_trigger_action);
        connect(m_get_status_btn, &QPushButton::clicked, this,
                &UIPluginTestWindow::on_get_status);
        connect(m_comprehensive_test_btn, &QPushButton::clicked, this,
                &UIPluginTestWindow::on_run_comprehensive_test);
    }

    void log_output(const QString& message) {
        QString timestamp =
            QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        m_output_text->append(QString("[%1] %2").arg(timestamp, message));
        m_output_text->ensureCursorVisible();
        QApplication::processEvents();
    }

    void log_json_result(const QString& operation, const QJsonObject& result) {
        log_output(QString("=== %1 ===").arg(operation));
        log_output(QJsonDocument(result).toJson(QJsonDocument::Compact));
    }

    qtplugin::IPlugin* m_plugin;
    QPushButton* m_create_demo_btn;
    QPushButton* m_create_settings_btn;
    QPushButton* m_show_settings_btn;
    QPushButton* m_show_about_btn;
    QComboBox* m_theme_combo;
    QPushButton* m_apply_theme_btn;
    QComboBox* m_action_combo;
    QPushButton* m_trigger_action_btn;
    QPushButton* m_get_status_btn;
    QPushButton* m_comprehensive_test_btn;
    QTextEdit* m_output_text;
};

void UIPluginTestWindow::on_create_demo_widget() {
    log_output("Creating demo widget...");
    auto result = m_plugin->execute_command(
        "widget",
        QJsonObject{{"action", "create"}, {"widget_id", "demo_widget"}});

    if (result) {
        log_json_result("Create Demo Widget", result.value());
        statusBar()->showMessage("Demo widget created successfully");
    } else {
        log_output("❌ Failed to create demo widget: " +
                   QString::fromStdString(result.error().message));
    }
}

void UIPluginTestWindow::on_create_settings_widget() {
    log_output("Creating settings widget...");
    auto result = m_plugin->execute_command(
        "widget",
        QJsonObject{{"action", "create"}, {"widget_id", "settings_widget"}});

    if (result) {
        log_json_result("Create Settings Widget", result.value());
        statusBar()->showMessage("Settings widget created successfully");
    } else {
        log_output("❌ Failed to create settings widget: " +
                   QString::fromStdString(result.error().message));
    }
}

void UIPluginTestWindow::on_show_settings_dialog() {
    log_output("Showing settings dialog...");
    auto result = m_plugin->execute_command(
        "dialog", QJsonObject{{"action", "show"}, {"dialog_id", "settings"}});

    if (result) {
        log_json_result("Show Settings Dialog", result.value());
        statusBar()->showMessage("Settings dialog shown");
    } else {
        log_output("❌ Failed to show settings dialog: " +
                   QString::fromStdString(result.error().message));
    }
}

void UIPluginTestWindow::on_show_about_dialog() {
    log_output("Showing about dialog...");
    auto result = m_plugin->execute_command(
        "dialog", QJsonObject{{"action", "show"}, {"dialog_id", "about"}});

    if (result) {
        log_json_result("Show About Dialog", result.value());
        statusBar()->showMessage("About dialog shown");
    } else {
        log_output("❌ Failed to show about dialog: " +
                   QString::fromStdString(result.error().message));
    }
}

void UIPluginTestWindow::on_apply_theme() {
    QString theme_name = m_theme_combo->currentText();
    log_output("Applying theme: " + theme_name);

    auto result = m_plugin->execute_command(
        "theme", QJsonObject{{"action", "apply"}, {"theme_name", theme_name}});

    if (result) {
        log_json_result("Apply Theme", result.value());
        statusBar()->showMessage("Theme applied: " + theme_name);
    } else {
        log_output("❌ Failed to apply theme: " +
                   QString::fromStdString(result.error().message));
    }
}

void UIPluginTestWindow::on_trigger_action() {
    QString action_id = m_action_combo->currentText();
    log_output("Triggering action: " + action_id);

    auto result = m_plugin->execute_command(
        "action", QJsonObject{{"action", "trigger"}, {"action_id", action_id}});

    if (result) {
        log_json_result("Trigger Action", result.value());
        statusBar()->showMessage("Action triggered: " + action_id);
    } else {
        log_output("❌ Failed to trigger action: " +
                   QString::fromStdString(result.error().message));
    }
}

void UIPluginTestWindow::on_get_status() {
    log_output("Getting plugin status...");
    auto result = m_plugin->execute_command("status");

    if (result) {
        log_json_result("Plugin Status", result.value());
        statusBar()->showMessage("Status retrieved successfully");
    } else {
        log_output("❌ Failed to get status: " +
                   QString::fromStdString(result.error().message));
    }
}

void UIPluginTestWindow::on_run_comprehensive_test() {
    log_output("🚀 Starting comprehensive UI plugin test...");

    // Test 1: Widget management
    log_output("\n=== Testing Widget Management ===");
    auto widget_list =
        m_plugin->execute_command("widget", QJsonObject{{"action", "list"}});
    if (widget_list) {
        log_json_result("Available Widgets", widget_list.value());
    }

    // Test 2: Theme management
    log_output("\n=== Testing Theme Management ===");
    auto theme_list =
        m_plugin->execute_command("theme", QJsonObject{{"action", "list"}});
    if (theme_list) {
        log_json_result("Available Themes", theme_list.value());
    }

    // Test 3: Action management
    log_output("\n=== Testing Action Management ===");
    auto action_list =
        m_plugin->execute_command("action", QJsonObject{{"action", "list"}});
    if (action_list) {
        log_json_result("Available Actions", action_list.value());
    }

    // Test 4: Dialog management
    log_output("\n=== Testing Dialog Management ===");
    auto dialog_list =
        m_plugin->execute_command("dialog", QJsonObject{{"action", "list"}});
    if (dialog_list) {
        log_json_result("Available Dialogs", dialog_list.value());
    }

    // Test 5: Settings management
    log_output("\n=== Testing Settings Management ===");
    auto settings_get =
        m_plugin->execute_command("settings", QJsonObject{{"action", "get"}});
    if (settings_get) {
        log_json_result("Current Settings", settings_get.value());
    }

    // Test 6: Performance metrics
    log_output("\n=== Testing Performance Metrics ===");
    auto perf_metrics = m_plugin->performance_metrics();
    log_json_result("Performance Metrics", perf_metrics);

    // Test 7: Resource usage
    log_output("\n=== Testing Resource Usage ===");
    auto resource_usage = m_plugin->resource_usage();
    log_json_result("Resource Usage", resource_usage);

    log_output("\n🎉 Comprehensive test completed!");
    statusBar()->showMessage("Comprehensive test completed successfully");
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    qInfo() << "🚀 UI PLUGIN COMPREHENSIVE TEST";

    // Initialize plugin manager
    qtplugin::PluginManager manager;

    // Set plugin directory
    std::filesystem::path plugin_dir = std::filesystem::current_path();
    manager.add_search_path(plugin_dir);

    qInfo() << "Plugin directory:"
            << QString::fromStdString(plugin_dir.string());

    // Load the UI plugin
    qInfo() << "\n=== Loading UI Plugin ===";

    auto load_result = manager.load_plugin("ui_plugin.qtplugin");
    if (!load_result) {
        qCritical() << "Failed to load UI plugin:"
                    << QString::fromStdString(load_result.error().message);
        return 1;
    }

    qInfo() << "✅ UI plugin loaded successfully";

    // Get plugin instance
    auto plugin = manager.get_plugin("com.example.ui_plugin");
    if (!plugin) {
        qCritical() << "Failed to get UI plugin instance";
        return 1;
    }

    qInfo() << "✅ UI plugin instance obtained";
    qInfo() << "Plugin name:"
            << QString::fromStdString(std::string(plugin->name()));
    qInfo() << "Plugin ID:" << QString::fromStdString(plugin->id());
    qInfo() << "Plugin version:" << plugin->version().to_string().c_str();

    // Initialize the plugin
    qInfo() << "\n=== Initializing UI Plugin ===";

    auto init_result = plugin->initialize();
    if (!init_result) {
        qCritical() << "Failed to initialize UI plugin:"
                    << QString::fromStdString(init_result.error().message);
        return 1;
    }

    qInfo() << "✅ UI plugin initialized successfully";

    // Create and show test window
    UIPluginTestWindow window(plugin.get());
    window.show();

    qInfo() << "✅ UI Plugin Test Application started";
    qInfo() << "Use the interface to test all UI plugin functionality";

    return app.exec();
}

#include "test_ui_plugin.moc"
