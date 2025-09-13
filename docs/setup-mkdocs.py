#!/usr/bin/env python3
"""
MkDocs Setup Script for QtPlugin Documentation

This script sets up MkDocs for the QtPlugin project documentation.
It installs required dependencies and validates the setup.
"""

import subprocess
import sys
import os
from pathlib import Path

def run_command(cmd, check=True) -> None:
    """Run a command and return the result."""
    print(f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, check=check, capture_output=True, text=True)
        if result.stdout:
            print(result.stdout)
        return result
    except subprocess.CalledProcessError as e:
        print(f"Error running command: {e}")
        if e.stderr:
            print(f"Error output: {e.stderr}")
        if check:
            sys.exit(1)
        return e

def check_python_version() -> None:
    """Check if Python version is compatible."""
    if sys.version_info < (3, 8):
        print("Error: Python 3.8 or higher is required")
        sys.exit(1)
    print(f"✅ Python version: {sys.version}")

def install_mkdocs() -> None:
    """Install MkDocs and required plugins."""
    print("\n📦 Installing MkDocs and plugins...")

    # Install from requirements file
    requirements_file = Path(__file__).parent / "requirements.txt"
    if requirements_file.exists():
        run_command([sys.executable, "-m", "pip", "install", "-r", str(requirements_file)])
    else:
        # Fallback to manual installation
        packages = [
            "mkdocs>=1.5.0",
            "mkdocs-material>=9.4.0",
            "mkdocs-minify-plugin>=0.7.0",
            "mkdocs-git-revision-date-localized-plugin>=1.2.0",
            "mkdocs-git-committers-plugin-2>=1.2.0",
            "mkdocs-tags-plugin>=0.3.0",
            "pygments>=2.16.0",
            "pymdown-extensions>=10.3.0"
        ]

        for package in packages:
            run_command([sys.executable, "-m", "pip", "install", package])

def validate_mkdocs_config() -> None:
    """Validate the MkDocs configuration."""
    print("\n🔍 Validating MkDocs configuration...")

    config_file = Path(__file__).parent.parent / "mkdocs.yml"
    if not config_file.exists():
        print("❌ mkdocs.yml not found")
        return False

    # Change to project root directory
    os.chdir(config_file.parent)

    # Validate configuration
    result = run_command(["mkdocs", "config"], check=False)
    if result.returncode == 0:
        print("✅ MkDocs configuration is valid")
        return True
    else:
        print("❌ MkDocs configuration has errors")
        return False

def build_docs() -> None:
    """Build the documentation."""
    print("\n🏗️ Building documentation...")

    result = run_command(["mkdocs", "build", "--clean"], check=False)
    if result.returncode == 0:
        print("✅ Documentation built successfully")
        return True
    else:
        print("❌ Documentation build failed")
        return False

def serve_docs() -> None:
    """Start the development server."""
    print("\n🚀 Starting development server...")
    print("📖 Documentation will be available at: http://127.0.0.1:8000")
    print("Press Ctrl+C to stop the server")

    try:
        subprocess.run(["mkdocs", "serve"], check=True)
    except KeyboardInterrupt:
        print("\n👋 Server stopped")
    except subprocess.CalledProcessError as e:
        print(f"❌ Failed to start server: {e}")

def check_documentation_structure() -> None:
    """Check if required documentation files exist."""
    print("\n📁 Checking documentation structure...")

    docs_dir = Path(__file__).parent
    required_files = [
        "index.md",
        "getting-started/overview.md",
        "getting-started/installation.md",
        "getting-started/quick-start.md",
        "getting-started/first-plugin.md",
        "api/index.md",
        "examples/index.md",
        "contributing/index.md",
        "appendix/faq.md",
        "user-guide/troubleshooting.md"
    ]

    missing_files = []
    for file_path in required_files:
        full_path = docs_dir / file_path
        if full_path.exists():
            print(f"✅ {file_path}")
        else:
            print(f"❌ {file_path} (missing)")
            missing_files.append(file_path)

    if missing_files:
        print(f"\n⚠️  {len(missing_files)} files are missing")
        return False
    else:
        print("\n✅ All required documentation files are present")
        return True

def main() -> None:
    """Main setup function."""
    print("🔧 QtPlugin MkDocs Setup")
    print("=" * 50)

    # Check Python version
    check_python_version()

    # Install MkDocs
    install_mkdocs()

    # Check documentation structure
    structure_ok = check_documentation_structure()

    # Validate configuration
    config_ok = validate_mkdocs_config()

    # Build documentation
    build_ok = build_docs()

    # Summary
    print("\n📊 Setup Summary")
    print("=" * 30)
    print(f"Documentation structure: {'✅ OK' if structure_ok else '❌ Issues'}")
    print(f"MkDocs configuration: {'✅ OK' if config_ok else '❌ Issues'}")
    print(f"Documentation build: {'✅ OK' if build_ok else '❌ Failed'}")

    if structure_ok and config_ok and build_ok:
        print("\n🎉 Setup completed successfully!")
        print("\nNext steps:")
        print("1. Run 'mkdocs serve' to start the development server")
        print("2. Open http://127.0.0.1:8000 in your browser")
        print("3. Edit documentation files and see live updates")

        # Ask if user wants to start the server
        try:
            response = input("\nStart development server now? (y/N): ").strip().lower()
            if response in ['y', 'yes']:
                serve_docs()
        except KeyboardInterrupt:
            print("\n👋 Goodbye!")
    else:
        print("\n❌ Setup completed with issues")
        print("Please fix the issues above and run the script again")
        sys.exit(1)

if __name__ == "__main__":
    main()
