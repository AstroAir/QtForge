"""
QtForge Python Bindings Type Stubs

Type definitions for the QtForge C++ extension module.
This file provides type information for mypy static type checking.
"""

from typing import Any, Tuple, Optional, Union
from typing_extensions import Self

# Module-level attributes
__version__: str
__version_major__: int
__version_minor__: int
__version_patch__: int

# Core classes
class PluginManager:
    """Plugin manager for loading and managing plugins."""
    def __init__(self) -> None: ...
    def plugin_count(self) -> int: ...

class Version:
    """Version class for handling semantic versioning."""
    def __init__(self, *args: Union[int, str]) -> None: ...  # Supports both (major, minor, patch) and (version_string)
    def __str__(self) -> str: ...
    def __lt__(self, other: Version) -> bool: ...
    def __le__(self, other: Version) -> bool: ...
    def __gt__(self, other: Version) -> bool: ...
    def __ge__(self, other: Version) -> bool: ...
    def __eq__(self, other: object) -> bool: ...
    def __ne__(self, other: object) -> bool: ...

class PluginMetadata:
    """Plugin metadata class."""
    def __init__(self) -> None: ...
    name: str
    version: Union[str, Version]
    description: str
    author: str

class PluginError(Exception):
    """Plugin error exception."""
    def __init__(self, code: int, message: str) -> None: ...

class PluginErrorCode:
    """Plugin error codes."""
    LOAD_FAILED: int
    INVALID_FORMAT: int
    DEPENDENCY_MISSING: int
    InvalidArgument: int

# Module-level functions
def version() -> str:
    """Get QtForge version string."""
    ...

def version_info() -> Tuple[int, int, int]:
    """Get QtForge version as tuple (major, minor, patch)."""
    ...

def test_function() -> str:
    """Test function for verifying bindings work."""
    ...

def get_version() -> str:
    """Get version string (alias for version())."""
    ...

def create_plugin_manager() -> PluginManager:
    """Create a new PluginManager instance."""
    ...

def load_plugin_demo(plugin_path: str) -> str:
    """Demo function for plugin loading."""
    ...

def utils_test() -> str:
    """Test function for utils module."""
    ...

def create_version(major: int, minor: int, patch: int) -> str:
    """Create a version string from major, minor, patch numbers."""
    ...

def parse_version(version_string: str) -> str:
    """Parse a version string and return formatted result."""
    ...

def create_error(code: int, message: str) -> str:
    """Create an error message with code and description."""
    ...

# Submodules
class CoreModule:
    """Core module containing plugin system functions."""
    def test_function(self) -> str: ...
    def get_version(self) -> str: ...
    def create_plugin_manager(self) -> PluginManager: ...
    def load_plugin_demo(self, plugin_path: str) -> str: ...

class UtilsModule:
    """Utils module containing utility functions."""
    def utils_test(self) -> str: ...
    def create_version(self, major: int, minor: int, patch: int) -> str: ...
    def parse_version(self, version_string: str) -> str: ...
    def create_error(self, code: int, message: str) -> str: ...
    def register_qt_conversions(self) -> None: ...

class MessageBus:
    """Message bus for inter-plugin communication."""
    def __init__(self) -> None: ...

class SecurityManager:
    """Security manager for plugin security."""
    def __init__(self) -> None: ...
    def get_security_level(self) -> str: ...

class ConfigurationManager:
    """Configuration manager for plugin settings."""
    def __init__(self) -> None: ...

class CommunicationModule:
    """Communication module for message bus functionality."""
    MessageBus: type[MessageBus]

class SecurityModule:
    """Security module for plugin security management."""
    SecurityManager: type[SecurityManager]

class ManagersModule:
    """Managers module for various manager classes."""
    ConfigurationManager: type[ConfigurationManager]

core: CoreModule
utils: UtilsModule
communication: CommunicationModule
security: SecurityModule
managers: ManagersModule
