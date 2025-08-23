#!/usr/bin/env python3
"""
QtPlugin Universal Uninstallation Script
Handles clean removal of QtPlugin library and system integration
"""

import os
import sys
import subprocess
import platform
import argparse
import shutil
import json
from pathlib import Path
from typing import List, Dict, Optional

class QtPluginUninstaller:
    """QtPlugin uninstaller"""
    
    def __init__(self, install_prefix: Optional[Path] = None):
        self.system = platform.system().lower()
        self.is_windows = self.system == 'windows'
        self.is_macos = self.system == 'darwin'
        self.is_linux = self.system == 'linux'
        
        # Set default install prefix
        if install_prefix:
            self.install_prefix = install_prefix
        elif self.is_windows:
            self.install_prefix = Path("C:/Program Files/QtPlugin")
        elif self.is_macos:
            self.install_prefix = Path("/usr/local")
        else:
            self.install_prefix = Path("/usr/local")
    
    def uninstall(self, options: Dict[str, any]) -> bool:
        """Main uninstallation process"""
        print(f"🗑️  Uninstalling QtPlugin from {self.install_prefix}")
        
        if not self._confirm_uninstall():
            print("❌ Uninstallation cancelled")
            return False
        
        success = True
        
        # Remove files
        if not self._remove_files():
            success = False
        
        # Clean system integration
        if not self._clean_system_integration():
            print("⚠️  Some system integration cleanup failed")
        
        # Remove from package registries
        if not self._clean_package_registries():
            print("⚠️  Some package registry cleanup failed")
        
        if success:
            print("✅ QtPlugin uninstalled successfully!")
        else:
            print("⚠️  Uninstallation completed with some errors")
        
        return success
    
    def _confirm_uninstall(self) -> bool:
        """Confirm uninstallation with user"""
        print("\n" + "="*50)
        print("⚠️  QtPlugin Uninstallation")
        print("="*50)
        print(f"This will remove QtPlugin from: {self.install_prefix}")
        print("\nThe following will be removed:")
        
        # List what will be removed
        items_to_remove = [
            f"{self.install_prefix}/lib/libqtplugin-*",
            f"{self.install_prefix}/include/qtplugin/",
            f"{self.install_prefix}/lib/cmake/QtPlugin/",
            f"{self.install_prefix}/lib/pkgconfig/qtplugin.pc",
            f"{self.install_prefix}/share/doc/qtplugin/",
            f"{self.install_prefix}/share/qtplugin/"
        ]
        
        for item in items_to_remove:
            if Path(item.split('*')[0]).parent.exists():
                print(f"  - {item}")
        
        print("\nSystem integration cleanup:")
        if self.is_linux:
            print("  - Library cache (ldconfig)")
            print("  - CMake package registry")
        elif self.is_macos:
            print("  - Dynamic library cache")
            print("  - Homebrew symlinks (if applicable)")
        
        print("="*50)
        
        try:
            response = input("Do you want to continue? [y/N]: ").strip().lower()
            return response in ['y', 'yes']
        except KeyboardInterrupt:
            print("\n❌ Uninstallation cancelled")
            return False
    
    def _remove_files(self) -> bool:
        """Remove QtPlugin files"""
        print("🗂️  Removing QtPlugin files...")
        
        success = True
        files_to_remove = [
            # Libraries
            self.install_prefix / "lib" / "libqtplugin-core.a",
            self.install_prefix / "lib" / "libqtplugin-core.so",
            self.install_prefix / "lib" / "libqtplugin-core.dylib",
            self.install_prefix / "lib" / "libqtplugin-security.a",
            self.install_prefix / "lib" / "libqtplugin-security.so",
            self.install_prefix / "lib" / "libqtplugin-security.dylib",
            self.install_prefix / "lib" / "libqtplugin-network.a",
            self.install_prefix / "lib" / "libqtplugin-network.so",
            self.install_prefix / "lib" / "libqtplugin-network.dylib",
            self.install_prefix / "lib" / "libqtplugin-ui.a",
            self.install_prefix / "lib" / "libqtplugin-ui.so",
            self.install_prefix / "lib" / "libqtplugin-ui.dylib",
            
            # pkg-config
            self.install_prefix / "lib" / "pkgconfig" / "qtplugin.pc",
        ]
        
        directories_to_remove = [
            # Headers
            self.install_prefix / "include" / "qtplugin",
            
            # CMake files
            self.install_prefix / "lib" / "cmake" / "QtPlugin",
            
            # Documentation
            self.install_prefix / "share" / "doc" / "qtplugin",
            
            # Examples and data
            self.install_prefix / "share" / "qtplugin",
        ]
        
        # Remove files
        for file_path in files_to_remove:
            if file_path.exists():
                try:
                    file_path.unlink()
                    print(f"  ✅ Removed: {file_path}")
                except PermissionError:
                    try:
                        # Try with sudo on Unix systems
                        if not self.is_windows:
                            subprocess.run(['sudo', 'rm', '-f', str(file_path)], check=True)
                            print(f"  ✅ Removed: {file_path}")
                        else:
                            print(f"  ❌ Permission denied: {file_path}")
                            success = False
                    except subprocess.CalledProcessError:
                        print(f"  ❌ Failed to remove: {file_path}")
                        success = False
                except Exception as e:
                    print(f"  ❌ Error removing {file_path}: {e}")
                    success = False
        
        # Remove directories
        for dir_path in directories_to_remove:
            if dir_path.exists():
                try:
                    shutil.rmtree(dir_path)
                    print(f"  ✅ Removed directory: {dir_path}")
                except PermissionError:
                    try:
                        # Try with sudo on Unix systems
                        if not self.is_windows:
                            subprocess.run(['sudo', 'rm', '-rf', str(dir_path)], check=True)
                            print(f"  ✅ Removed directory: {dir_path}")
                        else:
                            print(f"  ❌ Permission denied: {dir_path}")
                            success = False
                    except subprocess.CalledProcessError:
                        print(f"  ❌ Failed to remove directory: {dir_path}")
                        success = False
                except Exception as e:
                    print(f"  ❌ Error removing directory {dir_path}: {e}")
                    success = False
        
        return success
    
    def _clean_system_integration(self) -> bool:
        """Clean system integration"""
        print("🔧 Cleaning system integration...")
        
        success = True
        
        try:
            if self.is_linux:
                # Update library cache
                print("  Updating library cache...")
                subprocess.run(['sudo', 'ldconfig'], check=False)
                
                # Update desktop database if available
                if shutil.which('update-desktop-database'):
                    subprocess.run(['update-desktop-database'], check=False)
            
            elif self.is_macos:
                # Update dylib cache
                print("  Updating dynamic library cache...")
                subprocess.run(['update_dyld_shared_cache'], check=False)
                
                # Remove Homebrew symlinks if they exist
                if shutil.which('brew'):
                    brew_prefix = subprocess.run(['brew', '--prefix'], 
                                               capture_output=True, text=True)
                    if brew_prefix.returncode == 0:
                        brew_path = Path(brew_prefix.stdout.strip())
                        
                        # Remove library symlinks
                        for lib_file in (brew_path / "lib").glob("libqtplugin-*"):
                            if lib_file.is_symlink():
                                lib_file.unlink()
                                print(f"  ✅ Removed Homebrew symlink: {lib_file}")
                        
                        # Remove include symlinks
                        qtplugin_include = brew_path / "include" / "qtplugin"
                        if qtplugin_include.is_symlink():
                            qtplugin_include.unlink()
                            print(f"  ✅ Removed Homebrew symlink: {qtplugin_include}")
                        
                        # Remove pkg-config symlinks
                        pc_file = brew_path / "lib" / "pkgconfig" / "qtplugin.pc"
                        if pc_file.is_symlink():
                            pc_file.unlink()
                            print(f"  ✅ Removed Homebrew symlink: {pc_file}")
            
        except Exception as e:
            print(f"  ⚠️  System integration cleanup warning: {e}")
            success = False
        
        return success
    
    def _clean_package_registries(self) -> bool:
        """Clean package registries"""
        print("📦 Cleaning package registries...")
        
        success = True
        
        try:
            # CMake package registry
            cmake_registry_paths = []
            
            if self.is_windows:
                # Windows registry locations
                import winreg
                try:
                    key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, 
                                       r"Software\Kitware\CMake\Packages\QtPlugin")
                    winreg.DeleteKey(winreg.HKEY_CURRENT_USER, 
                                   r"Software\Kitware\CMake\Packages\QtPlugin")
                    print("  ✅ Removed from Windows CMake registry")
                except (FileNotFoundError, OSError):
                    pass
            else:
                # Unix CMake registry locations
                home = Path.home()
                cmake_registry_paths = [
                    home / ".cmake" / "packages" / "QtPlugin",
                    Path("/usr/share/cmake/packages/QtPlugin"),
                ]
            
            for registry_path in cmake_registry_paths:
                if registry_path.exists():
                    try:
                        shutil.rmtree(registry_path)
                        print(f"  ✅ Removed CMake registry: {registry_path}")
                    except Exception as e:
                        print(f"  ⚠️  Failed to remove CMake registry {registry_path}: {e}")
                        success = False
            
        except Exception as e:
            print(f"  ⚠️  Package registry cleanup warning: {e}")
            success = False
        
        return success

def main():
    parser = argparse.ArgumentParser(description='QtPlugin Universal Uninstallation Script')
    parser.add_argument('--prefix', type=Path,
                       help='Installation prefix to remove from (default: system-dependent)')
    parser.add_argument('--force', action='store_true',
                       help='Skip confirmation prompt')
    
    args = parser.parse_args()
    
    # Create uninstaller
    uninstaller = QtPluginUninstaller(args.prefix)
    
    # Uninstallation options
    options = {
        'force': args.force
    }
    
    # Override confirmation if force is specified
    if args.force:
        uninstaller._confirm_uninstall = lambda: True
    
    # Run uninstallation
    if uninstaller.uninstall(options):
        sys.exit(0)
    else:
        sys.exit(1)

if __name__ == '__main__':
    main()
