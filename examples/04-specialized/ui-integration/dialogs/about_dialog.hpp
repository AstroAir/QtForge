/**
 * @file about_dialog.hpp
 * @brief About dialog for UI plugin information
 * @version 3.0.0
 */

#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

/**
 * @brief About dialog showing plugin information
 */
class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog() override;

private:
    void setupUI();
};
