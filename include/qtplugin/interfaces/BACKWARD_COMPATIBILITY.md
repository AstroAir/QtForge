# Backward Compatibility Strategy

## Overview

This document outlines the backward compatibility strategy for the interface reorganization. The goal is to ensure that all existing code continues to work without any changes while we reorganize the interface structure.

## Forwarding Header Strategy

### Core Interfaces
Create forwarding headers in the original `core/` directory that include the new interface locations:

```cpp
// include/qtplugin/core/plugin_interface.hpp (forwarding header)
/**
 * @file plugin_interface.hpp
 * @brief Backward compatibility header - forwards to new location
 * @deprecated Use #include "qtplugin/interfaces/core/plugin_interface.hpp" instead
 */
#pragma once

#pragma message("Warning: qtplugin/core/plugin_interface.hpp is deprecated. Use qtplugin/interfaces/core/plugin_interface.hpp instead.")

#include "../interfaces/core/plugin_interface.hpp"
```

### Specialized Interfaces
Keep existing specialized interfaces in their current location initially, then create forwarding headers when moved:

```cpp
// include/qtplugin/interfaces/ui_plugin_interface.hpp (forwarding header)
/**
 * @file ui_plugin_interface.hpp  
 * @brief Backward compatibility header - forwards to new location
 * @deprecated Use #include "qtplugin/interfaces/ui/ui_plugin_interface.hpp" instead
 */
#pragma once

#pragma message("Warning: qtplugin/interfaces/ui_plugin_interface.hpp is deprecated. Use qtplugin/interfaces/ui/ui_plugin_interface.hpp instead.")

#include "ui/ui_plugin_interface.hpp"
```

## Implementation Phases

### Phase 1: Core Interface Movement ✅ COMPLETED
1. ✅ Move core interfaces to `interfaces/core/`
   - `plugin_interface.hpp` moved and updated to v3.2.0
   - `service_plugin_interface.hpp` moved and updated to v3.2.0
   - `dynamic_plugin_interface.hpp` moved and updated to v3.2.0
   - `advanced_plugin_interface.hpp` created (NEW) at v3.2.0
2. ✅ Create forwarding headers in `core/`
   - All core interfaces have forwarding headers with deprecation warnings
   - Backward compatibility maintained for existing code
3. ⏳ Update build system to include new directories (PENDING)
4. ⏳ Test that all existing code compiles (PENDING)

### Phase 2: Specialized Interface Movement  
1. Move specialized interfaces to appropriate categories
2. Create forwarding headers in original locations
3. Update internal includes to use new locations
4. Test that all functionality works

### Phase 3: Gradual Migration
1. Update examples to use new interface locations
2. Update documentation to reference new locations
3. Add deprecation warnings to forwarding headers
4. Plan eventual removal of forwarding headers

## Build System Compatibility

### CMakeLists.txt Updates
```cmake
# Add new interface directories to include path
target_include_directories(QtForgeCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/qtplugin/interfaces>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/qtplugin/interfaces/core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/qtplugin/interfaces/ui>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/qtplugin/interfaces/data>
    # ... other interface directories
)

# Update QTFORGE_CORE_HEADERS to include new locations
set(QTFORGE_CORE_HEADERS
    # New interface locations
    include/qtplugin/interfaces/core/plugin_interface.hpp
    include/qtplugin/interfaces/core/advanced_plugin_interface.hpp
    include/qtplugin/interfaces/core/dynamic_plugin_interface.hpp
    include/qtplugin/interfaces/core/service_plugin_interface.hpp
    
    # Backward compatibility headers (temporary)
    include/qtplugin/core/plugin_interface.hpp
    include/qtplugin/core/service_plugin_interface.hpp
    include/qtplugin/core/dynamic_plugin_interface.hpp
    
    # Other existing headers...
)
```

### XMake Updates
```lua
-- Add new interface directories
add_includedirs("include/qtplugin/interfaces", {public = true})
add_includedirs("include/qtplugin/interfaces/core", {public = true})
add_includedirs("include/qtplugin/interfaces/ui", {public = true})
-- ... other interface directories

-- Update header list
local qtforge_core_headers = {
    -- New interface locations
    "include/qtplugin/interfaces/core/plugin_interface.hpp",
    "include/qtplugin/interfaces/core/advanced_plugin_interface.hpp",
    -- ... other new headers
    
    -- Backward compatibility headers (temporary)
    "include/qtplugin/core/plugin_interface.hpp",
    -- ... other compatibility headers
}
```

## Testing Strategy

### Compilation Tests
1. **Existing Code Test**: Ensure all existing examples compile without changes
2. **New Location Test**: Verify new interface locations work correctly
3. **Mixed Usage Test**: Test that mixing old and new includes works

### Functionality Tests
1. **Plugin Loading**: Verify plugins using old includes still load correctly
2. **Interface Inheritance**: Test that interface inheritance works with forwarding headers
3. **Qt Meta-Object System**: Ensure Q_INTERFACES works with forwarded interfaces

### Build System Tests
1. **CMake Build**: Test that CMake builds work with new structure
2. **XMake Build**: Test that XMake builds work with new structure  
3. **Meson Build**: Test that Meson builds work with new structure

## Migration Timeline

### Immediate (Phase 1)
- Create new interface directory structure
- Move core interfaces with forwarding headers
- Update build systems
- Test compilation

### Short Term (Phase 2-3)
- Move specialized interfaces
- Create missing interfaces
- Update internal code to use new locations
- Update documentation

### Long Term (Phase 4)
- Add deprecation warnings to forwarding headers
- Update all examples to use new locations
- Plan removal of forwarding headers in future version

## Rollback Plan

If issues are encountered:

1. **Immediate Rollback**: Revert to original interface locations
2. **Partial Rollback**: Keep successfully moved interfaces, revert problematic ones
3. **Build System Rollback**: Revert build system changes while keeping new directory structure

## Success Metrics

- ✅ All existing code compiles without changes
- ✅ All tests pass with new interface organization
- ✅ Build systems work correctly
- ✅ Documentation is updated
- ✅ No functionality regressions
- ✅ New interface organization is logical and maintainable

## Communication Plan

1. **Developer Notice**: Inform developers about interface reorganization
2. **Documentation Updates**: Update all relevant documentation
3. **Migration Guide**: Provide guide for updating to new interface locations
4. **Deprecation Timeline**: Communicate timeline for removing forwarding headers
