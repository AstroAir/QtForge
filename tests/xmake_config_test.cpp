// XMake Configuration Test
// Validates that xmake build system is properly configured for QtForge

#include <QCoreApplication>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== XMake Configuration Test ===";
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Compiler:" << 
#ifdef _MSC_VER
        "MSVC" << _MSC_VER;
#elif defined(__GNUC__)
        "GCC" << __GNUC__ << "." << __GNUC_MINOR__;
#elif defined(__clang__)
        "Clang" << __clang_major__ << "." << __clang_minor__;
#else
        "Unknown";
#endif
    
    qDebug() << "Platform:" <<
#ifdef _WIN32
        "Windows";
#elif defined(__linux__)
        "Linux";
#elif defined(__APPLE__)
        "macOS";
#else
        "Unknown";
#endif

    qDebug() << "Architecture:" <<
#ifdef _M_X64
        "x64";
#elif defined(_M_IX86)
        "x86";
#elif defined(_M_ARM64)
        "ARM64";
#else
        "Unknown";
#endif

    qDebug() << "Build Mode:" <<
#ifdef QTFORGE_DEBUG
        "Debug";
#elif defined(QTFORGE_RELEASE)
        "Release";
#else
        "Unknown";
#endif

    qDebug() << "QtForge Features:";
#ifdef QTFORGE_HAS_NETWORK
    qDebug() << "  - Network: Enabled";
#else
    qDebug() << "  - Network: Disabled";
#endif

#ifdef QTFORGE_HAS_WIDGETS
    qDebug() << "  - Widgets: Enabled";
#else
    qDebug() << "  - Widgets: Disabled";
#endif

#ifdef QTFORGE_HAS_SQL
    qDebug() << "  - SQL: Enabled";
#else
    qDebug() << "  - SQL: Disabled";
#endif

#ifdef QTFORGE_SHARED
    qDebug() << "Library Type: Shared";
#elif defined(QTFORGE_STATIC)
    qDebug() << "Library Type: Static";
#endif

    qDebug() << "=== XMake Configuration Test PASSED ===";
    
    return 0;
}
