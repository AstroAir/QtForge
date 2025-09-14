# Backward Compatibility Plan

## Overview

This document outlines the strategy for maintaining backward compatibility during the Python module reorganization while eliminating duplicate implementations.

## Compatibility Goals

1. **Existing code continues to work** - No breaking changes to public APIs
2. **Gradual migration path** - Allow users to migrate at their own pace
3. **Clear deprecation warnings** - Inform users about preferred new APIs
4. **Documentation updates** - Provide clear migration guidance

## Component-Specific Compatibility

### 1. python_bridge.py Compatibility

#### Current Public API
```python
class PythonPluginBridge:
    # Request handling (unchanged)
    def handle_request(self, request: Dict[str, Any]) -> Dict[str, Any]
    
    # Plugin management (unchanged)
    def load_plugin(self, plugin_path: str, plugin_id: Optional[str] = None)
    def call_method(self, plugin_id: str, method_name: str, params: Optional[List[Any]] = None)
    def get_plugin_info(self, plugin_id: str)
    def unload_plugin(self, plugin_id: str)
    
    # Metadata and discovery (TO BE UPDATED)
    def extract_plugin_metadata(self, plugin_instance) -> Dict[str, Any]
    def discover_plugin_methods(self, plugin_instance) -> List[Dict[str, Any]]
    def discover_plugin_properties(self, plugin_instance) -> List[Dict[str, Any]]
```

#### Compatibility Strategy
- **Keep existing method signatures** - All public methods maintain their current signatures
- **Internal implementation changes** - Replace internal `_get_*` methods with shared utilities
- **Add deprecation warnings** - For any methods that will be removed in future versions
- **Maintain JSON response format** - Ensure response structures remain identical

#### Migration Plan
```python
# Phase 1: Add shared utilities import (backward compatible)
try:
    from src.python.shared import (
        extract_plugin_metadata_dict,
        discover_plugin_methods_dict,
        discover_plugin_properties_dict,
        ModuleWrapper
    )
    SHARED_UTILS_AVAILABLE = True
except ImportError:
    SHARED_UTILS_AVAILABLE = False

# Phase 2: Use shared utilities with fallback
def _get_plugin_metadata(self, plugin_instance):
    if SHARED_UTILS_AVAILABLE:
        return extract_plugin_metadata_dict(plugin_instance)
    else:
        # Fallback to original implementation
        return self._get_plugin_metadata_original(plugin_instance)

# Phase 3: Remove fallback code (future version)
```

### 2. pybind11 Bindings Compatibility

#### Current Public API
```python
# Core module functions (unchanged)
qtforge.get_version()
qtforge.test_function()
qtforge.create_plugin_manager()

# Core classes (unchanged)
qtforge.core.PluginManager
qtforge.core.PluginMetadata
qtforge.core.Version
qtforge.core.PluginState
qtforge.core.PluginCapability

# Utility functions (unchanged)
qtforge.utils.utils_test()
qtforge.utils.create_version()
qtforge.utils.parse_version()
```

#### Compatibility Strategy
- **Maintain all existing classes** - No changes to public C++ binding interfaces
- **Keep module structure** - All existing submodules remain accessible
- **Internal optimization** - Use shared utilities for internal operations where beneficial
- **No API changes** - All existing functions and classes work exactly as before

#### Migration Plan
```cpp
// Phase 1: Add shared utilities integration (internal only)
// No changes to public API

// Phase 2: Optimize internal implementations
// Still no changes to public API

// Phase 3: Add new convenience functions (optional)
module.def("analyze_plugin", [](py::object plugin_instance) {
    // New function using shared utilities
    // Existing code unaffected
});
```

## Import Compatibility Strategy

### 1. Shared Utilities Import Paths

#### Primary Import Path (New)
```python
from src.python.shared import (
    PluginMetadata,
    extract_plugin_metadata,
    discover_plugin_methods,
    ModuleWrapper
)
```

#### Backward Compatibility Imports
```python
# In python_bridge.py - maintain existing internal method names
class PythonPluginBridge:
    def _get_plugin_metadata(self, plugin_instance):
        # Delegate to shared utilities but maintain signature
        from src.python.shared import extract_plugin_metadata_dict
        return extract_plugin_metadata_dict(plugin_instance)
    
    def _get_plugin_methods(self, plugin_instance):
        from src.python.shared import discover_plugin_methods_dict
        return discover_plugin_methods_dict(plugin_instance)
```

### 2. Module Structure Compatibility

#### Before Reorganization
```
python_bridge.py (standalone)
src/python/
├── qtforge_python.cpp
├── core/
│   ├── core_bindings_basic.cpp
│   └── plugin_manager_simple.cpp
└── utils/
    └── utils_bindings.cpp
```

