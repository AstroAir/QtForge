"""
QtForge Utils Module Type Stubs

Type definitions for the qtforge.utils submodule.
Contains utility functions for version handling, error creation, and Qt conversions.
"""

from typing import Any

def utils_test() -> str:
    """Test function for verifying utils module works."""
    ...

def create_version(major: int, minor: int, patch: int) -> str:
    """Create a version string from major, minor, patch numbers.
    
    Args:
        major: Major version number
        minor: Minor version number  
        patch: Patch version number
        
    Returns:
        Formatted version string (e.g., "1.2.3")
    """
    ...

def parse_version(version_string: str) -> str:
    """Parse a version string and return formatted result.
    
    Args:
        version_string: Version string to parse (e.g., "1.2.3-beta")
        
    Returns:
        Parsed and formatted version string
    """
    ...

def create_error(code: int, message: str) -> str:
    """Create an error message with code and description.
    
    Args:
        code: Error code number
        message: Error message description
        
    Returns:
        Formatted error string
    """
    ...

def register_qt_conversions() -> None:
    """Register Qt type conversions with pybind11.
    
    This function sets up type conversions between Qt types and Python types
    for use in the Python bindings.
    """
    ...
