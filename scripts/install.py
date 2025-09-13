#!/usr/bin/env python3
"""
QtPlugin Universal Installation Script
Handles installation, dependency resolution, and system integration
"""

import os
import sys
import subprocess
import platform
import argparse
import shutil
import json
from pathlib import Path
from typing import List, Dict, Optional, Tuple

class SystemInfo:
    """System information detection"""
    
    def __init__(self) -> None:
        self.system = platform.system().lower()
        self.machine = platform.machine().lower()
        self.is_windows = self.system == 'windows'
        self.is_macos = self.system == 'darwin'
        self.is_linux = self.system == 'linux'
        
        # Detect Linux distribution
        self.distro = None
        self.package_manager = None
        
        if self.is_linux:
            self._detect_linux_distro()
        
        # Detect architecture
        if self.machine in ['x86_64', 'amd64']:
            self.arch = 'x64'
        elif self.machine in ['aarch64', 'arm64']:
            self.arch = 'arm64'
        elif self.machine.startswith('arm'):
            self.arch = 'arm'
        else:
            self.arch = 'x86'
    
    def _detect_linux_distro(self) -> None:
        """Detect Linux distribution and package manager"""
        try:
            # Try /etc/os-release first
            if Path('/etc/os-release').exists():
                with open('/etc/os-release', 'r') as f:
                    for line in f:
                        if line.startswith('ID='):
                            self.distro = line.split('=')[1].strip().strip('"')
                            break
            
            # Detect package manager
            if shutil.which('apt'):
                self.package_manager = 'apt'
            elif shutil.which('yum'):
                self.package_manager = 'yum'
            elif shutil.which('dnf'):
                self.package_manager = 'dnf'
            elif shutil.which('pacman'):
                self.package_manager = 'pacman'
            elif shutil.which('zypper'):
                self.package_manager = 'zypper'
            
        except Exception:
            pass

class DependencyManager:
    """Handles dependency detection and installation"""
    
    def __init__(self, system_info: SystemInfo) -> None:
        self.system_info = system_info
        self.required_deps = {
            'cmake': '3.21',
            'qt6': '6.0.0'
        }
    
    def check_cmake(self) -> Tuple[bool, Optional[str]]:
        """Check if CMake is available and meets version requirements"""
        try:
            result = subprocess.run(['cmake', '--version'], 
                                  capture_output=True, text=True)
            if result.returncode == 0:
                version_line = result.stdout.split('\n')[0]
                version = version_line.split()[2]
                return True, version
        except FileNotFoundError:
            pass
        return False, None
    
    def check_qt6(self) -> Tuple[bool, Optional[str]]:
        """Check if Qt6 is available"""
        # Try qmake6 first
        for qmake in ['qmake6', 'qmake']:
            try:
                result = subprocess.run([qmake, '-version'], 
                                      capture_output=True, text=True)
                if result.returncode == 0 and 'Qt version 6' in result.stdout:
                    lines = result.stdout.split('\n')
                    for line in lines:
                        if 'Qt version' in line:
                            version = line.split()[2]
                            return True, version
            except FileNotFoundError:
                continue
        
        # Try pkg-config
        try:
            result = subprocess.run(['pkg-config', '--modversion', 'Qt6Core'], 
                                  capture_output=True, text=True)
            if result.returncode == 0:
                return True, result.stdout.strip()
        except FileNotFoundError:
            pass
        
        return False, None
    
    def install_dependencies(self) -> bool:
        """Install missing dependencies"""
        print("🔍 Checking dependencies...")
        
        cmake_ok, cmake_version = self.check_cmake()
        qt6_ok, qt6_version = self.check_qt6()
        
        if cmake_ok:
            print(f"✅ CMake {cmake_version} found")
        else:
            print("❌ CMake not found")
            if not self._install_cmake():
                return False
        
        if qt6_ok:
            print(f"✅ Qt6 {qt6_version} found")
        else:
            print("❌ Qt6 not found")
            if not self._install_qt6():
                return False
        
        return True
    
    def _install_cmake(self) -> bool:
        """Install CMake using system package manager"""
        if self.system_info.is_windows:
            print("Please install CMake from https://cmake.org/download/")
            return False
        elif self.system_info.is_macos:
            if shutil.which('brew'):
                return self._run_command(['brew', 'install', 'cmake'])
            else:
                print("Please install Homebrew or CMake manually")
                return False
        else:
            if self.system_info.package_manager == 'apt':
                return self._run_command(['sudo', 'apt', 'update']) and \
                       self._run_command(['sudo', 'apt', 'install', '-y', 'cmake'])
            elif self.system_info.package_manager in ['yum', 'dnf']:
                return self._run_command(['sudo', self.system_info.package_manager, 
                                        'install', '-y', 'cmake'])
            elif self.system_info.package_manager == 'pacman':
                return self._run_command(['sudo', 'pacman', '-S', '--noconfirm', 'cmake'])
            else:
                print("Please install CMake using your system package manager")
                return False
    
    def _install_qt6(self) -> bool:
        """Install Qt6 using system package manager"""
        if self.system_info.is_windows:
            print("Please install Qt6 from https://www.qt.io/download")
            return False
        elif self.system_info.is_macos:
            if shutil.which('brew'):
                return self._run_command(['brew', 'install', 'qt@6'])
            else:
                print("Please install Homebrew or Qt6 manually")
                return False
        else:
            if self.system_info.package_manager == 'apt':
                return self._run_command(['sudo', 'apt', 'update']) and \
                       self._run_command(['sudo', 'apt', 'install', '-y', 
                                        'qt6-base-dev', 'qt6-tools-dev'])
            elif self.system_info.package_manager in ['yum', 'dnf']:
                return self._run_command(['sudo', self.system_info.package_manager, 
                                        'install', '-y', 'qt6-qtbase-devel'])
            elif self.system_info.package_manager == 'pacman':
                return self._run_command(['sudo', 'pacman', '-S', '--noconfirm', 'qt6-base'])
            else:
                print("Please install Qt6 using your system package manager")
                return False
    
    def _run_command(self, cmd: List[str]) -> bool:
        """Run a command and return success status"""
        try:
            result = subprocess.run(cmd, check=True)
            return result.returncode == 0
        except subprocess.CalledProcessError:
            return False
        except FileNotFoundError:
            return False