#### After Reorganization
```
python_bridge.py (uses shared utilities)
src/python/
├── qtforge_python.cpp
├── shared/                    # NEW
│   ├── __init__.py
│   ├── qtforge_plugin_utils.py
│   └── module_wrapper.py
├── core/
│   ├── core_bindings_basic.cpp
│   └── plugin_manager_simple.cpp
└── utils/
    └── utils_bindings.cpp
```

## Deprecation Strategy

### 1. Deprecation Timeline

#### Phase 1: Introduction (Current)
- Add shared utilities
- Maintain all existing APIs
- No deprecation warnings

#### Phase 2: Soft Deprecation (Next Release)
- Add deprecation warnings for internal methods that will be removed
- Provide migration documentation
- All existing code still works

#### Phase 3: Hard Deprecation (Future Release)
- Remove deprecated internal methods
- Keep all public APIs
- Provide clear error messages for removed functionality

### 2. Deprecation Warnings

```python
import warnings

def _get_plugin_metadata_original(self, plugin_instance):
    warnings.warn(
        "Internal method _get_plugin_metadata_original is deprecated. "
        "Use shared utilities instead.",
        DeprecationWarning,
        stacklevel=2
    )
    # Original implementation...
```

## Testing Compatibility

### 1. Backward Compatibility Tests

```python
# Test that existing code still works
def test_python_bridge_backward_compatibility():
    bridge = PythonPluginBridge()
    
    # Test that all existing methods still work
    assert hasattr(bridge, 'extract_plugin_metadata')
    assert hasattr(bridge, 'discover_plugin_methods')
    assert hasattr(bridge, 'discover_plugin_properties')
    
    # Test that method signatures are unchanged
    # Test that return formats are identical

def test_pybind11_backward_compatibility():
    import qtforge
    
    # Test that all existing modules are available
    assert hasattr(qtforge, 'core')
    assert hasattr(qtforge, 'utils')
    
    # Test that all existing functions work
    assert callable(qtforge.get_version)
    assert callable(qtforge.test_function)
```

### 2. Migration Tests

```python
# Test that new shared utilities work correctly
def test_shared_utilities_integration():
    from src.python.shared import extract_plugin_metadata
    
    # Test with sample plugin
    plugin = SamplePlugin()
    metadata = extract_plugin_metadata(plugin)
    
    # Verify metadata structure matches expected format
    assert isinstance(metadata, PluginMetadata)
    assert metadata.name == plugin.name
```

## Documentation Migration

### 1. Update Examples

#### Before (python_bridge.py usage)
```python
bridge = PythonPluginBridge()
metadata = bridge.extract_plugin_metadata(plugin_instance)
```

#### After (still works, but new option available)
```python
# Option 1: Existing code (unchanged)
bridge = PythonPluginBridge()
metadata = bridge.extract_plugin_metadata(plugin_instance)

# Option 2: Direct use of shared utilities (new)
from src.python.shared import extract_plugin_metadata
metadata = extract_plugin_metadata(plugin_instance)
```

### 2. Migration Guide

Create comprehensive migration guide covering:
- What changed and why
- How to update code (if desired)
- Compatibility guarantees
- Timeline for any future changes

## Error Handling Compatibility

### 1. Maintain Error Formats

```python
# Ensure error responses maintain the same structure
def handle_request(self, request):
    try:
        # New implementation using shared utilities
        pass
    except Exception as e:
        # Maintain exact same error response format
        return {
            "success": False,
            "error": str(e),
            "traceback": traceback.format_exc(),
            "id": request.get("id", 0)
        }
```

### 2. Error Message Compatibility

- Keep error message formats consistent
- Maintain error codes where they exist
- Ensure stack traces point to meaningful locations

## Rollback Strategy

### 1. Fallback Implementation

```python
# Keep original implementations as fallback
class PythonPluginBridge:
    def __init__(self):
        self.use_shared_utils = True
        try:
            from src.python.shared import extract_plugin_metadata_dict
            self._shared_extract_metadata = extract_plugin_metadata_dict
        except ImportError:
            self.use_shared_utils = False
    
    def _get_plugin_metadata(self, plugin_instance):
        if self.use_shared_utils:
            return self._shared_extract_metadata(plugin_instance)
        else:
            return self._get_plugin_metadata_original(plugin_instance)
```

### 2. Configuration Option

```python
# Allow users to disable shared utilities if needed
QTFORGE_USE_SHARED_UTILS = os.environ.get('QTFORGE_USE_SHARED_UTILS', 'true').lower() == 'true'
```

## Success Criteria

1. **All existing tests pass** without modification
2. **No breaking changes** to public APIs
3. **Performance maintained or improved**
4. **Clear migration path** for future enhancements
5. **Comprehensive documentation** for any changes
