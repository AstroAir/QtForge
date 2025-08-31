#!/usr/bin/env python3
"""
Error Handling Example

This example demonstrates comprehensive error handling patterns using
QtForge Python bindings, including error creation, exception handling,
and error recovery strategies.
"""

import sys
import os
from typing import Optional, Tuple, List

# Add the build directory to Python path (adjust path as needed)
# Try different possible paths for the build directory
possible_paths = ['../../build', '../build', './build', 'build']
for path in possible_paths:
    if os.path.exists(os.path.join(path, 'qtforge.cp312-mingw_x86_64_msvcrt_gnu.pyd')) or \
       os.path.exists(os.path.join(path, 'qtforge.cp311-win_amd64.pyd')):
        sys.path.insert(0, path)
        break


class ErrorHandlingExample:
    """Example class demonstrating error handling with QtForge."""

    def __init__(self) -> None:
        """Initialize the error handling example."""
        import qtforge
        self.qtforge = qtforge
        print("⚠️  Error Handling Example initialized")

    def demonstrate_error_creation(self) -> list[tuple[int, str]]:
        """Demonstrate creating different types of errors.

        Returns:
            List of tuples containing error codes and messages
        """
        print("\n🔨 Error Creation:")

        # Common error scenarios in plugin systems
        error_scenarios = [
            # Loading errors
            (100, "Plugin file not found"),
            (101, "Invalid plugin format"),
            (102, "Plugin load failed"),
            (103, "Symbol not found in plugin"),

            # Initialization errors
            (200, "Plugin initialization failed"),
            (201, "Configuration error"),
            (202, "Required dependency missing"),
            (203, "Version mismatch"),

            # Runtime errors
            (300, "Plugin execution failed"),
            (301, "Command not found"),
            (302, "Invalid parameters"),
            (303, "Plugin state error"),

            # Security errors
            (400, "Security violation"),
            (401, "Permission denied"),
            (402, "Invalid signature"),

            # System errors
            (500, "Out of memory"),
            (501, "Resource exhausted"),
            (502, "Network error")
        ]

        created_errors = []
        for code, message in error_scenarios:
            try:
                error_msg = self.qtforge.create_error(code, message)
                created_errors.append((code, error_msg))
                print(f"   Error {code}: {error_msg}")
            except Exception as e:
                print(f"   Failed to create error {code}: {e}")

        return created_errors

    def demonstrate_exception_handling(self) -> tuple[int, int]:
        """Demonstrate exception handling patterns.

        Returns:
            Tuple of (successful_operations, failed_operations)
        """
        print("\n🛡️  Exception Handling Patterns:")

        # Simulate various operations that might fail
        operations = [
            ("load_plugin", "nonexistent_plugin.so"),
            ("load_plugin", "corrupted_plugin.dll"),
            ("load_plugin", "valid_plugin.so"),
            ("create_version", "invalid.version.string"),
            ("parse_version", "1.2.3.4.5.6")
        ]

        successful_operations = 0
        failed_operations = 0

        for operation, parameter in operations:
            try:
                print(f"   Attempting {operation} with '{parameter}':")

                if operation == "load_plugin":
                    result = self.qtforge.load_plugin_demo(parameter)
                    print(f"     ✅ Success: {result}")
                    successful_operations += 1

                elif operation == "create_version":
                    # This would fail in a real implementation with invalid input
                    result = self.qtforge.parse_version(parameter)
                    print(f"     ✅ Success: {result}")
                    successful_operations += 1

                elif operation == "parse_version":
                    result = self.qtforge.parse_version(parameter)
                    print(f"     ✅ Success: {result}")
                    successful_operations += 1

            except ValueError as e:
                print(f"     ❌ ValueError: {e}")
                failed_operations += 1
            except RuntimeError as e:
                print(f"     ❌ RuntimeError: {e}")
                failed_operations += 1
            except Exception as e:
                print(f"     ❌ Unexpected error: {e}")
                failed_operations += 1

        print(
            f"\n   Summary: {successful_operations} successful, {failed_operations} failed")
        return successful_operations, failed_operations

    def demonstrate_error_recovery(self) -> bool:
        """Demonstrate error recovery strategies.

        Returns:
            True if recovery was successful, False otherwise
        """
        print("\n🔄 Error Recovery Strategies:")

        # Simulate plugin loading with fallbacks
        plugin_candidates = [
            "plugins/primary_plugin.so",
            "plugins/backup_plugin.so",
            "plugins/fallback_plugin.so",
            "plugins/minimal_plugin.so"
        ]

        loaded_plugin = None
        attempts = 0

        for plugin_path in plugin_candidates:
            attempts += 1
            try:
                print(f"   Attempt {attempts}: Loading {plugin_path}")
                result = self.qtforge.load_plugin_demo(plugin_path)
                print(f"     ✅ Success: {result}")
                loaded_plugin = plugin_path
                break

            except Exception as e:
                print(f"     ❌ Failed: {e}")
                error_msg = self.qtforge.create_error(
                    102, f"Failed to load {plugin_path}")
                print(f"     Error logged: {error_msg}")

                # Continue to next candidate
                continue

        if loaded_plugin:
            print(f"   🎉 Successfully loaded plugin: {loaded_plugin}")
        else:
            print(f"   💥 All plugin loading attempts failed")

        return loaded_plugin is not None

    def demonstrate_error_categorization(self) -> None:
        """Demonstrate error categorization and handling."""
        print("\n📂 Error Categorization:")

        # Categorize errors by type
        error_categories = {
            "Critical": [
                (500, "Out of memory"),
                (501, "System failure"),
                (502, "Hardware error")
            ],
            "Recoverable": [
                (300, "Plugin execution failed"),
                (301, "Command timeout"),
                (302, "Temporary resource unavailable")
            ],
            "User": [
                (400, "Invalid user input"),
                (401, "Permission denied"),
                (402, "Authentication failed")
            ],
            "Configuration": [
                (200, "Missing configuration"),
                (201, "Invalid configuration"),
                (202, "Configuration conflict")
            ]
        }

        for category, errors in error_categories.items():
            print(f"\n   {category} Errors:")
            for code, message in errors:
                error_msg = self.qtforge.create_error(code, message)
                print(f"     - {error_msg}")

                # Demonstrate different handling strategies
                if category == "Critical":
                    print(f"       → Strategy: Immediate shutdown")
                elif category == "Recoverable":
                    print(f"       → Strategy: Retry with backoff")
                elif category == "User":
                    print(f"       → Strategy: Show user-friendly message")
                elif category == "Configuration":
                    print(f"       → Strategy: Use default configuration")

    def demonstrate_error_logging(self) -> tuple[int, int, int]:
        """Demonstrate error logging and reporting.

        Returns:
            Tuple of (critical_count, error_count, warning_count)
        """
        print("\n📝 Error Logging and Reporting:")

        # Simulate error logging scenarios
        error_events = [
            ("2024-01-15 10:30:15", "WARN", 201,
             "Plugin configuration deprecated"),
            ("2024-01-15 10:30:16", "ERROR", 300, "Plugin execution timeout"),
            ("2024-01-15 10:30:17", "CRITICAL", 500, "Memory allocation failed"),
            ("2024-01-15 10:30:18", "INFO", 0, "Plugin loaded successfully"),
            ("2024-01-15 10:30:19", "ERROR", 404, "Plugin dependency not found")
        ]

        error_count = 0
        warning_count = 0
        critical_count = 0

        for timestamp, level, code, message in error_events:
            if level in ["ERROR", "CRITICAL"]:
                error_msg = self.qtforge.create_error(code, message)
                print(f"   [{timestamp}] {level}: {error_msg}")

                if level == "ERROR":
                    error_count += 1
                elif level == "CRITICAL":
                    critical_count += 1
            elif level == "WARN":
                warning_count += 1
                error_msg = self.qtforge.create_error(code, message)
                print(f"   [{timestamp}] {level}: {error_msg}")
            else:
                print(f"   [{timestamp}] {level}: {message}")

        print(f"\n   Error Summary:")
        print(f"     - Critical errors: {critical_count}")
        print(f"     - Errors: {error_count}")
        print(f"     - Warnings: {warning_count}")

        return critical_count, error_count, warning_count

    def run_complete_example(self) -> int:
        """Run the complete error handling example.

        Returns:
            Exit code: 0 for success, 1 for failure
        """
        print("QtForge Error Handling Example")
        print("=" * 50)

        try:
            # Demonstrate all error handling aspects
            created_errors = self.demonstrate_error_creation()
            success_count, fail_count = self.demonstrate_exception_handling()
            recovery_success = self.demonstrate_error_recovery()
            self.demonstrate_error_categorization()
            critical, errors, warnings = self.demonstrate_error_logging()

            print(f"\n🎉 Error handling example completed successfully!")
            print(f"   Errors created: {len(created_errors)}")
            print(
                f"   Operations: {success_count} successful, {fail_count} failed")
            print(
                f"   Recovery: {'Success' if recovery_success else 'Failed'}")
            print(
                f"   Log summary: {critical} critical, {errors} errors, {warnings} warnings")

            return 0

        except Exception as e:
            print(f"❌ Error during error handling example: {e}")
            import traceback
            traceback.print_exc()
            return 1


def main() -> int:
    """Main function to run the error handling example.

    Returns:
        Exit code: 0 for success, 1 for failure
    """
    try:
        example = ErrorHandlingExample()
        return example.run_complete_example()
    except ImportError as e:
        print(f"❌ Failed to import QtForge: {e}")
        print("Make sure QtForge Python bindings are built and in the Python path.")
        return 1


if __name__ == "__main__":
    exit_code = main()
    print(f"\nExample completed with exit code: {exit_code}")
    sys.exit(exit_code)