class QtPluginInstaller:
    """Main installer class"""
    
    def __init__(self, source_dir: Path, install_prefix: Optional[Path] = None) -> None:
        self.source_dir = source_dir
        self.system_info = SystemInfo()
        self.dependency_manager = DependencyManager(self.system_info)
        
        # Set default install prefix
        if install_prefix:
            self.install_prefix = install_prefix
        elif self.system_info.is_windows:
            self.install_prefix = Path("C:/Program Files/QtPlugin")
        elif self.system_info.is_macos:
            self.install_prefix = Path("/usr/local")
        else:
            self.install_prefix = Path("/usr/local")
        
        self.build_dir = self.source_dir / "build-install"
    
    def install(self, options: Dict[str, any]) -> bool:
        """Main installation process"""
        print(f"🚀 Installing QtPlugin on {self.system_info.system} ({self.system_info.arch})")
        print(f"📁 Install prefix: {self.install_prefix}")
        
        # Check and install dependencies
        if not self.dependency_manager.install_dependencies():
            print("❌ Failed to install dependencies")
            return False
        
        # Build and install
        if not self._build_and_install(options):
            print("❌ Build and installation failed")
            return False
        
        # System integration
        if not self._system_integration():
            print("⚠️  System integration failed, but installation completed")
        
        print("✅ QtPlugin installed successfully!")
        self._print_usage_info()
        return True
    
    def _build_and_install(self, options: Dict[str, any]) -> bool:
        """Build and install QtPlugin"""
        print("🔨 Building QtPlugin...")
        
        # Clean build directory
        if self.build_dir.exists():
            shutil.rmtree(self.build_dir)
        self.build_dir.mkdir(parents=True)
        
        # Configure
        cmake_args = [
            'cmake',
            '-S', str(self.source_dir),
            '-B', str(self.build_dir),
            f'-DCMAKE_BUILD_TYPE=Release',
            f'-DCMAKE_INSTALL_PREFIX={self.install_prefix}',
        ]
        
        if options.get('build_examples', True):
            cmake_args.append('-DQTPLUGIN_BUILD_EXAMPLES=ON')
        
        if options.get('build_network', False):
            cmake_args.append('-DQTPLUGIN_BUILD_NETWORK=ON')
        
        if options.get('build_ui', False):
            cmake_args.append('-DQTPLUGIN_BUILD_UI=ON')
        
        try:
            subprocess.run(cmake_args, check=True)
            print("✅ Configuration successful")
        except subprocess.CalledProcessError:
            print("❌ Configuration failed")
            return False
        
        # Build
        try:
            subprocess.run([
                'cmake', '--build', str(self.build_dir),
                '--config', 'Release',
                '--parallel', str(os.cpu_count() or 4)
            ], check=True)
            print("✅ Build successful")
        except subprocess.CalledProcessError:
            print("❌ Build failed")
            return False
        
        # Install
        install_cmd = ['cmake', '--install', str(self.build_dir)]
        
        # Add sudo for Unix systems if not root
        if not self.system_info.is_windows and os.geteuid() != 0:
            install_cmd = ['sudo'] + install_cmd
        
        try:
            subprocess.run(install_cmd, check=True)
            print("✅ Installation successful")
            return True
        except subprocess.CalledProcessError:
            print("❌ Installation failed")
            return False
    
    def _system_integration(self) -> bool:
        """Perform system integration"""
        print("🔧 Performing system integration...")
        
        try:
            if self.system_info.is_linux:
                # Update library cache
                subprocess.run(['sudo', 'ldconfig'], check=False)
                
                # Update desktop database if available
                if shutil.which('update-desktop-database'):
                    subprocess.run(['update-desktop-database'], check=False)
            
            elif self.system_info.is_macos:
                # Update dylib cache
                subprocess.run(['update_dyld_shared_cache'], check=False)
            
            return True
        except Exception as e:
            print(f"⚠️  System integration warning: {e}")
            return False
    
    def _print_usage_info(self) -> None:
        """Print usage information"""
        print("\n" + "="*50)
        print("📚 QtPlugin Installation Complete!")
        print("="*50)
        print(f"📁 Installed to: {self.install_prefix}")
        print("\n🔧 Usage in CMake projects:")
        print("   find_package(QtPlugin REQUIRED COMPONENTS Core)")
        print("   target_link_libraries(your_target QtPlugin::Core)")
        print("\n📦 Usage with pkg-config:")
        print("   pkg-config --cflags --libs qtplugin")
        print("\n📖 Documentation:")
        print(f"   {self.install_prefix}/share/doc/qtplugin/")
        print("\n🧪 Examples:")
        print(f"   {self.install_prefix}/share/qtplugin/examples/")
        print("="*50)

