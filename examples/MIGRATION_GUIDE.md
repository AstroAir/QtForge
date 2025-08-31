# QtForge Examples Migration Guide

**Migrating from the old examples structure to the new organized structure.**

## 🎯 Overview

The QtForge examples have been reorganized to eliminate duplication, improve learning progression, and provide clearer feature separation. This guide helps you migrate from the old structure to the new one.

## 📋 Quick Reference - Path Mapping

| Old Location | New Location | Status |
|--------------|--------------|--------|
| `basic_plugin/` | `01-fundamentals/basic-plugin/` | ✅ Simplified |
| `service_plugin/` | `03-services/background-tasks/` | ✅ Moved |
| `monitoring_plugin/` | `04-specialized/monitoring/` | ✅ Moved |
| `security_plugin/` | `04-specialized/security/` | ✅ Moved |
| `network_plugin/` | `04-specialized/network/` | ✅ Moved |
| `ui_plugin/` | `04-specialized/ui-integration/` | ✅ Moved |
| `communication_examples/` | `02-communication/` (split) | ✅ Reorganized |
| `comprehensive_example/` | `06-comprehensive/full-application/` | ✅ Moved |
| `python/` | `05-integration/python-bindings/` | ✅ Moved |
| `version_management/` | `05-integration/version-management/` | ✅ Moved |

## 🚀 What Changed

### ✅ Improvements

1. **Clear Learning Progression**: Examples now follow a logical learning path from beginner to advanced
2. **Eliminated Duplication**: Removed redundant implementations of the same features
3. **Better Organization**: Grouped examples by functionality and complexity
4. **Simplified Basic Examples**: Created truly minimal examples for beginners
5. **Focused Examples**: Each example now has a clear, unique purpose

### 📦 New Structure

```
examples/
├── 01-fundamentals/          # Essential concepts (NEW)
│   ├── hello-world/          # Minimal plugin (NEW)
│   ├── basic-plugin/         # Simplified from basic_plugin/
│   └── configuration/        # Config patterns (NEW)
├── 02-communication/         # From communication_examples/
│   ├── message-bus/          # Core MessageBus patterns
│   ├── request-response/     # Sync communication
│   └── event-driven/         # Event broadcasting
├── 03-services/              # Service architecture (NEW)
│   ├── background-tasks/     # From service_plugin/
│   ├── service-discovery/    # Service patterns (NEW)
│   └── workflow-orchestration/ # Complex workflows (NEW)
├── 04-specialized/           # Domain-specific features
│   ├── security/             # From security_plugin/
│   ├── monitoring/           # From monitoring_plugin/
│   ├── network/              # From network_plugin/
│   └── ui-integration/       # From ui_plugin/
├── 05-integration/           # External integration
│   ├── python-bindings/      # From python/
│   ├── version-management/   # From version_management/
│   └── marketplace/          # Plugin ecosystem (NEW)
└── 06-comprehensive/         # Complete applications
    ├── full-application/     # From comprehensive_example/
    └── performance-optimized/ # High-performance (NEW)
```

## 🔄 Migration Steps

### Step 1: Update Build References

If you reference examples in your CMakeLists.txt:

```cmake
# OLD
add_subdirectory(examples/basic_plugin)
add_subdirectory(examples/service_plugin)

# NEW
add_subdirectory(examples/01-fundamentals/basic-plugin)
add_subdirectory(examples/03-services/background-tasks)
```

### Step 2: Update Documentation Links

If you link to examples in documentation:

```markdown
<!-- OLD -->
See [basic plugin example](examples/basic_plugin/)
See [service plugin example](examples/service_plugin/)

<!-- NEW -->
See [basic plugin example](examples/01-fundamentals/basic-plugin/)
See [background tasks example](examples/03-services/background-tasks/)
```

### Step 3: Update Include Paths

If you copy code from examples:

```cpp
// OLD
#include "examples/basic_plugin/basic_plugin.hpp"

// NEW
#include "examples/01-fundamentals/basic-plugin/basic_plugin.hpp"
```

### Step 4: Update Learning Materials

If you have tutorials or training materials:

- **Start with**: `01-fundamentals/hello-world/` instead of `basic_plugin/`
- **Communication**: Use `02-communication/` instead of `communication_examples/`
- **Services**: Use `03-services/` instead of `service_plugin/`
- **Complete Demo**: Use `06-comprehensive/full-application/` instead of `comprehensive_example/`

## 🛠️ Automated Migration

### Script for Path Updates

