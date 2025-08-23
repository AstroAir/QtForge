#!/bin/bash
# QtPlugin RPM package pre-uninstallation script

# Package information
PACKAGE_NAME="qtplugin"

echo "Preparing to remove QtPlugin library..."

# Remove CMake module if it exists
CMAKE_MODULE="/usr/share/cmake/Modules/FindQtPlugin.cmake"
if [ -f "$CMAKE_MODULE" ]; then
    rm -f "$CMAKE_MODULE" 2>/dev/null || true
fi

echo "QtPlugin library removal prepared."

exit 0
