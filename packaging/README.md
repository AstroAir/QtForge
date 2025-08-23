# QtPlugin Packaging and Distribution

This directory contains comprehensive packaging and distribution configurations for QtPlugin across multiple platforms and package managers.

## 📦 Supported Package Formats

### Windows
- **NSIS Installer** (`.exe`) - User-friendly installer with GUI
- **MSI Package** (`.msi`) - Windows Installer package for enterprise deployment
- **ZIP Archive** (`.zip`) - Portable installation
- **NuGet Package** - For Visual Studio and MSBuild integration

### macOS
- **DMG Image** (`.dmg`) - Standard macOS installer with drag-and-drop interface
- **PKG Installer** (`.pkg`) - macOS package installer
- **Homebrew Formula** - For Homebrew package manager
- **TAR Archive** (`.tar.gz`) - Portable installation

### Linux
- **DEB Package** (`.deb`) - For Debian/Ubuntu and derivatives
- **RPM Package** (`.rpm`) - For Red Hat/Fedora/SUSE and derivatives
- **AppImage** (`.AppImage`) - Portable application format
- **Flatpak** (`.flatpak`) - Universal Linux package format
- **TAR Archive** (`.tar.gz`) - Portable installation

## 🛠️ Package Manager Integration

### vcpkg
```bash
vcpkg install qtplugin
```

### Conan
```bash
conan install qtplugin/3.0.0@
```

### Homebrew (macOS/Linux)
```bash
brew install qtplugin
```

### NuGet (Windows)
```xml
<PackageReference Include="QtPlugin" Version="3.0.0" />
```

## 🚀 Quick Installation

### Universal Install Script
```bash
# Download and run the universal installer
curl -sSL https://raw.githubusercontent.com/QtForge/QtPlugin/main/scripts/install.py | python3
```

### Platform-Specific Scripts
```bash
# Windows (PowerShell)
.\scripts\build.bat --package

# Unix/Linux/macOS
./scripts/build.sh --package
```

## 📋 Package Contents

All packages include:

### Core Components
- **QtPlugin Core Library** - Essential plugin system functionality
- **QtPlugin Security Module** - Security validation and permission management
- **Headers** - C++ header files for development
- **CMake Integration** - Find modules and config files
- **pkg-config Support** - For build system integration

### Optional Components
- **Network Support** - Network plugin capabilities (if enabled)
- **UI Support** - User interface plugin support (if enabled)
- **Examples** - Sample plugins and demonstrations
- **Documentation** - API documentation and guides

## 🔧 Build Configuration

### CMake Options
```cmake
# Core options
-DQTPLUGIN_BUILD_NETWORK=ON     # Enable network support
-DQTPLUGIN_BUILD_UI=ON          # Enable UI support
-DQTPLUGIN_BUILD_EXAMPLES=ON    # Build examples
-DQTPLUGIN_BUILD_TESTS=OFF      # Skip tests in packages

# Package-specific options
-DCPACK_GENERATOR="DEB;RPM"     # Specify package formats
```

### Package Generators by Platform
- **Linux**: `DEB;RPM;TGZ;STGZ`
- **Windows**: `NSIS;ZIP;WIX`
- **macOS**: `DragNDrop;TGZ;productbuild`

## 📁 Directory Structure

```
packaging/
├── debian/           # Debian package configuration
│   ├── postinst     # Post-installation script
│   └── prerm        # Pre-removal script
├── rpm/             # RPM package configuration
│   ├── postinstall.sh
│   └── preuninstall.sh
├── macos/           # macOS packaging
│   ├── postinstall
│   └── setup_dmg.applescript
├── appimage/        # AppImage configuration
│   └── build-appimage.sh
├── flatpak/         # Flatpak configuration
│   ├── org.qtforge.QtPlugin.yml
│   ├── org.qtforge.QtPlugin.desktop
│   └── org.qtforge.QtPlugin.metainfo.xml
├── vcpkg/           # vcpkg port files
│   ├── portfile.cmake
│   ├── vcpkg.json
│   └── usage
├── conan/           # Conan package
│   └── conanfile.py
├── homebrew/        # Homebrew formula
│   └── qtplugin.rb
└── nuget/           # NuGet package
    ├── QtPlugin.nuspec
    └── QtPlugin.targets
```

## 🔄 Upgrade Process

### Automatic Upgrade
```bash
# Check for updates
python3 scripts/upgrade.py --check-only

# Upgrade to latest version
python3 scripts/upgrade.py

# Upgrade to specific version
python3 scripts/upgrade.py --version 3.1.0
```

### Manual Upgrade
1. Download the new package for your platform
2. Backup current installation (optional)
3. Install new package (will replace old version)
4. Verify installation

## 🗑️ Uninstallation

### Universal Uninstaller
```bash
python3 scripts/uninstall.py
```

### Platform-Specific
- **Windows**: Use "Add or Remove Programs" or run the uninstaller
- **macOS**: Delete from Applications or use package manager
- **Linux**: Use package manager (`apt remove`, `yum remove`, etc.)

## 🔐 Code Signing

### Windows
- MSI packages can be signed with Authenticode certificates
- NSIS installers support code signing

### macOS
- DMG and PKG packages can be signed with Apple Developer certificates
- Notarization supported for distribution outside App Store

### Linux
- DEB and RPM packages can be signed with GPG keys
- AppImage and Flatpak support signing

## 🤖 CI/CD Integration

The packaging process is fully automated through GitHub Actions:

1. **Build and Test** - Compile and test on all platforms
2. **Package Creation** - Generate packages for each platform
3. **AppImage/Flatpak** - Create universal Linux packages
4. **Release** - Automatically create GitHub releases with all packages

### Triggers
- **Push to main** - Create development packages
- **Tag creation** - Create release packages
- **Pull requests** - Test packaging without creating packages

## 📊 Package Verification

### Checksums
All packages include SHA256 checksums for verification:
```bash
# Verify package integrity
sha256sum -c QtPlugin-3.0.0-checksums.txt
```

### Digital Signatures
- Windows packages are signed with Authenticode
- macOS packages are signed and notarized
- Linux packages include GPG signatures

## 🆘 Troubleshooting

### Common Issues

1. **Qt6 not found**
   - Install Qt6 development packages
   - Set `Qt6_DIR` environment variable

2. **CMake version too old**
   - Install CMake 3.21 or later
   - Use the provided build scripts

3. **Permission denied during installation**
   - Run installer as administrator (Windows)
   - Use `sudo` for system-wide installation (Unix)

4. **Package conflicts**
   - Remove old versions before installing
   - Use the upgrade script for smooth transitions

### Getting Help
- Check the [Installation Guide](../docs/INSTALLATION.md)
- Review [Troubleshooting](../docs/TROUBLESHOOTING.md)
- Open an issue on [GitHub](https://github.com/QtForge/QtPlugin/issues)

## 📄 License

All packaging configurations are provided under the same MIT license as QtPlugin.
