#!/usr/bin/env python3
"""
QtForge Packaging Optimization Script

Optimizes packaging process for different platforms and formats.
Includes package signing, verification, and distribution preparation.
"""

import os
import sys
import subprocess
import platform
import hashlib
import json
import shutil
from pathlib import Path
from typing import Dict, List, Optional, Any
from datetime import datetime

class PackagingOptimizer:
    """Optimizes packaging process for QtForge"""

    def __init__(self, build_dir: Path, output_dir: Path) -> None:
        self.build_dir = Path(build_dir)
        self.output_dir = Path(output_dir)
        self.system = platform.system().lower()
        self.arch = platform.machine().lower()

        # Create output directory
        self.output_dir.mkdir(parents=True, exist_ok=True)

        # Package metadata
        self.metadata = {
            'timestamp': datetime.utcnow().isoformat(),
            'system': self.system,
            'architecture': self.arch,
            'packages': {},
            'checksums': {},
            'signatures': {}
        }

    def optimize_binaries(self) -> None:
        """Optimize binaries for distribution"""
        print("🔧 Optimizing binaries...")

        if self.system == 'linux':
            self._strip_linux_binaries()
        elif self.system == 'windows':
            self._optimize_windows_binaries()
        elif self.system == 'darwin':
            self._optimize_macos_binaries()


    def deploy_examples(self) -> None:
        """Optionally deploy Qt runtime for example apps before packaging"""
        examples_root = self.build_dir / 'examples'
        if not examples_root.exists():
            print("ℹ️ No examples directory found for deployment")
            return

        apps: List[Path] = []
        if self.system == 'windows':
            # Look for .exe files under examples
            apps = list(examples_root.rglob('*.exe'))
            deployer = shutil.which('windeployqt')
            if not deployer:
                print("⚠️ windeployqt not found in PATH; skipping Qt runtime deployment for Windows")
                return
            for app in apps:
                print(f"🚚 Running windeployqt for {app}")
                subprocess.run([deployer, str(app)], check=False)
        elif self.system == 'darwin':
            # macOS app bundles (.app)
            apps = list(examples_root.rglob('*.app'))
            deployer = shutil.which('macdeployqt')
            if not deployer:
                print("⚠️ macdeployqt not found; skipping Qt runtime deployment for macOS")
                return
            for app in apps:
                print(f"🚚 Running macdeployqt for {app}")
                subprocess.run([deployer, str(app), '-always-overwrite'], check=False)
        elif self.system == 'linux':
            # Look for ELF executables (no extension) under examples
            potential = [p for p in examples_root.rglob('*') if p.is_file() and os.access(p, os.X_OK)]
            apps = [p for p in potential if p.suffix == '' and 'CMakeFiles' not in str(p)]
            deployer = shutil.which('linuxdeployqt')
            if not deployer:
                print("ℹ️ linuxdeployqt not found; you may install it to bundle Qt runtime on Linux AppImage/dir")
                return
            for app in apps:
                print(f"🚚 Running linuxdeployqt for {app}")
                subprocess.run([deployer, str(app), '-bundle-non-qt-libs'], check=False)

    def _strip_linux_binaries(self) -> None:
        """Strip debug symbols from Linux binaries"""
        for binary in self.build_dir.rglob('*'):
            if binary.is_file() and binary.suffix in ['.so', '']:
                try:
                    # Check if it's an ELF binary
                    result = subprocess.run(['file', str(binary)],
                                          capture_output=True, text=True)
                    if 'ELF' in result.stdout and 'not stripped' in result.stdout:
                        print(f"  Stripping {binary.name}")
                        subprocess.run(['strip', str(binary)], check=True)
                except subprocess.CalledProcessError:
                    print(f"  Warning: Could not strip {binary.name}")

    def _optimize_windows_binaries(self) -> None:
        """Optimize Windows binaries"""
        # Remove debug files in release builds
        for pdb_file in self.build_dir.rglob('*.pdb'):
            if 'release' in str(pdb_file).lower():
                print(f"  Removing debug file: {pdb_file.name}")
                pdb_file.unlink()

    def _optimize_macos_binaries(self) -> None:
        """Optimize macOS binaries"""
        for binary in self.build_dir.rglob('*'):
            if binary.is_file() and not binary.suffix:
                try:
                    # Check if it's a Mach-O binary
                    result = subprocess.run(['file', str(binary)],
                                          capture_output=True, text=True)
                    if 'Mach-O' in result.stdout:
                        print(f"  Stripping {binary.name}")
                        subprocess.run(['strip', str(binary)], check=True)
                except subprocess.CalledProcessError:
                    print(f"  Warning: Could not strip {binary.name}")

    def create_packages(self, package_types: List[str]) -> Dict[str, Path]:
        """Create optimized packages"""
        print(f"📦 Creating packages: {', '.join(package_types)}")

        packages = {}

        for package_type in package_types:
            try:
                if package_type.upper() == 'DEB':
                    package_path = self._create_deb_package()
                elif package_type.upper() == 'RPM':
                    package_path = self._create_rpm_package()
                elif package_type.upper() == 'NSIS':
                    package_path = self._create_nsis_package()
                elif package_type.upper() == 'WIX':
                    package_path = self._create_wix_package()
                elif package_type.upper() == 'DRAGNDROP':
                    package_path = self._create_dmg_package()
                elif package_type.upper() in ['TGZ', 'TAR.GZ']:
                    package_path = self._create_tarball()
                elif package_type.upper() == 'ZIP':
                    package_path = self._create_zip_package()
                else:
                    print(f"  Warning: Unknown package type {package_type}")
                    continue

                if package_path and package_path.exists():
                    packages[package_type] = package_path
                    self.metadata['packages'][package_type] = {
                        'path': str(package_path),
                        'size': package_path.stat().st_size
                    }
                    print(f"  ✅ Created {package_type}: {package_path.name}")

            except Exception as e:
                print(f"  ❌ Failed to create {package_type}: {e}")

        return packages

    def _create_deb_package(self) -> Optional[Path]:
        """Create DEB package using CPack"""
        cmd = ['cpack', '-G', 'DEB', '-B', str(self.output_dir)]
        result = subprocess.run(cmd, cwd=self.build_dir, capture_output=True)

        if result.returncode == 0:
            # Find the created DEB file
            for deb_file in self.output_dir.glob('*.deb'):
                return deb_file
        return None

    def _create_rpm_package(self) -> Optional[Path]:
        """Create RPM package using CPack"""
        cmd = ['cpack', '-G', 'RPM', '-B', str(self.output_dir)]
        result = subprocess.run(cmd, cwd=self.build_dir, capture_output=True)

        if result.returncode == 0:
            for rpm_file in self.output_dir.glob('*.rpm'):
                return rpm_file
        return None

    def _create_nsis_package(self) -> Optional[Path]:
        """Create NSIS installer"""
        cmd = ['cpack', '-G', 'NSIS', '-B', str(self.output_dir)]
        result = subprocess.run(cmd, cwd=self.build_dir, capture_output=True)

        if result.returncode == 0:
            for exe_file in self.output_dir.glob('*.exe'):
                return exe_file
        return None

    def _create_wix_package(self) -> Optional[Path]:
        """Create WiX MSI package"""
        cmd = ['cpack', '-G', 'WIX', '-B', str(self.output_dir)]
        result = subprocess.run(cmd, cwd=self.build_dir, capture_output=True)

        if result.returncode == 0:
            for msi_file in self.output_dir.glob('*.msi'):
                return msi_file
        return None

    def _create_dmg_package(self) -> Optional[Path]:
        """Create macOS DMG package"""
        cmd = ['cpack', '-G', 'DragNDrop', '-B', str(self.output_dir)]
        result = subprocess.run(cmd, cwd=self.build_dir, capture_output=True)

        if result.returncode == 0:
            for dmg_file in self.output_dir.glob('*.dmg'):
                return dmg_file
        return None

    def _create_tarball(self) -> Optional[Path]:
        """Create compressed tarball"""
        cmd = ['cpack', '-G', 'TGZ', '-B', str(self.output_dir)]
        result = subprocess.run(cmd, cwd=self.build_dir, capture_output=True)

        if result.returncode == 0:
            for tar_file in self.output_dir.glob('*.tar.gz'):
                return tar_file
        return None

    def _create_zip_package(self) -> Optional[Path]:
        """Create ZIP package"""
        cmd = ['cpack', '-G', 'ZIP', '-B', str(self.output_dir)]
        result = subprocess.run(cmd, cwd=self.build_dir, capture_output=True)

        if result.returncode == 0:
            for zip_file in self.output_dir.glob('*.zip'):
                return zip_file
        return None

    def generate_checksums(self, packages: Dict[str, Path]) -> None:
        """Generate checksums for all packages"""
        print("🔐 Generating checksums...")

        checksums_file = self.output_dir / 'checksums.txt'
        sha256_file = self.output_dir / 'SHA256SUMS'

        with open(checksums_file, 'w') as f, open(sha256_file, 'w') as sha_f:
            for package_type, package_path in packages.items():
                # Calculate SHA256
                sha256_hash = self._calculate_sha256(package_path)
                # Calculate MD5
                md5_hash = self._calculate_md5(package_path)

                self.metadata['checksums'][package_type] = {
                    'sha256': sha256_hash,
                    'md5': md5_hash
                }

                # Write to files
                f.write(f"{package_path.name}:\n")
                f.write(f"  SHA256: {sha256_hash}\n")
                f.write(f"  MD5:    {md5_hash}\n\n")

                sha_f.write(f"{sha256_hash}  {package_path.name}\n")

        print(f"  ✅ Checksums saved to {checksums_file.name}")

    def _calculate_sha256(self, file_path: Path) -> str:
        """Calculate SHA256 hash of a file"""
        sha256_hash = hashlib.sha256()
        with open(file_path, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                sha256_hash.update(chunk)
        return sha256_hash.hexdigest()

    def _calculate_md5(self, file_path: Path) -> str:
        """Calculate MD5 hash of a file"""
        md5_hash = hashlib.md5()
        with open(file_path, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                md5_hash.update(chunk)
        return md5_hash.hexdigest()

    def save_metadata(self) -> None:
        """Save packaging metadata"""
        metadata_file = self.output_dir / 'package-metadata.json'
        with open(metadata_file, 'w') as f:
            json.dump(self.metadata, f, indent=2)
        print(f"📋 Metadata saved to {metadata_file.name}")

def main() -> None:
    """Main function"""
    import argparse

    parser = argparse.ArgumentParser(description='Optimize QtForge packaging')
    parser.add_argument('--build-dir', type=Path, default=Path('build'),
                       help='Build directory')
    parser.add_argument('--output-dir', type=Path, default=Path('packages'),
                       help='Output directory for packages')
    parser.add_argument('--package-types', nargs='+',
                       default=['TGZ', 'ZIP'],
                       help='Package types to create')
    parser.add_argument('--optimize-binaries', action='store_true',
                       help='Optimize binaries before packaging')
    parser.add_argument('--deploy-examples', action='store_true',
                       help='Run windeployqt/macdeployqt/linuxdeployqt on built examples before packaging')

    args = parser.parse_args()

    optimizer = PackagingOptimizer(args.build_dir, args.output_dir)

    if args.optimize_binaries:
        optimizer.optimize_binaries()


    if args.deploy_examples:
        optimizer.deploy_examples()

    packages = optimizer.create_packages(args.package_types)

    if packages:
        optimizer.generate_checksums(packages)
        optimizer.save_metadata()

        print(f"\n✅ Packaging complete! Created {len(packages)} packages in {args.output_dir}")
    else:
        print("\n❌ No packages were created successfully")
        sys.exit(1)

if __name__ == '__main__':
    main()
