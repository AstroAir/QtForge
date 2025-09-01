/**
 * @file main.cpp
 * @brief Main entry point for UI plugin example
 * @version 3.0.0
 */

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <iostream>

#include "core/ui_plugin_core.hpp"
#include "dialogs/about_dialog.hpp"
#include "dialogs/settings_dialog.hpp"
#include "widgets/demo_widget.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setupUI();
        setupMenus();
        setupConnections();

        // Initialize the UI plugin core
        m_plugin_core = std::make_unique<UIPluginCore>(this);
        auto init_result = m_plugin_core->initialize();
        if (!init_result) {
            QMessageBox::critical(
                this, "Error",
                QString("Failed to initialize UI plugin: %1")
                    .arg(init_result.error().message.c_str()));
        }
    }

    ~MainWindow() override {
        if (m_plugin_core) {
            m_plugin_core->shutdown();
        }
    }

private slots:
    void showDemoWidget() {
        if (!m_demo_widget) {
            auto result = m_plugin_core->create_widget("demo_widget", this);
            if (result) {
                m_demo_widget = qobject_cast<DemoWidget*>(result.value());
                if (m_demo_widget) {
                    setCentralWidget(m_demo_widget);
                    m_demo_widget->show();
                }
            } else {
                QMessageBox::warning(this, "Error",
                                     QString("Failed to create demo widget: %1")
                                         .arg(result.error().message.c_str()));
            }
        } else {
            setCentralWidget(m_demo_widget);
            m_demo_widget->show();
        }
    }

    void showSettings() {
        auto settings_dialog = new SettingsDialog(this);

        // Set current plugin configuration
        if (m_plugin_core) {
            auto config = m_plugin_core->current_configuration();
            settings_dialog->setSettings(config);
        }

        connect(settings_dialog, &SettingsDialog::settingsChanged, this,
                [this](const QJsonObject& settings) {
                    if (m_plugin_core) {
                        auto result = m_plugin_core->configure(settings);
                        if (!result) {
                            QMessageBox::warning(
                                this, "Configuration Error",
                                QString("Failed to apply settings: %1")
                                    .arg(result.error().message.c_str()));
                        }
                    }
                });

        settings_dialog->exec();
        settings_dialog->deleteLater();
    }

    void showAbout() {
        auto about_dialog = new AboutDialog(this);
        about_dialog->exec();
        about_dialog->deleteLater();
    }

    void applyTheme(const QString& theme_name) {
        if (m_plugin_core) {
            auto result = m_plugin_core->apply_theme(theme_name);
            if (!result) {
                QMessageBox::warning(this, "Theme Error",
                                     QString("Failed to apply theme: %1")
                                         .arg(result.error().message.c_str()));
            } else {
                statusBar()->showMessage(
                    QString("Theme applied: %1").arg(theme_name), 2000);
            }
        }
    }

    void showPluginStatus() {
        if (m_plugin_core) {
            auto metrics = m_plugin_core->performance_metrics();
            auto resources = m_plugin_core->resource_usage();

            QString status_text =
                QString(
                    "Plugin Status:\n"
                    "- Initialized: %1\n"
                    "- Uptime: %2 ms\n"
                    "- Commands: %3\n"
                    "- Widgets: %4\n"
                    "- Current Theme: %5\n"
                    "- Memory: %6 KB\n"
                    "- CPU: %7%")
                    .arg(m_plugin_core->is_initialized() ? "Yes" : "No")
                    .arg(metrics["uptime_ms"].toInteger())
                    .arg(metrics["command_count"].toInteger())
                    .arg(metrics["widget_count"].toInteger())
                    .arg(metrics["current_theme"].toString())
                    .arg(resources["estimated_memory_kb"].toInteger())
                    .arg(resources["estimated_cpu_percent"].toDouble());

            QMessageBox::information(this, "Plugin Status", status_text);
        }
    }

