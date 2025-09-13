#!/usr/bin/env python3
"""
QtPlugin Upgrade Script
Handles version detection, compatibility checks, and smooth upgrades
"""

import os
import sys
import subprocess
import platform
import argparse
import json
import shutil
from pathlib import Path
from typing import List, Dict, Optional, Tuple
import re

class VersionInfo:
    """Version information handling"""
    
    def __init__(self, version_string: str) -> None:
        self.version_string = version_string
        self.major, self.minor, self.patch = self._parse_version(version_string)
    
    def _parse_version(self, version: str) -> Tuple[int, int, int]:
        """Parse version string into major.minor.patch"""
        # Remove 'v' prefix if present
        version = version.lstrip('v')
        
        # Extract version numbers
        match = re.match(r'(\d+)\.(\d+)\.(\d+)', version)
        if match:
            return int(match.group(1)), int(match.group(2)), int(match.group(3))
        else:
            raise ValueError(f"Invalid version format: {version}")
    
    def __str__(self) -> None:
        return f"{self.major}.{self.minor}.{self.patch}"
    
    def __lt__(self, other) -> None:
        return (self.major, self.minor, self.patch) < (other.major, other.minor, other.patch)
    
    def __le__(self, other) -> None:
        return (self.major, self.minor, self.patch) <= (other.major, other.minor, other.patch)
    
    def __gt__(self, other) -> None:
        return (self.major, self.minor, self.patch) > (other.major, other.minor, other.patch)
    
    def __ge__(self, other) -> None:
        return (self.major, self.minor, self.patch) >= (other.major, other.minor, other.patch)
    
    def __eq__(self, other) -> None:
        return (self.major, self.minor, self.patch) == (other.major, other.minor, other.patch)
    
    def is_compatible_with(self, other) -> bool:
        """Check if this version is compatible with another version"""
        # Same major version is compatible
        return self.major == other.major

