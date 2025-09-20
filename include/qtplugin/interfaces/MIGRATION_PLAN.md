# Interface Reorganization Migration Plan

## Current State Analysis

### Current Interface Locations
- **Core Interfaces**: `include/qtplugin/core/`
  - `plugin_interface.hpp` (IPlugin)
  - `service_plugin_interface.hpp` (IServicePlugin) 
  - `dynamic_plugin_interface.hpp` (IDynamicPlugin)
  - `advanced_plugin_interface.hpp` (IAdvancedPlugin) - **MISSING FILE**

- **Specialized Interfaces**: `include/qtplugin/interfaces/`
  - `ui_plugin_interface.hpp` (IUIPlugin)
  - `data_processor_plugin_interface.hpp` (IDataProcessorPlugin)
  - `network_plugin_interface.hpp` (INetworkPlugin)
  - `scripting_plugin_interface.hpp` (IScriptingPlugin)
  - `interface_validator.hpp` (Interface validation utilities)

- **Embedded Interfaces**: Various directories
  - `ISecurityValidator` in `security/security_validator.hpp`
  - `IMessageBus` in `communication/message_bus.hpp`
  - Service contracts in `communication/plugin_service_contracts.hpp`

### Files That Include Current Interfaces

**Core Interface Dependencies (plugin_interface.hpp):**
- 15+ example plugins
- All core components (plugin_manager, plugin_loader, etc.)
- Bridge implementations (Python, Lua)
- Test files
- Build system files (CMakeLists.txt, xmake.lua, meson.build)

**Specialized Interface Dependencies:**
- `ui_plugin_interface.hpp`: UI examples, capability discovery
- `data_processor_plugin_interface.hpp`: Capability discovery
- `network_plugin_interface.hpp`: Network examples, capability discovery
- `scripting_plugin_interface.hpp`: Capability discovery (currently disabled)

## Migration Strategy

### Phase 1: Backward Compatibility Setup
1. Create forwarding headers in original locations
2. Update build system to include new interface directories
3. Test that existing code continues to work

### Phase 2: Interface Movement
1. Move core interfaces to `interfaces/core/`
2. Move specialized interfaces to appropriate categories
3. Extract embedded interfaces to standalone files
4. Create missing interfaces (IAdvancedPlugin)

### Phase 3: Dependency Updates
1. Update all include statements
2. Update build system files
3. Update documentation and examples
4. Update Python/Lua bindings

## New Interface Organization

```
include/qtplugin/interfaces/
├── core/                    # Base plugin interfaces
│   ├── plugin_interface.hpp
│   ├── advanced_plugin_interface.hpp (NEW)
│   ├── dynamic_plugin_interface.hpp
│   └── service_plugin_interface.hpp
├── data/                    # Data processing interfaces
│   └── data_processor_plugin_interface.hpp
├── ui/                      # User interface interfaces
│   └── ui_plugin_interface.hpp
├── communication/           # Communication interfaces
│   ├── message_bus_interface.hpp (EXTRACTED)
│   └── service_contract_interface.hpp (EXTRACTED)
├── security/                # Security interfaces
│   ├── security_validator_interface.hpp (EXTRACTED)
│   ├── permission_manager_interface.hpp (NEW)
│   └── signature_verifier_interface.hpp (NEW)
├── platform/                # Platform-specific interfaces
│   ├── platform_plugin_interface.hpp (NEW)
│   └── platform_error_handler_interface.hpp (NEW)
├── workflow/                # Workflow interfaces
│   └── workflow_plugin_interface.hpp (NEW)
├── monitoring/              # Monitoring interfaces
│   └── monitoring_plugin_interface.hpp (NEW)
├── resource/                # Resource management interfaces
│   └── resource_manager_interface.hpp (NEW)
├── threading/               # Threading interfaces
│   └── threading_plugin_interface.hpp (NEW)
├── configuration/           # Configuration interfaces
│   └── configuration_plugin_interface.hpp (NEW)
├── scripting/               # Scripting interfaces
│   └── scripting_plugin_interface.hpp
├── network/                 # Network interfaces
│   └── network_plugin_interface.hpp
└── bridge/                  # Language bridge interfaces
    ├── python_bridge_interface.hpp (NEW)
    └── lua_bridge_interface.hpp (NEW)
```

## Backward Compatibility Strategy

### Forwarding Headers
Create forwarding headers in original locations:

```cpp
// include/qtplugin/core/plugin_interface.hpp
#pragma once
#include "../interfaces/core/plugin_interface.hpp"
```

### Build System Updates
- Update CMakeLists.txt QTFORGE_CORE_HEADERS
- Update xmake.lua qtforge_core_headers
- Update meson.build conditional_headers
- Add new interface directories to include paths

### Documentation Updates
- Update all documentation links
- Update README files
- Update API documentation
- Update example code

## Risk Mitigation

### Potential Issues
1. **Circular Dependencies**: New organization might create circular includes
2. **Build Breakage**: Complex dependencies might cause build failures
3. **Missing Interfaces**: Some embedded interfaces might be missed
4. **Version Conflicts**: Interface moves might create compatibility issues

### Mitigation Strategies
1. **Incremental Migration**: Move interfaces one category at a time
2. **Comprehensive Testing**: Test each move before proceeding
3. **Dependency Analysis**: Map all dependencies before moving
4. **Rollback Plan**: Keep original files until migration is complete

## Implementation Order

1. **Core Interfaces** (highest priority - most dependencies)
2. **Specialized Interfaces** (medium priority - existing files)
3. **Extracted Interfaces** (medium priority - need extraction)
4. **New Interfaces** (lowest priority - new functionality)

## Success Criteria

- All existing code compiles without changes
- All tests pass
- Documentation is updated
- Build systems work correctly
- New interface organization is logical and consistent
