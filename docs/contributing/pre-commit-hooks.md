# Pre-commit Hooks Guide

This document explains how to set up and use pre-commit hooks in the QtForge project to maintain code quality and consistency.

## Overview

Pre-commit hooks are automated checks that run before each commit to ensure code quality, formatting, and consistency. The QtForge project uses a comprehensive set of hooks for both C++ and Python code.

## Quick Start

### Prerequisites

- Python 3.8 or later
- Git repository (already set up for QtForge)
- CMake build system configured (for C++ linting)

### Installation

1. **Install pre-commit**:

   ```bash
   # Using pip
   pip install pre-commit

   # Or using the Python launcher on Windows
   py -m pip install pre-commit
   ```

2. **Install the hooks in your local repository**:

   ```bash
   # Navigate to the QtForge root directory
   cd /path/to/QtForge

   # Install pre-commit hooks
   pre-commit install

   # Also install commit-msg hooks (optional)
   pre-commit install --hook-type commit-msg
   ```

3. **Verify installation**:
   ```bash
   pre-commit --version
   ```

## Configured Hooks

The QtForge project includes the following pre-commit hooks:

### Basic File Checks

- **check-added-large-files**: Prevents committing files larger than 1MB
- **check-case-conflict**: Detects files that would conflict on case-insensitive filesystems
- **check-merge-conflict**: Finds merge conflict markers
- **check-yaml/json/toml/xml**: Validates syntax of configuration files
- **detect-private-key**: Scans for accidentally committed private keys
- **end-of-file-fixer**: Ensures files end with exactly one newline
- **trailing-whitespace**: Removes trailing whitespace
- **mixed-line-ending**: Standardizes line endings

### C++ Code Quality

- **clang-format**: Formats C++ code using the project's `.clang-format` configuration
- **clang-tidy**: Performs static analysis on C++ code (requires `compile_commands.json`)

### Python Code Quality

- **ruff**: Modern Python linter and formatter (replaces black, flake8, isort)
- **mypy**: Type checking using the project's `mypy.ini` configuration

### Additional Tools

- **prettier**: Formats JSON, YAML, Markdown, and other web-related files
- **cmake-format**: Formats CMake files
- **detect-secrets**: Scans for secrets and credentials

## Usage

### Automatic Execution

Once installed, pre-commit hooks run automatically on every `git commit`. If any hook fails:

1. The commit is blocked
2. Issues are reported
3. Some hooks automatically fix problems
4. You need to review changes and commit again

### Manual Execution

You can run hooks manually without committing:

```bash
# Run all hooks on staged files
pre-commit run

# Run all hooks on all files
pre-commit run --all-files

# Run a specific hook
pre-commit run clang-format
pre-commit run ruff

# Run hooks on specific files
pre-commit run --files src/core/plugin_manager.cpp
```

### Bypassing Hooks

In rare cases, you may need to bypass hooks:

```bash
# Bypass all pre-commit hooks
git commit --no-verify -m "Emergency fix"

# Bypass only specific hooks (not recommended)
SKIP=clang-tidy git commit -m "Skip clang-tidy for this commit"
```

**⚠️ Warning**: Only bypass hooks when absolutely necessary and ensure code quality through other means.

## Configuration Files

### `.pre-commit-config.yaml`

Main configuration file defining all hooks, their versions, and settings.

### `pyproject.toml`

Python project configuration including ruff and other Python tool settings.

### `.clang-format`

C++ code formatting rules (Google style with customizations).

### `.clang-tidy`

C++ static analysis configuration with project-specific rules.

### `mypy.ini`

Python type checking configuration.

### `.secrets.baseline`

Baseline file for the detect-secrets hook to avoid false positives.

## Troubleshooting

### Common Issues

#### 1. Hook Installation Fails

```bash
# Clear pre-commit cache and reinstall
pre-commit clean
pre-commit install --install-hooks
```

#### 2. clang-tidy Fails

- Ensure you have a `build/compile_commands.json` file
- Run CMake configuration: `cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`

#### 3. Network Issues During Installation

- Some hooks download tools during first use
- Ensure internet connectivity or use corporate proxy settings

#### 4. Python Import Errors

- Verify Python path and virtual environment
- Install missing dependencies: `pip install -r requirements.txt`

#### 5. File Encoding Issues

- Ensure files use UTF-8 encoding
- Check for BOM (Byte Order Mark) in files

### Hook-Specific Troubleshooting

#### ruff Issues

```bash
# Check ruff configuration
ruff check --show-settings

# Fix auto-fixable issues
ruff check --fix

# Format code
ruff format
```

#### clang-format Issues

```bash
# Check formatting without applying changes
clang-format --dry-run --Werror src/**/*.cpp

# Apply formatting
clang-format -i src/**/*.cpp
```

#### mypy Issues

```bash
# Run mypy manually
mypy src/python/

# Install missing type stubs
pip install types-PyYAML types-requests
```

## Updating Hooks

Keep hooks up to date for latest features and bug fixes:

```bash
# Update all hooks to latest versions
pre-commit autoupdate

# Update specific hook
pre-commit autoupdate --repo https://github.com/astral-sh/ruff-pre-commit
```

## Best Practices

1. **Run hooks locally**: Test changes before pushing to avoid CI failures
2. **Keep hooks updated**: Regular updates provide better performance and features
3. **Understand failures**: Don't just bypass hooks; understand and fix issues
4. **Consistent environment**: Use the same Python version across team members
5. **Document exceptions**: If you must bypass hooks, document why in the commit message

## Integration with IDEs

### Visual Studio Code

Install extensions for better integration:

- **Python**: Microsoft Python extension
- **C/C++**: Microsoft C/C++ extension
- **clang-format**: Formatting on save
- **Ruff**: Real-time linting

Configure settings.json:

```json
{
  "editor.formatOnSave": true,
  "python.linting.enabled": true,
  "python.linting.ruffEnabled": true,
  "C_Cpp.clang_format_style": "file"
}
```

### CLion/IntelliJ

- Enable clang-format integration
- Install Python plugin for Python files
- Configure code style to match project settings

## Contributing to Hook Configuration

When modifying hook configuration:

1. Test changes thoroughly with `pre-commit run --all-files`
2. Document any new requirements or dependencies
3. Update this guide if adding new hooks
4. Consider backward compatibility for existing contributors

## Support

If you encounter issues with pre-commit hooks:

1. Check this documentation first
2. Search existing GitHub issues
3. Run hooks with verbose output: `pre-commit run --verbose`
4. Create a new issue with detailed error messages and environment information

For more information, visit the [official pre-commit documentation](https://pre-commit.com/).
