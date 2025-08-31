/**
 * @file about_dialog.cpp
 * @brief About dialog implementation
 * @version 3.0.0
 */

#include "about_dialog.hpp"

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