```bash
#!/bin/bash
# migrate_paths.sh - Update file references

# Update CMakeLists.txt files
find . -name "CMakeLists.txt" -exec sed -i 's|examples/basic_plugin|examples/01-fundamentals/basic-plugin|g' {} \;
find . -name "CMakeLists.txt" -exec sed -i 's|examples/service_plugin|examples/03-services/background-tasks|g' {} \;
find . -name "CMakeLists.txt" -exec sed -i 's|examples/comprehensive_example|examples/06-comprehensive/full-application|g' {} \;

# Update documentation files
find . -name "*.md" -exec sed -i 's|examples/basic_plugin|examples/01-fundamentals/basic-plugin|g' {} \;
find . -name "*.md" -exec sed -i 's|examples/service_plugin|examples/03-services/background-tasks|g' {} \;
find . -name "*.md" -exec sed -i 's|examples/comprehensive_example|examples/06-comprehensive/full-application|g' {} \;

echo "Migration complete!"
```

### PowerShell Script (Windows)

```powershell
# migrate_paths.ps1 - Update file references

# Update CMakeLists.txt files
Get-ChildItem -Recurse -Name "CMakeLists.txt" | ForEach-Object {
    (Get-Content $_) -replace 'examples/basic_plugin', 'examples/01-fundamentals/basic-plugin' | Set-Content $_
    (Get-Content $_) -replace 'examples/service_plugin', 'examples/03-services/background-tasks' | Set-Content $_
    (Get-Content $_) -replace 'examples/comprehensive_example', 'examples/06-comprehensive/full-application' | Set-Content $_
}

# Update documentation files
Get-ChildItem -Recurse -Name "*.md" | ForEach-Object {
    (Get-Content $_) -replace 'examples/basic_plugin', 'examples/01-fundamentals/basic-plugin' | Set-Content $_
    (Get-Content $_) -replace 'examples/service_plugin', 'examples/03-services/background-tasks' | Set-Content $_
    (Get-Content $_) -replace 'examples/comprehensive_example', 'examples/06-comprehensive/full-application' | Set-Content $_
}

Write-Host "Migration complete!"
```

## 🔍 Feature Mapping

### If You Were Using...

#### `basic_plugin/` for learning
→ **Start with**: `01-fundamentals/hello-world/` (simpler)
→ **Then**: `01-fundamentals/basic-plugin/` (core concepts)

#### `basic_plugin/` for comprehensive API coverage
→ **Use**: `06-comprehensive/full-application/` (all features)

#### `service_plugin/` for background processing
→ **Use**: `03-services/background-tasks/` (same functionality)

#### `communication_examples/` for MessageBus
→ **Use**: `02-communication/message-bus/` (focused example)

#### `communication_examples/` for request-response
→ **Use**: `02-communication/request-response/` (dedicated example)

#### `comprehensive_example/` for everything
→ **Use**: `06-comprehensive/full-application/` (same, refined)

## ⚠️ Breaking Changes

### 1. Basic Plugin Simplified

The old `basic_plugin/` claimed "100% API coverage" and was overwhelming for beginners.

**Old**: 334 lines, all IPlugin methods
**New**: ~200 lines, essential methods only

**Migration**: If you need comprehensive API coverage, use `06-comprehensive/full-application/`

### 2. Communication Examples Split

The old `communication_examples/` was a single complex example.

**Old**: Single directory with all communication patterns
**New**: Split into focused examples in `02-communication/`

**Migration**: Use specific subdirectories for your use case

### 3. Service Plugin Moved

**Old**: `service_plugin/` (standalone)
**New**: `03-services/background-tasks/` (part of services category)

**Migration**: Update paths, functionality unchanged

## 🆘 Troubleshooting

### Build Errors

**Error**: `CMake Error: Cannot find source file "examples/basic_plugin/CMakeLists.txt"`
**Solution**: Update CMakeLists.txt to use new paths

**Error**: `fatal error: 'examples/basic_plugin/basic_plugin.hpp' file not found`
**Solution**: Update include paths in your code

### Missing Examples

**Issue**: Can't find equivalent of old example
**Solution**: Check the mapping table above or ask in GitHub discussions

### Functionality Differences

**Issue**: New example doesn't have all features of old one
**Solution**: Old examples had duplication - check `06-comprehensive/full-application/` for complete features

## 📞 Support

### Getting Help

1. **Check the mapping table** above for direct equivalents
2. **Review the new README** for learning paths
3. **Open a GitHub discussion** for specific migration questions
4. **Check existing issues** for common problems

### Reporting Issues

If you find migration problems:

1. **Search existing issues** first
2. **Provide specific details**: old path, new path, error message
3. **Include your use case**: what you were trying to accomplish
4. **Suggest improvements**: how could the migration be easier?

## 📅 Timeline

- **Phase 1** (Current): New structure available alongside old
- **Phase 2** (Next release): Deprecation warnings for old structure
- **Phase 3** (Future release): Old structure removed

**Recommendation**: Migrate now to avoid future disruption.

---

**Questions?** Open a [GitHub Discussion](https://github.com/qtforge/qtforge/discussions) or check the [main README](README.md) for the new structure overview.