def main() -> None:
    parser = argparse.ArgumentParser(description='QtPlugin Universal Installation Script')
    parser.add_argument('--source-dir', type=Path, default=Path.cwd(),
                       help='Source directory (default: current directory)')
    parser.add_argument('--prefix', type=Path,
                       help='Installation prefix (default: system-dependent)')
    parser.add_argument('--examples', action='store_true', default=True,
                       help='Install examples (default: enabled)')
    parser.add_argument('--network', action='store_true',
                       help='Build network support')
    parser.add_argument('--ui', action='store_true',
                       help='Build UI support')
    parser.add_argument('--skip-deps', action='store_true',
                       help='Skip dependency installation')
    
    args = parser.parse_args()
    
    # Validate source directory
    if not (args.source_dir / 'CMakeLists.txt').exists():
        print(f"❌ CMakeLists.txt not found in {args.source_dir}")
        print("Please run this script from the QtPlugin source directory")
        sys.exit(1)
    
    # Create installer
    installer = QtPluginInstaller(args.source_dir, args.prefix)
    
    # Installation options
    options = {
        'build_examples': args.examples,
        'build_network': args.network,
        'build_ui': args.ui,
        'skip_deps': args.skip_deps
    }
    
    # Run installation
    if installer.install(options):
        sys.exit(0)
    else:
        sys.exit(1)

if __name__ == '__main__':
    main()