private:
    void setupUI() {
        setWindowTitle("QtForge UI Plugin Example - Reorganized");
        setMinimumSize(800, 600);

        // Create central widget placeholder
        auto placeholder = new QWidget();
        auto layout = new QVBoxLayout(placeholder);

        auto welcome_label = new QLabel(
            "<h2>Welcome to QtForge UI Plugin Example</h2>"
            "<p>This example demonstrates the reorganized UI plugin structure "
            "with:</p>"
            "<ul>"
            "<li>Modular widget components</li>"
            "<li>Separated dialog classes</li>"
            "<li>Theme management system</li>"
            "<li>Plugin core architecture</li>"
            "</ul>"
            "<p>Use the menu to explore different components.</p>");
        welcome_label->setWordWrap(true);
        welcome_label->setAlignment(Qt::AlignCenter);
        layout->addWidget(welcome_label);

        auto button_layout = new QHBoxLayout();

        auto demo_button = new QPushButton("Show Demo Widget");
        connect(demo_button, &QPushButton::clicked, this,
                &MainWindow::showDemoWidget);
        button_layout->addWidget(demo_button);

        auto settings_button = new QPushButton("Settings");
        connect(settings_button, &QPushButton::clicked, this,
                &MainWindow::showSettings);
        button_layout->addWidget(settings_button);

        auto about_button = new QPushButton("About");
        connect(about_button, &QPushButton::clicked, this,
                &MainWindow::showAbout);
        button_layout->addWidget(about_button);

        layout->addLayout(button_layout);
        setCentralWidget(placeholder);

        // Setup status bar
        statusBar()->showMessage("UI Plugin Example Ready", 2000);
    }

    void setupMenus() {
        // File menu
        auto file_menu = menuBar()->addMenu("&File");

        auto show_demo_action = file_menu->addAction("Show &Demo Widget");
        connect(show_demo_action, &QAction::triggered, this,
                &MainWindow::showDemoWidget);

        file_menu->addSeparator();

        auto exit_action = file_menu->addAction("E&xit");
        connect(exit_action, &QAction::triggered, this, &QWidget::close);

        // View menu
        auto view_menu = menuBar()->addMenu("&View");

        auto themes_menu = view_menu->addMenu("&Themes");
        QStringList themes = {"default", "dark", "light", "blue", "green"};
        for (const auto& theme : themes) {
            auto theme_action = themes_menu->addAction(theme);
            connect(theme_action, &QAction::triggered, this,
                    [this, theme]() { applyTheme(theme); });
        }

        // Tools menu
        auto tools_menu = menuBar()->addMenu("&Tools");

        auto settings_action = tools_menu->addAction("&Settings");
        connect(settings_action, &QAction::triggered, this,
                &MainWindow::showSettings);

        auto status_action = tools_menu->addAction("Plugin &Status");
        connect(status_action, &QAction::triggered, this,
                &MainWindow::showPluginStatus);

        // Help menu
        auto help_menu = menuBar()->addMenu("&Help");

        auto about_action = help_menu->addAction("&About");
        connect(about_action, &QAction::triggered, this,
                &MainWindow::showAbout);
    }

    void setupConnections() {
        // Additional connections can be added here
    }

    std::unique_ptr<UIPluginCore> m_plugin_core;
    DemoWidget* m_demo_widget = nullptr;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    std::cout << "QtForge UI Plugin Example - Reorganized Structure\n";
    std::cout << "================================================\n\n";

    try {
        MainWindow window;
        window.show();

        std::cout << "✅ UI Plugin example started successfully!\n";
        std::cout << "   - Modular widget structure\n";
        std::cout << "   - Separated dialog components\n";
        std::cout << "   - Theme management system\n";
        std::cout << "   - Plugin core architecture\n\n";

        return app.exec();

    } catch (const std::exception& e) {
        std::cerr << "❌ Error starting UI plugin example: " << e.what()
                  << "\n";
        return 1;
    }
}

#include "main.moc"
