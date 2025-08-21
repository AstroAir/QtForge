# Migration Guide

## Upgrading to 3.0.0

### Breaking Changes
- Public API remains backward compatible for most use cases. Review the new component-based architecture for advanced customization.

### Recommended Actions
- Audit includes: prefer including <qtplugin/qtplugin.hpp> rather than individual headers unless you need specific components.
- Review security configuration defaults. The default SecurityLevel is Medium.

## From 2.x to 3.0
- Replace direct PluginManager construction with the new builder if you need fine-grained component selection.

## Deprecations
- Some platform-specific helpers are temporarily disabled behind build flags. See CMake options.

