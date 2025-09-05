"""
QtForge Marketplace Module Type Stubs

This module provides plugin marketplace and distribution functionality.
"""

from typing import Dict, List, Any, Optional

class PluginInfo:
    """Plugin information from marketplace."""
    plugin_id: str
    name: str
    version: str
    description: str
    author: str
    category: str
    tags: List[str]
    download_url: str
    size_bytes: int
    rating: float
    downloads: int
    
    def __init__(self) -> None: ...

class PluginReview:
    """Plugin review from marketplace."""
    reviewer: str
    rating: int
    comment: str
    date: str
    
    def __init__(self) -> None: ...

class InstallationResult:
    """Result of plugin installation."""
    success: bool
    plugin_id: str
    version: str
    error_message: str
    
    def __init__(self) -> None: ...

class PluginMarketplace:
    """Plugin marketplace client."""
    def __init__(self, marketplace_url: str, parent: Optional[Any] = None) -> None: ...
    
    def initialize(self) -> bool: ...
    def search_plugins(self, query: str, category: str = "", tags: List[str] = []) -> List[PluginInfo]: ...
    def get_plugin_details(self, plugin_id: str) -> Optional[PluginInfo]: ...
    def get_plugin_reviews(self, plugin_id: str) -> List[PluginReview]: ...
    def install_plugin(self, plugin_id: str, version: str = "latest") -> InstallationResult: ...
    def update_plugin(self, plugin_id: str) -> InstallationResult: ...
    def uninstall_plugin(self, plugin_id: str) -> bool: ...
    def get_installed_plugins(self) -> List[PluginInfo]: ...
    def check_for_updates(self) -> List[PluginInfo]: ...
    def get_categories(self) -> List[str]: ...
    def get_featured_plugins(self) -> List[PluginInfo]: ...
    def set_api_key(self, api_key: str) -> None: ...
    def is_authenticated(self) -> bool: ...

# Utility functions
def create_marketplace(marketplace_url: str) -> PluginMarketplace: ...
