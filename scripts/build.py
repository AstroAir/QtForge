#!/usr/bin/env python3
"""
QtPlugin Cross-Platform Build Script
Automates building and packaging for Windows, macOS, and Linux
"""

import os
import sys
import subprocess
import platform
import argparse
import shutil
from pathlib import Path
from typing import List, Dict, Optional

class BuildConfig:
    """Build configuration management"""
    
    def __init__(self):
        self.system = platform.system().lower()
        self.machine = platform.machine().lower()
        self.is_windows = self.system == 'windows'
        self.is_macos = self.system == 'darwin'
        self.is_linux = self.system == 'linux'
        
        # Detect architecture
        if self.machine in ['x86_64', 'amd64']:
            self.arch = 'x64'
        elif self.machine in ['aarch64', 'arm64']:
            self.arch = 'arm64'
        elif self.machine.startswith('arm'):
            self.arch = 'arm'
        else:
            self.arch = 'x86'
    
    def get_cmake_generator(self) -> str:
        """Get appropriate CMake generator for platform"""
        if self.is_windows:
            # Try to detect Visual Studio version
            vs_versions = ['2022', '2019', '2017']
            for vs in vs_versions:
                try:
                    result = subprocess.run(['where', f'devenv'], 
                                          capture_output=True, text=True)
                    if result.returncode == 0:
                        return f'Visual Studio 16 {vs}'
                except:
                    continue
            return 'Ninja'  # Fallback to Ninja
        elif self.is_macos:
            return 'Xcode'
        else:
            return 'Ninja'
    
    def get_package_formats(self) -> List[str]:
        """Get supported package formats for platform"""
        if self.is_windows:
            return ['NSIS', 'ZIP', 'WIX']
        elif self.is_macos:
            return ['DragNDrop', 'TGZ', 'productbuild']
        else:
            return ['DEB', 'RPM', 'TGZ', 'STGZ']

class QtPluginBuilder:
    """Main builder class"""
    
    def __init__(self, source_dir: Path, build_dir: Path, install_dir: Path):
        self.source_dir = source_dir
        self.build_dir = build_dir
        self.install_dir = install_dir
        self.config = BuildConfig()
        
        # Create directories
        self.build_dir.mkdir(parents=True, exist_ok=True)
        self.install_dir.mkdir(parents=True, exist_ok=True)
    
    def detect_qt(self) -> Optional[str]:
        """Detect Qt installation"""
        qt_paths = []
        
        if self.config.is_windows:
            qt_paths = [
                'C:/Qt/6.5.0/msvc2022_64',
                'C:/Qt/6.4.0/msvc2022_64',
                'C:/Qt/6.6.0/msvc2022_64'
            ]
        elif self.config.is_macos:
            qt_paths = [
                '/usr/local/Qt-6.5.0',
                '/opt/homebrew/opt/qt@6',
                '/usr/local/opt/qt@6'
            ]
        else:
            qt_paths = [
                '/usr/lib/qt6',
                '/usr/local/Qt-6.5.0',
                '/opt/qt6'
            ]
        
        # Check environment variable first
        qt_dir = os.environ.get('Qt6_DIR') or os.environ.get('QT_DIR')
        if qt_dir and Path(qt_dir).exists():
            return qt_dir
        
        # Check common paths
        for path in qt_paths:
            if Path(path).exists():
                return path
        
        return None
    
    def configure(self, options: Dict[str, any]) -> bool:
        """Configure the build with CMake"""
        print(f"🔧 Configuring build for {self.config.system} ({self.config.arch})")
        
        cmake_args = [
            'cmake',
            '-S', str(self.source_dir),
            '-B', str(self.build_dir),
            f'-DCMAKE_BUILD_TYPE={options.get("build_type", "Release")}',
            f'-DCMAKE_INSTALL_PREFIX={self.install_dir}',
        ]
        
        # Add generator
        generator = self.config.get_cmake_generator()
        if generator != 'Ninja' or shutil.which('ninja'):
            cmake_args.extend(['-G', generator])
        
        # Qt detection
        qt_dir = self.detect_qt()
        if qt_dir:
            cmake_args.append(f'-DQt6_DIR={qt_dir}/lib/cmake/Qt6')
            print(f"📦 Found Qt at: {qt_dir}")
        else:
            print("⚠️  Qt not found in standard locations, relying on system PATH")
        
        # Build options
        if options.get('build_tests', False):
            cmake_args.append('-DQTPLUGIN_BUILD_TESTS=ON')
        
        if options.get('build_examples', True):
            cmake_args.append('-DQTPLUGIN_BUILD_EXAMPLES=ON')
        
        if options.get('build_network', False):
            cmake_args.append('-DQTPLUGIN_BUILD_NETWORK=ON')
        
        if options.get('build_ui', False):
            cmake_args.append('-DQTPLUGIN_BUILD_UI=ON')
        
        # Platform-specific options
        if self.config.is_windows:
            cmake_args.extend([
                '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL',
                '-DCPACK_GENERATOR=NSIS;ZIP;WIX'
            ])
        elif self.config.is_macos:
            cmake_args.extend([
                '-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15',
                '-DCPACK_GENERATOR=DragNDrop;TGZ'
            ])
        else:
            cmake_args.extend([
                '-DCPACK_GENERATOR=DEB;RPM;TGZ'
            ])
        
        try:
            result = subprocess.run(cmake_args, cwd=self.build_dir, check=True)
            print("✅ Configuration successful")
            return True
        except subprocess.CalledProcessError as e:
            print(f"❌ Configuration failed: {e}")
            return False
    
    def build(self, parallel_jobs: int = 0) -> bool:
        """Build the project"""
        print("🔨 Building project...")
        
        if parallel_jobs == 0:
            parallel_jobs = os.cpu_count() or 4
        
        cmake_args = [
            'cmake',
            '--build', str(self.build_dir),
            '--config', 'Release',
            '--parallel', str(parallel_jobs)
        ]
        
        try:
            subprocess.run(cmake_args, check=True)
            print("✅ Build successful")
            return True
        except subprocess.CalledProcessError as e:
            print(f"❌ Build failed: {e}")
            return False
    
    def test(self) -> bool:
        """Run tests"""
        print("🧪 Running tests...")
        
        try:
            subprocess.run([
                'ctest',
                '--test-dir', str(self.build_dir),
                '--output-on-failure',
                '--parallel', str(os.cpu_count() or 4)
            ], check=True)
            print("✅ All tests passed")
            return True
        except subprocess.CalledProcessError as e:
            print(f"❌ Tests failed: {e}")
            return False
    
    def package(self) -> bool:
        """Create packages"""
        print("📦 Creating packages...")
        
        try:
            subprocess.run([
                'cpack',
                '--config', str(self.build_dir / 'CPackConfig.cmake')
            ], cwd=self.build_dir, check=True)
            print("✅ Packaging successful")
            return True
        except subprocess.CalledProcessError as e:
            print(f"❌ Packaging failed: {e}")
            return False
    
    def install(self) -> bool:
        """Install the project"""
        print("📥 Installing project...")
        
        try:
            subprocess.run([
                'cmake',
                '--install', str(self.build_dir),
                '--config', 'Release'
            ], check=True)
            print("✅ Installation successful")
            return True
        except subprocess.CalledProcessError as e:
            print(f"❌ Installation failed: {e}")
            return False

