#!/bin/bash
# QtPlugin RPM package post-installation script

# Package information
PACKAGE_NAME="qtplugin"
LIBRARY_PATH="/usr/lib64"
INCLUDE_PATH="/usr/include"
PKG_CONFIG_PATH="/usr/lib64/pkgconfig"

echo "Configuring QtPlugin library..."

# Update library cache
if command -v ldconfig >/dev/null 2>&1; then
    echo "Updating library cache..."
    ldconfig
fi

# Update pkg-config cache
if command -v pkg-config >/dev/null 2>&1; then
    echo "Updating pkg-config cache..."
    pkg-config --list-all | grep -q qtplugin && echo "QtPlugin pkg-config found" || true
fi

# Ensure proper permissions
if [ -d "$LIBRARY_PATH" ]; then
    chmod 755 "$LIBRARY_PATH"/libqtplugin-*.so.* 2>/dev/null || true
fi

if [ -d "$INCLUDE_PATH/qtplugin" ]; then
    chmod -R 644 "$INCLUDE_PATH"/qtplugin/*.hpp 2>/dev/null || true
    find "$INCLUDE_PATH/qtplugin" -type d -exec chmod 755 {} \; 2>/dev/null || true
fi

# Create CMake package registry entry
CMAKE_REGISTRY_DIR="/usr/share/cmake/Modules"
if [ -d "$CMAKE_REGISTRY_DIR" ]; then
    echo "# QtPlugin CMake module" > "$CMAKE_REGISTRY_DIR/FindQtPlugin.cmake" 2>/dev/null || true
fi

echo "QtPlugin library configured successfully."
echo ""
echo "To use QtPlugin in your CMake projects, add:"
echo "  find_package(QtPlugin REQUIRED COMPONENTS Core)"
echo ""
echo "For pkg-config, use:"
echo "  pkg-config --cflags --libs qtplugin"
echo ""

exit 0
