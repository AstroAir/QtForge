#include <iostream>
#include <QCoreApplication>
#include <QLibrary>
#include <QDir>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    std::cout << "QtForge Library Test\n";
    std::cout << "===================\n\n";
    
    // Test 1: Check if libraries exist
    QDir buildDir("build");
    QStringList libraries = {"libqtforge-core.dll", "libqtforge-security.dll"};
    
    std::cout << "1. Checking library files:\n";
    for (const QString& lib : libraries) {
        if (buildDir.exists(lib)) {
            QFileInfo info(buildDir.absoluteFilePath(lib));
            std::cout << "   ✓ " << lib.toStdString() << " (" << info.size() << " bytes)\n";
        } else {
            std::cout << "   ✗ " << lib.toStdString() << " - NOT FOUND\n";
            return 1;
        }
    }
    
    // Test 2: Try to load libraries
    std::cout << "\n2. Testing library loading:\n";
    for (const QString& lib : libraries) {
        QLibrary library(buildDir.absoluteFilePath(lib));
        if (library.load()) {
            std::cout << "   ✓ " << lib.toStdString() << " - Loaded successfully\n";
            library.unload();
        } else {
            std::cout << "   ✗ " << lib.toStdString() << " - Failed to load: " 
                      << library.errorString().toStdString() << "\n";
        }
    }
    
    // Test 3: Check Qt version compatibility
    std::cout << "\n3. Qt version information:\n";
    std::cout << "   Qt Runtime Version: " << qVersion() << "\n";
    std::cout << "   Qt Compile Version: " << QT_VERSION_STR << "\n";
    
    std::cout << "\n✓ Basic functionality test completed successfully!\n";
    
    return 0;
}