class QtPluginUpgrader:
    """QtPlugin upgrade manager"""
    
    def __init__(self) -> None:
        self.system = platform.system().lower()
        self.is_windows = self.system == 'windows'
        self.is_macos = self.system == 'darwin'
        self.is_linux = self.system == 'linux'
        
        # Default installation paths
        if self.is_windows:
            self.install_paths = [
                Path("C:/Program Files/QtPlugin"),
                Path("C:/Program Files (x86)/QtPlugin"),
                Path.home() / "AppData/Local/QtPlugin"
            ]
        elif self.is_macos:
            self.install_paths = [
                Path("/usr/local"),
                Path("/opt/homebrew"),
                Path.home() / ".local"
            ]
        else:
            self.install_paths = [
                Path("/usr/local"),
                Path("/usr"),
                Path.home() / ".local"
            ]
    
    def detect_current_installation(self) -> Optional[Tuple[Path, VersionInfo]]:
        """Detect current QtPlugin installation"""
        print("🔍 Detecting current QtPlugin installation...")
        
        # Try pkg-config first
        try:
            result = subprocess.run(['pkg-config', '--modversion', 'qtplugin'], 
                                  capture_output=True, text=True)
            if result.returncode == 0:
                version = VersionInfo(result.stdout.strip())
                
                # Get installation path from pkg-config
                result = subprocess.run(['pkg-config', '--variable=prefix', 'qtplugin'], 
                                      capture_output=True, text=True)
                if result.returncode == 0:
                    path = Path(result.stdout.strip())
                    print(f"✅ Found QtPlugin {version} via pkg-config at {path}")
                    return path, version
        except FileNotFoundError:
            pass
        
        # Try CMake package registry
        for install_path in self.install_paths:
            cmake_config = install_path / "lib/cmake/QtPlugin/QtPluginConfigVersion.cmake"
            if cmake_config.exists():
                version = self._extract_version_from_cmake(cmake_config)
                if version:
                    print(f"✅ Found QtPlugin {version} at {install_path}")
                    return install_path, version
        
        # Try header files
        for install_path in self.install_paths:
            header_file = install_path / "include/qtplugin/qtplugin.hpp"
            if header_file.exists():
                version = self._extract_version_from_header(header_file)
                if version:
                    print(f"✅ Found QtPlugin {version} at {install_path}")
                    return install_path, version
        
        print("❌ No QtPlugin installation detected")
        return None
    
    def _extract_version_from_cmake(self, cmake_file: Path) -> Optional[VersionInfo]:
        """Extract version from CMake config file"""
        try:
            with open(cmake_file, 'r') as f:
                content = f.read()
                match = re.search(r'set\(PACKAGE_VERSION "([^"]+)"\)', content)
                if match:
                    return VersionInfo(match.group(1))
        except Exception:
            pass
        return None
    
    def _extract_version_from_header(self, header_file: Path) -> Optional[VersionInfo]:
        """Extract version from header file"""
        try:
            with open(header_file, 'r') as f:
                content = f.read()
                
                # Look for version defines
                major_match = re.search(r'#define\s+QTPLUGIN_VERSION_MAJOR\s+(\d+)', content)
                minor_match = re.search(r'#define\s+QTPLUGIN_VERSION_MINOR\s+(\d+)', content)
                patch_match = re.search(r'#define\s+QTPLUGIN_VERSION_PATCH\s+(\d+)', content)
                
                if major_match and minor_match and patch_match:
                    version_str = f"{major_match.group(1)}.{minor_match.group(1)}.{patch_match.group(1)}"
                    return VersionInfo(version_str)
        except Exception:
            pass
        return None
    
    def get_latest_version(self) -> Optional[VersionInfo]:
        """Get latest available version from GitHub"""
        print("🌐 Checking for latest version...")
        
        try:
            # Use GitHub API to get latest release
            import urllib.request
            import json
            
            url = "https://api.github.com/repos/QtForge/QtPlugin/releases/latest"
            with urllib.request.urlopen(url) as response:
                data = json.loads(response.read().decode())
                tag_name = data['tag_name']
                version = VersionInfo(tag_name)
                print(f"✅ Latest version: {version}")
                return version
        except Exception as e:
            print(f"⚠️  Could not check latest version: {e}")
            return None
    
    def check_compatibility(self, current: VersionInfo, target: VersionInfo) -> bool:
        """Check if upgrade is compatible"""
        print(f"🔍 Checking compatibility: {current} -> {target}")
        
        if target < current:
            print("❌ Cannot downgrade to older version")
            return False
        
        if target == current:
            print("ℹ️  Already at target version")
            return True
        
        if not current.is_compatible_with(target):
            print(f"⚠️  Major version change: {current.major} -> {target.major}")
            print("This may require manual intervention for breaking changes")
            return self._confirm_major_upgrade()
        
        print("✅ Compatible upgrade")
        return True
    
    def _confirm_major_upgrade(self) -> bool:
        """Confirm major version upgrade with user"""
        print("\n" + "="*60)
        print("⚠️  MAJOR VERSION UPGRADE WARNING")
        print("="*60)
        print("This is a major version upgrade that may include breaking changes.")
        print("Please review the changelog and migration guide before proceeding.")
        print("")
        print("Recommended steps:")
        print("1. Backup your current installation")
        print("2. Review breaking changes in the changelog")
        print("3. Update your code if necessary")
        print("4. Test thoroughly after upgrade")
        print("="*60)
        
        try:
            response = input("Do you want to continue with the major upgrade? [y/N]: ").strip().lower()
            return response in ['y', 'yes']
        except KeyboardInterrupt:
            print("\n❌ Upgrade cancelled")
            return False
    
    def backup_installation(self, install_path: Path) -> Optional[Path]:
        """Create backup of current installation"""
        print("💾 Creating backup of current installation...")
        
        backup_dir = install_path.parent / f"QtPlugin-backup-{int(__import__('time').time())}"
        
        try:
            # Create backup directory
            backup_dir.mkdir(parents=True, exist_ok=True)
            
            # Copy important files
            files_to_backup = [
                "lib/libqtplugin-*",
                "include/qtplugin/",
                "lib/cmake/QtPlugin/",
                "lib/pkgconfig/qtplugin.pc"
            ]
            
            for pattern in files_to_backup:
                source_pattern = install_path / pattern
                if '*' in pattern:
                    # Handle glob patterns
                    import glob
                    for file_path in glob.glob(str(source_pattern)):
                        rel_path = Path(file_path).relative_to(install_path)
                        dest_path = backup_dir / rel_path
                        dest_path.parent.mkdir(parents=True, exist_ok=True)
                        
                        if Path(file_path).is_file():
                            shutil.copy2(file_path, dest_path)
                        elif Path(file_path).is_dir():
                            shutil.copytree(file_path, dest_path, dirs_exist_ok=True)
                else:
                    source_path = install_path / pattern
                    if source_path.exists():
                        dest_path = backup_dir / pattern
                        dest_path.parent.mkdir(parents=True, exist_ok=True)
                        
                        if source_path.is_file():
                            shutil.copy2(source_path, dest_path)
                        elif source_path.is_dir():
                            shutil.copytree(source_path, dest_path, dirs_exist_ok=True)
            
            print(f"✅ Backup created at: {backup_dir}")
            return backup_dir
            
        except Exception as e:
            print(f"❌ Backup failed: {e}")
            return None
    
    def upgrade(self, target_version: Optional[str] = None, backup: bool = True) -> bool:
        """Perform upgrade"""
        print("🚀 Starting QtPlugin upgrade...")
        
        # Detect current installation
        current_install = self.detect_current_installation()
        if not current_install:
            print("❌ No current installation found. Use install script instead.")
            return False
        
        install_path, current_version = current_install
        
        # Get target version
        if target_version:
            target_ver = VersionInfo(target_version)
        else:
            target_ver = self.get_latest_version()
            if not target_ver:
                print("❌ Could not determine target version")
                return False
        
        # Check compatibility
        if not self.check_compatibility(current_version, target_ver):
            return False
        
        if target_ver == current_version:
            print("✅ Already at the latest version")
            return True
        
        # Create backup if requested
        backup_path = None
        if backup:
            backup_path = self.backup_installation(install_path)
            if not backup_path:
                print("❌ Backup failed. Aborting upgrade.")
                return False
        
        # Download and install new version
        print(f"📥 Upgrading to QtPlugin {target_ver}...")
        
        # Use the install script for the actual installation
        install_script = Path(__file__).parent / "install.py"
        if install_script.exists():
            try:
                cmd = [sys.executable, str(install_script), "--prefix", str(install_path)]
                result = subprocess.run(cmd, check=True)
                
                if result.returncode == 0:
                    print(f"✅ Successfully upgraded to QtPlugin {target_ver}")
                    
                    if backup_path:
                        print(f"💾 Backup available at: {backup_path}")
                        print("You can remove the backup once you've verified the upgrade works correctly.")
                    
                    return True
                else:
                    print("❌ Upgrade failed")
                    return False
                    
            except subprocess.CalledProcessError as e:
                print(f"❌ Upgrade failed: {e}")
                
                # Offer to restore backup
                if backup_path and self._confirm_restore_backup():
                    self._restore_backup(backup_path, install_path)
                
                return False
        else:
            print("❌ Install script not found")
            return False
    
    def _confirm_restore_backup(self) -> bool:
        """Confirm backup restoration"""
        try:
            response = input("Do you want to restore the backup? [y/N]: ").strip().lower()
            return response in ['y', 'yes']
        except KeyboardInterrupt:
            return False
    
    def _restore_backup(self, backup_path: Path, install_path: Path) -> bool:
        """Restore from backup"""
        print("🔄 Restoring from backup...")
        
        try:
            # Remove current installation
            for item in install_path.glob("*qtplugin*"):
                if item.is_file():
                    item.unlink()
                elif item.is_dir():
                    shutil.rmtree(item)
            
            # Restore from backup
            for item in backup_path.rglob("*"):
                if item.is_file():
                    rel_path = item.relative_to(backup_path)
                    dest_path = install_path / rel_path
                    dest_path.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(item, dest_path)
            
            print("✅ Backup restored successfully")
            return True
            
        except Exception as e:
            print(f"❌ Backup restoration failed: {e}")
            return False

def main() -> None:
    parser = argparse.ArgumentParser(description='QtPlugin Upgrade Script')
    parser.add_argument('--version', type=str,
                       help='Target version to upgrade to (default: latest)')
    parser.add_argument('--no-backup', action='store_true',
                       help='Skip creating backup before upgrade')
    parser.add_argument('--check-only', action='store_true',
                       help='Only check current version and available updates')
    
    args = parser.parse_args()
    
    upgrader = QtPluginUpgrader()
    
    if args.check_only:
        current = upgrader.detect_current_installation()
        latest = upgrader.get_latest_version()
        
        if current:
            _, current_version = current
            print(f"Current version: {current_version}")
        else:
            print("No installation found")
        
        if latest:
            print(f"Latest version: {latest}")
            
            if current and latest > current_version:
                print("🔄 Update available!")
            elif current and latest == current_version:
                print("✅ Up to date")
        
        return
    
    # Perform upgrade
    success = upgrader.upgrade(
        target_version=args.version,
        backup=not args.no_backup
    )
    
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()
