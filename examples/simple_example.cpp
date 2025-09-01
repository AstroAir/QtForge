// Simple QtForge Example
// Demonstrates basic QtForge integration with xmake build system

#include <QCoreApplication>
#include <QDebug>
#include <iostream>

// Include QtForge headers if available
#ifdef QTFORGE_VERSION_STRING
#include "qtplugin/qtplugin.hpp"
#endif

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "QtForge Simple Example";
    qDebug() << "Built with xmake build system";
    
#ifdef QTFORGE_VERSION_STRING
    qDebug() << "QtForge Version:" << QTFORGE_VERSION_STRING;
#else
    qDebug() << "QtForge headers not found - this is expected for initial testing";
#endif
    
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Example completed successfully!";
    
    return 0;
}
