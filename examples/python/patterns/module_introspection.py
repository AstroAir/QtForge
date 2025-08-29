#!/usr/bin/env python3
"""
Module Introspection Example

This module demonstrates comprehensive module introspection patterns
for QtForge Python bindings.
"""

import sys
import os
from typing import Dict, List, Any, Optional

# Add the build directory to Python path (adjust path as needed)
possible_paths = ['../../build', '../build', './build', 'build']
for path in possible_paths:
    if os.path.exists(os.path.join(path, 'qtforge.cp312-mingw_x86_64_msvcrt_gnu.pyd')) or \
       os.path.exists(os.path.join(path, 'qtforge.cp311-win_amd64.pyd')):
        sys.path.insert(0, path)
        break


class ModuleIntrospectionExample:
    """Example class demonstrating module introspection patterns."""

    def __init__(self) -> None:
        """Initialize the module introspection example."""
        import qtforge
        self.qtforge = qtforge
        print("🔍 Module Introspection Example initialized")

    def demonstrate_basic_introspection(self) -> None:
        """Demonstrate basic module introspection."""
        print("\n📋 Basic Module Introspection:")

        # Analyze main module
        print(f"   Main Module Analysis:")
        print(f"     - Name: {self.qtforge.__name__}")
        print(f"     - File: {self.qtforge.__file__}")
        print(f"     - Version: {self.qtforge.__version__}")
        doc = self.qtforge.__doc__ or "No documentation available"
        print(f"     - Documentation: {doc[:100]}...")

    def demonstrate_attribute_categorization(self) -> None:
        """Demonstrate attribute categorization."""
        print("\n🏷️  Attribute Categorization:")

        # Get all attributes and categorize them
        all_attrs = dir(self.qtforge)
        functions = []
        modules = []
        constants = []

        for attr in all_attrs:
            if not attr.startswith('_'):
                obj = getattr(self.qtforge, attr)
                if callable(obj):
                    functions.append(attr)
                elif hasattr(obj, '__file__'):  # Module
                    modules.append(attr)
                else:
                    constants.append(attr)

        print(f"     - Functions: {len(functions)} - {functions}")
        print(f"     - Modules: {len(modules)} - {modules}")
        print(f"     - Constants: {len(constants)} - {constants}")

    def demonstrate_submodule_analysis(self) -> None:
        """Demonstrate submodule analysis."""
        print("\n🔍 Submodule Analysis:")

        # Analyze submodules
        all_attrs = dir(self.qtforge)
        modules = []

        for attr in all_attrs:
            if not attr.startswith('_'):
                obj = getattr(self.qtforge, attr)
                if hasattr(obj, '__file__'):  # Module
                    modules.append(attr)

        for module_name in modules:
            module = getattr(self.qtforge, module_name)
            module_attrs = [attr for attr in dir(
                module) if not attr.startswith('_')]
            print(
                f"     - {module_name} module: {len(module_attrs)} attributes")

    def demonstrate_function_signature_analysis(self) -> None:
        """Demonstrate function signature analysis."""
        print("\n📝 Function Signature Analysis:")

        import inspect

        # Get all functions
        all_attrs = dir(self.qtforge)
        functions = []

        for attr in all_attrs:
            if not attr.startswith('_'):
                obj = getattr(self.qtforge, attr)
                if callable(obj):
                    functions.append((attr, obj))

        # Analyze function signatures
        for func_name, func_obj in functions[:5]:  # Limit to first 5 for brevity
            try:
                sig = inspect.signature(func_obj)
                print(f"     - {func_name}{sig}")
            except (ValueError, TypeError):
                print(f"     - {func_name}: <signature unavailable>")

    def demonstrate_type_analysis(self) -> None:
        """Demonstrate type analysis of module objects."""
        print("\n🔬 Type Analysis:")

        # Analyze types of various objects
        all_attrs = dir(self.qtforge)
        type_counts: Dict[str, int] = {}

        for attr in all_attrs:
            if not attr.startswith('_'):
                obj = getattr(self.qtforge, attr)
                obj_type = type(obj).__name__
                type_counts[obj_type] = type_counts.get(obj_type, 0) + 1

        print("     Object Type Distribution:")
        for obj_type, count in sorted(type_counts.items()):
            print(f"       - {obj_type}: {count}")

    def demonstrate_documentation_extraction(self) -> None:
        """Demonstrate documentation extraction."""
        print("\n📚 Documentation Extraction:")

        # Extract documentation from functions
        all_attrs = dir(self.qtforge)
        documented_functions = []

        for attr in all_attrs:
            if not attr.startswith('_'):
                obj = getattr(self.qtforge, attr)
                if callable(obj) and obj.__doc__:
                    documented_functions.append((attr, obj.__doc__))

        print(f"     Functions with documentation: {len(documented_functions)}")
        for func_name, doc in documented_functions[:3]:  # Show first 3
            doc_preview = doc.strip().split('\n')[0] if doc else "No documentation"
            print(f"       - {func_name}: {doc_preview[:60]}...")

    def run_introspection_examples(self) -> int:
        """Run all introspection examples.

        Returns:
            Exit code: 0 for success, 1 for failure
        """
        print("Module Introspection Examples")
        print("=" * 40)

        try:
            self.demonstrate_basic_introspection()
            self.demonstrate_attribute_categorization()
            self.demonstrate_submodule_analysis()
            self.demonstrate_function_signature_analysis()
            self.demonstrate_type_analysis()
            self.demonstrate_documentation_extraction()

            print(f"\n🎉 Module introspection examples completed successfully!")
            return 0

        except Exception as e:
            print(f"❌ Error during introspection examples: {e}")
            import traceback
            traceback.print_exc()
            return 1


def main() -> int:
    """Main function to run the introspection examples.

    Returns:
        Exit code: 0 for success, 1 for failure
    """
    try:
        example = ModuleIntrospectionExample()
        return example.run_introspection_examples()
    except ImportError as e:
        print(f"❌ Failed to import QtForge: {e}")
        print("Make sure QtForge Python bindings are built and in the Python path.")
        return 1


if __name__ == "__main__":
    exit_code = main()
    print(f"\nIntrospection examples completed with exit code: {exit_code}")
    sys.exit(exit_code)