def main():
    parser = argparse.ArgumentParser(description='QtPlugin Cross-Platform Build Script')
    parser.add_argument('--source-dir', type=Path, default=Path.cwd(),
                       help='Source directory (default: current directory)')
    parser.add_argument('--build-dir', type=Path, default=Path.cwd() / 'build',
                       help='Build directory (default: ./build)')
    parser.add_argument('--install-dir', type=Path, default=Path.cwd() / 'install',
                       help='Install directory (default: ./install)')
    parser.add_argument('--build-type', choices=['Debug', 'Release', 'RelWithDebInfo'],
                       default='Release', help='Build type')
    parser.add_argument('--jobs', '-j', type=int, default=0,
                       help='Number of parallel jobs (default: auto-detect)')
    parser.add_argument('--tests', action='store_true',
                       help='Build and run tests')
    parser.add_argument('--examples', action='store_true', default=True,
                       help='Build examples (default: enabled)')
    parser.add_argument('--network', action='store_true',
                       help='Build network support')
    parser.add_argument('--ui', action='store_true',
                       help='Build UI support')
    parser.add_argument('--package', action='store_true',
                       help='Create packages after build')
    parser.add_argument('--install', action='store_true',
                       help='Install after build')
    parser.add_argument('--clean', action='store_true',
                       help='Clean build directory before building')
    
    args = parser.parse_args()
    
    # Clean build directory if requested
    if args.clean and args.build_dir.exists():
        print(f"🧹 Cleaning build directory: {args.build_dir}")
        shutil.rmtree(args.build_dir)
    
    # Create builder
    builder = QtPluginBuilder(args.source_dir, args.build_dir, args.install_dir)
    
    # Build options
    options = {
        'build_type': args.build_type,
        'build_tests': args.tests,
        'build_examples': args.examples,
        'build_network': args.network,
        'build_ui': args.ui
    }
    
    print(f"🚀 Starting build for QtPlugin on {builder.config.system} ({builder.config.arch})")
    
    # Configure
    if not builder.configure(options):
        sys.exit(1)
    
    # Build
    if not builder.build(args.jobs):
        sys.exit(1)
    
    # Test
    if args.tests:
        if not builder.test():
            sys.exit(1)
    
    # Install
    if args.install:
        if not builder.install():
            sys.exit(1)
    
    # Package
    if args.package:
        if not builder.package():
            sys.exit(1)
    
    print("🎉 Build completed successfully!")

if __name__ == '__main__':
    main()
